#ifndef __GOT_TIMERS

#define __GOT_TIMERS

dabTimerType *dabGetTimer(char *name);
void dabStartTimer(dabTimerType *timer);
void dabStopTimer(dabTimerType *timer);
void dabResetTimer(dabTimerType *timer);
void dabInitTimer(dabTimerType *timer, int reloadValue,
                                                      void (*handlerFn)(void));
void dabHandleTimers();

#endif

