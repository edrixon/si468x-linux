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
#include <signal.h>

#define __IN_MAIN

#include <pigpio.h>

#include "types.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dab.h"
#include "globals.h"

#ifdef __ARSE

SPI_FLAGS definition

31 x
30 x
29 x
28 x

27 x
26 x
25 x
24 x

23 x
22 x
21 0   8 bit word size
20 0

19 0
18 0
17 0
16 0

15 1   receive msb first
14 1   transmit msb first
13 0   number of bytes before switching mosi to be miso
12 0

11 0
10 0   
9  0   not 3-wire mode
8  0   main SPI

7  0   exclusive CE
6  0
5  0
4  0   active low CE

3  0
2  0
1  0   mode 0
0  0


#endif

void dabShmInit()
{
    int c;

    dabShMem -> engineState = DAB_ENGINE_NOTREADY;
    dabShMem -> engineVersion = DAB_ENGINE_VERSION;

    for(c = 0; c < DAB_FREQS; c++)
    {
        dabShMem -> dabFreq[c].freq = dab_freq[c];
        dabShMem -> dabFreq[c].serviceValid = TRUE;
        strcpy(dabShMem -> dabFreq[c].ensemble, "unknown");
    }

    dabShMem -> dabFreqs = DAB_FREQS;

    dabShMem -> dabCmd.cmd = DABCMD_NONE;
    dabShMem -> dabCmd.rtn = DABRET_READY;

    sem_init(&(dabShMem -> semaphore), 99, 1);

}


void dabGpioInit()
{
    gpioInitialise();

    gpioSetMode(DAB_RESET_PIN, PI_OUTPUT);
    gpioSetMode(DAB_INT_PIN, PI_INPUT);
    gpioSetPullUpDown(DAB_INT_PIN, PI_PUD_UP);
    gpioSetMode(DAB_ENABLE_PIN, PI_OUTPUT);

    gpioSetISRFunc(DAB_INT_PIN, FALLING_EDGE, -1, dabInterrupt);

}

int main(int argc, char *arv[])
{
    printf("\n");
    printf("DAB receiver control engine version %d.%d\n",
              (DAB_ENGINE_VERSION & 0xff00) >> 8, DAB_ENGINE_VERSION & 0x00ff);
    printf("\n");

    shmBegin();
    dabShmInit();

    dabGpioInit();

    spi = spiOpen(SPI_SI_CHANNEL, SPI_SPEED, SPI_FLAGS);
    if(spi < 0)
    {
        printf("Error: opening SPI port for Si468x\n");
    }

    dabMain();

    spiClose(spi);
    gpioTerminate();
}
