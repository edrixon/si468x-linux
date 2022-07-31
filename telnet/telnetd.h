#ifndef __GOT_TELNETD

#define __GOT_TELNETD

void tputs();
int tgets();
void telnetd();

#define TELNETD_PORT 5000
#define TELNETD_MAXCONNECTIONS 2

#endif

