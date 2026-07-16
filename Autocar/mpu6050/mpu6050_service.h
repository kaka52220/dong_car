#ifndef USER_MPU6050_MPU6050_SERVICE_H_
#define USER_MPU6050_MPU6050_SERVICE_H_

#include <stdbool.h>
#include <stdint.h>

#include "mpu6050.h"

/*
 * 高层裸机服务封装：
 * 1. 在 SYSCFG_DL_init() 之后传入 SysConfig 生成的 SDA/SCL pin；
 * 2. 在主循环中持续调用 mpu6050_service_process()。
 * 完整的 I2C/IOMUX 配置由服务层内部管理，避免 main 暴露底层细节。
 */
Mpu6050Status mpu6050_service_init(uint32_t sda_pin, uint32_t scl_pin,
    bool enable_justfloat);
void mpu6050_service_process(void);

#endif
