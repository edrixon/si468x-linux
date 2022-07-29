#ifndef __GOT_TYPES

#define __GOT_TYPES

#include <semaphore.h>
#include <time.h>

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

#define TRUE 1
#define FALSE 0
typedef unsigned char boolean;

typedef struct
{
    unsigned int freq;
    char ensemble[17];
    boolean serviceValid;
} dabFreqType;

typedef struct
{
    unsigned char chipRevision;
    unsigned char romID;
    unsigned int partNo;
    unsigned char activeImage;
} sysInfoType;

typedef struct
{
    unsigned char Freq;
    unsigned int  ServiceID;
    unsigned int  CompID;
    char          Label[17];
} DABService;

typedef struct
{
    unsigned int serviceID;
    unsigned int compID;
    unsigned char serviceMode;
    unsigned char  protectionInfo;
    unsigned short int bitRate;
    unsigned short int numCu;
    unsigned short int cuAddr;
} channelInfoType;

typedef struct
{
    unsigned short int bitRate;
    unsigned short int sampleRate;
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
    long int userPid;
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
    unsigned short int fibErrorCount;
    unsigned short int cuCount;
} sigQualityType;

typedef struct
{
    sem_t semaphore;
    sysInfoType sysInfo;
    int dabServiceValid;
    double audioLevel;
    unsigned int audioLevelRaw;
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
    dabFreqType dabFreq[DAB_MAX_FREQS];
    dabCmdType dabCmd;
    dabCmdRespType dabResp;
} dabShMemType;

#endif
