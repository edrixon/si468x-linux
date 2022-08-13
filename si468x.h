#ifndef __GOT_SI468X

#define __GOT_SI468X

#define SI46XX_RD_REPLY 				0x00
#define SI46XX_POWER_UP 				0x01
#define SI46XX_HOST_LOAD				0x04
#define SI46XX_FLASH_LOAD 				0x05
#define SI46XX_LOAD_INIT 				0x06
#define SI46XX_BOOT 					0x07
#define SI46XX_GET_PART_INFO 				0x08
#define SI46XX_GET_SYS_STATE 				0x09

#define SI46XX_GET_FUNC_INFO 				0x12

#define SI46XX_SET_PROPERTY 				0x13
#define SI46XX_GET_PROPERTY 				0x14

#define SI46XX_FM_TUNE_FREQ 				0x30
#define SI46XX_FM_SEEK_START 				0x31
#define SI46XX_FM_RSQ_STATUS 				0x32
#define SI46XX_FM_ACF_STATUS 				0x33
#define SI46XX_FM_RDS_STATUS 				0x34
#define SI46XX_FM_RDS_BLOCKCOUNT 			0x35

#define SI46XX_DAB_GET_DIGITAL_SERVICE_LIST 	        0x80
#define SI46XX_DAB_START_DIGITAL_SERVICE 		0x81
#define SI46XX_GET_DIGITAL_SERVICE_DATA			0x84

#define SI46XX_DAB_TUNE_FREQ 				0xB0
#define SI46XX_DAB_DIGRAD_STATUS 			0xB2
#define SI46XX_DAB_GET_EVENT_STATUS			0xB3
#define SI46XX_DAB_GET_ENSEMBLE_INFO                    0xB4
#define SI46XX_DAB_GET_SERVICE_LINKING_INFO 	        0xB7
#define SI46XX_DAB_SET_FREQ_LIST 			0xB8
#define SI46XX_DAB_GET_ENSEMBLE_INFO 			0xB4
#define SI46XX_GET_TIME					0xBC
#define SI46XX_DAB_GET_AUDIO_INFO 			0xBD

#define SI46XX_DAB_GET_ACF_STATUS                       0xC2

#define SI46XX_DAB_GET_SUBCHAN_INFO 			0xBE
#define SI46XX_DAB_GET_RSSI                             0xE5

#define SI46XX_DAB_ACF_CMFTNOISE_BER_LIMIT              0xB507
#define SI46XX_DAB_ACF_CMFTNOISE_LEVEL                  0xB508

void siWrite(uint32_t len);
void siResponseN(int len);
void siResponse(void);
void siCts();
void siGetPartInfo(void);
void siGetFuncInfo(void);
void siGetSysState();
void siPowerup();
void siLoadInit();
void siHostLoad();
void siFlashSetProperty(uint16_t property, uint16_t value);
void siSetProperty(uint16_t property, uint16_t value);
void siGetProperty(uint16_t property);
void siFlashLoad(uint32_t flash_addr);
void siBoot();
void siSetFreqList();
void siDabTuneFreq(uint8_t freq_index);
void siStartDigitalService(uint32_t serviceID, uint32_t compID);
void siStopDigitalService(uint32_t serviceID, uint32_t compID);
void siDabGetEnsembleInfo(void);
void siDabGetSubChannelInfo(uint32_t serviceID, uint32_t compID);
void siDabGetAudioInfo(void);
void siDabDigRadStatus(void);
void siGetDigitalServiceData(void);
void siGetRssi(void);
void siGetTime(void);
void siDabGetAcfStatus(void);
void siDabGetEventStatus(void);
void siGetDigitalServiceList(void);
void siInitDab();
void siReset();

#endif
