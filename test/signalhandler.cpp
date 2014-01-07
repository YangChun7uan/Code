#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Headdefine.h"
#include "signalhandler.h"


int g_nSigTerm;  //SIGTERM信号标记
int g_nSigInt;  //SIGPIPE信号标记
int g_nSigChild; //SIGCHILD信号标记
int g_nSigHup;   //SIGHUP信号标记
int g_nSigUsr2;  //SIGUSR2信号标记
int g_nSigAlarm; //SIGALARM信号标记
int g_nSigPipe;  //SIGPIPE信号标记
int g_nSigFlag;


void SigTerm(int signo)
{
    printf("SigTerm\n");
    g_nSigTerm = 1;
    g_nSigFlag = 1;
}

void SigChild(int signo)
{
    printf("SigChild\n");
    g_nSigChild = 1;
    g_nSigFlag = 1;
}

void SigHup(int signo)
{
    printf("SigHup\n");
    g_nSigHup = 1;
    g_nSigFlag = 1;
}

void SigUsr2(int signo)
{
    printf("SigUsr2\n");
    g_nSigUsr2 = 1;
    g_nSigFlag = 1;
}

void SigAlarm(int signo)
{
    printf("SigAlarm\n");
    g_nSigAlarm = 1;
    g_nSigFlag = 1;
}

void SigPipe(int signo)
{
    printf("SigPipe\n");
    g_nSigPipe = 1;
    g_nSigFlag = 1;
}


void SigInt(int signo)
{
    printf("SigInt\n");
    g_nSigInt = 1;
    g_nSigFlag = 1;
}

int _HandleSignal()
{
	int nRet = 0;
	
	if(g_nSigUsr2)
	{
		g_nSigUsr2 = 0;
		printf("Handle SIGUSER2\n");
		nRet = nRet < 0 ? nRet: 0;
	}
	
	if(g_nSigHup)
	{
		g_nSigHup = 0;
		printf("Handle SIGHUP\n");
		nRet = nRet < 0 ? nRet: 0;
	}
	
	if(g_nSigChild)
	{
		g_nSigChild = 0;
		printf("Handle SIGCHILD\n");
		nRet = nRet < 0 ? nRet: 0;
	}

    if(g_nSigAlarm)
    {
        g_nSigAlarm = 0;
        printf("Handle SIGALARM\n");
        nRet = ERR_SIGALARM;
    }
    
	if(g_nSigTerm)
	{
		g_nSigTerm = 0;
		printf("Handle SIGTERM\n");
		nRet = ERR_SIGTERM;
	}
	g_nSigFlag = 0;
	
	return nRet;
}

int* _RegisterSignal()
{
    if (SetHandler(SIGINT, SigInt, false) != 0)
    {
        perror("Unable to set signal handler for SIGINT");
        return NULL;
    }
    
    if (SetHandler(SIGTERM, SigTerm, false) != 0)
    {
        perror("Unable to set signal handler for SIGTERM");
        return NULL;
    }
    
    if (SetHandler(SIGHUP, SigHup, false) != 0)
    {
        perror("Unable to set signal handler for SIGHUP");
        return NULL;
    }
    
    if (SetHandler(SIGUSR2, SigUsr2, false) != 0)
    {
        perror("Unable to set signal handler for SIGUSR2");
        return NULL;
    }    

    if (SetHandler(SIGALRM, SigAlarm, false) != 0)
    {
        perror("Unable to set signal handler for SIGALARM");
        return NULL;
    }

    if (SetHandler(SIGPIPE, SIG_IGN, false) != 0)
    {
        perror("Unable to set signal handler for SIGPIPE");
        return NULL;
    }

    if (SetHandler(SIGUSR1, SIG_IGN, false) != 0)
    {
       perror("Unable to set signal handler for SIGUSR1");
       return NULL;
    }

    return &g_nSigFlag;
}

int SetHandler(int sig, sighandler_t handler, bool restart)
{
    struct sigaction new_action;
    struct sigaction old_action;

    memset(&new_action, 0, sizeof(struct sigaction));

    new_action.sa_handler = handler;

    /* Temporarily block all other signals during handler execution
     */
    sigfillset (&new_action.sa_mask);

    new_action.sa_flags = 0;
    /* Auto restart syscalls after handler return
     */
    //if(sig == SIGALRM)printf("init new_action.sa_flags %d\n", new_action.sa_flags);
    if(restart)
        new_action.sa_flags |= SA_RESTART;

    //if(sig == SIGALRM && restart)printf("new_action.sa_flags |= SA_RESTART\n");
    //if(sig == SIGALRM)printf("set new_action.sa_flags %d\n", new_action.sa_flags);
    if (sigaction(sig, NULL, &old_action) != 0)
    {
        return -1;
    }
    //if(sig == SIGALRM)printf("get old_action.sa_flags %d\n", old_action.sa_flags);

    /* Honor inherited SIG_IGN that's set by some shell's
     */
    if (old_action.sa_handler != SIG_IGN)
    {
        //if(sig == SIGALRM)printf("set new_action\n");
        if (sigaction(sig, &new_action, NULL) != 0)
        {
            return -1;
        }
    }

    return 0;
}

int GetOldSigAct(int sig)
{
    struct sigaction old_action;

    memset(&old_action, 0, sizeof(struct sigaction));

    if (sigaction(sig, NULL, &old_action) != 0)
    {
        return -1;
    }

    printf("get old_action.sa_flags %d\n", old_action.sa_flags);

    return 0;
}
