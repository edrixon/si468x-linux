#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "../types.h"
#include "../dabshmem.h"
#include "../shm.h"
#include "httpd.h"

#include "globals.h"

void freqIdToBlock(int freqId, char *block)
{
    sprintf(block, "%d%c", (freqId / 4) + 5, (freqId % 4) + 65);
}

int parseFilename(char *fname, char *contentType)
{
    char *cPtr;
    int c;
    int rtn;
    char strBuf[255];

    strcpy(strBuf, fname);

    *contentType = '\0';
    rtn = FALSE;

    cPtr = strtok(strBuf, ".");
    if(cPtr)
    {
        cPtr = strtok(NULL, ".");
        if(cPtr)
        {
            c = 0;
            while(contentHeaders[c].fileExtension[0] != '\0' &&
                            strcmp(contentHeaders[c].fileExtension, cPtr) != 0)
            {
                c++;
            }

            if(contentHeaders[c].fileExtension[0] != '\0')
            {
                strcpy(contentType, contentHeaders[c].mimeType);
                rtn = TRUE;
            }
        }
    }

    return rtn;
}

void httpSendHeader(char *contentType)
{
    tputs("HTTP/1.1 200 OK\n");
    tputs("Connection: Keep-alive\n");
    tputs("Server: dab radio\n");
    if(contentType == NULL)
    {
        tputs("Content-type: text/html\n");
    }
    else
    {
        sprintf(pBuf, "Content-type: %s\n", contentType);
    }
    tputs("\n");
}

void httpGetFreqs(char **params)
{
    dabFreqType *dFreq;
    int c;
    char block[8];
    int freqId;
 
    dFreq = dabShMem -> dabFreq; 

    httpSendHeader("application/json");
    tputs("{\n");

    sprintf(pBuf, "  \"numFreq\" : %d,\n", dabShMem -> dabFreqs);
    tputs(pBuf);
    sprintf(pBuf, "  \"freqs\" : [\n");
    tputs(pBuf);
   

    freqId = 0;
    c = dabShMem -> dabFreqs;
    while(c > 0)
    {
        tputs("      {\n"); 

        freqIdToBlock(freqId, block);
        sprintf(pBuf, "        \"block\" : \"%s\",\n", block);
        tputs(pBuf);
        sprintf(pBuf, "        \"freq\" : \"%3.3lf MHz\",\n",
                                                       dFreq -> freq / 1000.0);
        tputs(pBuf);
        sprintf(pBuf, "        \"ensemble\" : \"%s\",\n", dFreq -> ensemble);
        tputs(pBuf);

        sprintf(pBuf, "        \"rssi\" : \"%d dBuV\",\n",
                                                     dFreq -> sigQuality.rssi);
        tputs(pBuf);
        sprintf(pBuf, "        \"snr\" : \"%d dB\",\n",
                                                      dFreq -> sigQuality.snr);
        tputs(pBuf);
        sprintf(pBuf, "        \"cnr\" : \"%d dB\",\n",
                                                      dFreq -> sigQuality.cnr);
        tputs(pBuf);
        sprintf(pBuf, "        \"ficQuality\" : \"%d %%\"\n",
                                               dFreq -> sigQuality.ficQuality);
        tputs(pBuf);

        c--;
        dFreq++;
        freqId++;

        if(c == 0)
        {
            tputs("      }\n"); 
        }
        else
        {
            tputs("      },\n"); 
        }
    }

    tputs("    ]\n");
    tputs("}\n");

}

void httpGetSystem(char **params)
{
    httpSendHeader("application/json");
    tputs("{\n");
    sprintf(pBuf, "  \"partNo\" : \"Si%d\",\n", dabShMem -> sysInfo.partNo);
    tputs(pBuf);
    sprintf(pBuf, "  \"swVer\" : \"%d.%d.%d\"\n", dabShMem -> funcInfo.major,
                       dabShMem -> funcInfo.minor, dabShMem -> funcInfo.build);
    tputs(pBuf);
    tputs("}\n");
}

void httpGetEnsemble(char **params)
{
    DABService *dServ;
    int c;

    dServ = &(dabShMem -> service[0]);
    c = dabShMem -> numberofservices;

    httpSendHeader("application/json");
    tputs("{\n");

    sprintf(pBuf, "  \"numServices\" : \"%d\",\n", c);
    tputs(pBuf);

    tputs("  \"services\" : [\n");

    while(c)
    {
        tputs("      {\n"); 

        sprintf(pBuf, "         \"label\" : \"%s\",\n", dServ -> Label);
        tputs(pBuf);

        sprintf(pBuf, "         \"pty\" : \"%d\",\n", dServ -> programmeType);
        tputs(pBuf);

        sprintf(pBuf, "         \"compid\" : \"%d\",\n", dServ -> CompID);
        tputs(pBuf);

        sprintf(pBuf, "         \"servid\" : \"%d\"\n", dServ -> ServiceID);
        tputs(pBuf);

        dServ++;
        c--;

        if(c == 0)
        {
            tputs("      }\n"); 
        }
        else
        {
            tputs("      },\n"); 
        }
    }

    tputs("    ]\n");
    tputs("}\n");
}

void httpGetCurrent(char **params)
{
    dabFreqType *dFreq;
    int freqId;
    char timeStr[64];
    char block[8];

    freqId = dabShMem -> currentService.Freq;
    dFreq = &(dabShMem -> dabFreq[freqId]); 

    httpSendHeader("application/json");
    tputs("{\n");

    if(dabShMem -> time.tm_year > 0)
    {
        strftime(timeStr, 64, "%d %B %Y, %H:%M", &(dabShMem -> time));
    }
    else
    {
        strcpy(timeStr, "Date not available");
    }
    sprintf(pBuf, "  \"time\" : \"%s\",\n", timeStr);
    tputs(pBuf);

    sprintf(pBuf, "  \"freqID\" : \"%d\",\n", dabShMem -> currentService.Freq);
    tputs(pBuf);
    freqIdToBlock(dabShMem -> currentService.Freq, block);
    sprintf(pBuf, "  \"block\" : \"%s\",\n", block);
    tputs(pBuf);
    sprintf(pBuf, "  \"freq\" : \"%3.3lf MHz\",\n", dFreq -> freq / 1000.0);
    tputs(pBuf);
    sprintf(pBuf, "  \"ensembleName\" : \"%s\",\n", dFreq -> ensemble);
    tputs(pBuf);
    sprintf(pBuf, "  \"serviceLabel\" : \"%s\",\n",
                                             dabShMem -> currentService.Label);
    tputs(pBuf);
    sprintf(pBuf, "  \"serviceID\" : \"0x%08x\",\n",
                                         dabShMem -> currentService.ServiceID);
    tputs(pBuf);
    sprintf(pBuf, "  \"componentID\" : \"0x%08x\",\n",
                                            dabShMem -> currentService.CompID);
    tputs(pBuf);
    sprintf(pBuf, "  \"programmeType\" : \"0x%02x\",\n",
                                     dabShMem -> currentService.programmeType);
    tputs(pBuf);
    sprintf(pBuf, "  \"rssi\" : \"%d dBuV\",\n", dFreq -> sigQuality.rssi);
    tputs(pBuf);
    sprintf(pBuf, "  \"snr\" : \"%d dB\",\n", dFreq -> sigQuality.snr);
    tputs(pBuf);
    sprintf(pBuf, "  \"cnr\" : \"%d dB\",\n", dFreq -> sigQuality.cnr);
    tputs(pBuf);
    sprintf(pBuf, "  \"ficQuality\" : \"%d %%\"\n",
                                               dFreq -> sigQuality.ficQuality);
    tputs(pBuf);
    tputs("}\n");
}

void httpGetService(char **params)
{
    printf("GET SERVICE\n");

    httpSendHeader(NULL);

    sprintf(pBuf, "Service: %s\n", dabShMem -> currentService.Label);
    tputs(pBuf);
}


int httpBuiltInFile(char **params)
{
    // Handle the json stuff

    char *fName;
    char pBuf[255];
    int c;

    strcpy(pBuf, *params);
    fName = strtok(pBuf, "&");

    c = 0;
    while(httpBuiltIn[c].name[0] != '\0' &&
                                     strcmp(httpBuiltIn[c].name, fName) != 0)
    {
        c++;
    }

    if(httpBuiltIn[c].name[c] != '\0')
    {
        httpBuiltIn[c].fn(params);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int httpSendFile(char *fname)
{
    char fullFname[255];
    unsigned char rdBuff[64];
    char contentType[64];
    int readed;
    FILE *fp;

    sprintf(fullFname, "%s/%s", WWW_ROOT, fname);

    fp = fopen(fullFname, "r");
    if(!fp)
    {
        return FALSE;
    }

    if(parseFilename(fname, contentType) == TRUE)
    {
        httpSendHeader(contentType);
    }
    else
    {
        httpSendHeader(NULL);
    }

    do
    {
        readed = fread(rdBuff, 1, 64, fp);
        write(connFd, rdBuff, readed);
    }
    while(readed == 64);

    fclose(fp);

    return TRUE;
}

void httpGet(char **params)
{
    int sent;

    printf("GET %s\n", *params);

    if(strcmp(*params, "/") == 0 || strcmp(*params, "/index.htm") == 0)
    {
        sent = httpSendFile("index.html");
    }
    else
    {
        sent = httpSendFile(*params);
    }

    if(sent != TRUE)
    {
        // Real file not found response, try built-in file
        if(httpBuiltInFile(params) != TRUE)
        {
            httpSendFile("404.html");
        }
    }

    sleep(1);
}
