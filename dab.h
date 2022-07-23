#ifndef __GOT_DAB

#define __GOT_DAB

#define DAB_RESET_PIN  18
#define DAB_INT_PIN    4
#define DAB_ENABLE_PIN 17

#define SPI_FLAGS 0x0000c000
#define SPI_SPEED 1000000
#define SPI_CHANNEL 0
#define SPI_BUFF_SIZE 768

#define DAB_DEFAULT_FREQ 0
#define DAB_DEFAULT_SERVICE_ID 0xc224
#define DAB_DEFAULT_COMP_ID 0x20004
#define DAB_LAST_SERVICE "dablast.dat"

#define DAB_TICKTIME 20
#define DAB_RSSI_TICKS 18       
#define DAB_DLS_TICKS 100
#define DAB_TIME_TICKS 30
#define DAB_SHOWTIME_TICKS 1001
#define DAB_SHOWSIG_TICKS 500
#define DAB_SHOWSERV_TICKS 498

boolean dabServiceValid(void);
void dabWaitServiceList(void);
void dabParseServiceList(void);
void dabGetDigRadioStatus();
void dabGetEnsembleInfo(void);
void dabShowEnsemble();
void dabGetRssi();
void dabGetAudioInfo();
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
void dabShowSignal();
void dabShowServiceSummary();
void dabShowState();
void dabTune(DABService *service);
void dabMain();

#endif
