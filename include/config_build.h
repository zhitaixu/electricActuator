#pragma once

// 这玩意的用途：
// 1) 串口日志里打印固件版本（你刷错固件/刷旧固件，一眼识别）
// 2) Web 页面也显示版本，远程调试时不用猜“你板子里跑的是哪一版”
static const char* FW_NAME    = "electric-actuator";
static const char* FW_VERSION = "v2.0.0-hall-closedloop";