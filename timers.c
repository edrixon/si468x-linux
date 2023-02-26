#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "types.h"
#include "dab.h"
#include "dablogger.h"
#include "globals.h"

dabTimerType *dabGetTimer(char *name)
{
    int timerId;

    timerId = 0;
    while(timerId < DAB_MAXTIMERS && strcmp(name, dabTimers[timerId].name) != 0)
    {
        timerId++;
    }

    if(timerId < DAB_MAXTIMERS)
    {
        return &dabTimers[timerId];
    }
    else
    {
        return NULL;
    }
}

void dabStartTimer(dabTimerType *timer)
{
    timer -> enabled = TRUE;
}

void dabStopTimer(dabTimerType *timer)
{
    timer -> enabled = FALSE;
}

void dabResetTimer(dabTimerType *timer)
{
    timer -> count = timer -> reload;
}

void dabInitTimer(dabTimerType *timer, int reloadValue, void (*handlerFn)(void))
{
    timer -> reload = reloadValue;
    dabResetTimer(timer);
    if((void *)handlerFn != NULL)
    {
        timer -> handlerFn = handlerFn;
    }
    dabStartTimer(timer);
}

void dabHandleTimers()
{
    int c;
    dabTimerType *timer;

    timer = dabTimers;
    for(c = 0; c < DAB_MAXTIMERS; c++)
    {
        if(timer -> enabled == TRUE)
        {
            if(timer -> runAlways == TRUE ||
              (timer -> runAlways == FALSE  && dabLoggerRunning() == FALSE))
            {
                if(timer -> count)
                {
                    timer -> count--;
                    if(timer -> count == 0)
                    {
                        dabResetTimer(timer);
                        timer -> handlerFn();
                    }
                }
            }
        }

        timer++;
    }
}

