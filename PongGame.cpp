#include "PongGame.h"
#include <Arduino.h>
#include "GameState.h"

// ================= OLED =================
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

// ================= 버튼 핀 =================
#define L_UP     2
#define L_DOWN   5
#define R_UP     4
#define R_DOWN   7
#define BTN_BACK 3
#define RESET    6

// ================= 상수 =================
const unsigned long PADDLE_RATE = 64;
const uint8_t SCORE_LIMIT = 9;

const uint8_t MCU_X    = 12;
const uint8_t PLAYER_X = 115;

// ================= 게임 상태 =================
static bool game_over_state = false;

static uint8_t player_score = 0;
static uint8_t mcu_score = 0;

// ---- 랠리 ----
static uint16_t rally_count = 0;

// ---- 공 ----
static uint8_t ball_x = 53, ball_y = 26;
static int8_t  ball_dir_x = 1, ball_dir_y = 1;
static uint8_t ball_size;
static unsigned long ball_rate;

// ---- 패들 ----
static uint8_t paddle_height;
static uint8_t mcu_y = 16;
static uint8_t player_y = 16;

static unsigned long ball_update;
static unsigned long paddle_update;

// ================= 내부 함수 =================
static void drawCourt();
static void resetGame();
static void updateDifficultyByRally();

// ================= 초기화 =================
void pongInit()
{
  pinMode(L_UP, INPUT_PULLUP);
  pinMode(L_DOWN, INPUT_PULLUP);
  pinMode(R_UP, INPUT_PULLUP);
  pinMode(R_DOWN, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(RESET, INPUT_PULLUP);

  ball_update = millis();
  paddle_update = ball_update;
  resetGame();
}

// ================= 메인 =================
void pong()
{
  unsigned long time = millis();

  if (digitalRead(BTN_BACK) == LOW) {
    delay(200);
    appState = STATE_MENU;
    return;
  }

  if (digitalRead(RESET) == LOW) {
    delay(200);
    resetGame();
    return;
  }

  // ---------- BALL ----------
  if (time > ball_update && !game_over_state)
  {
    uint8_t new_x = ball_x + ball_dir_x;
    uint8_t new_y = ball_y + ball_dir_y;

    // 득점
    if (new_x <= 0 || new_x + ball_size >= 127)
    {
      if (new_x < 64) player_score++;
      else            mcu_score++;

      if (player_score == SCORE_LIMIT || mcu_score == SCORE_LIMIT)
        game_over_state = true;

      // 랠리 초기화
      rally_count = 0;

      ball_dir_x = -ball_dir_x;
    }

    // 상하 벽
    if (new_y <= 0 || new_y + ball_size >= 53)
      ball_dir_y = -ball_dir_y;

    // 왼쪽 패들
    if (new_x == MCU_X &&
        new_y + ball_size >= mcu_y &&
        new_y <= mcu_y + paddle_height)
    {
      ball_dir_x = -ball_dir_x;
      rally_count++;
      updateDifficultyByRally();
    }

    // 오른쪽 패들
    if (new_x + ball_size == PLAYER_X &&
        new_y + ball_size >= player_y &&
        new_y <= player_y + paddle_height)
    {
      ball_dir_x = -ball_dir_x;
      rally_count++;
      updateDifficultyByRally();
    }

    ball_x += ball_dir_x;
    ball_y += ball_dir_y;
    ball_update += ball_rate;
  }

  // ---------- PADDLE ----------
  if (time > paddle_update && !game_over_state)
  {
    paddle_update += PADDLE_RATE;

    if (digitalRead(L_UP) == LOW)   mcu_y--;
    if (digitalRead(L_DOWN) == LOW) mcu_y++;

    if (digitalRead(R_UP) == LOW)   player_y--;
    if (digitalRead(R_DOWN) == LOW) player_y++;

    if (mcu_y < 1) mcu_y = 1;
    if (mcu_y + paddle_height > 53)
      mcu_y = 53 - paddle_height;

    if (player_y < 1) player_y = 1;
    if (player_y + paddle_height > 53)
      player_y = 53 - paddle_height;
  }

  // ---------- DRAW ----------
  u8g2.firstPage();
  do {
    drawCourt();

    u8g2.drawBox(ball_x, ball_y, ball_size, ball_size);
    u8g2.drawVLine(MCU_X, mcu_y, paddle_height);
    u8g2.drawVLine(PLAYER_X, player_y, paddle_height);

    u8g2.setCursor(0, 64);
    u8g2.print(mcu_score);
    u8g2.setCursor(122, 64);
    u8g2.print(player_score);

    if (game_over_state)
    {
      const char* text = player_score > mcu_score ? "WIN -->" : "<-- WIN";
      u8g2.setCursor(45, 30);
      u8g2.print(text);
    }

  } while (u8g2.nextPage());
}

// ================= 보조 =================
static void drawCourt()
{
  u8g2.drawFrame(0, 0, 128, 54);
}

// 🔥 랠리 기반 난이도
static void updateDifficultyByRally()
{
  if (rally_count % 4 != 0) return;

  if (paddle_height > 3)
    paddle_height -= 2;

  if (ball_size > 1)
    ball_size--;

  if (ball_rate > 5)
    ball_rate--;
}

static void resetGame()
{
  ball_x = 53;
  ball_y = 26;
  ball_dir_x = 1;
  ball_dir_y = 1;

  ball_size = 4;
  ball_rate = 12;

  paddle_height = 10;

  mcu_y = 16;
  player_y = 16;

  rally_count = 0;

  mcu_score = 0;
  player_score = 0;
  game_over_state = false;
}
