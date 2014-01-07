
#ifndef __CACHE_SHAREMEMORY__H__
#define __CACHE_SHAREMEMORY__H__

//////////////////////////////////////////////////////////////////////
//���󷵻��붨��
#define CACHE_SHM_ERR_ATTACH_FTOK         -21
#define CACHE_SHM_ERR_ATTACH_REPEAT       -22
#define CACHE_SHM_ERR_ATTACH_SHMGET       -23
#define CACHE_SHM_ERR_CREATE_FTOK         -24
#define CACHE_SHM_ERR_CREATE_REPEAT       -25
#define CACHE_SHM_ERR_CREATE_SHMGET       -26
#define CACHE_SHM_ERR_DESTORY_SHMCTL      -27
#define CACHE_SHM_ERR_INITDATA_SHMAT      -28
#define CACHE_SHM_ERR_MEMLOCK_PARA        -29
#define CACHE_SHM_ERR_MEMUNLOCK_PARA      -30
#define CACHE_SHM_ERR_RELEASE_SHMDT       -31
#define CACHE_SHM_ERR_DESTROY_PARA        -32

struct ShmHeader
{
	long         length;			// length of data (including terminator)
	// TCHAR data[nAllocLength]
	
	void* data()                    // TCHAR* to managed data
		{ return (unsigned char*)(this+1); }
};


class CShareMemory
{
public:
	CShareMemory ( );
	~CShareMemory ( );

public:
	const void* MemGet ( );
	
    /* 
     * ���ܣ���ϵͳ���������ڴ�
     * ���룺pszPath��־�ļ�·����nSize��С��bAtExist�Ѵ���ʱ�Ƿ����ӣ�nProj
     * �����
     * ���أ�0
     * CACHE_SHM_ERR_CREATE_REPEAT
     * CACHE_SHM_ERR_CREATE_SHMGET
     */
    int Create(const char *pszPath, long nSize, bool bAtExist=false, int nProj = 1);
    int Create(int nKey, long nSize, bool bAtExist=false);

    /* 
     * ���ܣ������Ѿ����ڵĹ����ڴ�
     * ���룺pszPath��־�ļ�·����nProj
     * �����
     * ���أ�0
     * CACHE_SHM_ERR_ATTACH_FTOK
     * CACHE_SHM_ERR_ATTACH_REPEAT
     * CACHE_SHM_ERR_ATTACH_SHMGET
     */
	int Attach(const char *pszPath, int nProj = 1);
	int Attach(int nKey);

    /* 
     * ���ܣ��Ͽ������ڴ�����
     * ���룺
     * �����
     * ���أ�0
     * CACHE_SHM_ERR_RELEASE_SHMDT
     */
	int Release ( );
	int Detach ( );
	
    /* 
     * ���ܣ����ٹ����ڴ�
     * ���룺
     * �����
     * ���أ�0
     * CACHE_SHM_ERR_DESTROY_PARA
     */
	int Destroy ( );
	
public:
	inline const ShmHeader* GetMemData ( )  { return (const ShmHeader*) m_pData;  };
	
	inline int GetSHMID ( )  { return m_nSHMID; };
	
	inline long Length ()  { return m_pData->length; };
    
	inline const char* Strerr ( ) { return m_errmsg; };
protected:
	int m_nKey;                 
	int m_nSHMID;               //�����ڴ�id
	ShmHeader *m_pData;     //�����ڴ�ͷ����Ϣ
    pid_t m_pid;				//��ǰ��������pid
    
    char m_errmsg[1024];
protected:
	int InitData ( int );
};

#endif


