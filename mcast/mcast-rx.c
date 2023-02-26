//
// Ed's multicast receiver
// puts a multicast stream out to stdout
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "mcast.h"

#define RXBUFFLEN 1024

void help()
{
    printf("mcast-rx [-g <group address>] [-h] [-p <base port>] session\n");
    printf(" -g <group address>  (A.B.C)\n");
    printf(" -h   display in hex\n");
    printf(" -p <port number>    (Base port number)\n");
}

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int addrlen, sock, cnt;
    struct ip_mreq mreq;
    char message[RXBUFFLEN];
    char groupopt[32];
    char group[64];
    int c;
    int port;
    int printhex;

    strcpy(groupopt, GROUP);
    port = PORT;
    printhex = 0;
    do
    {
        c = getopt(argc, argv, "g:hp:?");
        switch(c)
        {
            case 'g':
                strcpy(groupopt, optarg);
                break;

            case 'h':
                printhex = 0xff;
                break;

            case 'p':
                port = atoi(optarg);
                break;

            case '?':
                help();
                break;

            default:;
        }
    }
    while(c != -1);

    if(argc - optind == 1)
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0)
        {
            perror("socket");
            exit(1);
        }

        sprintf(group, "%s.%s", groupopt, argv[optind]);
        port = port + atoi(argv[optind]);

        bzero((char *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        addrlen = sizeof(addr);
 
        if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        {        
            perror("bind");
            exit(1);
        }
    
        mreq.imr_multiaddr.s_addr = inet_addr(group);         
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
        if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                                      &mreq, sizeof(mreq)) < 0)
        {
            perror("setsockopt mreq");
            exit(1);
        }
         
        while(1)
        {
            cnt = recvfrom(sock, message, RXBUFFLEN, 0, 
                                          (struct sockaddr *) &addr, &addrlen);

            if(cnt < 0)
            {
                perror("recvfrom");
                exit(1);
            }

            c = 0;
            while(cnt)
            {
                if(printhex == 0)
                {
                    putchar(message[c]);
                }
                else
                {
                    if(c % 16 == 0)
                    {
                       printf("\n");
                    }
                    printf("%02X ", message[c]);
                    fflush(stdout);
                }
                cnt--;
                c++;
            }
            fflush(stdout);
        }
    }
}

