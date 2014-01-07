// ShareMem.cpp: implementation of the CShareMem class.
//
//////////////////////////////////////////////////////////////////////

/* ======================================================================

-- ftok
函数用于生成一个键值：key_t key，该键值将作为共享内存对象的唯一性标识符，
并提供给为shmget函数作为其输入参数；ftok 函数的输入参数包括一个文件（或目录）
路径名：pathname，以及一个额外的数字：proj_id，其中pathname所指定的文件（或目
录）要求必须已经存在，且proj_id不可为0； 

Attention: If the Path parameter of the ftok subroutine names a file that has 
been removed while keys still refer to it, the ftok subroutine returns an error. 
     ~~~~~~~
If that file is then re-created, the ftok subroutine will probably return a key 
                     ~~~~~~~~~~
different from the original one.
~~~~~~~~~

-- shmget
函数用于创建（或者获取）一个由key键值指定的共享内存对象，返回该对象的系统标识符：shmid； 


-- shmat
函数用于建立调用进程与由标识符shmid指定的共享内存对象之间的连接； 
对于64位进程，同一进程可连接最多268435456个共享内存段； 
对于32位进程，同一进程可连接最多11个共享内存段，除非使用扩展的shmat；
说明超出11个连接之后，所有后续的共享内存连接都将无法建立。错误码24的定义是EMFILE，
AIX给予的解释是：
The number of shared memory segments attached to the calling process exceeds the 
system-imposed limit。
解决这个问题的方法是，使用扩展的shmat；具体而言就是，在运行相关应用之前（确切地说，
是在共享内存被创建之前），首先在shell中设置EXTSHM环境变量，通过它扩展shmat，
对于源代码本身无需作任何修改：


-- shmdt
函数用于断开调用进程与共享内存对象之间的连接； 


-- shmctl
函数用于对已创建的共享内存对象进行查询、设值、删除等操作； 

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
    //已经attach或create
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
	    //共享内存已经被创建
	    //若bAtExist为真，则连接上已经存在的共享内存
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
	    //初始化头部信息失败，删除共享内存
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
    //已经attach或create
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


