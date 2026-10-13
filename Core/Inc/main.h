#ifndef MAIN_H
#define MAIN_H
#include "stm32f4xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart6;
void Error_Handler(void);
void MX_FREERTOS_Init(void);
#endif
