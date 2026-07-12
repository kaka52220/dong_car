#ifndef __GRAYSCALE_SENSOR_H
#define __GRAYSCALE_SENSOR_H

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "clock.h"
#include "hc05.h"

//=====================================================================================
//  引脚配置接口 (Pin Configuration)
//=====================================================================================
// --- 通道选择引脚定义 (AD0, AD1, AD2)  Channel selection pin definition (AD0, AD1, AD2)---
#define SENSOR_AD0_PORT         GrayS_PIN_0_PORT
#define SENSOR_AD0_PIN          GrayS_PIN_0_PIN

#define SENSOR_AD1_PORT         GrayS_PIN_1_PORT
#define SENSOR_AD1_PIN          GrayS_PIN_1_PIN

#define SENSOR_AD2_PORT         GrayS_PIN_2_PORT
#define SENSOR_AD2_PIN          GrayS_PIN_2_PIN

#define GrayS_OUT_PORT          GrayS_PIN_3_PORT
#define GrayS_OUT_PIN           GrayS_PIN_3_PIN

//=====================================================================================
//  GPIO操作抽象接口 (GPIO Operation Macros)
//=====================================================================================
#define GRAYSCALE_PIN_WRITE(port, pin, state) do { \
    if(state) DL_GPIO_setPins(port, pin); \
    else DL_GPIO_clearPins(port, pin); \
} while(0)

#define SENSOR_AD0_WRITE(state)  GRAYSCALE_PIN_WRITE(SENSOR_AD0_PORT, SENSOR_AD0_PIN, state)
#define SENSOR_AD1_WRITE(state)  GRAYSCALE_PIN_WRITE(SENSOR_AD1_PORT, SENSOR_AD1_PIN, state)
#define SENSOR_AD2_WRITE(state)  GRAYSCALE_PIN_WRITE(SENSOR_AD2_PORT, SENSOR_AD2_PIN, state)

#define SENSOR_OUT_READ()        (!!(DL_GPIO_readPins(GrayS_OUT_PORT, GrayS_OUT_PIN)))
//#define SDA_READ()          (DL_GPIO_readPins(I2C_PORT, I2C_SDA_PIN) ? 1 : 0) //相同效果
//--------------解释---------------------------//
/*TI MSPM0 DriverLib：返回原始位掩码

│ // TI MSPM0 DriverLib
│ uint32_t DL_GPIO_readPins(GPIO_Regs* port, uint32_t pins);


STM32 HAL：返回标准化逻辑值

│ // STM32 HAL 库
│ GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

返回值是 GPIO_PinState 枚举，只有两个值：

│ typedef enum {
│     GPIO_PIN_RESET = 0,  // 低电平
│     GPIO_PIN_SET   = 1   // 高电平
│ } GPIO_PinState;

所以 if (HAL_GPIO_ReadPin(port, pin) == 1) 完全没问题，因为 HAL 内部已经帮你做了归一化（? GPIO_PIN_SET :
GPIO_PIN_RESET）。*/
//--------------------------------------------------------//


//=====================================================================================
//  驱动函数接口 (Driver API)
//=====================================================================================

#define GRAYSCALE_SENSOR_CHANNELS   8   // 传感器通道总数 Number of sensor channels

void Grayscale_Sensor_Init(void);
void Grayscale_Sensor_Read_All(uint8_t* sensor_values);
uint16_t Grayscale_Sensor_Read_Single(uint8_t channel);

#endif // __GRAYSCALE_SENSOR_H
