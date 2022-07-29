#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>

#include "../types.h"
#include "../dabshmem.h"
#include "cli.h"
#include "telnetd.h"

int connFd;

extern char pBuf[];
extern char cliBuffer[];
extern int cliDone;

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

void telnetd()
{
    int listenFd;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen;
    int telnetDone;

    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0)
    {
        perror("socket: ");
    }
    else
    {
        bzero(&servAddr, sizeof(servAddr));

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(TELNETD_PORT);

        if(bind(listenFd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        {
            perror("bind: ");
        }
        else
        {
            if(listen(listenFd, 10) < 0)
            {
                perror("listen: ");
            }
            else
            {
                printf("Telnet server ready on port %d\n", TELNETD_PORT);

                telnetDone = FALSE;
                while(telnetDone == FALSE)
                {
                    addrLen = sizeof(struct sockaddr);
                    connFd = accept(listenFd, (struct sockaddr *)&clientAddr, &addrLen);

                    getAsciiTime(pBuf);
                    printf("[%s] Connected to client %s (port %d)\n", pBuf,
                                       inet_ntoa(clientAddr.sin_addr),
                                       ntohs(clientAddr.sin_port));

                    sprintf(pBuf, "Remote DAB receiver V1.0, radio is Si%d\n\n",
                                                   dabShMem -> sysInfo.partNo);
                    tputs(pBuf);

                    sprintf(pBuf, "%s\n", CLIHELLO);
                    tputs(pBuf);
                    cliDone = FALSE;
                    while(cliDone == FALSE)
                    {
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

                    getAsciiTime(pBuf);
                    printf("[%s] Disconnected\n", pBuf);

                    close(connFd);
                }
            }
        }
    }
}

