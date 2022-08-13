#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>


#include "../types.h"
#include "../dabshmem.h"
#include "../shm.h"
#include "cli.h"
#include "telnetd.h"
#include "commands.h"

int connFd;
int listenFd;
pid_t telnetdPid;

int showStatus;
int showStatusTime;
int showStatusLeft;

int showDls;
unsigned long int dlsMillis;

time_t lastTime;

telnetUserType telnetUsers[TELNETD_MAXCONNECTIONS];

extern char pBuf[];
extern char cliBuffer[];
extern int cliDone;
extern int cliTimeout;
extern int cliTimeLeft;

void telnetdIncUsers()
{
    shmLock();
    dabShMem -> telnetUsers++;
    shmFree();
}

void telnetdDecUsers()
{
    shmLock();
    dabShMem -> telnetUsers--;
    shmFree();
}

void tputs(char *str)
{
    int c;
    char xTra;

    c = strlen(str);
    while(c)
    {
        write(connFd, str, 1);
        if(*str == '\n')
        {
            xTra = '\r';
            write(connFd, &xTra, 1);
        }
        str++;
        c--;
    }
}

int tgets(char *str)
{
    unsigned char inchar;
    int done;
    int charsRead;

    done = FALSE;
    charsRead = 0;
    cliTimeLeft = cliTimeout;
    do
    {
        if(read(connFd, &inchar, 1) != 1)
        {
            if(cliTimeLeft == 0)
            {
                tputs("\nIdle timeout\n");
                done = TRUE;
                charsRead = -1;
            }
        }
        else
        {
            if(isprint(inchar) && charsRead < 80)
            {
                *str = inchar;
                str++;
                charsRead++;
            }
            else
            {
                switch(inchar)
                {
                    case '\n':
                        *str = '\0';
                        done = TRUE;
                        break;

                    case 0x08:
                        if(charsRead > 0)
                        {
                            charsRead--;
                            str--;
                        }
                        break;

                    default:;
                }
            }
        }
    }
    while(done == FALSE);

    return charsRead;
}

void getAsciiTime(char *buf)
{
    time_t timeNow;
    struct tm *localTimeTm;

    timeNow = time(NULL);
    localTimeTm = localtime(&timeNow);
    strftime(buf, 80, "%d %B %Y, %H:%M:%S", localTimeTm);
}

void telnetdSigalrm(int signum)
{
    double f;

    cliTimeLeft--;

    if(showStatus > 0)
    {
        showStatus--;
        showStatusLeft--;
        if(showStatusLeft == 0)
        {
            f = (double)dabShMem -> dabFreq[dabShMem -> currentService.Freq].freq / 1000.0;

            sprintf(pBuf, "  Frequency: %3.3lf MHz  %s, %s\n",
                    f, dabShMem -> Ensemble, dabShMem -> currentService.Label);
            tputs(pBuf);

            cmdRssi("");
            cmdTime("");

            showStatusLeft = showStatusTime;

        }
    }
    else
    {
        if(showStatus == 0)
        {
            tputs("Status messages timeout\n");
            showStatus = -1;
        }
    }

    if(showDls > 0)
    {
        showDls--;
        if(dlsMillis != dabShMem -> serviceDataMs)
        {
            sprintf(pBuf, "  [%s]\n", dabShMem -> serviceData);
            tputs(pBuf);
            dlsMillis = dabShMem -> serviceDataMs;
        }
    }
    else
    {
        if(showDls == 0)
        {
            tputs("DLS messages timedout\n");
            showDls = -1;
        }
    }

    alarm(1);
}

void telnetdSigint(int signum)
{
    pid_t myPid;

    myPid = getpid();

    if(myPid == telnetdPid)
    {
        printf("Telnet server exiting on SIGINT\n");
        dabShMem -> telnetUsers = 0;
    }
    else
    {
        printf("Client handler exiting on SIGINT\n");
    }
    close(listenFd);
    exit(0);
}

void telnetdSigchld(int signum)
{
    pid_t childPid;

    if(dabShMem -> telnetUsers)
    {
        telnetdDecUsers();
    }    

    childPid = waitpid(-1, NULL, 0);

    getAsciiTime(pBuf);

    printf("[%s] Client disconnected\n", pBuf);
    printf("[%s] Process %d exited (active connections %d)\n",
                                      pBuf, childPid, dabShMem -> telnetUsers);
}

void telnetdSession()
{
    alarm(1);

    sprintf(pBuf, "%s\n", CLIHELLO);
    tputs(pBuf);

    cliTimeout = CLITIMEOUT;
    sprintf(pBuf, "CLI timeout is %d seconds\n", cliTimeout); 
    tputs(pBuf);

    cliDone = FALSE;
    while(cliDone == FALSE)
    {
        lastTime = time(NULL);

        sprintf(pBuf, "%s ", CLIPROMPT);
        tputs(pBuf);
        if(tgets(cliBuffer) == -1)
        {
            cliDone = TRUE;
        }
        else
        {
            doCliCommand();
        }
    }
}

void telnetdChild()
{
    close(listenFd);

    sprintf(pBuf, "Remote DAB receiver V1.0, radio is Si%d\n\n",
                                               dabShMem -> sysInfo.partNo);
    tputs(pBuf);

    if(dabShMem -> telnetUsers > TELNETD_MAXCONNECTIONS)
    {
        tputs("Too many telnet connections!\n");
    }
    else
    {
        telnetdSession();                        
    }

    close(connFd);
    exit(0);
}

void telnetdConnection(pid_t childPid, struct sockaddr_in *clientAddr)
{
    close(connFd);
    telnetdIncUsers();

    getAsciiTime(pBuf);
    printf("[%s] Connected to client %s (port %d)\n",
                                     pBuf,
                                     inet_ntoa(clientAddr -> sin_addr),
                                     ntohs(clientAddr -> sin_port));
    printf("[%s] Spawned process %d (connection %d)\n",
                                     pBuf,
                                     childPid, dabShMem -> telnetUsers);
}

int telnetBind(int fd, struct sockaddr *sa, int sz)
{
    int rtnFd;
    int bindTimeout;

    printf("  Binding... ");
    fflush(stdout);

    bindTimeout = 120;
    do
    {
        rtnFd = bind(fd, sa, sz);
        if(rtnFd < 0)
        {
            bindTimeout--;
            if((bindTimeout % 2) == 0)
            {
                printf("/%c", 0x08);
            }
            else
            {
                printf("\\%c", 0x08);
            } 
            fflush(stdout);

            sleep(1);
        }
    }
    while(rtnFd < 0 && bindTimeout > 0);
  
    if(rtnFd < 0)
    {
        printf("FAILED\n");
    }
    else
    { 
        printf("OK\n"); 
    }

    return rtnFd;
}

void telnetd()
{
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen;
    struct sigaction sigintAction;
    pid_t telnetdChildPid;

    telnetdPid = getpid();

    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0)
    {
        perror("socket");
    }
    else
    {
        sigintAction.sa_handler = telnetdSigint;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGINT, &sigintAction, NULL);

        sigintAction.sa_handler = telnetdSigchld;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGCHLD, &sigintAction, NULL);

        sigintAction.sa_handler = telnetdSigalrm;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGALRM, &sigintAction, NULL);

        showStatus = -1;
        showStatusTime = 0;
        showDls = -1;
        dlsMillis = 0;

        bzero(&servAddr, sizeof(servAddr));

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(TELNETD_PORT);

        if(telnetBind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        {
            perror("bind");
        }
        else
        {
            if(listen(listenFd, 5) < 0)
            {
                perror("listen");
            }
            else
            {
                printf("  Ready on port %d...\n\n", TELNETD_PORT);

                dabShMem -> telnetUsers = 0;
                addrLen = sizeof(struct sockaddr);
                while(dabShMem -> engineState == DAB_ENGINE_READY)
                {
                    connFd = accept(listenFd,
                                     (struct sockaddr *)&clientAddr, &addrLen);

                    if(dabShMem -> engineState == DAB_ENGINE_READY)
                    {
                        if(connFd < 0)
                        {
   //                        perror("accept");
                        }
                        else
                        {
                            telnetdChildPid = fork();
                            if(telnetdChildPid < 0)
                            {
                                perror("fork");
                            }
                            else
                            {
                                if(telnetdChildPid == 0)
                                {
                                    telnetdChild();
                                }
                                else
                                {
                                    telnetdConnection(telnetdChildPid,
                                                                  &clientAddr);
                                }
                            }
                        }
                    }
                }

                printf("DAB engine not ready!!!\n");

            }
        }
    }
}

