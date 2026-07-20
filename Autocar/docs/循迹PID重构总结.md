# 循迹 PID 重构与调试总结

## 一、最终架构

```
主循环 while(1)
├── OLED_SHOW()          ← OLED 全屏渲染
├── line_follower_update() ← GPIO 直读 8 路灰度传感器 → s1~s8
├── CAR_CONTROL()
│   ├── 正常巡线: car_run(base_speed, line_folower(kp, ki, kd))
│   └── 丢线转弯: car_trun(left_flage ? 1 : -1)
└── 每 1s 串口: S值 + 左右轮速 + yaw
```

## 二、传感器模块切换

| 旧模块 | 新模块 |
|--------|--------|
| PCF8575 I2C 扩展 | GPIO 直读（AD0/AD1/AD2 选通道 + OUT 读值） |
| I2C 地址 `0x20`，需上拉 | 3 根地址线 + 1 根数据线，直接 GPIO |
| 读取 2 字节，取 bit7~0 | 循环选通道，逐个读 8 次，耗时 ~400μs |

**line_follower_update()**：
```c
void line_follower_update(void)
{
    uint8_t raw[8];
    Grayscale_Sensor_Read_All(raw);
    s1 = !raw[0]; s2 = !raw[1]; ... s8 = !raw[7];
}
```

s=1 白线，s=0 黑线（与原 PCF8575 版本语义一致）。

## 三、line_folower 重构：加权求和替代枚举

**旧版**：100 行 else-if 枚举每种探头组合 → error 在 ±7 之间，只能处理单一黑线。

**新版**：
```c
const int8_t weight[8] = {-7, -5, -3, -1, 1, 3, 5, 7};
// 遍历 8 路，黑线探头累加权重
for (i=0; i<8; i++)
    if (!data[i]) { sum_w += weight[i]; black_cnt++; }

error = sum_w / black_cnt;  // 加权平均，兼容多条黑线
```

| 新增特性 | 值 |
|----------|-----|
| 误差死区 | \|error\| < 2 → error=0 |
| 积分限幅 | ±100 |
| 输出限幅 | ±200 |

## 四、全白线丢线处理

```c
if (black_cnt == 0)  // 全白
{
    out_num++;
    if (out_num >= 3) trun_flage = true;  // 连续 3 次才触发
    integral = 0;
    return 0;
}
```

转弯方向由 `left_flage` 记录最后一次有黑线时线在左边还是右边。

## 五、已知问题：转弯时 PI 残留

**现象**：出了黑线后疯狂转动，不是慢转。

**根因**：增量式 PI 的 `ControlVelocity`（static 变量）在正常巡线时累积到 +200，触发转弯时 `car_run(0, 1)` 试图给 0 速度，但 PI 来不及衰减，仍输出 200 → 电机高速正转。

**待修复**：`car_trun` 需要绕开 PI 直接控制 PWM，或在丢线时先 `car_stop()` 清零 PI。

## 六、PID 参数调节

在 OLED 菜单 → PID：
- kp：比例系数（默认 7.0，转向不够加大到 15~30）
- ki：积分系数（默认 0.2）
- kd：微分系数（默认 0.5）
- rate：调节步长

## 七、串口输出格式

```
S:00111100 L:200 R:200 mm/s Y:12.3
  │        │     │        │
  传感器   左轮   右轮   偏航角
```
