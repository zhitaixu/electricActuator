#pragma once

// 你的实测标定点（样机）：
static const int ADC_AT_0DEG  = 3900;
static const int ADC_AT_93DEG = 2630;

// 逻辑上只认 0~90°
static const float LOGIC_MAX_DEG = 90.0f;
static const float PHYS_MAX_DEG  = 93.0f;

// 控制参数（先保守，能动再细调）
static const float DEG_DEADBAND = 0.6f;     // 误差小于此角度认为到位
static const float DUTY_MIN = 0.10f;
static const float DUTY_MAX = 0.35f;
static const float KP_DUTY_PER_DEG = 0.010f; // 每度误差给多少占空比（粗P）

// 若 Hall 长时间不跳变，用开环 kick
static const int HALL_STUCK_MS = 120;
static const int KICK_MS = 180;
static const float KICK_DUTY = 0.28f;
static const int DETECT_RETRY_MS = 600;
static const bool USE_POT_FEEDBACK = true;
static const bool HALL_USE_PULLUP = false;
static const uint8_t DEFAULT_HALL_MAP = 0; // Map 0 + Dir - = CCW (smooth)
static const int DEFAULT_DIR_SIGN = -1;     // -1 means angle increases in CCW
