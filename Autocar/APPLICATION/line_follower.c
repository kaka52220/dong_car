#include "line_follower.h"
#include "ti_msp_dl_config.h"
#include "grayscale_sensor.h"
#include "usart.h"
#include "draw.h"
#include "clock.h"
#include "electrical_machinery.h"

#include "stdbool.h"
// float kp = 7.0;
// float ki = 0.2;
// float kd = 0.5;
// int8_t base_speed = 200;
//bool   stop_flag = false;


unsigned char IRbuf[2];
uint8_t s1, s2, s3, s4, s5, s6, s7, s8;
uint8_t out_num = 0;
uint8_t actual_speed;
static float error = 0, last_error = 0, integral = 0;
static float last_valid_diff = 0;  /* 上次有效差速，丢线续行时用 */

int i = 0;

bool left_flage = false;
bool trun_flage = false;

/* 转弯方向锁定与退出防抖（文件作用域）
 * turn_dir_locked: 进入转弯模式时锁定，过程中不改，避免扫线抖动
 * turn_exit_cnt: 中心线检测防抖计数，要求连续 N 次才退出转弯 */
static bool turn_dir_locked = false;   /* true=左转, false=右转 */
static uint8_t turn_exit_cnt = 0;

void line_follower_update(void)
{
    uint8_t raw[8];
    Grayscale_Sensor_Read_All(raw);

    s1 = !raw[0];  s2 = !raw[1];
    s3 = !raw[2];  s4 = !raw[3];
    s5 = !raw[4];  s6 = !raw[5];
    s7 = !raw[6];  s8 = !raw[7];
}

/*
 * I2C 地址扫描（调试用，已注释）
 * int i2c_scan(void);
 */
void dayin(uint8_t *test)
{
    test[0] = s1;
    test[1] = s2;
    test[2] = s3;
    test[3] = s4;
    test[4] = s5;
    test[5] = s6;
    test[6] = s7;
    test[7] = s8;

 for (i = 0; i < 8; i++)
   {
         USART_SendData(test[i] + '0');
   }
     //  USART_SendString("\r\n");    
	delay_ms(100);
}
float line_folower(float kp, float ki, float kd)
{
    actual_speed = base_speed;

    /* ══════════════ 第1步：加权求和计算 error ══════════════ */
    const int8_t weight[8] = {7, 5, 3, 1, -1, -3, -5, -7};
    const uint8_t data[8]  = {s1, s2, s3, s4, s5, s6, s7, s8};
    int32_t sum_w    = 0;
    uint8_t black_cnt = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (!data[i]) { sum_w += weight[i]; black_cnt++; }  /* s=0 为黑线 */
    }

    /* ══════════════ 第2步：全白线检测 ══════════════ */
    if (black_cnt == 0)
    {
        /* 丢线续行：降速 + 保持上次差速方向，避免直行冲一段。
         * 原方案 return 0 → 四轮等速直行 → 丢线后"往前冲一段"。
         * 现在返回上次差速的一半，车继续差速转弯（减速），平滑过渡到转弯模式。 */
        actual_speed = base_speed / 2;           /* 降速到 50% */
        if (out_num == 0)
        {
            /* 第一周期丢线: 重置 CV 匹配降速后的 target，避免 CV 残留大值导致冲 */
            Velocity_ResetAll(actual_speed * 4);
        }
        out_num++;
        if (out_num >= 2)                        /* 原 >= 3，减少直行窗口 */
        {
            trun_flage = true;
            /* 用丢线前 last_error 的符号锁定直角转弯方向。
             * last_error 是丢线前最后一次有效巡线的误差，综合了所有传感器：
             *   > 0 → 线在左侧 → 左转去找线
             *   < 0 → 线在右侧 → 右转去找线
             * 加阈值避免 error 在 0 附近抖动导致方向乱切。
             * |last_error| 较小时保持上次的 turn_dir_locked。 */
            if (last_error > 1.0f)       turn_dir_locked = true;
            else if (last_error < -1.0f) turn_dir_locked = false;
            turn_exit_cnt = 0;
            {
                static uint32_t last_warn_ms = 0;
                if (tick_ms - last_warn_ms >= 500)
                {
                    last_warn_ms = tick_ms;
                    // USART_SendString((unsigned char*)"[WARN] All-white detected, entering turn mode\r\n");  /* VOFA 调试中，注释掉 */
                }
            }
        }
        integral = 0;
        return last_valid_diff * 0.5f;           /* 保持差速方向，幅度减半 */
    }

    /* ══════════════ 第3步：归一化 error + left_flage ══════════════ */
    out_num = 0;
    error   = (float)sum_w / (float)black_cnt;  /* 浮点除法，加权平均 */

    if (!s1)       left_flage = true;
    else if (!s8)  left_flage = false;

    /* ══════════════ 第4步：误差死区（|error|<2 → 0，抑制直行抖动） ══════════════ */
    if (error > -1 && error < 1) error = 0;

    /* ══════════════ 第5步：积分限幅（防饱和） ══════════════ */
    integral += error;
    {
        const float IMAX = 100.0f;
        if (integral >  IMAX) integral =  IMAX;
        if (integral < -IMAX) integral = -IMAX;
    }

    /* ══════════════ 第6步：位置式 PID ══════════════ */
    float output = kp * error + ki * integral + kd * (error - last_error);
    last_error = error;

    /* ══════════════ 第7步：输出限幅 ══════════════ */
    {
        const float OMAX = 400.0f;
        if (output >  OMAX) output =  OMAX;
        if (output < -OMAX) output = -OMAX;
    }

    last_valid_diff = output;   /* 保存有效差速，供丢线续行用 */
    return output;

}

void car_trun(int trun_pwm)
{
    /* 差速倒退转弯：一边正常前进，另一边一半速度倒退。
     * 比单边驱动（一边停一边转）转弯半径更小，适合直角循迹。
     *
     * 车头换向后电机位置：A,B=左轮, C,D=右轮。
     * 方向由 turn_dir_locked 决定（进入转弯时锁定，过程中不改）。
     * MOTOR_RAW(A,B,C,D) 中 A,B=左轮, C,D=右轮。
     *
     * 小冲防护：退出转弯时调 Velocity_ResetAll 把 CV 重置到匹配 base_speed
     * 的前进值，倒退的轮子切回前进时 PI 从正确初值开始，不会冲。 */
    if (turn_dir_locked)
    {
        /* 左转：右轮 C,D 正常前进，左轮 A,B 一半速度倒退 */
        MOTOR_RAW(-trun_pwm/2, -trun_pwm/2, trun_pwm, trun_pwm);
    }
    else
    {
        /* 右转：左轮 A,B 正常前进，右轮 C,D 一半速度倒退 */
        MOTOR_RAW(trun_pwm, trun_pwm, -trun_pwm/2, -trun_pwm/2);
    }

    /* 退出转弯：中间四个传感器(s3~s6)任意一个看到线（s=0 为黑线）就退出。
     * 原条件 !s4&&!s5（s4和s5都有线）太严格，车体歪时可能只扫到s3或s6→不退出→转过头。
     * 用 s3~s6 而非 s1~s8：外侧传感器在转弯时可能误扫到旁边线（十字/平行线），
     * 中间四个覆盖车体正前方，车转到大致对准线时才触发，既不误触发也不太严格。 */
    if (!s3 || !s4 || !s5 || !s6)
    {
         trun_flage = false;
         Velocity_ResetAll(base_speed * 4);
    }
}

void CAR_CONTROL(void)
{
    if(!trun_flage)
    {
        /* 用 actual_speed 而非 base_speed：丢线时 line_folower 会把
         * actual_speed 降到 base_speed/2，避免丢线窗口直行冲一段。
         * 正常巡线时 actual_speed = base_speed，行为不变。 */
        car_run(actual_speed, line_folower(kp, ki, kd));
    }
    else
    {
        /* 方向已在 turn_dir_locked 里锁定，这里只传 PWM 值。
         * 300 PWM ≈ 30% 占空比，单边驱动转弯速度适中。
         * 若转弯太慢/太快，调整此值（建议 200-500）。 */
        car_trun(200);
    }
}