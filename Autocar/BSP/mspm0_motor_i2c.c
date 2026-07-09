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