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
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    // 下面的嵌套循环的次数是根据主控频率和编译器生成的指令周期大致计算出来的，
    // 需要通过实际测试调整来达到所需的延时。
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 8000; j++)
        {
            // 仅执行一个足够简单以致于可以预测其执行时间的操作
            __asm__("nop"); // "nop" 代表“无操作”，在大多数架构中，这会消耗一个或几个时钟周期
        }
    }
}

// int main(void)
// {
//     SYSCFG_DL_init();
//     while (1)
//     {
//         DL_GPIO_clearPins(LED1_PORT,LED1_PIN_22_PIN);//输出低电平
//         delay_ms(1000);//延时大概1S
//         DL_GPIO_setPins(LED1_PORT,LED1_PIN_22_PIN);  //输出高电平
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
void MSPM0_Init(){
    SYSCFG_DL_init();
    SysTick_Init();
    NVIC_ClearPendingIRQ(GPIOA_INT_IRQn);
	NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOA_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}

int main(void)
{
    SYSCFG_DL_init();
    //Bluetooth_Init();
    //Key_Init();
    //delay_ms(100);
SysTick_Init();              // ← 新
  MotorTest_Init();            // ← 新增
    DL_GPIO_setPins(GPIO_MOTOR_STBY_PORT, GPIO_MOTOR_STBY_PIN);
    // DL_GPIO_clearPins(GPIO_MOTOR_STBY_PORT, GPIO_MOTOR_STBY_PIN);
    while(1)
    {
    //    car_run(100,0);
      MotorTest_Forward(500);  // ← 替换
      delay_ms(2000);
         MotorTest_Stop();
         delay_ms(2000);
    //       delay_ms(1000);
    MotorTest_Reverse(500);
     delay_ms(2000);
    //         delay_ms(2000);
    //           MotorTest_Stop();
    //            delay_ms(1000);
    }
}
// int main(void)
// {
//     SYSCFG_DL_init();
//     NVIC_EnableIRQ(KEY_INT_IRQN);//开启按键引脚的GPIOA端口中断
//     while (1)
//     {

//     }
// }

// void GROUP1_IRQHandler(void)//Group1的中断服务函数
// {
//     //读取Group1的中断寄存器并清除中断标志位
//     switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) )
//     {
//         //检查是否是KEY的GPIOB端口中断，注意是INT_IIDX，不是PIN_22_IIDX
//         case KEY_INT_IIDX:
//             //如果按键按下变为低电平
//             if( DL_GPIO_readPins(KEY_PORT, KEY_PIN_21_PIN) == 0 )
//             {
//                 //设置LED引脚状态翻转
//                 DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
//             }
//         break;
//     }
// }
