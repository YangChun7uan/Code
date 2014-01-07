#include "Headdefine.h"
#include "utility.h"
#include "sem.h"
#include "shm.h"

int main(int argc, char** argv)
{
    daemon();
    
    CShareMem  shm ;
    
    int nRet = shm.Create("./tmp", 1, 1024);
    if(nRet < 0)
    {
        printf("shm create failed\n");
        return -1;
    }
    unsigned long long *pBuff = (unsigned long long *)shm.MemGet();
    memset(pBuff, 0, 1024);
    
    unsigned long long big = 18446744073709551615;
    unsigned long long zero = 0;
    unsigned long long one = 1023435;
    unsigned long long two = 451451;
    unsigned long long thr = 10234689435;
    char a[128] = "aaaaaaaaaa";
    char b[128] = "bbbbbbbbba";
    char c[128] = "ccccccccca";
    int len = sizeof(unsigned long long);
    
    printf("len %d\n", len);
    

    int pid = fork();
    if(pid == 0)
    {
        while(true)
        {
            unsigned long long tmp = 0;
            
            memcpy(pBuff, &big, len);
            //*pBuff = big;
            
            //tmp = *pBuff;
            memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%lu,%lx\n", tmp, tmp);
                break;
            }
        }
        return 0;
        
    }
    
    
    int pid2 = fork();
    if(pid2 == 0)
    {
        while(true)
        {
            unsigned long long tmp = 0;
            
            memcpy(pBuff, &one, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%lu,%lx\n", tmp, tmp);
                break;
            }
        }
        return 0;
        
    }    
    
    int pid3 = fork();
    if(pid3 == 0)
    {
        while(true)
        {
            unsigned long long tmp = 0;
            
            memcpy(pBuff, &two, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%lu,%lx\n", tmp, tmp);
                break;
            }
        }
        return 0;
        
    }
    
    int pid4 = fork();
    if(pid4 == 0)
    {
        while(true)
        {
            unsigned long long tmp = 0;
            
            memcpy(pBuff, &thr, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%lu,%lx\n", tmp, tmp);
                break;
            }
        }
        return 0;
        
    }
    
    while(true)
    {
        unsigned long long tmp = 0;
        
        memcpy(pBuff, &zero, len);
        //*pBuff = zero;
        
        //tmp = *pBuff;
        memcpy(&tmp, pBuff, len);
        if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
        //if( tmp != big && tmp != zero)
        {
            printf("memory err tmp:%lu,%lx\n", tmp, tmp);
            break;
        }
    }
    
    return 0;
}
