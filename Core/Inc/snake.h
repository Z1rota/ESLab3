#ifndef SNAKE_H
#define SNAKE_H
#include <stdint.h>
#define SNAKE_W 30
#define SNAKE_HGT 12
#define SNAKE_MAX (SNAKE_W*SNAKE_HGT)
typedef enum { UP, RIGHT, DOWN, LEFT, ACTION, RESTART, NONE } Command;
typedef enum { READY, RUNNING, PAUSED, LOST, WON } GameStatus;
typedef struct { uint8_t x, y; } Cell;
typedef struct {
    Cell body[SNAKE_MAX], food;
    uint16_t length, score;
    Command dir;
    GameStatus status;
    uint32_t rng;
    uint8_t turned;
} Snake;
void Snake_Init(Snake *s, uint32_t seed);
void Snake_Command(Snake *s, Command cmd);
void Snake_Step(Snake *s);
#endif
