#ifndef PONG_GAME_H
#define PONG_GAME_H

#include <Arduino.h>
#include <U8g2lib.h>

// 외부에서 접근 가능한 함수
void pong();
void pongInit();

// U8g2 객체 (cpp에서 정의)
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

#endif
