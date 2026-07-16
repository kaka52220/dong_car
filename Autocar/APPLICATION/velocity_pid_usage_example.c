/**
 * @file    velocity_pid_usage_example.c
 * @brief   增量式PI速度环 — 集成示例 (展示怎样接入你的 main/ISR)
 *
 * 这是演示代码, 不是直接可用的。你需要根据实际情况调整。
 * 展示了三个关键修改点:
 *   1. main() 中的初始化
 *   2. 20ms 定时器 ISR 中的调用
 *   3. 主循环中的业务逻辑
 */

/*===================================================================
 *  第一步: 在 main() 中初始化
 *===================================================================

// --- 你当前的 empty.c ---
#include "velocity_pid.h"

int main(void)
{
    // 硬件初始化 (保持不变)
    MSPM0_Init();           // SysTick, TIMG0(编码器20ms), GPIO, PWM
    USART_Init();
    DL_GPIO_setPins(GPIO_MOTOR_STBY_PORT, GPIO_MOTOR_STBY_PIN);

    // === 新增: 初始化 PI 速度环 ===
    // Kp=1.5, Ki=0.1 是建议起步参数 (基于13线编码器+65mm轮径)
    // 后续可通过串口在线调整
    VelPID_InitAll(1.5f, 0.1f);

    USART_SendString((unsigned char*)"PI Speed Loop Ready\r\n");

    while (1)
    {
        // === 新增: 设置目标速度 (替换原来的 MOTOR_CONTROL) ===
        // 原来: MOTOR_CONTROL(20, 40, 60, 80);
        // 现在: 速度单位是 mm/s, 不是编码器脉冲数!
        //
        // 换算参考 (13线编码器, 65mm轮径, 20ms周期):
        //   目标 1 脉冲/周期 ≈ 785 mm/s (太快!)
        //   目标 100 mm/s ≈ 0.13 脉冲/周期
        //
        // 建议起步速度: 50~200 mm/s
        //   (50mm/s: 小车 1秒走5cm, 非常慢, 适合调试)
        //   (200mm/s: 小车 1秒走20cm, 中等速度)
        VelPID_SetAllTargets(100, 150, 150, 100);  // 前后各一组速度

        // === 原有的串口打印 (保持) ===
        // 打印速度 (mm/s) 和 PWM 占空比 (compare 值, 直接写入硬件)
        delay_ms(20);
        char buf[128];
        sprintf(buf,
            "A:%.0f B:%.0f C:%.0f D:%.0f | "
            "PWM:%d %d %d %d\r\n",
            VelPID_GetSpeed(MOTOR_A),
            VelPID_GetSpeed(MOTOR_B),
            VelPID_GetSpeed(MOTOR_C),
            VelPID_GetSpeed(MOTOR_D),
            VelPID_GetPWM(MOTOR_A),
            VelPID_GetPWM(MOTOR_B),
            VelPID_GetPWM(MOTOR_C),
            VelPID_GetPWM(MOTOR_D));
        USART_SendString((unsigned char*)buf);
    }
}

===================================================================
  第二步: 在 20ms 定时器 ISR 中调用 VelPID_UpdateAll
===================================================================

// --- 你当前的 interrupt.c: TIMER_TICK_INST_IRQHandler ---
//
// 原来这个 ISR (TIMG0, 20ms) 做的事情:
//   1. 把 temp_count 转移到 count
//   2. 判断方向
//   3. 清零 temp_count
//
// 现在在它后面加上 PI 更新:

void TIMER_TICK_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_TICK_INST))
    {
    case DL_TIMER_IIDX_ZERO:
        DL_TimerG_clearInterruptStatus(TIMER_TICK_INST, DL_TIMER_IIDX_ZERO);

        // ======== 你原有的编码器采样逻辑 ========
        // (temp_count → count, 判断方向, 清零 temp_count)
        // ... 保持不动 ...

        // ======== 新增: PI 速度环更新 ========
        // get_encoder_count() 返回的是本周期(20ms)的脉冲增量
        VelPID_UpdateAll(
            get_encoder_count('A'),  // A 电机本周期脉冲数
            get_encoder_count('B'),
            get_encoder_count('C'),
            get_encoder_count('D')
        );
        // 注意: VelPID_UpdateAll 内部调用了 MOTOR_PWM_SET 直接设置硬件PWM,
        //       所以不需要再在主循环中调用 MOTOR_CONTROL 了。
        //
        //       也就是说:
        //       原来的 MOTOR_CONTROL() 在 main() while(1) 中调用 →
        //       现在完全由 ISR 接管, main() 中只设 target, 不再调 MOTOR_CONTROL

        break;
    }
}

===================================================================
  第三步 (可选): 串口在线调参
===================================================================

// 在 while(1) 中加入简单的串口命令解析:
//
// 命令格式 (通过串口助手发送):
//   kp 2.0        → 全部电机 Kp = 2.0
//   ki 0.15       → 全部电机 Ki = 0.15
//   speed 100 120 130 110  → 设置 A/B/C/D 目标速度
//   stop          → 急停
//
// 示例实现:

void SerialCommandProcess(void)
{
    static char cmd_buf[64];
    static uint8_t idx = 0;

    // ... 接收串口字节到 cmd_buf, 遇到 \n 解析 ...

    float v;
    if (sscanf(cmd_buf, "kp %f", &v) == 1) {
        VelPID_SetParams(MOTOR_NUM, v, motors[0].ki);
    }
    else if (sscanf(cmd_buf, "ki %f", &v) == 1) {
        VelPID_SetParams(MOTOR_NUM, motors[0].kp, v);
    }
    else if (sscanf(cmd_buf, "speed %f %f %f %f", &a,&b,&c,&d) == 4) {
        VelPID_SetAllTargets(a, b, c, d);
    }
    else if (strncmp(cmd_buf, "stop", 4) == 0) {
        VelPID_EmergencyStop();
    }
}

===================================================================
  PI 参数调参指南
===================================================================

13 线编码器 + 65mm 轮径的特点:
  - 每圈仅 13 个脉冲, 速度信号比较"稀疏"
  - 低速时 (20ms 内 < 1 个脉冲) 速度检测有量化误差
  - 建议目标速度 ≥ 30 mm/s, 保证每周期有足够脉冲

调参步骤:
  1. 先用小速度 (50~80 mm/s) 测试
  2. Kp 从 1.0 开始, 逐步增大到出现抖动 → 回调 30%
  3. Ki 从 0.05 开始, 逐步增大到能快速消除静差
  4. 观察串口打印的 PWM 值:
     - 正常范围: 200~600 (对应占空比 20%~60%)
     - 如果 PWM 一直 999: 目标速度超出电机能力, 降低目标或检查机械
     - 如果 PWM 一直很小 (<100) 但速度正常: Kp 偏大, 可适当减小

  速度 vs PWM 对应关系 (实测估计, 以你之前数据为准):
    速度(mm/s) ≈ PWM × 0.09  (每点PWM对应约0.09mm/s)
    反向: 要达到 speed mm/s, 需要 PWM ≈ speed / 0.09

  例: 目标 100mm/s → 预估 PWM ≈ 100/0.09 ≈ 1111 > 999
      → 100mm/s 超出电机能力,PWM上限999对应最高速度约90mm/s

  建议可达到的目标速度范围: 10 ~ 80 mm/s
  (对应编码器 0.01~0.10 脉冲/周期, 非常稀疏但要靠滤波平滑)

===================================================================*/
