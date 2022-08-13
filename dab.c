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

unsigned int spi;
unsigned char spiBuf[SPI_BUFF_SIZE];

void dabGetDigRadioStatus(void);
void dabGetRssi(void);
void dabGetTime(void);
void dabShowTime(void);
void dabShowSignal(void);
void dabShowServiceSummary(void);

extern uint32_t dab_freq[];
extern int dab_freqs;

dabTimerType dabTimers[] = 
{
    { DAB_RSSI_TICKS, DAB_RSSI_TICKS, dabGetDigRadioStatus, "RSSI", FALSE,TRUE },
    { DAB_LOGGER_TICKS, DAB_LOGGER_TICKS, dabLogger, "LOGR", TRUE, TRUE },
    { DAB_TIME_TICKS, DAB_TIME_TICKS, dabGetTime, "TIME", FALSE, TRUE },
    { DAB_SHOWTIME_TICKS, DAB_SHOWTIME_TICKS, dabShowTime, "SHOWTIME", FALSE, TRUE },
    { DAB_SHOWSERV_TICKS, DAB_SHOWSERV_TICKS, dabShowServiceSummary, "SHOWSERV", FALSE, TRUE },
    { DAB_SHOWSIG_TICKS, DAB_SHOWSIG_TICKS, dabShowSignal, "SHOWSIG", FALSE, TRUE }
};

#define DAB_MAXTIMERS (sizeof(dabTimers) / sizeof(dabTimerType))

typedef unsigned char boolean;

uint8_t command_error;

int dabDone;

boolean dabServiceValid(void)
{
    dabFreqType *dFreq;

    siDabDigRadStatus();
    siResponseN(23);

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    if(((spiBuf[6] & 0x01) == 0x01) && (spiBuf[7] > 0x20) && (spiBuf[9] > 25))
    {
        dFreq -> serviceValid = TRUE;
        strcpy(dFreq -> ensemble, dabShMem -> Ensemble);

        dabShMem -> dabServiceValid = TRUE;
    }
    else
    {
        sprintf(dabShMem -> Ensemble, "No service");
        dabShMem -> numberofservices = 0;

        bzero(&(dabShMem -> service[0]),
                                       sizeof(DABService) * DAB_MAX_SERVICES);

        strcpy(dFreq -> ensemble, "No service");
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
}

void dabGetDigRadioStatus()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    siDabDigRadStatus();
    siResponseN(23);

    dFreq -> sigQuality.snr = spiBuf[8];
    dFreq -> sigQuality.ficQuality = spiBuf[9];
    dFreq -> sigQuality.cnr = spiBuf[10];
    dFreq -> sigQuality.fibErrorCount = spiBytesTo16(&spiBuf[11]);
    dabShMem -> cuCount = spiBytesTo16(&spiBuf[21]);

    dabGetRssi();
}

void dabGetAcfStatus()
{
    siDabGetAcfStatus();
    siResponseN(12);

    dabShMem -> audioLevelRaw = spiBytesTo16(&spiBuf[7]);
    dabShMem -> audioLevel =
                          20 * log((double)(dabShMem -> audioLevelRaw) / 16383.0);
}

void dabGetEnsembleInfo(void)
{
	uint8_t i;

	siDabDigRadStatus();
	siResponseN(23);
	if (dabServiceValid() == TRUE)
	{
		dabWaitServiceList();

		siDabGetEnsembleInfo();
		siResponseN(29);

		for (i = 0; i < 16; i++)
		{
			dabShMem -> Ensemble[i] = ((char)spiBuf[7 + i]);
		}
		dabShMem -> Ensemble[16] = '\0';

		siGetDigitalServiceList();
		siResponseN(6);

		int16_t len = spiBytesTo16(&spiBuf[5]);

		siResponseN(len + 4);
		dabParseServiceList();
	}
	else
	{
            //No services
	}
}

void dabShowEnsemble()
{
    int c;

    printf("Ensemble: %s\n", dabShMem -> Ensemble);
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
    printf("  [%ld]  Interrupt count - %ld\n",
                                     timeMillis(), dabShMem -> interruptCount);

    siResponse();
    status0 = spiBuf[1];
    printf("  [%ld]  Status - 0x%02x\n", timeMillis(), status0);
    if((status0 & 0x10) == 0x10)
    {
        // DLS data available

        siGetDigitalServiceData();
        siResponseN(20);

        printf("  [%ld]  Interrupt mask - 0x%02x\n", timeMillis(), spiBuf[5]);

        dataSrc = (spiBuf[8] >> 6) & 0x03;
        servID = spiBytesTo32(&spiBuf[9]);
        compID = spiBytesTo32(&spiBuf[13]);
        byteCount = spiBytesTo16(&spiBuf[19]);

        buffCount = spiBuf[6];
        printf("  [%ld]  Buffer count - %d\n", timeMillis(), buffCount);

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

            printf("  [%ld]  header 1 - 0x%02x, header 2 - 0x%02x\n",
                                               timeMillis(), header1, header2);

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
        else
        {
            printf("  [%ld]  Received %d type %d bytes for 0x%08x/0x%08x.\n",
                             timeMillis(), byteCount, dataSrc, servID, compID);
        }
    }
    else
    {
        printf("  [%ld]  Unhandled interrupt %02X %02X %02X %02X\n",
                    timeMillis(), spiBuf[1], spiBuf[2], spiBuf[3], spiBuf[4]);
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
    printf("  Frequency: %3.3lf MHz  Service: %s, %s\n", currentFreq(), 
                       dabShMem -> Ensemble, dabShMem -> currentService.Label);
}

void dabShowState()
{
    dabFreqType *dFreq;

    dFreq = &(dabShMem -> dabFreq[dabShMem -> currentService.Freq]);

    printf("  Ensemble: %s\n", dabShMem -> Ensemble);
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

    if(dabServiceValid() == TRUE)
    {
        dabGetEnsembleInfo();
        dabGetDigRadioStatus();
    }
}

void dabTune(DABService *service)
{
    dabGetEnsembleInfo();
    dabGetRssi();
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
