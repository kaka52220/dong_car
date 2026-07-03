/**
 * @file    hc05.c
 * @brief   HC05 蓝牙驱动 v2.0 — 环形队列 + 硬件FIFO + 帧解析 + 回调
 *
 * 架构：
 *   UART RX ISR ──→ 环形队列 ──→ BLE_Poll() 帧解析 ──→ 用户回调
 *                    ↑ head              ↑ tail
 *
 * 丢包防护：
 *   - 环形队列 256B，单生产者(ISR)/单消费者(main)，无需锁
 *   - RX FIFO 阈值为 HALF_FULL，中断频率降至 1/4
 *   - ISR 内循环排空全部 FIFO
 *   - 队列满时覆盖最旧字节，优先保留最新数据
 */
#include "hc05.h"
#include <stdio.h>
#include <stdarg.h>

/* ===================================================================
 *  静态变量 — 全部模块内部，外部不可见
 * =================================================================== */
static BLE_RingBuf  g_rxRing;
static char         g_frameBuf[BLE_FRAME_MAX];
static uint16_t     g_frameLen;
static uint8_t      g_frameReady;

static BLE_RxCallback       g_rxCallback;
static BLE_ConnectCallback  g_connCallback;
static uint8_t              g_lastConnect;

/* 连接消抖: 连续 CONN_DEBOUNCE_MAX 次读到同一状态才确认变化 */
#define  CONN_DEBOUNCE_MAX   5
static int8_t  g_connDebounce = 0;

/* ===================================================================
 *  环形队列内联操作 (power-of-2 大小, 位掩码取模)
 * =================================================================== */
static inline uint16_t RB_Used(const BLE_RingBuf *rb)
{
    return (rb->head - rb->tail) & (BLE_RB_SIZE - 1);
}

static inline uint8_t RB_IsEmpty(const BLE_RingBuf *rb)
{
    return rb->head == rb->tail;
}

static uint8_t RB_Put(BLE_RingBuf *rb, uint8_t byte)
{
    uint16_t next = (rb->head + 1) & (BLE_RB_SIZE - 1);
    if (next == rb->tail) return 0;  /* 满 */
    rb->buffer[rb->head] = byte;
    rb->head = next;
    return 1;
}

static uint8_t RB_Get(BLE_RingBuf *rb, uint8_t *byte)
{
    if (rb->tail == rb->head) return 0;  /* 空 */
    *byte = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & (BLE_RB_SIZE - 1);
    return 1;
}

static void RB_Flush(BLE_RingBuf *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

/* ===================================================================
 *  发送 — 利用 TX FIFO 批量写入
 * =================================================================== */
void BLE_SendByte(uint8_t ch)
{
    while (DL_UART_isBusy(UART_2_INST) == true) {}
    DL_UART_Main_transmitData(UART_2_INST, ch);
}

void BLE_SendString(const char *str)
{
    if (!str) return;
    while (*str) BLE_SendByte((uint8_t)*str++);
}

void BLE_Printf(const char *fmt, ...)
{
    char buf[BLE_FRAME_MAX];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) BLE_SendString(buf);
    BLE_SendString("\r\n");
}

/* ===================================================================
 *  UART2 ISR — 批量排空 RX FIFO → 环形队列
 * =================================================================== */
void UART_2_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_2_INST)) {

    case DL_UART_IIDX_RX:
        /*
         * 逐个字节读取直到 RX FIFO 空。
         *
         * 关键: FIFO 阈值 = ONE_ENTRY (1字节),
         * getPendingInterrupt 在 FIFO ≥ 1 时返回 IIDX_RX,
         * 读到空后自动退出 — 不会像 ONE_HALF 那样残留字节。
         *
         * 9600bps 下最大 ~1000 字节/秒，逐个中断完全可承受。
         */
        while (DL_UART_getPendingInterrupt(UART_2_INST) == DL_UART_IIDX_RX) {
            uint8_t byte = DL_UART_Main_receiveData(UART_2_INST);
            if (!RB_Put(&g_rxRing, byte)) {
                uint8_t dummy;
                RB_Get(&g_rxRing, &dummy);  /* 丢弃最旧 */
                RB_Put(&g_rxRing, byte);
            }
        }
        break;

    default:
        break;
    }
}

/* ===================================================================
 *  帧解析 — 从环形队列提取 '\n' 分隔的完整帧
 * =================================================================== */
static uint8_t FrameExtract(void)
{
    if (g_frameReady) return 0;  /* 上一帧未取走 */

    while (!RB_IsEmpty(&g_rxRing)) {
        uint8_t ch;
        RB_Get(&g_rxRing, &ch);

        if (ch == BLE_FRAME_DELIMITER) {
            /* 剥离尾部 \r */
            if (g_frameLen > 0 && g_frameBuf[g_frameLen - 1] == '\r')
                g_frameLen--;
            g_frameBuf[g_frameLen] = '\0';
            g_frameReady = 1;
            return 1;
        }

        if (g_frameLen >= BLE_FRAME_MAX - 1) {
            g_frameLen = 0;  /* 帧过长, 丢弃等下一个 \n */
            continue;
        }
        g_frameBuf[g_frameLen++] = (char)ch;
    }
    return 0;
}

/* ===================================================================
 *  API 实现
 * =================================================================== */
void BLE_Init(void)
{
    RB_Flush(&g_rxRing);
    g_frameLen   = 0;
    g_frameReady = 0;
    g_lastConnect = 0;

    /* RX FIFO 阈值: ONE_ENTRY (1字节触发, 配合while循环排空FIFO) */
    DL_UART_Main_setRXFIFOThreshold(UART_2_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);

    NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
}

uint8_t BLE_IsConnected(void)
{
    return BLE_LINK_STATE() ? BLE_CONNECTED : BLE_DISCONNECTED;
}

void BLE_OnConnectChange(BLE_ConnectCallback cb) { g_connCallback = cb; }

void BLE_OnFrame(BLE_RxCallback cb) { g_rxCallback = cb; }

uint8_t BLE_FrameAvailable(void) { return g_frameReady; }

uint16_t BLE_ReadFrame(char *buf, uint16_t maxLen)
{
    if (!g_frameReady || !buf || maxLen == 0) return 0;

    uint16_t n = g_frameLen;
    if (n >= maxLen) n = maxLen - 1;
    for (uint16_t i = 0; i < n; i++) buf[i] = g_frameBuf[i];
    buf[n] = '\0';

    g_frameReady = 0;
    g_frameLen   = 0;
    return n;
}

void BLE_FlushRx(void)
{
    RB_Flush(&g_rxRing);
    g_frameLen   = 0;
    g_frameReady = 0;
}

/* ===================================================================
 *  BLE_Poll — 主循环必须调用
 * =================================================================== */
void BLE_Poll(void)
{
    /* 连接状态变化检测（带消抖） */
    uint8_t raw = BLE_IsConnected();
    if (raw) {
        if (g_connDebounce <  CONN_DEBOUNCE_MAX) g_connDebounce++;
    } else {
        if (g_connDebounce > -CONN_DEBOUNCE_MAX) g_connDebounce--;
    }
    /* 正向阈值 → 确认连接; 负向阈值 → 确认断开 */
    uint8_t stable = (g_connDebounce >=  CONN_DEBOUNCE_MAX) ? BLE_CONNECTED
                   : (g_connDebounce <= -CONN_DEBOUNCE_MAX) ? BLE_DISCONNECTED
                   : g_lastConnect;
    if (stable != g_lastConnect) {
        g_lastConnect = stable;
        if (g_connCallback) g_connCallback(stable);
    }

    /* 帧提取 → 回调 或 标记就绪 */
    while (FrameExtract()) {
        if (g_rxCallback) {
            g_rxCallback(g_frameBuf, g_frameLen);
            g_frameReady = 0;
            g_frameLen   = 0;
        }
    }
}
