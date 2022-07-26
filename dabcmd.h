#ifndef __GOT_DABCMD

#define __GOT_DABCMD

#define DABCMD_NONE              0x0000
#define DABCMD_TUNE              0x1001
#define DABCMD_START_SERVICE     0x1002
#define DABCMD_GETCHANNEL_INFO   0x1003
#define DABCMD_TUNEFREQ          0x1004
#define DABCMD_SAVE              0x1005
#define DABCMD_RESET             0x1006
#define DABCMD_SETSQUELCH        0x1007
#define DABCMD_SETRSSITIME       0x1008
#define DABCMD_SETACQTIME        0x1009
#define DABCMD_SETSYNCTIME       0x100a
#define DABCMD_SETDETECTTIME     0x100b
#define DABCMD_LOGGERMODE        0x100c
#define DABCMD_EXIT              0x1099

#define DABRET_READY             0x0000
#define DABRET_BUSY              0xffff

#endif
