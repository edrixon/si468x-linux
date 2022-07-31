#ifndef __GOT_CLI

#define __GOT_CLI

#define CLIHELLO "Command line interpretter version 1.0"
#define CLIPROMPT ">"

#define CLITIMEOUT    600
#define CLIMINTIMEOUT 30

typedef struct
{
    char *name;
    char *helpStr;
    void (*fn)(char *param);
} CLICOMMAND;

void helpCmd(char *p);
void exitCmd(char *p);
void cmdTimeout(char *p);
void doCliCommand();
int doCommand(dabCmdType *cmd, dabCmdRespType *resp);

#endif
