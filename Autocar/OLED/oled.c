#include "oled.h"




uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_GPIO_I2C_DATA:
            OLED_SDIN_Write(arg_int); 
            break;
        case U8X8_MSG_GPIO_I2C_CLOCK:
            OLED_SCLK_Write(arg_int); 
            break;
        case U8X8_MSG_GPIO_SPI_DATA:
            OLED_SDIN_Write(arg_int); 
            break;
        case U8X8_MSG_GPIO_SPI_CLOCK:
            OLED_SCLK_Write(arg_int); 
            break;
        case U8X8_MSG_DELAY_MILLI:
            delay_ms(arg_int);
            break;
        // case U8X8_MSG_DELAY_10MICRO:
        //     delay_us(arg_int*10); 
        //     break;
        case U8X8_MSG_GPIO_CS:
            OLED_CS_Write(arg_int);
            break;
        case U8X8_MSG_GPIO_DC:
            OLED_DC_Write(arg_int);
            break;
        case U8X8_MSG_GPIO_RESET:
            OLED_RES_Write(arg_int);
            break;
    }
    return 1;
}

void GPIO_Init(void)
{
	OLED_SCLK_Clr();
	OLED_SDIN_Clr();
}
void u8g2Init(u8g2_t *u8g2)
{	
    //u8g2_Setup_ssd1306_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay);//spi
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);//开启显示
    u8g2_ClearDisplay(u8g2);//清空屏幕
    u8g2_ClearBuffer(u8g2);//清理缓存区
	GPIO_Init();
}