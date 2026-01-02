#ifndef STAIR_GAME_H
#define STAIR_GAME_H

#include <Arduino.h>
#include <U8g2lib.h>


// 버튼 핀
#define BTN_TURN 2     // 방향 전환
#define BTN_CLIMB 4    // 오르기
#define BTN_BACK   5   // 메뉴로 돌아가기

// 방향
// enum StairDir {
//   STAIR_LEFT = 0,
//   STAIR_RIGHT = 1
// };


// 외부 OLED 객체 (공유)
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

// 함수
void stairInit();
void stairGame();

#endif
