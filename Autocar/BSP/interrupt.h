/*
 *存放中断函数
 */
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include <stdbool.h>
#include "APPLICATION/velocity_pid.h"
typedef enum {
    FORWARD,  // 正向
    REVERSAL  // 反向
} ENCODER_DIR;

typedef struct {
    volatile long long temp_count;  //保存实时计数值
    int count;         				//根据定时器时间更新的计数值
    ENCODER_DIR dir;            	//旋转方向
} ENCODER_RES;

extern int nextway;//-1向上移动,1向下移动,2进入下一目录/启动应用,3返回上一级

/* IMU 采样标志：SysTick ISR 每 5ms 置 true，主循环检测后调 mpu6050_update()。
 * 不在 ISR 里做阻塞 I2C，避免 SysTick 卡死导致 tick_ms/dt 不准。
 * 曾试过计数器追赶方案，但连续调用时 dt 变成 I2C 耗时(2ms)被钳为 10ms
 * → 过度积分 → 转90°变110°。故改回 bool，dt 用真实间隔，不过冲。 */
extern volatile bool g_imu_update_flag;

int get_encoder_count(char encoder_name);
ENCODER_DIR get_encoder_dir(char encoder_name);
volatile long long get_encoder_temp_count(char encoder_name);

#endif  /* #ifndef _INTERRUPT_H_ */