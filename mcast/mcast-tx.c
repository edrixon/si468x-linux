//
// Ed's multicast transmitter
// puts stdin into a multicast stream
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

#define TXBUFFLEN 256

int readline(char *buff, int maxlen)
{
    char inchar;
    int charsread;

    charsread = 0;
    do
    {
        inchar = getchar();

        if(inchar == 0xff)
        {
            charsread = -1;
        }
        else
        {
            *buff = inchar;
            buff++;
            charsread++;
        }
    }
    while(charsread < maxlen && inchar != 0xff && inchar != '\n');

    *buff = '\0';

    return charsread;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int addrlen;
    int sock;
    int cnt;
    char message[TXBUFFLEN];
    char group[32];
    int charsread;
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
        addr.sin_addr.s_addr = inet_addr(group);
        addr.sin_port = htons(port);
        addrlen = sizeof(addr);
 
        while(1)
        {
            charsread = readline(message, TXBUFFLEN);
            if(charsread < 0)
            {
                exit(0);
            }

            cnt = sendto(sock, message, charsread, 0,
                                           (struct sockaddr *) &addr, addrlen);

            if(cnt < 0)
            {
                perror("sendto");
                exit(1);
            }
       }
   }
   else
   {
      printf("Missing argument\n");
   }
}
