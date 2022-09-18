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
#include "timers.h"
#include "globals.h"

int dabLoggerRunning()
{
    return dabShMem -> loggerRunning;
}

void dabInitLogger()
{
    dabLogger.state = LOGGER_INIT;
    dabLogger.freq = 21;
    bzero(&dabLogger.loggingService, sizeof(DABService));
    dabLogger.currentService = &(dabShMem -> currentService);
    dabShMem -> loggerMeasureSeconds =
                                   (DAB_LOGGER_TICKS * DAB_TICKTIME) / 1000.0;
}

void dabStartLoggerWait()
{
}

void dabStartLogger()
{
    dabTimerType *dTmr;

    printf("****\n");
    printf("**** Starting logger...\n");
    printf("****\n");
    dabLogger.state = LOGGER_TUNE;

    bcopy(
       dabLogger.currentService, &dabLogger.monitorService, sizeof(DABService));
    bcopy(
       &dabLogger.loggingService, dabLogger.currentService, sizeof(DABService));

    dTmr = dabGetTimer("LOGGER");
    dabStartTimer(dTmr);

    dabShMem -> loggerRunning = TRUE;

    dabLoggerTune();
}

void dabStopLogger()
{
    dabTimerType *dTmr;

    printf("****\n");
    printf("**** Stopping logger...\n");
    printf("****\n");
    loggerRestartDelay = LOG_RESTART_TICKS; 

    dabLogger.state = LOGGER_STARTWAIT;

    bcopy(dabLogger.currentService, &dabLogger.loggingService, sizeof(DABService));
    bcopy(&dabLogger.monitorService, dabLogger.currentService, sizeof(DABService));

    dTmr = dabGetTimer("LOGGER");
    dabStopTimer(dTmr);

    dabResetRadio();

    dabShMem -> loggerRunning = FALSE;
}

void dabControlLogger()
{
    switch(dabLogger.state)
    {
        case LOGGER_INIT:
            if(dabShMem -> telnetUsers == 0 && dabShMem -> httpUsers == 0)
            {
                dabStartLogger();
            }
            break;

        case LOGGER_TUNE:;
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

void dabLoggerMain()
{
#ifdef __SEPARATE_TUNE
    switch(dabLogger.state)
    {
        case LOGGER_TUNE:
            dabLoggerTune();
            dabLogger.state = LOGGER_MEASURE;
            break;

        case LOGGER_MEASURE:
            dabLoggerMeasure();
            dabLogger.state = LOGGER_TUNE;
            break;

        default:;
    }
#else

    dabLoggerMeasure();
    dabLoggerTune();

#endif
}

void dabLoggerTune()
{
    char block[6];

    dabLogger.currentService -> Freq = dabLogger.freq;

    freqIdToBlock(dabLogger.freq, block);
    printf("  Tuning to %s, %3.3lf MHz\n", block, currentFreq());

    dabTuneFreq(dabLogger.currentService);
}

void dabLoggerMeasure()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabLogger.freq]);

    dabLogger.currentService -> Freq = dabLogger.freq;
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

    dabLogger.freq++;
    if(dabLogger.freq == dabShMem -> numDabFreqs)
    {
        dabLogger.freq = 0;
    }

    printf("\n");
}
