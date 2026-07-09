#include "ti_msp_dl_config.h"
#include "clock.h"

volatile unsigned long tick_ms;
volatile uint32_t start_time;

int mspm0_delay_ms(unsigned long num_ms)
{
    start_time = tick_ms;//start = 100
    while (tick_ms - start_time < num_ms);//110 -100 = 10 < 10不成立 ,退出,表示delay10ms
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)//读取当前的 tick_ms 值，通过指针参数传出来。
{
    if (!count)
        return 1;
    count[0] = tick_ms;
    return 0;
}

void SysTick_Init(void)
{
    DL_SYSTICK_config(CPUCLK_FREQ/1000);//引发SysTick_Handler 
    NVIC_SetPriority(SysTick_IRQn, 0);
    
    NVIC_ClearPendingIRQ(TIMER_TICK_INST_INT_IRQN);//引发TIMER_TICK_INST_IRQHandler 编码器中断
	NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);//TIMG0
	DL_TimerG_startCounter(TIMER_TICK_INST);
}


