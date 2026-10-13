#include "main.h"
#include "cmsis_os.h"

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart6;
static TIM_HandleTypeDef htim1;

/* HAL uses TIM1; SysTick belongs exclusively to FreeRTOS. */
HAL_StatusTypeDef HAL_InitTick(uint32_t priority)
{
    __HAL_RCC_TIM1_CLK_ENABLE();
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = HAL_RCC_GetPCLK2Freq() / 1000000U - 1U;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 999;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) return HAL_ERROR;
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    uwTickPrio = priority;
    return HAL_TIM_Base_Start_IT(&htim1);
}
void HAL_SuspendTick(void) { __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE); }
void HAL_ResumeTick(void) { __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE); }
void TIM1_UP_TIM10_IRQHandler(void) { HAL_TIM_IRQHandler(&htim1); }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *h)
{ if (h->Instance == TIM1) HAL_IncTick(); }

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef o = {0};
    RCC_ClkInitTypeDef c = {0};
    o.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    o.HSIState = RCC_HSI_ON;
    o.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    o.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&o) != HAL_OK) Error_Handler();
    c.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    c.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    c.AHBCLKDivider = RCC_SYSCLK_DIV1;
    c.APB1CLKDivider = RCC_HCLK_DIV1;
    c.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&c, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void Peripherals_Init(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_USART6_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
    g.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &g);
    /* SDK OLED and PCA9538 keyboard: PB8=SCL, PB9=SDA. */
    g.Pin = GPIO_PIN_8|GPIO_PIN_9;
    g.Mode = GPIO_MODE_AF_OD;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &g);
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
    g.Pin = GPIO_PIN_6|GPIO_PIN_7;
    g.Mode = GPIO_MODE_AF_PP;
    g.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &g);
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX_RX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart6) != HAL_OK) Error_Handler();
}

int main(void)
{
    HAL_Init();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    SystemClock_Config();
    Peripherals_Init();
    MX_FREERTOS_Init();
    osKernelStart();
    Error_Handler();
}
void Error_Handler(void)
{
    __disable_irq();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIOD->BSRR = GPIO_PIN_15;
    for (;;) { __NOP(); }
}
void HardFault_Handler(void) { Error_Handler(); }
void MemManage_Handler(void) { Error_Handler(); }
void BusFault_Handler(void) { Error_Handler(); }
void UsageFault_Handler(void) { Error_Handler(); }
