/**
 * @file    bsp_sw_i2c.c
 * @brief   Software I2C (bit-bang) implementation for MSPM0G3507
 *
 *          Pin mapping:
 *            SCL -> PA15 (IOMUX_PINCM37, GPIOA, DL_GPIO_PIN_15)
 *            SDA -> PA17 (IOMUX_PINCM39, GPIOA, DL_GPIO_PIN_17)
 *
 *          SDA open-drain emulation:
 *            Low:  enable output + clear pin  (drives actively to GND)
 *            High: disable output             (hi-Z, external pull-up to VDD)
 *
 *          Timing tuned for ~100 kHz I2C at 32 MHz system clock.
 */

#include "bsp_sw_i2c.h"
#include "ti_msp_dl_config.h"

/*===========================================================================
 *  Pin / Port Definitions
 *===========================================================================*/

#define I2C_SCL_PIN     DL_GPIO_PIN_15
#define I2C_SDA_PIN     DL_GPIO_PIN_17
#define I2C_PORT        GPIOA
#define I2C_SCL_PINCM   IOMUX_PINCM37       /* PA15 */
#define I2C_SDA_PINCM   IOMUX_PINCM39       /* PA17 */

/*===========================================================================
 *  Timing Configuration
 *===========================================================================*/

/**
 * @brief   Software delay loop count for ~100 kHz I2C
 *
 *          At 32 MHz SYSOSC default, each loop iteration ≈ 5 cycles (nop + branch).
 *          32 iterations × 5 cycles ≈ 160 cycles ≈ 5.0 µs half-period → ~100 kHz SCL.
 *
 *          Tune this value if your clock differs. Use a scope on SCL to verify.
 */
#ifndef I2C_DELAY_COUNT
#define I2C_DELAY_COUNT     32
#endif

/*===========================================================================
 *  Low-level Pin Macros
 *===========================================================================*/

/* SCL is push-pull output: set = high, clear = low */
#define SCL_H()             DL_GPIO_setPins(I2C_PORT, I2C_SCL_PIN)
#define SCL_L()             DL_GPIO_clearPins(I2C_PORT, I2C_SCL_PIN)

/* SDA open-drain emulation */
#define SDA_H()             DL_GPIO_disableOutput(I2C_PORT, I2C_SDA_PIN)   /* release bus → hi-Z, pull-up high */
#define SDA_L()             do { \
                                DL_GPIO_clearPins(I2C_PORT, I2C_SDA_PIN);  \
                                DL_GPIO_enableOutput(I2C_PORT, I2C_SDA_PIN); \
                            } while(0)

#define SDA_READ()          (DL_GPIO_readPins(I2C_PORT, I2C_SDA_PIN) ? 1 : 0)

/*===========================================================================
 *  Static Helpers
 *===========================================================================*/

/** Brief delay for I2C timing (~5 µs at 32 MHz) */
static void i2c_delay(void)
{
    volatile uint32_t d = I2C_DELAY_COUNT;
    while (d--) {
        __asm("nop");
    }
}

/*===========================================================================
 *  Public API Implementation
 *===========================================================================*/

/**
 * @brief   Initialize I2C GPIO pins to idle state
 *
 *          SCL: push-pull output, starts HIGH (idle)
 *          SDA: initially input (hi-Z), external pull-up keeps HIGH
 */
void BSP_SW_I2C_Init(void)
{
    /* SCL — standard push-pull output, idle high */
    DL_GPIO_initDigitalOutput(I2C_SCL_PINCM);
    DL_GPIO_enableOutput(I2C_PORT, I2C_SCL_PIN);
    SCL_H();

    /* SDA — configured as input with input-buffer enabled (open-drain emulation) */
    DL_GPIO_initDigitalInput(I2C_SDA_PINCM);
    /* SDA already hi-Z (no output), pulled high by external resistor */

    /* Bus idle: both lines high */
    i2c_delay();
}

/**
 * @brief   I2C START: SDA falls while SCL is high
 *
 *          Idle state: SCL=H, SDA=H
 *          Sequence:  SDA↓ → delay → SCL↓
 */
void BSP_SW_I2C_Start(void)
{
    SDA_H();
    SCL_H();
    i2c_delay();

    SDA_L();
    i2c_delay();

    SCL_L();
    i2c_delay();
}

/**
 * @brief   I2C STOP: SDA rises while SCL is high
 *
 *          Sequence:  SCL↑ → delay → SDA↑ → delay
 */
void BSP_SW_I2C_Stop(void)
{
    SDA_L();
    i2c_delay();

    SCL_H();
    i2c_delay();

    SDA_H();
    i2c_delay();
}

/**
 * @brief   Send one byte MSB-first, return slave ACK bit
 * @param   data  Byte to transmit
 * @return  0 = ACK, 1 = NACK
 */
uint8_t BSP_SW_I2C_SendByte(uint8_t data)
{
    uint8_t i, ack;

    for (i = 0; i < 8; i++) {
        /* Put data bit on SDA (MSB first) */
        if (data & 0x80) {
            SDA_H();
        } else {
            SDA_L();
        }
        i2c_delay();

        /* Clock pulse: SCL high → sample → SCL low */
        SCL_H();
        i2c_delay();
        SCL_L();
        i2c_delay();

        data <<= 1;
    }

    /* Release SDA for slave ACK */
    SDA_H();
    i2c_delay();

    /* 9th clock for ACK */
    SCL_H();
    i2c_delay();
    ack = SDA_READ();
    SCL_L();
    i2c_delay();

    return ack;
}

/**
 * @brief   Read one byte from slave, MSB first
 * @param   ack  0 = send ACK (more bytes follow), 1 = send NACK (last byte)
 * @return  Byte read
 */
uint8_t BSP_SW_I2C_ReadByte(uint8_t ack)
{
    uint8_t i, data = 0;

    /* Release SDA so slave can drive it */
    SDA_H();

    for (i = 0; i < 8; i++) {
        data <<= 1;

        SCL_H();
        i2c_delay();
        if (SDA_READ()) {
            data |= 0x01;
        }
        SCL_L();
        i2c_delay();
    }

    /* Master ACK / NACK */
    if (ack) {
        SDA_H();   /* NACK: release high */
    } else {
        SDA_L();   /* ACK: pull low */
    }
    i2c_delay();

    SCL_H();
    i2c_delay();
    SCL_L();
    i2c_delay();

    /* Release SDA after ACK */
    SDA_H();

    return data;
}

/**
 * @brief   Send dummy byte to poll slave ACK (used for device detection)
 * @return  0 = ACK received, 1 = NACK
 */
uint8_t BSP_SW_I2C_WaitAck(void)
{
    uint8_t ack;

    SDA_H();
    i2c_delay();

    SCL_H();
    i2c_delay();
    ack = SDA_READ();
    SCL_L();
    i2c_delay();

    return ack;
}
