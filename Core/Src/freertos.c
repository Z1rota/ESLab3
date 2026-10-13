#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "board_io.h"

SemaphoreHandle_t i2cMutex;
static QueueHandle_t commands, frames;

static void InputTask(void const *argument)
{
    (void)argument;
    Command last=NONE, stable=NONE;
    unsigned count=0;
    TickType_t wake=xTaskGetTickCount();
    for (;;) {
        Command key=Board_ReadKey();
        if(key!=last) { last=key; count=0; }
        else if(count<2) ++count;
        if(count==2 && key!=stable) {
            stable=key;
            if(key!=NONE) (void)xQueueSend(commands,&key,0);
        }
        vTaskDelayUntil(&wake,pdMS_TO_TICKS(20));
    }
}
static void GameTask(void const *argument)
{
    (void)argument;
    static Snake game;
    Snake_Init(&game,HAL_GetTick());
    TickType_t wake=xTaskGetTickCount();
    unsigned elapsed=0;
    for (;;) {
        Command cmd;
        /* Limit work per cycle even if input arrives while draining. */
        for(unsigned n=0;n<8 && xQueueReceive(commands,&cmd,0)==pdPASS;++n) {
            GameStatus previous=game.status;
            Snake_Command(&game,cmd);
            if(previous!=game.status || cmd==RESTART) elapsed=0;
        }
        if(game.status==RUNNING) {
            elapsed+=20;
            if(elapsed>=300) { Snake_Step(&game); elapsed=0; }
        } else elapsed=0;
        HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,game.status==RUNNING?GPIO_PIN_SET:GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,game.status==PAUSED?GPIO_PIN_SET:GPIO_PIN_RESET);
        /* Length-one queue copies a consistent snapshot; display never shares game memory. */
        xQueueOverwrite(frames,&game);
        vTaskDelayUntil(&wake,pdMS_TO_TICKS(20));
    }
}
static void DisplayTask(void const *argument)
{
    (void)argument;
    static Snake snapshot;
    osDelay(100);
    for (;;) {
        if(xQueueReceive(frames,&snapshot,portMAX_DELAY)==pdPASS) Board_Draw(&snapshot);
        osDelay(20);
    }
}
void MX_FREERTOS_Init(void)
{
    i2cMutex=xSemaphoreCreateMutex();
    commands=xQueueCreate(8,sizeof(Command));
    frames=xQueueCreate(1,sizeof(Snake));
    if(!i2cMutex || !commands || !frames) Error_Handler();
    osThreadDef(input,InputTask,osPriorityAboveNormal,0,256);
    osThreadDef(game,GameTask,osPriorityNormal,0,384);
    osThreadDef(display,DisplayTask,osPriorityBelowNormal,0,512);
    if(!osThreadCreate(osThread(input),NULL) || !osThreadCreate(osThread(game),NULL) ||
       !osThreadCreate(osThread(display),NULL)) Error_Handler();
}
void vApplicationMallocFailedHook(void) { Error_Handler(); }
void vApplicationStackOverflowHook(TaskHandle_t task,char *name)
{ (void)task; (void)name; Error_Handler(); }
