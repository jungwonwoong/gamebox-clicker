#include <U8g2lib.h>
#include <Arduino.h>
#include "PongGame.h"
#include "SnakeGame.h"
#include "StairGame.h"
#include "GameState.h"
#include "Buttons.h"
//#include "InfiniteStairs.h"

// ================= OLED =================
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

volatile AppState appState = STATE_MENU;
int menuIndex = 0;
const char menu_0[] PROGMEM = "PONG";
const char menu_1[] PROGMEM = "SNAKE";
const char menu_2[] PROGMEM = "STAIRS";

const char* const menuItems[] PROGMEM = {
  menu_0, menu_1, menu_2
};
const uint8_t MENU_COUNT = 3;

// ================= SETUP =================
void setup()
{
  pinMode(BTN_UP,     INPUT_PULLUP);
  pinMode(BTN_DOWN,   INPUT_PULLUP);
  pinMode(BTN_LEFT,   INPUT_PULLUP);
  pinMode(BTN_RIGHT,  INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK,   INPUT_PULLUP);
  randomSeed(analogRead(A7));
  u8g2.begin();
  u8g2.setFlipMode(2);
  u8g2.setFont(u8g2_font_6x12_tf);
}

// ================= LOOP =================
void loop()
{
  switch (appState)
  {
    case STATE_MENU:
      menuLoop();
      break;

    case STATE_PONG:
      pong();
      break;

    case STATE_SNAKE:
      snake();
      break;

    case STATE_STAIRS:
      stairGame();
      break;
  }


}
void menuLoop()
{
  static unsigned long lastInput = 0;
  if (millis() - lastInput < 150) return;

  if (digitalRead(BTN_UP) == LOW)
  {
    
    menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT;
    lastInput = millis();
  }

  if (digitalRead(BTN_DOWN) == LOW)
  {
    
    menuIndex = (menuIndex + 1) % MENU_COUNT;
    lastInput = millis();
  }

  if (digitalRead(BTN_SELECT) == LOW)
  {
    
    if (menuIndex == 0) {pongInit();appState = STATE_PONG;}
    if (menuIndex == 1) {snakeInit();appState = STATE_SNAKE;}
    if (menuIndex == 2) {stairInit();appState = STATE_STAIRS;}
    delay(200);
  }

  drawMenu();
}

void drawMenu()
{
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setCursor(36, 12);
    u8g2.print("GAME MENU");

    for (uint8_t i = 0; i < MENU_COUNT; i++)
    {
      u8g2.setCursor(40, 30 + i * 14);
      if (i == menuIndex)
        u8g2.print("> ");
      else
        u8g2.print("  ");
char buffer[10];
strcpy_P(buffer, (char*)pgm_read_word(&menuItems[i]));
      u8g2.print(buffer);
    }
  } while (u8g2.nextPage());
}

