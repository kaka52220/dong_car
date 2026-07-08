/*
 *计时及延迟
 */
#ifndef _CLOCK_H_
#define _CLOCK_H_

#define delay_ms  mspm0_delay_ms

extern volatile unsigned long tick_ms;

int mspm0_delay_ms(unsigned long num_ms);
int mspm0_get_clock_ms(unsigned long *count);
void SysTick_Init(void);

#endif  /* #ifndef _CLOCK_H_ */