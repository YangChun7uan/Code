
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/signal.h>
#include "Semaphore.h"


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
     int val;                  /* value for SETVAL */
     struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
     unsigned short *array;    /* array for GETALL, SETALL */
                               /* Linux specific part: */
     struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

CSemaphore::CSemaphore()
{
//	memset ( &m_sembuf, 0, sizeof(m_sembuf) );
	m_nSemID = 0;
	m_nKey = 0;
	m_bAttach = false;
	m_nSemCnt = 0;
	m_nBCounterNum = 0;
	m_nSCounterNum = 0;
	m_nMaxVal = 0;
	memset(m_nGoNextSCounter, 0, sizeof(m_nGoNextSCounter));
	memset(m_nSCounterSeq, 0, sizeof(m_nSCounterSeq));
	memset(m_errmsg, 0, sizeof(m_errmsg));
}

CSemaphore::CSemaphore(int nKey, bool bAttach)
{
    m_nSemID = 0;
    m_nKey = 0;
    m_bAttach = false;
    m_nSemCnt = 0;
    m_nBCounterNum = 0;
    m_nSCounterNum = 0;
    m_nMaxVal = 0;
    memset(m_nGoNextSCounter, 0, sizeof(m_nGoNextSCounter));
    memset(m_nSCounterSeq, 0, sizeof(m_nSCounterSeq));
    memset(m_errmsg, 0, sizeof(m_errmsg));

	if ( bAttach )
		Attach ( nKey );
	else
		Create ( nKey );
}


CSemaphore::~CSemaphore()
{
	if ( ( m_nSemID > 0 ) && !m_bAttach )
		Release ( );
}


int CSemaphore::Create(const char* pszFile, int nCnt, int initVal, int nProj)
{
    int nKey = ftok ( pszFile, nProj );

    if ( nKey == -1 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATE_FTOK, strerror(errno));
        return CACHE_SEM_ERR_CREATE_FTOK;
    }
    
    return Create(nKey, nCnt, initVal);
}

int CSemaphore::Create(int nKey, int nCnt, int initVal)
{
    assert ( nKey != -1 );
    int nSemID, nRet;
    union semun sem ;   

    if ( nCnt < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_CREATE_PARA);
        return CACHE_SEM_ERR_CREATE_PARA; 
    }
    nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATE_SEMGET, strerror(errno));
        return CACHE_SEM_ERR_CREATE_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;

    //初始化信号量
    sem.val = initVal; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < nCnt; i++ )
        sem.array[i] = initVal;
    
    nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;

    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATE_SEMCTL, strerror(errno));
        return CACHE_SEM_ERR_CREATE_SEMCTL;
    }
    ///初始化后立即调用，主要是要调用semop函数操作改变arg.buf->sem_otime
    if(initVal == SEM_VALUE_MAX)
    {
        nRet = Decrease(0, true, true);
        if(nRet < 0)
            return nRet;
        
        nRet = Increase(0, true, true);
        if(nRet < 0)
            return nRet;
    }
    else
    {
        nRet = Increase(0, true, true);
        if(nRet < 0)
            return nRet;
        
        nRet = Decrease(0, true, true);
        if(nRet < 0)
            return nRet;
    }

    return m_nSemID;
}

int CSemaphore::Test0(int nSemIdx, bool bNoWait)
{
    short nFlag = 0;
    
    if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_TEST0_PARA);
        return CACHE_SEM_ERR_TEST0_PARA;
    }
    
    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, 0, nFlag};
      
    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno == EAGAIN)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_TEST0_NOT0, strerror(errno));
        return CACHE_SEM_ERR_TEST0_NOT0;
    }
    else if(nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_TEST0_SEMOP, strerror(errno));
        return CACHE_SEM_ERR_TEST0_SEMOP;
    }
    
    return nRet;
}

int CSemaphore::Lock(int nSemIdx, bool bNoWait)
{
	short nFlag = 0;

	if (m_nSemID == 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_LOCK_NOINIT);
	    return CACHE_SEM_ERR_LOCK_NOINIT;
	}
	if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_LOCK_PARA);
		return CACHE_SEM_ERR_LOCK_PARA;
	}
	
	nFlag = nFlag | SEM_UNDO;
	
	if ( bNoWait )
	    nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, -1, nFlag};
	  
	int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_LOCK_SEMOP, strerror(errno));
        return CACHE_SEM_ERR_LOCK_SEMOP;
    }
    
    return nRet;
}

int CSemaphore::Unlock(int nSemIdx, bool bNoWait)
{
	short nFlag = 0;

	if (m_nSemID == 0 )
    {
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_UNLOCK_NOINIT);
        return CACHE_SEM_ERR_UNLOCK_NOINIT;
    }
	if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_UNLOCK_PARA);
        return CACHE_SEM_ERR_UNLOCK_PARA;
    }
	
	nFlag = nFlag | SEM_UNDO;
	
    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, 1, nFlag };

    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_UNLOCK_SEMOP, strerror(errno));
        return CACHE_SEM_ERR_UNLOCK_SEMOP;
    }
    
    return nRet;
}

int CSemaphore::Inc(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    if(m_nBCounterNum > 0)
    {
        return CounterInc(nSemIdx);
    }
    else
    {
        return Increase(nSemIdx, bNoWait, bUndo, val);
    }
}

int CSemaphore::Increase(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_INCREASE_NOINIT);
        return CACHE_SEM_ERR_INCREASE_NOINIT;
    }
    
    if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_INCREASE_PARA);
        return CACHE_SEM_ERR_INCREASE_PARA;
    }

    short nFlag = 0;
    
    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;
    if ( bUndo )
        nFlag = nFlag | SEM_UNDO;

    struct sembuf sops = {nSemIdx, val, nFlag};
      
    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_INCREASE_SEMOP, strerror(errno));
        return CACHE_SEM_ERR_INCREASE_SEMOP;
    }
    return nRet;
}

int CSemaphore::Dec(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    
    if(m_nBCounterNum > 0)
    {
        return CounterDec(nSemIdx);
    }
    else
    {
        return Decrease(nSemIdx, bNoWait, bUndo, val);
    }
}

int CSemaphore::Decrease(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_DECREASE_NOINIT);
        return CACHE_SEM_ERR_DECREASE_NOINIT;
    }
    
    if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_DECREASE_PARA);
        return CACHE_SEM_ERR_DECREASE_PARA;
    }

    short nFlag = 0;

    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;
    if ( bUndo )
        nFlag = nFlag | SEM_UNDO;

    struct sembuf sops = {nSemIdx, 0-val, nFlag };

    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_DECREASE_SEMOP, strerror(errno));
        return CACHE_SEM_ERR_DECREASE_SEMOP;
    }

    return nRet;
}

int CSemaphore::Wait(int nSemIdx, bool bNoWait, bool bReverse)
{
    int nRet = 0;

    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_WAIT_NOINIT);
        return CACHE_SEM_ERR_WAIT_NOINIT;
    }
    if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_WAIT_PARA);
        return CACHE_SEM_ERR_WAIT_PARA;
    }
    
    if(bReverse)
    {
        if ( (nRet = Unlock ( nSemIdx, bNoWait )) == 0 )
            nRet = Lock ( nSemIdx, bNoWait );
    }
    else
    {
        if ( (nRet = Lock ( nSemIdx, bNoWait )) == 0 )
            nRet = Unlock ( nSemIdx, bNoWait );
    }

    return nRet;
}

int CSemaphore::Release()
{
    m_nSemID = 0;
	m_nSemCnt = 0;
	return 0;
}

int CSemaphore::Destroy()
{
    int nRet = 0;

    
    if(m_nSemID > 0)
    {
        nRet = semctl ( m_nSemID, m_nSemCnt, IPC_RMID, 0 );
        if(nRet < 0)
        {
            m_nSemID = 0;
            m_nSemCnt = 0;
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_DESTROY_SEMCTL, strerror(errno));
            return CACHE_SEM_ERR_DESTROY_SEMCTL;
        }
        
    }

    m_nSemID = 0;
    m_nSemCnt = 0;
    
    return 0;
}

int CSemaphore::Attach ( const char* pszFile, int nProj)
{
	int nKey = ftok ( pszFile, nProj );
	if ( nKey == -1 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_ATTACH_FTOK, strerror(errno));
		return CACHE_SEM_ERR_ATTACH_FTOK;
	}
	return Attach ( nKey);
}

int CSemaphore::Attach(int nKey)
{
    int nFlag = 0;
    m_nSemID = semget ( nKey, 0, nFlag );
    if (m_nSemID < 0)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_ATTACH_SEMGET, strerror(errno));
		return CACHE_SEM_ERR_ATTACH_SEMGET;
    }

    struct semid_ds sem_info;
    memset ( &sem_info, 0, sizeof(sem_info) );
    union semun arg;
    arg.buf = &sem_info;

    bool bInit = false;
    int nMaxTime = 3;
    for(int i=0; i<nMaxTime; i++ )
    {
        if ( semctl (m_nSemID, 0, IPC_STAT, arg) == -1 )
        {   
            perror("semctl error"); 
            break;
        }
        
        m_nSemCnt = arg.buf->sem_nsems; 
        if ( arg.buf->sem_otime != 0 )
        { 
            bInit = true;
            break;
        }
        else
        {
            sleep (1);  
        }
    }

    if ( !bInit )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_ATTACH_NOINIT);
	    return CACHE_SEM_ERR_ATTACH_NOINIT;
    }

    m_nKey = nKey;
    m_bAttach = true;

    return m_nSemID;
}

int CSemaphore::GetSemVal(int nSemIdx)
{
	if (m_nSemID == 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_GETSEMVAL_NOINIT);
	    return CACHE_SEM_ERR_GETSEMVAL_NOINIT;
	}

	int nRet = semctl ( m_nSemID, nSemIdx, GETVAL, 0 );
	if( nRet<0)
	{
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_GETSEMVAL_SEMCTL, strerror(errno));
	    return CACHE_SEM_ERR_GETSEMVAL_SEMCTL;
	}
	
	return nRet;
}

int CSemaphore::SetSemVal(int nSemIdx, ushort val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_SETSEMVAL_NOINIT);
        return CACHE_SEM_ERR_SETSEMVAL_NOINIT;
    }

    union semun sem ;
    sem.val = val;

    if( semctl ( m_nSemID, nSemIdx, SETVAL, sem ) < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_SETSEMVAL_SEMCTL, strerror(errno));
        return CACHE_SEM_ERR_SETSEMVAL_SEMCTL;
    }
    
    return 0;
}

int CSemaphore::GetAllSemVal(ushort* value, int nCnt)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_GETALLSEMVAL_NOINIT);
        return CACHE_SEM_ERR_GETALLSEMVAL_NOINIT;
    }
    
    union semun sem ;
    sem.array = value;
    
    int nRet = semctl (m_nSemID, nCnt, GETALL, sem);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if (nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_GETALLSEMVAL_SEMCTL, strerror(errno));
        return CACHE_SEM_ERR_GETALLSEMVAL_SEMCTL;
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////

int CSemaphore::CreateCounter(const char* pszFile, int nCounterNum, int nProj)
{
    int nKey = ftok ( pszFile, nProj );

    if ( nKey == -1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATECOUNTER_FTOK, strerror(errno));
        return CACHE_SEM_ERR_CREATECOUNTER_FTOK;
    }
    return CreateCounter(nKey, nCounterNum);
}

int CSemaphore::AttachCounter ( const char* pszFile, int nProj)
{
    int nKey = ftok ( pszFile, nProj );
    if ( nKey == -1 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_ATTACHCOUNTER_FTOK, strerror(errno));
        return CACHE_SEM_ERR_ATTACHCOUNTER_FTOK;
    }

    return AttachCounter ( nKey);
}

int CSemaphore::GetBigCounterNum()
{
    return GetSemVal(m_nSemCnt - 1);
}

#ifdef MOTHOD_SW_COUNTER

//   [ Sem1 Sem2 ]  = SmallCounter
//   两个信号量为1个SmallCounter，组成1个小计数器，第一个信号量用作开关，第二个用于计数
//
//   [Group1]...[GroupN] = BigCounter  
//   多个SmallCounter组成1个大计数器
//
//   [BigCounter1] ... [BigCounterN] = Semaphores
//   信号量集根据大计数器的个数，为每个大计数器平均分配SmallCounter数量
//
//   每个BigCounter的最大计数值 = SmallCounter数量 * SEM_VALUE_MAX
int CSemaphore::CreateCounter(int nKey, int nCounterNum)
{
    if ( nCounterNum > MAX_BIGCOUNTER_NUM || nCounterNum < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_CREATECOUNTER_PARA);
        return CACHE_SEM_ERR_CREATECOUNTER_PARA;
    }
    int nCnt = 65535;
    int nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATECOUNTER_SEMGET, strerror(errno));
        return CACHE_SEM_ERR_CREATECOUNTER_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;
    m_nBCounterNum = nCounterNum;
    int res = 0;
    if(m_nBCounterNum % 2 == 0)
    {
        res = 0;//剩下的是单数
    }
    else
        res = 1;//剩下的是双数
    
    m_nSCounterNum  = ((m_nSemCnt - res - m_nBCounterNum - 1) / m_nBCounterNum) / 2 ;
    m_nMaxVal = m_nSCounterNum * SEM_VALUE_MAX;

    //初始化信号量
    union semun sem;
    sem.val = 0; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < m_nSCounterNum * 2 * m_nBCounterNum; i++ )
    {
        if(i % (m_nSCounterNum * 2) == 0)                  //第一个开关打开
        {
            sem.array[i] = 0;
            continue;
        }
        if(i % 2 == 0)                  //其余开关关闭
        {
            sem.array[i] = 1;
        }
        else                            //计数器清0
            sem.array[i] = 0;
    }
    
    for ( int i = m_nSCounterNum * 2 * m_nBCounterNum; i < m_nSCounterNum * 2 * m_nBCounterNum + m_nBCounterNum; i++ )//SmallCounter记录清0
    {
        sem.array[i] = 0;
    }
    
    for ( int i = m_nSCounterNum * 2 * m_nBCounterNum + m_nBCounterNum; i < m_nSemCnt - 1; i++ )//用于锁
    {
        sem.array[i] = 0;
    }
    
    sem.array[m_nSemCnt - 1] = m_nBCounterNum; //最后一个用于记录BigCounter个数
    
    int nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;
    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATECOUNTER_SEMCTL, strerror(errno));
        return CACHE_SEM_ERR_CREATECOUNTER_SEMCTL;
    }
    
    nRet = Increase(0, true, true);
    if(nRet < 0)
        return nRet;
    
    nRet = Decrease(0, true, true);
    if(nRet < 0)
        return nRet;
    
    return m_nSemID;
}

int CSemaphore::AttachCounter(int nKey)
{
    m_nSemID = semget ( nKey, 0, 0 );
    if (m_nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET, strerror(errno));
        return CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET;
    }

    struct semid_ds sem_info;
    memset ( &sem_info, 0, sizeof(sem_info) );
    union semun arg;
    arg.buf = &sem_info;

    bool bInit = false;
    int nMaxTime = 3;
    for( int i=0; i<nMaxTime; i++ )
    {
        if ( semctl (m_nSemID, 0, IPC_STAT, arg) == -1 )
        {   
            perror("semctl error"); 
            break;
        }
        
        m_nSemCnt = arg.buf->sem_nsems; 
        if ( arg.buf->sem_otime != 0 )
        { 
            bInit = true;
            break;
        }
        else
        {
            sleep (1);  
        }
    }
    if ( !bInit )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT);
        return CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT;
    }
    m_nKey = nKey;
    m_bAttach = true;
    
    m_nBCounterNum = GetBigCounterNum();
    int res = 0;
    if(m_nBCounterNum % 2 == 0)
    {
        res = 0;//剩下的是单数
    }
    else
        res = 1;//剩下的是双数
    
    m_nSCounterNum  = ((m_nSemCnt - res - m_nBCounterNum - 1) / m_nBCounterNum) / 2 ;
    m_nMaxVal = m_nSCounterNum * SEM_VALUE_MAX;
    
    for(int i = 0; i < m_nBCounterNum; ++i)
    {
        m_nSCounterSeq[i] = GetSemVal( GetCounterRecSemSeq(i) );
    }
    
    return m_nSemID;
}

int CSemaphore::GetCounterSWSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * nBCounterSeq + 2 * m_nSCounterSeq[nBCounterSeq] ;
}

int CSemaphore::GetCounterSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * nBCounterSeq + 2 * m_nSCounterSeq[nBCounterSeq] + 1  ;
}

int CSemaphore::GetCounterRecSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * m_nBCounterNum + nBCounterSeq ;
}

int CSemaphore::CounterInc(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERINC_NOINIT);
        return CACHE_SEM_ERR_COUNTERINC_NOINIT;
    }

    int nRet = -1;
    for(int i = 0; i < m_nSCounterNum; ++i)
    {
        struct sembuf sops[2];
        sops[0].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;
        sops[0].sem_op = 0;                                     //非0立即返回EAGAIN
        sops[0].sem_flg = IPC_NOWAIT;
        sops[1].sem_num = GetCounterSemSeq(nBCounterSeq);
        sops[1].sem_op = 1;                                     //阻塞+1，超过最大值返回ERANGE
        sops[1].sem_flg = 0;
        
        nRet =  semop(m_nSemID, sops, 2);
        if(nRet < 0 && errno == EINTR)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
            return SEM_EINTR;
        }
        
        else if(nRet < 0 && errno == EAGAIN) //开关非0，已经转到下一个SmallCounter
        {
            if(GetSemVal( GetCounterSWSemSeq(nBCounterSeq) ) != 0)//开关非0
            {
                ++ m_nSCounterSeq[nBCounterSeq];
                if(m_nSCounterSeq[nBCounterSeq] >= m_nSCounterNum)
                {
                    m_nSCounterSeq[nBCounterSeq] = 0;
                }
                continue;
            }
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERINC_VAL);
                return CACHE_SEM_ERR_COUNTERINC_VAL;
            }
        }
        
        else if(nRet < 0 && errno == ERANGE) //超过最大值，转到下一个index继续
        {
            int save = m_nSCounterSeq[nBCounterSeq];
            
            ++ m_nSCounterSeq[nBCounterSeq];
            if(m_nSCounterSeq[nBCounterSeq] >= m_nSCounterNum)
            {
                m_nSCounterSeq[nBCounterSeq] = 0;
            }
            
            if( GetSemVal( GetCounterSemSeq(nBCounterSeq) ) == 0 )             //下一个SmallCounter计数值为0
                SetSemVal( GetCounterSWSemSeq(nBCounterSeq), 0);               //先开启下一个计数器
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, SEM_COUNTER_FULL);
                return SEM_COUNTER_FULL;
            }

            SetSemVal(GetCounterRecSemSeq(nBCounterSeq), m_nSCounterSeq[nBCounterSeq]);            //设置成当正在使用的SmallCounter的index
            
            SetSemVal( 2 * m_nSCounterNum * nBCounterSeq + 2 * save, 1);                            //再关闭这一个计数器

            continue;
        }
        
        else if(nRet < 0)
        {
            
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERINC_SEMOP, strerror(errno));
            return CACHE_SEM_ERR_COUNTERINC_SEMOP;
        }
        else
            return 0;
    }

    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERINC);
    return CACHE_SEM_ERR_COUNTERINC;
}

int CSemaphore::CounterDec(int nBCounterSeq )
{    
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERDEC_NOINIT);
        return CACHE_SEM_ERR_COUNTERDEC_NOINIT;
    }
    
    int nRet = -1;
    for(int i = 0; i < m_nSCounterNum; ++i)
    {
        struct sembuf sops[2];
        memset(sops, 0, sizeof(sops));
        int size = 0;
        
        if(m_nGoNextSCounter[nBCounterSeq] == 0)
        {
            sops[0].sem_num = GetCounterSWSemSeq(nBCounterSeq);
            sops[0].sem_op = 0;                                     
            sops[0].sem_flg = IPC_NOWAIT;
            sops[1].sem_num = GetCounterSemSeq(nBCounterSeq);
            sops[1].sem_op = -1;                                     
            sops[1].sem_flg = 0;
            size = 2;
        }
        else
        {
            sops[0].sem_num = GetCounterSemSeq(nBCounterSeq);
            sops[0].sem_op = -1;                                    
            sops[0].sem_flg = IPC_NOWAIT;
            size = 1;
        }
        
        nRet =  semop(m_nSemID, sops, size);
        
        if(nRet < 0 && errno == EINTR)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
            return SEM_EINTR;
        }
        
        else if(nRet < 0 && errno == EAGAIN)
        {
            bool flag = false;
            if( GetSemVal( GetCounterSWSemSeq(nBCounterSeq) ) != 0)         
            {
                m_nGoNextSCounter[nBCounterSeq] = 1;  
                flag = true;
            }
            
            if( GetSemVal( GetCounterSemSeq(nBCounterSeq) ) == 0 )        
            {
                m_nGoNextSCounter[nBCounterSeq] = 0;
                ++ m_nSCounterSeq[nBCounterSeq];
                if(m_nSCounterSeq[nBCounterSeq] >= m_nSCounterNum)
                {
                    m_nSCounterSeq[nBCounterSeq] = 0;
                }
                flag = true;
            }
            
            if(!flag)
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERDEC_VAL);
                return CACHE_SEM_ERR_COUNTERDEC_VAL;
            }
            
            continue;
        }
        
        else if(nRet < 0)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERDEC_SEMOP, strerror(errno));
            return CACHE_SEM_ERR_COUNTERDEC_SEMOP;
        }
        else
        {
            return 0;
        }
    }
    
    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERDEC);
    return CACHE_SEM_ERR_COUNTERDEC;
}

int GetCounterVal (int nBCounterSeq )
{
    return 0;
}

#else

int CSemaphore::GetCounterSWSemSeq(int nBCounterSeq)
{
    return nBCounterSeq * 2 + 1 ;
}

int CSemaphore::GetCounterSemSeq(int nBCounterSeq)
{
    return nBCounterSeq * 2 ;
}

int CSemaphore::CreateCounter(int nKey, int nCounterNum)
{
    if ( nCounterNum > MAX_BIGCOUNTER_NUM || nCounterNum < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_CREATECOUNTER_PARA);
        return CACHE_SEM_ERR_CREATECOUNTER_PARA;
    }
    int nCnt = nCounterNum * 2 + 1;
    int nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATECOUNTER_SEMGET, strerror(errno));
        return CACHE_SEM_ERR_CREATECOUNTER_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;
    m_nBCounterNum = nCounterNum;
    m_nMaxVal = SEM_VALUE_MAX * SEM_VALUE_MAX;

    //初始化信号量
    union semun sem;
    sem.val = 0; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < nCnt; i++ )
    {
        sem.array[i] = 0;
    }
    sem.array[m_nSemCnt - 1] = m_nBCounterNum; //最后一个用于记录BigCounter个数
    
    int nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;
    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_CREATECOUNTER_SEMCTL, strerror(errno));
        return CACHE_SEM_ERR_CREATECOUNTER_SEMCTL;
    }
    
    nRet = Increase(0, true, true);
    if(nRet < 0)
        return nRet;
    
    nRet = Decrease(0, true, true);
    if(nRet < 0)
        return nRet;
    
    return m_nSemID;
}

int CSemaphore::AttachCounter(int nKey)
{
    m_nSemID = semget ( nKey, 0, 0 );
    if (m_nSemID == -1)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET, strerror(errno));
        return CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET;
    }

    struct semid_ds sem_info;
    memset ( &sem_info, 0, sizeof(sem_info) );
    union semun arg;
    arg.buf = &sem_info;

    bool bInit = false;
    int nMaxTime = 3;
    for( int i=0; i<nMaxTime; i++ )
    {
        if ( semctl (m_nSemID, 0, IPC_STAT, arg) == -1 )
        {   
            perror("semctl error"); 
            break;
        }
        
        m_nSemCnt = arg.buf->sem_nsems; 
        if ( arg.buf->sem_otime != 0 )
        { 
            bInit = true;
            break;
        }
        else
        {
            sleep (1);  
        }
    }
    if ( !bInit )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT);
        return CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT;
    }
    
    m_nKey = nKey;
    m_bAttach = true;
    m_nBCounterNum = GetBigCounterNum();
    m_nMaxVal = SEM_VALUE_MAX * SEM_VALUE_MAX;
    
    return m_nSemID;
}

int CSemaphore::CounterInc(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERINC_NOINIT);
        return CACHE_SEM_ERR_COUNTERINC_NOINIT;
    }
    int nRet = 0;
    for(int i = 0; i < 32; ++i)
    {
        struct sembuf sop[1];
        sop[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;
        sop[0].sem_op = 1;
        sop[0].sem_flg = IPC_NOWAIT;
        
        nRet =  semop(m_nSemID, sop, 1);
        if(nRet >= 0)
        {
            return 0;
        }        
        
        else if(nRet < 0 && errno == EINTR)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
            return SEM_EINTR;
        }
        
        else if(nRet < 0 && errno == ERANGE)    //进位
        {
            struct sembuf sops[2];
            sops[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;
            sops[0].sem_op = - SEM_VALUE_MAX;
            sops[0].sem_flg = IPC_NOWAIT;
            sops[1].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;
            sops[1].sem_op = 1;
            sops[1].sem_flg = IPC_NOWAIT;
            
            nRet =  semop(m_nSemID, sops, 2);
            if(nRet >= 0)
            {
                return 0;
            }
            
            else if(nRet < 0 && errno == EINTR)
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
                return SEM_EINTR;
            }
            
            else if(nRet < 0 && errno == EAGAIN)    //低位不够减
            {
                continue;
            }
            
            else if(nRet < 0 && errno == ERANGE)    //高位超过最大值
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, SEM_COUNTER_FULL);
                return SEM_COUNTER_FULL;
            }
            
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERINC_SEMOP2, strerror(errno));
                return CACHE_SEM_ERR_COUNTERINC_SEMOP2;
            }
        }
        
        else
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERINC_SEMOP, strerror(errno));
            return CACHE_SEM_ERR_COUNTERINC_SEMOP;
        }
    }

    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERINC);
    return CACHE_SEM_ERR_COUNTERINC;
}

int CSemaphore::CounterDec(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERDEC_NOINIT);
        return CACHE_SEM_ERR_COUNTERDEC_NOINIT;
    }
    int nRet = 0;
START:
    
    if(m_nGoNextSCounter[nBCounterSeq] == 1)    //高位有进位
    {
        for(int i = 0; i < 32; ++i)
        {
            struct sembuf sop[1];
            sop[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;   
            sop[0].sem_op = -1;
            sop[0].sem_flg = IPC_NOWAIT;
            
            nRet =  semop(m_nSemID, sop, 1);//低位-1
            if(nRet >= 0)
            {
                m_nGoNextSCounter[nBCounterSeq] = 1;
                return 0;
            }
            
            else if(nRet < 0 && errno == EINTR)
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
                return SEM_EINTR;
            }
            
            else if(nRet < 0 && errno == EAGAIN)//低位不够-1
            {
                struct sembuf sops[2];
                sops[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;   
                sops[0].sem_op = SEM_VALUE_MAX;
                sops[0].sem_flg = IPC_NOWAIT;
                sops[1].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;     
                sops[1].sem_op = -1;
                sops[1].sem_flg = IPC_NOWAIT;
                
                nRet =  semop(m_nSemID, sops, 2);   //执行退位  低位加满  高位-1
                if(nRet >= 0)
                {
                    if( GetSemVal(GetCounterSWSemSeq(nBCounterSeq)) > 0 )
                        m_nGoNextSCounter[nBCounterSeq] = 1;
                    else
                        m_nGoNextSCounter[nBCounterSeq] = 0;
                        
                    return 0;
                }
                
                else if(nRet < 0 && errno == EINTR)
                {
                    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
                    return SEM_EINTR;
                }
                
                else if(nRet < 0 && errno == ERANGE)    //低位不能加满
                {
                    continue;
                }
                
                else if(nRet < 0 && errno == EAGAIN)    //高位不能-1 无进位
                {
                    m_nGoNextSCounter[nBCounterSeq] = 0;
                    break;
                }
                
                else
                {
                    snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERDEC_SEMOP2, strerror(errno));
                    return CACHE_SEM_ERR_COUNTERDEC_SEMOP2;
                }
            }
            
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERDEC_SEMOP1, strerror(errno));
                return CACHE_SEM_ERR_COUNTERDEC_SEMOP1;
            }
            
        }//end for

        //snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d]", __func__, CACHE_SEM_ERR_COUNTERDEC);
        //return CACHE_SEM_ERR_COUNTERDEC;
    }//end if
    
    struct sembuf sops[2];
    sops[0].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;   
    sops[0].sem_op = 0;
    sops[0].sem_flg = IPC_NOWAIT;
    sops[1].sem_num = GetCounterSemSeq(nBCounterSeq)  ;     
    sops[1].sem_op = - 1;
    sops[1].sem_flg = 0;
    
    nRet =  semop(m_nSemID, sops, 2);//检查高位    低位-1
    if(nRet >= 0)
    {
        m_nGoNextSCounter[nBCounterSeq] = 0;//高位无进位
        return 0;
    }
    
    else if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    
    else if(nRet < 0 && errno == EAGAIN)    //高位有进位
    {
        m_nGoNextSCounter[nBCounterSeq] = 1;
        goto START;
    }
    
    else
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "[%s=%d] %s", __func__, CACHE_SEM_ERR_COUNTERDEC_SEMOP3, strerror(errno));
        return CACHE_SEM_ERR_COUNTERDEC_SEMOP3;
    }
    
    return 0;
}

unsigned long long CSemaphore::GetCounterVal (int nBCounterSeq )
{
    int high = GetSemVal(GetCounterSWSemSeq(nBCounterSeq));
    int low = GetSemVal(GetCounterSemSeq(nBCounterSeq));
    
    unsigned long long val = high*(SEM_VALUE_MAX+1) + low;
    return val;
}
#endif
