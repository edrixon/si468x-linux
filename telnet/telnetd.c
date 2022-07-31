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
#include "cli.h"
#include "telnetd.h"

int connFd;
int listenFd;
int telnetdConnections;
pid_t telnetdPid;

time_t lastTime;

extern char pBuf[];
extern char cliBuffer[];
extern int cliDone;
extern int cliTimeout;

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
    fd_set set;
    int rv;
    struct timeval timeout;

    FD_ZERO(&set);
    FD_SET(connFd, &set);


    done = FALSE;
    charsRead = 0;
    do
    {
        timeout.tv_sec = cliTimeout;
        timeout.tv_usec = 0;
        rv = select(connFd + 1, &set, NULL, NULL, &timeout);
        if(rv == -1)
        {
            perror("select");
            charsRead = -1;
            done = TRUE;
        }
        else
        {
            if(rv == 0)
            {
                // timeout
                tputs("\nIdle timeout\n");
                done = TRUE;
                charsRead = -1;
            }
            else
            {
                if(read(connFd, &inchar, 1) != 1)
                {
                    done = TRUE;
                    charsRead = -1;
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

void telnetdSigint(int signum)
{
    pid_t myPid;

    myPid = getpid();

    if(myPid == telnetdPid)
    {
        printf("Telnet server exiting on SIGINT\n");
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
    if(telnetdConnections)
    {
        telnetdConnections--;
    }    

//    while(waitpid(-1, NULL, 0) > 0);

    getAsciiTime(pBuf);
    printf("[%s] Disconnected (remaining connections %d)\n",
                                                     pBuf, telnetdConnections);
}

void telnetdSession()
{
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

    if(telnetdConnections >= TELNETD_MAXCONNECTIONS)
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
    telnetdConnections++;

    getAsciiTime(pBuf);
    printf("[%s] Connected to client %s (port %d)\n",
                                     pBuf,
                                     inet_ntoa(clientAddr -> sin_addr),
                                     ntohs(clientAddr -> sin_port));
    printf("[%s] Spawned process %d (connection %d)\n",
                                     pBuf,
                                     childPid, telnetdConnections);
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

        bzero(&servAddr, sizeof(servAddr));

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(TELNETD_PORT);

        if(bind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
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

                telnetdConnections = 0;
                addrLen = sizeof(struct sockaddr);
                while(1)
                {
                    connFd = accept(listenFd,
                                     (struct sockaddr *)&clientAddr, &addrLen);

                    if(connFd < 0)
                    {
//                        perror("accept");
                    }
                    else
                    {
                        telnetdChildPid = fork();
                        if(telnetdChildPid == 0)
                        {
                            telnetdChild();
                        }
                        else
                        {
                            telnetdConnection(telnetdChildPid, &clientAddr);
                        }
                    }
                }
            }
        }
    }
}

