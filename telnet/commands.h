#ifndef __GOT_COMMANDS

#define __GOT_COMMANDS

#define SHOW_STATUS_CONT_TIME 5

int getCpuTemperature(double *);
void cmdScan(char *);
void cmdTime(char *);
void cmdRssi(char *);
void cmdEnsemble(char *);
void cmdTune(char *);
void cmdTuneFreq(char *);
void cmdFreq(char *);
void cmdGetChannelInfo(char *);
void cmdChannelInfo(char *);
void cmdAudioInfo(char *);
void cmdSave(char *);
void cmdVersion(char *);
void cmdShowStatusCont(char *);
void cmdResetRadio(char *);
void cmdShowDls(char *);
void cmdShowInterruptCount(char *);
void cmdSquelch(char *);
void cmdValidAcqTime(char *);
void cmdValidRssiTime(char *);
void cmdValidDetectTime(char *);
void cmdValidSyncTime(char *);
void cmdLogMode(char *);
void cmdGpsInfo(char *);
void cmdListFiles(char *);
void cmdRemoveFile(char *);
void cmdShowTemperature(char *);

#endif
