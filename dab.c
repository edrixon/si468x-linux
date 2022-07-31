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

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dab.h"

unsigned int spi;
unsigned char spiBuf[SPI_BUFF_SIZE];

void dabGetDigRadioStatus(void);
void dabGetRssi(void);
void dabGetTime(void);
void dabShowTime(void);
void dabShowSignal(void);
void dabShowServiceSummary(void);

#ifdef DAB_USE_ALL_CHANNELS

uint32_t dab_freq[] =
{
           174928, 176640, 178352, 180064, 181936, 183648, 185360,
           187072, 188928, 190640, 192352, 194064, 195936, 197648,
           199360, 201072, 202928, 204640, 206352, 208064, 209936,
           211648, 213360, 215072, 216928, 218640, 220352, 222064,
           223936, 225648, 227360, 229072, 230748, 232496, 234208,
           235776, 237448, 239200
};

#else

uint32_t dab_freq[] =
{
           223936, 225648, 227360, 229072, 230748, 232496, 234208
};

#endif

int dab_freqs = (sizeof(dab_freq) / sizeof(dab_freq[0]));

dabTimerType dabTimers[] = 
{
    { DAB_RSSI_TICKS, DAB_RSSI_TICKS, dabGetDigRadioStatus, "RSSI", TRUE },
    { DAB_TIME_TICKS, DAB_TIME_TICKS, dabGetTime, "TIME", TRUE },
    { DAB_SHOWTIME_TICKS, DAB_SHOWTIME_TICKS, dabShowTime, "SHOWTIME", TRUE },
    { DAB_SHOWSERV_TICKS, DAB_SHOWSERV_TICKS, dabShowServiceSummary, "SHOWSERV", TRUE },
    { DAB_SHOWSIG_TICKS, DAB_SHOWSIG_TICKS, dabShowSignal, "SHOWSIG", TRUE }
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
    siDabDigRadStatus();
    siResponseN(23);

    dabShMem -> signalQuality.snr = spiBuf[8];
    dabShMem -> signalQuality.ficQuality = spiBuf[9];
    dabShMem -> signalQuality.cnr = spiBuf[10];
    dabShMem -> signalQuality.fibErrorCount = spiBytesTo16(&spiBuf[11]);
    dabShMem -> signalQuality.cuCount = spiBytesTo16(&spiBuf[21]);

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

void dabGetRssi()
{
    siGetRssi();
    siResponseN(6);

    dabShMem -> signalQuality.rssi = spiBytesTo16(&spiBuf[5]);
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

    dabGetSubChannelInfo(service -> ServiceID, service -> CompID, &(dabShMem -> dabResp.channelInfo));

//    dabShowSubChannelInfo(&(dabShMem -> dabResp.channelInfo));
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
    uint16_t len;
    uint8_t dataSrc;
    uint16_t byteCount;
    uint16_t compID;
    uint16_t servID;
//    uint16_t numSegs;
//    uint16_t segNum;
//    uint8_t buffCount;

//    printf("DAB interrupt - %d, %d, %d\n", gpio, level, ticks);

    if(level == 0)
    {
        siResponse();
        if((spiBuf[1] & 0x10) == 0x10)
        {
            siGetDigitalServiceData();
            siResponseN(20);

            len = spiBytesTo16(&spiBuf[19]);
            if(len < (SPI_BUFF_SIZE - 24))
            {
                siResponseN(len + 24);
            }
            else
            {
                siResponseN(SPI_BUFF_SIZE - 1);
            }

//            buffCount = spiBuf[6];
            dataSrc = (spiBuf[8] >> 6) & 0x03;
            servID = spiBytesTo32(&spiBuf[9]);
            compID = spiBytesTo32(&spiBuf[13]);
            byteCount = spiBytesTo16(&spiBuf[19]);
//            segNum = spiBytesTo16(&spiBuf[21]);
//            numSegs = spiBytesTo16(&spiBuf[23]);

            if(dataSrc == 0x02)
            {
                dabShMem -> serviceDataMs = timeMillis();

                printf("[%ld]  Received %d DLS bytes for 0x%08x/0x%08x.\n",
                         dabShMem -> serviceDataMs, byteCount, servID, compID);
                bzero(dabShMem -> serviceData, DAB_MAX_SERVICEDATA_LEN);
                bcopy(&spiBuf[27], dabShMem -> serviceData, byteCount - 3);
                printf("> %s <\n", dabShMem -> serviceData);
            }
            else
            {
                printf("[%ld]  Received %d type %d bytes for 0x%08x/0x%08x.\n",
                             timeMillis(), byteCount, dataSrc, servID, compID);
            }
        }
    }
}

void dabBegin()
{
    printf("Radio initialising...\n");
    siReset();
    siInitDab();

    siGetPartInfo();
    siGetSysState();

    printf("  Found Si%d Rev %d, ROM ID %d.  Active image %d\n",
            dabShMem -> sysInfo.partNo, dabShMem -> sysInfo.chipRevision,
                   dabShMem -> sysInfo.romID, dabShMem -> sysInfo.activeImage);
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

    strftime(strBuf, 80, "  Date: %d %B %Y, %H:%M", &(dabShMem -> time));
    printf("%s\n", strBuf);
}

void dabHandleTimers()
{
    int c;

    for(c = 0; c < DAB_MAXTIMERS; c++)
    {
//        printf("Timer task - %s\n", dabTimers[c].name);
        if(dabTimers[c].enabled == TRUE)
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

void dabShowSignal()
{
    printf("  RSSI: %d dBuV  SNR: %d dB  CNR: %d dB  FIC Quality: %d %%  FIB errors: %d\n",
                                     (dabShMem -> signalQuality.rssi / 256),
                                      dabShMem -> signalQuality.snr,
                                      dabShMem -> signalQuality.cnr,
                                      dabShMem -> signalQuality.ficQuality,
                                      dabShMem -> signalQuality.fibErrorCount);
}

void dabShowServiceSummary()
{
    double f;

    f = (double)dab_freq[dabShMem -> currentService.Freq] / 1000.0;

    printf("  Frequency: %3.3lf MHz  Service: %s, %s\n", f,
                       dabShMem -> Ensemble, dabShMem -> currentService.Label);
}

void dabShowState()
{
    uint32_t f;

    f = dab_freq[dabShMem -> currentService.Freq];
    printf("  Ensemble: %s\n", dabShMem -> Ensemble);
    printf("  Frequency: %3.3lf MHz\n",
                           (double)f / 1000.0); 
    printf("  Service: %s (0x%08x/0x%08x)\n",
                                       dabShMem -> currentService.Label,
                                       dabShMem -> currentService.ServiceID,
                                       dabShMem -> currentService.CompID);
    printf("  RSSI: %d dBuV\n", (dabShMem -> signalQuality.rssi / 256));
    printf("  SNR: %d dB   CNR: %d dB\n",
                 dabShMem -> signalQuality.snr, dabShMem -> signalQuality.cnr);
    printf("  FIC quality: %d %%\n", dabShMem -> signalQuality.ficQuality);
    printf("  FIB error count: %d\n", dabShMem -> signalQuality.fibErrorCount);  
    printf("  CU count: %d\n", dabShMem -> signalQuality.cuCount);

    printf("  Bit rate: %d\n", dabShMem -> audioInfo.bitRate);
    printf("  Sample rate: %d\n", dabShMem -> audioInfo.sampleRate);
    printf("  Dynamic range: %d\n", dabShMem -> audioInfo.drcGain);
    printf("  Mode: %d\n", dabShMem -> audioInfo.mode);
}

void dabTuneFreq(DABService *service)
{
    dabStopCurrentDigitalService();
    siDabTuneFreq(service -> Freq);
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

void dabMain()
{
    DABService lastService;

    dabGetLastService(&lastService);

    dabTuneFreq(&lastService);
    if(dabServiceValid() == TRUE)
    {
        dabTune(&lastService);
    }

    printf("Ready...\n");

    dabDone = FALSE;
    while(dabDone == FALSE)
    {
        dabCommand();

        if(dabDone == FALSE)
        {
            dabHandleTimers();
            milliSleep(DAB_TICKTIME);
        }
    }
}
