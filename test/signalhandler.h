#ifndef __OUTPUT_SIGNAL_HANDLER_H__
#define __OUTPUT_SIGNAL_HANDLER_H__

#include <signal.h>


#define ERR_SIGTERM     -1
#define ERR_SIGALARM    -2

typedef void (*sighandler_t)(int);
int SetHandler(int sig, sighandler_t handler, bool restart = true);

void SigTerm(int signo);

void SigChild(int signo);

void SigHup(int signo);

void SigUsr2(int signo);

void SigAlarm(int signo);

void SigPipe(int signo);

int GetOldSigAct(int sig);
int* _RegisterSignal();
    
int _HandleSignal();

#endif // __SIGNAL_HANDLER_H__
