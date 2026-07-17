#include "ti_msp_dl_config.h"
#include "clock.h"
#include "mspm0_motor_i2c.h"

#define I2C_INST            I2C_BUS_INST
#define I2C_IOMUX_SCL       GPIO_I2C_BUS_IOMUX_SCL
#define I2C_IOMUX_SDA       GPIO_I2C_BUS_IOMUX_SDA
#define I2C_SCL_PORT        GPIO_I2C_BUS_SCL_PORT
#define I2C_SDA_PORT        GPIO_I2C_BUS_SDA_PORT
#define I2C_SCL_PIN         GPIO_I2C_BUS_SCL_PIN
#define I2C_SDA_PIN         GPIO_I2C_BUS_SDA_PIN
#define I2C_SDA_FUNC        GPIO_I2C_BUS_IOMUX_SDA_FUNC
#define I2C_SCL_FUNC        GPIO_I2C_BUS_IOMUX_SCL_FUNC 
#define SYSCFG_I2C_init     SYSCFG_DL_I2C_BUS_init

#define I2C_TIMEOUT_MS  (10)

static int mspm0_i2c_disable(void)
{
    DL_I2C_reset(I2C_INST);
    DL_GPIO_initDigitalOutput(I2C_IOMUX_SCL);
    DL_GPIO_initDigitalInputFeatures(I2C_IOMUX_SDA,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_clearPins(I2C_SCL_PORT, I2C_SCL_PIN);
    DL_GPIO_enableOutput(I2C_SCL_PORT, I2C_SCL_PIN);
    return 0;
}

static int mspm0_i2c_enable(void)
{
    DL_I2C_reset(I2C_INST);
    DL_GPIO_initPeripheralInputFunctionFeatures(I2C_IOMUX_SDA,
        I2C_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(I2C_IOMUX_SCL,
        I2C_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(I2C_IOMUX_SDA);
    DL_GPIO_enableHiZ(I2C_IOMUX_SCL);
    DL_I2C_enablePower(I2C_INST);
    SYSCFG_I2C_init();
    return 0;
}

void motor_motor_i2c_sda_unlock(void)
{
    uint8_t cycleCnt = 0;
    mspm0_i2c_disable();
    do
    {
        DL_GPIO_clearPins(I2C_SCL_PORT, I2C_SCL_PIN);
        mspm0_delay_ms(1);
        DL_GPIO_setPins(I2C_SCL_PORT, I2C_SCL_PIN);
        mspm0_delay_ms(1);

        if(DL_GPIO_readPins(I2C_SDA_PORT, I2C_SDA_PIN))
            break;
    }while(++cycleCnt < 100);
    mspm0_i2c_enable();
}

int mspm0_motor_i2c_write(unsigned char slave_addr,
                     unsigned char reg_addr,
                     unsigned char length,
                     unsigned char const *data)
{
    unsigned int cnt = length;
    unsigned char const *ptr = data;
    unsigned long start, cur;

    if (!length)
        return 0;

    mspm0_get_clock_ms(&start);

    DL_I2C_transmitControllerData(I2C_INST, reg_addr);
    DL_I2C_clearInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);

    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));

    DL_I2C_startControllerTransfer(I2C_INST, slave_addr, DL_I2C_CONTROLLER_DIRECTION_TX, length+1);

    do {
        unsigned fillcnt;
        fillcnt = DL_I2C_fillControllerTXFIFO(I2C_INST, (uint8_t*)ptr, cnt);
        cnt -= fillcnt;
        ptr += fillcnt;

        mspm0_get_clock_ms(&cur);
        if(cur >= (start + I2C_TIMEOUT_MS))
        {
            motor_motor_i2c_sda_unlock();
            return -1;
        }
    } while (!DL_I2C_getRawInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TX_DONE));

    return 0;
}

int mspm0_motor_i2c_read(unsigned char slave_addr,
                    unsigned char reg_addr,
                    unsigned char length,
                    unsigned char *data)
{
    unsigned i = 0;
    unsigned long start, cur;

    if (!length)
        return 0;

    /* 清零接收缓冲区，防止读取失败后残留旧数据 */
    {
        unsigned ci;
        for (ci = 0; ci < length; ci++) data[ci] = 0;
    }

    mspm0_get_clock_ms(&start);

    DL_I2C_transmitControllerData(I2C_INST, reg_addr);
    I2C_INST->MASTER.MCTR = I2C_MCTR_RD_ON_TXEMPTY_ENABLE;
    DL_I2C_clearInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);

    while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE));

    DL_I2C_startControllerTransfer(I2C_INST, slave_addr, DL_I2C_CONTROLLER_DIRECTION_RX, length);

    do {
        if (!DL_I2C_isControllerRXFIFOEmpty(I2C_INST))
        {
            uint8_t c;
            c = DL_I2C_receiveControllerData(I2C_INST);
            if (i < length)
            {
                data[i] = c;
                ++i;
            }
        }
        
        mspm0_get_clock_ms(&cur);
        if(cur >= (start + I2C_TIMEOUT_MS))
        {
            motor_motor_i2c_sda_unlock();
            return -1;
        }
    } while(!DL_I2C_getRawInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_RX_DONE));

    if (!DL_I2C_isControllerRXFIFOEmpty(I2C_INST))
    {
        uint8_t c;
        c = DL_I2C_receiveControllerData(I2C_INST);
        if (i < length)
        {
            data[i] = c;
            ++i;
        }
    }

    I2C_INST->MASTER.MCTR = 0;
    DL_I2C_flushControllerTXFIFO(I2C_INST);

    if(i == length)
        return 0;
    else
        return -1;
}

/*
 * 纯读操作（不发送寄存器地址），适用于 PCF8574 等无寄存器概念的芯片。
 * I2C 波形: START + slave_addr(R) + Data... + STOP
 */
int mspm0_motor_i2c_read_direct(unsigned char slave_addr,
                                unsigned char length,
                                unsigned char *data)
{
    unsigned i = 0;
    unsigned long start, cur;

    if (!length)
        return 0;

    /* 清零接收缓冲区 */
    {
        unsigned ci;
        for (ci = 0; ci < length; ci++) data[ci] = 0;
    }

    mspm0_get_clock_ms(&start);

    /* 关键区别：不调用 DL_I2C_transmitControllerData，不发送寄存器地址。
     * 同时清除 RD_ON_TXEMPTY_ENABLE 防止之前 mspm0_motor_i2c_read 残留的标志干扰纯读 */
    I2C_INST->MASTER.MCTR &= ~I2C_MCTR_RD_ON_TXEMPTY_ENABLE;
    DL_I2C_clearInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);

    {
        unsigned long wait_start;
        mspm0_get_clock_ms(&wait_start);
        while (!(DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        {
            unsigned long wait_cur;
            mspm0_get_clock_ms(&wait_cur);
            if (wait_cur >= (wait_start + 5))  /* 5ms 超时 */
            {
                motor_motor_i2c_sda_unlock();
                return -3;  /* 总线卡在 BUSY */
            }
        }
    }

    DL_I2C_startControllerTransfer(I2C_INST, slave_addr,
                                   DL_I2C_CONTROLLER_DIRECTION_RX, length);

    do {
        if (!DL_I2C_isControllerRXFIFOEmpty(I2C_INST))
        {
            uint8_t c;
            c = DL_I2C_receiveControllerData(I2C_INST);
            if (i < length)
            {
                data[i] = c;
                ++i;
            }
        }

        /* PCF8575 不响应地址 → NACK中断，立刻退出避免傻等 10ms */
        if (DL_I2C_getRawInterruptStatus(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_NACK))
        {
            motor_motor_i2c_sda_unlock();
            return -2;  /* NACK: 从机未应答，检查地址/接线 */
        }

        mspm0_get_clock_ms(&cur);
        if (cur >= (start + I2C_TIMEOUT_MS))
        {
            motor_motor_i2c_sda_unlock();
            return -1;  /* 超时: 总线卡死或无设备 */
        }
    } while (!DL_I2C_getRawInterruptStatus(I2C_INST,
               DL_I2C_INTERRUPT_CONTROLLER_RX_DONE));

    if (!DL_I2C_isControllerRXFIFOEmpty(I2C_INST))
    {
        uint8_t c;
        c = DL_I2C_receiveControllerData(I2C_INST);
        if (i < length)
        {
            data[i] = c;
            ++i;
        }
    }

    I2C_INST->MASTER.MCTR = 0;
    DL_I2C_flushControllerTXFIFO(I2C_INST);

    if (i == length)
        return 0;
    else
        return -1;
}