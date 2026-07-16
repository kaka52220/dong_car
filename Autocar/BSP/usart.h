#ifndef	__USART_H__
#define __USART_H__

#include "ti_msp_dl_config.h"
#include "stdio.h"

void USART_Init(void);

void USART_SendData(unsigned char data);
void USART_SendString(unsigned char *str);

/* VOFA+ JustFloat 协议: 发送 float 数组作为波形数据 */
void VOFA_SendFrame(float *data, uint8_t count);

#endif
