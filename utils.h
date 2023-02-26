#ifndef __GOT_UTILS

#define __GOT_UTILS

unsigned long int timeMillis();
void milliSleep(int ms);
void hdSpiResponse(int len);
uint16_t spiBytesTo16(uint8_t *dPtr);
uint32_t spiBytesTo32(uint8_t *dPtr);
void dabshieldPowerup();
void dabshieldReset();
void freqIdToBlock(int id, char *block);
double freqIdToMHz(int id);
double currentFreq(void);
dabFreqType *currentDabFreq(void);

#endif
