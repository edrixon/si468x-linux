#ifndef __GOT_TYPES

#define __GOT_TYPES

#include <semaphore.h>
#include <time.h>

#define DAB_MAX_FREQS    80
#define DAB_MAX_SERVICES 16
#define DAB_MAX_SERVICEDATA_LEN 128

#define DAB_ENGINE_READY    0x55
#define DAB_ENGINE_NOTREADY 0x00

typedef unsigned char boolean;

typedef struct
{
    pid_t pid;
    char *clientAddr[20];
} telnetUserType;

typedef struct
{
    unsigned int count;
    unsigned int reload;
    void (*handlerFn)(void);
    char name[16];
    int runAlways;
    int enabled;
} dabTimerType;

#define TRUE 1
#define FALSE 0
typedef unsigned char boolean;

typedef struct
{
    int rssi;
    int snr;
    int ficQuality;
    int cnr;
    unsigned short int fibErrorCount;
} sigQualityType;

typedef struct
{
    unsigned int freq;
    char ensemble[17];
    boolean serviceValid;
    sigQualityType sigQuality;
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
    unsigned char major;
    unsigned char minor;
    unsigned char build;
} funcInfoType;

typedef struct
{
    unsigned char Freq;
    unsigned int  ServiceID;
    unsigned int  CompID;
    char          Label[17];
    unsigned char programmeType;
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
    int squelch;
    int rssiTime;
    int acqTime;
    int syncTime;
    int detectTime;
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
    int rssiTime;
    int rssiThreshold;
    int acqTime;
    int syncTime;
    int detectTime;
} validSignalType;

typedef struct
{
    int engineVersion;
    int engineState;
    sem_t semaphore;
    int loggerRunning;
    validSignalType validSignal;
    sysInfoType sysInfo;
    funcInfoType funcInfo;
    int dabServiceValid;
    unsigned long int interruptCount;
    double audioLevel;
    unsigned int audioLevelRaw;
    struct tm time;
    DABService currentService;
    audioInfoType audioInfo;
    unsigned short int cuCount;
    unsigned char numberofservices;
    DABService service[DAB_MAX_SERVICES];
    unsigned long int serviceDataMs;
    char serviceData[DAB_MAX_SERVICEDATA_LEN];
    double loggerMeasureSeconds;
    int dabFreqs;
    dabFreqType dabFreq[DAB_MAX_FREQS];
    dabCmdType dabCmd;
    dabCmdRespType dabResp;
    int telnetUsers;
    int httpUsers;
} dabShMemType;

#endif
