/**
 * @file    velocity_pid.c
 * @brief   增量式PI速度环实现 — 4路独立控制
 *
 * 核心公式 (增量式PI):
 *   e[n]  = target_speed - current_speed       // 本次误差
 *   Δu    = Kp * (e[n] - e[n-1])  +  Ki * e[n]  // 增量式PI
 *   u[n]  = u[n-1] + Δu                        // 累加到PWM输出
 *
 * 为什么用增量式:
 *   1. 不需要保存积分累加值, Ki*e[n] 本身就是 I 项的增量
 *   2. 目标速度突变时不会有积分冲击
 *   3. 抗饱和实现简单: PWM 到顶时直接丢弃 Δu 中的 I 分量
 *
 * 速度计算:
 *   instant_speed(mm/s) = (pulse / 13) × π × 65mm × (1000/20)Hz
 *                       = pulse × 0.07692 × 204.1 × 50
 *                       ≈ pulse × 785.4
 *   然后一阶低通滤波: speed = α * instant + (1-α) * speed
 *
 * 关于 13 线编码器:
 *   13 线意味着每圈仅 13 个脉冲, 分辨率较低。
 *   以 100rpm 为例: 20ms 内约 0.43 个脉冲。
 *   处理策略:
 *   - 速度低通滤波系数 α 设大一点 (0.5~0.7), 不过度依赖瞬时值
 *   - 低速时 (每周期 < 1 个脉冲) 用"脉冲到达事件"来估计速度
 *   - 本实现用累计脉冲+低通滤波, 简单且稳定
 */

#include "velocity_pid.h"
#include <string.h>  /* memset */

/*===================================================================
 *  内部常量
 *===================================================================*/

/* 速度滤波器系数: 越大越灵敏, 越小越平滑 (0~1) */
#define SPEED_FILTER_ALPHA   0.8f   /* 13线编码器脉冲稀疏, 提高信任瞬时值 */

/* 积分限幅: 单次 Ki*e[n] 的最大绝对值 */
/* 防止启动阶段积分累积过快导致大幅超调 */
#define INTEGRAL_LIMIT       80.0f

/* 死区补偿: 目标速度>此值时, 若PWM<DEADZONE则强制拉到DEADZONE */
/* 克服电机静态摩擦力, 避免有目标但电机不转 */
#define DEADZONE_SPEED_THRESH  15.0f   /* mm/s */
#define DEADZONE_PWM           200     /* PWM 最小启动值 (0~PWM_MAX) */

/* 堵转保护 */
#define STALL_PWM_THRESHOLD   999     /* 暂关堵转保护, 先调通PI再恢复 */
#define STALL_SPEED_THRESHOLD 8.0f    /* mm/s, 低于此速度认为堵转 */
#define STALL_COUNT_MAX       25      /* 连续 25 周期 (500ms) 判定堵转 */
#define COOLDOWN_COUNT        50      /* 冷却 50 周期 (1000ms) */

/* 速度物理常数: 每脉冲对应的 mm/s */
/*
 * 编码器 13线, A+B双相=26中断/圈, 减速比 1:28
 * 车轮每圈 = 26×28 = 728 中断
 * 1 中断位移 = π×65/728 = 0.2805mm
 * 采样频率 = 1000/20 = 50 Hz
 * 瞬时速度 = 脉冲数 × 0.2805mm × 50Hz = 脉冲数 × 14.03 mm/s
 */
#define PULSE_TO_MMS  (3.14159265f * WHEEL_DIAMETER_MM / (ENCODER_PPR * 2 * 28) * (1000.0f / PID_PERIOD_MS))

/*===================================================================
 *  全局电机状态数组
 *===================================================================*/
VelPID_t motors[MOTOR_NUM];

/*===================================================================
 *  外部依赖: PWM输出函数 (你现有的 hardware 函数)
 *
 *  你需要确保以下函数在你的项目中可用:
 *    - DL_TimerG_setCaptureCompareValue(PWMx_INST, compare, CC_IDX)
 *
 *  PWM 输出: compare = duty (直接赋值, 与你现有 MOTOR_CONTROL 一致)
 *  方向由 IN1/IN2 GPIO 控制, PWM 只管调速。
 *===================================================================*/

/*
 * PWM 通道映射 (来自你现有的 MOTOR_CONTROL 函数):
 *   A 电机: PWMA_INST, GPIO_PWMA_C0_IDX  (TIMG8, CCP0)
 *   B 电机: PWMB_INST, GPIO_PWMB_C1_IDX  (TIMG7, CCP1)
 *   C 电机: PWMC_INST, GPIO_PWMC_C1_IDX  (TIMG12, CCP1)
 *   D 电机: PWMD_INST, GPIO_PWMD_C1_IDX  (TIMG6, CCP1)
 */
#define MOTOR_PWM_SET(motor_id, duty)                           \
    do {                                                        \
        uint32_t _cmp = (uint32_t)(duty);                       \
        if (_cmp > PWM_MAX) _cmp = PWM_MAX;                     \
        switch (motor_id) {                                     \
        case MOTOR_A:                                           \
            DL_TimerG_setCaptureCompareValue(PWMA_INST, _cmp,   \
                GPIO_PWMA_C0_IDX);                              \
            break;                                              \
        case MOTOR_B:                                           \
            DL_TimerG_setCaptureCompareValue(PWMB_INST, _cmp,   \
                GPIO_PWMB_C1_IDX);                              \
            break;                                              \
        case MOTOR_C:                                           \
            DL_TimerG_setCaptureCompareValue(PWMC_INST, _cmp,   \
                GPIO_PWMC_C1_IDX);                              \
            break;                                              \
        case MOTOR_D:                                           \
            DL_TimerG_setCaptureCompareValue(PWMD_INST, _cmp,   \
                GPIO_PWMD_C1_IDX);                              \
            break;                                              \
        }                                                       \
    } while (0)

/* 方向控制 — 由 velocity_pid_adapter.c 提供, 完全复制你现有逻辑 */
extern void motor_set_direction_single(uint8_t motor_id, int8_t dir);

/*===================================================================
 *  内部辅助宏
 *===================================================================*/
#define ABS(x)    ((x) < 0 ? -(x) : (x))
#define CLAMP(v, lo, hi)  do { if ((v) < (lo)) (v) = (lo); if ((v) > (hi)) (v) = (hi); } while (0)

/*===================================================================
 *  VelPID_InitAll — 初始化全部 4 路
 *===================================================================*/
void VelPID_InitAll(float kp, float ki)
{
    memset(motors, 0, sizeof(motors));
    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        motors[i].kp = kp;
        motors[i].ki = ki;
    }
}

/*===================================================================
 *  VelPID_SetTarget — 设定单路目标速度
 *===================================================================*/
void VelPID_SetTarget(uint8_t motor_id, float speed_mms)
{
    if (motor_id >= MOTOR_NUM) return;
    motors[motor_id].target_speed = speed_mms;
}

/*===================================================================
 *  VelPID_SetAllTargets — 快捷设定4路
 *===================================================================*/
void VelPID_SetAllTargets(float a, float b, float c, float d)
{
    motors[MOTOR_A].target_speed = a;
    motors[MOTOR_B].target_speed = b;
    motors[MOTOR_C].target_speed = c;
    motors[MOTOR_D].target_speed = d;
}

/*===================================================================
 *  VelPID_SetParams — 在线调整 PI 参数
 *===================================================================*/
void VelPID_SetParams(uint8_t motor_id, float kp, float ki)
{
    if (motor_id >= MOTOR_NUM) {
        /* motor_id == MOTOR_NUM 表示全部 */
        for (uint8_t i = 0; i < MOTOR_NUM; i++) {
            motors[i].kp = kp;
            motors[i].ki = ki;
        }
        return;
    }
    motors[motor_id].kp = kp;
    motors[motor_id].ki = ki;
}

/*===================================================================
 *  VelPID_GetSpeed / VelPID_GetPWM
 *===================================================================*/
float VelPID_GetSpeed(uint8_t motor_id)
{
    if (motor_id >= MOTOR_NUM) return 0;
    return motors[motor_id].speed;
}

int16_t VelPID_GetPWM(uint8_t motor_id)
{
    if (motor_id >= MOTOR_NUM) return 0;
    return motors[motor_id].pwm;
}

/*===================================================================
 *  VelPID_Update — 核心: 单路增量式PI更新 (在20ms ISR中调用)
 *
 *  参数 encoder_pulses: 本周期(20ms)内的编码器脉冲增量
 *        = get_encoder_count() 的返回值
 *===================================================================*/
void VelPID_Update(uint8_t motor_id, int32_t encoder_pulses)
{
    if (motor_id >= MOTOR_NUM) return;

    VelPID_t *m = &motors[motor_id];

    /*--- 1. 瞬时速度计算 (mm/s), 始终为正 (编码器只给脉冲数) ---*/
    float instant_speed = (float)encoder_pulses * PULSE_TO_MMS;

    /*--- 2. 一阶低通滤波 ---*/
    m->speed = SPEED_FILTER_ALPHA * instant_speed
             + (1.0f - SPEED_FILTER_ALPHA) * m->speed;

    /*--- 3. 增量式 PI 计算 ---*/
    /*
     * 速度环用绝对值: target_speed 的符号只控制方向 (在步骤9中处理),
     *                PI 只管速度大小。
     *  例: target=+100, speed=80 → error=20 → 加速前进
     *      target=-100, speed=80 → error=20 → 加速后退
     *      target=  0, speed=80 → error=-80 → 减速到零
     */
    float target_mag = (m->target_speed >= 0) ? m->target_speed : -m->target_speed;
    float error = target_mag - m->speed;

    /* Ki*error 项 (积分增量) */
    float ki_term = m->ki * error;

    /* 积分限幅: 防止启动和大偏差时积分增量过大 */
    CLAMP(ki_term, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);

    /* 增量式公式: Δu = Kp*(e[n] - e[n-1]) + Ki*e[n] */
    float delta = m->kp * (error - m->last_error) + ki_term;

    /*--- 4. 积分抗饱和 ---*/
    /* PWM 已到上限, delta>0 (还要加) → 去掉积分项, 只保留 P */
    if ((m->pwm >= PWM_MAX && delta > 0) ||
        (m->pwm <= 0      && delta < 0)) {
        delta = m->kp * (error - m->last_error);
    }

    /*--- 5. 累加输出 ---*/
    m->pwm += (int16_t)delta;

    /* PWM 限幅 */
    CLAMP(m->pwm, 0, PWM_MAX);

    /*--- 6. 死区补偿 ---*/
    /* 目标速度较高但 PWM 太低 → 电机克服不了静摩擦, 强制给一个最小 PWM */
    if (m->target_speed > DEADZONE_SPEED_THRESH && m->pwm > 0 && m->pwm < DEADZONE_PWM) {
        m->pwm = DEADZONE_PWM;
    }
    if (m->target_speed < -DEADZONE_SPEED_THRESH && m->pwm > 0 && m->pwm < DEADZONE_PWM) {
        m->pwm = DEADZONE_PWM;
    }

    /*--- 7. 堵转保护 ---*/
    if (m->cooldown > 0) {
        /* 冷却中: 保持停机, 不检测堵转 */
        m->cooldown--;
        m->pwm = 0;
    } else {
        /* 条件: PWM 全力输出 且 速度几乎为零 */
        if (m->pwm >= STALL_PWM_THRESHOLD &&
            ABS(m->speed) < STALL_SPEED_THRESHOLD) {
            m->stall_cnt++;
            if (m->stall_cnt >= STALL_COUNT_MAX) {
                /* 确认堵转 → 停机 + 进入冷却 */
                m->pwm = 0;
                m->last_error = 0;
                m->stall_cnt = 0;
                m->cooldown = COOLDOWN_COUNT;
            }
        } else {
            /* 正常运转 → 清零堵转计数 */
            m->stall_cnt = 0;
        }
    }

    /*--- 8. 输出 PWM 到硬件 ---*/
    MOTOR_PWM_SET(motor_id, m->pwm);

    /*--- 9. 方向控制 ---*/
    if (m->target_speed > 0) {
        motor_set_direction_single(motor_id, 1);   /* 前进 */
    } else if (m->target_speed < 0) {
        motor_set_direction_single(motor_id, -1);  /* 后退 */
    } else {
        motor_set_direction_single(motor_id, 0);   /* 停止/刹车 */
    }

    /*--- 10. 保存本次误差供下次增量计算 ---*/
    m->last_error = error;
}

/*===================================================================
 *  VelPID_UpdateAll — ISR 中一次更新 4 路
 *===================================================================*/
void VelPID_UpdateAll(int32_t enc_a, int32_t enc_b,
                       int32_t enc_c, int32_t enc_d)
{
    VelPID_Update(MOTOR_A, enc_a);
    VelPID_Update(MOTOR_B, enc_b);
    VelPID_Update(MOTOR_C, enc_c);
    VelPID_Update(MOTOR_D, enc_d);
}

/*===================================================================
 *  VelPID_EmergencyStop — 急停
 *===================================================================*/
void VelPID_EmergencyStop(void)
{
    for (uint8_t i = 0; i < MOTOR_NUM; i++) {
        motors[i].pwm = 0;
        motors[i].target_speed = 0;
        motors[i].last_error = 0;
        motors[i].stall_cnt = 0;
        MOTOR_PWM_SET(i, 0);
        motor_set_direction_single(i, 0);
    }
}
