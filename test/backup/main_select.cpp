#include "Headdefine.h"
#include "signalhandler.h"
#include "utility.h"
#include "sem.h"

#define FIFO_SERVER "./tmp"

CSignal g_signal;

int clildfolw(int *fdsin, int fdsnum)
{
    printf("[%d] child start\n", getpid());

    int *fdsout = new int[fdsnum];
    while (true)
    {
        if (g_signal.HandleSignals() < 0)
            break;

        memset(fdsout, 0, sizeof(int)* fdsnum);
        int nRet = select_timeout(fdsin, fdsnum, fdsout, NULL);
        if (nRet <= 0)
        {
            printf("[%d] select_timeout failed\n", getpid());
            break;
        }

        for (int i = 0; i < nRet; i++)
        {
            if (fdsout[i] != 0)
            {
                printf("[%d] select fd OK %d\n", getpid(), fdsout[i]);
                
                char buff[128] = { 0 };
                int len = read(fdsout[i], buff, 1);
                if (len == 0)
                {
                    printf("[%d] socket close %s,%d\n", getpid(), strerror(errno),fdsout[i]);
                    delete fdsout;
                    return 0;
                }
                if (len < 0)
                {
                    printf("[%d] read failed %s,%d\n", getpid(), strerror(errno),fdsout[i]);
                    continue;
                }
                if (len == 1)
                    printf("[%d] read 1 byte :%s\n", getpid(), buff);

            }
        }
    }
    delete fdsout;
    return 0;
}

int main(int argc, char** argv)
{
    g_signal.RegisterSignals();

    int port = 7777;
    int g_servfd = listen_net(port);
    if (g_servfd < 0)
    {
        printf("listen_net failed %s\n", strerror(errno));
        return -1;
    }
    printf("listen_net start\n");
    
    int clifd1 = my_accept(g_servfd);
    if (clifd1 < 0)
    {
        printf("my_accept failed %s\n", strerror(errno));
        return -1;
    }
    set_fd_nonblock(clifd1);
    printf("my_accept clifd1 %d\n", clifd1);

    int clifd2 = my_accept(g_servfd);
    if (clifd2 < 0)
    {
        printf("my_accept failed %s\n", strerror(errno));
        return -1;
    }
    set_fd_nonblock(clifd2);
    printf("my_accept clifd2 %d\n", clifd2);
    
    int fds[2];
    fds[0] = clifd1;
    fds[1] = clifd2;

    int pid1 = fork();
    if (pid1 == 0) //子进程
    {
        clildfolw(fds, 2);

        return 0;

    }
    
    
    int pid2 = fork();
    if (pid2 == 0) //子进程
    {
        clildfolw(fds, 2);

        return 0;

    }
     
    
    sleep(1000);

    return 0;
}

