#ifdef __IN_MAIN

#include "dablogger.h"
#include "buzzer.h"

unsigned int spi;
unsigned char spiBuf[SPI_BUFF_SIZE];

dabTimerType dabTimers[] = 
{
    {
        DAB_LED_TICKS,
        DAB_LED_TICKS,
        dabLedHandler,
        "LEDS",
        TRUE,
        TRUE
    },
    {
        DAB_BZR_TICKS,
        DAB_BZR_TICKS,
        dabBuzzHandler,
        "BUZZ",
        TRUE,
        FALSE
    },
    {
        DAB_RSSI_TICKS,
        DAB_RSSI_TICKS,
        dabScheduledGetDigRadioStatus,
        "RSSI",
        FALSE,
        TRUE
    },
    {
        DAB_LOGGER_SCAN_TICKS,
        DAB_LOGGER_SCAN_TICKS,
        dabLoggerMain,
        "LOGGER",
        TRUE,
        FALSE
    },
    {
        DAB_TIME_TICKS,
        DAB_TIME_TICKS,
        dabGetTime,
        "TIME",
        FALSE,
        TRUE
    },
    {
        DAB_SHOWTIME_TICKS,
        DAB_SHOWTIME_TICKS,
        dabShowTime,
        "SHOWTIME",
        FALSE,
        TRUE
    },
    {
        DAB_SHOWSERV_TICKS,
        DAB_SHOWSERV_TICKS,
        dabShowServiceSummary,
        "SHOWSERV",
        FALSE,
        TRUE
    },
    {
        DAB_SHOWSIG_TICKS,
        DAB_SHOWSIG_TICKS,
        dabShowSignal,
        "SHOWSIG",
        FALSE,
        TRUE
    }
};

int dab_maxtimers = (sizeof(dabTimers) / sizeof(dabTimerType));

uint8_t command_error;

int dabDone;

dabLoggerType dabLogger;

uint32_t dabMhz[] =
{
    174928, 176640, 178352, 180064, 181936, 183648, 185360,
    187072, 188928, 190640, 192352, 194064, 195936, 197648,
    199360, 201072, 202928, 204640, 206352, 208064, 209936,
    211648, 213360, 215072, 216928, 218640, 220352, 222064,
    223936, 225648, 227360, 229072, 230784, 232496, 234208,
    235776, 237488, 239200
};

int numDabMhz = (sizeof(dabMhz) / sizeof(uint32_t));

int loggerRestartDelay;

#else

extern unsigned int spi;
extern unsigned char spiBuf[SPI_BUFF_SIZE];

extern dabTimerType dabTimers[];

extern uint8_t command_error;

extern int dabDone;

extern dabLoggerType dabLogger;

extern uint32_t dabMhz[];
extern int      numDabMhz;

extern int dab_maxtimers;

extern int loggerRestartDelay;

#endif

#define DAB_MAXTIMERS dab_maxtimers

