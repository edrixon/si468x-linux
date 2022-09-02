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
    { "/channel", httpSetChannel },
    { "/service", httpSetService },
    { "", NULL }
};

httpHandlerType httpHandlers[] =
{
    { "GET", httpGet },
    { "", NULL }
};

#else

extern char pBuf[];

extern int connFd;
extern int listenFd;
extern pid_t httpdPid;
extern int webDone;

extern httpHandlerType httpHandlers[];
extern httpHandlerType httpBuiltIn[];
extern contentHeaderType contentHeaders[];

#endif
