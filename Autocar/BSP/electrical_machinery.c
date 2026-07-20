#include "electrical_machinery.h"
#include "ti_msp_dl_config.h"
#include "usart.h"
//#include "draw.h"
#include "interrupt.h"
#include "clock.h"
#include "stdio.h"

int MAX=0;
int MIN=0;

float Velcity_Kp=0.5,  Velcity_Ki=0.3,  Velcity_Kd;

/*===================================================================
 *  速度计算 (mm/s) — 13线编码器 + 65mm轮径, 20ms采样
 *
 *  编码器 ISR: A相+B相都触发 → 每圈 26 中断
 *  减速比 1:28 → 车轮每圈 = 26×28 = 728 中断
 *  瞬时速度 = |脉冲数| × (π×65/728) × 50Hz = |脉冲数| × 14.03 mm/s
 *  取绝对值: 方向由 MOTOR_CONTROL 的 IN1/IN2 控制, 速度只算大小
 *  一阶低通滤波: speed = α×instant + (1-α)×speed
 *===================================================================*/
#define SPEED_FILTER_ALPHA   0.5f
#define PULSE_TO_MMS         14.03f  /* π×65/(13×2×28)×50 */

static float motor_speed_A, motor_speed_B, motor_speed_C, motor_speed_D;

float calculate_motor_speed(char motor_name)
{
    int   pulse   = get_encoder_count(motor_name);
    /* 取绝对值: 编码器ISR用±计数表示方向, 速度只看大小 */
    int   mag     = (pulse >= 0) ? pulse : -pulse;
    float instant = (float)mag * PULSE_TO_MMS;
    float *sp     = NULL;

    switch (motor_name) {
    case 'A': sp = &motor_speed_A; break;
    case 'B': sp = &motor_speed_B; break;
    case 'C': sp = &motor_speed_C; break;
    case 'D': sp = &motor_speed_D; break;
    default:  return 0;
    }
    *sp = SPEED_FILTER_ALPHA * instant + (1.0f - SPEED_FILTER_ALPHA) * (*sp);
    return *sp;
}

void reset_motor_speed(char motor_name)
{
    switch (motor_name) {
    case 'A': motor_speed_A = 0; break;
    case 'B': motor_speed_B = 0; break;
    case 'C': motor_speed_C = 0; break;
    case 'D': motor_speed_D = 0; break;
    }
}

/*===================================================================
 *  PI 内部状态 — 提到文件作用域，便于 Velocity_ResetAll 重置
 *
 *  原本 ControlVelocity/Last_bias 是 Velocity_X 函数内的 static 变量，
 *  外部无法访问。丢线瞬间 target 从 ±400 突变到 200 时，旧的 CV
 *  还停留在尖锐转弯积累的大值（可能 ±800），需要数十周期收敛，
 *  期间电机被过度驱动 → 速度飙升。
 *
 *  现提到文件作用域，Velocity_ResetAll 可直接重置。
 *===================================================================*/
static int cv_A=0, lb_A=0;
static int cv_B=0, lb_B=0;
static int cv_C=0, lb_C=0;
static int cv_D=0, lb_D=0;

int Velocity_A(int TargetVelocity, int CurrentVelocity)
{
    int Bias = TargetVelocity - CurrentVelocity;
    cv_A += Velcity_Kp*(Bias - lb_A) + Velcity_Ki*Bias;
    if(cv_A > 999)  cv_A = 999;
    if(cv_A < -999) cv_A = -999;
    lb_A = Bias;
    return cv_A;
}

int Velocity_B(int TargetVelocity, int CurrentVelocity)
{
    int Bias = TargetVelocity - CurrentVelocity;
    cv_B += Velcity_Kp*(Bias - lb_B) + Velcity_Ki*Bias;
    if(cv_B > 999)  cv_B = 999;
    if(cv_B < -999) cv_B = -999;
    lb_B = Bias;
    return cv_B;
}

int Velocity_C(int TargetVelocity, int CurrentVelocity)
{
    int Bias = TargetVelocity - CurrentVelocity;
    cv_C += Velcity_Kp*(Bias - lb_C) + Velcity_Ki*Bias;
    if(cv_C > 999)  cv_C = 999;
    if(cv_C < -999) cv_C = -999;
    lb_C = Bias;
    return cv_C;
}

int Velocity_D(int TargetVelocity, int CurrentVelocity)
{
    int Bias = TargetVelocity - CurrentVelocity;
    cv_D += Velcity_Kp*(Bias - lb_D) + Velcity_Ki*Bias;
    if(cv_D > 999)  cv_D = 999;
    if(cv_D < -999) cv_D = -999;
    lb_D = Bias;
    return cv_D;
}

/*===================================================================
 *  Velocity_ResetAll — 重置 4 路 PI 的 CV 和 Last_bias
 *
 *  作用: 当 target 发生大幅突变时（如丢线瞬间从 ±400 突变到 200），
 *        旧的 CV 还停留在尖锐转弯积累的大值，需要数十周期才能收敛，
 *        期间电机被过度驱动 → 速度飙升。本函数立即把 CV 重置到
 *        与新 target 匹配的初值，避免积分饱和引发的超调。
 *
 *  调用时机: line_folower 检测到全白丢线的第一个周期。
 *  seed_target: 期望重置后维持的目标速度（mm/s），CV 按经验比例
 *               seed_target * 2.2 初始化，避免从 0 起步导致电机
 *               瞬间停转再重启的抖动。
 *===================================================================*/
void Velocity_ResetAll(int seed_target)
{
    /* 经验值: PWM ≈ speed * 2.2 (实测 200mm/s 对应 PWM≈440) */
    int seed_cv = (int)(seed_target * 2.2f);
    if (seed_cv > 800)  seed_cv = 800;
    if (seed_cv < -800) seed_cv = -800;

    cv_A = seed_cv;  lb_A = 0;
    cv_B = seed_cv;  lb_B = 0;
    cv_C = seed_cv;  lb_C = 0;
    cv_D = seed_cv;  lb_D = 0;
}

void MOTOR_CONTROL(int TargetVelocity_A, int TargetVelocity_B, int TargetVelocity_C, int TargetVelocity_D)
{
    int ControlVelocity_A = Velocity_A(TargetVelocity_A, (int)calculate_motor_speed('A'));
    int ControlVelocity_B = Velocity_B(TargetVelocity_B, (int)calculate_motor_speed('B'));
    int ControlVelocity_C = Velocity_C(TargetVelocity_C, (int)calculate_motor_speed('C'));
    int ControlVelocity_D = Velocity_D(TargetVelocity_D, (int)calculate_motor_speed('D'));
	// int ControlVelocity_A = TargetVelocity_A;
    // int ControlVelocity_B = TargetVelocity_B;
    // int ControlVelocity_C =  TargetVelocity_C;
    // int ControlVelocity_D = TargetVelocity_D;

		if(ControlVelocity_A > 0)
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
    }
    else if (ControlVelocity_A == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
    }
    else if (ControlVelocity_A < 0) 
	{
        DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN); 
        ControlVelocity_A = -ControlVelocity_A;
    }

	if(ControlVelocity_B > 0)
	{
         DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
	     DL_GPIO_setPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
 
    }
    else if (ControlVelocity_B == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
    }
    else if (ControlVelocity_B < 0) 
	{
        DL_GPIO_setPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        ControlVelocity_B = -ControlVelocity_B;
    }

	if(ControlVelocity_C > 0)
	{
        DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
    }
    else if (ControlVelocity_C == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
    }
    else if (ControlVelocity_C < 0) 
	{
        DL_GPIO_setPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
        ControlVelocity_C = -ControlVelocity_C;
    }

	if(ControlVelocity_D > 0)
	{
      DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
    }
    else if (ControlVelocity_D == 0) 
	{
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
    }
    else if (ControlVelocity_D < 0) 
	{
         DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        ControlVelocity_D = -ControlVelocity_D;
    }
    // ===== 每 50ms 打印 PID 输出（调试用） =====
    //{//独立作用域
    //    static uint32_t last_print = 0;
    //    if (tick_ms - last_print >= 50)
    //    {
    //        last_print = tick_ms;
    //        char buf[60];
    //        sprintf(buf, "A=%d B=%d C=%d D=%d\r\n",
    //            ControlVelocity_A, ControlVelocity_B,
    //            ControlVelocity_C, ControlVelocity_D);
    //        USART_SendString((unsigned char*)buf);
    //    }
    //}

    DL_TimerG_setCaptureCompareValue(PWMA_INST, ControlVelocity_A, GPIO_PWMA_C0_IDX);
	DL_TimerG_setCaptureCompareValue(PWMB_INST, ControlVelocity_B, GPIO_PWMB_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMC_INST, ControlVelocity_C, GPIO_PWMC_C1_IDX);
	DL_TimerG_setCaptureCompareValue(PWMD_INST, ControlVelocity_D, GPIO_PWMD_C1_IDX);
}

/*
 * 绕过增量式 PI，直接设置 PWM 和方向（供 car_trun 使用）。
 * pwm 正值=正转，负值=反转，范围 ±999。
 */
void MOTOR_RAW(int A, int B, int C, int D)
{
    #define RAW_SET(INST, CH, VAL, IN1_PORT, IN1_PIN, IN2_PORT, IN2_PIN)  \
    do {                                                                    \
        int v = (VAL);                                                      \
        if (v > 0) {                                                        \
            DL_GPIO_setPins(IN2_PORT, IN2_PIN);                             \
            DL_GPIO_clearPins(IN1_PORT, IN1_PIN);                           \
        } else if (v < 0) {                                                 \
            DL_GPIO_clearPins(IN2_PORT, IN2_PIN);                           \
            DL_GPIO_setPins(IN1_PORT, IN1_PIN);                             \
            v = -(v);                                                       \
        } else {                                                            \
            DL_GPIO_clearPins(IN1_PORT, IN1_PIN);                           \
            DL_GPIO_clearPins(IN2_PORT, IN2_PIN);                           \
        }                                                                   \
        if (v > 999) v = 999;                                               \
        DL_TimerG_setCaptureCompareValue(INST, v, CH);                      \
    } while(0)

    /* 车头换向后 MOTOR_CONTROL 极性已全部反转，MOTOR_RAW 需同步：
     *   A 新极性: IN1 高前进 → 对调 IN1/IN2 参数
     *   B 新极性: IN2 高前进 → 不对调（恢复原样）
     *   C 新极性: IN2 高前进 → 不对调（恢复原样）
     *   D 新极性: IN1 高前进 → 对调 IN1/IN2 参数
     * RAW_SET 宏 v>0 时 "宏IN2" 置高，所以把物理上要高的那个引脚
     * 放到宏的 IN2 位置（第6,7参数）。 */
    RAW_SET(PWMA_INST, GPIO_PWMA_C0_IDX, A,
           GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN,
           GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
    RAW_SET(PWMB_INST, GPIO_PWMB_C1_IDX, B,
           GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN,
           GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
    RAW_SET(PWMC_INST, GPIO_PWMC_C1_IDX, C,
           GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN,
           GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
    RAW_SET(PWMD_INST, GPIO_PWMD_C1_IDX, D,
           GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN,
           GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
    #undef RAW_SET
}

void car_run(int base_speed_pct, float differential)
{
    int base_speed = base_speed_pct * 4;
    //浮点运算保证精度，最后再转int
    float left_speed_f  = base_speed - differential / 2.0f;
    float right_speed_f = base_speed + differential / 2.0f;

    int left_speed  = (int)left_speed_f;
    int right_speed = (int)right_speed_f;

    //限幅不变
    if(left_speed > 400) left_speed = 400;
    else if(left_speed < -400) left_speed = -400;
    if(right_speed > 400) right_speed = 400;
    else if(right_speed < -400) right_speed = -400;

    /* 车头换向后电机位置变更：A,B=左轮, C,D=右轮（原 A,B=右, C,D=左）。
     * MOTOR_CONTROL 参数顺序固定为 (A,B,C,D)，所以左轮目标传给 A,B，
     * 右轮目标传给 C,D。MOTOR_CONTROL 内部方向极性已测试正确，不动。 */
    MOTOR_CONTROL(left_speed, left_speed, right_speed, right_speed);
}

void car_stop()
{
	car_run(0, 0);
}