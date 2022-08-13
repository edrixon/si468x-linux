#ifndef __GOT_LOGGER

#define __GOT_LOGGER

#define LOGGER_INIT    0
#define LOGGER_TUNE    1
#define LOGGER_MEASURE 2

void dabControlLogger(void);
void dabLogger(void);
void dabLoggerTune(void);
void dabLoggerMeasure(void);
void dabInitLogger(void);
void dabStartLogger(void);
void dabStopLogger(void);
int dabLoggerRunning(void);


#endif
