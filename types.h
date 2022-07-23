#ifndef __GOT_TYPES

#define __GOT_TYPES

#define DAB_MAX_FREQS    80
#define DAB_MAX_SERVICES 16
#define DAB_MAX_SERVICEDATA_LEN 128

typedef struct dabTimerType
{
    unsigned int count;
    unsigned int reload;
    void (*handlerFn)(void);
    char name[16];
    int enabled;
} dabTimerType;

typedef unsigned char boolean;

typedef struct
{
    unsigned char chipRevision;
    unsigned char romID;
    unsigned int partNo;
    unsigned char activeImage;
} sysInfoType;

typedef struct
{
    uint8_t   Freq;
    uint32_t  ServiceID;
    uint32_t  CompID;
    char      Label[17];
} DABService;

typedef struct
{
    uint32_t serviceID;
    uint32_t compID;
    uint8_t serviceMode;
    uint8_t protectionInfo;
    uint16_t bitRate;
    uint16_t numCu;
    uint16_t cuAddr;
} channelInfoType;

typedef struct
{
    uint16_t bitRate;
    uint16_t sampleRate;
    int mode;
    int drcGain;
} audioInfoType;

typedef union
{
    DABService service;
} paramType;

typedef union
{
    channelInfoType channelInfo;
} dabCmdRespType;

typedef struct
{
    unsigned long int userPid;
    int cmd;
    int rtn;
    paramType params;
} dabCmdType;

typedef struct
{
    int rssi;
    int snr;
    int ficQuality;
    int cnr;
    uint16_t fibErrorCount;
    uint16_t cuCount;
} sigQualityType;

typedef struct
{
    sem_t semaphore;
    sysInfoType sysInfo;
    int dabServiceValid;
    struct tm time;
    DABService currentService;
    audioInfoType audioInfo;
    sigQualityType signalQuality;
    char Ensemble[17];
    unsigned char numberofservices;
    DABService service[DAB_MAX_SERVICES];
    unsigned long int serviceDataMs;
    char serviceData[DAB_MAX_SERVICEDATA_LEN];
    int dabFreqs;
    uint32_t dabFreq[DAB_MAX_FREQS];
    dabCmdType dabCmd;
    dabCmdRespType dabResp;
} dabShMemType;

#endif
