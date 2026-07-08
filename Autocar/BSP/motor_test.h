/*
 * motor_test.h — 单电机开环控制 API（Motor A）
 *
 * 使用项目已有的 SysConfig 引脚定义，无需额外 GPIO 初始化。
 * 硬件映射:
 *   PWM   → PA26 (TIMG8 CCP0, SysConfig 已配)
 *   IN1   → PA27 (GPIO_MOTOR_AIN1, SysConfig 已配为输出)
 *   IN2   → PA25 (GPIO_MOTOR_AIN2, SysConfig 已配为输出)
 *   STBY  → PB0  (GPIO_MOTOR_STBY, 在 main 里拉高)
 */
#ifndef _MOTOR_TEST_H_
#define _MOTOR_TEST_H_

#include <stdint.h>

/* ══════════════ API ══════════════ */

/** 初始化 Motor A — 停转电机（GPIO 方向已由 SYSCFG_DL_init 配置） */
void MotorTest_Init(void);

/** 正转, speed: 0(停) ~ 999(最快) */
void MotorTest_Forward(uint16_t speed);

/** 反转, speed: 0(停) ~ 999(最快) */
void MotorTest_Reverse(uint16_t speed);

/** 停止 (IN1=IN2=0, 刹车) */
void MotorTest_Stop(void);

/** 统一速度设置: >0 正转, <0 反转, =0 停止, 范围 -999~999 */
void MotorTest_SetSpeed(int16_t speed);

#endif
