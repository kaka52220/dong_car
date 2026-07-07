#ifndef __KEY_H__
#define __KEY_H__
#include "ti_msp_dl_config.h"

#define  KEY_FUNCtion 1 //设置为0 时,将完全关闭按键这个功能

/* ── 按键事件位掩码 ── */
#define KEY_EVENT_KEY1   (1 << 0)
#define KEY_EVENT_KEY2   (1 << 1)
#define KEY_EVENT_PB21   (1 << 2)
#define KEY_EVENT_KEY4   (1 << 3)

/* ISR 写入，主循环读出并清除 */
extern volatile uint8_t key_events;

void Key_Init(void);
void Key_Process(void);  /* 在主循环中调用 */

#endif