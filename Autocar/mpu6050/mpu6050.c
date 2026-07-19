#include "mpu6050.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#include "clock.h"
#include "ti_msp_dl_config.h"

#define MPU6050_REG_SMPLRT_DIV          (0x19U)
#define MPU6050_REG_CONFIG              (0x1AU)
#define MPU6050_REG_GYRO_CONFIG         (0x1BU)
#define MPU6050_REG_ACCEL_CONFIG        (0x1CU)
#define MPU6050_REG_ACCEL_CONFIG2       (0x1DU)
#define MPU6050_REG_INT_ENABLE          (0x38U)
#define MPU6050_REG_ACCEL_XOUT_H        (0x3BU)
#define MPU6050_REG_USER_CTRL           (0x6AU)
#define MPU6050_REG_PWR_MGMT_1          (0x6BU)
#define MPU6050_REG_PWR_MGMT_2          (0x6CU)
#define MPU6050_REG_WHO_AM_I            (0x75U)

#define MPU6050_WHO_AM_I_VALUE          (0x68U)
#define MPU6050_I2C_TIMEOUT_MS          (15U)
#define MPU6050_CALIBRATION_SAMPLES     (MPU6050_SAMPLE_RATE_HZ * 3U)
#define MPU6050_CALIBRATION_TIMEOUT_MS  (10000U)
#define MPU6050_GYRO_LSB_PER_DPS        (65.5f)
#define MPU6050_ACCEL_LSB_PER_G         (16384.0f)
#define MPU6050_RAD_TO_DEG              (57.2957795131f)
#define MPU6050_DEG_TO_RAD              (0.0174532925199f)
#define MPU6050_MAHONY_KP               (1.8f)
#define MPU6050_MAHONY_KI               (0.035f)
#define MPU6050_MAHONY_KP_Z             (0.05f)  /* Z轴无绝对参考，极低修正 */
#define MPU6050_STATIONARY_GYRO_DPS     (0.80f)
#define MPU6050_STATIONARY_ACCEL_G      (0.060f)
#define MPU6050_MOTION_GYRO_DPS         (1.50f)
#define MPU6050_MOTION_ACCEL_G          (0.120f)
#define MPU6050_STATIONARY_SCORE_MAX    (MPU6050_SAMPLE_RATE_HZ / 2U)
#define MPU6050_STATIONARY_SCORE_ENTER  (MPU6050_SAMPLE_RATE_HZ / 5U)
#define MPU6050_STATIONARY_SCORE_EXIT   (MPU6050_SAMPLE_RATE_HZ / 20U)
#define MPU6050_BIAS_TRACK_ALPHA        (1.0f / (5.0f * MPU6050_SAMPLE_RATE_HZ))
#define MPU6050_YAW_RATE_DEADBAND_DPS   (0.50f)  /* 原 0.1 太小，手持微颤被误积 */
#define MPU6050_TRANSFER_ERROR_MASK     \
    (DL_I2C_INTERRUPT_CONTROLLER_NACK | \
     DL_I2C_INTERRUPT_CONTROLLER_ARBITRATION_LOST)

typedef struct
{
    float q0;
    float q1;
    float q2;
    float q3;
    float integral_x;
    float integral_y;
    uint32_t last_update_ms;
    uint16_t stationary_score;
    uint8_t consecutive_errors;
} Mpu6050Filter;

static Mpu6050Data g_data;
static Mpu6050Filter g_filter;
static Mpu6050BusConfig g_bus;
static bool g_i2c_nack_seen;

uint32_t mpu6050_millis(void)
{
    return tick_ms;
}

void mpu6050_delay_ms(uint32_t milliseconds)
{
    delay_ms(milliseconds);
}

static bool time_expired(uint32_t start, uint32_t timeout_ms)
{
    return (uint32_t)(tick_ms - start) >= timeout_ms;
}

static bool i2c_wait_idle(void)
{
    const uint32_t start = tick_ms;

    while ((DL_I2C_getControllerStatus(g_bus.i2c) &
            DL_I2C_CONTROLLER_STATUS_IDLE) == 0U)
    {
        if (time_expired(start, MPU6050_I2C_TIMEOUT_MS))
        {
            return false;
        }
    }
    return true;
}

static void i2c_bus_recover(void)
{
    uint32_t pulse;

    DL_I2C_reset(g_bus.i2c);

    DL_GPIO_initDigitalInputFeatures(g_bus.sda_iomux,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalOutput(g_bus.scl_iomux);
    DL_GPIO_setPins(g_bus.scl_port, g_bus.scl_pin);
    DL_GPIO_enableOutput(g_bus.scl_port, g_bus.scl_pin);

    for (pulse = 0U; pulse < 9U; pulse++)
    {
        if (DL_GPIO_readPins(g_bus.sda_port, g_bus.sda_pin) != 0U)
        {
            break;
        }
        DL_GPIO_clearPins(g_bus.scl_port, g_bus.scl_pin);
        mpu6050_delay_ms(1U);
        DL_GPIO_setPins(g_bus.scl_port, g_bus.scl_pin);
        mpu6050_delay_ms(1U);
    }

    DL_GPIO_initPeripheralInputFunctionFeatures(g_bus.sda_iomux,
        g_bus.sda_function, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(g_bus.scl_iomux,
        g_bus.scl_function, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(g_bus.sda_iomux);
    DL_GPIO_enableHiZ(g_bus.scl_iomux);
    DL_I2C_enablePower(g_bus.i2c);
    g_bus.reinitialize_i2c();
}

static bool i2c_error_interrupt_pending(void)
{
    const uint32_t status = DL_I2C_getRawInterruptStatus(
        g_bus.i2c, MPU6050_TRANSFER_ERROR_MASK);

    if ((status & DL_I2C_INTERRUPT_CONTROLLER_NACK) != 0U)
    {
        g_i2c_nack_seen = true;
    }
    return status != 0U;
}

static bool i2c_write_once(uint8_t reg, uint8_t value)
{
    const uint32_t start = tick_ms;
    bool value_queued = false;

    if (!i2c_wait_idle())
    {
        return false;
    }

    DL_I2C_flushControllerTXFIFO(g_bus.i2c);
    DL_I2C_flushControllerRXFIFO(g_bus.i2c);
    g_bus.i2c->MASTER.MCTR = 0U;
    DL_I2C_clearInterruptStatus(g_bus.i2c,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE | MPU6050_TRANSFER_ERROR_MASK);
    DL_I2C_transmitControllerData(g_bus.i2c, reg);
    DL_I2C_startControllerTransfer(g_bus.i2c, g_bus.address,
        DL_I2C_CONTROLLER_DIRECTION_TX, 2U);

    while (DL_I2C_getRawInterruptStatus(g_bus.i2c,
               DL_I2C_INTERRUPT_CONTROLLER_TX_DONE) == 0U)
    {
        if (!value_queued &&
            !DL_I2C_isControllerTXFIFOFull(g_bus.i2c))
        {
            DL_I2C_transmitControllerData(g_bus.i2c, value);
            value_queued = true;
        }
        if (i2c_error_interrupt_pending() ||
            time_expired(start, MPU6050_I2C_TIMEOUT_MS))
        {
            g_bus.i2c->MASTER.MCTR = 0U;
            return false;
        }
    }
    g_bus.i2c->MASTER.MCTR = 0U;
    return value_queued && !i2c_error_interrupt_pending();
}

static bool i2c_read_once(uint8_t reg, uint8_t *data, uint8_t length)
{
    uint8_t received = 0U;
    const uint32_t start = tick_ms;

    if ((data == NULL) || (length == 0U) || !i2c_wait_idle())
    {
        return false;
    }

    DL_I2C_flushControllerTXFIFO(g_bus.i2c);
    DL_I2C_flushControllerRXFIFO(g_bus.i2c);
    g_bus.i2c->MASTER.MCTR = I2C_MCTR_RD_ON_TXEMPTY_ENABLE;
    DL_I2C_clearInterruptStatus(g_bus.i2c,
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE | MPU6050_TRANSFER_ERROR_MASK);
    DL_I2C_transmitControllerData(g_bus.i2c, reg);
    DL_I2C_startControllerTransfer(g_bus.i2c, g_bus.address,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    while (DL_I2C_getRawInterruptStatus(g_bus.i2c,
               DL_I2C_INTERRUPT_CONTROLLER_RX_DONE) == 0U)
    {
        while (!DL_I2C_isControllerRXFIFOEmpty(g_bus.i2c) &&
               (received < length))
        {
            data[received++] = DL_I2C_receiveControllerData(g_bus.i2c);
        }

        if (i2c_error_interrupt_pending() ||
            time_expired(start, MPU6050_I2C_TIMEOUT_MS))
        {
            g_bus.i2c->MASTER.MCTR = 0U;
            DL_I2C_flushControllerTXFIFO(g_bus.i2c);
            return false;
        }
    }

    while (!DL_I2C_isControllerRXFIFOEmpty(g_bus.i2c) &&
           (received < length))
    {
        data[received++] = DL_I2C_receiveControllerData(g_bus.i2c);
    }

    g_bus.i2c->MASTER.MCTR = 0U;
    DL_I2C_flushControllerTXFIFO(g_bus.i2c);
    return (received == length) && !i2c_error_interrupt_pending();
}

static bool i2c_write(uint8_t reg, uint8_t value)
{
    if (i2c_write_once(reg, value))
    {
        return true;
    }
    i2c_bus_recover();
    return i2c_write_once(reg, value);
}

static bool i2c_read(uint8_t reg, uint8_t *data, uint8_t length)
{
    if (i2c_read_once(reg, data, length))
    {
        return true;
    }
    i2c_bus_recover();
    return i2c_read_once(reg, data, length);
}

static int16_t be16(const uint8_t *bytes)
{
    return (int16_t)(((uint16_t)bytes[0] << 8U) | bytes[1]);
}

static bool read_raw(float accel_g[3], float gyro_dps[3], float *temperature_c)
{
    uint8_t raw[14];

    if (!i2c_read(MPU6050_REG_ACCEL_XOUT_H, raw, sizeof(raw)))
    {
        return false;
    }

    accel_g[0] = MPU6050_MOUNT_X_SIGN * (float)be16(&raw[0]) /
                 MPU6050_ACCEL_LSB_PER_G;
    accel_g[1] = MPU6050_MOUNT_Y_SIGN * (float)be16(&raw[2]) /
                 MPU6050_ACCEL_LSB_PER_G;
    accel_g[2] = MPU6050_MOUNT_Z_SIGN * (float)be16(&raw[4]) /
                 MPU6050_ACCEL_LSB_PER_G;
    *temperature_c = (float)be16(&raw[6]) / 340.0f + 36.53f;
    gyro_dps[0] = MPU6050_MOUNT_X_SIGN * (float)be16(&raw[8]) /
                  MPU6050_GYRO_LSB_PER_DPS;
    gyro_dps[1] = MPU6050_MOUNT_Y_SIGN * (float)be16(&raw[10]) /
                  MPU6050_GYRO_LSB_PER_DPS;
    gyro_dps[2] = MPU6050_MOUNT_Z_SIGN * (float)be16(&raw[12]) /
                  MPU6050_GYRO_LSB_PER_DPS;
    return true;
}

static bool configure_sensor(bool reset_device)
{
    uint8_t who_am_i;

    if (reset_device)
    {
        if (!i2c_write(MPU6050_REG_PWR_MGMT_1, 0x80U))
        {
            return false;
        }
        mpu6050_delay_ms(100U);
    }

    /* PLL X gyro；1 kHz/(4+1)=200 Hz；DLPF 3 约为 42~44 Hz。 */
    if (!i2c_write(MPU6050_REG_PWR_MGMT_1, 0x01U) ||
        !i2c_write(MPU6050_REG_PWR_MGMT_2, 0x00U) ||
        !i2c_write(MPU6050_REG_USER_CTRL, 0x00U) ||
        !i2c_write(MPU6050_REG_SMPLRT_DIV,
            (uint8_t)((1000U / MPU6050_SAMPLE_RATE_HZ) - 1U)) ||
        !i2c_write(MPU6050_REG_CONFIG, 0x03U) ||
        !i2c_write(MPU6050_REG_GYRO_CONFIG, 0x08U) ||
        !i2c_write(MPU6050_REG_ACCEL_CONFIG, 0x00U) ||
        !i2c_write(MPU6050_REG_ACCEL_CONFIG2, 0x03U) ||
        !i2c_write(MPU6050_REG_INT_ENABLE, 0x01U))
    {
        return false;
    }
    mpu6050_delay_ms(30U);

    if (!i2c_read(MPU6050_REG_WHO_AM_I, &who_am_i, 1U))
    {
        return false;
    }
    return (who_am_i & 0x7EU) == MPU6050_WHO_AM_I_VALUE;
}

static Mpu6050Status calibrate_gyro(float average_accel[3])
{
    float accel[3];
    float gyro[3];
    float temperature;
    float gyro_sum[3] = {0.0f, 0.0f, 0.0f};
    float accel_sum[3] = {0.0f, 0.0f, 0.0f};
    uint32_t valid_samples = 0U;
    const uint32_t start = tick_ms;

    while (valid_samples < MPU6050_CALIBRATION_SAMPLES)
    {
        float accel_norm_sq;
        float gyro_norm_sq;

        if (!read_raw(accel, gyro, &temperature))
        {
            return MPU6050_STATUS_I2C_ERROR;
        }

        accel_norm_sq = accel[0] * accel[0] + accel[1] * accel[1] +
                        accel[2] * accel[2];
        gyro_norm_sq = gyro[0] * gyro[0] + gyro[1] * gyro[1] +
                       gyro[2] * gyro[2];

        if ((accel_norm_sq < (0.85f * 0.85f)) ||
            (accel_norm_sq > (1.15f * 1.15f)) ||
            (gyro_norm_sq > (8.0f * 8.0f)))
        {
            valid_samples = 0U;
            memset(gyro_sum, 0, sizeof(gyro_sum));
            memset(accel_sum, 0, sizeof(accel_sum));
        }
        else
        {
            uint32_t axis;
            for (axis = 0U; axis < 3U; axis++)
            {
                gyro_sum[axis] += gyro[axis];
                accel_sum[axis] += accel[axis];
            }
            valid_samples++;
        }

        if (time_expired(start, MPU6050_CALIBRATION_TIMEOUT_MS))
        {
            return MPU6050_STATUS_CALIBRATION_MOVING;
        }
        mpu6050_delay_ms(MPU6050_SAMPLE_PERIOD_MS);
    }

    for (uint32_t axis = 0U; axis < 3U; axis++)
    {
        g_data.gyro_bias_dps[axis] =
            gyro_sum[axis] / (float)MPU6050_CALIBRATION_SAMPLES;
        average_accel[axis] =
            accel_sum[axis] / (float)MPU6050_CALIBRATION_SAMPLES;
    }
    return MPU6050_STATUS_OK;
}

static void set_initial_attitude(const float accel[3])
{
    const float roll = atan2f(accel[1], accel[2]);
    const float pitch = atan2f(-accel[0],
        sqrtf(accel[1] * accel[1] + accel[2] * accel[2]));
    const float cr = cosf(roll * 0.5f);
    const float sr = sinf(roll * 0.5f);
    const float cp = cosf(pitch * 0.5f);
    const float sp = sinf(pitch * 0.5f);

    g_filter.q0 = cr * cp;
    g_filter.q1 = sr * cp;
    g_filter.q2 = cr * sp;
    g_filter.q3 = -sr * sp;
}

static void fusion_update(float gx, float gy, float gz,
    float ax, float ay, float az, float dt)
{
    float q0 = g_filter.q0;
    float q1 = g_filter.q1;
    float q2 = g_filter.q2;
    float q3 = g_filter.q3;
    const float accel_norm_sq = ax * ax + ay * ay + az * az;

    gx *= MPU6050_DEG_TO_RAD;
    gy *= MPU6050_DEG_TO_RAD;
    gz *= MPU6050_DEG_TO_RAD;

    if (accel_norm_sq > 0.01f)
    {
        const float reciprocal_norm = 1.0f / sqrtf(accel_norm_sq);
        float error_x;
        float error_y;
        float error_z;
        float estimated_x;
        float estimated_y;
        float estimated_z;

        ax *= reciprocal_norm;
        ay *= reciprocal_norm;
        az *= reciprocal_norm;
        estimated_x = 2.0f * (q1 * q3 - q0 * q2);
        estimated_y = 2.0f * (q0 * q1 + q2 * q3);
        estimated_z = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
        error_x = ay * estimated_z - az * estimated_y;
        error_y = az * estimated_x - ax * estimated_z;
        error_z = ax * estimated_y - ay * estimated_x;

        g_filter.integral_x += MPU6050_MAHONY_KI * error_x * dt;
        g_filter.integral_y += MPU6050_MAHONY_KI * error_y * dt;
        /* 重力无法观测绝对航向：error_z 无物理意义，KP_Z 接近 0。
         * 若 KP_Z=1.8，纯 pitch/roll 时四元数滞后会导致 yaw 被错误修正 */
        gx += MPU6050_MAHONY_KP * error_x + g_filter.integral_x;
        gy += MPU6050_MAHONY_KP * error_y + g_filter.integral_y;
        gz += MPU6050_MAHONY_KP_Z * error_z;
    }

    {
        const float half_dt = 0.5f * dt;
        const float old_q0 = q0;
        const float old_q1 = q1;
        const float old_q2 = q2;

        q0 += (-old_q1 * gx - old_q2 * gy - q3 * gz) * half_dt;
        q1 += ( old_q0 * gx + old_q2 * gz - q3 * gy) * half_dt;
        q2 += ( old_q0 * gy - old_q1 * gz + q3 * gx) * half_dt;
        q3 += ( old_q0 * gz + old_q1 * gy - old_q2 * gx) * half_dt;
    }

    {
        const float reciprocal_norm = 1.0f /
            sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
        g_filter.q0 = q0 * reciprocal_norm;
        g_filter.q1 = q1 * reciprocal_norm;
        g_filter.q2 = q2 * reciprocal_norm;
        g_filter.q3 = q3 * reciprocal_norm;
    }
}

static void update_euler_and_yaw(float dt)
{
    const float q0 = g_filter.q0;
    const float q1 = g_filter.q1;
    const float q2 = g_filter.q2;
    const float q3 = g_filter.q3;
    const float sin_pitch = 2.0f * (q0 * q2 - q3 * q1);
    const float accel_norm = sqrtf(g_data.accel_g[0] * g_data.accel_g[0] +
        g_data.accel_g[1] * g_data.accel_g[1] +
        g_data.accel_g[2] * g_data.accel_g[2]);
    float vertical_rate_dps = 0.0f;

    g_data.roll_deg = atan2f(2.0f * (q0 * q1 + q2 * q3),
        1.0f - 2.0f * (q1 * q1 + q2 * q2)) * MPU6050_RAD_TO_DEG;
    g_data.pitch_deg = asinf(fmaxf(-1.0f, fminf(1.0f, sin_pitch))) *
                       MPU6050_RAD_TO_DEG;

    /*
     * MPU6050 没有磁力计，yaw 只能由陀螺积分得到。将机体系角速度
     * 投影到重力方向，可避免俯仰/横滚修正误差被误算成 yaw。
     */
    if (accel_norm > 0.5f)
    {
        vertical_rate_dps =
            (g_data.gyro_dps[0] * g_data.accel_g[0] +
             g_data.gyro_dps[1] * g_data.accel_g[1] +
             g_data.gyro_dps[2] * g_data.accel_g[2]) / accel_norm;
    }

    if (g_data.stationary ||
        (fabsf(vertical_rate_dps) < MPU6050_YAW_RATE_DEADBAND_DPS))
    {
        vertical_rate_dps = 0.0f;
    }
    g_data.yaw_deg += vertical_rate_dps * dt;
}

Mpu6050Status mpu6050_init(const Mpu6050BusConfig *config)
{
    float average_accel[3];
    uint8_t who_am_i = 0U;
    Mpu6050Status calibration_status;

    if ((config == NULL) || (config->i2c == NULL) ||
        (config->sda_port == NULL) || (config->scl_port == NULL) ||
        (config->sda_pin == 0U) || (config->scl_pin == 0U) ||
        (config->sda_iomux == 0U) || (config->scl_iomux == 0U) ||
        ((config->address != 0x68U) && (config->address != 0x69U)) ||
        (config->reinitialize_i2c == NULL))
    {
        return MPU6050_STATUS_INVALID_CONFIG;
    }

    g_bus = *config;
    memset(&g_data, 0, sizeof(g_data));
    memset(&g_filter, 0, sizeof(g_filter));
    g_filter.q0 = 1.0f;
    g_i2c_nack_seen = false;
    mpu6050_delay_ms(20U);
    /* 正常启动沿用 SysConfig 配置，仅在线路确实被拉低时解锁总线。 */
    if (DL_I2C_getSDAStatus(g_bus.i2c) == DL_I2C_CONTROLLER_SDA_LOW)
    {
        i2c_bus_recover();
    }
    if (!i2c_read(MPU6050_REG_WHO_AM_I, &who_am_i, 1U))
    {
        if ((DL_I2C_getSDAStatus(g_bus.i2c) ==
                DL_I2C_CONTROLLER_SDA_LOW) ||
            (DL_I2C_getSCLStatus(g_bus.i2c) ==
                DL_I2C_CONTROLLER_SCL_LOW))
        {
            return MPU6050_STATUS_I2C_BUS_STUCK;
        }
        return g_i2c_nack_seen ? MPU6050_STATUS_I2C_NO_ACK :
                                 MPU6050_STATUS_I2C_ERROR;
    }
    if ((who_am_i & 0x7EU) != MPU6050_WHO_AM_I_VALUE)
    {
        return MPU6050_STATUS_NOT_FOUND;
    }
    if (!configure_sensor(true))
    {
        return MPU6050_STATUS_I2C_ERROR;
    }
    calibration_status = calibrate_gyro(average_accel);
    if (calibration_status != MPU6050_STATUS_OK)
    {
        return calibration_status;
    }

    set_initial_attitude(average_accel);
    /* 启动校准已经确认设备静止，直接从静止锁定状态开始。 */
    g_filter.stationary_score = MPU6050_STATIONARY_SCORE_MAX;
    g_data.stationary = true;
    g_filter.last_update_ms = tick_ms;
    return MPU6050_STATUS_OK;
}

bool mpu6050_update(void)
{
    float raw_gyro[3];
    float dt;
    float accel_norm;
    float gyro_norm;
    uint32_t now;

    /* g_bus 未初始化（mpu6050_service_init 被跳过），直接返回 */
    if (g_bus.i2c == NULL)
    {
        return false;
    }

    if (!read_raw(g_data.accel_g, raw_gyro, &g_data.temperature_c))
    {
        g_data.communication_errors++;
        if (++g_filter.consecutive_errors >= 3U)
        {
            (void)configure_sensor(true);
            g_filter.consecutive_errors = 0U;
            g_filter.last_update_ms = tick_ms;
        }
        return false;
    }
    g_filter.consecutive_errors = 0U;

    now = tick_ms;
    dt = (float)(uint32_t)(now - g_filter.last_update_ms) * 0.001f;
    g_filter.last_update_ms = now;
    if ((dt < 0.004f) || (dt > 0.05f))
    {
        dt = 0.01f;
    }

    for (uint32_t axis = 0U; axis < 3U; axis++)
    {
        g_data.gyro_dps[axis] = raw_gyro[axis] - g_data.gyro_bias_dps[axis];
    }
    accel_norm = sqrtf(g_data.accel_g[0] * g_data.accel_g[0] +
                       g_data.accel_g[1] * g_data.accel_g[1] +
                       g_data.accel_g[2] * g_data.accel_g[2]);
    gyro_norm = sqrtf(g_data.gyro_dps[0] * g_data.gyro_dps[0] +
                      g_data.gyro_dps[1] * g_data.gyro_dps[1] +
                      g_data.gyro_dps[2] * g_data.gyro_dps[2]);

    if ((fabsf(accel_norm - 1.0f) > MPU6050_MOTION_ACCEL_G) ||
        (gyro_norm > MPU6050_MOTION_GYRO_DPS))
    {
        g_filter.stationary_score = 0U;
        g_data.stationary = false;
    }
    else
    {
        const bool quiet =
            (fabsf(accel_norm - 1.0f) < MPU6050_STATIONARY_ACCEL_G) &&
            (gyro_norm < MPU6050_STATIONARY_GYRO_DPS);

        if (quiet)
        {
            if (g_filter.stationary_score < MPU6050_STATIONARY_SCORE_MAX)
            {
                g_filter.stationary_score++;
            }
        }
        else if (g_filter.stationary_score >= 4U)
        {
            g_filter.stationary_score -= 4U;
        }
        else
        {
            g_filter.stationary_score = 0U;
        }

        if (!g_data.stationary &&
            (g_filter.stationary_score >= MPU6050_STATIONARY_SCORE_ENTER))
        {
            g_data.stationary = true;
        }
        else if (g_data.stationary &&
                 (g_filter.stationary_score <= MPU6050_STATIONARY_SCORE_EXIT))
        {
            g_data.stationary = false;
        }
    }

    if (g_data.stationary)
    {
        for (uint32_t axis = 0U; axis < 3U; axis++)
        {
            g_data.gyro_bias_dps[axis] += MPU6050_BIAS_TRACK_ALPHA *
                (raw_gyro[axis] - g_data.gyro_bias_dps[axis]);
            g_data.gyro_dps[axis] = 0.0f;
        }
    }

    fusion_update(g_data.gyro_dps[0], g_data.gyro_dps[1],
        g_data.gyro_dps[2], g_data.accel_g[0], g_data.accel_g[1],
        g_data.accel_g[2], dt);
    update_euler_and_yaw(dt);
    g_data.sample_count++;
    return true;
}

const Mpu6050Data *mpu6050_get_data(void)
{
    return &g_data;
}

float mpu6050_get_roll(void)
{
    return g_data.roll_deg;
}

float mpu6050_get_pitch(void)
{
    return g_data.pitch_deg;
}

float mpu6050_get_yaw(void)
{
    return g_data.yaw_deg;
}
