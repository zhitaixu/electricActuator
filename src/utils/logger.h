#pragma once
#include <Arduino.h>

#define LOGI(fmt, ...) do{ Serial.printf("[I] " fmt "\n", ##__VA_ARGS__); }while(0)
#define LOGW(fmt, ...) do{ Serial.printf("[W] " fmt "\n", ##__VA_ARGS__); }while(0)
#define LOGE(fmt, ...) do{ Serial.printf("[E] " fmt "\n", ##__VA_ARGS__); }while(0)