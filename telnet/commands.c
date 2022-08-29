#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../types.h"
#include "../dabcmd.h"
#include "../dabshmem.h"
#include "commands.h"
#include "telnetd.h"
#include "cli.h"

#include "globals.h"

double freqIdToMHz(int id)
{
    double f;

    f = (double)dabShMem -> dabFreq[id].freq / 1000.0;

    return f;
}

void freqIdToBlock(int id, char *block)
{
    int x;
    int y;

    x = (id / 4) + 5;
    y = (id % 4) + 65;

    sprintf(block, "%d%c", x, y);
}

void cmdSquelch(char *ptr)
{
    int sqLevel;
    dabCmdType dabCmd;

    if(*ptr != '\0')
    {
        sqLevel = atoi(ptr);
        if(sqLevel < 127 && sqLevel > 0)
        {
            dabCmd.cmd = DABCMD_SETSQUELCH;
            dabCmd.params.squelch = sqLevel;
            doCommand(&dabCmd, NULL);
        }
        else
        {
            tputs("Bad squelch level\n");
        }
    }

    sprintf(pBuf, "Squelch level - %d dBuV\n", dabShMem -> validSignal.rssiThreshold);
    tputs(pBuf);
}

void cmdShowInterruptCount(char *ptr)
{
    sprintf(pBuf, "Interrupt count: %ld\n", dabShMem -> interruptCount);
    tputs(pBuf);
}

void cmdShowDls(char *ptr)
{
    if(showDls == FALSE)
    {
        showDls = TRUE;
        tputs("DLS message display enabled\n");;
    }
    else
    {
        showDls = FALSE;
        tputs("DLS message display disabled\n");
    }
}

void cmdResetRadio(char *ptr)
{
    dabCmdType dabCmd;

    dabCmd.cmd = DABCMD_RESET;
    doCommand(&dabCmd, NULL);

    tputs("Radio reset\n");
}

void cmdShowStatusCont(char *ptr)
{
    char *cPtr;

    if(*ptr != '\0')
    {
        cPtr = strtok(ptr, " ");
        showStatusTime = atoi(cPtr);
        showStatusLeft = showStatusTime;
    }

    if(showStatusTime == 0)
    {
        tputs("Status updates are disabled\n");
    }
    else
    {
        sprintf(pBuf, "Status updates for every %d seconds\n",
                                                   showStatusTime);
        tputs(pBuf);
    }
}

void cmdVersion(char *pPtr)
{
    int major;
    int minor;

    major = (dabShMem -> engineVersion & 0xff00) >> 8;
    minor = dabShMem -> engineVersion & 0x00ff;

    sprintf(pBuf, "  Engine version: %d.%d\n", major, minor);
    tputs(pBuf);
}

void cmdSave(char *pPtr)
{
    dabCmdType dabCmd;

    if(dabShMem -> dabServiceValid == TRUE)
    {
        dabCmd.cmd = DABCMD_SAVE;
        doCommand(&dabCmd, NULL);

        tputs("Service saved\n");
    }
    else
    {
        tputs("Service not valid - not saved\n");
    }
}

void cmdScan(char *pPtr)
{
    dabCmdType dabCmd;
    DABService curService;
    int c;
    dabFreqType *dFreq;
    char block[6];

    bcopy(&(dabShMem -> currentService), &curService, sizeof(DABService));
 
    dabShMem -> time.tm_year = 0;

    sprintf(pBuf, "Scanning %d frequencies\n", dabShMem -> dabFreqs);
    tputs(pBuf);

    for(c = 0; c < dabShMem -> dabFreqs; c++)
    {
        dabCmd.cmd = DABCMD_TUNEFREQ;  
        dabCmd.params.service.Freq = c;
        doCommand(&dabCmd, NULL);

        dFreq = &(dabShMem -> dabFreq[c]);
        freqIdToBlock(c, block);
        sprintf(pBuf, "  Frequency: %s %3.3lf MHz  %s\n", block,
                                            freqIdToMHz(c), dFreq -> ensemble);
        tputs(pBuf);
    }

    bcopy(&curService, &dabCmd.params.service, sizeof(DABService));

    dabCmd.cmd = DABCMD_TUNEFREQ;
    doCommand(&dabCmd, NULL);

    if(curService.CompID != 0 && curService.ServiceID != 0)
    {
        dabCmd.cmd = DABCMD_TUNE;  
        doCommand(&dabCmd, NULL);
    }

    dFreq = &(dabShMem -> dabFreq[curService.Freq]);
    freqIdToBlock(curService.Freq, block);
    sprintf(pBuf,
             "  Frequency: %s %3.3lf MHz  Service: %s, %s (0x%08x/0x%08x)\n",
                                       block,
                                       freqIdToMHz(curService.Freq),
                                       dFreq -> ensemble,
                                       dabShMem -> currentService.Label,
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID);
    tputs(pBuf);
}

void cmdTime(char *pPtr)
{
    if(dabShMem -> time.tm_year > 0)
    {
        strftime(pBuf, 80, "  Date: %d %B %Y, %H:%M\n", &(dabShMem -> time));
    }
    else
    {
        strcpy(pBuf, "  Date not available\n");
    }
    tputs(pBuf);
}


void cmdRssi(char *pPtr)
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    sprintf(pBuf,
       "  RSSI: %d dBuV  SNR: %d dB  CNR: %d dB"
       "  FIC Quality: %d %%  FIB errors: %d\n",
                                      dFreq -> sigQuality.rssi,
                                      dFreq -> sigQuality.snr,
                                      dFreq -> sigQuality.cnr,
                                      dFreq -> sigQuality.ficQuality,
                                      dFreq -> sigQuality.fibErrorCount);
    tputs(pBuf);
}

void cmdAudioInfo(char *pPtr)
{
    sprintf(pBuf, "  Bit rate: %d kbps  Sample rate: %d Hz  Mode: ",
                                            dabShMem -> audioInfo.bitRate,
                                            dabShMem -> audioInfo.sampleRate);

    if(dabShMem -> audioInfo.mode > 3)
    {
        strcat(pBuf, "?");
    }
    else
    {
        strcat(pBuf, aMode[dabShMem -> audioInfo.mode]);
    }
    strcat(pBuf, "\n");
    tputs(pBuf);
}

void cmdEnsemble(char *pPtr)
{
    int c;
    dabFreqType *dFreq;
    int pty;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    if(dabShMem -> dabServiceValid == TRUE)
    {
        sprintf(pBuf, "Ensemble: %s\n\n", dFreq -> ensemble);
        tputs(pBuf);

        sprintf(pBuf, "  Component ID\tService ID\tType\tPTY\tProgramme name\n");
        tputs(pBuf);

        for(c = 0; c < 65; c++)
        {
            tputs("-");
        }
        tputs("\n");

        for(c = 0; c < dabShMem -> numberofservices; c++)
        {
            if(dabShMem -> currentService.CompID ==
                 dabShMem -> service[c].CompID &&
               dabShMem -> currentService.ServiceID ==
                 dabShMem -> service[c].ServiceID)
            {
                tputs(" *");
            }
            else
            {
                tputs("  ");
            }

            sprintf(pBuf, "0x%08x\t0x%08x\t",
                                      dabShMem -> service[c].ServiceID,
                                      dabShMem -> service[c].CompID);
            tputs(pBuf);

            if((dabShMem -> service[c].programmeType & 0x01) == 0x01)
            {
                tputs("data\t\t");
            }
            else
            {
                pty = (dabShMem -> service[c].programmeType >> 1) & 0x1f;
                sprintf(pBuf, "audio\t%s\t", ptyNames[pty]);
                tputs(pBuf);
            }

            sprintf(pBuf, "%s\n", dabShMem -> service[c].Label);
            tputs(pBuf);
        }
    }
    else
    {
        sprintf(pBuf, "No DAB service!\n");
        tputs(pBuf);
    }
}

void cmdGetChannelInfo(char *cPtr)
{
    dabCmdType dabCmd;
    dabCmdRespType resp;
    channelInfoType *cInfo;

    bcopy(&(dabShMem -> currentService), &dabCmd.params.service,
                                                           sizeof(DABService));

    dabCmd.cmd = DABCMD_GETCHANNEL_INFO;  
    doCommand(&dabCmd, &resp);

    cInfo = &resp.channelInfo;

    tputs("  Mode: ");
    if(cInfo -> serviceMode > 8)
    {
        sprintf(pBuf, "%d", cInfo -> serviceMode);
        tputs(pBuf);
    }
    else
    {
        tputs(serviceModeNames[cInfo -> serviceMode]);
    }
  
    tputs("  Protection: "); 
    if(cInfo -> protectionInfo > 13)
    {
        sprintf(pBuf, "%d", cInfo -> protectionInfo);
    }
    else
    {
        if(cInfo -> protectionInfo < 6)
        {
            sprintf(pBuf, "UEP %d", cInfo -> protectionInfo);
        }
        else
        {
            if(cInfo -> protectionInfo < 10)
            {
                sprintf(pBuf, "EEP A-%d", cInfo -> protectionInfo - 5);
            }
            else
            {
                sprintf(pBuf, "EEP B-%d", cInfo -> protectionInfo - 9);
            }            
        } 
        tputs(pBuf);
    } 

    sprintf(pBuf, "  Bit rate: %d k  Num CUs: %d\n", cInfo -> bitRate,
                                                              cInfo -> numCu);
    tputs(pBuf);
}

void cmdFreq(char *pPtr)
{
    int c;
    char block[6];
    int d;

    if(*pPtr == '\0')
    {
        for(c = 0; c < dabShMem -> dabFreqs; c++)
        {
            if((c % 20) == 0)
            {
                sprintf(pBuf,"\n\t\tFrequency\tEnsemble\t\tRSSI\tSNR\tCNR\tFIC\tFIB errors\n");
                tputs(pBuf);
                sprintf(pBuf,"\t\tMHz\t\t\t\t\tdBuV\tdB\tdB\t%%\tper second\n");
                tputs(pBuf);
                for(d = 0; d < 100; d++)
                {
                    tputs("-");
                }
                tputs("\n");
            }


            if(dabShMem -> currentService.Freq == c)
            {
                tputs(" *");
            }
            else
            {
                tputs("  ");
            }
            sprintf(pBuf, "%d\t", c);
            tputs(pBuf);

            freqIdToBlock(c, block);
            sprintf(pBuf, " %s\t%3.3lf\t\t%-16s\t", block, freqIdToMHz(c),
                                             dabShMem -> dabFreq[c].ensemble);
            tputs(pBuf);

            if(dabShMem -> dabFreq[c].sigQuality.rssi != 0)
            {
                sprintf(pBuf, "%d\t%d\t%d\t%d\t%1.0lf\n",
                          dabShMem -> dabFreq[c].sigQuality.rssi,
                          dabShMem -> dabFreq[c].sigQuality.snr,
                          dabShMem -> dabFreq[c].sigQuality.cnr,
                          dabShMem -> dabFreq[c].sigQuality.ficQuality,
       dabShMem -> dabFreq[c].sigQuality.fibErrorCount / dabShMem -> loggerMeasureSeconds);
                tputs(pBuf);
            }
            else
            {
                tputs("\n");
            }
        }
    }
    else
    {
        cmdTuneFreq(pPtr);
    }
}

void cmdTune(char *pPtr)
{
    dabCmdType dabCmd;
    char *cPtr;
    char *endPtr;
    double f;
    dabFreqType *dFreq;
    char block[6];

    if(*pPtr == '\0')
    {
        dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);
        f = freqIdToMHz(dabShMem -> currentService.Freq);

        freqIdToBlock(dabShMem -> currentService.Freq, block);
        sprintf(pBuf, "  Frequency: %s %3.3lf MHz - %s\n", block,
                                                      f, dFreq -> ensemble);
        tputs(pBuf);

        sprintf(pBuf, "  Service ID: 0x%08x  Component ID: 0x%08x - %s\n",
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID,
                                       dabShMem -> currentService.Label);
        tputs(pBuf);

    }
    else
    {
        cPtr = strtok(pPtr, " ");
        if(cPtr == NULL)
        {
            tputs("Missing service ID\n");
        }
        else
        {
            dabCmd.params.service.ServiceID = strtol(cPtr, &endPtr, 16);

            cPtr = strtok(NULL, " ");
            if(cPtr == NULL)
            {
                tputs("Missing component ID\n");
            }
            else
            {
                dabCmd.params.service.CompID = strtol(cPtr, &endPtr, 16);

                dabCmd.cmd = DABCMD_TUNE;  
                doCommand(&dabCmd, NULL);

                dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);
                f = freqIdToMHz(dabShMem -> currentService.Freq);
                freqIdToBlock(dabShMem -> currentService.Freq, block);
                sprintf(pBuf, "  Frequency: %s %3.3lf MHz  Service: %s, %s\n",
                          block, f,
                          dFreq -> ensemble, dabShMem -> currentService.Label);
                tputs(pBuf);
            }
        }
    }
}

void cmdTuneFreq(char *pPtr)
{
    dabCmdType dabCmd;
    char *cPtr;
    double f;
    dabFreqType *dFreq;
    char block[6];

    if(*pPtr == '\0')
    {
        dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);
        f = freqIdToMHz(dabShMem -> currentService.Freq);
        freqIdToBlock(dabShMem -> currentService.Freq, block);

        sprintf(pBuf, "  Frequency: %s %3.3lf MHz  Service: %s, %s\n", block,
                       f, dFreq -> ensemble, dabShMem -> currentService.Label);
        tputs(pBuf);
    }
    else
    {
        cPtr = strtok(pPtr, " ");
        dabCmd.params.service.Freq = atoi(cPtr);

        if(dabCmd.params.service.Freq >= dabShMem -> dabFreqs)
        {
            tputs("Invalid frequency\n");
        }
        else
        {
            f = freqIdToMHz(dabCmd.params.service.Freq);
            freqIdToBlock(dabCmd.params.service.Freq, block);
            dabCmd.cmd = DABCMD_TUNEFREQ;  
            doCommand(&dabCmd, NULL);
            dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);
            sprintf(pBuf, "  Frequency: %3.3lf MHz  Ensemble: %s\n", f,
                                                            dFreq -> ensemble);
            tputs(pBuf);
        }
    }
}

void cmdValidAcqTime(char *ptr)
{
    int acqTime;
    dabCmdType dabCmd;

    if(*ptr != '\0')
    {
        acqTime = atoi(ptr);
        dabCmd.cmd = DABCMD_SETACQTIME;
        dabCmd.params.acqTime = acqTime;
        doCommand(&dabCmd, NULL);
    }

    sprintf(pBuf, "Valid ACQ time - %d ms\n", dabShMem -> validSignal.acqTime);
    tputs(pBuf);
}

void cmdValidRssiTime(char *ptr)
{
    int rssiTime;
    dabCmdType dabCmd;

    if(*ptr != '\0')
    {
        rssiTime = atoi(ptr);
        dabCmd.cmd = DABCMD_SETRSSITIME;
        dabCmd.params.rssiTime = rssiTime;
        doCommand(&dabCmd, NULL);
    }

    sprintf(pBuf, "Valid RSSI time - %d ms\n", dabShMem -> validSignal.rssiTime);
    tputs(pBuf);
}

void cmdValidSyncTime(char *ptr)
{
    int syncTime;
    dabCmdType dabCmd;

    if(*ptr != '\0')
    {
        syncTime = atoi(ptr);
        dabCmd.cmd = DABCMD_SETSYNCTIME;
        dabCmd.params.syncTime = syncTime;
        doCommand(&dabCmd, NULL);
    }

    sprintf(pBuf, "Valid sync time - %d ms\n", dabShMem -> validSignal.syncTime);
    tputs(pBuf);
}

void cmdValidDetectTime(char *ptr)
{
    int detectTime;
    dabCmdType dabCmd;

    if(*ptr != '\0')
    {
        detectTime = atoi(ptr);
        dabCmd.cmd = DABCMD_SETDETECTTIME;
        dabCmd.params.detectTime = detectTime;
        doCommand(&dabCmd, NULL);
    }

    sprintf(pBuf, "Valid detect time - %d ms\n", dabShMem -> validSignal.detectTime);
    tputs(pBuf);
}

