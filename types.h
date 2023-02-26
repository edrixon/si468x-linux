#ifndef __GOT_TYPES

#define __GOT_TYPES

#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define LOGGER_INIT      0
#define LOGGER_SCAN      1
#define LOGGER_COVERAGE  2
#define LOGGER_STARTWAIT 3

#define DAB_MAX_FREQS    80
#define DAB_MAX_SERVICES 16
#define DAB_MAX_SERVICEDATA_LEN 128

#define DAB_ENGINE_READY    0x55
#define DAB_ENGINE_NOTREADY 0x00

#define DAB_SESSION_TELNET  0
#define DAB_SESSION_HTTP    1

typedef unsigned char boolean;

typedef struct
{
    pid_t pid;
    char name[32];
} dabProcessType;

typedef struct
{
    int typeOfSession;
    char *clientAddr[20];
    void *nextUser;
} remoteUserType;

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
    time_t ticks;
    int rssi;
    int snr;
    int ficQuality;
    int cnr;
    unsigned short int fibErrorCount;
    unsigned short int fibErrorDiff;
    double fibErrorRate;
} sigQualityType;

typedef struct
{
    unsigned int freq;
    char ensemble[17];
    boolean serviceValid;
    unsigned short int cuCount;
    sigQualityType sigQuality;
    boolean minAlarmEnabled;
    sigQualityType minSignal;
    boolean maxAlarmEnabled;
    sigQualityType maxSignal;
} dabFreqType;

typedef struct
{
    unsigned char major;
    unsigned char minor;
    unsigned char build;
} funcInfoType;

typedef struct
{
    unsigned char chipRevision;
    unsigned char romID;
    unsigned int  partNo;
    unsigned char activeImage;
    funcInfoType funcInfo;
} sysInfoType;

typedef struct
{
    unsigned char Freq;
    unsigned int  ServiceID;
    unsigned int  CompID;
    char          Label[17];
    char          shortLabel[17];
    unsigned char programmeType;
} DABService;

typedef struct
{
    int state;
    int runMode;
    int freq;
    DABService monitorService;
    DABService loggingService;
    DABService *currentService;
    char logFilename[64];
} dabLoggerType;

typedef struct
{
    unsigned char  serviceMode;
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
    int runMode;
} paramType;

typedef union
{
    channelInfoType channelInfo;
    int runMode;
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
    DABService lastService;
    validSignalType validSignal;
    int coverageTime;
    char logFilename[255];
} sysConfigType;

typedef struct
{
    time_t seconds;
    int fix;
    double latitude;
    double longitude;
    double altitude;
    double speed;
} gpsInfoType;

typedef struct
{
    int engineVersion;
    int engineState;
    sem_t semaphore;
    sysConfigType sysConfig;
    sysInfoType sysInfo;
    gpsInfoType gpsInfo;
    unsigned long int interruptCount;
    double audioLevel;
    unsigned int audioLevelRaw;
    struct tm time;
    DABService currentService;
    audioInfoType audioInfo;
    channelInfoType channelInfo;
    unsigned char numberofservices;
    DABService service[DAB_MAX_SERVICES];
    unsigned long int serviceDataMs;
    char serviceData[DAB_MAX_SERVICEDATA_LEN];
    int loggerRunning;
    double loggerMeasureSeconds;
    int numDabFreqs;
    dabFreqType dabFreq[DAB_MAX_FREQS];
    dabCmdType dabCmd;
    dabCmdRespType dabResp;
    remoteUserType *remoteUsers;
    int telnetUsers;
    int httpUsers;
} dabShMemType;

#endif
