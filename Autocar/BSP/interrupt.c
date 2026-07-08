#include "ti_msp_dl_config.h"
#include "interrupt.h"
#include "clock.h"
//#include "mpu6050.h"

int nextway=0;

static ENCODER_RES motor_encoderA,motor_encoderB,motor_encoderC,motor_encoderD;

//获取编码器的值
int get_encoder_count(char encoder_name)
{
    switch (encoder_name) {
        case 'A':
            return motor_encoderA.count;
        case 'B':
            return motor_encoderB.count;
        case 'C':
            return motor_encoderC.count;
        case 'D':
            return motor_encoderD.count;
        default:
            return 0;
    }
}
//获取编码器的方向
ENCODER_DIR get_encoder_dir(char encoder_name)
{
    switch (encoder_name) {
        case 'A':
            return motor_encoderA.dir;
        case 'B':
            return motor_encoderB.dir;
        case 'C':
            return motor_encoderC.dir;
        case 'D':
            return motor_encoderD.dir;
        default:
            return 0;
    }
}
volatile long long get_encoder_temp_count(char encoder_name)
{
    switch (encoder_name) {
        case 'A':
            return motor_encoderA.temp_count;
        case 'B':
            return motor_encoderB.temp_count;
        case 'C':
            return motor_encoderC.temp_count;
        case 'D':
            return motor_encoderD.temp_count;
        default:
            return 0;
    }
}


void SysTick_Handler(void)
{
    tick_ms++;
}

void TIMER_TICK_INST_IRQHandler(void)
{
	//20ms归零中断触发
	if( DL_TimerG_getPendingInterrupt(TIMER_TICK_INST) == DL_TIMER_IIDX_ZERO )
	{
		motor_encoderA.count = motor_encoderA.temp_count;
        motor_encoderB.count = motor_encoderB.temp_count;
        motor_encoderC.count = motor_encoderC.temp_count;
        motor_encoderD.count = motor_encoderD.temp_count;
        //确定方向
        motor_encoderA.dir = ( motor_encoderA.count >= 0 ) ? FORWARD : REVERSAL;
        motor_encoderB.dir = ( motor_encoderB.count >= 0 ) ? FORWARD : REVERSAL;
        motor_encoderC.dir = ( motor_encoderC.count >= 0 ) ? FORWARD : REVERSAL;
        motor_encoderD.dir = ( motor_encoderD.count >= 0 ) ? FORWARD : REVERSAL;
        //编码器计数值清零
        motor_encoderA.temp_count = 0;
        motor_encoderB.temp_count = 0;
        motor_encoderC.temp_count = 0;
        motor_encoderD.temp_count = 0;
	}
}

// void GROUP1_IRQHandler(void)
// {
//     switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
//     {
//         // pins affected: ["E1A","E1B","E2A","E2B","KEY2","KEY3","PIN_INT"]
//         case DL_INTERRUPT_GROUP1_IIDX_GPIOA:
//             switch (DL_GPIO_getPendingInterrupt(GPIOA)) {
//                 case GPIO_KEYS_KEY2_IIDX:
//                     ; // 空语句，解决C23扩展警告
//                     static uint32_t key2_start_time = 0;
//                     if(tick_ms - key2_start_time >= 300)
//                     {
//                         nextway = 1;
//                         key2_start_time = tick_ms;
//                     }
//                     break;
//                 case GPIO_KEYS_KEY3_IIDX:
//                     ; // 空语句，解决C23扩展警告
//                     static uint32_t key3_start_time = 0;
//                     if(tick_ms - key3_start_time >= 300)
//                     {
//                         nextway = 2;
//                         key3_start_time = tick_ms;
//                     }
//                     break;
//                 case GPIO_MPU6050_PIN_INT_IIDX:
//                     Read_Quad();
//                     break;
//                 case GPIO_ENCODER_E1A_IIDX:
//                     if(!DL_GPIO_readPins(GPIOA, GPIO_ENCODER_E1B_PIN))
//                     {
//                         motor_encoderA.temp_count--;
//                     }
//                     else
//                     {
//                         motor_encoderA.temp_count++;
//                     }
//                     break;
//                 case GPIO_ENCODER_E1B_IIDX:
//                 	if(!DL_GPIO_readPins(GPIOA, GPIO_ENCODER_E1A_PIN))
//                     {
//                         motor_encoderA.temp_count++;
//                     }
//                     else
//                     {
//                         motor_encoderA.temp_count--;
//                     }
//                     break;
//                 case GPIO_ENCODER_E2A_IIDX:
//                     if(!DL_GPIO_readPins(GPIOA, GPIO_ENCODER_E2B_PIN))
//                     {
//                         motor_encoderB.temp_count--;
//                     }
//                     else
//                     {
//                         motor_encoderB.temp_count++;
//                     }
//                     break;
//                 case GPIO_ENCODER_E2B_IIDX:
//                 	if(!DL_GPIO_readPins(GPIOA, GPIO_ENCODER_E2A_PIN))
//                     {
//                         motor_encoderB.temp_count++;
//                     }
//                     else
//                     {
//                         motor_encoderB.temp_count--;
//                     }
//                     break;
//                 default:
//                     break;
//             }
//         // pins affected: ["KEY1","KEY4","E3A","E3B","E4A","E4B"]
//         case DL_INTERRUPT_GROUP1_IIDX_GPIOB:
//             switch (DL_GPIO_getPendingInterrupt(GPIOB))
//             {
//                 case GPIO_KEYS_KEY1_IIDX:
//                     ; // 空语句，解决C23扩展警告
//                     static uint32_t key1_start_time = 0;
//                     if(tick_ms - key1_start_time >= 300)
//                     {
//                         nextway = -1;
//                         key1_start_time = tick_ms;
//                     }
//                     break;
//                 case GPIO_KEYS_KEY4_IIDX:
//                     ; // 空语句，解决C23扩展警告
//                     static uint32_t key4_start_time = 0;
//                     if(tick_ms - key4_start_time >= 300)
//                     {
//                         nextway = 3;
//                         key4_start_time = tick_ms;
//                     }
//                     break;
//                 case GPIO_ENCODER_E3A_IIDX:
//                     if(!DL_GPIO_readPins(GPIOB, GPIO_ENCODER_E3B_PIN))
//                     {
//                         motor_encoderC.temp_count++;
//                     }
//                     else
//                     {
//                         motor_encoderC.temp_count--;
//                     }
//                     break;
//                 case GPIO_ENCODER_E3B_IIDX:
//                 	if(!DL_GPIO_readPins(GPIOB, GPIO_ENCODER_E3A_PIN))
//                     {
//                         motor_encoderC.temp_count--;
//                     }
//                     else
//                     {
//                         motor_encoderC.temp_count++;
//                     }
//                     break;
//                 case GPIO_ENCODER_E4A_IIDX:
//                     if(!DL_GPIO_readPins(GPIOB, GPIO_ENCODER_E4B_PIN))
//                     {
//                         motor_encoderD.temp_count++;
//                     }
//                     else
//                     {
//                         motor_encoderD.temp_count--;
//                     }
//                     break;
//                 case GPIO_ENCODER_E4B_IIDX:
//                 	if(!DL_GPIO_readPins(GPIOB, GPIO_ENCODER_E4A_PIN))
//                     {
//                         motor_encoderD.temp_count--;
//                     }
//                     else
//                     {
//                         motor_encoderD.temp_count++;
//                     }
//                     break;
//                 default:
//                     break;
//             }
//         default:
//             break;
//     }
// }