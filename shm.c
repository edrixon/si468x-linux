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

extern int dab_freqs;
extern uint32_t dab_freq[];

void shmBegin()
{
    dabShMemType newMem;
    int fd;

    fd = open(DAB_SHMEM_NAME, O_RDWR | O_CREAT);

    bzero(&newMem, sizeof(dabShMemType));
    write(fd, &newMem, sizeof(dabShMemType));    

    dabShMem = (dabShMemType *)mmap(NULL, sizeof(dabShMemType),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED,
                                     fd, 0);
}

void shmLock()
{
    sem_wait(&(dabShMem -> semaphore));
}

void shmFree()
{
    sem_post(&(dabShMem -> semaphore));
}

dabShMemType *shmAttach()
{
    dabShMemType *shMem;
    int fd;

    fd = open(DAB_SHMEM_NAME, O_RDWR | O_CREAT);

    shMem = (dabShMemType *)mmap(NULL, sizeof(dabShMemType),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED,
                                     fd, 0);

    return shMem;
}
