#include "Headdefine.h"
#include "sem.h"

#define FIFO_SERVER "./tmp"

int main(int argc, char** argv)
{
    CSem m_sem;
    int cnt = 10;
    int nRet = m_sem.Attach(FIFO_SERVER);
    if (nRet < 0)
    {
        printf("Sem::Create() failed:%s[%s:%d]\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    printf("---start---\n");

    for(int i = 0; i < m_sem.GetSemCnt; i++)
    {
        nRet = m_sem.GetSemVal(i);
        printf("---index %d, val--%d\n",i, nRet);
    }
    
    
    return 0;
}

