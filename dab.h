#ifndef __GOT_DAB

#define __GOT_DAB

#define DAB_ENGINE_VERSION 0x0100

#define DAB_RESET_PIN   18
#define DAB_INT_PIN     4
#define DAB_ENABLE_PIN  17

#define SPI_FLAGS       0x0000c000
#define SPI_SPEED       1000000
#define SPI_SI_CHANNEL  0
#define SPI_ADC_CHANNEL 1
#define SPI_BUFF_SIZE  768

#define DAB_DEFAULT_FREQ 29
#define DAB_DEFAULT_SERVICE_ID 0xc224
#define DAB_DEFAULT_COMP_ID 0x20004
#define DAB_LAST_SERVICE "dablast.dat"

#define DAB_VALID_RSSI_TIME      100  // ms
#define DAB_VALID_RSSI_THRESHOLD 8    // dBuV
#define DAB_VALID_ACQ_TIME       3000 // ms
#define DAB_VALID_SYNC_TIME      1500 // ms
#define DAB_VALID_DETECT_TIME    50   // ms

#define DAB_TICKTIME       20
#define DAB_LOGGER_TICKS   125
#define DAB_RSSI_TICKS     300     
#define DAB_DLS_TICKS      301
#define DAB_TIME_TICKS     302
#define DAB_SHOWSERV_TICKS 500
#define DAB_SHOWSIG_TICKS  501
#define DAB_SHOWTIME_TICKS 502

void dabSetValidRssiTime(int newVal);
void dabSetValidRssiThreshold(int newVal);
void dabSetValidAcqTime(int newVal);
void dabSetValidSyncTime(int newVal);
void dabSetValidDetectTime(int newVal);

boolean dabServiceValid(void);
void dabWaitServiceList(void);
void dabParseServiceList(void);
void dabGetDigRadioStatus();
int dabGetEnsembleInfo(void);
void dabGetEnsembleName(char *ensemble);
void dabShowEnsemble();
void dabGetRssi();
void dabGetAudioLevel();
void dabGetChannelInfo(DABService *service);
void dabGetSubChannelInfo(uint32_t serviceID, uint32_t compID,
                                                     channelInfoType *cInfo);
void dabShowSubChannelInfo(channelInfoType *cInfo);
void dabStartDigitalService(uint32_t serviceID, uint32_t compID);
void dabSaveLastService();
void dabGetLastService(DABService *lastService);
void dabInterrupt(int gpio, int level, unsigned int ticks);
void dabBegin();
void dabCommand();
void dabGetTime();
void dabShowTime();
void dabHandleTimers();
void dabGetAcfStatus();
void dabShowSignal();
void dabShowServiceSummary();
void dabShowState();
void dabTune(DABService *service);
void dabTuneFreq(DABService *service);
void dabResetRadio(void);
void dabMain();

#endif
