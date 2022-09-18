#ifdef __IN_MAIN

#include "httpd.h"
#include "httphandlers.h"

char pBuf[255];

int connFd;
int listenFd;
pid_t httpdPid;
int webDone;

contentHeaderType contentHeaders[] =
{
    { "html", "text/html", FALSE },
    { "css", "text/css", FALSE },
    { "jpg", "image/jpeg", TRUE },
    { "json", "application/json", FALSE },
    { "js", "text/javascript", FALSE },
    { "", "" }
};

httpHandlerType httpBuiltIn[] =
{
    { "/system", httpGetSystem },
    { "/current", httpGetCurrent },
    { "/freq", httpGetFreqs },
    { "/ensemble", httpGetEnsemble },
    { "/servicedata", httpGetServiceData },
    { "/channel", httpSetChannel },
    { "/service", httpSetService },
    { "/dabradio", httpGetDabRadio },
    { "", NULL }
};

httpHandlerType httpHandlers[] =
{
    { "GET", httpGet },
    { "", NULL }
};

char *audioModes[MAX_AUDIO_MODES] =
{
    "dual",
    "mono",
    "stereo",
    "joint stereo"
};

char *serviceModes[MAX_SERVICE_MODES] =
{
//    "audio stream",
    "dab",
    "data stream",
//    "fidc",
    "data",
//    "msc data",
    "data",
    "dab+",
    "dab",
//    "fic",
    "data",
//    "xpad",
    "data",
//    "no media"
    "data"
};

#else

extern char pBuf[];

extern int connFd;
extern int listenFd;
extern pid_t httpdPid;
extern int webDone;

extern char *audioModes[];
extern char *serviceModes[];

extern httpHandlerType httpHandlers[];
extern httpHandlerType httpBuiltIn[];
extern contentHeaderType contentHeaders[];

#endif
