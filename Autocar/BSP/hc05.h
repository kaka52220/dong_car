#ifndef __HC05_H__
#define __HC05_H__

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

/* ===================================================================
 *  HC05 蓝牙驱动 v2.0 — 环形队列 + 硬件FIFO + 帧解析 + 回调
 *
 *  使用方式（二选一）：
 *
 *  【回调模式】推荐
 *    void onFrame(const char *data, uint16_t len) { ... }
 *    BLE_Init();
 *    BLE_OnFrame(onFrame);
 *    while(1) { BLE_Poll(); }
 *
 *  【轮询模式】
 *    BLE_Init();
 *    while(1) {
 *        BLE_Poll();
 *        if (BLE_FrameAvailable()) {
 *            char buf[128];
 *            BLE_ReadFrame(buf, sizeof(buf));
 *            ...
 *        }
 *    }
 * =================================================================== */

/* ==================== 配置常量 ==================== */

/* 环形队列大小 — 必须是 2 的幂 (256 / 512 / 1024) */
#define BLE_RB_SIZE          256

/* 单帧最大长度（含 '\0'） */
#define BLE_FRAME_MAX        128

/* 帧分隔符 */
#define BLE_FRAME_DELIMITER  '\n'

/* 连接状态宏 */
#define BLE_LINK_STATE() \
    ((DL_GPIO_readPins(HC05_PORT, HC05_STATE_PIN) & HC05_STATE_PIN) ? 1 : 0)

#define BLE_CONNECTED        1
#define BLE_DISCONNECTED     0

/* ==================== 环形队列 ==================== */

typedef struct {
    uint8_t          buffer[BLE_RB_SIZE];
    volatile uint16_t head;       /* ISR 写入位置 */
    volatile uint16_t tail;       /* 主循环读取位置 */
} BLE_RingBuf;

/* ==================== 回调类型 ==================== */

/**
 * @brief 帧接收回调
 * @param frame  指向帧数据（已 '\0' 结尾，不含 \r\n）
 * @param len    帧长度（不含 '\0'）
 */
typedef void (*BLE_RxCallback)(const char *frame, uint16_t len);

/**
 * @brief 蓝牙连接状态变化回调
 * @param connected  1=已连接, 0=已断开
 */
typedef void (*BLE_ConnectCallback)(uint8_t connected);

/* ==================== API ==================== */

/* ---------- 初始化 ---------- */

/**
 * @brief 初始化蓝牙模块
 * @note  会覆写 UART RX FIFO 阈值为 HALF_FULL，
 *        使能 UART2 中断，初始化环形队列。
 *        必须在 SYSCFG_DL_init() 之后调用。
 */
void BLE_Init(void);

/* ---------- 连接状态 ---------- */

/** @return BLE_CONNECTED 或 BLE_DISCONNECTED */
uint8_t BLE_IsConnected(void);

/**
 * @brief 注册连接状态变化回调
 * @note  状态变化时在 BLE_Poll() 中触发
 */
void BLE_OnConnectChange(BLE_ConnectCallback cb);

/* ---------- 发送 ---------- */

/** @brief 发送单个字节 */
void BLE_SendByte(uint8_t ch);

/** @brief 发送字符串（不自动加换行） */
void BLE_SendString(const char *str);

/**
 * @brief 格式化发送（自动追加 \r\n）
 * @note  单次最大 128 字节，超出截断
 */
void BLE_Printf(const char *fmt, ...);

/* ---------- 接收（回调模式） ---------- */

/**
 * @brief 注册帧接收回调
 * @note  收到完整帧（以 \n 分隔）时，在 BLE_Poll() 中触发。
 *        帧数据已剥离 \r\n，以 '\0' 结尾。
 */
void BLE_OnFrame(BLE_RxCallback cb);

/* ---------- 接收（轮询模式） ---------- */

/** @return 是否有完整帧等待读取 */
uint8_t BLE_FrameAvailable(void);

/**
 * @brief 读取一帧
 * @param buf     输出缓冲区
 * @param maxLen  缓冲区大小
 * @return 实际读取的字节数（不含 '\0'）；0 表示没有帧
 */
uint16_t BLE_ReadFrame(char *buf, uint16_t maxLen);

/** @brief 丢弃接收缓冲中的所有数据 */
void BLE_FlushRx(void);

/* ---------- 主循环处理 ---------- */

/**
 * @brief 主循环处理函数 — 必须周期性调用
 * @note  负责：帧解析 → 回调分发 → 连接状态检测
 *         建议每 1~5ms 调用一次
 */
void BLE_Poll(void);

/* ==================== 兼容旧 API（可选） ==================== */

#define Bluetooth_Init()         BLE_Init()
#define Get_Bluetooth_ConnectFlag() BLE_IsConnected()
#define BLE_send_String(s)       BLE_SendString((const char *)(s))

#endif /* __HC05_H__ */
