#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

// ================== FEATURES ====================
#define ENABLE_WIFI             1
#define ENABLE_SERVER           1

#define ENABLE_DISPLAY          1
#define ENABLE_UI               1

#define ENABLE_ESPNOW           1

#define ENABLE_WAKE_LIGHT       1
#define ENABLE_WAKE_COFFEE      0
#define ENABLE_WAKE_MUSIC       1
#define ENABLE_WAKE_BACKUP      0

// ================ USER SELECT ====================

#define USER_IS_LEO 0
#define USER_IS_JANEK 0
#define USER_IS_DAVE 1

// ================ USER wake_music raspberry pi name ====================

#if USER_IS_LEO // zero-one
#define RPI_NAME "http://polywaker-music.local:3141"
#elif USER_IS_JANEK
#define RPI_NAME "http://nice-pi:3141/"
#elif USER_IS_DAVE
#define RPI_NAME "http://pi.local:3141"
#endif

// ================ USER wake_light MAC addresses ====================

#if USER_IS_LEO
uint8_t MAC_ADDRESS_LIGHT[6] = {0x08, 0x3A, 0x8D, 0xCC, 0xC1, 0x0A}; // "led-base-02 for leo "
#elif USER_IS_JANEK
uint8_t MAC_ADDRESS_LIGHT[6] = {0x08, 0x3A, 0x8D, 0xD1, 0xB5, 0x00}; // 08:3a:8d:d1:b5:00 "halo light "
#elif USER_IS_DAVE
uint8_t MAC_ADDRESS_LIGHT[6] = {0x08, 0x3A, 0x8D, 0xCC, 0x5A, 0xAE}; // "polywaker-lightbader-04"
#endif

// ================ USER wake_coffee MAC addresses ====================

uint8_t MAC_ADDRESS_COFFEE[6] = {0x08, 0x3A, 0x8D, 0xD1, 0xA7, 0x2A}; // polywaker-coffee-01

// ================ Development ===================

#define ENABLE_DEBUG_PRINT_TASK_TIMINGS 0
#define ENABLE_AUTO_START 0
#define SERIAL_BAUD_RATE 115200

// ================ PINOUT ========================

#define ROTARY_ENCODER_A_PIN        33
#define ROTARY_ENCODER_B_PIN        14
#define ROTARY_ENCODER_BUTTON_PIN   13
#define BUTTON_ARCADE               26

#define PIN_DISPLAY_SDA             21
#define PIN_DISPLAY_SCL             22

#define PIN_I2S_LRCLK   15
#define PIN_I2S_BCK     16
#define PIN_I2S_DOUT    17

#define PIN_I2S_MCLK    0 // not used


#endif // CONFIG_H
