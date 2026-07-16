# MPU6050 服务调用

## 读取姿态

```c
float roll  = mpu6050_get_roll();
float pitch = mpu6050_get_pitch();
float yaw   = mpu6050_get_yaw();
```

这些接口返回最近一次 `mpu6050_service_process()` 更新后的角度，单位为度。
Yaw 为连续累计角度，不会在 ±180° 处跳变。

## 右转 90° 示例

下面的电机函数是占位名称，需要替换为项目中的实际电机接口：

```c
#include <math.h>

#include "mpu6050/mpu6050.h"
#include "mpu6050/mpu6050_service.h"

void car_turn_right_90_degrees(void)
{
    const float start_yaw = mpu6050_get_yaw();

    motor_turn_right();
    while (fabsf(mpu6050_get_yaw() - start_yaw) < 90.0f)
    {
        /* 必须持续调用，才能采样并更新 yaw。 */
        mpu6050_service_process();
    }
    motor_stop();
}
```

实际车辆存在惯性时，可将停止阈值提前到 85°～88°，再根据车速和制动效果调整。
