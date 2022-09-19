#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "mcast.h"

#define RXBUFFLEN 1024

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int addrlen, sock, cnt;
    struct ip_mreq mreq;
    char message[RXBUFFLEN];
    char group[32];
    int c;
    int port;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    if(argc > 1)
    {
        port = PORT + atoi(argv[1]);
        sprintf(group, "%s.%s", GROUP, argv[1]);  

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
                putchar(message[c]);
                cnt--;
                c++;
            }
            fflush(stdout);
        }
    }
}

