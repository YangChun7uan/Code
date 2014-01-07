#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*
信号量个数                  执行Lock和Unlock次数
32999       2   32818       98816
32998       2   32820       98816
32997       2   32822       98816       
32900       3   32884       131584      131584-98816=1*32768

32802       15   32770      524800
32801       15   32785      524800      0
32800       16   0          524800      0
32799       16   32784      557568      557568-524800=32768

32791       22   32774      754176      
32790       23   32774      786944      786944-754176=1*32768
32789       24   32776      819712      819712-786944=1*32768
32788       25   32780      852480      852480-819712=1*32768
32787       26   32786      885248      885248-852480=1*32768

32781       39   32773      1311232     
32780       42   32776      1409536     1409536-1311232=3*32768
32779       46   32774      1540608     1540608-1409536=4*32768
32778       51   32770      1704448     1704448-1540608=5*32768

32770            0          8389120     
32769            0          16777728    16777728-8389120=256*32768
32768            0          
32767 

*/


union semun {
     int val;
     struct semid_ds *buf;
     unsigned short *array;
     struct seminfo *__buf;
};

int main(int argc, char** argv)
{
    int nKey = ftok( "./main", 1);
    if ( nKey < 0 )
    {
        printf("ftok err:%s\n", strerror(errno));
        return -1;
    }
    
    int nNumber = 34000;
    int nSemID = semget ( nKey, nNumber,IPC_CREAT|0666 );
    if (nSemID < 0)
    {
        printf("semget err:%s\n", strerror(errno));
        return -1;
    } 
    
    union semun sem ;
    sem.val = 1; 
    sem.array = new ushort [nNumber];
    for ( int i = 0; i < nNumber; i++ )
        sem.array[i] = 1;
    
    int nRet = semctl (nSemID, nNumber, SETALL, sem);
    delete [] sem.array;
    if ( nRet < 0 )
    {        
        printf("semctl err:%s\n", strerror(errno));
        return -1;
    }
    
    int count = 0;
    for(int i = 0; i < nNumber; ++i)
    {
//        if(i == 33000)
//            i = 0;
        
        struct sembuf sops;
        sops.sem_num = i;
        sops.sem_flg = SEM_UNDO;
        
        sops.sem_op = -1;
        nRet =  semop(nSemID,&sops,1);
        if(nRet < 0)
        {
            printf("semop -1 err %s\n",strerror(errno));
            printf("i %d count %d\n", i, count);
            break;
        }
        
        sops.sem_op = 1;
        nRet =  semop(nSemID,&sops,1);
        if(nRet < 0)
        {
            printf("semop +1 err %s\n",strerror(errno));
            printf("i %d count %d\n", i, count);
            break;
        }
        
        ++count;
    }
    
    semctl ( nSemID, 0, IPC_RMID);
    
    return 0;
}
