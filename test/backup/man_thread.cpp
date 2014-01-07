#include "Headdefine.h"
#include "signalhandler.h"
#include <pthread.h>
#include "utility.h"
CSignal g_signal;

#define FIFO_SERVER "./tmp"

extern long msgrcv_time = 0;
extern long msgsnd_time = 0;
extern long read_time = 0;
extern long write_time = 0;
extern int msgsnd_count = 0;
extern int msgrcv_count = 0;
extern int read_count = 0;
extern int write_count = 0;


class Thread
{
public:
    Thread();
    ~Thread();
    int ThreadStart();
    //void* ThreadRun(void *arg);
    int ThreadClean();
    
    pthread_t m_threadid[2];
};




void* ThreadRun(void *arg)
{
    printf("[%d] thread start\n", pthread_self());
    
    int error = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if(error != 0)
        printf("pthread_setcanceltype failed %s\n",strerror(errno));
    
    while(true)
    {
        printf("[%d] thread sleep\n", pthread_self());
        sleep(2);
        printf("[%d] thread g_nSigTerm %d\n", pthread_self(),g_nSigTerm);
        //pthread_testcancel();
        if(g_signal.HandleSignals() < 0)
            break;
    }
    printf("[%d] thread exit\n", pthread_self());
    return ((void *)0);
}

void* ThreadRun2(void *arg)
{
    printf("[%d] thread start\n", pthread_self());
    
    int error = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if(error != 0)
        printf("pthread_setcanceltype failed %s\n",strerror(errno));
    
    while(true)
    {
        printf("[%d] thread sleep\n", pthread_self());
        sleep(3);
        //pthread_testcancel();
        printf("[%d] thread g_nSigTerm %d\n", pthread_self(),g_nSigTerm);
        if(g_signal.HandleSignals() < 0)
            break;
    }
    printf("[%d] thread exit\n", pthread_self());
    return ((void *)0);
}

Thread::Thread()
{
    for(int i = 0; i < 2; ++i)
        m_threadid[i] = 0;
}

Thread::~Thread()
{
}

int Thread::ThreadStart()
{
        int error = pthread_create(&m_threadid[0], NULL, ThreadRun, NULL);
        if(error != 0)
        {
            printf("main pthread_create failed %s\n",strerror(errno));
            ThreadClean();
            return -1;
        }
        
         error = pthread_create(&m_threadid[1], NULL, ThreadRun2, NULL);
        if(error != 0)
        {
            printf("main pthread_create failed %s\n",strerror(errno));
            ThreadClean();
            return -1;
        }
}


int Thread::ThreadClean()
{
    for(int i = 0; i < 2; ++i)
    {
        if(m_threadid[i] != 0)
        {
            printf("main pthread_cancel tid %d\n", m_threadid[i]);
            //int error = pthread_cancel(m_threadid[i]);
            int error = pthread_kill(m_threadid[i],SIGTERM);
            if(error != 0)
                printf("main pthread_cancel failed %s\n",strerror(errno));
            
            printf("main g_nSigTerm %d \n",g_nSigTerm);
        }
    }
}

int main(int argc, char** argv)
{
    g_signal.RegisterSignals();
    
    Thread t;
    
    printf("main ThreadStart()\n");
    t.ThreadStart();
    
    printf("main sleep 1000\n");
    sleep(1000);
    
    printf("main ThreadClean()\n");
    sleep(2);
    t.ThreadClean();
    
    printf("main sleep 10\n");
    sleep(10);
    printf("main g_nSigTerm %d \n",g_nSigTerm);
    
    return 0;
    
}
