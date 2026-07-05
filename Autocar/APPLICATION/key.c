#include "key.h"

void Key_Init(void)
{
    #if KEY_FUNCtion 
        NVIC_EnableIRQ( GPIOB_INT_IRQn);//开启按键引脚的GPIOA端口中断
        NVIC_EnableIRQ( GPIOA_INT_IRQn);
    #endif
}

void GROUP1_IRQHandler(void)
{
    //读取GROUP里的中断寄存器并清除中断标志位
    switch( DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1) )
    {
        //检查是否是KEY的GPIOB端口中断，注意是INT_IIDX，不是KEY_KEY1_IIDX
        case KEY_GPIOB_INT_IIDX :
           if( DL_GPIO_readPins(KEY_KEY1_PORT, KEY_KEY1_PIN) > 0 )
            {
                //设置LED引脚状态翻转
                DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
            }
            if( DL_GPIO_readPins(KEY_PB21_PORT, KEY_PB21_PIN) == 0 )
            {
                //设置LED引脚状态翻转
                DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
            }
            if( DL_GPIO_readPins(KEY_KEY4_PORT, KEY_KEY4_PIN) > 0 )
            {
                //设置LED引脚状态翻转
                DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
            }
            DL_GPIO_clearInterruptStatus(GPIOB,
                KEY_KEY1_PIN | KEY_PB21_PIN | KEY_KEY4_PIN);
            break;
         case KEY_GPIOA_INT_IIDX :
        //如果按键按下变为高电平
            if( DL_GPIO_readPins(KEY_KEY2_PORT, KEY_KEY2_PIN) > 0 )
            {
                //设置LED引脚状态翻转
                DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
            }
            DL_GPIO_clearInterruptStatus(GPIOA, KEY_KEY2_PIN);
        break;

        default:
            // 非按键中断，直接退出
            break;
    }
}