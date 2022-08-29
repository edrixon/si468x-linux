#include <stdio.h>
#include <string.h>
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
#include "globals.h"

int dabLoggerRunning()
{
    return dabShMem -> loggerRunning;
}

void dabInitLogger()
{
    loggerState = LOGGER_INIT;
    loggerFreq = 21;
    bzero(&loggingService, sizeof(DABService));
    currentService = &(dabShMem -> currentService);
    dabShMem -> loggerMeasureSeconds =
                            (double)(DAB_TICKTIME * DAB_LOGGER_TICKS) / 1000.0;
}

void dabStartLoggerWait()
{
}

void dabStartLogger()
{
    printf("****\n");
    printf("**** Starting logger...\n");
    printf("****\n");
    loggerState = LOGGER_TUNE;

    bcopy(currentService, &monitorService, sizeof(DABService));
    bcopy(&loggingService, currentService, sizeof(DABService));

    dabShMem -> loggerRunning = TRUE;
}

void dabStopLogger()
{
    printf("****\n");
    printf("**** Stopping logger...\n");
    printf("****\n");
    loggerRestartDelay = LOG_RESTART_TICKS; 

    loggerState = LOGGER_STARTWAIT;

    bcopy(currentService, &loggingService, sizeof(DABService));
    bcopy(&monitorService, currentService, sizeof(DABService));

    dabResetRadio();

    dabShMem -> loggerRunning = FALSE;
}

void dabControlLogger()
{
    switch(loggerState)
    {
        case LOGGER_INIT:
            if(dabShMem -> telnetUsers == 0 && dabShMem -> httpUsers == 0)
            {
                dabStartLogger();
            }
            break;

        case LOGGER_TUNE:
            if(dabShMem -> telnetUsers > 0 || dabShMem -> httpUsers > 0)
            {
                dabStopLogger();
            }
            break;
     

        case LOGGER_MEASURE:
            if(dabShMem -> telnetUsers > 0 || dabShMem -> httpUsers > 0)
            {
                dabStopLogger();
            }
            break;

        case LOGGER_STARTWAIT:
            if(dabShMem -> telnetUsers > 0 || dabShMem -> httpUsers > 0)
            {
                loggerRestartDelay = LOG_RESTART_TICKS; 
            }
            else
            {
                if(loggerRestartDelay == 0)
                {
                    dabStartLogger();
                }    
                else
                {
                    loggerRestartDelay--;
                }
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
    char block[6];

    currentService -> Freq = loggerFreq;

    freqIdToBlock(loggerFreq, block);
    printf("  Tuning to %s, %3.3f MHz\n", block,
                                          freqIdToMHz(currentService -> Freq));

    dabTuneFreq(currentService);
}

void dabLoggerMeasure()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[loggerFreq]);

    currentService -> Freq = loggerFreq;
    dabServiceValid();
    if(dabGetEnsembleInfo() == TRUE)
    {
        printf("  Found ensemble '%s'\n", dFreq -> ensemble );
    }
    else
    {
        printf("  No DAB service found\n");
    }
    dabGetDigRadioStatus();
    dabShowSignal();

    loggerFreq++;
    if(loggerFreq == dabShMem -> dabFreqs)
    {
        loggerFreq = 0;
    }

    printf("\n");
}
