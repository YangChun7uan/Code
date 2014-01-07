
#ifndef __CACHE_SEMAPHORE_H___
#define __CACHE_SEMAPHORE_H___



#ifndef SEM_VALUE_MAX
#define  SEM_VALUE_MAX 32767
#endif

#define MAX_BIGCOUNTER_NUM                 128
#define MAX_PROCESS_READ_LOCK_NUM                 10

#define SEM_EINTR   -2
#define SEM_COUNTER_FULL                -3

#define CACHE_SEM_ERR_CREATECOUNTER_FTOK      -21
#define CACHE_SEM_ERR_CREATECOUNTER_PARA      -22
#define CACHE_SEM_ERR_CREATECOUNTER_SEMGET    -23
#define CACHE_SEM_ERR_CREATECOUNTER_SEMCTL    -24
#define CACHE_SEM_ERR_CREATE_FTOK             -25
#define CACHE_SEM_ERR_CREATE_PARA             -26
#define CACHE_SEM_ERR_CREATE_SEMGET           -27
#define CACHE_SEM_ERR_CREATE_SEMCTL           -28
#define CACHE_SEM_ERR_ATTACHCOUNTER_FTOK      -29
#define CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET    -30
#define CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT    -31
#define CACHE_SEM_ERR_TEST0_NOT0              -32
#define CACHE_SEM_ERR_TEST0_PARA              -33
#define CACHE_SEM_ERR_TEST0_SEMOP             -34
#define CACHE_SEM_ERR_LOCK_NOINIT             -35
#define CACHE_SEM_ERR_LOCK_PARA               -36
#define CACHE_SEM_ERR_LOCK_SEMOP              -37
#define CACHE_SEM_ERR_UNLOCK_NOINIT           -38
#define CACHE_SEM_ERR_UNLOCK_PARA             -39
#define CACHE_SEM_ERR_UNLOCK_SEMOP            -40
#define CACHE_SEM_ERR_INCREASE_NOINIT         -41
#define CACHE_SEM_ERR_INCREASE_PARA           -42
#define CACHE_SEM_ERR_INCREASE_SEMOP          -43
#define CACHE_SEM_ERR_DECREASE_NOINIT         -44
#define CACHE_SEM_ERR_DECREASE_PARA           -45
#define CACHE_SEM_ERR_DECREASE_SEMOP          -46
#define CACHE_SEM_ERR_WAIT_NOINIT             -47
#define CACHE_SEM_ERR_WAIT_PARA               -48
#define CACHE_SEM_ERR_DESTROY_SEMCTL          -49
#define CACHE_SEM_ERR_ATTACH_FTOK             -50
#define CACHE_SEM_ERR_ATTACH_SEMGET           -51
#define CACHE_SEM_ERR_ATTACH_NOINIT           -52
#define CACHE_SEM_ERR_GETSEMVAL_NOINIT        -53
#define CACHE_SEM_ERR_GETSEMVAL_SEMCTL        -54
#define CACHE_SEM_ERR_SETSEMVAL_NOINIT        -55
#define CACHE_SEM_ERR_SETSEMVAL_SEMCTL        -56
#define CACHE_SEM_ERR_GETALLSEMVAL_NOINIT     -57
#define CACHE_SEM_ERR_GETALLSEMVAL_SEMCTL     -58
#define CACHE_SEM_ERR_COUNTERINC_VAL          -59
#define CACHE_SEM_ERR_COUNTERINC_SEMOP        -60
#define CACHE_SEM_ERR_COUNTERINC_SEMOP2       -61
#define CACHE_SEM_ERR_COUNTERINC_NOINIT       -62
#define CACHE_SEM_ERR_COUNTERINC              -63
#define CACHE_SEM_ERR_COUNTERDEC_NOINIT       -64
#define CACHE_SEM_ERR_COUNTERDEC_PARA         -65
#define CACHE_SEM_ERR_COUNTERDEC_VAL          -66
#define CACHE_SEM_ERR_COUNTERDEC_SEMOP        -67
#define CACHE_SEM_ERR_COUNTERDEC_SEMOP1       -68
#define CACHE_SEM_ERR_COUNTERDEC_SEMOP2       -69
#define CACHE_SEM_ERR_COUNTERDEC_SEMOP3       -70
#define CACHE_SEM_ERR_COUNTERDEC              -71


class CSemaphore  
{
public:
	CSemaphore();
	~CSemaphore();
	CSemaphore ( int nKey, bool bAttach=false );

public:
    
    /* 
     * 功能：在系统创建信号量集
     * 输入：pszPath标志文件路径，nCnt信号量数量，initVal信号量初始值，nProj
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_CREATE_FTOK  
     * CACHE_SEM_ERR_CREATE_PARA  
     * CACHE_SEM_ERR_CREATE_SEMGET
     * CACHE_SEM_ERR_CREATE_SEMCTL
     * 
     */
    int Create ( const char* pszPath, int nCnt=1, int initVal = 1, int nProj=1);
    
    int Create ( int nKey, int nCnt=1, int initVal = 1 );

    /* 
     * 功能：连接系统已经存在的信号量集
     * 输入：pszPath标志文件路径，nProj
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_ATTACH_FTOK  
     * CACHE_SEM_ERR_ATTACH_SEMGET
     * CACHE_SEM_ERR_ATTACH_NOINIT
     * 
     */
    int Attach ( const char* pszPath, int nProj=1);
    
    int Attach ( int nKey);
    
    /* 
     * 功能：释放已经连接的信号量集
     * 输入：
     * 输出：
     * 返回：0
     */
    int Release ();
    
    /* 
     * 功能：销毁已经连接的信号量集
     * 输入：
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_DESTROY_SEMCTL
     */
    int Destroy ();

    /* 
     * 功能：等待信号资源
     * 输入：nSemIdx信号量序号，bNoWait是否阻塞等待，bReverse是否先加再减
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_WAIT_NOINIT
     * CACHE_SEM_ERR_WAIT_PARA  
     * 
     */
    int Wait (int nSemIdx=0, bool bNoWait=false, bool bReverse = false);

    /* 
     * 功能：测试信号资源是否为0
     * 输入：nSemIdx信号量序号，bNoWait是否阻塞等待
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_TEST0_NOT0
     * CACHE_SEM_ERR_TEST0_PARA  
     * CACHE_SEM_ERR_TEST0_SEMOP 
     */
    int Test0 (int nSemIdx=0, bool bNoWait=false);

    /* 
     * 功能：给指定信号上锁
     * 输入：nSemIdx信号量序号，bNoWait是否阻塞等待
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_LOCK_NOINIT
     * CACHE_SEM_ERR_LOCK_PARA  
     * CACHE_SEM_ERR_LOCK_SEMOP 
     * 
     * 
     */
	int Lock ( int nSemIdx=0, bool bNoWait=false);

    /* 
     * 功能：给指定信号解锁
     * 输入：nSemIdx信号量序号，bNoWait是否阻塞等待
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_UNLOCK_NOINIT
     * CACHE_SEM_ERR_UNLOCK_PARA  
     * CACHE_SEM_ERR_UNLOCK_SEMOP 
     * 
     */
	int Unlock ( int nSemIdx=0, bool bNoWait=true);

    /* 
     * 功能：创建大计数器集
     * 输入：pszPath标志文件路径，nCounterNum大计数器的个数，nProj
     * 输出：
     * 返回：
     * CACHE_SEM_ERR_CREATECOUNTER_FTOK  
     * CACHE_SEM_ERR_CREATECOUNTER_PARA  
     * CACHE_SEM_ERR_CREATECOUNTER_SEMGET
     * CACHE_SEM_ERR_CREATECOUNTER_SEMCTL
     */
	int CreateCounter ( const char* pszPath, int nCounterNum = 1, int nProj=1);
	
	int CreateCounter (int nKey, int nCounterNum );
	
    /* 
     * 功能：连接已经存在的大计数器集
     * 输入：pszPath标志文件路径，nProj
     * 输出：
     * 返回：0
     * CACHE_SEM_ERR_ATTACHCOUNTER_FTOK  
     * CACHE_SEM_ERR_ATTACHCOUNTER_SEMGET
     * CACHE_SEM_ERR_ATTACHCOUNTER_NOINIT
     * 
     */
    int AttachCounter ( const char* pszPath, int nProj=1);
    
    int AttachCounter (int nKey);
    
    /* 
     * 功能：1）增加指定信号量资源
     *       2）为指定的大计数器+1
     * 输入：1）nSemIdx信号量序号，bNoWait是否阻塞等待，bUndo是否指定UNDO标记，val增加的值
     *       2）nSemIdx大计数器序号
     * 输出：
     * 返回：0
     * 1)
     * CACHE_SEM_ERR_INCREASE_NOINIT
     * CACHE_SEM_ERR_INCREASE_PARA  
     * CACHE_SEM_ERR_INCREASE_SEMOP 
     * 2)
     * CACHE_SEM_ERR_COUNTERINC_VAL   
     * CACHE_SEM_ERR_COUNTERINC_SEMOP 
     * CACHE_SEM_ERR_COUNTERINC_SEMOP2
     * CACHE_SEM_ERR_COUNTERINC_NOINIT
     * CACHE_SEM_ERR_COUNTERINC
     */
    int Inc ( int nSemIdx=0, bool bNoWait=true, bool bUndo = false, int val = 1);
    
    /* 
     * 功能：1）减少指定信号量资源
     *       2）为指定的大计数器-1
     * 输入：1）nSemIdx信号量序号，bNoWait是否阻塞等待，bUndo是否指定UNDO标记，val减少的值
     *       2）nSemIdx大计数器序号
     * 输出：
     * 返回：0
     * 1）
     * CACHE_SEM_ERR_DECREASE_NOINIT
     * CACHE_SEM_ERR_DECREASE_PARA  
     * CACHE_SEM_ERR_DECREASE_SEMOP 
     * 2）
     * CACHE_SEM_ERR_COUNTERDEC_NOINIT
     * CACHE_SEM_ERR_COUNTERDEC_PARA  
     * CACHE_SEM_ERR_COUNTERDEC_VAL   
     * CACHE_SEM_ERR_COUNTERDEC_SEMOP 
     * CACHE_SEM_ERR_COUNTERDEC_SEMOP1
     * CACHE_SEM_ERR_COUNTERDEC_SEMOP2
     * CACHE_SEM_ERR_COUNTERDEC_SEMOP3
     * CACHE_SEM_ERR_COUNTERDEC
     */
    int Dec ( int nSemIdx=0, bool bNoWait=false, bool bUndo = false, int val = 1);

    /* 
     * 功能：获取大计数器的当前值
     * 输入：
     * 输出：
     * 返回：大计数器的当前值
     */
    unsigned long long GetCounterVal (int nBCounterSeq ) ;
    
    /* 
     * 功能：获取大计数器的计数最大值
     * 输入：
     * 输出：
     * 返回：大计数器的计数最大值
     */
    inline int GetCounterMaxVal ( ) { return m_nMaxVal; };
    

public:
    inline int GetSemID ( ) { return m_nSemID; };
    
    int GetSemVal (int nSemIdx);
    
    int SetSemVal (int nSemIdx, ushort val);
    
    int GetAllSemVal (ushort* value, int nCnt);
    
    inline char* Strerr ( ) { return m_errmsg; };

protected:
    int m_nSemID;
    int m_nKey;
    int m_nSemCnt;
    bool m_bAttach;
    
    int m_nBCounterNum;            //BigCounter个数
    int m_nSCounterNum;            //每个BigCounter的SmallCounter数量
    int m_nMaxVal;                 //每个BigCounter的最大计数值
    
    int m_nSCounterSeq[MAX_BIGCOUNTER_NUM];    //每个BigCounter当前的SmallCounter序列
    int m_nGoNextSCounter[MAX_BIGCOUNTER_NUM];   //每个BigCounter的标记，1表示writer已经切换到下一行
    
    char m_errmsg[1024];
    
private:
    
    int Increase ( int nSemIdx=0, bool bNoWait=true, bool bUndo = false, int val = 1);
    
    int Decrease ( int nSemIdx=0, bool bNoWait=false, bool bUndo = false, int val = 1);
    
    int CounterInc (int nBCounterSeq = 0);
    
    int CounterDec (int nBCounterSeq = 0);
    
    int GetBigCounterNum();
    
    int GetCounterSWSemSeq(int nBCounterSeq);
    
    int GetCounterSemSeq(int nBCounterSeq);
    
    int GetCounterRecSemSeq(int nBCounterSeq);
};


/*
 * 用于限定锁的生命周期
 */
class Locker
{
public:
    Locker(CSemaphore &lock)
    {
        m_nSemIdx = -1;
        sem = &lock;
    }
    
    ~Locker()
    {
        if(sem != NULL)
            sem->Unlock(m_nSemIdx);
    }
    
    int Lock( int nSemIdx )
    {
        m_nSemIdx = nSemIdx;
        return sem->Lock(nSemIdx);
    }
    int Unlock()
    {
        int nRet = sem->Unlock(m_nSemIdx);
        sem = NULL;
        return nRet;
    }
    
    inline CSemaphore* GetSem()
    {
        return sem;
    }
private:
    int     m_nSemIdx;
    CSemaphore*    sem;
};
#endif // !defined(AFX_SEM_H__0D8E2B47_41E9_4A51_AF61_6CF375833E31__INCLUDED_)

