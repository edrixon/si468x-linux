#ifndef __GOT_COMMANDS

#define __GOT_COMMANDS

#define SHOW_STATUS_CONT_TIME 5

void cmdScan(char *);
void cmdTime(char *);
void cmdRssi(char *);
void cmdEnsemble(char *);
void cmdTune(char *);
void cmdTuneFreq(char *);
void cmdFreq(char *);
void cmdGetChannelInfo(char *);
void cmdAudioInfo(char *);
void cmdSave(char *);
void cmdVersion(char *);
void cmdShowStatusCont(char *);
void cmdResetRadio(char *);
void cmdShowDls(char *);
void cmdShowInterruptCount(char *);

#endif
