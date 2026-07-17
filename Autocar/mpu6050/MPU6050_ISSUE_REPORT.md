# MPU6050 Yaw 角度偏小问题 — 排查报告

> **状态：未解决** | 最后更新：2026-07-16 | 联系人：上一轮 AI

---

## 一、问题概述

MPU6050 陀螺仪的 Z 轴（Yaw）角度累积偏小。手动旋转小车后，OLED 和串口显示的 yaw 角度小于实际旋转角度。

| 测试动作 | 实际角度 | 显示 yaw | 比例 |
|---------|---------|----------|------|
| 旋转 180° | 180° | ~90° | ~50% |
| 旋转 90°（早期） | 90° | ~44° | ~49% |
| 旋转 90°（绕过service_process后） | 90° | ~67° | ~74% |

**核心结论：LSB 换算和芯片量程已确认正确（raw 峰值可达 80°/s），问题在积分环节。**

---

## 二、代码架构

### 文件结构

```
Autocar/
├── empty.c                          # 主循环，包含 DEBUG 串口输出
├── OLED/
│   └── draw.c                       # OLED_SHOW()，分页渲染间调用 mpu6050_update()
└── mpu6050/
    ├── mpu6050.h                    # 公共接口 + Mpu6050Data 结构体
    ├── mpu6050.c                    # 核心驱动：初始化、校准、读传感器、Mahony 融合、yaw 积分
    ├── mpu6050_service.h            # 高层服务声明
    ├── mpu6050_service.c            # 高层服务：节拍控制、初始化、重试
    └── MPU6050_ISSUE_REPORT.md      # ← 本文件
```

### 数据流

```
MPU6050 硬件
  │  I2C (I2C1, addr 0x68)
  ▼
read_raw()                              [mpu6050.c:256]
  │  读取 14 字节: ACCEL(6) + TEMP(2) + GYRO(6)
  │  be16() 大端转小端
  │  raw / MPU6050_GYRO_LSB_PER_DPS (65.5f) → gyro_dps (°/s)
  ▼
mpu6050_update()                        [mpu6050.c:555]
  │  dt = now - last_update_ms，限幅 4~50ms（超出钳制为 10ms）
  │  gyro_dps = raw - bias
  │  stationary 检测（基于 gyro/accel 阈值）
  │  静止时：跟踪 bias，gyro_dps 强制清零
  ├─► fusion_update()                  [mpu6050.c:394]
  │     Mahony AHRS 滤波器 (Kp=1.8, Ki=0.035)
  │     用加速度计修正 roll/pitch，yaw 无绝对参考（无磁力计）
  │     四元数积分 → g_filter.q0~q3
  └─► update_euler_and_yaw()           [mpu6050.c:457]
        从四元数提取 roll/pitch
        yaw: 将 gyro 向量投影到重力方向后积分
          vertical_rate_dps = (gyro · accel) / |accel|
          yaw_deg += vertical_rate_dps * dt
        静止或角速度 < 0.1°/s 时清零 vertical_rate_dps
```

### 调用链（当前状态）

```
main() while(1):
  ├─ OLED_SHOW()                          [draw.c:360]
  │    u8g2 分 8 页渲染，每页间调用一次 mpu6050_update()
  │    每页约 15ms，dt ≈ 15ms
  │
  ├─ for i=0..3:                          [empty.c:190]
  │    mpu6050_update()                   ← 已绕过 service_process
  │    CAR_CONTROL()
  │
  └─ DEBUG 串口输出（每 500ms）
       raw / gz / yaw / iyaw / bias
```

### 关键配置

```c
// mpu6050.h
#define MPU6050_SAMPLE_RATE_HZ   (200U)       // 目标采样率
#define MPU6050_SAMPLE_PERIOD_MS (5)          // 目标采样周期

// mpu6050.c
#define MPU6050_GYRO_LSB_PER_DPS (65.5f)      // ±500°/s 量程 LSB
#define MPU6050_MAHONY_KP        (1.8f)       // Mahony 比例增益
#define MPU6050_MAHONY_KI        (0.035f)     // Mahony 积分增益
#define MPU6050_YAW_RATE_DEADBAND_DPS (0.10f) // yaw 死区

// GYRO_CONFIG 寄存器写入值: 0x08 → FS_SEL=1 → ±500°/s
```

---

## 三、排查历程

### 阶段 1：怀疑时序/限幅问题
- **现象**：转 180° 显示 -90.5°，恰好一半
- **排查**：OLED 渲染一帧约 150ms，dt 可能超过 50ms 被钳制为 10ms
- **改动**：把主循环从「OLED 和 MPU 串行」改为「OLED 渲染中穿插 MPU 采样 + 后续补 4 轮」
- **结果**：无改善

### 阶段 2：怀疑量程/FS_SEL 不匹配
- **现象**：精确的 2:1 比例
- **假设**：芯片 FS_SEL 可能 3 位宽 vs 代码假定的 2 位宽，导致实际量程 ±1000°/s 但 LSB 用 ±500°/s
- **改动**：加了 DEBUG 打印 raw（LSB 换算后、bias 修正前的原始值）
- **结果**：raw 峰值达 **80°/s**，说明 LSB 换算正确，**排除了量程问题**

### 阶段 3：怀疑 service_process 节拍逻辑
- **关键发现**：`mpu6050_service_process()` 内部有 `g_next_sample_ms += 5` 的节拍控制和 `__WFI()` 忙等，和 OLED 的渲染节奏不同步
- **改动**：所有调用点绕过 service_process，直接用 `mpu6050_update()`
  - `empty.c:192` — 主循环补采样
  - `draw.c:371` — OLED 分页渲染间
- **结果**：转 90° 从 ~44° 提升到 ~67°，有改善但仍不准

### 阶段 4：加独立积分器（当前）
- **改动**：在 DEBUG 中加了 `iyaw`（独立简单积分器：直接用 gz × 实际 500ms 间隔累加）
- **目的**：区分问题是出在 `gyro_dps` 本身 还是 `update_euler_and_yaw` 的投影/滤波逻辑
  - `iyaw` ≈ `yaw` → 问题在传感器数据
  - `iyaw` ≈ 90 但 `yaw` ≈ 67 → 问题在 `update_euler_and_yaw()`

### 其他发现
- **bias 始终为 0**：可能校准时小车未完全静止。但静止时 raw=0，零偏本就小，影响有限
- **OLED 和 MPU6050 共用 I2C 总线**：MPU 用 I2C1，OLED 也用 I2C（通过 u8g2），可能存在总线冲突风险

---

## 四、当前 DEBUG 输出格式

串口每 500ms 输出一行（波特率见工程配置）：

```
raw=80 gz=80 yaw=-29 iyaw=-30 bias=0
```

| 字段 | 含义 | 来源 |
|------|------|------|
| `raw` | LSB换算后、bias修正前的角速度 (°/s) | `gyro_dps + gyro_bias_dps` |
| `gz` | bias修正后的 Z 轴角速度 (°/s) | `Mpu6050Data.gyro_dps[2]` |
| `yaw` | 驱动累积航向 (°) | `Mpu6050Data.yaw_deg` |
| `iyaw` | 独立简单积分器 (°)，gz × 500ms 累加 | 在 empty.c DEBUG 块中自行维护 |
| `bias` | 当前 Z 轴零偏 (°/s) | `Mpu6050Data.gyro_bias_dps[2]` |

另有 `00000000` 行是灰度传感器数据（每秒一次），可忽略。

---

## 五、下一步排查方向（建议优先级）

### 🔴 P0：等 iyaw 数据
编译当前代码，旋转 90°，对比 `iyaw` 和 `yaw`。这将直接定位问题层。

### 🟠 P1：如果 iyaw ≈ yaw（都是 ~67°）
问题在 **gyro_dps 本身**。可能原因：
1. **dt 被频繁钳制**：在 `mpu6050_update()` 中打印或累计 dt，看实际值
2. **stationary 误判**：转动过程中 stationary 检测是否误触发，导致 gyro_dps 短期清零
3. **Mahony 滤波器影响**：`fusion_update` 中的 `error_z` 修正项是否系统性偏置
4. **采样丢失**：OLED 渲染期间实际采样次数是否足够

### 🟡 P2：如果 iyaw ≈ 90 但 yaw ≈ 67
问题在 `update_euler_and_yaw()`：
1. **vertical_rate_dps 投影**：小车倾斜时投影损失
2. **死区判断**：`MPU6050_YAW_RATE_DEADBAND_DPS = 0.10f` 是否过宽
3. **accel_norm 阈值**：`if (accel_norm > 0.5f)` 是否在转动时短暂失效

### 🟢 P3：终极方案
如果以上都排除了，在 `mpu6050_update()` 末尾直接做最简单的积分：
```c
g_data.yaw_deg += g_data.gyro_dps[2] * dt;  // 绕过所有投影和死区
```
对比结果，确认是否是 projection 逻辑的问题。

---

## 六、关键代码位置速查

| 位置 | 内容 |
|------|------|
| `mpu6050.c:26` | `MPU6050_GYRO_LSB_PER_DPS` (65.5f) |
| `mpu6050.c:256-279` | `read_raw()` — 原始传感器读取 |
| `mpu6050.c:301` | `GYRO_CONFIG = 0x08` (FS_SEL=1) |
| `mpu6050.c:394-455` | `fusion_update()` — Mahony AHRS |
| `mpu6050.c:457-492` | `update_euler_and_yaw()` — yaw 积分 |
| `mpu6050.c:555-651` | `mpu6050_update()` — 采样主函数 |
| `mpu6050.c:577-582` | dt 限幅逻辑 (4~50ms，超出→10ms) |
| `mpu6050_service.c:113-136` | `mpu6050_service_process()` — 节拍控制（**目前已绕过**） |
| `draw.c:360-373` | `OLED_SHOW()` — 分页渲染 + MPU 采样 |
| `empty.c:148-196` | main 主循环 + DEBUG 输出 |

---

## 七、已排除的假设

- ❌ 量程/FS_SEL 不匹配（raw 峰值 80 证实 LSB 正确）
- ❌ service_process 节拍逻辑（绕过后有改善但未完全解决）
- ❌ bias 校准（bias=0，静止时 raw=0，问题不大）

