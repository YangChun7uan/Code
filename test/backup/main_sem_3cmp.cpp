#include "Headdefine.h"
#include "signalhandler.h"
#include <sys/sem.h>
#include <semaphore.h>
#include "Sem.h"
#include "utility.h"
CSignal g_signal;

#define FIFO_SERVER "./tmp"

long g_1_time = 0;
long g_2_time = 0;
long g_3_time = 0;
long g_1_count = 0;
long g_2_count = 0;
long g_3_count = 0;

int getval(sem_t *sem)
{
    int  sval =  0;
    int nRet = sem_getvalue(sem, &sval);
    if (nRet < 0)
    {
        printf("sem_getvalue failed %s\n", strerror(errno));
    }
    printf("sem_getvalue sval ret%d val%d\n", nRet, sval);
    
    return nRet;
}



int main(int argc, char** argv)
{
    g_signal.RegisterSignals();
    
    
    unsigned val = 32767;

    
    ///////////////
    sem_t  sem2 = 0;
    int nRet = sem_init (&sem2, 0, val);
    if (nRet < 0 || errno != 0)
    {
        printf("sem_init failed %s\n", strerror(errno));
        return -1;
    }
    printf("sem_init %d\n", sem2);
    getval(&sem2);
    
    /////////////
    sem_t* sem1;
    sem1 = sem_open(FIFO_SERVER, O_CREAT|O_EXCL, 0666, val);
    if (sem1 == SEM_FAILED)
    {
        printf("sem_open failed %s\n", strerror(errno));
        
        if(errno == EEXIST)
        {
            int nRet = sem_unlink(FIFO_SERVER);
            if (nRet < 0)
            {
                printf("sem_unlink failed %s\n", strerror(errno));
                return -1;
            }
        }
        sem1 = sem_open(FIFO_SERVER, O_CREAT|O_EXCL, 0666, val);
        if (sem1 == SEM_FAILED)
        {
            printf("sem_open again failed %s\n", strerror(errno));
            return -1;
        }
    }
    printf("sem_open %d\n", *sem1);
    getval(sem1);
    
    //////////////////
    CSem m_sem;
    int m_nSemID = m_sem.Create(FIFO_SERVER, 1, val);
    if (m_nSemID < 0)
    {
        printf("Sem::Create() failed:%s[%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    } 
    for(int i = 0; i < m_sem.GetSemCnt(); i++)
    {
        printf("---index %d, val--%d\n",i, m_sem.GetSemVal(i));
    }
    

    struct timeval time_a,time_b;
    long long i = 0;
    while(i < 32767)
    {
        /////////////////////////////////
        gettimeofday(&time_a,NULL);
        nRet = sem_post(sem1);
        nRet = sem_wait(sem1);
        gettimeofday(&time_b,NULL);
        if (nRet < 0)
        {
            printf("sem_post failed %s\n", strerror(errno));
            break;
        }
        
        long use_time1 = (time_b.tv_sec-time_a.tv_sec)*1000000 + (time_b.tv_usec - time_a.tv_usec);
        if(use_time1 < 10)
        {
            g_1_time += use_time1 ;
            g_1_count++;
        }
        
        i++;
    }
    
    i = 0;
    while(i < 32767)
    {
        /////////////////////////////////
        gettimeofday(&time_a,NULL);
        nRet = sem_post(&sem2);
        nRet = sem_wait(&sem2);
        gettimeofday(&time_b,NULL);
        if (nRet < 0)
        {
            printf("sem_post failed %s\n", strerror(errno));
            break;
        }
        

        long use_time2 = (time_b.tv_sec-time_a.tv_sec)*1000000 + (time_b.tv_usec - time_a.tv_usec);
        if(use_time2 < 10)
        {
            g_2_time += use_time2 ;
            g_2_count++;
        }

        
        i++;
    }
    
    struct sembuf sops1 = {0, 1, IPC_NOWAIT };
    struct sembuf sops2 = {0, -1, 0 };
    i = 0;
    while(i < 32767)
    {
        //////////////////////////////////
        gettimeofday(&time_a,NULL);
        nRet = semop(m_nSemID, &sops1, 1);
        nRet = semop(m_nSemID, &sops2, 1);
        gettimeofday(&time_b,NULL);
        if (nRet < 0)
        {
            printf("semop failed %s\n", strerror(errno));
            break;
        }
        
        long use_time3 = (time_b.tv_sec-time_a.tv_sec)*1000000 + (time_b.tv_usec - time_a.tv_usec);
        if(use_time3 < 10)
        {
            g_3_time += use_time3 ;
            g_3_count++;
        }
        
        i++;
    }
    
    
    double avg1 = (double)g_1_time/(double)g_1_count;
    printf("sem_post analyse:%ld ,avgage time %lf us\n", g_1_count, avg1);

    double avg2 = (double)g_2_time/(double)g_2_count;
    printf("sem_post analyse:%ld ,avgage time %lf us\n", g_2_count, avg2);
    
    double avg3 = (double)g_3_time/(double)g_3_count;
    printf("sem_post analyse:%ld ,avgage time %lf us\n", g_3_count, avg3);
    
    nRet = sem_unlink(FIFO_SERVER);
    if (nRet < 0)
    {
        printf("sem_unlink failed %s\n", strerror(errno));
    }
    
    nRet = sem_destroy (&sem2);
    if (nRet < 0)
    {
        printf("sem_destroy failed %s\n", strerror(errno));
    }
    
    nRet = m_sem.Destroy();
    if (nRet < 0)
    {
        printf("m_sem.Destroy failed %s\n", strerror(errno));
    }
    return 0;
}

