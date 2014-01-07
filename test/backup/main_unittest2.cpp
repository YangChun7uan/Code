#include "Headdefine.h"
#include "utility.h"
#include "sem.h"
#include "shm.h"

int main(int argc, char** argv)
{
    daemon();
    
    CShareMem  shm ;
    
    int nRet = shm.Create("./tmp2", 1, 1024);
    if(nRet < 0)
    {
        printf("shm create failed\n");
        return -1;
    }
    unsigned int *pBuff = (unsigned int *)shm.MemGet();
    memset(pBuff, 0, 1024);
    
    unsigned int big = 18446744073709551615;
    unsigned int zero = 0;
    unsigned int one = 1023435;
    unsigned int two = 451451;
    unsigned int thr = 6735;
    char a[128] = "aaaaaaaaaa";
    char b[128] = "bbbbbbbbba";
    char c[128] = "ccccccccca";
    int len = sizeof(unsigned int);
    
    printf("len %d\n", len);
    

    int pid = fork();
    if(pid == 0)
    {
        while(true)
        {
            unsigned int tmp = 0;
            
            memcpy(pBuff, &big, len);
            //*pBuff = big;
            
            //tmp = *pBuff;
            memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%u,%x\n", tmp, tmp);
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
            unsigned int tmp = 0;
            
            memcpy(pBuff, &one, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%u,%x\n", tmp, tmp);
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
            unsigned int tmp = 0;
            
            memcpy(pBuff, &two, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%u,%x\n", tmp, tmp);
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
            unsigned int tmp = 0;
            
            memcpy(pBuff, &thr, len);
            //*pBuff = one;
            
            tmp = *pBuff;
            //memcpy(&tmp, pBuff, len);
            if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
            //if( tmp != big && tmp != zero)
            {
                printf("memory err tmp:%u,%x\n", tmp, tmp);
                break;
            }
        }
        return 0;
        
    }
    
    while(true)
    {
        unsigned int tmp = 0;
        
        memcpy(pBuff, &zero, len);
        //*pBuff = zero;
        
        //tmp = *pBuff;
        memcpy(&tmp, pBuff, len);
        if( memcmp(&tmp, &big,len) != 0 && memcmp(&tmp, &zero,len) != 0 && memcmp(&tmp, &one,len) != 0 && memcmp(&tmp, &two,len) != 0 && memcmp(&tmp, &thr,len) != 0)
        //if( tmp != big && tmp != zero)
        {
            printf("memory err tmp:%u,%x\n", tmp, tmp);
            break;
        }
    }
    
    return 0;
}
