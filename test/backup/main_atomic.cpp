


#include "../Headdefine.h"
#include "../signalhandler.h"
#include <sys/shm.h>

#ifdef PPC405_ERRATA
#   define PPC405_ERR77_SYNC   "    sync\n"
#else
#   define PPC405_ERR77_SYNC
#endif

#ifdef LINUX
int atomic_cas32_i(volatile int *mem, int oldval, int newval)
{
    int prev;

    asm volatile ("lock; cmpxchgl %1, %2"
                  : "=a" (prev)
                  : "r" (newval), "m" (*(mem)), "0"(oldval)
                  : "memory", "cc");
    return prev;
}

char atomic_cas32_c(volatile char *mem, char oldval, char newval)
{
    char prev;

    asm volatile ("lock; cmpxchgb %1, %2"
                  : "=a" (prev)
                  : "r" (newval), "m" (*(mem)), "0"(oldval)
                  : "memory", "cc");
    return prev;
}

#else
int atomic_cas32_i(volatile int *mem, int oldval, int newval)
{
    int prev;

    __asm__ __volatile__("loop_%=:\n"                  /* lost reservation     */
                  " lwarx   %0,0,%1\n"      /* load and reserve     */
                  " cmpw    %0,%3\n"        /* compare operands     */
                  " bne-    exit_%=\n"      /* skip if not equal    */
                  PPC405_ERR77_SYNC             /* ppc405 Erratum 77    */
                  " stwcx.  %2,0,%1\n"      /* store new value      */
                  " bne-    loop_%=\n"      /* loop if lost         */
                  "exit_%=:\n"                  /* not equal            */
                  : "=&r" (prev)
                  : "b" (mem), "r" (newval), "r" (oldval)
                  : "cc", "memory");

    return prev;
}

void inline compare_and_swap (volatile int * p, int oldval, int newval)
{
    __asm__ __volatile__ ("lwarx %0, 0, %1 \n\t" : "=&r"(ret) : "r"(p));
}

char atomic_cas32_c(volatile char *mem, char oldval, char newval)
{
    return oldval;
}
#endif

void spin_lock(int *p)
{
    while( atomic_cas32_i(p, 0, 1) != 0 )
    {
    }
}


void spin_unlock(int *p)
{
    while( atomic_cas32_i(p, 1, 0) != 0 )
    {
    }
}


int main(int argc, char **argv)
{
//    int pid = fork();
//    if(pid > 0)
//    {
//        return 0;
//    }
//    setsid();
//    pid = fork();
//    if(pid > 0)
//    {
//        return 0;
//    }

    _RegisterSignal();

    int shm_id = shmget ( 111111, 16, IPC_CREAT|IPC_EXCL|0666 );
    if(shm_id < 0)
    {
        cout << "shmget error:" << strerror(errno) << endl;

        shm_id = shmget ( 111111, 0, S_IRUSR|S_IWUSR );
        if(shm_id < 0)
        {
            cout << "shmget error:" << strerror(errno) << endl;
            return -1;
        }
    }

    void *pdata = (void*) shmat ( shm_id, 0, 0 );
    if(pdata == NULL)
    {
        cout << "shmat error:" << strerror(errno) << endl;
        return -1;
    }

    int *flag = (int *)pdata;

    int i = 0;
    while(i++ < 10)
    {
        if( _HandleSignal() < 0 )
            goto destroy;

        spin_lock(flag);
        cout << "lock ok:  " << getpid() << endl;
        sleep(10000);

        spin_unlock(flag);
        cout << "unlock ok:" << getpid() << endl;
        sleep(10000);
    }

destroy:
    shmid_ds buf;
    memset ( &buf, 0, sizeof(buf) );
    if ( shmctl ( shm_id, IPC_RMID, &buf) < 0 )
    {
        cout << "shmctl error" << strerror(errno) << endl;
    }

    return 0;
}
