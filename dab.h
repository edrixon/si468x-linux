#ifndef __GOT_DAB

#define __GOT_DAB

#define DAB_ENGINE_VERSION 0x0100

// GPIO pins for DABShield board
#define DAB_INT_PIN     4
#define DAB_ENABLE_PIN  17
#define DAB_RESET_PIN   18

// Cooling fan - enabled in /boot/config.txt
// So, don't use this pin for anything else...
#define DAB_FAN_PIN     27

// Shutdown - enabled in /boot/config.txt
// So, don't use this pin for anything else...
#define DAB_SDWN_PIN    23

// Coverage logger activity LED
#define DAB_ACT_PIN     13

// LEDs
#define DAB_LED1_PIN    5
#define DAB_LED2_PIN    6

// "Powered up" pin - turned on in /boot/config.txt
// Goes off when everything is shutdown
// Don't use this pin for anything else 
// an extra line of code
#define DAB_PUP_PIN     12

// SPI setup
#define SPI_FLAGS       0x0000c000
#define SPI_SPEED       1000000
#define SPI_SI_CHANNEL  0
#define SPI_ADC_CHANNEL 1
#define SPI_BUFF_SIZE  768

// Default configuration
#define DAB_DEFAULT_FREQ       29                 // 225.648MHz
#define DAB_DEFAULT_SERVICE_ID 0xc224             // Radio 4
#define DAB_DEFAULT_COMP_ID    0x20004            // Radio 4
#define DAB_SYSCONFIG          "dabrx.config"

// See Si data sheet - values used as defaults
#define DAB_VALID_RSSI_TIME      100  // ms
#define DAB_VALID_RSSI_THRESHOLD 8    // dBuV
#define DAB_VALID_ACQ_TIME       3000 // ms
#define DAB_VALID_SYNC_TIME      1500 // ms
#define DAB_VALID_DETECT_TIME    50   // ms

// Scheduling times
#define DAB_TICKTIME       20
#define DAB_BZR_TICKS      5
#define DAB_LED_TICKS      3
#define DAB_LOGGER_SCAN_TICKS 60
#define DAB_DLS_TICKS      290
#define DAB_RSSI_TICKS     300     
#define DAB_TIME_TICKS     310
#define DAB_SHOWSIG_TICKS  490
#define DAB_SHOWSERV_TICKS 500
#define DAB_SHOWTIME_TICKS 510
#define DAB_LOGGER_COVER_TICKS 500

// Log file directory
#define LOGDIR "logger"

void dabSetValidRssiTime(int newVal);
void dabSetValidRssiThreshold(int newVal);
void dabSetValidAcqTime(int newVal);
void dabSetValidSyncTime(int newVal);
void dabSetValidDetectTime(int newVal);

boolean dabServiceValid(void);
void dabWaitServiceList(void);
void dabParseServiceList(void);
void dabScheduledGetDigRadioStatus();
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
sysConfigType *dabGetSysConfig(void);
void dabSaveSysConfig(void);
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
