#include "usart.h"


#define RE_0_BUFF_LEN_MAX	128

volatile uint8_t  recv0_buff[RE_0_BUFF_LEN_MAX] = {0};//将数组的全体元素初始化为 0
volatile uint16_t recv0_length = 0;//在中断函数里会改变的值,建议使用volatile,控制每次读取时会重原始内存重新读取
volatile uint8_t  recv0_flag = 0;

void USART_Init(void)
{
	//清除串口中断标志
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
	//使能串口中断
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

//串口发送一个字节
void USART_SendData(unsigned char data)
{
	while( DL_UART_isBusy(UART_0_INST) == true );
	DL_UART_Main_transmitData(UART_0_INST, data);
}
void USART_SendString(unsigned char *str)
{
    while( str && *str ) // 地址为不空 && 值不为空
    {
        USART_SendData(*str++);
    }
}

//重定向fputc函数
//Redirect fputc function
int uart_fputc(int ch, FILE *stream)
{
    while( DL_UART_isBusy(UART_0_INST) == true );
    DL_UART_Main_transmitData(UART_0_INST, ch);
    return ch;
}

//重定向fputs函数
//Redirect fputs function
int fputs(const char* restrict s, FILE* restrict stream) {

    uint16_t char_len=0;
    while(*s!=0)
    {
        while( DL_UART_isBusy(UART_0_INST) == true );
        DL_UART_Main_transmitData(UART_0_INST, *s++);
        char_len++;
    }
    return char_len;
}
int puts(const char* _ptr)
{
 return 0;
}

/*===================================================================
 *  VOFA+ JustFloat 协议
 *  每帧: N×float(4字节大端?) + 帧尾 0x00 0x00 0x80 0x7F
 *
 *  注意: ARM Cortex-M 是小端, float 在内存中就是小端字节序,
 *        VOFA+ JustFloat 也是小端, 所以直接按字节发送即可。
 *===================================================================*/
void VOFA_SendFrame(float *data, uint8_t count)
{
    uint8_t *p;
    for (uint8_t i = 0; i < count; i++) {
        p = (uint8_t *)&data[i];
        USART_SendData(p[0]);
        USART_SendData(p[1]);
        USART_SendData(p[2]);
        USART_SendData(p[3]);
    }
    /* JustFloat 帧尾 */
    USART_SendData(0x00);
    USART_SendData(0x00);
    USART_SendData(0x80);
    USART_SendData(0x7F);
}

void UART_0_INST_IRQHandler(void)
{
	uint8_t receivedData = 0;
	
	//如果产生了串口中断
	switch( DL_UART_getPendingInterrupt(UART_0_INST) )
	{
		case DL_UART_IIDX_RX://如果是接收中断			
			receivedData = DL_UART_Main_receiveData(UART_0_INST);
		// USART_SendData(receivedData);  /* 关闭回显，避免干扰 VOFA JustFloat */
			// 检查缓冲区是否已满	Check if the buffer is full
			if (recv0_length < RE_0_BUFF_LEN_MAX - 1)
			{
				recv0_buff[recv0_length++] = receivedData;
			}
			else
			{
				recv0_length = 0;
			}

			// 标记接收标志	
			recv0_flag = 1;
		
			break;
		
		default://其他的串口中断
			break;
	}
}
