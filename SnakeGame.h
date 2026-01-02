#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Arduino.h>
#include <U8g2lib.h>

// 방향 정의 (충돌 방지)
#define SNAKE_RIGHT 0
#define SNAKE_UP    1
#define SNAKE_LEFT  2
#define SNAKE_DOWN  3

// 외부 OLED 공유
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

// 공개 함수
void snakeInit();
void snake();

#endif
