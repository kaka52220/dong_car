#include "mpu6050_service.h"

#include <stddef.h>
#include <stdint.h>

//#include "UART/vofa_justfloat.h"  /* VOFA 暂未移植，已注释 */
#include "mpu6050.h"
#include "ti_msp_dl_config.h"

static uint32_t g_next_sample_ms;
static bool g_justfloat_enabled;

/*
 * I2C1 完整初始化：SYSCFG 只配了时钟+滤波器，
 * 缺少 Controller 使能、Timer Period、FIFO 阈值。
 */
static void mpu6050_i2c1_full_init(void)
{
    SYSCFG_DL_I2C1_init();
    DL_I2C_resetControllerTransfer(I2C1_INST);
    DL_I2C_setTimerPeriod(I2C1_INST, 9);
    DL_I2C_setControllerTXFIFOThreshold(I2C1_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C1_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C1_INST);
    DL_I2C_enableController(I2C1_INST);
}

static const Mpu6050BusConfig g_bus_config = {
    .i2c = I2C1_INST,
    .address = MPU6050_DEFAULT_ADDRESS,
    .sda_port = GPIO_I2C1_SDA_PORT,
    .sda_pin = GPIO_I2C1_SDA_PIN,
    .sda_iomux = GPIO_I2C1_IOMUX_SDA,
    .sda_function = GPIO_I2C1_IOMUX_SDA_FUNC,
    .scl_port = GPIO_I2C1_SCL_PORT,
    .scl_pin = GPIO_I2C1_SCL_PIN,
    .scl_iomux = GPIO_I2C1_IOMUX_SCL,
    .scl_function = GPIO_I2C1_IOMUX_SCL_FUNC,
    .reinitialize_i2c = mpu6050_i2c1_full_init,
};

static void send_boot_heartbeat(void)
{
    uint32_t heartbeat;

    if (!g_justfloat_enabled)
    {
        return;
    }

    for (heartbeat = 0U; heartbeat < 10U; heartbeat++)
    {
        //vofa_justfloat_send_status(100.0f + (float)heartbeat);
        delay_cycles(CPUCLK_FREQ / 50U);
    }
}

static void wait_before_retry(Mpu6050Status status)
{
    if (status == MPU6050_STATUS_TIMEBASE_ERROR)
    {
        delay_cycles(CPUCLK_FREQ / 2U);
    }
    else
    {
        mpu6050_delay_ms(500U);
    }
}

Mpu6050Status mpu6050_service_init(uint32_t sda_pin, uint32_t scl_pin,
    bool enable_justfloat)
{
    Mpu6050Status status;

    if ((sda_pin != g_bus_config.sda_pin) ||
        (scl_pin != g_bus_config.scl_pin))
    {
        return MPU6050_STATUS_INVALID_CONFIG;
    }

    g_justfloat_enabled = enable_justfloat;
    send_boot_heartbeat();

    /* 确保 I2C1 控制器在首次通信前已完整初始化 */
    mpu6050_i2c1_full_init();

    {
        uint8_t retry;
        const uint8_t max_retries = 10U;

        for (retry = 0U; retry < max_retries; retry++)
        {
            status = mpu6050_init(&g_bus_config);
            if (status == MPU6050_STATUS_OK ||
                status == MPU6050_STATUS_INVALID_CONFIG)
            {
                break;
            }
            DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
            wait_before_retry(status);
        }
    }

    if (status != MPU6050_STATUS_OK)
    {
        return status;
    }
    DL_GPIO_clearPins(LED1_PORT, LED1_PIN_22_PIN);
    g_next_sample_ms = mpu6050_millis();
    return MPU6050_STATUS_OK;
}

void mpu6050_service_process(void)
{
    g_next_sample_ms += MPU6050_SAMPLE_PERIOD_MS;
    if (mpu6050_update())
    {
        /* VOFA 暂未移植，跳过波形输出 */
    }
    else
    {
        /* VOFA 暂未移植，跳过错误上报 */
    }

    while ((int32_t)(g_next_sample_ms - mpu6050_millis()) > 0)
    {
        __WFI();
    }

    /* 严重超时时重新对齐，避免连续追赶造成无意义的密集采样。 */
    if ((int32_t)(mpu6050_millis() - g_next_sample_ms) >
        (int32_t)(MPU6050_SAMPLE_PERIOD_MS * 4U))
    {
        g_next_sample_ms = mpu6050_millis();
    }
}
