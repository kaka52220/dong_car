/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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

// #include "ti_msp_dl_config.h"

// //自定义延时（不精确）
// void delay_ms(unsigned int ms)
// {
//     unsigned int i, j;
//     // 下面的嵌套循环的次数是根据主控频率和编译器生成的指令周期大致计算出来的，
//     // 需要通过实际测试调整来达到所需的延时�?
//     for (i = 0; i < ms; i++)
//     {
//         for (j = 0; j < 8000; j++)
//         {
//             // 仅执行一个足够简单以致于可以预测其执行时间的操作
//             __asm__("nop"); // "nop" 代表“无操作”，在大多数架构中，这会消耗一个或几个时钟周期
//         }
//     }
// }

// int main(void)
// {
//     SYSCFG_DL_init();
//     while (1)
//     {
//         DL_GPIO_clearPins(LED1_PORT,LED1_PIN_22_PIN);//输出低电�?
//         delay_ms(1000);//延时大概1S
//         DL_GPIO_setPins(LED1_PORT,LED1_PIN_22_PIN);  //输出高电�?
//         delay_ms(1000);//延时大概1S
//     }
// }
#include "ti_msp_dl_config.h"
#include "board.h"
#include "hc05.h"
#include "APPLICATION/key.h"
#include "clock.h"
#include "interrupt.h"//中断
#include "electrical_machinery.h"//小车控制
#include "motor_test.h"
#include "APPLICATION/line_follower.h"
#include "APPLICATION/grayscale_sensor.h"
#include "usart.h"
void MSPM0_Init(){
    SYSCFG_DL_init();
    SysTick_Init();//系统1ms定时器和编码�?20ms定时//编码器和key中断,位于interrupt.c
    
    NVIC_ClearPendingIRQ(GPIOA_INT_IRQn);
	NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOA_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}


// uint8_t g_sensor_data[GRAYSCALE_SENSOR_CHANNELS];
// int i;
uint8_t test[8] = {0};
int main(void)
{
    //Bluetooth_Init();
    MSPM0_Init();
   // USART_Init();
    //Key_Init();
   
   DL_GPIO_setPins(GPIO_MOTOR_STBY_PORT, GPIO_MOTOR_STBY_PIN);  // �? 加这  
    while(1)
    {
      
     if(stop_flag)car_stop();
             else CAR_CONTROL();
	// Grayscale_Sensor_Read_All(g_sensor_data);  
    //     for (i = 0; i < GRAYSCALE_SENSOR_CHANNELS; i++)
    //     {
    //         USART_SendData(g_sensor_data[i] + '0');
    //     }
    //          USART_SendString("\r\n");    
	// 	delay_ms(30);

    }
}