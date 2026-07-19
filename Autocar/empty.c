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
#include "APPLICATION/velocity_pid.h"
#include "oled.h"
#include "draw.h"//显示内容
#include "mpu6050/mpu6050_service.h"


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
    MSPM0_Init();
    USART_Init();
    u8g2_t u8g2;
    u8g2Init(&u8g2);
    DL_GPIO_setPins(GPIO_MOTOR_STBY_PORT, GPIO_MOTOR_STBY_PIN);

    USART_SendString((unsigned char*)"Encoder Test\r\n");

    // /* MPU6050 初始化（I2C1: SDA=PA30, SCL=PA15, 不启用VOFA波形输出） */
    {
        Mpu6050Status mpu_status = mpu6050_service_init(
            DL_GPIO_PIN_30, DL_GPIO_PIN_15, false);
        if (mpu_status == MPU6050_STATUS_OK)
        {
            USART_SendString((unsigned char*)"MPU6050 init OK\r\n");
        }
        else
        {
            USART_SendString((unsigned char*)"MPU6050 init FAIL\r\n");
            /* 用简单方式打出错误码的十六进制值 */
            {
                int raw = (int)mpu_status;
                char hex[12];
                const char digits[] = "0123456789ABCDEF";
                uint8_t i = 0;
                if (raw < 0)
                {
                    USART_SendString((unsigned char*)"  raw= -0x");
                    raw = -raw;
                }
                else
                {
                    USART_SendString((unsigned char*)"  raw= 0x");
                }
                do {
                    hex[i++] = digits[raw & 0xF];
                    raw >>= 4;
                } while (raw != 0 && i < 8);
                hex[i] = '\0';
                /* 反转 */
                for (uint8_t j = 0; j < i / 2; j++) {
                    char t = hex[j];
                    hex[j] = hex[i - 1 - j];
                    hex[i - 1 - j] = t;
                }
                USART_SendString((unsigned char*)hex);
                USART_SendString((unsigned char*)"\r\n");
            }
        }
    }


    while (1)
    {
        // mpu6050_update();
        OLED_SHOW(&u8g2);

        line_follower_update();
        if(stop_flag) car_stop();
         else CAR_CONTROL();  

        /* ===== 每1秒打印一次传感器数据 ===== */
        {
            static uint32_t last_sensor_ms = 0;
            if (tick_ms - last_sensor_ms >= 1000)
            {
                last_sensor_ms = tick_ms;
                const Mpu6050Data *d = mpu6050_get_data();
                char buf[80];
                sprintf(buf,
                    "S:%d%d%d%d%d%d%d%d L:%.0f R:%.0f mm/s Y:%.1f\r\n",
                    s1, s2, s3, s4, s5, s6, s7, s8,
                    calculate_motor_speed('C'),
                    calculate_motor_speed('A'),
                    d->yaw_deg);
                USART_SendString((unsigned char*)buf);
            }
        }
    }
}

        //CAR_CONTROL();
        //delay_ms(20);

        // // ========== VOFA+ 波形模式 (每100ms发一帧) ==========
        // {
        //     static uint8_t cnt = 0;
        //     cnt++;
        //     if (cnt >= 5) {  // 20ms×5 = 100ms
        //         cnt = 0;
        //         float vofa_data[8];
        //         vofa_data[0] = calculate_motor_speed('A');
        //         vofa_data[1] = calculate_motor_speed('B');
        //         vofa_data[2] = calculate_motor_speed('C');
        //         vofa_data[3] = calculate_motor_speed('D');
        //         // 预留 PWM 通道, 目前填 0
        //         vofa_data[4] = 0;
        //         vofa_data[5] = 0;
        //         vofa_data[6] = 0;
        //         vofa_data[7] = 0;
        //         VOFA_SendFrame(vofa_data, 8);
        //     }
        // }

        // // ========== 文本调试模式 (二选一, 用VOFA时注释掉下面) ==========
        // char buf[100];
        // sprintf(buf, "A:%d/%.0f B:%d/%.0f C:%d/%.0f D:%d/%.0f\r\n",
        //     get_encoder_count('A'), calculate_motor_speed('A'),
        //     get_encoder_count('B'), calculate_motor_speed('B'),
        //     get_encoder_count('C'), calculate_motor_speed('C'),
        //     get_encoder_count('D'), calculate_motor_speed('D'));
        // USART_SendString((unsigned char*)buf);