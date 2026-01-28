#pragma once
#include <Arduino.h>

/* ===== DRV8313 ===== */
static const int PIN_IN1    = 9;
static const int PIN_EN1    = 10;   // EN (enable)
static const int PIN_IN2    = 11;
static const int PIN_EN2    = 12;   // EN (enable)
static const int PIN_IN3    = 13;
static const int PIN_EN3    = 14;   // EN (enable)
static const int PIN_NSLEEP = 21;
static const int PIN_NRESET = 47;
static const int PIN_NFAULT = 42;

/* ===== Hall ===== */
static const int PIN_HALL_U = 37;
static const int PIN_HALL_V = 38;
static const int PIN_HALL_W = 39;

/* ===== Pot ADC ===== */
static const int PIN_POT_ADC = 20;

/* ===== Keys (外部上拉到3.3V，按下接GND) ===== */
static const int PIN_KEY_MINUS = 15;
static const int PIN_KEY_PLUS  = 16;
static const int PIN_KEY_OK    = 17;

/* ===== 74HC595 =====
   你定稿：IO4=SRCLK, IO5=RCLK, IO6=SER, IO7=OE
*/
static const int PIN_595_SRCLK = 4;
static const int PIN_595_RCLK  = 5;
static const int PIN_595_SER   = 6;
static const int PIN_595_OE    = 7;   // 74HC595 OE 为低使能
