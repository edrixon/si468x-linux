#ifdef __IN_MAIN

#include "dablogger.h"

unsigned int spi;
unsigned char spiBuf[SPI_BUFF_SIZE];

dabTimerType dabTimers[] = 
{
    { DAB_RSSI_TICKS, DAB_RSSI_TICKS, dabGetDigRadioStatus, "RSSI", FALSE,TRUE },
    { DAB_LOGGER_TICKS, DAB_LOGGER_TICKS, dabLogger, "LOGR", TRUE, TRUE },
    { DAB_TIME_TICKS, DAB_TIME_TICKS, dabGetTime, "TIME", FALSE, TRUE },
    { DAB_SHOWTIME_TICKS, DAB_SHOWTIME_TICKS, dabShowTime, "SHOWTIME", FALSE, TRUE },
    { DAB_SHOWSERV_TICKS, DAB_SHOWSERV_TICKS, dabShowServiceSummary, "SHOWSERV", FALSE, TRUE },
    { DAB_SHOWSIG_TICKS, DAB_SHOWSIG_TICKS, dabShowSignal, "SHOWSIG", FALSE, TRUE }
};

int dab_maxtimers = (sizeof(dabTimers) / sizeof(dabTimerType));

uint8_t command_error;

int dabDone;

int loggerState;
int loggerFreq;
DABService monitorService;
DABService loggingService;
DABService *currentService;

uint32_t dab_freq[] =
{
           174928, 176640, 178352, 180064, 181936, 183648, 185360,
           187072, 188928, 190640, 192352, 194064, 195936, 197648,
           199360, 201072, 202928, 204640, 206352, 208064, 209936,
           211648, 213360, 215072, 216928, 218640, 220352, 222064,
           223936, 225648, 227360, 229072, 230748, 232496, 234208,
           235776, 237448, 239200
};

int dab_freqs = (sizeof(dab_freq) / sizeof(uint32_t));

int loggerRestartDelay;

#else

extern unsigned int spi;
extern unsigned char spiBuf[SPI_BUFF_SIZE];

extern dabTimerType dabTimers[];

extern uint8_t command_error;

extern int dabDone;

extern int loggerState;
extern int loggerFreq;
extern DABService monitorService;
extern DABService loggingService;
extern DABService *currentService;

extern uint32_t dab_freq[];
extern int dab_freqs;

extern int dab_maxtimers;

extern int loggerRestartDelay;

#endif

#define DAB_MAXTIMERS dab_maxtimers
#define DAB_FREQS     dab_freqs

