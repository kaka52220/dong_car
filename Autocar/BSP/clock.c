#include "ti_msp_dl_config.h"
#include "clock.h"

volatile unsigned long tick_ms;
volatile uint32_t start_time;

void delay_us(unsigned long __us) 
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 38;//38为32Mhz下的补偿值

	// Calculate the number of clocks required = delay microseconds * number of clocks per microsecond
    ticks = __us * (80000000 / 1000000);//系统时钟频率为 80 MHz

	// Get the current SysTick value
    told = SysTick->VAL;//SysTick 定时器的当前递减计数值（从 LOAD 开始向下计数到 0，然后重载）。

    while (1)
    {
		// Repeatedly refresh to get the current SysTick value
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else//：发生溢出（计数值从 0 跳变到 LOAD）
                tcnt += SysTick->LOAD - tnow + told;

            told = tnow;

			// If the required number of clocks is reached, exit the loop
            if (tcnt >= ticks)
                break;
        }
    }
}

int mspm0_delay_ms(unsigned long num_ms)
{
    start_time = tick_ms;//start = 100
    while (tick_ms - start_time < num_ms);//110 -100 = 10 < 10不成��? ,退��?,表示delay10ms
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)//读取当前��? tick_ms 值，通过指针参数传出来��?
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
    
    NVIC_ClearPendingIRQ(TIMER_TICK_INST_INT_IRQN);//引发TIMER_TICK_INST_IRQHandler 编码器中��?
	NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);//TIMG0
	DL_TimerG_startCounter(TIMER_TICK_INST);
}


