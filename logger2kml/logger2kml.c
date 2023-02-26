#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLENGTH 1024
#define MAXVALUES 4

typedef enum { TRUE, FALSE } booleanType;
typedef enum { CNR, RSSI } valType;

char lastPosition[128];

unsigned long int mapColours[MAXVALUES] = 
{
    0xff000000,  // white
    0xff0000ff,  // red
    0xff00ffff,  // yellow
    0xff00ff00   // green
};

int cnrLimits[MAXVALUES] = { 0, 10, 15, 20 };
int rssiLimits[MAXVALUES] = { 30, 40, 50, 60 };
valType checkValue;
int *valLimits;

void writeKmlHeader()
{
    int dotIconNumber;

    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    printf("  <Document>\n");

    for(dotIconNumber = 0; dotIconNumber < MAXVALUES; dotIconNumber++)
    {
        printf("    <Style id=\"dotIcon%d\">\n", dotIconNumber);
        printf("       <IconStyle>\n");
        printf("         <color>%08X</color>\n", mapColours[dotIconNumber]);
        printf("         <colorMode>normal</colorMode>\n");
        printf("         <scale>1.00</scale>\n");
        printf("         <Icon><href>dot.png</href></Icon>\n");
        printf("       </IconStyle>\n");
        printf("    </Style>\n");
        printf("\n");
    }

    printf("    <Style id=\"pathNormal\">\n");
    printf("      <LineStyle>\n");
    printf("        <color>FF0000FF</color>\n");
    printf("        <colorMode>normal</colorMode>\n");
    printf("        <width>2.00</width>\n");
    printf("      </LineStyle>\n");
    printf("    </Style>\n");
    printf("\n");
}

void addPlacemark(char *line)
{
    int dotIconNumber;
    char *valPtr; 
    char position[128];
    char latitude[64];
    char placeName[64];
    double freq;
    char block[16];
    char ensemble[32];
    int rssi;
    int snr;
    int cnr;
    int plotValue;
    int ficQuality;
    booleanType gotValRange;
    int *chkValPtr;

    // Date
    valPtr = strtok(line, ",");

    // Time
    valPtr = strtok(NULL, ",");
    strcpy(placeName, valPtr);

    // Latitude
    valPtr = strtok(NULL, ",");
    strcpy(latitude, valPtr);

    // Longitude
    valPtr = strtok(NULL, ",");
    sprintf(position, "%s,%s", valPtr, latitude);

    // RSSI
    valPtr = strtok(NULL, ",");
    rssi = atoi(valPtr);

    // SNR
    valPtr = strtok(NULL, ",");
    snr = atoi(valPtr);

    // CNR
    valPtr = strtok(NULL, ",");
    cnr = atoi(valPtr);

    // FIC quality
    valPtr = strtok(NULL, ",");
    ficQuality = atoi(valPtr);

    // Frequency
    valPtr = strtok(NULL, ",");
    freq = atof(valPtr);

    // Channel block 
    valPtr = strtok(NULL, ",");
    strcpy(block, valPtr);

    // Ensemble name
    valPtr = strtok(NULL, ",\n");
    strcpy(ensemble, valPtr);

    if(checkValue == CNR)
    {
        plotValue = cnr;
    }
    else
    {
        plotValue = rssi;
    }

    dotIconNumber = MAXVALUES - 1;
    gotValRange = FALSE;
    while(dotIconNumber >= 0 && gotValRange == FALSE)
    {
        if(plotValue > valLimits[dotIconNumber])
        {
            gotValRange = TRUE;
        }
        else
        {
            dotIconNumber--;
        }
    }

    printf("    <Placemark>\n");
    if(checkValue == CNR)
    {
//        printf("      <name>CNR %d</name>\n", plotValue);
    }
    else
    {
//        printf("      <name>RSSI %d</name>\n", plotValue);
    }
    printf("      <styleUrl>#dotIcon%d</styleUrl>\n", dotIconNumber);
    printf("      <description>\n");
    printf("        <![CDATA[\n");
    printf("          <h1>%s</h1>\n", ensemble);
    printf("          <br>Frequency: %3.3lf MHz (%s)</br>\n", freq, block);
    printf("          <br>RSSI: %d dBuV</br>\n", rssi);
    printf("          <br>CNR: %d dB</br>\n", cnr);
    printf("          <br>SNR: %d dB</br>\n", snr);
    printf("          <br>FIC quality: %d %%</br>\n", ficQuality);
    printf("        ]]>\n");
    printf("      </description>\n");
    printf("      <Point>\n");
    printf("        <coordinates>%s</coordinates>\n", position);
    printf("      </Point>\n");
    printf("    </Placemark>\n");
    printf("\n");

    if(lastPosition[0] != '\0')
    {
        printf("    <Placemark>\n");
        printf("      <styleUrl>#pathNormal</styleUrl>\n");
        printf("      <MultiGeometry>\n");
        printf("        <LineString>\n");
        printf("          <tessellate>1</tessellate>\n");
        printf("          <coordinates>\n");
        printf("            %s %s\n", lastPosition, position);
        printf("          </coordinates>\n");
        printf("        </LineString>\n");
        printf("      </MultiGeometry>\n");
        printf("    </Placemark>\n");
        printf("\n");
    }

    strcpy(lastPosition, position);
}

void writeKmlFooter()
{
    printf("  </Document>\n");
    printf("</kml>\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    FILE *fp;
    int readed;
    char *lineBuffer;
    size_t lineLength;
    char *inFilename;
    booleanType badArg;

    checkValue = CNR;
    badArg = FALSE;

    switch(argc)
    {
        case 2:
            inFilename = argv[1];
            break;

        case 3:
            inFilename = argv[2];
            if(strcmp(argv[1], "-rssi") == 0)
            {
                checkValue = RSSI;
            }
            else
            {
                if(strcmp(argv[1], "-cnr") == 0)
                {
                    checkValue = CNR;
                }
                else
                {
                    badArg = TRUE;
                }
            }
            break;

        default:;
    }

    if(badArg == TRUE)
    {
        printf("logger2kml [-cnr | -rssi] <logfile name>\n");
        exit(-1);
    }

    if(checkValue == CNR)
    {
        valLimits = cnrLimits;
    }
    else
    {
        valLimits = rssiLimits;
    }

    fp = fopen(inFilename, "r");
    if(fp == NULL)
    {
        perror("fopen()");
        exit(-1);
    }

    lineBuffer = (char *)malloc(MAXLENGTH);
    if(lineBuffer == NULL)
    {
        perror("malloc()");
        exit(-1);
    }

    writeKmlHeader();

    lastPosition[0] = '\0';
    do
    {
        readed = getline(&lineBuffer, &lineLength, fp);
        if(readed != -1)
        {
            addPlacemark(lineBuffer);
        }
    }
    while(readed != -1);

    writeKmlFooter();

    free(lineBuffer);

    fclose(fp);
}
