/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2500
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for PWMA */
#define PWMA_INST                                                          TIMG8
#define PWMA_INST_IRQHandler                                    TIMG8_IRQHandler
#define PWMA_INST_INT_IRQN                                      (TIMG8_INT_IRQn)
#define PWMA_INST_CLK_FREQ                                              10000000
/* GPIO defines for channel 0 */
#define GPIO_PWMA_C0_PORT                                                  GPIOA
#define GPIO_PWMA_C0_PIN                                          DL_GPIO_PIN_26
#define GPIO_PWMA_C0_IOMUX                                       (IOMUX_PINCM59)
#define GPIO_PWMA_C0_IOMUX_FUNC                      IOMUX_PINCM59_PF_TIMG8_CCP0
#define GPIO_PWMA_C0_IDX                                     DL_TIMER_CC_0_INDEX

/* Defines for PWMB */
#define PWMB_INST                                                          TIMG7
#define PWMB_INST_IRQHandler                                    TIMG7_IRQHandler
#define PWMB_INST_INT_IRQN                                      (TIMG7_INT_IRQn)
#define PWMB_INST_CLK_FREQ                                              10000000
/* GPIO defines for channel 1 */
#define GPIO_PWMB_C1_PORT                                                  GPIOA
#define GPIO_PWMB_C1_PIN                                          DL_GPIO_PIN_24
#define GPIO_PWMB_C1_IOMUX                                       (IOMUX_PINCM54)
#define GPIO_PWMB_C1_IOMUX_FUNC                      IOMUX_PINCM54_PF_TIMG7_CCP1
#define GPIO_PWMB_C1_IDX                                     DL_TIMER_CC_1_INDEX

/* Defines for PWMC */
#define PWMC_INST                                                         TIMG12
#define PWMC_INST_IRQHandler                                   TIMG12_IRQHandler
#define PWMC_INST_INT_IRQN                                     (TIMG12_INT_IRQn)
#define PWMC_INST_CLK_FREQ                                              10000000
/* GPIO defines for channel 1 */
#define GPIO_PWMC_C1_PORT                                                  GPIOB
#define GPIO_PWMC_C1_PIN                                          DL_GPIO_PIN_24
#define GPIO_PWMC_C1_IOMUX                                       (IOMUX_PINCM52)
#define GPIO_PWMC_C1_IOMUX_FUNC                     IOMUX_PINCM52_PF_TIMG12_CCP1
#define GPIO_PWMC_C1_IDX                                     DL_TIMER_CC_1_INDEX

/* Defines for PWMD */
#define PWMD_INST                                                          TIMG6
#define PWMD_INST_IRQHandler                                    TIMG6_IRQHandler
#define PWMD_INST_INT_IRQN                                      (TIMG6_INT_IRQn)
#define PWMD_INST_CLK_FREQ                                              10000000
/* GPIO defines for channel 1 */
#define GPIO_PWMD_C1_PORT                                                  GPIOA
#define GPIO_PWMD_C1_PIN                                          DL_GPIO_PIN_22
#define GPIO_PWMD_C1_IOMUX                                       (IOMUX_PINCM47)
#define GPIO_PWMD_C1_IOMUX_FUNC                      IOMUX_PINCM47_PF_TIMG6_CCP1
#define GPIO_PWMD_C1_IDX                                     DL_TIMER_CC_1_INDEX



/* Defines for TIMER_TICK */
#define TIMER_TICK_INST                                                  (TIMG0)
#define TIMER_TICK_INST_IRQHandler                              TIMG0_IRQHandler
#define TIMER_TICK_INST_INT_IRQN                                (TIMG0_INT_IRQn)
#define TIMER_TICK_INST_LOAD_VALUE                                       (9999U)




/* Defines for I2C_BUS */
#define I2C_BUS_INST                                                        I2C0
#define I2C_BUS_INST_IRQHandler                                  I2C0_IRQHandler
#define I2C_BUS_INST_INT_IRQN                                      I2C0_INT_IRQn
#define I2C_BUS_BUS_SPEED_HZ                                              400000
#define GPIO_I2C_BUS_SDA_PORT                                              GPIOA
#define GPIO_I2C_BUS_SDA_PIN                                      DL_GPIO_PIN_28
#define GPIO_I2C_BUS_IOMUX_SDA                                    (IOMUX_PINCM3)
#define GPIO_I2C_BUS_IOMUX_SDA_FUNC                     IOMUX_PINCM3_PF_I2C0_SDA
#define GPIO_I2C_BUS_SCL_PORT                                              GPIOA
#define GPIO_I2C_BUS_SCL_PIN                                      DL_GPIO_PIN_31
#define GPIO_I2C_BUS_IOMUX_SCL                                    (IOMUX_PINCM6)
#define GPIO_I2C_BUS_IOMUX_SCL_FUNC                     IOMUX_PINCM6_PF_I2C0_SCL


/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           10000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_10_MHZ_115200_BAUD                                       (5)
#define UART_0_FBRD_10_MHZ_115200_BAUD                                      (27)





/* Port definition for Pin Group LED1 */
#define LED1_PORT                                                        (GPIOB)

/* Defines for PIN_22: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED1_PIN_22_PIN                                         (DL_GPIO_PIN_22)
#define LED1_PIN_22_IOMUX                                        (IOMUX_PINCM50)
/* Port definition for Pin Group HC05 */
#define HC05_PORT                                                        (GPIOA)

/* Defines for STATE: GPIOA.7 with pinCMx 14 on package pin 49 */
#define HC05_STATE_PIN                                           (DL_GPIO_PIN_7)
#define HC05_STATE_IOMUX                                         (IOMUX_PINCM14)
/* Port definition for Pin Group GPIO_OLED */
#define GPIO_OLED_PORT                                                   (GPIOB)

/* Defines for PIN_RES: GPIOB.10 with pinCMx 27 on package pin 62 */
#define GPIO_OLED_PIN_RES_PIN                                   (DL_GPIO_PIN_10)
#define GPIO_OLED_PIN_RES_IOMUX                                  (IOMUX_PINCM27)
/* Defines for PIN_SCL: GPIOB.9 with pinCMx 26 on package pin 61 */
#define GPIO_OLED_PIN_SCL_PIN                                    (DL_GPIO_PIN_9)
#define GPIO_OLED_PIN_SCL_IOMUX                                  (IOMUX_PINCM26)
/* Defines for PIN_SDA: GPIOB.8 with pinCMx 25 on package pin 60 */
#define GPIO_OLED_PIN_SDA_PIN                                    (DL_GPIO_PIN_8)
#define GPIO_OLED_PIN_SDA_IOMUX                                  (IOMUX_PINCM25)
/* Defines for PIN_DC: GPIOB.11 with pinCMx 28 on package pin 63 */
#define GPIO_OLED_PIN_DC_PIN                                    (DL_GPIO_PIN_11)
#define GPIO_OLED_PIN_DC_IOMUX                                   (IOMUX_PINCM28)
/* Defines for PIN_CS: GPIOB.14 with pinCMx 31 on package pin 2 */
#define GPIO_OLED_PIN_CS_PIN                                    (DL_GPIO_PIN_14)
#define GPIO_OLED_PIN_CS_IOMUX                                   (IOMUX_PINCM31)
/* Defines for KEY1: GPIOB.18 with pinCMx 44 on package pin 15 */
#define KEY_KEY1_PORT                                                    (GPIOB)
// groups represented: ["GPIO_ENCODER","KEY"]
// pins affected: ["E3A","E3B","E4A","E4B","KEY1","PB21","KEY4"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define KEY_KEY1_IIDX                                       (DL_GPIO_IIDX_DIO18)
#define KEY_KEY1_PIN                                            (DL_GPIO_PIN_18)
#define KEY_KEY1_IOMUX                                           (IOMUX_PINCM44)
/* Defines for KEY2: GPIOA.13 with pinCMx 35 on package pin 6 */
#define KEY_KEY2_PORT                                                    (GPIOA)
// groups represented: ["GPIO_ENCODER","KEY"]
// pins affected: ["E1A","E1B","E2A","E2B","KEY2","KEY3"]
#define GPIO_MULTIPLE_GPIOA_INT_IRQN                            (GPIOA_INT_IRQn)
#define GPIO_MULTIPLE_GPIOA_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define KEY_KEY2_IIDX                                       (DL_GPIO_IIDX_DIO13)
#define KEY_KEY2_PIN                                            (DL_GPIO_PIN_13)
#define KEY_KEY2_IOMUX                                           (IOMUX_PINCM35)
/* Defines for PB21: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_PB21_PORT                                                    (GPIOB)
#define KEY_PB21_IIDX                                       (DL_GPIO_IIDX_DIO21)
#define KEY_PB21_PIN                                            (DL_GPIO_PIN_21)
#define KEY_PB21_IOMUX                                           (IOMUX_PINCM49)
/* Defines for KEY4: GPIOB.1 with pinCMx 13 on package pin 48 */
#define KEY_KEY4_PORT                                                    (GPIOB)
#define KEY_KEY4_IIDX                                        (DL_GPIO_IIDX_DIO1)
#define KEY_KEY4_PIN                                             (DL_GPIO_PIN_1)
#define KEY_KEY4_IOMUX                                           (IOMUX_PINCM13)
/* Defines for KEY3: GPIOA.29 with pinCMx 4 on package pin 36 */
#define KEY_KEY3_PORT                                                    (GPIOA)
#define KEY_KEY3_IIDX                                       (DL_GPIO_IIDX_DIO29)
#define KEY_KEY3_PIN                                            (DL_GPIO_PIN_29)
#define KEY_KEY3_IOMUX                                            (IOMUX_PINCM4)
/* Defines for E1A: GPIOA.0 with pinCMx 1 on package pin 33 */
#define GPIO_ENCODER_E1A_PORT                                            (GPIOA)
#define GPIO_ENCODER_E1A_IIDX                                (DL_GPIO_IIDX_DIO0)
#define GPIO_ENCODER_E1A_PIN                                     (DL_GPIO_PIN_0)
#define GPIO_ENCODER_E1A_IOMUX                                    (IOMUX_PINCM1)
/* Defines for E1B: GPIOA.1 with pinCMx 2 on package pin 34 */
#define GPIO_ENCODER_E1B_PORT                                            (GPIOA)
#define GPIO_ENCODER_E1B_IIDX                                (DL_GPIO_IIDX_DIO1)
#define GPIO_ENCODER_E1B_PIN                                     (DL_GPIO_PIN_1)
#define GPIO_ENCODER_E1B_IOMUX                                    (IOMUX_PINCM2)
/* Defines for E2A: GPIOA.8 with pinCMx 19 on package pin 54 */
#define GPIO_ENCODER_E2A_PORT                                            (GPIOA)
#define GPIO_ENCODER_E2A_IIDX                                (DL_GPIO_IIDX_DIO8)
#define GPIO_ENCODER_E2A_PIN                                     (DL_GPIO_PIN_8)
#define GPIO_ENCODER_E2A_IOMUX                                   (IOMUX_PINCM19)
/* Defines for E2B: GPIOA.9 with pinCMx 20 on package pin 55 */
#define GPIO_ENCODER_E2B_PORT                                            (GPIOA)
#define GPIO_ENCODER_E2B_IIDX                                (DL_GPIO_IIDX_DIO9)
#define GPIO_ENCODER_E2B_PIN                                     (DL_GPIO_PIN_9)
#define GPIO_ENCODER_E2B_IOMUX                                   (IOMUX_PINCM20)
/* Defines for E3A: GPIOB.15 with pinCMx 32 on package pin 3 */
#define GPIO_ENCODER_E3A_PORT                                            (GPIOB)
#define GPIO_ENCODER_E3A_IIDX                               (DL_GPIO_IIDX_DIO15)
#define GPIO_ENCODER_E3A_PIN                                    (DL_GPIO_PIN_15)
#define GPIO_ENCODER_E3A_IOMUX                                   (IOMUX_PINCM32)
/* Defines for E3B: GPIOB.16 with pinCMx 33 on package pin 4 */
#define GPIO_ENCODER_E3B_PORT                                            (GPIOB)
#define GPIO_ENCODER_E3B_IIDX                               (DL_GPIO_IIDX_DIO16)
#define GPIO_ENCODER_E3B_PIN                                    (DL_GPIO_PIN_16)
#define GPIO_ENCODER_E3B_IOMUX                                   (IOMUX_PINCM33)
/* Defines for E4A: GPIOB.2 with pinCMx 15 on package pin 50 */
#define GPIO_ENCODER_E4A_PORT                                            (GPIOB)
#define GPIO_ENCODER_E4A_IIDX                                (DL_GPIO_IIDX_DIO2)
#define GPIO_ENCODER_E4A_PIN                                     (DL_GPIO_PIN_2)
#define GPIO_ENCODER_E4A_IOMUX                                   (IOMUX_PINCM15)
/* Defines for E4B: GPIOB.3 with pinCMx 16 on package pin 51 */
#define GPIO_ENCODER_E4B_PORT                                            (GPIOB)
#define GPIO_ENCODER_E4B_IIDX                                (DL_GPIO_IIDX_DIO3)
#define GPIO_ENCODER_E4B_PIN                                     (DL_GPIO_PIN_3)
#define GPIO_ENCODER_E4B_IOMUX                                   (IOMUX_PINCM16)
/* Defines for AIN1: GPIOA.27 with pinCMx 60 on package pin 31 */
#define GPIO_MOTOR_AIN1_PORT                                             (GPIOA)
#define GPIO_MOTOR_AIN1_PIN                                     (DL_GPIO_PIN_27)
#define GPIO_MOTOR_AIN1_IOMUX                                    (IOMUX_PINCM60)
/* Defines for AIN2: GPIOA.25 with pinCMx 55 on package pin 26 */
#define GPIO_MOTOR_AIN2_PORT                                             (GPIOA)
#define GPIO_MOTOR_AIN2_PIN                                     (DL_GPIO_PIN_25)
#define GPIO_MOTOR_AIN2_IOMUX                                    (IOMUX_PINCM55)
/* Defines for BIN1: GPIOB.25 with pinCMx 56 on package pin 27 */
#define GPIO_MOTOR_BIN1_PORT                                             (GPIOB)
#define GPIO_MOTOR_BIN1_PIN                                     (DL_GPIO_PIN_25)
#define GPIO_MOTOR_BIN1_IOMUX                                    (IOMUX_PINCM56)
/* Defines for BIN2: GPIOB.20 with pinCMx 48 on package pin 19 */
#define GPIO_MOTOR_BIN2_PORT                                             (GPIOB)
#define GPIO_MOTOR_BIN2_PIN                                     (DL_GPIO_PIN_20)
#define GPIO_MOTOR_BIN2_IOMUX                                    (IOMUX_PINCM48)
/* Defines for CIN1: GPIOA.14 with pinCMx 36 on package pin 7 */
#define GPIO_MOTOR_CIN1_PORT                                             (GPIOA)
#define GPIO_MOTOR_CIN1_PIN                                     (DL_GPIO_PIN_14)
#define GPIO_MOTOR_CIN1_IOMUX                                    (IOMUX_PINCM36)
/* Defines for CIN2: GPIOA.16 with pinCMx 38 on package pin 9 */
#define GPIO_MOTOR_CIN2_PORT                                             (GPIOA)
#define GPIO_MOTOR_CIN2_PIN                                     (DL_GPIO_PIN_16)
#define GPIO_MOTOR_CIN2_IOMUX                                    (IOMUX_PINCM38)
/* Defines for DIN1: GPIOB.17 with pinCMx 43 on package pin 14 */
#define GPIO_MOTOR_DIN1_PORT                                             (GPIOB)
#define GPIO_MOTOR_DIN1_PIN                                     (DL_GPIO_PIN_17)
#define GPIO_MOTOR_DIN1_IOMUX                                    (IOMUX_PINCM43)
/* Defines for DIN2: GPIOB.19 with pinCMx 45 on package pin 16 */
#define GPIO_MOTOR_DIN2_PORT                                             (GPIOB)
#define GPIO_MOTOR_DIN2_PIN                                     (DL_GPIO_PIN_19)
#define GPIO_MOTOR_DIN2_IOMUX                                    (IOMUX_PINCM45)
/* Defines for STBY: GPIOB.0 with pinCMx 12 on package pin 47 */
#define GPIO_MOTOR_STBY_PORT                                             (GPIOB)
#define GPIO_MOTOR_STBY_PIN                                      (DL_GPIO_PIN_0)
#define GPIO_MOTOR_STBY_IOMUX                                    (IOMUX_PINCM12)
/* Defines for PIN_0: GPIOA.12 with pinCMx 34 on package pin 5 */
#define GrayS_PIN_0_PORT                                                 (GPIOA)
#define GrayS_PIN_0_PIN                                         (DL_GPIO_PIN_12)
#define GrayS_PIN_0_IOMUX                                        (IOMUX_PINCM34)
/* Defines for PIN_1: GPIOB.23 with pinCMx 51 on package pin 22 */
#define GrayS_PIN_1_PORT                                                 (GPIOB)
#define GrayS_PIN_1_PIN                                         (DL_GPIO_PIN_23)
#define GrayS_PIN_1_IOMUX                                        (IOMUX_PINCM51)
/* Defines for PIN_2: GPIOB.27 with pinCMx 58 on package pin 29 */
#define GrayS_PIN_2_PORT                                                 (GPIOB)
#define GrayS_PIN_2_PIN                                         (DL_GPIO_PIN_27)
#define GrayS_PIN_2_IOMUX                                        (IOMUX_PINCM58)
/* Defines for PIN_3: GPIOB.7 with pinCMx 24 on package pin 59 */
#define GrayS_PIN_3_PORT                                                 (GPIOB)
#define GrayS_PIN_3_PIN                                          (DL_GPIO_PIN_7)
#define GrayS_PIN_3_IOMUX                                        (IOMUX_PINCM24)




/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_PWMA_init(void);
void SYSCFG_DL_PWMB_init(void);
void SYSCFG_DL_PWMC_init(void);
void SYSCFG_DL_PWMD_init(void);
void SYSCFG_DL_TIMER_TICK_init(void);
void SYSCFG_DL_I2C_BUS_init(void);
void SYSCFG_DL_UART_0_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
