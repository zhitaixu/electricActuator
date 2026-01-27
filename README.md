# Electric Actuator (ESP32-S3 + DRV8313) — README v2

## 1. 项目简介

本项目为电动执行器样机/产品化原型，主控为 **ESP32-S3-WROOM-1-R8N8**，三相无刷电机驱动为 **TI DRV8313PWPR**。
位置反馈使用 **电位器 ADC**，电机端集成 **三路霍尔 Hall(U/V/W)**，用于 **Hall 闭环换相**（相比纯开环更稳定、起步成功率更高、抖动更少）。
交互包括 **19 颗离散 LED（断码屏风格）**、**3 个按键（- / + / OK）**，并提供 **SoftAP + Web UI** 用于调试与控制。

---

## 2. 产品功能

### 2.1 基本行为

* 机械量程：0°–93°（物理硬限位），**实际业务按 0°–90°理解/显示/控制**
* 休眠策略：无操作 1 分钟息屏；息屏时任意按键唤醒显示
* 按键逻辑（规划）

  1. 唤醒后按 **OK** 进入 KVS 调整（KVS 图标闪烁）
  2. 按 **- / +** 递减/递增
  3. 再按 **OK** 确认保存（样机阶段可只保存到 RAM；产品化阶段保存到 NVS）

### 2.2 电机控制（从样机到产品化的路线）

* 阶段 0：开环 6-step（用于硬件联调、自检、验证相序/线序）
* **阶段 1：Hall 闭环换相（推荐写入产品功能）**

  * Hall(U/V/W) 解析为 6 个有效状态（001/010/011/100/101/110）
  * Hall 状态变化触发换相（commutation），显著提升低速稳定性
  * 启动策略可采用“开环推一下 → 捕获 Hall → 切闭环”
* 阶段 2：角度闭环（产品化目标）

  * Hall 负责“怎么转得顺”，电位器负责“转到哪里停”
  * 目标角度 → 误差 → 输出转速/占空比 → Hall 闭环换相执行
  * 接近 93°硬限位区降速/停机保护

---

## 3. 硬件资源

### 3.1 供电与下载

* 外部可调电源输入（12V 侧为主功率供电）
* 板上 DC-DC：12V → 3.3V（MCU/逻辑）
* UART0 烧录

### 3.2 DRV8313（三相驱动）GPIO

* `IN1 = IO9`
* `EN1(PWM) = IO10`
* `IN2 = IO11`
* `EN2(PWM) = IO12`
* `IN3 = IO13`
* `EN3(PWM) = IO14`
* `nSLEEP = IO21`
* `nRESET = IO47`
* `nFAULT = IO42`（低=故障）

### 3.3 Hall（三路）GPIO

* `HALL_U = IO37`
* `HALL_V = IO38`
* `HALL_W = IO39`
* Hall 供电 3.3V，上拉（外部上拉即可，避免再开内部上拉造成不必要静态电流）

### 3.4 电位器 ADC

* `POT_ADC = IO20`
* 电位器两端：3.3V / GND（不分压到 1V）
* 样机标定参考：

  * 0° ≈ **3900**
  * 93° ≈ **2630**
* 推荐抗噪电路：

  * 滑臂串联 1k–4.7k（推荐 2.2k）
  * ADC 对地 100nF（低通滤波）
  * 走线远离 U/V/W 与功率回流

### 3.5 按键 GPIO（外部上拉到 3.3V）

* `KEY_MINUS = IO15`
* `KEY_PLUS  = IO16`
* `KEY_OK    = IO17`

> 外部上拉时，软件侧不建议启用内部上拉；逻辑不会冲突，但会增加静态电流，且排查时容易混乱。

---

## 4. 相序与相线映射（已验证可转）

这是“能不能转”的核心真源。已确认可用组合如下：

* `phase_map = 1-2-3（可用）`
* `invert = 0（可用）`

驱动通道与相定义：

* `ph=1 → (IN1, EN1) → DRV 通道1 → OUT1`
* `ph=2 → (IN2, EN2) → DRV 通道2 → OUT2`
* `ph=3 → (IN3, EN3) → DRV 通道3 → OUT3`

电机三相线对应：

* **U = phase1**
* **V = phase2**
* **W = phase3**

> 如果后续改板/改线导致不转：第一优先检查这张映射表是否被破坏；第二才去调软件参数。

---

## 5. LED 显示系统

### 5.1 19 颗 LED 的逻辑含义

* **LED1~LED5：状态图标**

  * LED1：RUN（运行中）
  * LED2：VALVE（开阀角度）
  * LED3：KVS（最大KVS设定）
  * LED4：TEMP（温度）
  * LED5：PCT（百分比）
* **LED6~LED12：左侧七段**
* **LED13~LED19：右侧七段**

### 5.2 74HC595 + ULN2803（最终映射）

说明：每片 595 只用 QB~QH（QA 不用），且为布线做了倒序映射。

* **595#1：图标 5 颗（LED1~LED5）**

  * 从 `QF → QB` 递增对应 `LED1 → LED5`
  * `QF=LED1, QE=LED2, QD=LED3, QC=LED4, QB=LED5`

* **595#2：左侧七段（LED6~LED12）**

  * 从 `QH → QB` 递增对应 `LED6 → LED12`
  * `QH=LED6, QG=LED7, QF=LED8, QE=LED9, QD=LED10, QC=LED11, QB=LED12`

* **595#3：右侧七段（LED13~LED19）**

  * 从 `QH → QB` 递增对应 `LED13 → LED19`
  * `QH=LED13, QG=LED14, QF=LED15, QE=LED16, QD=LED17, QC=LED18, QB=LED19`

### 5.3 七段段位定义（用于 0~9 段码表）

左七段（LED6~12）段位布局：

* 顶：LED6
* 左上/右上：LED7 / LED8
* 中：LED9
* 左下/右下：LED10 / LED11
* 底：LED12

右七段（LED13~19）同理：

* 顶：LED13
* 左上/右上：LED14 / LED15
* 中：LED16
* 左下/右下：LED17 / LED18
* 底：LED19

---

## 6. SoftAP + Web UI

* SoftAP：

  * SSID：`test111111`
  * 密码：无
* Web 页面（规划/实现方向）：

  * 状态：角度（ADC/deg）、Hall 状态、nFAULT、运行模式
  * 控制：Enable/Disable、电机转动、占空比/速度、目标角度
  * 标定：记录 0°/93°、保存、恢复默认
  * 调试：日志、实时传感器刷新、故障提示

---

## 7. 标定数据持久化与固件升级不丢

* 默认值写在 `config_calib.h`（第一次启动就可运行）
* 若 NVS 中存在有效标定值，则优先使用 NVS
* 写入策略：

  * 仅在用户“确认保存”时写一次（避免频繁写）
  * 写后校验（范围/CRC）
* 升级不丢的前提：

  * 升级仅覆盖 App 分区，不做整片 Flash erase
  * NVS 位于独立分区

---

## 8. 软件工程目录结构（规划）

```text
/firmware
  /include
    config_pins.h        # 所有GPIO定义（唯一真源）
    config_calib.h       # 标定参数：ADC->角度、限位、滤波参数（含默认值）
    config_build.h       # 版本号/编译信息（日志/网页显示/追溯固件）

  /src
    main.cpp             # 启动、调度、状态机入口

    /drivers
      drv8313_6step.cpp  # DRV8313驱动（开环 + Hall闭环接口）
      drv8313_6step.h
      hall.cpp           # 霍尔采样/去抖/状态解析（输出 hall_state）
      hall.h
      pot_adc.cpp        # ADC采样+滤波+角度映射
      pot_adc.h
      shift595.cpp       # 74HC595底层（shift out + latch + OE）
      shift595.h
      keys.cpp           # 按键扫描+去抖+事件输出
      keys.h

    /app
      state_machine.cpp  # 状态机：休眠/显示/设置/运行/故障
      state_machine.h
      ui_led.cpp         # 逻辑UI -> LED位图（含段码表）
      ui_led.h
      control.cpp        # 控制策略：开环/闭环换相/角度闭环（逐步演进）
      control.h

    /net
      ap_web.cpp         # SoftAP启动、HTTP路由
      ap_web.h
      web_assets.h       # 内嵌HTML/CSS/JS（样机阶段内嵌最省事）

    /utils
      logger.cpp         # 统一日志（带时间戳/模块名/级别）
      timebase.cpp       # 统一时间基与节拍（避免 delay 堵塞）
```

---

## 9. 下一步建议（你要“做成完整产品”必做的三件事）

1. **Hall 闭环换相落地**：把当前可转的开环逻辑变成“Hall 触发换相 + 启动捕获”
2. **角度闭环**：用电位器闭环“停到目标角度”，并对 90°/93°区做保护
3. **UI 规则固化**：把“显示什么、什么时候闪、什么时候息屏”写成状态机，不要散落在代码里

---

如果你确认 README v2 的结构没问题，下一步我建议你别急着“全功能齐活”，先选一个最小闭环产品路径：
**Web UI 设目标角度 → Hall 闭环换相转动 → 电位器闭环停住 → RUN 图标亮**。
这条链路跑通，你这个“产品”就已经成立了，剩下只是加细节而不是赌命排雷。
