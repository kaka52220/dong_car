#ifndef USER_MPU6050_MPU6050_H_
#define USER_MPU6050_MPU6050_H_

#include <stdbool.h>
#include <stdint.h>

#include <ti/driverlib/driverlib.h>

#define MPU6050_DEFAULT_ADDRESS (0x68U)

/* 200 Hz 是姿态响应、软浮点负载、串口带宽和噪声之间的平衡值。 */
#define MPU6050_SAMPLE_RATE_HZ (200U)
#define MPU6050_SAMPLE_PERIOD_MS (1000U / MPU6050_SAMPLE_RATE_HZ)

/*
 * 默认安装方向沿用本工程旧驱动：传感器 X/Y 轴反向，Z 轴同向。
 * 若模块安装方向不同，只需修改这三个宏；取值应为 1.0f 或 -1.0f。
 */
#ifndef MPU6050_MOUNT_X_SIGN
#define MPU6050_MOUNT_X_SIGN (-1.0f)
#endif
#ifndef MPU6050_MOUNT_Y_SIGN
#define MPU6050_MOUNT_Y_SIGN (-1.0f)
#endif
#ifndef MPU6050_MOUNT_Z_SIGN
#define MPU6050_MOUNT_Z_SIGN (1.0f)
#endif

typedef enum
{
    MPU6050_STATUS_OK = 0,
    MPU6050_STATUS_TIMEBASE_ERROR = -1,
    MPU6050_STATUS_I2C_ERROR = -2,
    MPU6050_STATUS_NOT_FOUND = -3,
    MPU6050_STATUS_CALIBRATION_MOVING = -4,
    MPU6050_STATUS_I2C_BUS_STUCK = -5,
    MPU6050_STATUS_I2C_NO_ACK = -6,
    MPU6050_STATUS_INVALID_CONFIG = -7
} Mpu6050Status;

typedef struct
{
    I2C_Regs *i2c;
    uint8_t address;
    GPIO_Regs *sda_port;
    uint32_t sda_pin;
    uint32_t sda_iomux;
    uint32_t sda_function;
    GPIO_Regs *scl_port;
    uint32_t scl_pin;
    uint32_t scl_iomux;
    uint32_t scl_function;
    void (*reinitialize_i2c)(void);
} Mpu6050BusConfig;

typedef struct
{
    float roll_deg;
    float pitch_deg;
    float yaw_deg;
    float temperature_c;
    float gyro_dps[3];
    float accel_g[3];
    float gyro_bias_dps[3];
    uint32_t sample_count;
    uint32_t communication_errors;
    bool stationary;
} Mpu6050Data;

/* 在 SYSCFG_DL_init() 之后调用。配置会复制到驱动内部。 */
Mpu6050Status mpu6050_init(const Mpu6050BusConfig *config);

/* 按 MPU6050_SAMPLE_RATE_HZ 调用，成功取得并融合样本时返回 true。 */
bool mpu6050_update(void);

const Mpu6050Data *mpu6050_get_data(void);

/* 直接读取最近一次融合结果，适合运动控制中的快速轮询。 */
float mpu6050_get_roll(void);
float mpu6050_get_pitch(void);
float mpu6050_get_yaw(void);

/* 本裸机驱动使用 1 ms SysTick，同时为 I2C 超时和姿态积分提供时基。 */
uint32_t mpu6050_millis(void);
void mpu6050_delay_ms(uint32_t milliseconds);

#endif
