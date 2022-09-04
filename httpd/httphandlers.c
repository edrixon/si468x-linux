#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#include "../types.h"
#include "../dabshmem.h"
#include "../shm.h"
#include "../dabcmd.h"
#include "httpd.h"
#include "httphandlers.h"

#include "globals.h"

void freqIdToBlock(int freqId, char *block)
{
    sprintf(block, "%d%c", (freqId / 4) + 5, (freqId % 4) + 65);
}

int parseFilename(char *fname, int *needLength, char *contentType)
{
    char *cPtr;
    int c;
    int rtn;
    char strBuf[255];

    strcpy(strBuf, fname);

    *contentType = '\0';
    *needLength = FALSE;
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
                *needLength = contentHeaders[c].needLength;
                rtn = TRUE;
            }
        }
    }

    return rtn;
}

void httpSendHeader(char *fName, int respCode, int needLength, char *contentType)
{
    struct stat statBuff;

    sprintf(pBuf, "HTTP/1.1 %d ", respCode);
    switch(respCode)
    {
        case 404:
            strcat(pBuf, "Not found\n");
            break;

        default:
            strcat(pBuf, "OK\n");
            break;
    }
    tputsCRLF(TRUE, pBuf);

//    tputsCRLF(TRUE, "Connection: Keep-alive\n");
//    tputsCRLF(TRUE, "Keep-alive: timeout=20, max=10\n");
    tputsCRLF(TRUE, "Server: dab radio\n");

    sprintf(pBuf, "Content-type: %s\n", contentType);
    tputsCRLF(TRUE, pBuf);

    if(needLength == TRUE)
    {
        stat(fName, &statBuff);

        sprintf(pBuf, "Content-length: %ld\n", statBuff.st_size);
        tputsCRLF(TRUE, pBuf);
    }

    tputsCRLF(TRUE, "\n"); 
}

void httpGetServiceData(char **params)
{
    httpSendHeader(NULL, 200, FALSE, "application/json");

    tputs("{\n");
    sprintf(pBuf, "  \"serviceDataMs\" : %ld,\n", dabShMem -> serviceDataMs);
    tputs(pBuf);
    sprintf(pBuf, "  \"serviceData\" : \"%s\"\n", dabShMem -> serviceData);
    tputs(pBuf);
    tputs("}\n");
}

void httpGetFreqs(char **params)
{
    dabFreqType *dFreq;
    int c;
    char block[8];
    int freqId;
 
    dFreq = dabShMem -> dabFreq; 

    httpSendHeader(NULL, 200, FALSE, "application/json");
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
    httpSendHeader(NULL, 200, FALSE, "application/json");
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

    httpSendHeader(NULL, 200, FALSE, "application/json");
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

    httpSendHeader(NULL, 200, FALSE, "application/json");
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

void httpSetChannel(char **params)
{
    char *p1;
    dabCmdType dabCmd;
    char *fName;
    int channelBlock;
    char strBuf[255];

    strcpy(strBuf, *params);
    fName = strtok(strBuf, "&");
    p1 = strtok(NULL, "&");
    if(p1 == NULL)
    {
        printf("Bad parameter\n");
        return;
    }

    channelBlock = atoi(p1);

    dabCmd.params.service.Freq = channelBlock;
    dabCmd.cmd = DABCMD_TUNEFREQ;  
    doCommand(&dabCmd, NULL);

    httpGetCurrent(params);
}

void httpSetService(char **params)
{
    char *p1;
    char *p2;
    char *fName;
    dabCmdType dabCmd;
    int serviceId;
    int compId;
    char strBuf[255];

    strcpy(strBuf, *params);
    fName = strtok(strBuf, "&");
    p1 = strtok(NULL, "&");
    p2 = strtok(NULL,"&");
    if(p1 == NULL || p2 == NULL)
    {
        printf("Bad parameter\n");
        return;
    }

    serviceId = atoi(p1);
    compId = atoi(p2);

    dabCmd.params.service.ServiceID = serviceId;
    dabCmd.params.service.CompID = compId;
    dabCmd.cmd = DABCMD_TUNE;  
    doCommand(&dabCmd, NULL);

    httpGetCurrent(params);
}

int httpBuiltInFile(char **params)
{
    // Handle the json stuff

    char *fName;
    char strBuf[255];
    int c;

    strcpy(strBuf, *params);
    fName = strtok(strBuf, "&");

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

int httpSendFile(int respCode, char *fname)
{
    char fullFname[255];
    unsigned char rdBuff[1400];
    char contentType[64];
    int readed;
    int needLength;
    long bytesSent;
    FILE *fp;

    bytesSent = 0;

    sprintf(fullFname, "%s/%s", WWW_ROOT, fname);

    fp = fopen(fullFname, "r");
    if(!fp)
    {
        return FALSE;
    }

    if(parseFilename(fname, &needLength, contentType) == TRUE)
    {
        httpSendHeader(fullFname, respCode, needLength, contentType);
    }
    else
    {
        httpSendHeader(fullFname, respCode, FALSE, "html/text");
    }

    do
    {
        readed = fread(rdBuff, 1, 1400, fp);
        write(connFd, rdBuff, readed);
        bytesSent = bytesSent + readed;
    }
    while(readed == 1400);

    fclose(fp);

    return TRUE;
}

void httpGet(char **params)
{
    int sent;

    printf("GET %s\n", *params);

    if(strcmp(*params, "/") == 0 || strcmp(*params, "/index.htm") == 0)
    {
        sent = httpSendFile(200, "index.html");
    }
    else
    {
        sent = httpSendFile(200, *params);
    }

    if(sent != TRUE)
    {
        // Real file not found response, try built-in file
        if(httpBuiltInFile(params) != TRUE)
        {
            httpSendFile(404, "404.html");
        }
    }

    sleep(1);
}

int doCommand(dabCmdType *cmd, dabCmdRespType *resp)
{
    int rtn;

    shmLock();
    cmd -> userPid = getpid();
    bcopy(cmd, &(dabShMem -> dabCmd), sizeof(dabCmdType));
    while(dabShMem -> dabCmd.cmd != DABCMD_NONE);
    rtn = dabShMem -> dabCmd.rtn;
    if(resp != NULL)
    {
        bcopy(&(dabShMem -> dabResp), resp, sizeof(dabCmdRespType));
    }
    shmFree();

    return rtn;
}
