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
#include <ctype.h>

#define __IN_MAIN

#include "../types.h"
#include "../dabshmem.h"
#include "../dabcmd.h"
#include "../shm.h"

#include "httpd.h"
#include "globals.h"

int main(int argc, char *argv[])
{
    fprintf(stderr, "%d\n", getpid());
    setvbuf(stdout, NULL, _IOLBF, 0);

    printf("\nRemote DAB receiver HTTP server V1.0\n");

    dabShMem = shmAttach();

    printf("  Waiting for DAB engine to start... ");
    fflush(stdout);
    while(dabShMem -> engineState != DAB_ENGINE_READY);
    printf("OK\n");

    printf("  Radio is Si%d\n", dabShMem -> sysInfo.partNo);
    printf("  Found control engine version %d.%d\n",
                            (dabShMem -> engineVersion & 0xff00) >> 8,
                             dabShMem -> engineVersion & 0x00ff);

    httpd();
}
