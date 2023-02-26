#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dablogger.h"
#include "dab.h"
#include "buzzer.h"
#include "timers.h"
#include "globals.h"

time_t lastGpsFix;

int dabGetLoggerMode()
{
    return dabLogger.runMode;
}

int dabLoggerSetRunMode(int runMode)
{
    if(runMode == LOGGER_SCAN || runMode == LOGGER_COVERAGE)
    {
        dabLogger.runMode = runMode;
    }

    return dabLogger.runMode;
}

int dabLoggerRunning()
{
    return dabShMem -> loggerRunning;
}

void dabInitLogger()
{
    dabLogger.state = LOGGER_INIT;
    dabLogger.runMode = LOGGER_SCAN;
    dabLogger.freq = 21;
    bzero(&dabLogger.loggingService, sizeof(DABService));
    dabLogger.currentService = &(dabShMem -> currentService);
    dabShMem -> loggerMeasureSeconds =
                               (DAB_LOGGER_SCAN_TICKS * DAB_TICKTIME) / 1000.0;
}

void dabStartLoggerWait()
{
}

void dabStartLogger()
{
    dabTimerType *dTmr;
    struct tm *tmPtr;
    time_t secsNow;
    char fname[32];

    printf("****\n");
    printf("**** Starting logger - ");

    if(dabLogger.runMode == LOGGER_COVERAGE)
    {
        printf("COVERAGE mode\n");
    }
    else
    {
        printf("SCAN mode\n");
    }
    printf("****\n");

    dTmr = dabGetTimer("LOGGER");

    dabLogger.state = dabLogger.runMode;
    if(dabLogger.runMode == LOGGER_COVERAGE)
    {
        dabInitTimer(dTmr, DAB_LOGGER_COVER_TICKS, NULL);
        dabLogger.freq = dabShMem -> currentService.Freq;
        lastGpsFix = 0;

        time(&secsNow);
        tmPtr = localtime(&secsNow);
        strftime(fname, sizeof(dabLogger.logFilename),
                                         "%d%m%y%H%M%S.log", tmPtr);
        sprintf(dabLogger.logFilename, "%s/%s", LOGDIR, fname);
        printf("  Logfile - %s\n", dabLogger.logFilename);
        dabShowServiceSummary();
    }
    else
    {
        dabInitTimer(dTmr, DAB_LOGGER_SCAN_TICKS, NULL);
        dabLogger.logFilename[0] = '\0';
        bcopy(dabLogger.currentService, &dabLogger.monitorService,
                                                           sizeof(DABService));
        bcopy(&dabLogger.loggingService, dabLogger.currentService,
                                                           sizeof(DABService));

        dabLoggerTune();
    }

    dabStartTimer(dTmr);

    dabShMem -> loggerRunning = TRUE;
}

void dabStopLogger()
{
    dabTimerType *dTmr;

    printf("****\n");
    printf("**** Stopping logger...\n");
    printf("****\n");
    loggerRestartDelay = LOG_RESTART_TICKS; 

    dabLogger.state = LOGGER_STARTWAIT;

    if(dabLogger.runMode != LOGGER_COVERAGE)
    {
        bcopy(dabLogger.currentService, &dabLogger.loggingService,
                                                           sizeof(DABService));
        bcopy(&dabLogger.monitorService, dabLogger.currentService,
                                                           sizeof(DABService));
    }

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

        case LOGGER_SCAN:;
        case LOGGER_COVERAGE:
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
    switch(dabLogger.state)
    {
        case LOGGER_SCAN:
            dabLoggerMeasure();
            dabLoggerTune();
            break;

        case LOGGER_COVERAGE:
            dabLoggerCoverage();
            break;

        default:;
    }
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

void dabLoggerWrite(struct tm *tmPtr, gpsInfoType *gpsInfo,
                                                    sigQualityType *sigQuality)
{
    FILE *fp;
    dabFreqType *dFreq;
    char block[6];

    dabBuzzOn();

    dFreq = currentDabFreq();
    freqIdToBlock(dabLogger.freq, block);

    fp = fopen(dabLogger.logFilename, "a");
    if(fp == NULL)
    {
        printf("  Error opening logfile\n");
        return;
    }

    printf("  Append logfile entry\n");

    fprintf(fp, "%02d-%02d-%02d,%02d:%02d:%02d,"
                "%0.6f,%0.6f,"
                "%d,%d,%d,%d,"
                "%3.3lf,%s,%s\n",
                   tmPtr -> tm_mday,
                   (tmPtr -> tm_mon) + 1,
                   (tmPtr -> tm_year) - 100,
                   tmPtr -> tm_hour,
                   tmPtr -> tm_min,
                   tmPtr -> tm_sec,
                   gpsInfo -> latitude,
                   gpsInfo -> longitude,
                   sigQuality -> rssi,
                   sigQuality -> snr,
                   sigQuality -> cnr,
                   sigQuality -> ficQuality,
                   currentFreq(),
                   block,
                   dFreq -> ensemble);

    fclose(fp);
}

void dabLoggerCoverage()
{
    dabFreqType *dFreq;
    char tmStr[80];
    struct tm *tmPtr;
    gpsInfoType gpsInfo;

    shmLock();
    bcopy(&(dabShMem -> gpsInfo), &gpsInfo, sizeof(gpsInfoType));
    shmFree();

    switch(gpsInfo.fix)
    {
        case 2:
            printf("2D fix");
            break;

        case 3:
            printf("3D fix");
            break;

        default:
            printf("No GPS fix\n");
            return;
    }

    tmPtr = localtime(&gpsInfo.seconds);
    strftime(tmStr, 80, " at %d %B %Y, %H:%M:%S", tmPtr);
    printf("%s (%ld seconds since last)\n",
                                         tmStr, gpsInfo.seconds - lastGpsFix);

    if(isfinite(gpsInfo.latitude) && isfinite(gpsInfo.longitude))
    {
        printf("  Latitude: %0.6f  Longitude: %0.6f\n",
                                      gpsInfo.latitude,
                                      gpsInfo.longitude);
        printf("  Altitude: %0.3f m  Speed: %0.3f m/s\n",
                                      gpsInfo.altitude,
                                      gpsInfo.speed);

        dabGetEnsembleInfo();
        dabGetDigRadioStatus();
        dabShowServiceSummary();

        dFreq = currentDabFreq();
        printf("  RSSI: %d dBuV  SNR: %d dB  CNR: %d dB  FIC quality: %d %%\n",
                                      dFreq -> sigQuality.rssi,
                                      dFreq -> sigQuality.snr,
                                      dFreq -> sigQuality.cnr,
                                      dFreq -> sigQuality.ficQuality);

        lastGpsFix = gpsInfo.seconds;

        dabLoggerWrite(tmPtr, &gpsInfo, &(dFreq -> sigQuality));

    }
    else
    {
        printf("  Invalid GPS position\n");
    }

}
