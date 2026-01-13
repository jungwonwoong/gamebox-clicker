#include "StairGame.h"
#include <Arduino.h>
#include "GameState.h"
#include "Buttons.h"
#include <EEPROM.h>
#define EEPROM_ADDR_BEST_TIME 0   // 4바이트
// ================= OLED (외부 공유) =================
extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

// ===== 상수 =====
#define TOTAL_STEPS     100
#define VISIBLE_STEPS   8


#define STEP_W          9
#define STEP_H          4
#define STEP_DX         9
#define STEP_DY         7

#define PLAYER_Y        50
static bool timerRunning = false;
static unsigned long startTime = 0;
static unsigned long elapsedTime = 0;
static unsigned long bestTime = 0;

// ===== 방향 =====
enum StairDir { LEFT, RIGHT };
// ===== 구조체 =====
struct Step {
  uint8_t x;
  uint8_t y;
};
// ===== 게임상태 =====
static Step steps[TOTAL_STEPS];
static int currentStep;
static int playerX;
int dir=0;
int leftAvail=0;
int rightAvail=0;
int runSlots=0;
static bool gameOver;
//static unsigned long failTime;
static StairDir playerDir;
// ===== 내부 함수 =====
static void generateSteps();
static void drawGame();
static void resetGame();
static void handleInput();
static void scrollStepsDown();

// ================= 초기화 =================
void stairInit() {
  // pinMode(BTN_LEFT, INPUT_PULLUP);
  // pinMode(BTN_RIGHT, INPUT_PULLUP);
    // EEPROM에서 최고 기록 읽기
  EEPROM.get(EEPROM_ADDR_BEST_TIME, bestTime);

  // 초기값 보호 (처음 실행 시)
  if (bestTime == 0xFFFFFFFF || bestTime == 0) {
    bestTime = 0;
  }
  resetGame();
}

// ================= 메인 =================
void stairGame() {
  if (digitalRead(BTN_BACK) == LOW) {
    delay(200);          // 디바운스
    appState = STATE_MENU;
    return;              // snake 즉시 종료
  }
  if (gameOver) {
    drawGame();
    delay(500);
      resetGame();
    return;
  }

  handleInput();
  drawGame();
}

// ================= 입력 =================
static void handleInput() {
  static bool lastTurn = HIGH;
  static bool lastClimb = HIGH;

  bool turnNow = digitalRead(BTN_LEFT);
  bool climbNow = digitalRead(BTN_RIGHT);

  int next = currentStep + 1;
  if (next >= TOTAL_STEPS) return;

  // ================= 방향 전환 =================
  if (lastTurn == HIGH && turnNow == LOW) {
    // 방향만 전환 (이동 X, 판정 X)
    playerDir = (playerDir == LEFT) ? RIGHT : LEFT;
    StairDir stepDir = (steps[next].x > playerX) ? RIGHT : LEFT;
    if (playerDir == stepDir) {
      playerX = steps[next].x;
      currentStep++;
      scrollStepsDown();
    } else {
      scrollStepsDown();
      gameOver = true;
    }
  }

  // ================= 오르기 =================
  if (lastClimb == HIGH && climbNow == LOW) {
    // ⏱ 첫 이동 시 타이머 시작
    if (!timerRunning) {
      timerRunning = true;
      startTime = millis();
    }
    // 다음 계단이 어느 쪽인지 판단
    StairDir stepDir = (steps[next].x > playerX) ? RIGHT : LEFT;

    // 방향이 맞으면 이동
    if (playerDir == stepDir) {
      playerX = steps[next].x;
      currentStep++;
      scrollStepsDown();
          // 🏁 100번째 계단 도달
      if (currentStep >= TOTAL_STEPS - 1) {
        timerRunning = false;
        elapsedTime = millis() - startTime;

        // 🥇 최고 기록 갱신
        if (bestTime == 0 || elapsedTime < bestTime) {
          bestTime = elapsedTime;
          EEPROM.put(EEPROM_ADDR_BEST_TIME, bestTime);
        }

        gameOver = true;
       // failTime = millis();
      }
    } else {
      // 실패 → 떨어짐
      gameOver = true;
     // failTime = millis();
    }
  }

  lastTurn = turnNow;
  lastClimb = climbNow;
}

// ================= 계단 스크롤 =================
static void scrollStepsDown() {
  for (int i = 0; i < TOTAL_STEPS; i++) {
    steps[i].y += STEP_DY;
  }
}

// ================= 그리기 =================
static void drawGame() {
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.firstPage();
  do {
    // 계단
    for (int i = 0; i < VISIBLE_STEPS; i++) {
      int idx = currentStep + i;
      if (idx >= TOTAL_STEPS) break;

      u8g2.drawBox(
        steps[idx].x,
        steps[idx].y,
        STEP_W,
        STEP_H
      );
    }

    // 캐릭터 몸통
    u8g2.drawBox(playerX + 2, PLAYER_Y - 6, 5, 6);

    // 코 (방향 표시)
    if (playerDir == LEFT) {
      u8g2.drawBox(playerX, PLAYER_Y - 8, 3, 2);
    } else {
      u8g2.drawBox(playerX + STEP_W - 3, PLAYER_Y - 8, 3, 2);
    }
  if (timerRunning) {
  elapsedTime = millis() - startTime;
  }

  unsigned long secNow  = elapsedTime / 100;
  unsigned long secBest = bestTime / 100;

  // 최고 기록
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.setCursor(0, 64);
  u8g2.print("PR:");
  u8g2.print(secBest);
  u8g2.print("s");

  // 현재 스톱워치
  u8g2.setCursor(110, 64);
  u8g2.print(secNow);
  u8g2.print("s");

  if(gameOver) {
    u8g2.setFont(u8g2_font_ncenB14_tr);   // 큰 폰트 선택
    u8g2.setCursor(15, 20);
    if (currentStep >= TOTAL_STEPS - 1) {
       u8g2.print(" CLEAR !");
    } else {
    u8g2.print("You Died!");
    }
    
  }
  } while (u8g2.nextPage());

  
}

// ================= 리셋 =================
static void resetGame() {
  generateSteps();
  currentStep = 0;
  playerX = steps[0].x;
  playerDir = RIGHT;
  gameOver = false;
    // ⏱ 타이머 리셋
  timerRunning = false;
  startTime = 0;
  elapsedTime = 0;
}

static void generateSteps() {
  int minX = 9;
  int maxX = 120 - STEP_W;

  int totalSlots = (maxX - minX) / STEP_DX;
  int centerSlot = totalSlots / 2;

  int currentSlot = centerSlot;
  int x = minX + currentSlot * STEP_DX;
  int y = PLAYER_Y;

  for (int i = 0; i < TOTAL_STEPS; i++) {

    // 현재 계단 저장
    steps[i] = { x, y };
    // 현재 위치 기준으로 가능한 범위 계산
    if(i==0) {
      leftAvail  = -currentSlot;
      rightAvail = totalSlots - currentSlot;

      // 다음 run 크기 (좌:-, 우:+)
      do {
        runSlots=random(leftAvail, rightAvail + 1);
      } while (runSlots == 0);
      // runSlots 만큼 계단 생성
      dir = (runSlots > 0) ? 1 : -1;
     }
    int count = abs(runSlots);

    for (int r = 0; r < count && i < TOTAL_STEPS - 1; r++) {
      currentSlot =currentSlot+ dir;
      x = minX + currentSlot * STEP_DX;
      y -= STEP_DY;
      i++;
      steps[i] = { x, y };
    }

    y -= STEP_DY;
    if(i!=0) {
      leftAvail  = -currentSlot;
      rightAvail = totalSlots - currentSlot;
      do {
        runSlots=random(leftAvail, rightAvail + 1);
      } while (runSlots == 0);

    // runSlots 만큼 계단 생성
      dir = (runSlots > 0) ? 1 : -1;
      currentSlot =currentSlot+ dir;
      x = minX + currentSlot * STEP_DX;
    }
  }
}
