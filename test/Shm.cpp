// ShareMem.cpp: implementation of the CShareMem class.
//
//////////////////////////////////////////////////////////////////////

/* ======================================================================

-- ftok
������������һ����ֵ��key_t key���ü�ֵ����Ϊ�����ڴ�����Ψһ�Ա�ʶ����
���ṩ��Ϊshmget������Ϊ�����������ftok �����������������һ���ļ�����Ŀ¼��
·������pathname���Լ�һ����������֣�proj_id������pathname��ָ�����ļ�����Ŀ
¼��Ҫ������Ѿ����ڣ���proj_id����Ϊ0�� 

Attention: If the Path parameter of the ftok subroutine names a file that has 
been removed while keys still refer to it, the ftok subroutine returns an error. 
     ~~~~~~~
If that file is then re-created, the ftok subroutine will probably return a key 
                     ~~~~~~~~~~
different from the original one.
~~~~~~~~~

-- shmget
�������ڴ��������߻�ȡ��һ����key��ֵָ���Ĺ����ڴ���󣬷��ظö����ϵͳ��ʶ����shmid�� 


-- shmat
�������ڽ������ý������ɱ�ʶ��shmidָ���Ĺ����ڴ����֮������ӣ� 
����64λ���̣�ͬһ���̿��������268435456�������ڴ�Σ� 
����32λ���̣�ͬһ���̿��������11�������ڴ�Σ�����ʹ����չ��shmat��
˵������11������֮�����к����Ĺ����ڴ����Ӷ����޷�������������24�Ķ�����EMFILE��
AIX����Ľ����ǣ�
The number of shared memory segments attached to the calling process exceeds the 
system-imposed limit��
����������ķ����ǣ�ʹ����չ��shmat��������Ծ��ǣ����������Ӧ��֮ǰ��ȷ�е�˵��
���ڹ����ڴ汻����֮ǰ����������shell������EXTSHM����������ͨ������չshmat��
����Դ���뱾���������κ��޸ģ�


-- shmdt
�������ڶϿ����ý����빲���ڴ����֮������ӣ� 


-- shmctl
�������ڶ��Ѵ����Ĺ����ڴ������в�ѯ����ֵ��ɾ���Ȳ����� 

====================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/signal.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#include "Shm.h"


CShm::CShm ( )
{
	m_nSHMID = 0;
	m_nKey = 0;
	m_pData = NULL;
	memset(m_errmsg, 0, sizeof(m_errmsg));
}


CShm::~CShm ( )
{
	if ( m_nSHMID > 0 )
	{
		Release ( );
	}
}

int CShm::Destroy()
{
//	printf ( "CShm::Destroy id:%d\n", m_nSHMID );
    if ( m_nSHMID == 0 )
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SHM_ERR_DESTROY_PARA);
        return SHM_ERR_DESTROY_PARA;
    }
	shmid_ds buf;
	memset ( &buf, 0, sizeof(buf) );
	
	int nRet = 0;
	if ( shmctl ( m_nSHMID, IPC_RMID, &buf) < 0 )
	{
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_DESTORY_SHMCTL, strerror(errno));
        nRet = SHM_ERR_DESTORY_SHMCTL;
	}
	m_nSHMID = 0;
	m_nKey = 0;
	m_pData = NULL;
        
	return nRet;
}


int CShm::Release()
{
	int nRet = 0;

	if ( m_pData != NULL )
	{
	    nRet = shmdt ( (const void*)m_pData );
	    if(nRet < 0)
	    {
	        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_RELEASE_SHMDT, strerror(errno));
	        nRet = SHM_ERR_RELEASE_SHMDT;
	    }
	}

	m_nSHMID = 0;
	m_nKey = 0;
	m_pData = NULL;

	return nRet;
}

int CShm::Create(const char *pszPath, long nSize, bool bAtExist , int nProj)
{
	int nKey = ftok ( pszPath, nProj );
	if ( nKey == -1 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_CREATE_FTOK, strerror(errno));
		return SHM_ERR_CREATE_FTOK;
	}
    
	return Create ( nKey, nSize, bAtExist );
}

int CShm::Create(int nKey, long nSize, bool bAtExist)
{
    //�Ѿ�attach��create
    if( m_nKey != 0 || m_nSHMID != 0 || m_pData != NULL)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SHM_ERR_CREATE_REPEAT);
        return SHM_ERR_CREATE_REPEAT;
    }   
	int nSHMID = 0;
	long nAllocSize = 0;
	
	nAllocSize = sizeof(CShareMemData) + nSize; 

#ifdef AIX
    nSHMID = shmget ( nKey, nAllocSize, IPC_CREAT|IPC_EXCL|SHM_LGPAGE|SHM_PIN|0666 );
#else
    nSHMID = shmget ( nKey, nAllocSize, IPC_CREAT|IPC_EXCL|0666 );
#endif
	if ( nSHMID < 0 )
	{
	    //�����ڴ��Ѿ�������
	    //��bAtExistΪ�棬���������Ѿ����ڵĹ����ڴ�
		if ( ( errno == EEXIST) && bAtExist )
			return Attach(nKey);
		else
		{
		    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_CREATE_SHMGET, strerror(errno));
			return SHM_ERR_CREATE_SHMGET;
		}
	}
    
    int nRet = InitData ( nSHMID ) ;
	if ( nRet < 0 )
	{	    
	    //��ʼ��ͷ����Ϣʧ�ܣ�ɾ�������ڴ�
		shmid_ds buf;
		memset ( &buf, 0, sizeof(buf) );
		shmctl ( nSHMID, IPC_RMID, &buf);
		return nRet;
	}
	m_nKey = nKey;
	m_pData->length = nSize; 
    
	//memset(m_pData->data(), 0, nSize);
//	printf ( "CShareMem::Create id:%d ref:%d len:%d\n", 
//			m_nSHMID, m_pData->refs, m_pData->length );
    
	return  m_nSHMID;
}

int CShm::Attach(const char *pszPath, int nProj)
{
	int nKey = ftok ( pszPath, nProj );
	if ( nKey == -1 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_ATTACH_FTOK, strerror(errno));
		return SHM_ERR_ATTACH_FTOK;
	}

	return Attach ( nKey );
}

int CShm::Attach(int nKey)
{
    //�Ѿ�attach��create
    if( m_nKey != 0 || m_nSHMID != 0 || m_pData != NULL)
    {
        snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d", __func__, SHM_ERR_ATTACH_REPEAT);
        return SHM_ERR_ATTACH_REPEAT;
    }   
	int nSHMID = 0;

#ifdef AIX
	nSHMID = shmget ( nKey, 0, S_IRUSR|S_IWUSR|SHM_LGPAGE|SHM_PIN );
#else
	nSHMID = shmget ( nKey, 0, S_IRUSR|S_IWUSR );	
#endif

	if ( nSHMID < 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_ATTACH_SHMGET, strerror(errno));
		return SHM_ERR_ATTACH_SHMGET;
	}
    
    int nRet = InitData ( nSHMID );
	if ( nRet < 0 )
		return nRet;
		
	m_nKey = nKey;

//	printf ( "CShm::Attach id:%d ref:%d len:%d\n", 
//			m_nSHMID, m_pData->refs, m_pData->length );
    
	return m_nSHMID;
}

int CShm::Detach()
{
	return Release ( );
}

const void* CShm::MemGet() 
{
	if ( m_nKey == 0 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%s", __func__, "NULL");
		return NULL;
	}

	ASSERT ( m_pData );

	return m_pData->data ( );
}

int CShm::InitData( int nSHMID )
{
	CShareMemData *pData = (CShareMemData*) shmat ( nSHMID, 0, 0 );
	if ( pData == NULL || pData == (void*)-1 )
	{
	    snprintf(m_errmsg, sizeof(m_errmsg), "%s=%d,%s", __func__, SHM_ERR_INITDATA_SHMAT, strerror(errno));
		return SHM_ERR_INITDATA_SHMAT;
	}
    m_pid = getpid();
	m_nSHMID = nSHMID;
	m_pData = pData;
    
	return 0;
}


