#include "snake.h"
#include <string.h>
static int same(Cell a, Cell b) { return a.x == b.x && a.y == b.y; }
static void food(Snake *s)
{
    if (s->length == SNAKE_MAX) { s->status = WON; return; }
    s->rng = s->rng * 1664525U + 1013904223U;
    unsigned skip = s->rng % (SNAKE_MAX - s->length);
    /* Select among free cells: bounded even when only one cell remains. */
    for (unsigned y=0; y<SNAKE_HGT; ++y) for (unsigned x=0; x<SNAKE_W; ++x) {
        Cell c = {(uint8_t)x, (uint8_t)y};
        unsigned i;
        for (i=0; i<s->length && !same(c,s->body[i]); ++i) {}
        if (i == s->length && skip-- == 0) { s->food=c; return; }
    }
}
void Snake_Init(Snake *s, uint32_t seed)
{
    memset(s,0,sizeof(*s));
    s->length=3; s->dir=RIGHT; s->status=READY; s->rng=seed;
    for (unsigned i=0; i<3; ++i) s->body[i]=(Cell){(uint8_t)(10-i),6};
    food(s);
}
void Snake_Command(Snake *s, Command cmd)
{
    if (cmd == RESTART || (cmd == ACTION && (s->status == LOST || s->status == WON))) {
        Snake_Init(s,s->rng); s->status=RUNNING;
    } else if (cmd == ACTION) {
        s->status = s->status == RUNNING ? PAUSED : RUNNING;
    } else if (cmd <= LEFT && s->status == RUNNING && !s->turned &&
               cmd != s->dir && ((unsigned)cmd+2U)%4U != (unsigned)s->dir) {
        s->dir=cmd; s->turned=1;
    }
}
void Snake_Step(Snake *s)
{
    if (s->status != RUNNING) return;
    int x=s->body[0].x, y=s->body[0].y;
    if (s->dir==UP) --y;
    if (s->dir==DOWN) ++y;
    if (s->dir==LEFT) --x;
    if (s->dir==RIGHT) ++x;
    s->turned=0;
    if (x<0 || x>=SNAKE_W || y<0 || y>=SNAKE_HGT) { s->status=LOST; return; }
    Cell head={(uint8_t)x,(uint8_t)y};
    int grow=same(head,s->food);
    /* The tail moves away on a non-growing step. */
    for (unsigned i=0; i<s->length-(grow?0U:1U); ++i)
        if (same(head,s->body[i])) { s->status=LOST; return; }
    if (grow) { ++s->length; ++s->score; }
    memmove(&s->body[1], &s->body[0], (s->length-1)*sizeof(Cell));
    s->body[0]=head;
    if (grow) food(s);
}
