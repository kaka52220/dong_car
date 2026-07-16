/*
 *sysconfig配置步骤:
 *      GPIO: 
 *      1.添加GPIO模块。   
 *      2.将该组命名为“ GPIO_OLED ”。  
 *      3.设置5个pin分别命名为“ PIN_RES ”,“ PIN_SCL ”,“ PIN_SDA ”,“ PIN_DC ”,“ PIN_CS ”。
 *      9.根据您的需要设置引脚。
 */

#ifndef _OLED_H_
#define _OLED_H_
 
#include "clock.h"    
#include "u8g2.h"  
#include "ti_msp_dl_config.h"

#define OLED_IIC_MODE

//---------------OLED--------------//  					   
#define OLED_SCLK_Clr() 	DL_GPIO_clearPins(GPIO_OLED_PORT, GPIO_OLED_PIN_SCL_PIN )
#define OLED_SCLK_Set()	 	DL_GPIO_setPins(GPIO_OLED_PORT, GPIO_OLED_PIN_SCL_PIN )

#define OLED_SDIN_Clr() 	DL_GPIO_clearPins(GPIO_OLED_PORT, GPIO_OLED_PIN_SDA_PIN )
#define OLED_SDIN_Set() 	DL_GPIO_setPins(GPIO_OLED_PORT, GPIO_OLED_PIN_SDA_PIN )

#define OLED_RES_Clr() 	    DL_GPIO_clearPins(GPIO_OLED_PORT, GPIO_OLED_PIN_RES_PIN )
#define OLED_RES_Set() 	    DL_GPIO_setPins(GPIO_OLED_PORT, GPIO_OLED_PIN_RES_PIN )

#define OLED_DC_Clr() 	    DL_GPIO_clearPins(GPIO_OLED_PORT, GPIO_OLED_PIN_DC_PIN )
#define OLED_DC_Set() 	    DL_GPIO_setPins(GPIO_OLED_PORT, GPIO_OLED_PIN_DC_PIN )

#define OLED_CS_Clr() 	    DL_GPIO_clearPins(GPIO_OLED_PORT, GPIO_OLED_PIN_CS_PIN )
#define OLED_CS_Set() 	    DL_GPIO_setPins(GPIO_OLED_PORT, GPIO_OLED_PIN_CS_PIN )

 //------------GPIO---------------//
#define OLED_SDIN_Write(v)  (v ? OLED_SDIN_Set()  :  OLED_SDIN_Clr())
#define OLED_SCLK_Write(v)  (v ? OLED_SCLK_Set()  :  OLED_SCLK_Clr())
#define OLED_DC_Write(v)    (v ? OLED_DC_Set()    :  OLED_DC_Clr())
#define OLED_CS_Write(v)    (v ? OLED_CS_Set()    :  OLED_CS_Clr())
#define OLED_RES_Write(v)   (v ? OLED_RES_Set()   :  OLED_RES_Clr())

void u8g2Init(u8g2_t *u8g2);

 
#endif