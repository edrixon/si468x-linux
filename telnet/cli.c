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

#include "../types.h"
#include "../dabshmem.h"
#include "../dabcmd.h"
#include "../shm.h"

#include "commands.h"
#include "cli.h"
#include "telnetd.h"

#include "globals.h"

void cmdTimeout(char *p)
{
    if(*p != '\0')
    {
        cliTimeout = atoi(p);
        if(cliTimeout < CLIMINTIMEOUT)
        {
            cliTimeout = CLIMINTIMEOUT;
        }

        cliTimeLeft = cliTimeout;
    }

    sprintf(pBuf, "CLI timeout: %d seconds\n", cliTimeout);;
    tputs(pBuf);
}

void helpCmd(char *p)
{
    CLICOMMAND *cmdPtr;

    sprintf(pBuf, "Available commands:-\n");
    tputs(pBuf);

    cmdPtr = cliCmd;
    while(cmdPtr -> fn != NULL)
    {
        sprintf(pBuf, "  ");
        tputs(pBuf);
        if(cmdPtr -> helpStr[0] == '\0')
        {
            sprintf(pBuf, "%s\n", cmdPtr -> name);
        }
        else
        {
            sprintf(pBuf, "%s\n", cmdPtr -> helpStr);
        }
        tputs(pBuf);
        cmdPtr++;
    }
}

void exitCmd(char *p)
{
    cliDone = TRUE;
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
        if(cliBuffer[0] != '\0')
        {
            sprintf(pBuf, "Bad command\n");
            tputs(pBuf);
        }
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

//    sprintf(pBuf, "Claiming shared memory...\n");
//    tputs(pBuf);
    shmLock();
    cmd -> userPid = getpid();
    bcopy(cmd, &(dabShMem -> dabCmd), sizeof(dabCmdType));
//    sprintf(pBuf, "Executing command...\n");
//    tputs(pBuf);
    while(dabShMem -> dabCmd.cmd != DABCMD_NONE);
    rtn = dabShMem -> dabCmd.rtn;
    if(resp != NULL)
    {
        bcopy(&(dabShMem -> dabResp), resp, sizeof(dabCmdRespType));
    }
    shmFree();
//    sprintf(pBuf, "Done\n");
//    tputs(pBuf);

    return rtn;
}
