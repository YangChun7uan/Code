// Semaphore.cpp: implementation of the CSem class.
//
//////////////////////////////////////////////////////////////////////

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
#include "Sem.h"



/*====================================================================
	int semget(key,count,flags)		��ȡ�ź������ϵı�ʶ��
		key_tkey; 					�ź������ϵļ�
		intcount; 					�ź���������Ԫ�ظ���
		intflags; 					��ѡ����
		return 						�ź������ϱ�ʶ��,�������򷵻�-1
=====================================================================*/ 

/*====================================================================
	int semop(sid,ops,nops)			�ź�������
		int sid; 					�ź������ϱ�ʶ��
		struct sembuf *ops; 		�ź��������ṹ��ָ��
		intnops; 					�ź��������ṹ�ĸ���
		return 						�������ǰ�����ź��������һ����
									������ź�����ֵ,�������򷵻�-1
ϵͳ����semop�������ź��������е�һ�������ź������в���,
�����������û��ṩ�Ĳ����ṹ����������,�ýṹ����: 
	struct sembuf
	{ 
		short sem_num; 				�ź����ڼ����е��±�
		short sem_op; 				����ֵ
		short sem_flg; 				������־ 
	}; 
=====================================================================*/ 

/*====================================================================
	int semctl(sid,semnum,cmd,arg) 	�����ź�������
		intsid; 					�ź������ϱ�ʶ��
		intsemnum; 					�ź���Ԫ�ر��
		intcmd; 					��������
		union semun
		{ 
			intval; 
			struct semid_ds *buf; 
			ushort*array;
		} arg; 						�������
=====================================================================*/ 
/*
��ȡ���ź������� 
int semctl(sid,count,IPC_RMID,0) 
int sid; �ź������ϱ�ʶ�� 
int count; �ź���������Ԫ�ظ���
*/

/*
�������ź������ϵĳ�ֵ(��ʼ��) 
�ź������ϸս���ʱ,���ź����ĳ�ֵ��ȷ��,��Ҫ�趨��ֵ����ֵ���趨����SETALL��SETVAL���

����SETALL����,���ʽΪ: 
int semctl(sid,count,SETALL,arg) 
int sid; �ź������ϱ�ʶ��
int count; �ź���������Ԫ�ظ���
ushort *arg; �������
�����������arg�е�ǰcount��ֵ���θ��������и��ź���,һ�ο��趨����ź����ĳ�ֵ�� 

����SETVAL����,���ʽΪ: 
int semctl(sid,semnum,SETVAL,arg) 
int sid; �ź������ϱ�ʶ��
int semnum; �ź���Ԫ�ر��
int arg; �������
�����arg.intval��ֵ���������е�semnum���ź���,һ�ν����趨һ���ź����ĳ�ֵ�� 
*/

/*
�۲�ѯ�ź������ϵĵ�ǰֵ 
��ѯ�ź������ϵĵ�ǰֵ����GETALL��GETVAL���

����GETALL����,���ʽΪ: 
int semctl(sid,count,GETALL,arg) 
int sid; �ź������ϱ�ʶ��
int count; �ź���������Ԫ�ظ���
ushort *arg; �������
��������ź��������и��ź����ĵ�ǰֵ���ص�����arg�С�

����GETVAL����,���ʽΪ: 
int semctl(sid,semnum,GETVAL,0) 
int sid; �ź������ϱ�ʶ��
int semnum; �ź���Ԫ�ر��
������Ѽ����е�semnum���ź����ĵ�ǰֵ��Ϊ���õķ���ֵ�� 
*/

/*
�ܲ�ѯĳ���ź����ĵȴ������� 
��һ������Ҫִ���ź�������ʱ���������߱�������,�й��ź����ĵȴ�������Ҳ��Ӧ�仯�� 

ͨ��GETNCNT����ɲ�ѯ�ȴ��ź�����ֵ�Ľ�����,���ʽ����: 
int semctl(sid,semnum,GETNCNT,0) 
int sid; �ź������ϱ�ʶ��
int semnum; �ź���Ԫ�ر��
������ѵȴ���semnum���ź�����ֵ�Ľ�������Ϊ���õķ���ֵ�� 

ͨ��GETZCNT����ɲ�ѯ�ȴ��ź���ֵΪ0�Ľ�����,���ʽ����: 
int semctl(sid,semnum,GETZCNT,0) 
int sid; �ź������ϱ�ʶ��
int semnum; �ź���Ԫ�ر��
������ѵȴ���semnum���ź���ֵΪ0�Ľ�������Ϊ���õķ���ֵ�� 
*/

 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSem::CSem()
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

CSem::CSem(int nKey, bool bAttach)
{
	m_nKey = nKey;

	if ( bAttach )
		Attach ( nKey );
	else
		Create ( nKey );
}


CSem::~CSem()
{
	if ( ( m_nSemID > 0 ) && !m_bAttach )
		Release ( );
}


int CSem::Create(const char* pszFile, int nCnt, int initVal, int nProj)
{
    int nKey = ftok ( pszFile, nProj );

    if ( nKey == -1 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATE_FTOK, strerror(errno));
        return SEM_ERR_CREATE_FTOK;
    }
    
    return Create(nKey, nCnt, initVal);
}

int CSem::Create(int nKey, int nCnt, int initVal)
{
    assert ( nKey != -1 );
    int nSemID, nRet;
    union semun sem ;   

    if ( nCnt < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_CREATE_PARA);
        return SEM_ERR_CREATE_PARA; 
    }
    nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATE_SEMGET, strerror(errno));
        return SEM_ERR_CREATE_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;

    //��ʼ���ź���
    sem.val = initVal; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < nCnt; i++ )
        sem.array[i] = initVal;
    
    nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;

    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATE_SEMCTL, strerror(errno));
        return SEM_ERR_CREATE_SEMCTL;
    }
    ///��ʼ�����������ã���Ҫ��Ҫ����semop���������ı�arg.buf->sem_otime
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

int CSem::Test0(int nSemIdx, bool bNoWait)
{
    short nFlag = 0;
    
    if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_TEST0_PARA);
        return SEM_ERR_TEST0_PARA;
    }
    
    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, 0, nFlag};
      
    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno == EAGAIN)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_TEST0_NOT0, strerror(errno));
        return SEM_ERR_TEST0_NOT0;
    }
    else if(nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_TEST0_SEMOP, strerror(errno));
        return SEM_ERR_TEST0_SEMOP;
    }
    
    return nRet;
}

int CSem::Lock(int nSemIdx, bool bNoWait)
{
	short nFlag = 0;

	if (m_nSemID == 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_LOCK_NOINIT);
	    return SEM_ERR_LOCK_NOINIT;
	}
	if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_LOCK_PARA);
		return SEM_ERR_LOCK_PARA;
	}
	
	nFlag = nFlag | SEM_UNDO;
	
	if ( bNoWait )
	    nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, -1, nFlag};
	  
	int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_LOCK_SEMOP, strerror(errno));
        return SEM_ERR_LOCK_SEMOP;
    }
    
    return nRet;
}

int CSem::Unlock(int nSemIdx, bool bNoWait)
{
	short nFlag = 0;

	if (m_nSemID == 0 )
    {
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_UNLOCK_NOINIT);
        return SEM_ERR_UNLOCK_NOINIT;
    }
	if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_UNLOCK_PARA);
        return SEM_ERR_UNLOCK_PARA;
    }
	
	nFlag = nFlag | SEM_UNDO;
	
    if ( bNoWait )
        nFlag = nFlag | IPC_NOWAIT;

    struct sembuf sops = {nSemIdx, 1, nFlag };

    int nRet = -1;
    nRet =  semop(m_nSemID,&sops,1);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_UNLOCK_SEMOP, strerror(errno));
        return SEM_ERR_UNLOCK_SEMOP;
    }
    
    return nRet;
}

int CSem::Inc(int nSemIdx, bool bNoWait, bool bUndo, int val)
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

int CSem::Increase(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_INCREASE_NOINIT);
        return SEM_ERR_INCREASE_NOINIT;
    }
    
    if ( !m_bAttach && (nSemIdx>m_nSemCnt || nSemIdx<0) )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_INCREASE_PARA);
        return SEM_ERR_INCREASE_PARA;
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
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_INCREASE_SEMOP, strerror(errno));
        return SEM_ERR_INCREASE_SEMOP;
    }
    return nRet;
}

int CSem::Dec(int nSemIdx, bool bNoWait, bool bUndo, int val)
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

int CSem::Decrease(int nSemIdx, bool bNoWait, bool bUndo, int val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_DECREASE_NOINIT);
        return SEM_ERR_DECREASE_NOINIT;
    }
    
    if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_DECREASE_PARA);
        return SEM_ERR_DECREASE_PARA;
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
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if(nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_DECREASE_SEMOP, strerror(errno));
        return SEM_ERR_DECREASE_SEMOP;
    }

    return nRet;
}

int CSem::Wait(int nSemIdx, bool bNoWait, bool bReverse)
{
    int nRet = 0;

    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_WAIT_NOINIT);
        return SEM_ERR_WAIT_NOINIT;
    }
    if ( (!m_bAttach && (nSemIdx>m_nSemCnt)) || nSemIdx<0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_WAIT_PARA);
        return SEM_ERR_WAIT_PARA;
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

int CSem::Release()
{
    m_nSemID = 0;
	m_nSemCnt = 0;
	return 0;
}

int CSem::Destroy()
{
    int nRet = 0;

    
    if(m_nSemID > 0)
    {
        nRet = semctl ( m_nSemID, m_nSemCnt, IPC_RMID, 0 );
        if(nRet < 0)
        {
            m_nSemID = 0;
            m_nSemCnt = 0;
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_DESTROY_SEMCTL, strerror(errno));
            return SEM_ERR_DESTROY_SEMCTL;
        }
        
    }

    m_nSemID = 0;
    m_nSemCnt = 0;
    
    return 0;
}

int CSem::Attach ( const char* pszFile, int nProj)
{
	int nKey = ftok ( pszFile, nProj );
	if ( nKey == -1 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_ATTACH_FTOK, strerror(errno));
		return SEM_ERR_ATTACH_FTOK;
	}
	return Attach ( nKey);
}

int CSem::Attach(int nKey)
{
    int nFlag = 0;
    m_nSemID = semget ( nKey, 0, nFlag );
    if (m_nSemID < 0)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_ATTACH_SEMGET, strerror(errno));
		return SEM_ERR_ATTACH_SEMGET;
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
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_ATTACH_NOINIT);
	    return SEM_ERR_ATTACH_NOINIT;
    }

    m_nKey = nKey;
    m_bAttach = true;

    return m_nSemID;
}

int CSem::GetSemVal(int nSemIdx)
{
	if (m_nSemID == 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_GETSEMVAL_NOINIT);
	    return SEM_ERR_GETSEMVAL_NOINIT;
	}

	int nRet = semctl ( m_nSemID, nSemIdx, GETVAL, 0 );
	if( nRet<0)
	{
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_GETSEMVAL_SEMCTL, strerror(errno));
	    return SEM_ERR_GETSEMVAL_SEMCTL;
	}
	
	return nRet;
}

int CSem::SetSemVal(int nSemIdx, ushort val)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_SETSEMVAL_NOINIT);
        return SEM_ERR_SETSEMVAL_NOINIT;
    }

    union semun sem ;
    sem.val = val;

    if( semctl ( m_nSemID, nSemIdx, SETVAL, sem ) < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_SETSEMVAL_SEMCTL, strerror(errno));
        return SEM_ERR_SETSEMVAL_SEMCTL;
    }
    
    return 0;
}

int CSem::GetAllSemVal(ushort* value, int nCnt)
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_GETALLSEMVAL_NOINIT);
        return SEM_ERR_GETALLSEMVAL_NOINIT;
    }
    
    union semun sem ;
    sem.array = value;
    
    int nRet = semctl (m_nSemID, nCnt, GETALL, sem);
    if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    else if (nRet < 0 && errno != EINTR)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_GETALLSEMVAL_SEMCTL, strerror(errno));
        return SEM_ERR_GETALLSEMVAL_SEMCTL;
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////

int CSem::CreateCounter(const char* pszFile, int nCounterNum, int nProj)
{
    int nKey = ftok ( pszFile, nProj );

    if ( nKey == -1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATECOUNTER_FTOK, strerror(errno));
        return SEM_ERR_CREATECOUNTER_FTOK;
    }
    return CreateCounter(nKey, nCounterNum);
}

int CSem::AttachCounter ( const char* pszFile, int nProj)
{
    int nKey = ftok ( pszFile, nProj );
    if ( nKey == -1 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_ATTACHCOUNTER_FTOK, strerror(errno));
        return SEM_ERR_ATTACHCOUNTER_FTOK;
    }

    return AttachCounter ( nKey);
}

int CSem::GetBigCounterNum()
{
    return GetSemVal(m_nSemCnt - 1);
}

#ifdef MOTHOD_SW_COUNTER

//   [ Sem1 Sem2 ]  = SmallCounter
//   �����ź���Ϊ1��SmallCounter�����1��С����������һ���ź����������أ��ڶ������ڼ���
//
//   [Group1]...[GroupN] = BigCounter  
//   ���SmallCounter���1���������
//
//   [BigCounter1] ... [BigCounterN] = Semaphores
//   �ź��������ݴ�������ĸ�����Ϊÿ���������ƽ������SmallCounter����
//
//   ÿ��BigCounter��������ֵ = SmallCounter���� * SEM_VALUE_MAX
int CSem::CreateCounter(int nKey, int nCounterNum)
{
    if ( nCounterNum > MAX_BIGCOUNTER_NUM || nCounterNum < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_CREATECOUNTER_PARA);
        return SEM_ERR_CREATECOUNTER_PARA;
    }
    int nCnt = 65535;
    int nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATECOUNTER_SEMGET, strerror(errno));
        return SEM_ERR_CREATECOUNTER_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;
    m_nBCounterNum = nCounterNum;
    int res = 0;
    if(m_nBCounterNum % 2 == 0)
    {
        res = 0;//ʣ�µ��ǵ���
    }
    else
        res = 1;//ʣ�µ���˫��
    
    m_nSCounterNum  = ((m_nSemCnt - res - m_nBCounterNum - 1) / m_nBCounterNum) / 2 ;
    m_nMaxVal = m_nSCounterNum * SEM_VALUE_MAX;

    //��ʼ���ź���
    union semun sem;
    sem.val = 0; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < m_nSCounterNum * 2 * m_nBCounterNum; i++ )
    {
        if(i % (m_nSCounterNum * 2) == 0)                  //��һ�����ش�
        {
            sem.array[i] = 0;
            continue;
        }
        if(i % 2 == 0)                  //���࿪�عر�
        {
            sem.array[i] = 1;
        }
        else                            //��������0
            sem.array[i] = 0;
    }
    
    for ( int i = m_nSCounterNum * 2 * m_nBCounterNum; i < m_nSCounterNum * 2 * m_nBCounterNum + m_nBCounterNum; i++ )//SmallCounter��¼��0
    {
        sem.array[i] = 0;
    }
    
    for ( int i = m_nSCounterNum * 2 * m_nBCounterNum + m_nBCounterNum; i < m_nSemCnt - 1; i++ )//������
    {
        sem.array[i] = 0;
    }
    
    sem.array[m_nSemCnt - 1] = m_nBCounterNum; //���һ�����ڼ�¼BigCounter����
    
    int nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;
    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATECOUNTER_SEMCTL, strerror(errno));
        return SEM_ERR_CREATECOUNTER_SEMCTL;
    }
    
    nRet = Increase(0, true, true);
    if(nRet < 0)
        return nRet;
    
    nRet = Decrease(0, true, true);
    if(nRet < 0)
        return nRet;
    
    return m_nSemID;
}

int CSem::AttachCounter(int nKey)
{
    m_nSemID = semget ( nKey, 0, 0 );
    if (m_nSemID == -1)
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_ATTACHCOUNTER_SEMGET, strerror(errno));
        return SEM_ERR_ATTACHCOUNTER_SEMGET;
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
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_ATTACHCOUNTER_NOINIT);
        return SEM_ERR_ATTACHCOUNTER_NOINIT;
    }
    m_nKey = nKey;
    m_bAttach = true;
    
    m_nBCounterNum = GetBigCounterNum();
    int res = 0;
    if(m_nBCounterNum % 2 == 0)
    {
        res = 0;//ʣ�µ��ǵ���
    }
    else
        res = 1;//ʣ�µ���˫��
    
    m_nSCounterNum  = ((m_nSemCnt - res - m_nBCounterNum - 1) / m_nBCounterNum) / 2 ;
    m_nMaxVal = m_nSCounterNum * SEM_VALUE_MAX;
    
    for(int i = 0; i < m_nBCounterNum; ++i)
    {
        m_nSCounterSeq[i] = GetSemVal( GetCounterRecSemSeq(i) );
    }
    
    return m_nSemID;
}

int CSem::GetCounterSWSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * nBCounterSeq + 2 * m_nSCounterSeq[nBCounterSeq] ;
}

int CSem::GetCounterSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * nBCounterSeq + 2 * m_nSCounterSeq[nBCounterSeq] + 1  ;
}

int CSem::GetCounterRecSemSeq(int nBCounterSeq)
{
    return 2 * m_nSCounterNum * m_nBCounterNum + nBCounterSeq ;
}

int CSem::CounterInc(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERINC_NOINIT);
        return SEM_ERR_COUNTERINC_NOINIT;
    }

    int nRet = -1;
    for(int i = 0; i < m_nSCounterNum; ++i)
    {
        struct sembuf sops[2];
        sops[0].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;
        sops[0].sem_op = 0;                                     //��0��������EAGAIN
        sops[0].sem_flg = IPC_NOWAIT;
        sops[1].sem_num = GetCounterSemSeq(nBCounterSeq);
        sops[1].sem_op = 1;                                     //����+1���������ֵ����ERANGE
        sops[1].sem_flg = 0;
        
        nRet =  semop(m_nSemID, sops, 2);
        if(nRet < 0 && errno == EINTR)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
            return SEM_EINTR;
        }
        
        else if(nRet < 0 && errno == EAGAIN) //���ط�0���Ѿ�ת����һ��SmallCounter
        {
            if(GetSemVal( GetCounterSWSemSeq(nBCounterSeq) ) != 0)//���ط�0
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
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERINC_VAL);
                return SEM_ERR_COUNTERINC_VAL;
            }
        }
        
        else if(nRet < 0 && errno == ERANGE) //�������ֵ��ת����һ��index����
        {
            int save = m_nSCounterSeq[nBCounterSeq];
            
            ++ m_nSCounterSeq[nBCounterSeq];
            if(m_nSCounterSeq[nBCounterSeq] >= m_nSCounterNum)
            {
                m_nSCounterSeq[nBCounterSeq] = 0;
            }
            
            if( GetSemVal( GetCounterSemSeq(nBCounterSeq) ) == 0 )             //��һ��SmallCounter����ֵΪ0
                SetSemVal( GetCounterSWSemSeq(nBCounterSeq), 0);               //�ȿ�����һ��������
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_COUNTER_FULL);
                return SEM_COUNTER_FULL;
            }

            SetSemVal(GetCounterRecSemSeq(nBCounterSeq), m_nSCounterSeq[nBCounterSeq]);            //���óɵ�����ʹ�õ�SmallCounter��index
            
            SetSemVal( 2 * m_nSCounterNum * nBCounterSeq + 2 * save, 1);                            //�ٹر���һ��������

            continue;
        }
        
        else if(nRet < 0)
        {
            
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERINC_SEMOP, strerror(errno));
            return SEM_ERR_COUNTERINC_SEMOP;
        }
        else
            return 0;
    }

    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERINC);
    return SEM_ERR_COUNTERINC;
}

int CSem::CounterDec(int nBCounterSeq )
{    
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERDEC_NOINIT);
        return SEM_ERR_COUNTERDEC_NOINIT;
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
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
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
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERDEC_VAL);
                return SEM_ERR_COUNTERDEC_VAL;
            }
            
            continue;
        }
        
        else if(nRet < 0)
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERDEC_SEMOP, strerror(errno));
            return SEM_ERR_COUNTERDEC_SEMOP;
        }
        else
        {
            return 0;
        }
    }
    
    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERDEC);
    return SEM_ERR_COUNTERDEC;
}

int GetCounterVal (int nBCounterSeq )
{
    return 0;
}

#else

int CSem::GetCounterSWSemSeq(int nBCounterSeq)
{
    return nBCounterSeq * 2 + 1 ;
}

int CSem::GetCounterSemSeq(int nBCounterSeq)
{
    return nBCounterSeq * 2 ;
}

int CSem::CreateCounter(int nKey, int nCounterNum)
{
    if ( nCounterNum > MAX_BIGCOUNTER_NUM || nCounterNum < 1 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_CREATECOUNTER_PARA);
        return SEM_ERR_CREATECOUNTER_PARA;
    }
    int nCnt = nCounterNum * 2 + 1;
    int nSemID = semget ( nKey,nCnt,IPC_CREAT|0666 );
    if (nSemID == -1)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATECOUNTER_SEMGET, strerror(errno));
        return SEM_ERR_CREATECOUNTER_SEMGET;
    }

    m_nKey = nKey;
    m_nSemID = nSemID;
    m_nSemCnt = nCnt;
    m_nBCounterNum = nCounterNum;
    m_nMaxVal = SEM_VALUE_MAX * SEM_VALUE_MAX;

    //��ʼ���ź���
    union semun sem;
    sem.val = 0; 
    sem.array = new ushort [nCnt];
    for ( int i = 0; i < nCnt; i++ )
    {
        sem.array[i] = 0;
    }
    sem.array[m_nSemCnt - 1] = m_nBCounterNum; //���һ�����ڼ�¼BigCounter����
    
    int nRet = semctl (m_nSemID, nCnt, SETALL, sem);
    delete [] sem.array;
    if ( nRet < 0 )
    {
        
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_CREATECOUNTER_SEMCTL, strerror(errno));
        return SEM_ERR_CREATECOUNTER_SEMCTL;
    }
    
    nRet = Increase(0, true, true);
    if(nRet < 0)
        return nRet;
    
    nRet = Decrease(0, true, true);
    if(nRet < 0)
        return nRet;
    
    return m_nSemID;
}

int CSem::AttachCounter(int nKey)
{
    m_nSemID = semget ( nKey, 0, 0 );
    if (m_nSemID == -1)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_ATTACHCOUNTER_SEMGET, strerror(errno));
        return SEM_ERR_ATTACHCOUNTER_SEMGET;
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
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_ATTACHCOUNTER_NOINIT);
        return SEM_ERR_ATTACHCOUNTER_NOINIT;
    }
    
    m_nKey = nKey;
    m_bAttach = true;
    m_nBCounterNum = GetBigCounterNum();
    m_nMaxVal = SEM_VALUE_MAX * SEM_VALUE_MAX;
    
    return m_nSemID;
}

int CSem::CounterInc(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERINC_NOINIT);
        return SEM_ERR_COUNTERINC_NOINIT;
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
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
            return SEM_EINTR;
        }
        
        else if(nRet < 0 && errno == ERANGE)    //��λ
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
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
                return SEM_EINTR;
            }
            
            else if(nRet < 0 && errno == EAGAIN)    //��λ������
            {
                continue;
            }
            
            else if(nRet < 0 && errno == ERANGE)    //��λ�������ֵ
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_COUNTER_FULL);
                return SEM_COUNTER_FULL;
            }
            
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERINC_SEMOP2, strerror(errno));
                return SEM_ERR_COUNTERINC_SEMOP2;
            }
        }
        
        else
        {
            snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERINC_SEMOP, strerror(errno));
            return SEM_ERR_COUNTERINC_SEMOP;
        }
    }

    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERINC);
    return SEM_ERR_COUNTERINC;
}

int CSem::CounterDec(int nBCounterSeq )
{
    if (m_nSemID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERDEC_NOINIT);
        return SEM_ERR_COUNTERDEC_NOINIT;
    }
    int nRet = 0;
START:
    
    if(m_nGoNextSCounter[nBCounterSeq] == 1)    //��λ�н�λ
    {
        for(int i = 0; i < 32; ++i)
        {
            struct sembuf sop[1];
            sop[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;   
            sop[0].sem_op = -1;
            sop[0].sem_flg = IPC_NOWAIT;
            
            nRet =  semop(m_nSemID, sop, 1);//��λ-1
            if(nRet >= 0)
            {
                m_nGoNextSCounter[nBCounterSeq] = 1;
                return 0;
            }
            
            else if(nRet < 0 && errno == EINTR)
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
                return SEM_EINTR;
            }
            
            else if(nRet < 0 && errno == EAGAIN)//��λ����-1
            {
                struct sembuf sops[2];
                sops[0].sem_num = GetCounterSemSeq(nBCounterSeq)  ;   
                sops[0].sem_op = SEM_VALUE_MAX;
                sops[0].sem_flg = IPC_NOWAIT;
                sops[1].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;     
                sops[1].sem_op = -1;
                sops[1].sem_flg = IPC_NOWAIT;
                
                nRet =  semop(m_nSemID, sops, 2);   //ִ����λ  ��λ����  ��λ-1
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
                    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
                    return SEM_EINTR;
                }
                
                else if(nRet < 0 && errno == ERANGE)    //��λ���ܼ���
                {
                    continue;
                }
                
                else if(nRet < 0 && errno == EAGAIN)    //��λ����-1 �޽�λ
                {
                    m_nGoNextSCounter[nBCounterSeq] = 0;
                    break;
                }
                
                else
                {
                    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERDEC_SEMOP2, strerror(errno));
                    return SEM_ERR_COUNTERDEC_SEMOP2;
                }
            }
            
            else
            {
                snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERDEC_SEMOP1, strerror(errno));
                return SEM_ERR_COUNTERDEC_SEMOP1;
            }
            
        }//end for

        //snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SEM_ERR_COUNTERDEC);
        //return SEM_ERR_COUNTERDEC;
    }//end if
    
    struct sembuf sops[2];
    sops[0].sem_num = GetCounterSWSemSeq(nBCounterSeq)  ;   
    sops[0].sem_op = 0;
    sops[0].sem_flg = IPC_NOWAIT;
    sops[1].sem_num = GetCounterSemSeq(nBCounterSeq)  ;     
    sops[1].sem_op = - 1;
    sops[1].sem_flg = 0;
    
    nRet =  semop(m_nSemID, sops, 2);//����λ    ��λ-1
    if(nRet >= 0)
    {
        m_nGoNextSCounter[nBCounterSeq] = 0;//��λ�޽�λ
        return 0;
    }
    
    else if(nRet < 0 && errno == EINTR)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_EINTR, strerror(errno));
        return SEM_EINTR;
    }
    
    else if(nRet < 0 && errno == EAGAIN)    //��λ�н�λ
    {
        m_nGoNextSCounter[nBCounterSeq] = 1;
        goto START;
    }
    
    else
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SEM_ERR_COUNTERDEC_SEMOP3, strerror(errno));
        return SEM_ERR_COUNTERDEC_SEMOP3;
    }
    
    return 0;
}

unsigned long long CSem::GetCounterVal (int nBCounterSeq )
{
    int high = GetSemVal(GetCounterSWSemSeq(nBCounterSeq));
    int low = GetSemVal(GetCounterSemSeq(nBCounterSeq));
    
    unsigned long long val = high*(SEM_VALUE_MAX+1) + low;
    return val;
}
#endif
