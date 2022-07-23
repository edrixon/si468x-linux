#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <strings.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <ctype.h>

#define __IN_MAIN

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dab.h"

#define CLIHELLO "Command line interpretter version 1.0"
#define CLIPROMPT "CLI> "

typedef struct
{
    char *name;
    void (*fn)(char *param);
} CLICOMMAND;

void helpCmd(char *p);
void exitCmd(char *p);
int doCommand();
void cliLoop();
void cliSetup();

char cliBuffer[80];
char *cliPtr;
int charsAvailable;
int cliDone;

CLICOMMAND cliCmd[] =
{
    { "exit", exitCmd },
    { "help", helpCmd },
    { "?", helpCmd },
    { "", NULL }
};

dabCmdType dabCmd;

void helpCmd(char *p)
{
    CLICOMMAND *cmdPtr;

    printf("Available commands:-");

    cmdPtr = cliCmd;
    while(cmdPtr -> fn != NULL)
    {
        printf("  ");
        printf(cmdPtr -> name);
        cmdPtr++;
    }
}

void exitCmd(char *p)
{
    cliDone = 1;
}

void doCliCommand()
{
    int c;
    char *paramPtr;

    c = 0;
    while(cliCmd[c].name[0] != '\0' &&
              strncmp(cliBuffer, cliCmd[c].name, strlen(cliCmd[c].name)) != 0)
    {
        c++;
    } 

    if(cliCmd[c].name[0] == '\0')
    {
        printf("Bad command\n");
    }
    else
    {
        paramPtr = cliBuffer;
        while(*paramPtr != ' ' && *paramPtr != '\0')
        {
            paramPtr++;
        }

        if(*paramPtr == ' ')
        {
            paramPtr++;
        }

        cliCmd[c].fn(paramPtr);
    }
}

int doCommand(dabCmdType *cmd, dabCmdRespType *resp)
{
    int rtn;

    printf("Claiming shared memory...\n");
    shmLock();
    cmd -> userPid = getpid();
    bcopy(cmd, &(dabShMem -> dabCmd), sizeof(dabCmdType));
    printf("Executing command...\n");
    while(dabShMem -> dabCmd.cmd != DABCMD_NONE);
    rtn = dabShMem -> dabCmd.rtn;
    if(resp != NULL)
    {
        bcopy(&(dabShMem -> dabResp), resp, sizeof(dabCmdRespType));
    }
    shmFree();
    printf("Done\n");

    return rtn;
}

int main(int argc, char *argv[])
{
    dabShMem = shmAttach();

    printf("%s\n", dabShMem -> Ensemble);

    dabCmd.params.service.Freq = 0;
    dabCmd.params.service.ServiceID = 0xc22c;
    dabCmd.params.service.CompID = 0x2000c;
    dabCmd.cmd = DABCMD_TUNE;  
    doCommand(&dabCmd, NULL);

    cliDone = 0;
    while(cliDone == 0)
    {
        printf("> ");
        scanf("%s", cliBuffer);

        doCliCommand();
    }
}
