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
    { "html", "text/html" },
    { "css", "text/css" },
    { "json", "application/js" },
    { "js", "text/javascript" },
    { "", "" }
};

httpHandlerType httpBuiltIn[] =
{
    { "/service", httpGetService },
    { "/system", httpGetSystem },
    { "/current", httpGetCurrent },
    { "/freq", httpGetFreqs },
    { "/ensemble", httpGetEnsemble },
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
