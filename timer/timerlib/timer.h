#ifdef __TIMER_H__
#define __TIMER_H__
int timer_init();

void timer_destroy();

int timer_add(long /*sec*/, long /*usec*/, void(* /*hndlr*/)(void *),
              void * /*hndlr_arg*/);
#endif //__TIMER_H__

