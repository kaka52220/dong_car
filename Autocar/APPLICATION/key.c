#include "key.h"
#include "hc05.h"

volatile uint8_t key_events = 0;

void Key_Init(void)
{
    #if KEY_FUNCtion
        NVIC_EnableIRQ(GPIOB_INT_IRQn);
        NVIC_EnableIRQ(GPIOA_INT_IRQn);
    #endif
}

/* ── ISR：只标记端口事件，不读引脚电平（避免短脉冲丢失）── */
void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1))
    {
        case KEY_GPIOB_INT_IIDX:
            /* 不再读电平 → 边沿 = 一定触发，用后处理区分具体按键 */
            key_events |= (KEY_EVENT_KEY1 | KEY_EVENT_PB21 | KEY_EVENT_KEY4);

            DL_GPIO_clearInterruptStatus(GPIOB,
                KEY_KEY1_PIN | KEY_PB21_PIN | KEY_KEY4_PIN);
            break;

        case KEY_GPIOA_INT_IIDX:
            key_events |= KEY_EVENT_KEY2;

            DL_GPIO_clearInterruptStatus(GPIOA, KEY_KEY2_PIN);
            break;

        default:
            break;
    }
}

/* ── 主循环调用：读电平区分按键 + 执行动作 ── */
void Key_Process(void)
{
    uint8_t events;

    /* 临界区保护：防止 ISR 在读写之间插入 */
    __disable_irq();
    events = key_events;
    key_events = 0;
    __enable_irq();

    if (events == 0)
        return;

    /* ─── 诊断模式：每个按键发不同蓝牙消息 ─── */
    if (events & KEY_EVENT_KEY1)
    {
        BLE_send_String((uint8_t *)"[KEY1] PB18\r\n");
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
    }
    if (events & KEY_EVENT_KEY2)
    {
        BLE_send_String((uint8_t *)"[KEY2] PA13\r\n");
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
    }
    if (events & KEY_EVENT_PB21)
    {
        BLE_send_String((uint8_t *)"[KEY3] PB21\r\n");
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
    }
    if (events & KEY_EVENT_KEY4)
    {
        BLE_send_String((uint8_t *)"[KEY4] PB1\r\n");
        DL_GPIO_togglePins(LED1_PORT, LED1_PIN_22_PIN);
    }
}
