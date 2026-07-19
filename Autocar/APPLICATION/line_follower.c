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

int i = 0;

bool left_flage = false;
bool trun_flage = false;

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
        out_num++;
        actual_speed = 0;
        if (out_num >= 3)
        {
            trun_flage = true;
            {
                static uint32_t last_warn_ms = 0;
                if (tick_ms - last_warn_ms >= 500)
                {
                    last_warn_ms = tick_ms;
                    USART_SendString((unsigned char*)"[WARN] All-white detected, entering turn mode\r\n");
                }
            }
        }
        integral = 0;
        return 0;
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

    return output;

}

void car_trun(int8_t trun_speed)
{
    car_run(0, trun_speed);
    if(!s4 || !s5)
    {
        trun_flage = false;
    }
}

void CAR_CONTROL(void)
{
    if(!trun_flage)
    {
        car_run(base_speed, line_folower(kp, ki, kd));
    }
    else 
    {
        if(left_flage)
        {
            car_trun(100); 
        }
        else 
        {
            car_trun(-100);
        }
    }
}