# Electric Actuator (ESP32-S3 + DRV8313)

## 1. 项目概述

本项目是电动执行器样机的产品化原型，主控为 **ESP32-S3-WROOM-1-R8N8**，三相无刷电机驱动为 **TI DRV8313**。  
位置反馈使用 **电位器 ADC**，电机端集成 **三路 Hall(U/V/W)**，实现 **Hall 闭环换相**，并基于电位器角度闭环停位。

交互包括 **19 颗离散 LED（7 段数码 + 状态灯）**、**3 个按键（- / + / OK）**，并提供 **SoftAP + Web UI** 用于运行控制与 OTA 升级。

---

## 2. 运行功能（当前实现）

### 2.1 基本行为

- 机械量程：0°~93°（硬限位），业务显示/控制也以此为准  
- 1 分钟无操作熄屏，任意按键唤醒  
- 温度无传感器，固定显示 25  

### 2.2 电机控制链路

- **Hall 闭环换相**：Hall 6 个有效状态触发 commutation  
- **电位器闭环**：根据目标角度与当前角度误差，输出转速/占空比  
- **方向规则**：  
  - 逆时针（CCW）→ 角度增加  
  - 顺时针（CW）→ 角度减小  

---

## 3. Hall 顺序与映射（已验证顺滑）

顺时针手拨 Hall 序列（连续变化）：

- `001-011-010-110-100-101`

当前已验证可用方向映射：

- `Map 0 + Dir +` → 逆时针（顺滑）
- `Map 3 + Dir -` → 顺时针（顺滑）

默认配置：

- `DEFAULT_HALL_MAP=0`
- `DEFAULT_DIR_SIGN=+1`

运行时自动根据目标方向切换映射/方向，保证顺/逆时针都顺滑。

---

## 4. 按键与 LED 行为

### 4.1 按键

- **OK**：进入/确认 KVS 调整  
- **+ / -**：KVS 模式下增减开阀比例  
- 确认后开始向目标角度转动（KVS → 目标角度）

### 4.2 LED 逻辑

- **LED1**：RUN（运行中闪烁，熄灭表示未转动）  
- **LED2**：VALVE（开阀角度）  
- **LED3**：KVS（最大 KVS 设定）  
- **LED4**：TEMP（温度）  
- **LED5**：PCT（百分比）  

显示规则：

- 正常显示：LED2 与 LED4 轮显  
- KVS 调整：LED2 与 LED3 同步闪烁  
- 7 段数码：  
  - VALVE 页面显示当前角度  
  - TEMP 页面固定显示 25  
  - KVS 模式显示 KVS 百分比  

---

## 5. SoftAP + Web UI

### 5.1 SoftAP

- SSID：`test111111`  
- 密码：无  

### 5.2 运行页面（最终版）

Web UI 提供：

- 目标角度 / 速度 / 扭矩（占空比）设置  
- 关键状态（角度/目标/速度/扭矩/Hall/nFAULT 等）  
- LED 段码显示模拟  
- 按键模拟（- / + / OK）  
- 预设方向按钮用于调试  

### 5.3 OTA 升级（SoftAP 上传 bin）

1. 连接设备 SoftAP（SSID：`test111111`）  
2. 打开 Web UI，进入 **OTA Upgrade**  
3. 选择编译生成的 `firmware.bin` 上传  
4. 上传成功后设备自动重启并切换到新固件  

> 已启用 `partitions.csv`（包含 `ota_0/ota_1`）。

---

## 6. 硬件资源（关键 GPIO）

### DRV8313

- `IN1 = IO9`  
- `EN1(PWM) = IO10`  
- `IN2 = IO11`  
- `EN2(PWM) = IO12`  
- `IN3 = IO13`  
- `EN3(PWM) = IO14`  
- `nSLEEP = IO21`  
- `nRESET = IO47`  
- `nFAULT = IO42`（低=故障）  

### Hall

- `HALL_U = IO37`  
- `HALL_V = IO38`  
- `HALL_W = IO39`  
- 外部上拉 3.3V（不建议再开内部上拉）  

### 电位器 ADC

- `POT_ADC = IO20`  
- 标定参考：  
  - 0° ≈ 3900  
  - 93° ≈ 2630  

### 按键

- `KEY_MINUS = IO15`  
- `KEY_PLUS  = IO16`  
- `KEY_OK    = IO17`  

---

## 7. 工程目录结构（实际）

```text
(project root)
  platformio.ini
  partitions.csv
  /include
    config_pins.h
    config_calib.h
    config_build.h
    /drivers
      drv8313_hall.h
      hall.h
      pot_adc.h
      shift595.h
      keys.h

  /src
    main.cpp

    /drivers
      drv8313_hall.cpp
      hall.cpp
      pot_adc.cpp
      shift595.cpp
      keys.cpp

    /app
      control.cpp
      control.h
      ui_led.cpp
      ui_led.h

    /net
      ap_web.cpp
      ap_web.h
      web_assets.h

    /utils
      logger.h
      timebase.h
```

---

## 8. 控制原理（简述）

- **Hall 闭环换相**：Hall 6 个有效码触发 6-step commutation  
- **方向反转原理**：同一 Hall 序列下更换映射或反向相序，可实现反转  
- **位置闭环**：电位器角度作为反馈，达到目标角度后停止  
- **抖动本质**：Hall 顺序与换相表不匹配会导致卡顿/抖动/发热  

