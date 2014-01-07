// ShareMem.h: interface for the CShm class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _THIS_SHAREMEM_HEADER_FILE_
#define _THIS_SHAREMEM_HEADER_FILE_

#ifndef ASSERT 
#ifdef _DEBUG
#define ASSERT(bf) \
{ \
	if(!(bf)) \
	{ \
		printf("Assert failed at line %d of file %s\n", __LINE__,__FILE__); \
		exit (0); \
	} \
}
#else
#define ASSERT(bf)	void ()
#endif	/* _DEBUG */ 
#endif	/* ASSERT */

//////////////////////////////////////////////////////////////////////
//错误返回码定义
#define SHM_ERR_ATTACH_FTOK         -21
#define SHM_ERR_ATTACH_REPEAT       -22
#define SHM_ERR_ATTACH_SHMGET       -23
#define SHM_ERR_CREATE_FTOK         -24
#define SHM_ERR_CREATE_REPEAT       -25
#define SHM_ERR_CREATE_SHMGET       -26
#define SHM_ERR_DESTORY_SHMCTL      -27
#define SHM_ERR_INITDATA_SHMAT      -28
#define SHM_ERR_MEMLOCK_PARA        -29
#define SHM_ERR_MEMUNLOCK_PARA      -30
#define SHM_ERR_RELEASE_SHMDT       -31
#define SHM_ERR_DESTROY_PARA        -32

struct CShareMemData
{
	long         length;			// length of data (including terminator)
	// TCHAR data[nAllocLength]
	
	void* data()                    // TCHAR* to managed data
		{ return (unsigned char*)(this+1); }
};


class CShm
{
public:
	CShm ( );
	~CShm ( );

public:
	const void* MemGet ( );
	
    /* 
     * 功能：在系统创建共享内存
     * 输入：pszPath标志文件路径，nSize大小，bAtExist已存在时是否连接，nProj
     * 输出：
     * 返回：0
     * SHM_ERR_CREATE_REPEAT
     * SHM_ERR_CREATE_SHMGET
     */
    int Create(const char *pszPath, long nSize, bool bAtExist=false, int nProj = 1);
    int Create(int nKey, long nSize, bool bAtExist=false);

    /* 
     * 功能：连接已经存在的共享内存
     * 输入：pszPath标志文件路径，nProj
     * 输出：
     * 返回：0
     * SHM_ERR_ATTACH_FTOK
     * SHM_ERR_ATTACH_REPEAT
     * SHM_ERR_ATTACH_SHMGET
     */
	int Attach(const char *pszPath, int nProj = 1);
	int Attach(int nKey);

    /* 
     * 功能：断开共享内存连接
     * 输入：
     * 输出：
     * 返回：0
     * SHM_ERR_RELEASE_SHMDT
     */
	int Release ( );
	int Detach ( );
	
    /* 
     * 功能：销毁共享内存
     * 输入：
     * 输出：
     * 返回：0
     * SHM_ERR_DESTROY_PARA
     */
	int Destroy ( );
	
public:
	inline const CShareMemData* GetMemData ( )  { return (const CShareMemData*) m_pData;  };
	
	inline int GetSHMID ( )  { return m_nSHMID; };
	
	inline long Length ()  { ASSERT ( m_pData ); return m_pData->length; };
    
	inline const char* Strerr ( ) { return m_errmsg; };
protected:
	int m_nKey;                 
	int m_nSHMID;               //共享内存id
	CShareMemData *m_pData;     //共享内存头部信息
    pid_t m_pid;				//当前操作进程pid
    
    char m_errmsg[1024];
protected:
	int InitData ( int );
};

#endif


