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
    int c;

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
        }
        else
        {
            printf("fscanf() error\n");
        }

        fclose(fp);

        if(keepGoing == 1)
        {
            printf("%0.1lf :", temp);
            c = ((temp - 30) * 5) - 20;
            while(c > 0)
            {
                printf("*");
                c--;
            }
            printf("\n");

//            printf("%d\n", c);

            sleep(10);
        }
        else
        {
            printf("CPU temperature = %0.1lf degrees\n", temp);
        }
    }
    while(keepGoing == 1);


    return 0;
}
