#include "SnakeGame.h"
#include <Arduino.h>
#include "GameState.h"
// ================= OLED (외부 공유) =================
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

// ================= 버튼 핀 =================
#define BTN_UP     5
#define BTN_DOWN   3
#define BTN_LEFT   2
#define BTN_RIGHT  4
#define BTN_BACK 7

// ================= 게임 상수 =================
static const uint8_t CELL_SIZE = 3;
static const uint8_t MAX_LEN   = 100;
static const uint8_t GRID_W    = 42;   // 128 / 4 ≈ 31
static const uint8_t GRID_H    = 21;   // 64 / 4 ≈ 19

// ================= 게임 상태 =================
static int snake_x[MAX_LEN];
static int snake_y[MAX_LEN];
static int snake_length;

static int food_x, food_y;
static int direction;

static unsigned long lastUpdate;
static unsigned int game_speed;

// ================= 내부 함수 선언 =================
static void spawnFood();
static void readDirection();
static void moveSnake();
static bool hitWall();
static bool eatFood();
static void drawGame();

// ================= 초기화 =================
void snakeInit()
{
  snake_length = 3;

  snake_x[0] = 15; snake_y[0] = 10;
  snake_x[1] = 14; snake_y[1] = 10;
  snake_x[2] = 13; snake_y[2] = 10;

  direction = SNAKE_RIGHT;
  game_speed = 180;
  lastUpdate = millis();

  randomSeed(analogRead(A6));
  spawnFood();
}

// ================= 메인 프레임 =================
void snake()
{
  readDirection();
  if (digitalRead(BTN_BACK) == LOW) {
    delay(200);          // 디바운스
    appState = STATE_MENU;
    return;              // snake 즉시 종료
  }
  if (millis() - lastUpdate < game_speed)
    return;

  lastUpdate = millis();
  moveSnake();

  if (hitWall())
  {
    snakeInit();   // 충돌 시 리셋
    return;
  }

  if (eatFood())
  {
    if (game_speed > 60)
      game_speed -= 5;
  }

  drawGame();
}

// ================= 입력 처리 =================
static void readDirection()
{
  if (digitalRead(BTN_UP) == LOW && direction != SNAKE_DOWN)
    direction = SNAKE_UP;

  if (digitalRead(BTN_DOWN) == LOW && direction != SNAKE_UP)
    direction = SNAKE_DOWN;

  if (digitalRead(BTN_LEFT) == LOW && direction != SNAKE_RIGHT)
    direction = SNAKE_LEFT;

  if (digitalRead(BTN_RIGHT) == LOW && direction != SNAKE_LEFT)
    direction = SNAKE_RIGHT;
}

// ================= 이동 =================
static void moveSnake()
{
  for (int i = snake_length - 1; i > 0; i--)
  {
    snake_x[i] = snake_x[i - 1];
    snake_y[i] = snake_y[i - 1];
  }

  if (direction == SNAKE_UP)    snake_y[0]--;
  if (direction == SNAKE_DOWN)  snake_y[0]++;
  if (direction == SNAKE_LEFT)  snake_x[0]--;
  if (direction == SNAKE_RIGHT) snake_x[0]++;
}

// ================= 충돌 =================
static bool hitWall()
{
  return (snake_x[0] < 0 || snake_x[0] >= GRID_W ||
          snake_y[0] < 0 || snake_y[0] >= GRID_H);
}

// ================= 먹이 =================
static bool eatFood()
{
  if (snake_x[0] == food_x && snake_y[0] == food_y)
  {
    if (snake_length < MAX_LEN)
    {
      snake_x[snake_length] = food_x;
      snake_y[snake_length] = food_y;
      snake_length++;
    }
    spawnFood();
    return true;
  }
  return false;
}

// ================= 먹이 생성 =================
static void spawnFood()
{
  bool overlap;
  do {
    overlap = false;
    food_x = random(0, GRID_W);
    food_y = random(0, GRID_H);

    for (int i = 0; i < snake_length; i++)
    {
      if (food_x == snake_x[i] && food_y == snake_y[i])
      {
        overlap = true;
        break;
      }
    }
  } while (overlap);
}

// ================= 화면 출력 =================
static void drawGame()
{
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);

    // Food
    u8g2.drawBox(
      food_x * CELL_SIZE + 1,
      food_y * CELL_SIZE + 1,
      CELL_SIZE,
      CELL_SIZE
    );

    // Snake
    for (int i = 0; i < snake_length; i++)
    {
      int x = snake_x[i] * CELL_SIZE + 1;
      int y = snake_y[i] * CELL_SIZE + 1;

      if (i == 0)
        u8g2.drawBox(x, y, CELL_SIZE, CELL_SIZE);
      else
        u8g2.drawFrame(x, y, CELL_SIZE, CELL_SIZE);
    }

  } while (u8g2.nextPage());
}
