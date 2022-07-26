#ifndef __GOT_HTTPD

#define __GOT_HTTPD

void httpdIncUsers();
void httpdDecUsers();
void tputs(char *str);
void tputsCRLF(int expandCRLF, char *str);
int tgets(char *str);
void getAsciiTime(char *buf);
void telnetdSigalrm(int signum);
void httpdSigint(int signum);
void httpdSigchld(int signum);
void httpdSession();
void httpdChild();
void httpd();

#define HTTPD_MAXCONNECTIONS   8 
#define HTTPD_PORT             80

#define WWW_ROOT               "/root/dab/www"

#define MAX_AUDIO_MODES 4
#define MAX_SERVICE_MODES 9

typedef struct
{
    char *name;
    void (*fn)(char **params);
} httpHandlerType;

typedef struct
{
    char *fileExtension;
    char *mimeType;
    int needLength;
} contentHeaderType;

#endif

