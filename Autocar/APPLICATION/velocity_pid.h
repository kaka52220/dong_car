/**
 * @file    velocity_pid.h
 * @brief   四驱小车 — 增量式PI速度环
 * @note    编码器: 13线, 轮径: 65mm, 芯片: MSPM0G3507
 *
 * 使用方式:
 *   1. main() 中调用 VelPID_InitAll() 初始化 4 路电机
 *   2. 在 20ms 定时器 ISR 中调用 VelPID_UpdateAll()
 *   3. main() 中调用 VelPID_SetTarget() 设定目标速度
 *
 * 速度单位: mm/s (物理线速度)
 * 采样周期: 20ms (与编码器 TIMG0 采样对齐)
 */

#ifndef __VELOCITY_PID_H__
#define __VELOCITY_PID_H__
#include "ti_msp_dl_config.h"
#include <stdint.h>

/*===================================================================
 *  硬件参数 — 按实际修改
 *===================================================================*/
#define ENCODER_PPR        13      // 编码器线数 (脉冲/圈)
#define WHEEL_DIAMETER_MM  65.0f   // 轮径 (mm)
#define PID_PERIOD_MS      20      // PID 采样周期 (ms), 需与编码器采样对齐

/* PWM 硬件参数 */
#define PWM_MAX            999     // PWM 周期 (自动重载值), 你当前用 999

/*===================================================================
 *  电机索引
 *===================================================================*/
#define MOTOR_A   0
#define MOTOR_B   1
#define MOTOR_C   2
#define MOTOR_D   3
#define MOTOR_NUM 4

/*===================================================================
 *  单电机 PI 状态结构体
 *===================================================================*/
typedef struct {
    /*--- 用户可调参数 ---*/
    float kp;               // 比例系数
    float ki;               // 积分系数

    /*--- 目标速度 (mm/s) ---*/
    float target_speed;     // 用户设定的目标线速度

    /*--- 速度估计 (mm/s, 已滤波) ---*/
    float speed;            // 当前滤波后速度

    /*--- PI 内部状态 ---*/
    float last_error;       // e[n-1], 用于增量式计算 Δu = Kp*(e[n]-e[n-1]) + Ki*e[n]
    int32_t encoder_prev;   // 上一周期的编码器累加值 (用于计算增量)

    /*--- PWM 输出 ---*/
    int16_t pwm;            // 当前 PWM 占空比 [0, PWM_MAX]

    /*--- 保护 ---*/
    uint8_t stall_cnt;      // 堵转持续计数
    uint8_t cooldown;       // 堵转后冷却计数
} VelPID_t;

/*===================================================================
 *  全局电机数组 (在 .c 中定义)
 *===================================================================*/
extern VelPID_t motors[MOTOR_NUM];

/*===================================================================
 *  API 函数
 *===================================================================*/

/**
 * @brief 初始化所有 4 路电机的 PI 控制器
 * @param kp  比例系数 (建议初值 1.0 ~ 3.0)
 * @param ki  积分系数 (建议初值 0.05 ~ 0.3)
 */
void VelPID_InitAll(float kp, float ki);

/**
 * @brief 设置目标速度
 * @param motor_id  MOTOR_A / MOTOR_B / MOTOR_C / MOTOR_D
 * @param speed_mms 目标线速度 (mm/s), 正=前进, 负=后退, 0=停止
 */
void VelPID_SetTarget(uint8_t motor_id, float speed_mms);

/**
 * @brief 设置 4 路电机目标速度 (快捷函数)
 */
void VelPID_SetAllTargets(float a, float b, float c, float d);

/**
 * @brief 在线修改 PI 参数
 * @param motor_id  电机ID, MOTOR_NUM=4 表示同时修改全部
 * @param kp  新的比例系数
 * @param ki  新的积分系数
 */
void VelPID_SetParams(uint8_t motor_id, float kp, float ki);

/**
 * @brief 获取当前速度
 */
float VelPID_GetSpeed(uint8_t motor_id);

/**
 * @brief 获取当前 PWM 输出
 */
int16_t VelPID_GetPWM(uint8_t motor_id);

/**
 * @brief 核心更新函数 — 在 20ms 定时器 ISR 中调用
 * @param motor_id          电机ID
 * @param encoder_count     编码器在本周期内的脉冲增量
 *                            (就是 get_encoder_count() 的返回值, 每个周期清零)
 *
 * 调用顺序:
 *   count = get_encoder_count('A');  // 读取并清零
 *   VelPID_Update(MOTOR_A, count);   // 传入本周期脉冲数
 *
 * 在ISR中典型写法:
 *   void TIMER_TICK_INST_IRQHandler(void) {
 *       VelPID_UpdateAll(get_encoder_count('A'),
 *                        get_encoder_count('B'),
 *                        get_encoder_count('C'),
 *                        get_encoder_count('D'));
 *   }
 */
void VelPID_Update(uint8_t motor_id, int32_t encoder_count);

/**
 * @brief 更新所有 4 路 (适用于 ISR 中一调用)
 */
void VelPID_UpdateAll(int32_t enc_a, int32_t enc_b,
                       int32_t enc_c, int32_t enc_d);

/**
 * @brief 紧急停止所有电机 (PWM清零, 内部状态重置)
 */
void VelPID_EmergencyStop(void);

#endif /* __VELOCITY_PID_H__ */
