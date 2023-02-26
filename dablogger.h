#ifndef __GOT_LOGGER

#define __GOT_LOGGER

#define LOG_RESTART_TICKS 1500

int dabGetLoggerMode(void);
void dabControlLogger(void);
void dabLoggerMain(void);
void dabLoggerTune(void);
void dabLoggerMeasure(void);
void dabInitLogger(void);
void dabStartLogger(void);
void dabStopLogger(void);
int dabLoggerRunning(void);
int dabLoggerSetRunMode(int);
void dabLoggerCoverage(void);


#endif
