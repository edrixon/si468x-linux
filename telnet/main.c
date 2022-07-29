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

#include "cli.h"
#include "telnetd.h"

char pBuf[255];

extern int cliDone;
extern char cliBuffer[];

int main(int argc, char *argv[])
{
    dabShMem = shmAttach();

    telnetd();
}
