#ifndef __GOT_DABSHMEM

#define __GOT_DABSHMEM

#define DAB_SHMEM_NAME "/dev/shm/dab-shared"

#endif

#ifdef __IN_MAIN

dabShMemType *dabShMem;

#else

extern dabShMemType *dabShMem;

#endif
