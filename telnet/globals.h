#ifdef __IN_MAIN

#include "commands.h"

char cliBuffer[80];
char *cliPtr;
int charsAvailable;
int cliDone;
int cliTimeout;
int cliTimeLeft;

CLICOMMAND cliCmd[] =
{
    { "ainfo", "", cmdAudioInfo },
    { "acqtime", "acqtime [<time ms>]", cmdValidAcqTime },
    { "chaninfo", "", cmdGetChannelInfo },
    { "cstatus", "cstatus [<interval>]", cmdShowStatusCont },
    { "dettime", "dettime [<time ms>]", cmdValidDetectTime },
    { "dls", "", cmdShowDls },
    { "ensemble", "", cmdEnsemble },
    { "exit", "", exitCmd },
    { "freq", "freq [<freq id>]", cmdFreq },
    { "help", "", helpCmd },
    { "intcount", "", cmdShowInterruptCount },
    { "reset", "", cmdResetRadio },
    { "rssi", "", cmdRssi },
    { "save", "", cmdSave },
    { "scan", "", cmdScan },
    { "service", "service [<service ID> <component ID>]", cmdTune },
    { "sigtime", "sigtime [<time ms>]", cmdValidRssiTime },
    { "squelch", "squelch [<level>]", cmdSquelch },
    { "synctime", "synctime [<time ms>]", cmdValidSyncTime },
    { "time", "", cmdTime },
    { "tout", "tout [<cli timeout>]", cmdTimeout },
    { "ver", "", cmdVersion },
    { "?", "", helpCmd },
    { "", "", NULL }
};

char *aMode[] =
{
    "dual",
    "mono",
    "stereo",
    "joint stereo"
};

char *serviceModeNames[] =
{
    "audio stream",
    "data stream",
    "fidc",
    "msc data",
    "dab+",
    "dab",
    "fic",
    "xpad",
    "no media"
};

char *ptyNames[] =
{
    "none",
    "News",
    "Affairs",
    "Info",
    "Sport",
    "Educate",
    "Drama",
    "Culture",
    "Science",
    "Varied",
    "Pop M",
    "Rock M",
    "Easy M",
    "Light M",
    "Classics",
    "Other M",
    "Weather",
    "Finance",
    "Children",
    "Social",
    "Religion",
    "Phone in",
    "Travel",
    "Leisure",
    "Jazz",
    "Country",
    "Nation M",
    "Oldies",
    "Folk M",
    "Document",
    "not use",
    "not use"
};

char pBuf[255];

int connFd;
int listenFd;
pid_t telnetdPid;

int showStatusTime;
int showStatusLeft;

int showDls;
unsigned long int dlsMillis;

time_t lastTime;

telnetUserType telnetUsers[TELNETD_MAXCONNECTIONS];

#else

extern char cliBuffer[];
extern char *cliPtr;
extern int charsAvailable;
extern int cliDone;
extern int cliTimeout;
extern int cliTimeLeft;

extern CLICOMMAND cliCmd[];
extern char *aMode[];
extern char *serviceModeNames[];
extern char *ptyNames[];

extern char pBuf[];

extern int connFd;
extern int listenFd;
extern pid_t telnetdPid;

extern int showStatusTime;
extern int showStatusLeft;

extern int showDls;
extern unsigned long int dlsMillis;

extern time_t lastTime;

extern telnetUserType telnetUsers[];

#endif
