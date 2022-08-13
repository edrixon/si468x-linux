#include <stdio.h>
#include <strings.h>

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dablogger.h"
#include "dab.h"

int loggerState;
int loggerFreq;
DABService monitorService;
DABService loggingService;
DABService *currentService;

int dabLoggerRunning()
{
    if(loggerState == LOGGER_INIT)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void dabInitLogger()
{
    loggerState = LOGGER_INIT;
    loggerFreq = 0;
    bzero(&loggingService, sizeof(DABService));
    currentService = &(dabShMem -> currentService);
}

void dabStartLogger()
{
    printf("Starting logger...\n");
    loggerState = LOGGER_TUNE;

    bcopy(currentService, &monitorService, sizeof(DABService));
    bcopy(&loggingService, currentService, sizeof(DABService));
}

void dabStopLogger()
{
    printf("Stopping logger...\n");
    loggerState = LOGGER_INIT;

    bcopy(currentService, &loggingService, sizeof(DABService));
    bcopy(&monitorService, currentService, sizeof(DABService));

    dabResetRadio();
}

void dabControlLogger()
{
    switch(loggerState)
    {
        case LOGGER_INIT:
            if(dabShMem -> telnetUsers == 0)
            {
                dabStartLogger();
            }
            break;

        case LOGGER_TUNE:
            if(dabShMem -> telnetUsers > 0)
            {
                dabStopLogger();
            }
            break;
     

        case LOGGER_MEASURE:
            if(dabShMem -> telnetUsers > 0)
            {
                dabStopLogger();
            }
            break;

        default:;
    }
}

void dabLogger()
{
    switch(loggerState)
    {
        case LOGGER_TUNE:
            dabLoggerTune();
            loggerState = LOGGER_MEASURE;
            break;

        case LOGGER_MEASURE:
            dabLoggerMeasure();
            loggerState = LOGGER_TUNE;
            break;

        default:;
    }
}

void dabLoggerTune()
{
    currentService -> Freq = loggerFreq;

    printf("  Logger: Tune %d - %3.3f MHz\n",
                  currentService -> Freq, freqIdToMHz(currentService -> Freq));

    dabTuneFreq(currentService);
}

void dabLoggerMeasure()
{
    printf("  Logger: Measure signal\n");

    currentService -> Freq = loggerFreq;
    dabServiceValid();
    printf("  Ensemble: %s\n", dabShMem -> dabFreq[loggerFreq].ensemble);
    dabGetDigRadioStatus();
    dabShowSignal();

    loggerFreq++;
    if(loggerFreq == dabShMem -> dabFreqs)
    {
        loggerFreq = 0;
    }
}
