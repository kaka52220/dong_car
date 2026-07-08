/*
 * motor_test.c — 单电机开环控制（Motor A）
 *
 * 完全开环：不依赖编码器、不依赖 PID。
 * 直接控制 H 桥方向引脚 + PWM 占空比。
 *
 * H 桥真值表:
 *   IN1=0 IN2=0 → 刹车
 *   IN1=0 IN2=1 → 正转
 *   IN1=1 IN2=0 → 反转
 *
 * PWM 公式 (edge-align, init=HIGH, period=1000):
 *   compare = 999 - speed
 *   speed=0   → 999 → 几乎无输出
 *   speed=999 → 0   → 几乎全开
 */
#include "motor_test.h"
#include "ti_msp_dl_config.h"

/* ══════════════ 初始化 ══════════════ */
void MotorTest_Init(void)
{
    MotorTest_Stop();
}


// void MotorTest_Forward(uint16_t speed)
// {
//     if (speed > 999) speed = 999;

//  DL_GPIO_setPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
//         DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);

//     DL_TimerG_setCaptureCompareValue(PWMC_INST,
//         999 - speed, GPIO_PWMC_C1_IDX);//定时器外设,比较寄存器,比较通道
// }
// void MotorTest_Stop(void)
// {
//     DL_GPIO_setPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);  /* IN1=0 */
//     DL_GPIO_setPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);  /* IN2=0 */

//     DL_TimerG_setCaptureCompareValue(PWMC_INST,
//         999, GPIO_PWMC_C1_IDX);//定时器外设,比较寄存器,比较通道
// }
// void MotorTest_Reverse(uint16_t speed)
// {
//     if (speed > 999) speed = 999;

// DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
// 		DL_GPIO_setPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);

//     DL_TimerG_setCaptureCompareValue(PWMC_INST,
//         999 - speed, GPIO_PWMC_C1_IDX);
// }


// void MotorTest_Forward(uint16_t speed)
// {
//     if (speed > 999) speed = 999;

//  DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
//         DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);

//     DL_TimerG_setCaptureCompareValue(PWMD_INST,
//         999 - speed, GPIO_PWMD_C1_IDX);//定时器外设,比较寄存器,比较通道
// }
// void MotorTest_Stop(void)
// {
//     DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);  /* IN1=0 */
//     DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);  /* IN2=0 */

//     DL_TimerG_setCaptureCompareValue(PWMD_INST,
//         999, GPIO_PWMD_C1_IDX);//定时器外设,比较寄存器,比较通道
// }
// void MotorTest_Reverse(uint16_t speed)
// {
//     if (speed > 999) speed = 999;

// DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
// 		DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);

//     DL_TimerG_setCaptureCompareValue(PWMD_INST,
//         999 - speed, GPIO_PWMD_C1_IDX);
// }


///////////B
void MotorTest_Forward(uint16_t speed)
{
    if (speed > 999) speed = 999;

 DL_GPIO_setPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
        DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);

    DL_TimerG_setCaptureCompareValue(PWMB_INST,
        999 - speed, GPIO_PWMB_C1_IDX);//定时器外设,比较寄存器,比较通道
}
void MotorTest_Stop(void)
{
    DL_GPIO_setPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);  /* IN1=0 */
    DL_GPIO_setPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);  /* IN2=0 */

    DL_TimerG_setCaptureCompareValue(PWMB_INST,
        999, GPIO_PWMB_C1_IDX);//定时器外设,比较寄存器,比较通道
}
void MotorTest_Reverse(uint16_t speed)
{
    if (speed > 999) speed = 999;

DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
		DL_GPIO_setPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);

    DL_TimerG_setCaptureCompareValue(PWMB_INST,
        999 - speed, GPIO_PWMB_C1_IDX);
}




/* ══════════════ 统一速度接口 ══════════════ */
void MotorTest_SetSpeed(int16_t speed)
{
    if (speed > 0)
        MotorTest_Forward((uint16_t)speed);
    else if (speed < 0)
        MotorTest_Reverse((uint16_t)(-speed));
    else
        MotorTest_Stop();
}
