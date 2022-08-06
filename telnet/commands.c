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
extern int showStatusTime;

char *aMode[] =
{
    "dual", "mono", "stereo", "joint stereo"
};

void cmdResetRadio(char *ptr)
{
    dabCmdType dabCmd;

    dabCmd.cmd = DABCMD_RESET;
    doCommand(&dabCmd, NULL);

    tputs("Radio reset\n");
}

void cmdShowStatusCont(char *ptr)
{
    if(*ptr != '\0')
    {
        showStatusTime = atoi(ptr);
        alarm(showStatusTime);
    }

    sprintf(pBuf, "Continual status update time: %d s\n", showStatusTime);
    tputs(pBuf);
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
    double f;

    bcopy(&(dabShMem -> currentService), &curService, sizeof(DABService));
 
    sprintf(pBuf, "Scanning %d frequencies\n", dabShMem -> dabFreqs);
    tputs(pBuf);

    for(c = 0; c < dabShMem -> dabFreqs; c++)
    {
        f = (double)dabShMem -> dabFreq[c].freq / 1000.0;
        dabCmd.cmd = DABCMD_TUNEFREQ;  
        dabCmd.params.service.Freq = c;
        doCommand(&dabCmd, NULL);
        sprintf(pBuf, "  Frequency: %3.3lf MHz  %s\n", f,
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

        f = (double)dabShMem -> dabFreq[curService.Freq].freq / 1000.0;
        sprintf(pBuf,
             "  Frequency: %3.3lf MHz  Service: %s, %s (0x%08x/0x%08x)\n",
                                       f,
                                       dabShMem -> Ensemble,
                                       dabShMem -> currentService.Label,
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID);
        tputs(pBuf);
    }
}

void cmdTime(char *pPtr)
{
    if(dabShMem -> time.tm_mon < 0)
    {
        strcpy(pBuf, "  Date not available\n");
    }
    else
    {
        strftime(pBuf, 80, "  Date: %d %B %Y, %H:%M\n", &(dabShMem -> time));
    }
    tputs(pBuf);
}


void cmdRssi(char *pPtr)
{
    sprintf(pBuf,
       "  RSSI: %d dBuV  SNR: %d dB  CNR: %d dB"
       "  FIC Quality: %d %%  FIB errors: %d\n",
                                     (dabShMem -> signalQuality.rssi / 256),
                                      dabShMem -> signalQuality.snr,
                                      dabShMem -> signalQuality.cnr,
                                      dabShMem -> signalQuality.ficQuality,
                                      dabShMem -> signalQuality.fibErrorCount);
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

            sprintf(pBuf, " %3.3lf MHz  %s\n",
                                 (double)dabShMem -> dabFreq[c].freq / 1000.0,
                                 dabShMem -> dabFreq[c].ensemble);
            tputs(pBuf);
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
        f = (double)dabShMem -> dabFreq[dabShMem -> currentService.Freq].freq / 1000.0;

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

                f = (double)dabShMem -> dabFreq[dabShMem -> currentService.Freq].freq / 1000.0;
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
            f = (double)dabShMem -> dabFreq[dabCmd.params.service.Freq].freq / 1000.0;
            dabCmd.cmd = DABCMD_TUNEFREQ;  
            doCommand(&dabCmd, NULL);
            sprintf(pBuf, "  Frequency: %3.3lf MHz  Ensemble: %s\n", f,
                                                       dabShMem -> Ensemble);
            tputs(pBuf);
        }
    }
}
