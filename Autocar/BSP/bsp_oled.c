/**
 * @file    bsp_oled.c
 * @brief   SSD1306 0.96" OLED 128x64 I2C driver implementation
 *
 *          I2C format: [Control Byte] + [Data Bytes]
 *            Control Byte: 0x00 = command, 0x40 = display data
 *
 *          Pages: 0-7 (each = 8 vertical pixels)
 *          Columns: 0-127
 */

#include "bsp_oled.h"
#include "bsp_sw_i2c.h"
#include "bsp_oled_font.h"

/*===========================================================================
 *  Internal Constants
 *===========================================================================*/

/** SSD1306 I2C 7-bit address (0x3C << 1 = 0x78) */
#define OLED_ADDR           0x78

/** Control bytes */
#define OLED_CMD_BYTE       0x00    /* Next byte(s) are commands */
#define OLED_DATA_BYTE      0x40    /* Next byte(s) are display data */

/*===========================================================================
 *  Static Module State
 *===========================================================================*/

static uint8_t oled_curX   = 0;     /* Current column (0-127) */
static uint8_t oled_curPage = 0;    /* Current page (0-7) */

/*===========================================================================
 *  Low-level I2C Write Helpers
 *===========================================================================*/

/**
 * @brief   Write a command byte to OLED
 */
static void OLED_WriteCmd(uint8_t cmd)
{
    BSP_SW_I2C_Start();
    BSP_SW_I2C_SendByte(OLED_ADDR);
    BSP_SW_I2C_SendByte(OLED_CMD_BYTE);
    BSP_SW_I2C_SendByte(cmd);
    BSP_SW_I2C_Stop();
}

/**
 * @brief   Write a single data byte to OLED
 */
static void OLED_WriteData(uint8_t data)
{
    BSP_SW_I2C_Start();
    BSP_SW_I2C_SendByte(OLED_ADDR);
    BSP_SW_I2C_SendByte(OLED_DATA_BYTE);
    BSP_SW_I2C_SendByte(data);
    BSP_SW_I2C_Stop();
}

/**
 * @brief   Write multiple data bytes to OLED (burst mode)
 * @param   buf   Data buffer
 * @param   len   Number of bytes
 */
static void OLED_WriteMultiData(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    BSP_SW_I2C_Start();
    BSP_SW_I2C_SendByte(OLED_ADDR);
    BSP_SW_I2C_SendByte(OLED_DATA_BYTE);

    for (i = 0; i < len; i++) {
        BSP_SW_I2C_SendByte(buf[i]);
    }

    BSP_SW_I2C_Stop();
}

/*===========================================================================
 *  SSD1306 Initialization Sequence
 *===========================================================================*/

/**
 * @brief   Full SSD1306 init sequence for 128x64 OLED
 *
 *          Based on SSD1306 datasheet recommended init flow.
 *          Charge pump enabled for 3.3V operation.
 */
void BSP_OLED_Init(void)
{
    /* 1. Initialize I2C bus first */
    BSP_SW_I2C_Init();

    /* 2. Display off during configuration */
    OLED_WriteCmd(0xAE);    /* Display OFF */

    /* 3. Clock / oscillator */
    OLED_WriteCmd(0xD5);    /* Set display clock divide ratio / oscillator */
    OLED_WriteCmd(0x80);    /* Default: divide=1, freq=8 */

    /* 4. Multiplex ratio */
    OLED_WriteCmd(0xA8);    /* Set multiplex ratio */
    OLED_WriteCmd(0x3F);    /* 64 (0x3F + 1 = 64 rows) */

    /* 5. Display offset */
    OLED_WriteCmd(0xD3);    /* Set display offset */
    OLED_WriteCmd(0x00);    /* 0 */

    /* 6. Display start line */
    OLED_WriteCmd(0x40);    /* Start line 0 */

    /* 7. Charge pump (required for 3.3V) */
    OLED_WriteCmd(0x8D);    /* Charge pump setting */
    OLED_WriteCmd(0x14);    /* Enable charge pump */

    /* 8. Memory addressing mode */
    OLED_WriteCmd(0x20);    /* Set memory addressing mode */
    OLED_WriteCmd(0x00);    /* Horizontal addressing mode */

    /* 9. Segment remap */
    OLED_WriteCmd(0xA0);    /* Column 0 mapped to SEG0 (A0) or SEG127 (A1) */
    OLED_WriteCmd(0xA1);    /* Column 127 = SEG0 (flip horizontal) — set to A0 for normal */

    /* 10. COM scan direction */
    OLED_WriteCmd(0xC8);    /* COM scan from COM[N-1] to COM0 (flip vertical) */
                            /* C0 = normal, C8 = flipped */

    /* 11. COM pins hardware configuration */
    OLED_WriteCmd(0xDA);    /* Set COM pins */
    OLED_WriteCmd(0x12);    /* Alternative COM pin config, disable remap */

    /* 12. Contrast */
    OLED_WriteCmd(0x81);    /* Set contrast */
    OLED_WriteCmd(0xCF);    /* Default = 0x7F, max = 0xFF */

    /* 13. Pre-charge period */
    OLED_WriteCmd(0xD9);    /* Set pre-charge period */
    OLED_WriteCmd(0xF1);    /* Phase 1: 1 DCLK, Phase 2: 15 DCLKs */

    /* 14. VCOMH deselect level */
    OLED_WriteCmd(0xDB);    /* Set VCOMH deselect level */
    OLED_WriteCmd(0x40);    /* ~0.77 x VCC */

    /* 15. Entire display ON (follow RAM content) */
    OLED_WriteCmd(0xA4);    /* Normal display (A5 = entire display ON) */

    /* 16. Normal / Inverse */
    OLED_WriteCmd(0xA6);    /* Normal (A7 = inverse) */

    /* 17. Scroll disable */
    OLED_WriteCmd(0x2E);    /* Deactivate scroll */

    /* 18. Clear display */
    BSP_OLED_Clear();

    /* 19. Display ON */
    OLED_WriteCmd(0xAF);    /* Display ON */
}

/*===========================================================================
 *  Basic Display Control
 *===========================================================================*/

void BSP_OLED_DisplayOn(void)
{
    OLED_WriteCmd(0xAF);
}

void BSP_OLED_DisplayOff(void)
{
    OLED_WriteCmd(0xAE);
}

void BSP_OLED_SetContrast(uint8_t contrast)
{
    OLED_WriteCmd(0x81);
    OLED_WriteCmd(contrast);
}

/*===========================================================================
 *  Clear / Fill
 *===========================================================================*/

void BSP_OLED_Clear(void)
{
    uint8_t page, col;

    for (page = 0; page < OLED_PAGES; page++) {
        BSP_OLED_SetPos(0, page);
        BSP_SW_I2C_Start();
        BSP_SW_I2C_SendByte(OLED_ADDR);
        BSP_SW_I2C_SendByte(OLED_DATA_BYTE);
        for (col = 0; col < OLED_WIDTH; col++) {
            BSP_SW_I2C_SendByte(0x00);
        }
        BSP_SW_I2C_Stop();
    }

    oled_curX    = 0;
    oled_curPage = 0;
}

void BSP_OLED_Fill(uint8_t pattern)
{
    uint8_t page, col;

    for (page = 0; page < OLED_PAGES; page++) {
        BSP_OLED_SetPos(0, page);
        BSP_SW_I2C_Start();
        BSP_SW_I2C_SendByte(OLED_ADDR);
        BSP_SW_I2C_SendByte(OLED_DATA_BYTE);
        for (col = 0; col < OLED_WIDTH; col++) {
            BSP_SW_I2C_SendByte(pattern);
        }
        BSP_SW_I2C_Stop();
    }

    oled_curX    = 0;
    oled_curPage = 0;
}

/*===========================================================================
 *  Cursor & Data Output
 *===========================================================================*/

void BSP_OLED_SetPos(uint8_t x, uint8_t page)
{
    if (x >= OLED_WIDTH)  x = 0;
    if (page >= OLED_PAGES) page = 0;

    oled_curX    = x;
    oled_curPage = page;

    OLED_WriteCmd(0xB0 + page);               /* Set page address */
    OLED_WriteCmd(0x00 + (x & 0x0F));          /* Set lower column */
    OLED_WriteCmd(0x10 + ((x >> 4) & 0x0F));   /* Set higher column */
}

void BSP_OLED_WriteByte(uint8_t data)
{
    OLED_WriteData(data);
    oled_curX++;
}

/*===========================================================================
 *  Framebuffer Refresh
 *===========================================================================*/

void BSP_OLED_Refresh(const uint8_t *buf)
{
    uint8_t page;

    for (page = 0; page < OLED_PAGES; page++) {
        BSP_OLED_SetPos(0, page);
        OLED_WriteMultiData(buf + (page * OLED_WIDTH), OLED_WIDTH);
    }
}

/*===========================================================================
 *  Character Display
 *===========================================================================*/

void BSP_OLED_ShowChar(uint8_t ch)
{
    uint8_t i;

    /* Handle newline */
    if (ch == '\n') {
        oled_curPage++;
        if (oled_curPage >= OLED_PAGES) {
            oled_curPage = 0;
        }
        oled_curX = 0;
        BSP_OLED_SetPos(oled_curX, oled_curPage);
        return;
    }

    /* Filter non-printable characters */
    if (ch < FONT_START || ch > FONT_END) {
        ch = ' ';  /* Replace with space */
    }

    /* Auto-wrap: if text would overflow, advance to next line */
    if ((oled_curX + FONT_WIDTH) > OLED_WIDTH) {
        oled_curPage++;
        oled_curX = 0;
        if (oled_curPage >= OLED_PAGES) {
            oled_curPage = 0;
        }
        BSP_OLED_SetPos(oled_curX, oled_curPage);
    }

    /* Send font columns via burst I2C */
    BSP_SW_I2C_Start();
    BSP_SW_I2C_SendByte(OLED_ADDR);
    BSP_SW_I2C_SendByte(OLED_DATA_BYTE);

    for (i = 0; i < FONT_WIDTH; i++) {
        BSP_SW_I2C_SendByte(OLED_Font6x8[ch - FONT_START][i]);
    }

    BSP_SW_I2C_Stop();

    oled_curX += FONT_WIDTH;
}

void BSP_OLED_ShowString(const char *str)
{
    while (*str) {
        BSP_OLED_ShowChar((uint8_t)*str);
        str++;
    }
}

/*===========================================================================
 *  Number Display
 *===========================================================================*/

/**
 * @brief   Display an unsigned 32-bit integer in decimal
 */
void BSP_OLED_ShowUNum(uint32_t num, uint8_t len)
{
    char buf[11];   /* Max 10 digits + null */
    uint8_t digits, i;

    /* Convert to string (right-aligned) */
    digits = 0;
    do {
        buf[digits++] = '0' + (num % 10);
        num /= 10;
    } while (num > 0 && digits < 10);

    /* Pad with spaces */
    for (i = digits; i < len && i < 10; i++) {
        buf[i] = ' ';
    }
    if (i > digits) digits = i;

    /* Print in reverse */
    for (i = digits; i > 0; i--) {
        BSP_OLED_ShowChar(buf[i - 1]);
    }
}

/**
 * @brief   Display a signed 32-bit integer
 */
void BSP_OLED_ShowNum(int32_t num, uint8_t len)
{
    if (num < 0) {
        BSP_OLED_ShowChar('-');
        BSP_OLED_ShowUNum((uint32_t)(-num), len - 1);
    } else {
        BSP_OLED_ShowUNum((uint32_t)num, len);
    }
}

/**
 * @brief   Display a hexadecimal number
 */
void BSP_OLED_ShowHex(uint32_t num, uint8_t len)
{
    uint8_t i, nibble;

    if (len > 8) len = 8;

    for (i = 0; i < len; i++) {
        nibble = (num >> ((len - 1 - i) * 4)) & 0x0F;
        if (nibble < 10) {
            BSP_OLED_ShowChar('0' + nibble);
        } else {
            BSP_OLED_ShowChar('A' + nibble - 10);
        }
    }
}

/**
 * @brief   Display a binary number
 */
void BSP_OLED_ShowBin(uint32_t num, uint8_t len)
{
    uint8_t i;

    if (len > 32) len = 32;

    for (i = 0; i < len; i++) {
        if (num & (1 << (len - 1 - i))) {
            BSP_OLED_ShowChar('1');
        } else {
            BSP_OLED_ShowChar('0');
        }
    }
}

/*===========================================================================
 *  Graphics Primitives
 *===========================================================================*/

void BSP_OLED_SetPixel(uint8_t *buf, uint8_t x, uint8_t y, uint8_t color)
{
    uint8_t page;
    uint16_t idx;

    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;

    page = y / 8;
    idx  = page * OLED_WIDTH + x;

    if (color) {
        buf[idx] |=  (1 << (y % 8));
    } else {
        buf[idx] &= ~(1 << (y % 8));
    }
}

void BSP_OLED_DrawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    uint8_t col, page;
    uint8_t startPage, endPage;
    uint8_t rowStart, rowEnd;
    uint8_t mask, r;

    /* Clamp to display bounds */
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    if (x + width  > OLED_WIDTH)  width  = OLED_WIDTH  - x;
    if (y + height > OLED_HEIGHT) height = OLED_HEIGHT - y;
    if (width == 0 || height == 0) return;

    startPage = y / 8;
    endPage   = (y + height - 1) / 8;

    for (page = startPage; page <= endPage && page < OLED_PAGES; page++) {
        /* Determine which rows within this page are affected */
        rowStart = (page == startPage) ? (y % 8) : 0;
        rowEnd   = (page == endPage)   ? ((y + height - 1) % 8) : 7;

        /* Build bitmask for rows in this page */
        mask = 0;
        for (r = rowStart; r <= rowEnd; r++) {
            mask |= (1 << r);
        }

        BSP_OLED_SetPos(x, page);
        BSP_SW_I2C_Start();
        BSP_SW_I2C_SendByte(OLED_ADDR);
        BSP_SW_I2C_SendByte(OLED_DATA_BYTE);
        for (col = 0; col < width; col++) {
            if (color) {
                BSP_SW_I2C_SendByte(mask);
            } else {
                BSP_SW_I2C_SendByte(0x00);
            }
        }
        BSP_SW_I2C_Stop();
    }
}

void BSP_OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    uint8_t page;

    for (page = 0; page < height; page++) {
        BSP_OLED_SetPos(x, y + page);
        OLED_WriteMultiData(bitmap + (page * width), width);
    }
}
