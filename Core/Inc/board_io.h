#ifndef BOARD_IO_H
#define BOARD_IO_H
#include "snake.h"
#include "FreeRTOS.h"
#include "semphr.h"
extern SemaphoreHandle_t i2cMutex;
Command Board_ReadKey(void);
void Board_Draw(const Snake *s);
#endif
