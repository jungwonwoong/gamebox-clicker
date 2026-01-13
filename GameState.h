#ifndef GAME_STATE_H
#define GAME_STATE_H

enum AppState {
  STATE_MENU,
  STATE_PONG,
  STATE_SNAKE,
  STATE_STAIRS,
};

extern volatile AppState appState;

#endif
