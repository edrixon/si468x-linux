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
    while ((spiBuf[1] & 0x80) == 0x00);

    if ((spiBuf[1] & 0x40) == 0x40)
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

void siHostLoad()
{
	uint16_t index;
	uint16_t patchsize;
	uint16_t i;

	patchsize = sizeof(rom_patch_016);
	index = 0;

	while (index < patchsize)
	{
		spiBuf[0] = SI46XX_HOST_LOAD;
		spiBuf[1] = 0x00;
		spiBuf[2] = 0x00;
		spiBuf[3] = 0x00;
		for (i = 4; (i < SPI_BUFF_SIZE) && (index < patchsize); i++)
		{
			spiBuf[i] = rom_patch_016[index];
			index++;
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

	spiBuf[4] = property & 0xFF;		//SPI CLock
	spiBuf[5] = (property >> 8) & 0xFF;
	spiBuf[6] = value & 0xFF;
	spiBuf[7] = (value >> 8) & 0xFF;

	siWrite(8);
	siCts();

}

void siSetProperty(uint16_t property, uint16_t value)
{
	spiBuf[0] = SI46XX_SET_PROPERTY;
	spiBuf[1] = 0x00;

	spiBuf[2] = property & 0xFF;
	spiBuf[3] = (property >> 8) & 0xFF;
	spiBuf[4] = value & 0xFF;
	spiBuf[5] = (value >> 8) & 0xFF;

	siWrite(6);
	siCts();
}

void siGetProperty(uint16_t property)
{
	spiBuf[0] = SI46XX_GET_PROPERTY;
	spiBuf[1] = 0x01;
        spiBuf[2] = property & 0x00ff;
        spiBuf[3] = (property >> 8) & 0xff;

        siWrite(4);
        siCts();
}

void siFlashLoad(uint32_t flash_addr)
{
	spiBuf[0] = SI46XX_FLASH_LOAD;
	spiBuf[1] = 0x00;
	spiBuf[2] = 0x00;
	spiBuf[3] = 0x00;

	spiBuf[4] = (flash_addr  & 0xff);
	spiBuf[5] = ((flash_addr >> 8)  & 0xff);
	spiBuf[6] = ((flash_addr >> 16)  & 0xff);
	spiBuf[7] = ((flash_addr >> 24) & 0xff);

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

	spiBuf[0] = SI46XX_DAB_SET_FREQ_LIST;
	spiBuf[1] = DAB_FREQS;
	spiBuf[2] = 0x00;
	spiBuf[3] = 0x00;

	for (i = 0; i < DAB_FREQS; i++)
	{
		freq = dab_freq[i];

		spiBuf[4 + (i * 4)] = (freq  & 0xff);
		spiBuf[5 + (i * 4)] = ((freq >> 8)  & 0xff);
		spiBuf[6 + (i * 4)] = ((freq >> 16)  & 0xff);
		spiBuf[7 + (i * 4)] = ((freq >> 24) & 0xff);
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
	while ((spiBuf[1] & 0x01) == 0); //STCINT
}

void siStopDigitalService(uint32_t serviceID, uint32_t compID)
{
    spiBuf[0] = SI46XX_DAB_START_DIGITAL_SERVICE;
    spiBuf[1] = 0x00; // Audio service, 1 is data service
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    spiBuf[4] = serviceID & 0xff;
    spiBuf[5] = (serviceID >> 8) & 0xff;
    spiBuf[6] = (serviceID >> 16) & 0xff;
    spiBuf[7] = (serviceID >> 24) & 0xff;
    spiBuf[8] = compID & 0xff;
    spiBuf[9] = (compID >> 8) & 0xff;
    spiBuf[10] = (compID >> 16) & 0xff;
    spiBuf[11] = (compID >> 24) & 0xff;
    siWrite(12);
    siCts();
}
void siStartDigitalService(uint32_t serviceID, uint32_t compID)
{
    int c;

    dabShMem -> currentService.ServiceID = serviceID;
    dabShMem -> currentService.CompID = compID;

    c = 0;
    while(c < dabShMem -> numberofservices &&
                          (serviceID != dabShMem -> service[c].ServiceID ||
                                      compID != dabShMem -> service[c].CompID))
    {
        c++;
    }

    if(c == dabShMem -> numberofservices)
    {
        dabShMem -> currentService.Label[0] = '\0';
    }
    else
    {
        strcpy(dabShMem -> currentService.Label, dabShMem -> service[c].Label);
    }

    spiBuf[0] = SI46XX_DAB_START_DIGITAL_SERVICE;
    spiBuf[1] = 0x00;
    spiBuf[2] = 0x00;
    spiBuf[3] = 0x00;
    spiBuf[4] = serviceID & 0xff;
    spiBuf[5] = (serviceID >> 8) & 0xff;
    spiBuf[6] = (serviceID >> 16) & 0xff;
    spiBuf[7] = (serviceID >> 24) & 0xff;
    spiBuf[8] = compID & 0xff;
    spiBuf[9] = (compID >> 8) & 0xff;
    spiBuf[10] = (compID >> 16) & 0xff;
    spiBuf[11] = (compID >> 24) & 0xff;
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
	spiBuf[4] = serviceID & 0xff;
	spiBuf[5] = (serviceID >> 8) & 0xff;
	spiBuf[6] = (serviceID >> 16) & 0xff;
	spiBuf[7] = (serviceID >> 24) & 0xff;
	siWrite(8);
	siCts();
}

void siDabGetSubChannelInfo(uint32_t serviceID, uint32_t compID)
{
	spiBuf[0] = SI46XX_DAB_GET_SUBCHAN_INFO;
	spiBuf[1] = 0x00;
	spiBuf[2] = 0x00;
	spiBuf[3] = 0x00;
	spiBuf[4] = serviceID & 0xff;
	spiBuf[5] = (serviceID >> 8) & 0xff;
	spiBuf[6] = (serviceID >> 16) & 0xff;
	spiBuf[7] = (serviceID >> 24) & 0xff;
	spiBuf[8] = compID & 0xff;
	spiBuf[9] = (compID >> 8) & 0xff;
	spiBuf[10] = (compID >> 16) & 0xff;
	spiBuf[11] = (compID >> 24) & 0xff;
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
    siHostLoad();
    siLoadInit();
    siFlashSetProperty(1, 10000);
}
