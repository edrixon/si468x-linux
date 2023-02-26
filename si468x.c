//
// Ed's Si468x Linux driver for DABShield Arduino board
// Code based on Arduino C++ library
// Supports DAB functions only
//

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
#include "si468xROM.h"
#include "si468x.h"
#include "dabshmem.h"
#include "dabcmd.h"
#include "shm.h"
#include "utils.h"
#include "dab.h"
#include "globals.h"

void siSplit32(uint8_t *bPtr, uint32_t wrd)
{
    int c;
    uint32_t mask;
    int sft;

    sft = 0;
    mask = 0x000000ff;

    for(c = 0; c < 4; c++)
    {
        *bPtr = (wrd & mask) >> sft;

        bPtr++;
        mask = mask << 8;
        sft = sft + 8;
    }
}

void siSplit16(uint8_t *bPtr, uint16_t wrd)
{
    *bPtr = (wrd & 0x00ff);
    *(bPtr + 1) = (wrd & 0xff00) >> 8;
}

void siWrite(uint32_t len)
{
    spiXfer(spi, (char *)spiBuf, (char *)spiBuf, len);
}

void siResponseN(int len)
{
    len++;

    bzero(spiBuf, len);
    siWrite(len);
}

void siResponse(void)
{
    siResponseN(4);
}

void siCts()
{
    uint32_t timeout;

    command_error = 0;
    timeout = 1000;
    do
    {
        milliSleep(4);
        siResponse();
        timeout--;
        if(timeout == 0)
        {
            printf("Error: SPI timeout waiting for CTS\n");
            command_error = 0x80;
            break;
        }
    }
    while((spiBuf[1] & 0x80) != 0x80);

    if((spiBuf[1] & 0x40) == 0x40)
    {
        siResponseN(5);
        command_error = 0x80 | spiBuf[5];
    }
}

void siGetPartInfo(void)
{
    spiBuf[0] = SI46XX_GET_PART_INFO;
    spiBuf[1] = 0x00;
    siWrite(2);
    siCts();
}

void siGetFuncInfo(void)
{
    spiBuf[0] = SI46XX_GET_FUNC_INFO;
    spiBuf[1] = 0x00;
    siWrite(2);
    siCts();
}

void siGetSysState()
{
    bzero(spiBuf, 32);
    spiBuf[0] = SI46XX_GET_SYS_STATE;
    spiBuf[1] = 0x00;
    siWrite(2);
    siCts();
}

void siPowerup()
{
    spiBuf[0] = SI46XX_POWER_UP;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x17;
    spiBuf[3] = 0x48;
    spiBuf[4] = 0x00;
    spiBuf[5] = 0xf8;
    spiBuf[6] = 0x24;
    spiBuf[7] = 0x01;
    spiBuf[8] = 0x1F;
    spiBuf[9] = 0x10;
    spiBuf[10] = 0x00;
    spiBuf[11] = 0x00;
    spiBuf[12] = 0x00;
    spiBuf[13] = 0x18;
    spiBuf[14] = 0x00;
    spiBuf[15] = 0x00;

    siWrite(16);
    siCts();

    spiBuf[0] = SI46XX_POWER_UP;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x17;
    spiBuf[3] = 0x48;
    spiBuf[4] = 0x00;
    spiBuf[5] = 0xf8;
    spiBuf[6] = 0x24;
    spiBuf[7] = 0x01;
    spiBuf[8] = 0x1F;
    spiBuf[9] = 0x10;
    spiBuf[10] = 0x00;
    spiBuf[11] = 0x00;
    spiBuf[12] = 0x00;
    spiBuf[13] = 0x18;
    spiBuf[14] = 0x00;
    spiBuf[15] = 0x00;

    siWrite(16);
    siCts();
}

void siLoadInit()
{
    spiBuf[0] = SI46XX_LOAD_INIT;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siHostLoad(uint8_t *patch, uint16_t patchSize)
{
    uint16_t bytesLoaded; 
    uint16_t i;

    bytesLoaded = 0;

    while(bytesLoaded < patchSize)
    {
        spiBuf[0] = SI46XX_HOST_LOAD;
        spiBuf[1] = 0x00;
        spiBuf[2] = 0x00;
        spiBuf[3] = 0x00;
        for(i = 4; (i < SPI_BUFF_SIZE) && (bytesLoaded < patchSize); i++)
        {
            spiBuf[i] = *patch;

            patch++;
            bytesLoaded++;
        }

        siWrite(i);
        siCts();
    }
}

void siFlashSetProperty(uint16_t property, uint16_t value)
{
    spiBuf[0] = SI46XX_FLASH_LOAD;
    spiBuf[1] = 0x10;
    spiBuf[2] = 0x0;
    spiBuf[3] = 0x0;

    siSplit16(&spiBuf[4], property);
    siSplit16(&spiBuf[6], value);

    siWrite(8);
    siCts();
}

void siSetProperty(uint16_t property, uint16_t value)
{
    spiBuf[0] = SI46XX_SET_PROPERTY;
    spiBuf[1] = 0x00;

    siSplit16(&spiBuf[2], property);
    siSplit16(&spiBuf[4], value);

    siWrite(6);
    siCts();
}

void siGetProperty(uint16_t property)
{
    spiBuf[0] = SI46XX_GET_PROPERTY;
    spiBuf[1] = 0x01;
    siSplit16(&spiBuf[2], property);

    siWrite(4);
    siCts();
}

void siFlashLoad(uint32_t flash_addr)
{
    spiBuf[0] = SI46XX_FLASH_LOAD;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;

    siSplit32(&spiBuf[4], flash_addr);

    spiBuf[8] = 0x00;
    spiBuf[9] = 0x00;
    spiBuf[10] = 0x00;
    spiBuf[11] = 0x00;

    siWrite(12);
    siCts();
}

void siBoot()
{
    spiBuf[0] = SI46XX_BOOT;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siSetFreqList()
{
    uint8_t i;
    uint32_t freq;
    int c;

    spiBuf[0] = SI46XX_DAB_SET_FREQ_LIST;
    spiBuf[1] = numDabMhz;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;

    for(i = 0; i < numDabMhz; i++)
    {
        freq = dabMhz[i];

        c = (i * 4) + 4;
        siSplit32(&spiBuf[c], freq);
    }

    siWrite(4 + (i * 4));
    siCts();
}

void siDabTuneFreq(uint8_t freq_index)
{
    uint32_t timeout;

    spiBuf[0] = SI46XX_DAB_TUNE_FREQ;
    spiBuf[1] = 0x00;
    spiBuf[2] = freq_index;
    spiBuf[3] = 0x00;
    spiBuf[4] = 0x00;
    spiBuf[5] = 0x00;
    siWrite(6);
    siCts();

    timeout = 1000;
    do
    {
        milliSleep(4);
        siResponse();
        timeout--;
        if(timeout == 0)
        {
            command_error |= 0x80;
            break;
        }
    }
    while((spiBuf[1] & 0x01) != 0x01); //STCINT
}

void siStopDigitalService(uint32_t serviceID, uint32_t compID)
{
    spiBuf[0] = SI46XX_DAB_START_DIGITAL_SERVICE;
    spiBuf[1] = 0x00; // Audio service, 1 is data service
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    siSplit32(&spiBuf[4], serviceID);
    siSplit32(&spiBuf[8], compID);
    siWrite(12);
    siCts();
}
void siStartDigitalService(uint32_t serviceID, uint32_t compID)
{
    spiBuf[0] = SI46XX_DAB_START_DIGITAL_SERVICE;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    siSplit32(&spiBuf[4], serviceID);
    siSplit32(&spiBuf[8], compID);
    siWrite(12);
    siCts();
}

void siDabGetEnsembleInfo(void)
{
    spiBuf[0] = SI46XX_DAB_GET_ENSEMBLE_INFO;
    spiBuf[1] = 0x00;
    siWrite(2);
    siCts();
}

void siDabGetServiceInfo(uint32_t serviceID)
{
    spiBuf[0] = SI46XX_DAB_GET_SERVICE_INFO;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    siSplit32(&spiBuf[4], serviceID);
    siWrite(8);
    siCts();
}

void siDabGetSubChannelInfo(uint32_t serviceID, uint32_t compID)
{
    spiBuf[0] = SI46XX_DAB_GET_SUBCHAN_INFO;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    siSplit32(&spiBuf[4], serviceID);
    siSplit32(&spiBuf[8], compID);

    siWrite(12);
    siCts();
}

void siDabGetAudioInfo(void)
{
    spiBuf[0] = SI46XX_DAB_GET_AUDIO_INFO;
    spiBuf[1] = 0x00;
    siWrite(2);
    siCts();
}

void siDabDigRadStatus(void)
{
    spiBuf[0] = SI46XX_DAB_DIGRAD_STATUS;
    spiBuf[1] = 0x09; //Clear Interrupts: DIGRAD_ACK | STC_ACK
    siWrite(2);
    siCts();
}

void siGetDigitalServiceData(void)
{
    spiBuf[0] = SI46XX_GET_DIGITAL_SERVICE_DATA;
    spiBuf[1] = 0x01;
    siWrite(2);
    siCts();
}

void siGetRssi(void)
{
    spiBuf[0] = SI46XX_DAB_GET_RSSI;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siGetTime(void)
{
    spiBuf[0] = SI46XX_GET_TIME;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siDabGetEventStatus(void)
{
    spiBuf[0] = SI46XX_DAB_GET_EVENT_STATUS;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siDabGetAcfStatus(void)
{
    spiBuf[0] = SI46XX_DAB_GET_ACF_STATUS;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siGetDigitalServiceList(void)
{
    spiBuf[0] = SI46XX_DAB_GET_DIGITAL_SERVICE_LIST;
    spiBuf[1] = 0x00;

    siWrite(2);
    siCts();
}

void siInitDab()
{
    siFlashLoad(0x6000);
    siBoot();
    siSetFreqList();

    // Enable DSRVINT interrupt
    siSetProperty(0x0000, 0x0010);

    // Front end tuning magic numbers
    siSetProperty(0x1710, 0xF9FF);
    siSetProperty(0x1711, 0x0172);
    siSetProperty(0x1712, 0x0001);

    //enable DSRVPCKTINT
    siSetProperty(0x8100, 0x0001);

    //enable XPAD data
    siSetProperty(0xb400, 0x0007);
}

void siReset()
{
    siPowerup();
    siLoadInit();
    siHostLoad(rom_patch_016, sizeof(rom_patch_016));
    siLoadInit();
    siFlashSetProperty(1, 10000);
}
