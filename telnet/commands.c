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

extern char pBuf[];
extern int showStatus;
extern int showStatusTime;
extern int showStatusLeft;
extern int showDls;

char *aMode[] =
{
    "dual", "mono", "stereo", "joint stereo"
};

double freqIdToMHz(int id)
{
    double f;

    f = (double)dabShMem -> dabFreq[id].freq / 1000.0;

    return f;
}

void cmdShowInterruptCount(char *ptr)
{
    sprintf(pBuf, "Interrupt count: %ld\n", dabShMem -> interruptCount);
    tputs(pBuf);
}

void cmdShowDls(char *ptr)
{
    if(*ptr != '\0')
    {
        showDls = atoi(ptr);
        if(showDls < 5)
        {
           showDls = 5;
        }
    }

    if(showDls < 0)
    {
        tputs("DLS message display is disabled\n");
    }
    else
    {
        sprintf(pBuf, "Displaying DLS messages for %d seconds\n", showDls);
        tputs(pBuf);
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
    int tmpShowStatus;
    int tmpShowStatusTime;
    int minShowStatus;
    char *cPtr;

    if(*ptr != '\0')
    {
        cPtr = strtok(ptr, " ");
        tmpShowStatusTime = atoi(cPtr);

        cPtr = strtok(NULL, " ");
        if(cPtr == NULL)
        {
            tputs("Missing timeout\n");
        }
        else
        {
            tmpShowStatus = atoi(cPtr);
            minShowStatus = 2 * tmpShowStatusTime;

            if(tmpShowStatus < minShowStatus)
            {
                showStatus = minShowStatus;
            }
            else
            {
                showStatus = tmpShowStatus;
            }

            showStatusTime = tmpShowStatusTime;
            showStatusLeft = showStatusTime;
        }
    }

    if(showStatus < 0)
    {
        tputs("Status updates are disabled\n");
    }
    else
    {
        sprintf(pBuf, "Status updates for every %d seconds for %d seconds\n",
                                                   showStatusTime, showStatus);
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

    bcopy(&(dabShMem -> currentService), &curService, sizeof(DABService));
 
    dabShMem -> time.tm_year = 0;

    sprintf(pBuf, "Scanning %d frequencies\n", dabShMem -> dabFreqs);
    tputs(pBuf);

    for(c = 0; c < dabShMem -> dabFreqs; c++)
    {
        dabCmd.cmd = DABCMD_TUNEFREQ;  
        dabCmd.params.service.Freq = c;
        doCommand(&dabCmd, NULL);
        sprintf(pBuf, "  Frequency: %3.3lf MHz  %s\n", freqIdToMHz(c),
                                                       dabShMem -> Ensemble);
        tputs(pBuf);
    }

    if(curService.CompID != 0 && curService.ServiceID != 0)
    {
        bcopy(&curService, &dabCmd.params.service, sizeof(DABService));

        dabCmd.cmd = DABCMD_TUNEFREQ;
        doCommand(&dabCmd, NULL);

        dabCmd.cmd = DABCMD_TUNE;  
        doCommand(&dabCmd, NULL);

        sprintf(pBuf,
             "  Frequency: %3.3lf MHz  Service: %s, %s (0x%08x/0x%08x)\n",
                                       freqIdToMHz(curService.Freq),
                                       dabShMem -> Ensemble,
                                       dabShMem -> currentService.Label,
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID);
        tputs(pBuf);
    }
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

    if(dabShMem -> dabServiceValid == TRUE)
    {
        sprintf(pBuf, "Ensemble: %s\n", dabShMem -> Ensemble);
        tputs(pBuf);

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

            sprintf(pBuf, "0x%08x   0x%08x   %s\n",
                                      dabShMem -> service[c].ServiceID,
                                      dabShMem -> service[c].CompID,
                                      dabShMem -> service[c].Label);
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

    sprintf(pBuf, "  Mode: %d  Protection: %d  Bit rate: %d k  Num CUs: %d\n",
                                                       cInfo -> serviceMode,
                                                       cInfo -> protectionInfo,
                                                       cInfo -> bitRate,
                                                       cInfo -> numCu);
    tputs(pBuf);
}

void cmdFreq(char *pPtr)
{
    int c;

    if(*pPtr == '\0')
    {
        tputs("Frequency list:-\n");
        for(c = 0; c < dabShMem -> dabFreqs; c++)
        {
            if(dabShMem -> currentService.Freq == c)
            {
                tputs(" *");
            }
            else
            {
                tputs("  ");
            }
            sprintf(pBuf, "%d", c);
            tputs(pBuf);

            sprintf(pBuf, " %3.3lf MHz  %16s - ", freqIdToMHz(c),
                                             dabShMem -> dabFreq[c].ensemble);
            tputs(pBuf);

            if(dabShMem -> dabFreq[c].sigQuality.rssi != 0)
            {
                sprintf(pBuf, "RSSI %d dBuV  SNR %d dB  CNR %d dB\n",
                          dabShMem -> dabFreq[c].sigQuality.rssi,
                          dabShMem -> dabFreq[c].sigQuality.snr,
                          dabShMem -> dabFreq[c].sigQuality.cnr);
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

    if(*pPtr == '\0')
    {
        f = freqIdToMHz(dabShMem -> currentService.Freq);

        sprintf(pBuf, "  Frequency: %3.3lf MHz - %s\n",
                                                      f, dabShMem -> Ensemble);
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

                f = freqIdToMHz(dabShMem -> currentService.Freq);
                sprintf(pBuf, "  Frequency: %3.3lf MHz  Service: %s, %s\n",
                   f, dabShMem -> Ensemble, dabShMem -> currentService.Label);
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

    if(*pPtr == '\0')
    {
        f = (double)dabShMem -> dabFreq[dabShMem -> currentService.Freq].freq / 1000.0;

        sprintf(pBuf, "  Frequency: %3.3lf MHz  Service: %s, %s\n", f,
                       dabShMem -> Ensemble, dabShMem -> currentService.Label);
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
            dabCmd.cmd = DABCMD_TUNEFREQ;  
            doCommand(&dabCmd, NULL);
            sprintf(pBuf, "  Frequency: %3.3lf MHz  Ensemble: %s\n", f,
                                                       dabShMem -> Ensemble);
            tputs(pBuf);
        }
    }
}
