#include "pigpio.h"
#include "types.h"
#include "timers.h"
#include "buzzer.h"
#include "dab.h"
#include "globals.h"

int ledState;

void dabLedInit()
{
    ledState = 0;
    gpioWrite(DAB_ACT_PIN, 0);
}

void dabLedHandler()
{
    if(ledState == 0)
    {
        ledState = 1;
    }
    else
    {
        ledState = 0;
    }

    gpioWrite(DAB_ACT_PIN, ledState);
}

void dabBuzzOn()
{
    dabTimerType *buzzTimer;

    buzzTimer = dabGetTimer("BUZZ");
    dabStartTimer(buzzTimer);
    gpioWrite(DAB_LED1_PIN, 1);
}

void dabBuzzOff()
{
    gpioWrite(DAB_LED1_PIN, 0);
}

void dabBuzzHandler()
{
    dabTimerType *buzzTimer;

    buzzTimer = dabGetTimer("BUZZ");
    dabStopTimer(buzzTimer);
    dabBuzzOff();
}

