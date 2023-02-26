#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    char tempStr[32];
    double temp;
    int keepGoing;

    keepGoing = 0;

    if(argc == 2 && strcmp(argv[1], "-c") == 0)
    {
        keepGoing = 1;
    }

    do
    {
        fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        if(fp == NULL)
        {
            printf("fopen() error\n");
            return -1;
        }

        if(fscanf(fp, "%s", tempStr) == 1)
        {
            temp = (double)(atoi(tempStr)) / 1000.0;
            printf("CPU temperature = %0.1lf degrees\n", temp);
        }
        else
        {
            printf("fscanf() error\n");
        }

        fclose(fp);

        if(keepGoing == 1)
        {
            sleep(10);
        }
    }
    while(keepGoing == 1);


    return 0;
}
