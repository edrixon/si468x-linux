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

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dab.h"

extern uint8_t spiBuf[];

void dabshieldPowerup()
{
    gpioWrite(DAB_ENABLE_PIN, 0);
    milliSleep(500);
    gpioWrite(DAB_ENABLE_PIN, 1);
    milliSleep(100);
}

void dabshieldReset()
{
    gpioWrite(DAB_RESET_PIN, 0);
    milliSleep(100);
    gpioWrite(DAB_RESET_PIN, 1); 
    milliSleep(100);
}

unsigned long int timeMillis()
{
    struct timeval time;

    gettimeofday(&time, NULL);

    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

void milliSleep(int ms)
{
    struct timespec req;
    struct timespec rem;

    req.tv_sec = 0;
    req.tv_nsec = ms * 1000000;
    nanosleep(&req, &rem);
}

void hdSpiResponse(int len)
{
    int c;

    c = 0;
    while(len)
    {
        printf("%02x ", spiBuf[c]);
        c++;
        len--;

        if(c % 16 == 0)
        {
            printf("\n");
        }
    }

    printf("\n");
}

uint16_t spiBytesTo16(uint8_t *dPtr)
{
    uint16_t rtn;

    rtn = dPtr[0] + ((uint16_t)dPtr[1] << 8);

    return rtn; 
}

uint32_t spiBytesTo32(uint8_t *dPtr)
{
    uint32_t rtn;

    rtn = dPtr[0] +
          ((uint32_t)dPtr[1] << 8) +
          ((uint32_t)dPtr[2] << 16) +
          ((uint32_t)dPtr[3] << 24);

    return rtn;
}
