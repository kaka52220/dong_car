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
#define GPIO_PWMB_C1_PIN                                          DL_GPIO_PIN_18
#define GPIO_PWMB_C1_IOMUX                                       (IOMUX_PINCM40)
#define GPIO_PWMB_C1_IOMUX_FUNC                      IOMUX_PINCM40_PF_TIMG7_CCP1
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



/* Defines for UART_2 */
#define UART_2_INST                                                        UART2
#define UART_2_INST_FREQUENCY                                           10000000
#define UART_2_INST_IRQHandler                                  UART2_IRQHandler
#define UART_2_INST_INT_IRQN                                      UART2_INT_IRQn
#define GPIO_UART_2_RX_PORT                                                GPIOB
#define GPIO_UART_2_TX_PORT                                                GPIOB
#define GPIO_UART_2_RX_PIN                                        DL_GPIO_PIN_16
#define GPIO_UART_2_TX_PIN                                        DL_GPIO_PIN_15
#define GPIO_UART_2_IOMUX_RX                                     (IOMUX_PINCM33)
#define GPIO_UART_2_IOMUX_TX                                     (IOMUX_PINCM32)
#define GPIO_UART_2_IOMUX_RX_FUNC                      IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_2_IOMUX_TX_FUNC                      IOMUX_PINCM32_PF_UART2_TX
#define UART_2_BAUD_RATE                                                  (9600)
#define UART_2_IBRD_10_MHZ_9600_BAUD                                        (65)
#define UART_2_FBRD_10_MHZ_9600_BAUD                                         (7)





/* Port definition for Pin Group LED1 */
#define LED1_PORT                                                        (GPIOB)

/* Defines for PIN_22: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED1_PIN_22_PIN                                         (DL_GPIO_PIN_22)
#define LED1_PIN_22_IOMUX                                        (IOMUX_PINCM50)
/* Port definition for Pin Group GPIO_GRP_0 */
#define GPIO_GRP_0_PORT                                                  (GPIOB)

/* Defines for PIN_0: GPIOB.13 with pinCMx 30 on package pin 1 */
#define GPIO_GRP_0_PIN_0_PIN                                    (DL_GPIO_PIN_13)
#define GPIO_GRP_0_PIN_0_IOMUX                                   (IOMUX_PINCM30)
/* Port definition for Pin Group HC05 */
#define HC05_PORT                                                        (GPIOA)

/* Defines for STATE: GPIOA.7 with pinCMx 14 on package pin 49 */
#define HC05_STATE_PIN                                           (DL_GPIO_PIN_7)
#define HC05_STATE_IOMUX                                         (IOMUX_PINCM14)
/* Defines for KEY1: GPIOB.18 with pinCMx 44 on package pin 15 */
#define KEY_KEY1_PORT                                                    (GPIOB)
// pins affected by this interrupt request:["KEY1","PB21","KEY4"]
#define KEY_GPIOB_INT_IRQN                                      (GPIOB_INT_IRQn)
#define KEY_GPIOB_INT_IIDX                      (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define KEY_KEY1_IIDX                                       (DL_GPIO_IIDX_DIO18)
#define KEY_KEY1_PIN                                            (DL_GPIO_PIN_18)
#define KEY_KEY1_IOMUX                                           (IOMUX_PINCM44)
/* Defines for KEY2: GPIOA.13 with pinCMx 35 on package pin 6 */
#define KEY_KEY2_PORT                                                    (GPIOA)
// pins affected by this interrupt request:["KEY2"]
#define KEY_GPIOA_INT_IRQN                                      (GPIOA_INT_IRQn)
#define KEY_GPIOA_INT_IIDX                      (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
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
void SYSCFG_DL_UART_2_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
