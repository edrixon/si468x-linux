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

extern unsigned int spi;
extern unsigned char spiBuf[];

extern int dab_freqs;
extern uint32_t dab_freq[];

void dabSigint(int signum)
{
    dabShMem -> engineVersion = 0;
    exit(0);
}

int main(int argc, char *arv[])
{
    int c;
    struct sigaction sigintAction;

    printf("\n");
    printf("DAB receiver control engine version %d.%d\n",
              (DAB_ENGINE_VERSION & 0xff00) >> 8, DAB_ENGINE_VERSION & 0x00ff);
    printf("\n");

    shmBegin();
    for(c = 0; c < dab_freqs; c++)
    {
        dabShMem -> dabFreq[c].freq = dab_freq[c];
        dabShMem -> dabFreq[c].serviceValid = TRUE;
        strcpy(dabShMem -> dabFreq[c].ensemble, "unknown");
    }
    dabShMem -> dabFreqs = dab_freqs;

    dabShMem -> dabCmd.cmd = DABCMD_NONE;
    dabShMem -> dabCmd.rtn = DABRET_READY;

    dabShMem -> engineVersion = DAB_ENGINE_VERSION;

    sem_init(&(dabShMem -> semaphore), 99, 1);

    gpioInitialise();

    gpioSetMode(DAB_RESET_PIN, PI_OUTPUT);
    gpioSetMode(DAB_INT_PIN, PI_INPUT);
    gpioSetPullUpDown(DAB_INT_PIN, PI_PUD_UP);
    gpioSetMode(DAB_ENABLE_PIN, PI_OUTPUT);

    gpioSetAlertFunc(DAB_INT_PIN, dabInterrupt);

    spi = spiOpen(SPI_SI_CHANNEL, SPI_SPEED, SPI_FLAGS);
    if(spi < 0)
    {
        printf("Error: opening SPI port for Si468x\n");
    }


    sigintAction.sa_handler = dabSigint;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_flags = 0;
    sigaction(SIGINT, &sigintAction, NULL);

    dabBegin();

    dabMain();

    spiClose(spi);
    gpioTerminate();
}
