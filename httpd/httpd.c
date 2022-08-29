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
#include "httpd.h"

#include "globals.h"

void httpdIncUsers()
{
    shmLock();
    dabShMem -> httpUsers++;
    shmFree();
}

void httpdDecUsers()
{
    shmLock();
    dabShMem -> httpUsers--;
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
    do
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

void httpdSigalrm(int signum)
{
    // scheduler

    alarm(1);
}

void httpdSigint(int signum)
{
    pid_t myPid;

    myPid = getpid();

    if(myPid == httpdPid)
    {
        printf("HTTP server exiting on SIGINT\n");
        dabShMem -> httpUsers = 0;
    }
    else
    {
        printf("Client handler exiting on SIGINT\n");
    }
    close(listenFd);
    exit(0);
}

void httpdSigchld(int signum)
{
    pid_t childPid;

    if(dabShMem -> httpUsers)
    {
        httpdDecUsers();
    }    

    childPid = waitpid(-1, NULL, 0);

    getAsciiTime(pBuf);

    printf("[%s] Client disconnected\n", pBuf);
    printf("[%s] Process %d exited (active connections %d)\n",
                                      pBuf, childPid, dabShMem -> httpUsers);
}

void httpdHandleRequest(char *reqBuf)
{
    char *req;
    char *p[16];
    int c;
    int paramCount;

    paramCount = 0;

    req = strtok(reqBuf, " ");

    do
    {
        p[paramCount] = strtok(NULL, " ");
        paramCount++;
    }
    while(paramCount < 16 && p[paramCount] != NULL);

    c = 0;
    while(httpHandlers[c].name[0] != '\0' &&
                                        strcmp(req, httpHandlers[c].name) != 0)
    {
        c++;
    }

    if(httpHandlers[c].name[0] != '\0')
    {
        httpHandlers[c].fn(p);
    }
}

void httpdSession()
{
    int charsRead;
    char reqBuf[255];

    alarm(1);

    printf("Waiting for logger to stop... ");
    fflush(stdout);
    while(dabShMem -> loggerRunning == TRUE);
    printf("OK\n");

    webDone = FALSE;
    while(webDone == FALSE)
    {
        // Do stuff
        charsRead = tgets(reqBuf);


        if(charsRead < 0)
        {
            webDone = TRUE;
        }
        else
        {
            if(charsRead)
            {
                httpdHandleRequest(reqBuf);
            }
        }
    }
}

void httpdChild()
{
    close(listenFd);

    if(dabShMem -> httpUsers > HTTPD_MAXCONNECTIONS)
    {
        printf("Too many web users\n");
    }
    else
    {
        httpdSession();                        
    }

    close(connFd);
    exit(0);
}

void httpdConnection(pid_t childPid, struct sockaddr_in *clientAddr)
{
    close(connFd);
    httpdIncUsers();

    getAsciiTime(pBuf);
    printf("[%s] Connected to client %s (port %d)\n",
                                     pBuf,
                                     inet_ntoa(clientAddr -> sin_addr),
                                     ntohs(clientAddr -> sin_port));
    printf("[%s] Spawned process %d (connection %d)\n",
                                     pBuf,
                                     childPid, dabShMem -> httpUsers);
}

int httpBind(int fd, struct sockaddr *sa, int sz)
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

void httpd()
{
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen;
    struct sigaction sigintAction;
    pid_t httpdChildPid;

    httpdPid = getpid();

    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0)
    {
        perror("socket");
    }
    else
    {
        sigintAction.sa_handler = httpdSigint;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGINT, &sigintAction, NULL);

        sigintAction.sa_handler = httpdSigchld;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGCHLD, &sigintAction, NULL);

        sigintAction.sa_handler = httpdSigalrm;
        sigemptyset(&sigintAction.sa_mask);
        sigintAction.sa_flags = 0;
        sigaction(SIGALRM, &sigintAction, NULL);

        bzero(&servAddr, sizeof(servAddr));

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(HTTPD_PORT);

        if(httpBind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
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
                printf("  Ready on port %d...\n\n", HTTPD_PORT);

                dabShMem -> httpUsers = 0;
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
                            httpdChildPid = fork();
                            if(httpdChildPid < 0)
                            {
                                perror("fork");
                            }
                            else
                            {
                                if(httpdChildPid == 0)
                                {
                                    httpdChild();
                                }
                                else
                                {
                                    httpdConnection(httpdChildPid,
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

