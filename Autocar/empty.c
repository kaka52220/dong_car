/**
 * @file    empty.c
 * @brief   HC05 蓝牙驱动 v2.1 — 纯净测试 (无OLED)
 *
 *  硬件: MSPM0G3507 + HC05 (UART2) + LED1 (PB22)
 *
 *  命令 (手机蓝牙串口发送, 以 \n 结尾):
 *    ON  → LED 亮     OFF → LED 灭
 *    CLS → 清空缓冲    ?   → 状态查询
 *    其他 → 回声
 */

#include "ti_msp_dl_config.h"
#include "board.h"
#include "hc05.h"
#include <stdio.h>
#include <string.h>

/* ===================================================================
 *  状态
 * =================================================================== */
static volatile uint32_t g_rxCount   = 0;
static volatile uint32_t g_tickCount = 0;
static volatile uint8_t  g_connected = 0;

/* ===================================================================
 *  辅助
 * =================================================================== */
void delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 1000; j++) __asm("nop");
}

static void LED_Set(uint8_t on)
{
    if (on)
        DL_GPIO_setPins(LED1_PORT, LED1_PIN_22_PIN);
    else
        DL_GPIO_clearPins(LED1_PORT, LED1_PIN_22_PIN);
}

/**
 * @brief 打印一帧的原始十六进制 — 用于诊断波特率/编码问题
 */
static void DumpHex(const char *data, uint16_t len)
{
    printf("[HEX] ");
    for (uint16_t i = 0; i < len; i++) {
        printf("%02X ", (unsigned char)data[i]);
    }
    printf("  (len=%u)\r\n", (unsigned int)len);
}

/* ===================================================================
 *  回调
 * =================================================================== */
static void onConnectChange(uint8_t connected)
{
    g_connected = connected;
    printf("[BLE] >>> %s <<<\r\n", connected ? "CONNECTED" : "DISCONNECTED");

    if (connected) {
        /* 连接成功: LED 快闪 3 下 */
        for (int i = 0; i < 3; i++) {
            LED_Set(1); delay_ms(100);
            LED_Set(0); delay_ms(100);
        }
        BLE_Printf("HC05 v2.1 Ready");
    }
}

static void onFrame(const char *frame, uint16_t len)
{
    g_rxCount++;

    /* 打印原始 HEX + 文本 — 帮助诊断波特率/乱码问题 */
    DumpHex(frame, len);
    printf("[BLE RX #%lu] \"%s\"\r\n", (unsigned long)g_rxCount, frame);

    /* ---- 命令分发 ---- */
    if (strcmp(frame, "ON") == 0) {
        LED_Set(1);
        BLE_Printf("LED:ON");

    } else if (strcmp(frame, "OFF") == 0) {
        LED_Set(0);
        BLE_Printf("LED:OFF");

    } else if (strcmp(frame, "CLS") == 0) {
        BLE_FlushRx();
        BLE_Printf("RX Flushed");

    } else if (strcmp(frame, "?") == 0) {
        BLE_Printf("CONN:%s RX:%lu T:%lu",
                   g_connected ? "Y" : "N",
                   (unsigned long)g_rxCount,
                   (unsigned long)g_tickCount);

    } else if (len > 0) {
        /* 回声 — 用 BLE_SendByte 逐字节验证发送路径 */
        BLE_SendString("Echo: ");
        BLE_SendString(frame);
        BLE_SendByte('\r');
        BLE_SendByte('\n');
    }
}

/* ===================================================================
 *  main
 * =================================================================== */
int main(void)
{
    SYSCFG_DL_init();
    delay_ms(100);

    printf("\r\n");
    printf("========================================\r\n");
    printf("  HC05 v2.1 Test (no OLED)\r\n");
    printf("========================================\r\n");

    /* BLE 初始化 */
    BLE_Init();
    BLE_OnFrame(onFrame);
    BLE_OnConnectChange(onConnectChange);

    printf("[INIT] BLE_Init()           OK\r\n");
    printf("[INIT] BLE_IsConnected()    = %d\r\n", BLE_IsConnected());
    printf("[INIT] BLE_FrameAvailable() = %d\r\n", BLE_FrameAvailable());

    /* 启动消息 — 验证发送路径 */
    BLE_Printf("Boot OK");
    printf("[INIT] BLE_Printf() sent\r\n");
    printf("========================================\r\n");
    printf("  等待手机连接 (HC05 STATE=PA7)\r\n");
    printf("  命令: ON / OFF / CLS / ?\r\n");
    printf("========================================\r\n");

    uint16_t loopCnt = 0;

    while (1) {
        BLE_Poll();

        loopCnt++;
        if (loopCnt >= 100) {   /* ~2s */
            loopCnt = 0;
            g_tickCount++;

            /* LED: 已连=常亮, 未连=闪 */
            if (g_connected) {
                LED_Set(1);
            } else {
                LED_Set(g_tickCount & 1);
            }

            printf("[TICK] %04lu conn=%d rx=%lu\r\n",
                   (unsigned long)g_tickCount,
                   (int)g_connected,
                   (unsigned long)g_rxCount);

            /* 定时心跳 — 仅已连接时发, 验证 TX 路径 */
            if (g_connected) {
                BLE_Printf("T:%04lu", (unsigned long)g_tickCount);
            }
        }

        delay_ms(20);
    }
}
