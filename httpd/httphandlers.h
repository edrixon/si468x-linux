#ifndef __GOT_HTTPHANDLERS

#define __GOT_HTTPHANDLERS


int doCommand(dabCmdType *cmd, dabCmdRespType *resp);
void httpGetService();
void httpGetServiceData();
void httpGetFreqs();
void httpGetEnsemble();
void httpGetSystem();
void httpGetCurrent();
void httpSetChannel();
void httpSetService();
void httpGetDabRadio();

void httpGet(char **params);

#endif
