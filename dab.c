#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <strings.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <signal.h>

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

void dabSetValidRssiTime(int newVal)
{
    siSetProperty(SI46XX_DAB_VALID_RSSI_TIME, newVal);
    dabShMem -> validSignal.rssiTime = newVal;
}

void dabSetValidRssiThreshold(int newVal)
{
    siSetProperty(SI46XX_DAB_VALID_RSSI_THRESHOLD, newVal);
    dabShMem -> validSignal.rssiThreshold = newVal;
}

void dabSetValidAcqTime(int newVal)
{
    siSetProperty(SI46XX_DAB_VALID_ACQ_TIME, newVal);
    dabShMem -> validSignal.acqTime = newVal;
}

void dabSetValidSyncTime(int newVal)
{
    siSetProperty(SI46XX_DAB_VALID_SYNC_TIME, newVal);
    dabShMem -> validSignal.syncTime = newVal;
}

void dabSetValidDetectTime(int newVal)
{
    siSetProperty(SI46XX_DAB_VALID_DETECT_TIME, newVal);
    dabShMem -> validSignal.detectTime = newVal;
}

int dabGetValidRssiThreshold()
{
    siGetProperty(SI46XX_DAB_VALID_RSSI_THRESHOLD);
    siResponseN(4);

    return spiBytesTo16(&spiBuf[5]);
}

boolean dabServiceValid(void)
{
    dabFreqType *dFreq;

    siDabDigRadStatus();
    siResponseN(23);

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    dFreq -> sigQuality.rssi = spiBuf[7];
    dFreq -> sigQuality.snr = spiBuf[8];
    dFreq -> sigQuality.ficQuality = spiBuf[9];
    dFreq -> sigQuality.cnr = spiBuf[10];
    dFreq -> sigQuality.fibErrorCount = spiBytesTo16(&spiBuf[11]);

    if((spiBuf[6] & 0x05) == 0x05)
    {
        dFreq -> serviceValid = TRUE;

        dabShMem -> dabServiceValid = TRUE;
        dabShMem -> cuCount = spiBytesTo16(&spiBuf[21]);

    }
    else
    {
        dabShMem -> numberofservices = 0;

        bzero(&(dabShMem -> service[0]),
                                       sizeof(DABService) * DAB_MAX_SERVICES);

        dFreq -> serviceValid = FALSE;
        dabShMem -> dabServiceValid = FALSE;
    }

    return dabShMem -> dabServiceValid;
}

void dabWaitServiceList(void)
{
	uint32_t timeout;

	timeout = 1000;
	do
	{
		milliSleep(4);
		siDabGetEventStatus();
		siResponseN(8);
		timeout--;
		if(timeout == 0)
		{
			command_error |= 0x80;
			break;
		}
	}
	while ((spiBuf[6] & 0x01) == 0x00); //Service List Ready ?
}

void dabParseServiceList(void)
{
	uint16_t i;
	uint16_t j;
	uint16_t offset;
	uint32_t serviceID;
	uint32_t componentID;
	uint16_t numberofcomponents;
	uint16_t listsize;
	uint16_t version;

	listsize = spiBytesTo16(&spiBuf[5]);
	version = spiBytesTo16(&spiBuf[7]);
	(void)listsize; //not used
	(void)version;	//not used

	dabShMem -> numberofservices = spiBuf[9];
	if(dabShMem -> numberofservices > DAB_MAX_SERVICES)
	{
		dabShMem -> numberofservices = DAB_MAX_SERVICES;
	}

	offset = 13;

	for (i = 0; i < dabShMem -> numberofservices; i++)
	{
                serviceID = spiBytesTo32(&spiBuf[offset]);
		componentID = 0;

		numberofcomponents = spiBuf[offset + 5] & 0x0F;

		for (j = 0; j < 16; j++)
		{
			dabShMem -> service[i].Label[j] =
                                                        spiBuf[offset + 8 + j];
		}
		dabShMem -> service[i].Label[16] = '\0';

		offset += 24;

		for (j = 0; j < numberofcomponents; j++)
		{
			if (j == 0)
			{
                                componentID = spiBytesTo32(&spiBuf[offset]);
			}
			offset += 4;
		}

		dabShMem -> service[i].ServiceID = serviceID;
		dabShMem -> service[i].CompID = componentID;
	}

    for (i = 0; i < dabShMem -> numberofservices; i++)
    {
        siDabGetServiceInfo(dabShMem -> service[i].ServiceID);
        siResponseN(26);

        dabShMem -> service[i].programmeType = spiBuf[5];
    }
}

void dabGetDigRadioStatus()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    siDabDigRadStatus();
    siResponseN(23);

    dFreq -> sigQuality.rssi = spiBuf[7];
    dFreq -> sigQuality.snr = spiBuf[8];
    dFreq -> sigQuality.ficQuality = spiBuf[9];
    dFreq -> sigQuality.cnr = spiBuf[10];
    dFreq -> sigQuality.fibErrorCount = spiBytesTo16(&spiBuf[11]);
    dabShMem -> cuCount = spiBytesTo16(&spiBuf[21]);

    dabGetEnsembleInfo();

//    dabGetRssi();
}

void dabGetAcfStatus()
{
    siDabGetAcfStatus();
    siResponseN(12);

    dabShMem -> audioLevelRaw = spiBytesTo16(&spiBuf[7]);
    dabShMem -> audioLevel =
                          20 * log((double)(dabShMem -> audioLevelRaw) / 16383.0);
}

void dabGetEnsembleName(char *ensemble)
{
    int i;

    siDabGetEnsembleInfo();
    siResponseN(29);

    if(spiBuf[7] == '\0')
    {
        sprintf(ensemble, "No service");
    }
    else
    {
        for (i = 0; i < 16; i++)
        {
            ensemble[i] = ((char)spiBuf[7 + i]);
        }
        ensemble[16] = '\0';
    }
}

int dabGetEnsembleInfo(void)
{
    uint16_t len;
    dabFreqType *dFreq;
    int rtn;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    dabGetEnsembleName(dFreq -> ensemble);

    rtn = FALSE;
    if(dabServiceValid() == TRUE)
    {
        dabWaitServiceList();

        siGetDigitalServiceList();
        siResponseN(6);

        len = spiBytesTo16(&spiBuf[5]);

        siResponseN(len + 4);
        dabParseServiceList();

        rtn = TRUE;
    }

    return rtn;
}

void dabShowEnsemble()
{
    int c;
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    printf("Ensemble: %s\n", dFreq -> ensemble);
    printf("Number of services: %d\n", dabShMem -> numberofservices);

    printf("       Serv. ID    Comp. ID    Label\n");
    printf("  ----------------------------------------\n");
    for(c = 0; c < dabShMem -> numberofservices; c++)
    {

        printf("  %02d - %08x    %08x    %s\n",
                 c, dabShMem -> service[c].ServiceID,
                  dabShMem -> service[c].CompID, dabShMem -> service[c].Label); 
    }
}

void dabGetFuncInfo()
{
    siGetFuncInfo();
    siResponseN(12);

    dabShMem -> funcInfo.major = spiBuf[5];
    dabShMem -> funcInfo.minor = spiBuf[6];
    dabShMem -> funcInfo.build = spiBuf[7];
}

void dabGetPartInfo()
{
    siGetPartInfo();
    siResponseN(10);

    dabShMem -> sysInfo.chipRevision = spiBuf[5];
    dabShMem -> sysInfo.romID = spiBuf[6];
    dabShMem -> sysInfo.partNo = spiBytesTo16(&spiBuf[9]);
}

void dabGetSysState()
{
    siGetSysState();
    siResponseN(6);

    dabShMem -> sysInfo.activeImage = spiBuf[5];
}

void dabGetRssi()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    siGetRssi();
    siResponseN(6);

    dFreq -> sigQuality.rssi = spiBytesTo16(&spiBuf[5]) / 256;
}

void dabGetAudioInfo()
{
    siDabGetAudioInfo();
    siResponseN(20);

    dabShMem -> audioInfo.bitRate = spiBytesTo16(&spiBuf[5]);
    dabShMem -> audioInfo.sampleRate = spiBytesTo16(&spiBuf[7]);
    dabShMem -> audioInfo.mode = (spiBuf[9] & 0x03);
    dabShMem -> audioInfo.drcGain = spiBuf[10];
}

void dabGetChannelInfo(DABService *service)
{
    dabGetSubChannelInfo(service -> ServiceID,
                        service -> CompID, &(dabShMem -> dabResp.channelInfo));

}

void dabGetSubChannelInfo(uint32_t serviceID, uint32_t compID,
                                                        channelInfoType *cInfo)
{
    siDabGetSubChannelInfo(serviceID, compID);
    siResponseN(12);

    cInfo -> serviceID = serviceID;
    cInfo -> compID = compID;
    cInfo -> serviceMode = spiBuf[5];
    cInfo -> protectionInfo = spiBuf[6];
    cInfo -> bitRate = spiBytesTo16(&spiBuf[7]);
    cInfo -> numCu = spiBytesTo16(&spiBuf[9]);
    cInfo -> cuAddr = spiBytesTo16(&spiBuf[11]);
}

void dabShowSubChannelInfo(channelInfoType *cInfo)
{
    printf("  Service ID: %08x\n", cInfo -> serviceID);
    printf("  Component ID: %08x\n", cInfo -> compID);
    printf("  Service mode: %d\n", cInfo -> serviceMode);
    printf("  Protection info: %d\n", cInfo -> protectionInfo);
    printf("  Bit rate: %d\n", cInfo -> bitRate);
    printf("  Number of CU's: %d\n", cInfo -> numCu);
    printf("  CU start address: %d\n", cInfo -> cuAddr);
}

void dabStartDigitalService(uint32_t serviceID, uint32_t compID)
{
    siStartDigitalService(serviceID, compID);
    sleep(1); 
    dabGetAudioInfo();
}


void dabStopCurrentDigitalService()
{
    if(dabShMem -> currentService.ServiceID != 0)
    {
        siStopDigitalService(dabShMem -> currentService.ServiceID,
                                            dabShMem -> currentService.CompID);

        bzero(&(dabShMem -> currentService), sizeof(DABService));
        bzero(&(dabShMem -> audioInfo), sizeof(audioInfoType));
        bzero(&(dabShMem -> time), sizeof(struct tm));
    }
}

void dabSaveLastService()
{
    int fd;

    fd = open(DAB_LAST_SERVICE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    write(fd, &(dabShMem -> currentService), sizeof(DABService));
    close(fd);
}

void dabGetLastService(DABService *lastService)
{
    int fd;

    fd = open(DAB_LAST_SERVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("Loading defaults\n");
        fd = open(DAB_LAST_SERVICE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        bzero(lastService, sizeof(DABService));
        lastService -> ServiceID = DAB_DEFAULT_SERVICE_ID;
        lastService -> CompID = DAB_DEFAULT_COMP_ID;
        lastService -> Freq = DAB_DEFAULT_FREQ;
        write(fd, lastService, sizeof(DABService));
    }
    else
    {
        printf("Using settings from last time\n");
        read(fd, lastService, sizeof(DABService));
    }

    printf("Service ID: 0x%08x  Component ID: 0x%08x\n", lastService -> ServiceID, lastService -> CompID);
    close(fd);
}

void dabInterrupt(int gpio, int level, unsigned int ticks)
{
    uint8_t status0;
    uint8_t dataSrc;
    uint16_t byteCount;
    uint16_t compID;
    uint16_t servID;
    uint8_t buffCount;
    uint8_t header1;
    uint8_t header2;
    uint8_t *dlsData;

    if(dabLoggerRunning() == TRUE)
    {
        return;
    }

    dabShMem -> interruptCount++;

    siResponse();
    status0 = spiBuf[1];
    if((status0 & 0x10) == 0x10)
    {
        // DLS data available

        siGetDigitalServiceData();
        siResponseN(20);

        dataSrc = (spiBuf[8] >> 6) & 0x03;
        servID = spiBytesTo32(&spiBuf[9]);
        compID = spiBytesTo32(&spiBuf[13]);
        byteCount = spiBytesTo16(&spiBuf[19]);

        buffCount = spiBuf[6];

        if(byteCount < (SPI_BUFF_SIZE - 24))
        {
            siResponseN(byteCount + 24);
        }
        else
        {
            siResponseN(SPI_BUFF_SIZE - 1);
        }

        if(dataSrc == 0x02)
        {
            // DLS data received

            dabShMem -> serviceDataMs = timeMillis();

            header1 = spiBuf[25];
            header2 = spiBuf[26];

            if((header1 & 0x10) == 0x10)
            {
                // DLS tags
                printf("  [%ld]  DLS tags received\n", timeMillis());
            }
            else
            {
                // DLS message

                dlsData = &spiBuf[27];
                byteCount = byteCount - 2 - 1;

                printf("  [%ld]  %d DLS message bytes for 0x%08x/0x%08x.\n",
                                      timeMillis(), byteCount, servID, compID);
                bzero(dabShMem -> serviceData, DAB_MAX_SERVICEDATA_LEN);
                bcopy(dlsData, dabShMem -> serviceData, byteCount);
                printf("  [%ld]  [%s]\n",
                                        timeMillis(), dabShMem -> serviceData);
            }
        }
    }
}

void dabResetHardware()
{
    dabshieldPowerup();
    dabshieldReset();

    siReset();
    siInitDab();
}

void dabBegin()
{
    printf("Radio initialising...\n");
    dabResetHardware();

    dabGetPartInfo();
    dabGetFuncInfo();
    dabGetSysState();

    printf("  Found Si%d Rev %d, ROM ID %d.\n",
            dabShMem -> sysInfo.partNo, dabShMem -> sysInfo.chipRevision,
                                                    dabShMem -> sysInfo.romID);

    printf("  Active image %d, version %d.%d.%d\n",
                          dabShMem -> sysInfo.activeImage,
                          dabShMem -> funcInfo.major,
                          dabShMem -> funcInfo.minor,
                          dabShMem -> funcInfo.build);
}

void dabResetRadio()
{
    DABService currentService;

    bcopy(&(dabShMem -> currentService), &currentService, sizeof(DABService));
    dabShMem -> time.tm_year = 0;

    dabBegin();
    dabTuneFreq(&currentService);
    if(dabServiceValid() == TRUE)
    {
        dabTune(&currentService);
    }

}

void dabCommand()
{
    if(dabShMem -> dabCmd.cmd != DABCMD_NONE)
    {
        dabShMem -> dabCmd.rtn = DABRET_BUSY;

        printf("Command 0x%04x received from PID - %ld\n",
                           dabShMem -> dabCmd.cmd, dabShMem -> dabCmd.userPid);

        switch(dabShMem -> dabCmd.cmd)
        {
            case DABCMD_TUNE:
               printf("  ** Change service\n");
               dabTune(&(dabShMem -> dabCmd.params.service));
               break;

            case DABCMD_TUNEFREQ:
               printf("  ** Change frequency\n");
               dabTuneFreq(&(dabShMem -> dabCmd.params.service));
               break;

            case DABCMD_START_SERVICE:
               printf("  ** Start service\n");
               break;

            case DABCMD_GETCHANNEL_INFO:
               printf("  **Get sub-channel information\n");
               dabGetChannelInfo(&(dabShMem -> dabCmd.params.service));
               break;

            case DABCMD_SAVE:
               printf("  **Save current service\n");
               dabSaveLastService();
               break;

            case DABCMD_RESET:
               printf("  **Reset radio\n");
               dabResetRadio();
               break;

            case DABCMD_SETSQUELCH:
               printf("  **Set squelch level\n");
               dabSetValidRssiThreshold(dabShMem -> dabCmd.params.squelch);
               break;

            case DABCMD_SETRSSITIME:
               printf("  **Set valid RSSI time\n");
               dabSetValidRssiTime(dabShMem -> dabCmd.params.rssiTime);
               break;

            case DABCMD_SETACQTIME:
               printf("  **Set valid ACQ time\n");
               dabSetValidAcqTime(dabShMem -> dabCmd.params.acqTime);
               break;

            case DABCMD_SETSYNCTIME:
               printf("  **Set valid sync time\n");
               dabSetValidSyncTime(dabShMem -> dabCmd.params.syncTime);
               break;

            case DABCMD_SETDETECTTIME:
               printf("  **Set valid detect time\n");
               dabSetValidDetectTime(dabShMem -> dabCmd.params.detectTime);
               break;

            case DABCMD_EXIT:
               printf("  **Exit\n");
               dabDone = TRUE;
               break;

            default:
               printf("  **Illegal command\n");
        }

        dabShMem -> dabCmd.userPid = 0;
        dabShMem -> dabCmd.cmd = DABCMD_NONE;
        dabShMem -> dabCmd.rtn = DABRET_READY;
    }
}

void dabGetTime()
{
    siGetTime();

    siResponseN(12);

    bzero(&(dabShMem -> time), sizeof(struct tm));

    dabShMem -> time.tm_year = spiBytesTo16(&spiBuf[5]) - 1900;
    dabShMem -> time.tm_mon = spiBuf[7] - 1;
    dabShMem -> time.tm_mday = spiBuf[8];
    dabShMem -> time.tm_hour = spiBuf[9];
    dabShMem -> time.tm_min = spiBuf[10];
    dabShMem -> time.tm_sec = spiBuf[11];
}

void dabShowTime()
{
    char strBuf[80];

    if(dabShMem -> time.tm_year > 0)
    {
        strftime(strBuf, 80, "  Date: %d %B %Y, %H:%M", &(dabShMem -> time));
        printf("%s\n", strBuf);
    }
    else
    {
        printf("  No time information available\n");
    }
}

void dabHandleTimers()
{
    int c;

    for(c = 0; c < DAB_MAXTIMERS; c++)
    {
        if(dabTimers[c].enabled == TRUE)
        {
            if(dabTimers[c].runAlways == TRUE ||
              (dabTimers[c].runAlways == FALSE  && dabLoggerRunning() == FALSE))
            {
                if(dabTimers[c].count)
                {
                    dabTimers[c].count--;
                    if(dabTimers[c].count == 0)
                    {
                        dabTimers[c].count = dabTimers[c].reload;
                        dabTimers[c].handlerFn();
                    }
                }
            }
        }
    }
}

void dabShowSignal()
{ 
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    printf("  RSSI: %d dBuV  SNR: %d dB  CNR: %d dB  FIC Quality: %d %%  FIB errors: %d\n",
                                      dFreq -> sigQuality.rssi,
                                      dFreq -> sigQuality.snr,
                                      dFreq -> sigQuality.cnr,
                                      dFreq -> sigQuality.ficQuality,
                                      dFreq -> sigQuality.fibErrorCount);
}

void dabShowServiceSummary()
{
    dabFreqType *dFreq;
    char block[6];

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    freqIdToBlock(dabShMem -> currentService.Freq, block);
    printf("  Frequency: %s, %3.3lf MHz  Service: %s, %s\n", block,
           currentFreq(), dFreq -> ensemble, dabShMem -> currentService.Label);
}

void dabShowState()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    printf("  Ensemble: %s\n", dFreq -> ensemble);
    printf("  Frequency: %3.3lf MHz\n", currentFreq());
    printf("  Service: %s (0x%08x/0x%08x)\n",
                                       dabShMem -> currentService.Label,
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID);
    printf("  RSSI: %d dBuV\n", dFreq -> sigQuality.rssi);
    printf("  SNR: %d dB   CNR: %d dB\n",
                 dFreq -> sigQuality.snr, dFreq -> sigQuality.cnr);
    printf("  FIC quality: %d %%\n", dFreq -> sigQuality.ficQuality);
    printf("  FIB error count: %d\n", dFreq -> sigQuality.fibErrorCount);  
    printf("  CU count: %d\n", dabShMem -> cuCount);

    printf("  Bit rate: %d\n", dabShMem -> audioInfo.bitRate);
    printf("  Sample rate: %d\n", dabShMem -> audioInfo.sampleRate);
    printf("  Dynamic range: %d\n", dabShMem -> audioInfo.drcGain);
    printf("  Mode: %d\n", dabShMem -> audioInfo.mode);
}

void dabTuneFreq(DABService *service)
{
    dabStopCurrentDigitalService();
    siDabTuneFreq(service -> Freq);

    bzero(&(dabShMem -> currentService), sizeof(DABService));
    dabShMem -> currentService.Freq = service -> Freq;

    if(dabGetEnsembleInfo() == TRUE)
    {
        dabGetDigRadioStatus();
    }
}

void dabTune(DABService *service)
{
    dabGetEnsembleInfo();
//    dabGetRssi();
    dabStartDigitalService(service -> ServiceID, service -> CompID);
    dabGetDigRadioStatus();

    dabShowServiceSummary();
}

void dabGetBer()
{
    uint16_t audioLevel;
    uint16_t comfortNoiseLevel;
    uint16_t propBerLimit;
    uint16_t propNoiseLevel;

    siGetProperty(SI46XX_DAB_ACF_CMFTNOISE_BER_LIMIT);
    siResponseN(4);
    propBerLimit = spiBytesTo16(&spiBuf[5]);

    siGetProperty(SI46XX_DAB_ACF_CMFTNOISE_LEVEL);
    siResponseN(4);
    propNoiseLevel = spiBytesTo16(&spiBuf[5]);

    do
    {
        siDabGetAcfStatus();
        siResponseN(12);

	audioLevel= spiBytesTo16(&spiBuf[7]);
	comfortNoiseLevel= spiBytesTo16(&spiBuf[9]);    
    }
    while(comfortNoiseLevel < 999);

    siSetProperty(SI46XX_DAB_ACF_CMFTNOISE_BER_LIMIT, propBerLimit);
}

void dabSigint(int signum)
{
    printf("Exiting on SIGINT...\n");
    dabShMem -> engineVersion = 0;
    dabShMem -> engineState = DAB_ENGINE_NOTREADY;
    exit(0);
}

void dabMain()
{
    DABService lastService;
    struct sigaction sigintAction;

    sigintAction.sa_handler = dabSigint;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_flags = 0;
    sigaction(SIGINT, &sigintAction, NULL);

    dabGetLastService(&lastService);

    dabBegin();

    siSetProperty(SI46XX_DAB_VALID_RSSI_TIME, DAB_VALID_RSSI_TIME);
    dabSetValidRssiThreshold(DAB_VALID_RSSI_THRESHOLD);
    dabSetValidRssiTime(DAB_VALID_RSSI_TIME);
    dabSetValidAcqTime(DAB_VALID_ACQ_TIME);
    dabSetValidSyncTime(DAB_VALID_SYNC_TIME);
    dabSetValidDetectTime(DAB_VALID_DETECT_TIME);

    dabTuneFreq(&lastService);
    if(dabServiceValid() == TRUE)
    {
        dabTune(&lastService);
    }

    dabInitLogger();

    dabShMem -> engineState = DAB_ENGINE_READY;

    printf("Ready...\n");

    dabDone = FALSE;
    while(dabDone == FALSE)
    {
        dabCommand();

        if(dabDone == FALSE)
        {
            dabControlLogger();
            dabHandleTimers();
            milliSleep(DAB_TICKTIME);
        }
    }

    dabShMem -> engineState = DAB_ENGINE_NOTREADY;
}
