#include "Headdefine.h"
#include "signalhandler.h"
#include <sys/sem.h>
#include "Sem.h"
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

int main(int argc, char** argv)
{
    CSem m_sem;
    int cnt = 10;
    int m_nSemID = m_sem.Create(FIFO_SERVER, cnt, 0);
    if (m_nSemID < 0)
    {
        printf("Sem::Create() failed:%s[%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    } 
    for(int i = 0; i < m_sem.GetSemCnt(); i++)
    {
        printf("---index %d, val--%d\n",i, m_sem.GetSemVal(i));
    }
    
   struct sembuf sops[2];
   
   sops[0].sem_num = 0;
   sops[0].sem_op =  0;
   sops[0].sem_flg = IPC_NOWAIT;
   
   sops[1].sem_num = 1;
   sops[1].sem_op =  0;
   sops[1].sem_flg = 0;


    int nRet = -1;
    nRet =  semop(m_nSemID,sops,2);
    if(nRet < 0)
    {
        printf("semop failed %s\n",strerror(errno));

        for(int i = 0; i < m_sem.GetSemCnt(); i++)
        {
            nRet = m_sem.GetSemVal(i);
            printf("---index %d, val--%d\n",i, nRet);
        }
        return -2;
    }
    for(int i = 0; i < m_sem.GetSemCnt(); i++)
    {
        nRet = m_sem.GetSemVal(i);
        printf("---index %d, val--%d\n",i, nRet);
    }
    
    m_sem.Destroy();
    return 0;
}
