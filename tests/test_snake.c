#include "snake.h"
#include <string.h>
#define CHECK(x) do { if (!(x)) return __LINE__; } while(0)
int test_all(void)
{
    Snake s;
    Snake_Init(&s,123);
    CHECK(s.length==3 && s.status==READY);
    Cell head=s.body[0];
    Snake_Step(&s);
    CHECK(memcmp(&head,&s.body[0],sizeof(head))==0);
    Snake_Command(&s,ACTION);
    Snake_Command(&s,LEFT);
    CHECK(s.dir==RIGHT);
    Snake_Command(&s,UP);
    Snake_Command(&s,LEFT);
    CHECK(s.dir==UP); /* One turn per movement tick. */
    Snake_Step(&s);
    CHECK(s.body[0].y==5 && s.body[0].x==10);
    Snake_Command(&s,ACTION);
    CHECK(s.status==PAUSED);
    head=s.body[0]; Snake_Step(&s);
    CHECK(memcmp(&head,&s.body[0],sizeof(head))==0);
    Snake_Command(&s,RESTART);
    CHECK(s.status==RUNNING && s.score==0);
    s.food=(Cell){11,6}; Snake_Step(&s);
    CHECK(s.length==4 && s.score==1 && s.body[0].x==11);
    for(unsigned i=0;i<s.length;++i)
        CHECK(memcmp(&s.food,&s.body[i],sizeof(Cell))!=0);
    s.body[0]=(Cell){29,6}; s.dir=RIGHT; Snake_Step(&s);
    CHECK(s.status==LOST);
    Snake_Command(&s,ACTION); CHECK(s.status==RUNNING && s.length==3);
    /* A square: moving into the departing tail is legal. */
    s.length=4; s.body[0]=(Cell){1,1}; s.body[1]=(Cell){1,2};
    s.body[2]=(Cell){2,2}; s.body[3]=(Cell){2,1};
    s.dir=RIGHT; s.food=(Cell){9,9}; Snake_Step(&s);
    CHECK(s.status==RUNNING && s.body[0].x==2);
    /* Moving into the neck is a collision. */
    s.dir=LEFT; Snake_Step(&s); CHECK(s.status==LOST);
    Snake_Init(&s,1); s.status=RUNNING; s.length=SNAKE_MAX-1;
    s.body[0]=(Cell){1,0};
    unsigned n=1;
    for(unsigned y=0;y<SNAKE_HGT;++y) for(unsigned x=0;x<SNAKE_W;++x)
        if(y!=0 || x>1) s.body[n++]=(Cell){(uint8_t)x,(uint8_t)y};
    s.food=(Cell){0,0}; s.dir=LEFT; Snake_Step(&s);
    CHECK(s.status==WON && s.length==SNAKE_MAX);
    /* Many seeds: food must always be in bounds and off the snake. */
    for(unsigned seed=0;seed<1000;++seed) {
        Snake_Init(&s,seed);
        CHECK(s.food.x<SNAKE_W && s.food.y<SNAKE_HGT);
        for(unsigned i=0;i<s.length;++i)
            CHECK(memcmp(&s.food,&s.body[i],sizeof(Cell))!=0);
    }
    return 0;
}
