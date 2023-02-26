#include <stdio.h>
#include <strings.h>

#include "si468xROM.h"

#define BUFFLEN 256

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char writeBuffer[BUFFLEN];
    int writeSize;
    unsigned char *bPtr;
    int dataSize;

    fp = fopen("rompatch.bin", "w");

    dataSize = sizeof(rom_patch_016);
    printf("Length = %d\n", dataSize);

    bPtr = rom_patch_016;
    do
    {
        if(dataSize > BUFFLEN)
        {
            writeSize = BUFFLEN;
        }
        else
        {
            writeSize = dataSize;
        }

        printf("*"); fflush(stdout);
        bcopy(bPtr, writeBuffer, writeSize);
        fwrite(writeBuffer, 1, writeSize, fp);

        dataSize = dataSize - writeSize;
        bPtr = bPtr + writeSize;
    }
    while(dataSize);
    printf("\n");

    fclose(fp);

    return 0;
}
