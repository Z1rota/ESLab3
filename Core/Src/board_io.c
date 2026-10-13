#include "main.h"
#include "board_io.h"
#include <string.h>

/* HAL expects the 7-bit I2C address shifted left once. */
#define OLED_ADDR (0x3cU << 1)
#define KEY_ADDR  (0x71U << 1)
static uint8_t frame[1024];
static int displayReady;

static HAL_StatusTypeDef writeReg(uint16_t addr, uint8_t reg, uint8_t value)
{ return HAL_I2C_Mem_Write(&hi2c1,addr,reg,I2C_MEMADD_SIZE_8BIT,&value,1,20); }

Command Board_ReadKey(void)
{
    static const Command map[4][3] = {
        {NONE,UP,NONE}, {LEFT,ACTION,RIGHT}, {NONE,DOWN,NONE}, {NONE,NONE,RESTART}
    };
    Command result=NONE;
    unsigned pressed=0;
    int ok=1;
    xSemaphoreTake(i2cMutex,portMAX_DELAY);
    ok = writeReg(KEY_ADDR,2,0)==HAL_OK && writeReg(KEY_ADDR,1,0)==HAL_OK;
    for (unsigned row=0; ok && row<4; ++row) {
        uint8_t pins=0xff;
        ok = writeReg(KEY_ADDR,3,(uint8_t)~(1U<<row))==HAL_OK;
        if (ok) ok = HAL_I2C_Mem_Read(&hi2c1,KEY_ADDR,0,I2C_MEMADD_SIZE_8BIT,&pins,1,20)==HAL_OK;
        for (unsigned col=0; ok && col<3; ++col) if (!(pins & (0x10U<<col))) {
            ++pressed; result=map[row][col];
        }
    }
    if (writeReg(KEY_ADDR,3,0xff)!=HAL_OK) ok=0;
    xSemaphoreGive(i2cMutex);
    /* Ignore multiple keys (matrix ghosting) and failed I2C transactions. */
    return ok && pressed==1 ? result : NONE;
}
static void pixel(unsigned x, unsigned y)
{ if (x<128 && y<64) frame[x+128*(y/8)] |= (uint8_t)(1U<<(y%8)); }
static void box(unsigned x,unsigned y,unsigned w,unsigned h)
{ for (unsigned j=0;j<h;++j) for (unsigned i=0;i<w;++i) pixel(x+i,y+j); }
static void text(unsigned x,unsigned y,const char *s)
{
    /* Original compact 3x5 bitmap font, rows packed in 15 bits. */
    static const char chars[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ# ";
    static const uint16_t glyph[]={
        0x7b6f,0x2492,0x73e7,0x73cf,0x5bc9,0x79cf,0x79ef,0x7249,0x7bef,0x7bcf,
        0x2bed,0x6bae,0x7927,0x6b6e,0x79a7,0x79a4,0x796f,0x5bed,0x7497,
        0x126f,0x5bad,0x4927,0x5fed,0x5f7d,0x7b6f,0x7be4,0x7b7b,0x7bad,
        0x79cf,0x7492,0x5b6f,0x5b6a,0x5bfd,0x5aad,0x5a92,0x72a7,0x5f7d,0
    };
    while (*s && x+3<=128) {
        const char *p=strchr(chars,*s++);
        uint16_t bits=p?glyph[p-chars]:0;
        for(unsigned row=0;row<5;++row) for(unsigned col=0;col<3;++col)
            if(bits & (1U<<(14-row*3-col))) pixel(x+col,y+row);
        x+=4;
    }
}
void Board_Draw(const Snake *s)
{
    static const uint8_t init[]={0xae,0x20,0x02,0xb0,0xc8,0x00,0x10,0x40,
        0x81,0x7f,0xa1,0xa6,0xa8,0x3f,0xa4,0xd3,0,0xd5,0xf0,
        0xd9,0x22,0xda,0x12,0xdb,0x20,0x8d,0x14,0xaf};
    if (!displayReady) {
        int ok=1;
        xSemaphoreTake(i2cMutex,portMAX_DELAY);
        for (unsigned i=0;ok && i<sizeof(init);++i) ok=writeReg(OLED_ADDR,0,init[i])==HAL_OK;
        xSemaphoreGive(i2cMutex);
        if (!ok) { HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_SET); return; }
        displayReady=1;
    }
    memset(frame,0,sizeof(frame));
    char label[33];
    static const char *status[]={"5 START","5 PAUSE","5 RESUME","LOST 5 RETRY","WON 5 RETRY"};
    strcpy(label,status[s->status]);
    strcat(label," SCORE ");
    unsigned len=strlen(label), score=s->score;
    label[len++]=(char)('0'+score/100);
    label[len++]=(char)('0'+score/10%10);
    label[len++]=(char)('0'+score%10);
    label[len]=0;
    text(2,3,label);
    box(3,13,122,1); box(3,62,122,1); box(3,13,1,50); box(124,13,1,50);
    for (unsigned i=0;i<s->length;++i) box(4+s->body[i].x*4,14+s->body[i].y*4,3,3);
    if (s->status!=WON) box(4+s->food.x*4,14+s->food.y*4,3,3);
    for(unsigned page=0;page<8;++page) {
        /* Release between pages so the input task can scan the same bus. */
        xSemaphoreTake(i2cMutex,portMAX_DELAY);
        int ok=writeReg(OLED_ADDR,0,0xb0+page)==HAL_OK &&
               writeReg(OLED_ADDR,0,0)==HAL_OK && writeReg(OLED_ADDR,0,0x10)==HAL_OK;
        if(ok) ok=HAL_I2C_Mem_Write(&hi2c1,OLED_ADDR,0x40,I2C_MEMADD_SIZE_8BIT,
                                   &frame[page*128],128,30)==HAL_OK;
        xSemaphoreGive(i2cMutex);
        if(!ok) { displayReady=0; HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,GPIO_PIN_SET); return; }
    }
    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,s->status==LOST?GPIO_PIN_SET:GPIO_PIN_RESET);
}
