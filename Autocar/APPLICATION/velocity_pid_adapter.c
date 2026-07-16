/**
 * @file    velocity_pid_adapter.c
 * @brief   PI速度环的硬件适配层
 *
 * 方向控制: 完全复制自 electrical_machinery.c 的 MOTOR_CONTROL 函数,
 *          方向逻辑已验证正确, 无需再改。
 * PWM输出: 已在 velocity_pid.c 的 MOTOR_PWM_SET 宏中直接操作硬件。
 */

#include "ti_msp_dl_config.h"
#include "velocity_pid.h"

/*===================================================================
 *  方向控制 — 严格按 electrical_machinery.c 的 MOTOR_CONTROL 逻辑
 *
 *  来源: G:\dong_car_code\Autocar\BSP\electrical_machinery.c
 *  A: 前进 → AIN2=1, AIN1=0 | 后退 → AIN2=0, AIN1=1 | 停 → AIN1=0,AIN2=0
 *  B: 前进 → BIN1=1, BIN2=0 | 后退 → BIN1=0, BIN2=1 | 停 → BIN1=0,BIN2=0
 *  C: 前进 → CIN1=1, CIN2=0 | 后退 → CIN1=0, CIN2=1 | 停 → CIN1=0,CIN2=0
 *  D: 前进 → DIN2=1, DIN1=0 | 后退 → DIN2=0, DIN1=1 | 停 → DIN1=0,DIN2=0
 *===================================================================*/
void motor_set_direction_single(uint8_t motor_id, int8_t dir)
{
    switch (motor_id) {

    case MOTOR_A:
        if (dir > 0) {                      /* 前进 */
            DL_GPIO_setPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        } else if (dir < 0) {               /* 后退 */
            DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
        } else {                            /* 停止 */
            DL_GPIO_clearPins(GPIO_MOTOR_AIN1_PORT, GPIO_MOTOR_AIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_AIN2_PORT, GPIO_MOTOR_AIN2_PIN);
        }
        break;

    case MOTOR_B:
        if (dir > 0) {                      /* 前进 */
            DL_GPIO_setPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        } else if (dir < 0) {               /* 后退 */
            DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        } else {                            /* 停止 */
            DL_GPIO_clearPins(GPIO_MOTOR_BIN1_PORT, GPIO_MOTOR_BIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_BIN2_PORT, GPIO_MOTOR_BIN2_PIN);
        }
        break;

    case MOTOR_C:
        if (dir > 0) {                      /* 前进 */
            DL_GPIO_setPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
        } else if (dir < 0) {               /* 后退 */
            DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
        } else {                            /* 停止 */
            DL_GPIO_clearPins(GPIO_MOTOR_CIN1_PORT, GPIO_MOTOR_CIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_CIN2_PORT, GPIO_MOTOR_CIN2_PIN);
        }
        break;

    case MOTOR_D:
        if (dir > 0) {                      /* 前进 */
            DL_GPIO_setPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        } else if (dir < 0) {               /* 后退 */
            DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
            DL_GPIO_setPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
        } else {                            /* 停止 */
            DL_GPIO_clearPins(GPIO_MOTOR_DIN1_PORT, GPIO_MOTOR_DIN1_PIN);
            DL_GPIO_clearPins(GPIO_MOTOR_DIN2_PORT, GPIO_MOTOR_DIN2_PIN);
        }
        break;
    }
}
