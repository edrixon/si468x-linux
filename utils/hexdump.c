#include <stdio.h>
#include <ctype.h>

#define BUFF_SIZE 16

int main(int argc, char *argv[])
{
    char buff[BUFF_SIZE];
    int readed;
    FILE *fp;
    unsigned long offset;
    int c;

    fp = fopen(argv[1], "r");
    if(fp)
    {
        offset = 0;
        do
        {
            readed = fread(buff, sizeof(char), BUFF_SIZE, fp);

            printf("%08lX  ", offset);

            for(c = 0; c < readed; c++)
            {
                printf("%02X ", buff[c]);
            }

            printf("   :");

            for(c = 0; c < readed; c++)
            {
                if(isprint(buff[c]))
                {
                    printf("%c", buff[c]);
                }
                else
                {
                    printf(".");
                }
            }

            printf(":\n");

            offset = offset + BUFF_SIZE;

        }
        while(readed == BUFF_SIZE);

        fclose(fp);
    }
    else
    {
        printf("Cannot open %s\n", argv[1]);
    }
}
