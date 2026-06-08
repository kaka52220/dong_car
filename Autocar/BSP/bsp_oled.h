/**
 * @file    bsp_oled.h
 * @brief   SSD1306 0.96" OLED 128x64 I2C display driver interface
 *
 *          I2C address: 0x3C (7-bit) → write = 0x78
 *          Resolution:  128 x 64 pixels
 *          Driver IC:   SSD1306
 */

#ifndef BSP_OLED_H
#define BSP_OLED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/*  Display Constants                                                         */
/* -------------------------------------------------------------------------- */

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8           /* 64 / 8 = 8 pages (0-7) */

/* -------------------------------------------------------------------------- */
/*  Public Functions                                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief   Initialize OLED: hardware init sequence + clear display
 * @note    Calls BSP_SW_I2C_Init() internally on first call.
 *          Must be called once before any other display function.
 */
void BSP_OLED_Init(void);

/**
 * @brief   Clear entire display (fill with 0x00)
 */
void BSP_OLED_Clear(void);

/**
 * @brief   Fill entire display with a byte pattern
 * @param   pattern  Byte to fill (0x00 = blank, 0xFF = all on)
 */
void BSP_OLED_Fill(uint8_t pattern);

/**
 * @brief   Update full framebuffer to OLED (128x64 = 1024 bytes)
 * @param   buf  Pointer to 1024-byte buffer (8 pages x 128 columns)
 * @note    Buffer layout: buf[page * 128 + col]
 *          Page 0 = top 8 rows, Page 7 = bottom 8 rows
 */
void BSP_OLED_Refresh(const uint8_t *buf);

/**
 * @brief   Set cursor position for text / pixel drawing
 * @param   x     Column (0-127)
 * @param   page  Page (0-7), each page = 8 pixel rows
 */
void BSP_OLED_SetPos(uint8_t x, uint8_t page);

/**
 * @brief   Draw one byte (8 vertical pixels) at current position, auto-advance
 * @param   data  Byte to draw (LSB = top pixel)
 */
void BSP_OLED_WriteByte(uint8_t data);

/**
 * @brief   Display a single character at current cursor
 * @param   ch  ASCII character to display
 * @note    Uses built-in 6x8 font. Automatically wraps to next line.
 *          Call BSP_OLED_SetPos() first to set starting position.
 */
void BSP_OLED_ShowChar(uint8_t ch);

/**
 * @brief   Display a null-terminated string
 * @param   str  String to display (ASCII)
 * @note    Wraps to next line automatically.
 *          Newline ('\n') advances to the next page.
 */
void BSP_OLED_ShowString(const char *str);

/**
 * @brief   Display a signed integer at current cursor
 * @param   num  Number to display (-2147483648 ~ 2147483647)
 * @param   len  Minimum field width (padded with spaces on left)
 */
void BSP_OLED_ShowNum(int32_t num, uint8_t len);

/**
 * @brief   Display an unsigned integer at current cursor
 * @param   num  Number to display (0 ~ 4294967295)
 * @param   len  Minimum field width (padded with spaces on left)
 */
void BSP_OLED_ShowUNum(uint32_t num, uint8_t len);

/**
 * @brief   Display a hexadecimal number at current cursor
 * @param   num  Number to display
 * @param   len  Number of hex digits (1-8)
 */
void BSP_OLED_ShowHex(uint32_t num, uint8_t len);

/**
 * @brief   Display a binary number at current cursor
 * @param   num  Number to display
 * @param   len  Number of binary digits (1-32)
 */
void BSP_OLED_ShowBin(uint32_t num, uint8_t len);

/**
 * @brief   Draw a filled rectangle
 * @param   x       Start column (0-127)
 * @param   y       Start row (0-63)
 * @param   width   Width in pixels
 * @param   height  Height in pixels
 * @param   color   0 = black (clear), 1 = white (set)
 */
void BSP_OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief   Set a single pixel in the 1024-byte framebuffer
 * @param   buf     Pointer to framebuffer (1024 bytes)
 * @param   x       Column (0-127)
 * @param   y       Row (0-63)
 * @param   color   0 = clear, 1 = set
 */
void BSP_OLED_SetPixel(uint8_t *buf, uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief   Display a bitmap image
 * @param   x       Start column
 * @param   y       Start page (0-7)
 * @param   width   Bitmap width in pixels
 * @param   height  Bitmap height in pages (each page = 8 rows)
 * @param   bitmap  Pointer to bitmap data
 */
void BSP_OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

/**
 * @brief   Turn display ON
 */
void BSP_OLED_DisplayOn(void);

/**
 * @brief   Turn display OFF (sleep mode)
 */
void BSP_OLED_DisplayOff(void);

/**
 * @brief   Set contrast level
 * @param   contrast  0 (min) ~ 255 (max)
 */
void BSP_OLED_SetContrast(uint8_t contrast);

#ifdef __cplusplus
}
#endif

#endif /* BSP_OLED_H */
