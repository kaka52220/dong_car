/**
 * @file    bsp_sw_i2c.h
 * @brief   Software I2C (bit-bang) interface for MSPM0G3507
 * @note    SCL -> PA15 (IOMUX_PINCM37), SDA -> PA17 (IOMUX_PINCM39)
 *
 *          Uses GPIO bit-banging to emulate I2C protocol.
 *          SDA operates in open-drain emulation mode:
 *          - Output LOW:  enable output + clear pin
 *          - Output HIGH: disable output (hi-Z, pull-up pulls high)
 *          SCL operates in push-pull mode.
 */

#ifndef BSP_SW_I2C_H
#define BSP_SW_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/*  Public Functions                                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief   Initialize software I2C GPIO pins and idle state
 * @note    Must be called before any other I2C function.
 *          Configures SCL as output (push-pull), SDA as input (open-drain).
 */
void BSP_SW_I2C_Init(void);

/**
 * @brief   Generate I2C START condition (SDA falls while SCL is high)
 */
void BSP_SW_I2C_Start(void);

/**
 * @brief   Generate I2C STOP condition (SDA rises while SCL is high)
 */
void BSP_SW_I2C_Stop(void);

/**
 * @brief   Send one byte via I2C, MSB first
 * @param   data  Byte to send
 * @return  ACK bit from slave: 0 = ACK, 1 = NACK
 */
uint8_t BSP_SW_I2C_SendByte(uint8_t data);

/**
 * @brief   Read one byte from I2C slave, MSB first
 * @param   ack  0 = send ACK after read, 1 = send NACK (last byte)
 * @return  Byte read from slave
 */
uint8_t BSP_SW_I2C_ReadByte(uint8_t ack);

/**
 * @brief   Wait for slave ACK after address/data send
 * @return  0 = ACK received, 1 = NACK / timeout
 */
uint8_t BSP_SW_I2C_WaitAck(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SW_I2C_H */
