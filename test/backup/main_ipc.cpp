/*
 * 测试各种IPC消耗资源量
 *
 */

#include "Headdefine.h"
#include "signalhandler.h"
#include "Semaphore.h"
#include "ShareMemory.h"
#include "Socket.h"
#include "utility.h"
//#include "../r6code/cache/cache_api.h"


#define CLIENT_NUM 10
#define MAX_FD     512

char g_ip[128] = {0};
int g_len = 128;
int g_usleep = 5000;
int g_verify = 0;
int g_port = 7623;
char g_sKeyFile[128] = {0};
unsigned long long  loop_times = 0;

int* g_nSig = 0;

char wbuff[1024 * 1024] = { 0 };
char rbuff[1024 * 1024] = { 0 };

int nRet;

struct data
{
public:
    data()
    {
        for(int i = 0; i < 256; ++i)
        {
            a[i] = i;
        }
    }

    int compare()
    {
        for(int i = 0; i < 256; ++i)
        {
            if( a[i] != i)
            {
                printf("data failed\n");fflush(NULL);
                return -1;
            }
        }
        return 0;
    }

    long long a[256];
};
struct data g_data;

struct timeval time_b, time_a, time_c, time_d, time_e, time_f;
unsigned long long g_count0 = 0;
unsigned long long g_count1 = 0;
unsigned long long g_count2 = 0;
unsigned long long total_time0 = 0;
unsigned long long total_time1 = 0;
unsigned long long total_time2 = 0;
unsigned long long recv_delay_count0 = 0;
unsigned long long recv_delay0 = 0;
unsigned long long g_delay_max = 0;

int Rqueue();
int Wqueue();
int Rsem();
int Wsem();
int Rfifo();
int Wfifo();
int Rsock();
int Wsock();
int Rusock();
int Wusock();
int Queue();
int Sem();
int Fifo();
int Sock();
int Usock();
int Rpoll();
int Rpoll2();
int Rpollset();
#ifdef OCE_CACHE
int Rcache();
int Wcache();
int Cache_();
#endif

int main(int argc, char** argv)
{
    //获取程序启动参数
    int optch = 0;
    static char optstring[] = "o:l:s:vi:t:p:k:";
    extern char *optarg;

    char smode[128] = { 0 };
    int mode = -1;
    while ((optch = getopt(argc, argv, optstring)) != -1)
    {
        switch (optch)
        {
        case 'o'://启动模式
            strcpy(smode, optarg);
            if (strcmp(optarg, "rqueue") == 0)
                mode = 0;
            if (strcmp(optarg, "wqueue") == 0)
                mode = 1;
            if (strcmp(optarg, "queue") == 0)
                mode = 10;
            if (strcmp(optarg, "rsem") == 0)
                mode = 2;
            if (strcmp(optarg, "wsem") == 0)
                mode = 3;
            if (strcmp(optarg, "sem") == 0)
                mode = 11;
            if (strcmp(optarg, "rfifo") == 0)
                mode = 4;
            if (strcmp(optarg, "wfifo") == 0)
                mode = 5;
            if (strcmp(optarg, "fifo") == 0)
                mode = 12;
            if (strcmp(optarg, "rsock") == 0)
                mode = 6;
            if (strcmp(optarg, "wsock") == 0)
                mode = 7;
            if (strcmp(optarg, "sock") == 0)
                mode = 13;
            if (strcmp(optarg, "rusock") == 0)
                mode = 8;
            if (strcmp(optarg, "wusock") == 0)
                mode = 9;
            if (strcmp(optarg, "usock") == 0)
                mode = 14;
            if (strcmp(optarg, "rpoll") == 0)
                mode = 15;
            if (strcmp(optarg, "rpoll2") == 0)
                mode = 16;
            if (strcmp(optarg, "rpollset") == 0)
                mode = 17;
#ifdef OCE_CACHE
            if (strcmp(optarg, "rcache") == 0)
                mode = 18;
            if (strcmp(optarg, "wcache") == 0)
                mode = 19;
            if (strcmp(optarg, "cache") == 0)
                mode = 20;
#endif
            break;
        case 'l':
            g_len = atoi(optarg);
            break;
        case 's':
            g_usleep = atoi(optarg);
            break;
        case 'v':
            g_verify = 1;
            break;
        case 'i':
            strcpy(g_ip, optarg);
            break;
        case 't':
            loop_times = atol(optarg);
            break;
        case 'p':
            g_port = atol(optarg);
            break;
        case 'k':
            strcpy(g_sKeyFile, optarg);
            break;
        default:
            exit(-1);
        }
    }

    daemon();

    if(g_verify == 1)
    {
        memcpy(wbuff+sizeof(struct timeval), &g_data, sizeof(g_data));
        g_len = sizeof(struct timeval) + sizeof(g_data);
    }
    else
    {
        for (int i = 0; i < sizeof(wbuff); i++)
        {
            wbuff[i] = 'b';
        }
        wbuff[0] = 'a';
        wbuff[g_len-1] = 'a';
        wbuff[sizeof(wbuff) - 1] = '\0';
    }

    g_nSig = _RegisterSignal();
    if(g_nSig == NULL)
    {
        printf("Could not set up signal handling.  Aborting.\n");
        return -1;
    }

    if (mode == 0)
    {
        Rqueue();
    }
    if (mode == 1)
    {
        Wqueue();
    }
    if (mode == 2)
    {
        Rsem();
    }
    if (mode == 3)
    {
        Wsem();
    }
    if (mode == 4)
    {
        Rfifo();
    }
    if (mode == 5)
    {
        Wfifo();
    }
    if (mode == 6)
    {
        Rsock();
    }
    if (mode == 7)
    {
        Wsock();
    }
    if (mode == 8)
    {
        Rusock();
    }
    if (mode == 9)
    {
        Wusock();
    }
    if (mode == 10)
    {
        Queue();
    }
    if (mode == 11)
    {
        Sem();
    }
    if (mode == 12)
    {
        Fifo();
    }
    if (mode == 13)
    {
        Sock();
    }
    if (mode == 14)
    {
        Usock();
    }
    if (mode == 15)
    {
        Rpoll();
    }
    if (mode == 16)
    {
        Rpoll2();
    }
    if (mode == 17)
    {
        Rpollset();
    }
#ifdef OCE_CACHE
    if (mode == 18)
    {
        Rcache();
    }
    if (mode == 19)
    {
        Wcache();
    }
    if (mode == 20)
    {
        Cache_();
    }
#endif

    if(total_time0 != 0)
    {
        double avg1 = (double) total_time0 / (double) g_count0;
        double avg2 = (double) g_count0 / (double)((double) total_time0 / 1000000);
        printf("%s total0 g_count0:%lld, avgage %lf us, max %lf num/s\n", smode, g_count0, avg1,avg2);
    }
    if(recv_delay0 != 0)
    {
        double avg1 = (double) recv_delay0 / (double) recv_delay_count0;
        printf("%s recv delay avgage %lf us , max delay %llu\n", smode, avg1, g_delay_max);
    }
    if(total_time1 != 0)
    {
        double avg1 = (double) total_time1 / (double) g_count1;
        double avg2 = (double) g_count1 / (double)((double) total_time1 / 1000000);
        printf("%s total1 g_count1:%lld, avgage %lf us, max %lf num/s\n", smode, g_count1, avg1,avg2);
    }
    if(total_time2 != 0)
    {
        double avg1 = (double) total_time2 / (double) g_count2;
        double avg2 = (double) g_count2 / (double)((double) total_time2 / 1000000);
        printf("%s total2 g_count2:%lld, avgage %lf us, max %lf num/s\n", smode, g_count2, avg1,avg2);
    }

    printf("exit.\n");
    return 0;
}

int Rqueue()
{
    int msgid = conn_q("./main");
    if (msgid < 0)
    {
        printf("connq err\n");
        return -1;
    }

    struct msgform msg;
    memset(&msg, 0, sizeof(msg));

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0);
        gettimeofday(&time_b, NULL);
        struct timeval *time_w = (struct timeval *)(msg.mtext);

        if (nRet < 0)
        {
            printf("msgrcv fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        if(g_verify == 1)
        {
            struct data* d = (struct data* )(msg.mtext+sizeof(struct timeval));
            if( d->compare() < 0)
            {
                printf("format error %s\n");fflush(NULL);
            }
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        unsigned long long delay_time = (time_b.tv_sec - time_w->tv_sec) * 1000000 + (time_b.tv_usec - time_w->tv_usec);
        if(delay_time < 10000)
        {
            if(delay_time > g_delay_max)
                g_delay_max = delay_time;
            recv_delay_count0++;
            recv_delay0 += delay_time;
        }
    }


    return 0;
}
int Wqueue()
{
    int msgid = conn_q("./main");
    if (msgid < 0)
    {
        printf("connq err\n");
        return -1;
    }

    struct msgform msg;
    memset(&msg, 0, sizeof(msg));
    msg.mtype = 1;
    memcpy(msg.mtext, wbuff, g_len);
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        memcpy(msg.mtext, &time_a, sizeof(time_a));
        nRet = msgsnd(msgid, &msg, g_len, IPC_NOWAIT);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("msgsnd fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }

    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);

    return 0;
}
int Rsem()
{
    CSemaphore m_sem;
    int cnt = 1;
    int m_nSemID = m_sem.Create("./main", 1, 0);
    if (m_nSemID < 0)
    {
        printf("%s\n", m_sem.Strerr());
        return -1;
    }
    m_sem.SetSemVal(0, 0);

    struct sembuf sops[1];
    sops[0].sem_num = 0;
    sops[0].sem_op = -g_len;
    sops[0].sem_flg = 0;

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = semop(m_nSemID, sops, 1);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("semop fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;
    }

    return 0;
}
int Wsem()
{

    CSemaphore m_sem;
    int m_nSemID = m_sem.Attach("./main");
    if (m_nSemID < 0)
    {
        printf("%s\n", m_sem.Strerr());
        return -1;
    }

    struct sembuf sops[1];
    sops[0].sem_num = 0;
    sops[0].sem_op = g_len;
    sops[0].sem_flg = IPC_NOWAIT;
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = semop(m_nSemID, sops, 1);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("semop fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }
    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);

    return 0;
}
int Rfifo()
{
    unlink("./FIFO");
    nRet = mkfifo("./FIFO", 0777);
    if (nRet < 0)
    {
        printf("mkfifo failed:%s \n", strerror(errno));
        return -1;
    }

    int fd = open("./FIFO", O_RDONLY);
    if (fd == -1)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = read(fd, rbuff, g_len);
        gettimeofday(&time_b, NULL);
        struct timeval *time_w = (struct timeval *)(rbuff);

        if (nRet != g_len)
        {
            printf("read fialed nRet %d %s\n", nRet, strerror(errno));
            break;
        }
        g_count0++;

        if(g_verify == 1)
        {
            struct data* d = (struct data* )(rbuff+sizeof(struct timeval));
            if( d->compare() < 0)
            {
                printf("format error %s\n");fflush(NULL);
            }
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        unsigned long long delay_time = (time_b.tv_sec - time_w->tv_sec) * 1000000 + (time_b.tv_usec - time_w->tv_usec);
        if(delay_time < 10000)
        {
            if(delay_time > g_delay_max)
                g_delay_max = delay_time;
            recv_delay_count0++;
            recv_delay0 += delay_time;
        }
    }

    return 0;
}
int Wfifo()
{

    int fd = open("./FIFO", O_WRONLY);
    if (fd == -1)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        memcpy(wbuff, &time_a, sizeof(time_a));
        nRet = write(fd, wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet != g_len)
        {
            printf("write fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }
    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);

    return 0;
}
int Rsock()
{
    int sfd = listen_net(g_port);
    if (sfd < 0)
    {
        printf("listen_net failed:%s \n", strerror(errno));
        return -1;
    }

    int fd = my_accept(sfd);
    if (fd < 0)
    {
        printf("my_accept failed:%s \n", strerror(errno));
        return -1;
    }

    int buff = 1024*1024;
    socklen_t len = sizeof(int);
    if(setsockopt(fd , SOL_SOCKET,SO_RCVBUF , &buff , len) == -1)
    {
        printf("setsockopt SO_RCVBUF failed:%s\n", strerror(errno));
    }
    if(setsockopt(fd , SOL_SOCKET,SO_SNDBUF , &buff , len) == -1)
    {
        printf("setsockopt SO_SNDBUF failed:%s\n", strerror(errno));
    }

    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = read(fd, rbuff, g_len);
        //nRet = readn(fd, rbuff, g_len, 0);
        gettimeofday(&time_b, NULL);
        struct timeval *time_w = (struct timeval *)(rbuff);

        if (nRet != g_len)
        {
            printf("read fialed nRet %d %s\n", nRet, strerror(errno));
            break;
        }
        g_count0++;

        if(g_verify == 1)
        {
            struct data* d = (struct data* )(rbuff+sizeof(struct timeval));
            if( d->compare() < 0)
            {
                printf("format error %s\n");fflush(NULL);
            }
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        unsigned long long delay_time = (time_b.tv_sec - time_w->tv_sec) * 1000000 + (time_b.tv_usec - time_w->tv_usec);
        if(delay_time < 10000)
        {
            if(delay_time > g_delay_max)
                g_delay_max = delay_time;
            recv_delay_count0++;
            recv_delay0 += delay_time;
        }
    }
    gettimeofday(&time_d, NULL);


    unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
    total_time2 += use_time2;
    g_count2 = g_count0;

    return 0;
}
int Wsock()
{
    int fd = ConnectServer(g_ip, g_port);
    if (fd < 0)
    {
        printf("ConnectServer failed:%s \n", strerror(errno));
        return -1;
    }

    int buff = 1024*1024;
    socklen_t len = sizeof(int);
    if(setsockopt(fd , SOL_SOCKET,SO_RCVBUF , &buff , len) == -1)
    {
        printf("setsockopt SO_RCVBUF failed:%s\n", strerror(errno));
    }
    if(setsockopt(fd , SOL_SOCKET,SO_SNDBUF , &buff , len) == -1)
    {
        printf("setsockopt SO_SNDBUF failed:%s\n", strerror(errno));
    }
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        memcpy(wbuff, &time_a, sizeof(time_a));
        nRet = write(fd, wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet != g_len)
        {
            printf("write fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }

    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);

//    int pid[10] = {0};
//    for(int i = 0; i< 10; i++)
//    {
//        pid[i] = fork();
//        if(pid[i] == 0)
//        {
//            while (true)
//            {
//                if (_HandleSignal() < 0)
//                    break;
//
//                gettimeofday(&time_a, NULL);
//                nRet = write(fd, wbuff, g_len);
//                gettimeofday(&time_b, NULL);
//
//                if (nRet != g_len)
//                {
//                    printf("write fialed %s\n", strerror(errno));
//                    return -1;
//                }
//                g_count0++;
//
//                unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
//                total_time0 += use_time0;
//
//                if(g_usleep > 0)
//                    usleep(g_usleep);
//            }
//            return 0;
//        }
//    }
//    printf("father exit\n");


    return 0;
}

int Rusock()
{
    unlink("./USOCK");
    CSocket sock;
    nRet = sock.Listen("./USOCK");
    if (nRet < 0)
    {
        printf("Listen failed:%s \n", strerror(errno));
        return -1;
    }

    int fd = sock.Accept();
    if (nRet < 0)
    {
        printf("Accept failed:%s \n", strerror(errno));
        return -1;
    }

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = read(fd, rbuff, g_len);
        gettimeofday(&time_b, NULL);
        struct timeval *time_w = (struct timeval *)(rbuff);

        if (nRet != g_len)
        {
            printf("read fialed nRet %d %s\n", nRet, strerror(errno));
            break;
        }
        g_count0++;

        if(g_verify == 1)
        {
            struct data* d = (struct data* )(rbuff+sizeof(struct timeval));
            if( d->compare() < 0)
            {
                printf("format error %s\n");fflush(NULL);
            }
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        unsigned long long delay_time = (time_b.tv_sec - time_w->tv_sec) * 1000000 + (time_b.tv_usec - time_w->tv_usec);
        if(delay_time < 10000)
        {
            if(delay_time > g_delay_max)
                g_delay_max = delay_time;
            recv_delay_count0++;
            recv_delay0 += delay_time;
        }
    }

    return 0;
}
int Wusock()
{
    CSocket sock;
    int fd = sock.Connect("./USOCK");
    if (fd < 0)
    {
        printf("Connect failed:%s \n", strerror(errno));
        return -1;
    }
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        memcpy(wbuff, &time_a, sizeof(time_a));
        nRet = write(fd, wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet != g_len)
        {
            printf("write fialed %s\n", strerror(errno));
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }
    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);
    return 0;
}

#ifdef OCE_CACHE
int Rcache()
{
    Cache  cache;

    int nRet = cache.Attach(g_sKeyFile, READER);
    if(nRet < 0)
    {
        printf("%s\n", cache.Strerr() );
        return -1;
    }
    printf("Attach success %d\n", nRet);
    if( cache.Register( 0 ) < 0)
        return -1;

    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = cache.Read(rbuff, sizeof(rbuff), g_nSig);
        gettimeofday(&time_b, NULL);
        struct timeval *time_w = (struct timeval *)(rbuff);

        if(nRet == 0)
        {
            printf("writer exit\n");
            continue;
        }
        if (nRet != g_len)
        {
            printf("read fialed nRet %d %s\n", nRet, cache.Strerr());
            break;
        }
        g_count0++;


        if(g_verify == 1)
        {
            struct data* d = (struct data* )(rbuff+sizeof(struct timeval));
            if( d->compare() < 0)
            {
                printf("format error %s\n");fflush(NULL);
            }
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        unsigned long long delay_time = (time_b.tv_sec - time_w->tv_sec) * 1000000 + (time_b.tv_usec - time_w->tv_usec);
        if(delay_time < 10000)
        {
            if(delay_time > g_delay_max)
                g_delay_max = delay_time;
            recv_delay_count0++;
            recv_delay0 += delay_time;
        }
    }
    return 0;
}
int Wcache()
{
    Cache cache;

    int nRet = cache.Attach(g_sKeyFile, WRITER);
    if (nRet < 0)
    {
        printf("%s\n", cache.Strerr());
        return -1;
    }

    printf("Attach success %d\n", nRet);
    nRet = cache.Register();
    if (nRet < 0)
    {
        printf("%s\n", cache.Strerr());
        return -1;
    }
    printf("Register  OK %d\n", nRet);

    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        memcpy(wbuff, &time_a, sizeof(time_a));
        nRet = cache.Write(wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet != g_len)
        {
            printf("write fialed %s\n", cache.Strerr());
            return -1;
        }
        g_count0++;

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;

        if(g_usleep > 0)
            usleep(g_usleep);
    }
    double time = (time_b.tv_sec-time_c.tv_sec)*1000000 + (time_b.tv_usec - time_c.tv_usec);
    double speed = (double)g_count0/time;
    printf("speed:%lf unit/s \n", speed*1000000);

    return 0;
}

#endif

int Queue()
{

    int msgid = conn_q("./main");
    if (msgid < 0)
    {
        printf("connq err\n");
        return -1;
    }

    struct msgform read;
    memset(&read, 0, sizeof(read));


    struct msgform write;
    memset(&write, 0, sizeof(write));
    write.mtype = 1;
    memcpy(write.mtext, wbuff, g_len);

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = msgsnd(msgid, &write, g_len, IPC_NOWAIT);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = msgrcv(msgid, &read, sizeof(read.mtext), 0, 0);
        gettimeofday(&time_d, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;
        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;
    }

    return 0;
}

#ifdef OCE_CACHE
int Cache_()
{
    Cache cachew;
    Cache cacher;

    int nRet = cachew.Attach(g_sKeyFile, WRITER);
    if (nRet < 0)
    {
        printf("%s\n", cachew.Strerr());
        return -1;
    }
    nRet = cacher.Attach(g_sKeyFile, READER);
    if (nRet < 0)
    {
        printf("%s\n", cacher.Strerr());
        return -1;
    }

    printf("Attach success %d\n", nRet);
    nRet = cachew.Register();
    if (nRet < 0)
    {
        printf("%s\n", cachew.Strerr());
        return -1;
    }
    nRet = cacher.Register();
    if (nRet < 0)
    {
        printf("%s\n", cacher.Strerr());
        return -1;
    }
    printf("Register  OK %d\n", nRet);

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = cachew.Write(wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet != g_len)
        {
            printf("fialed %s\n", cachew.Strerr());
            break;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = cacher.Read(rbuff, sizeof(rbuff), g_nSig);
        gettimeofday(&time_d, NULL);

        if (nRet != g_len)
        {
            printf("fialed %s\n", cacher.Strerr());
            break;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;
        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;
    }

    return 0;
}
#endif

//int Sem()
//{
//    CSem m_sem;
//    int cnt = 1;
//    int m_nSemID = m_sem.Create("./main", 1, 0);
//    if (m_nSemID < 0)
//    {
//        printf("%s\n", m_sem.Strerr());
//        return -1;
//    }
//    m_sem.SetSemVal(0, 0);
//
//    struct sembuf read[1];
//    read[0].sem_num = 0;
//    read[0].sem_op = -g_len;
//    read[0].sem_flg = 0;
//
//    struct sembuf write[1];
//    write[0].sem_num = 0;
//    write[0].sem_op = g_len;
//    write[0].sem_flg = IPC_NOWAIT;
//
//    while (true)
//    {
//        if (_HandleSignal() < 0)
//            break;
//
//        gettimeofday(&time_a, NULL);
//        nRet = semop(m_nSemID, write, 1);
//        gettimeofday(&time_b, NULL);
//
//        if (nRet < 0)
//        {
//            printf("fialed %s\n", strerror(errno));
//            return -1;
//        }
//
//        usleep(g_usleep);
//
//        gettimeofday(&time_c, NULL);
//        nRet = semop(m_nSemID, read, 1);
//        gettimeofday(&time_d, NULL);
//
//        if (nRet < 0)
//        {
//            printf("fialed %s\n", strerror(errno));
//            return -1;
//        }
//
//        g_count0++;
//
//        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
//        total_time1 += use_time1;
//
//        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
//        total_time2 += use_time2;
//    }
//    return 0;
//}
int Sem()
{
    CSemaphore m_sem;
    int cnt = 1;
    int m_nSemID = m_sem.Create("./main2", 2, 0);
    if (m_nSemID < 0)
    {
        printf("%s\n", m_sem.Strerr());
        return -1;
    }
    m_sem.SetSemVal(0, 0);

    struct sembuf read[2];
    read[0].sem_num = 0;
    read[0].sem_op = -g_len;
    read[0].sem_flg = 0;
    read[1].sem_num = 1;
    read[1].sem_op =  0;
    read[1].sem_flg = 0;

    struct sembuf write[2];
    write[0].sem_num = 0;
    write[0].sem_op = g_len;
    write[0].sem_flg = IPC_NOWAIT;
    write[1].sem_num = 1;
    write[1].sem_op = 0;
    write[1].sem_flg = IPC_NOWAIT;

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = semop(m_nSemID, write, 1);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = semop(m_nSemID, read, 1);
        gettimeofday(&time_d, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;

        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;
    }
    return 0;
}
int Fifo()
{
    unlink("./FIFO");
    nRet = mkfifo("./FIFO", 0777);
    if (nRet < 0)
    {
        printf("mkfifo failed:%s \n", strerror(errno));
        return -1;
    }

    int rfd = open("./FIFO", O_RDWR);
    if (rfd < 0)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }

    int wfd = open("./FIFO", O_RDWR);
    if (wfd < 0)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = write(wfd, wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = read(rfd, rbuff, g_len);
        gettimeofday(&time_d, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;

        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;

    }
    return 0;
}

int g_sfd = 0;
int g_afd = 0;
int g_cfd = 0;
void *thread_function(void *arg)
{
    g_cfd = ConnectServer(g_ip, g_port);
    if (g_cfd < 0)
    {
        printf("ConnectServer failed:%s \n", strerror(errno));
        return NULL;
    }
    return NULL;
}
int Sock()
{
    pthread_t mythread;
    nRet = pthread_create(&mythread, NULL, thread_function, NULL);
    if ( nRet < 0 )
    {
        printf("error creating thread.");
        abort();
    }

    g_sfd = listen_net(g_port);
    if (g_sfd < 0)
    {
        printf("listen_net failed:%s \n", strerror(errno));
        return -1;
    }

    g_afd = my_accept(g_sfd);
    if (g_afd < 0)
    {
        printf("my_accept failed:%s \n", strerror(errno));
        return -1;
    }
    close(g_sfd);
    g_sfd = 0;

    nRet = pthread_join(mythread, NULL);
    if ( nRet < 0)
    {
        printf("error joining thread.");
        abort();
    }

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = write(g_afd, wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = read(g_cfd, rbuff, g_len);
        gettimeofday(&time_d, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;

        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;

    }
    return 0;
}
int Usock()
{
    int  sockfd[2] = {0};
    nRet = socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);
    if(nRet != 0)
    {
        printf("socketpair fialed %s\n", strerror(errno));
        return -1;
    }

    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        gettimeofday(&time_a, NULL);
        nRet = write(sockfd[0], wbuff, g_len);
        gettimeofday(&time_b, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        if(g_usleep > 0)
            usleep(g_usleep);

        gettimeofday(&time_c, NULL);
        nRet = read(sockfd[1], rbuff, g_len);
        gettimeofday(&time_d, NULL);

        if (nRet < 0)
        {
            printf("fialed %s\n", strerror(errno));
            return -1;
        }

        g_count1++;
        g_count2++;

        unsigned long long use_time1 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time1 += use_time1;

        unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
        total_time2 += use_time2;

    }

    return 0;
}
int Rpoll()
{
    int servfd = listen_net(g_port);
    if (servfd < 0)
    {
        printf("listen_net failed:%s \n", strerror(errno));
        return -1;
    }

    struct pollfd fds[CLIENT_NUM];

    for(int i = 0; i< CLIENT_NUM; i++)
    {
        fds[i].fd = my_accept(servfd);
        if (fds[i].fd < 0)
        {
            printf("my_accept failed %s\n", strerror(errno));
            return -1;
        }
        int buff = 1024*1024;
        socklen_t len = sizeof(int);
        if(setsockopt(fds[i].fd , SOL_SOCKET,SO_RCVBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_RCVBUF failed:%s\n", strerror(errno));
        }
        if(setsockopt(fds[i].fd , SOL_SOCKET,SO_SNDBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_SNDBUF failed:%s\n", strerror(errno));
        }
    }
    for(int i = 0; i< CLIENT_NUM; i++)
    {
        fds[i].events = POLLIN;
        printf("my_accept clifd[%d] %d\n", i , fds[i].fd);
    }

    int conn = CLIENT_NUM;
    int nRet = 0;
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        if (conn == 0)
        {
            printf("no fd poll\n");
            break;
        }
        if (g_count2 >= loop_times && loop_times != 0)
        {
            printf("finish\n");
            break;
        }
        gettimeofday(&time_a, NULL);
        nRet = poll(fds, CLIENT_NUM, -1);
        gettimeofday(&time_b, NULL);
        if (nRet < 0)
        {
            printf("poll failed %s\n", strerror(errno));
            break;
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;
        g_count0++;

        for(int i = 0; i< CLIENT_NUM; i++)
        {
            if(fds[i].revents & (POLLIN | POLLERR))
            {
                gettimeofday(&time_e, NULL);
                nRet = readn(fds[i].fd, rbuff, g_len, 0);
                gettimeofday(&time_f, NULL);
                if (nRet == 0)
                {
                    printf("%d fd %d close\n", i, fds[i].fd);
                    fds[i].fd = -1;
                    conn -- ;
                    break;
                }
                if (nRet != g_len)
                {
                    printf("readn fialed nRet %d %s\n", nRet, strerror(errno));
                    break;
                }

                if(g_verify == 1)
                {
                    struct data* d = (struct data* )(rbuff+sizeof(struct timeval));
                    if( d->compare() < 0)
                    {
                        printf("format error %s\n");fflush(NULL);
                    }
                }

                unsigned long long use_time1 = (time_f.tv_sec - time_e.tv_sec) * 1000000 + (time_f.tv_usec - time_e.tv_usec);
                total_time1 += use_time1;
                g_count1++;
                g_count2++;
            }
        }

    }
    gettimeofday(&time_d, NULL);


    unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
    total_time2 += use_time2;

    return 0;
}
int Rpoll2()
{
    int servfd = listen_net(g_port);
    if (servfd < 0)
    {
        printf("listen_net failed:%s \n", strerror(errno));
        return -1;
    }

    struct pollfd fds[CLIENT_NUM];

    for(int i = 0; i< CLIENT_NUM; i++)
    {
        fds[i].fd = my_accept(servfd);
        if (fds[i].fd < 0)
        {
            printf("my_accept failed %s\n", strerror(errno));
            return -1;
        }
        int buff = 1024*1024;
        socklen_t len = sizeof(int);
        if(setsockopt(fds[i].fd , SOL_SOCKET,SO_RCVBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_RCVBUF failed:%s\n", strerror(errno));
        }
        if(setsockopt(fds[i].fd , SOL_SOCKET,SO_SNDBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_SNDBUF failed:%s\n", strerror(errno));
        }
    }
    for(int i = 0; i< CLIENT_NUM; i++)
    {
        fds[i].events = POLLIN;
        printf("my_accept clifd[%d] %d\n", i , fds[i].fd);
    }

    char  buff[CLIENT_NUM][1024];
    int   readlen[CLIENT_NUM];
    memset(buff, 0, CLIENT_NUM*1024);
    memset(readlen, 0, sizeof(readlen));

    int conn = CLIENT_NUM;
    int nRet = 0;
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        if (conn == 0)
        {
            printf("no fd poll\n");
            break;
        }
        if (g_count2 >= loop_times  && loop_times != 0)
        {
            printf("finish\n");
            break;
        }
        gettimeofday(&time_a, NULL);
        nRet = poll(fds, CLIENT_NUM, -1);
        gettimeofday(&time_b, NULL);
        if (nRet < 0)
        {
            printf("poll failed %s\n", strerror(errno));
            break;
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;
        g_count0++;

        for(int i = 0; i< CLIENT_NUM; i++)
        {
            if(fds[i].revents & (POLLIN | POLLERR))
            {
                gettimeofday(&time_e, NULL);
                nRet = read(fds[i].fd, buff[i]+readlen[i], g_len-readlen[i]);
                gettimeofday(&time_f, NULL);
                if (nRet == 0)
                {
                    printf("%d fd %d close\n", i, fds[i].fd);
                    fds[i].fd = -1;
                    conn -- ;
                    break;
                }
                if (nRet < 0)
                {
                    printf("%d fd %d err %s \n", i, fds[i].fd,strerror(errno));
                    fds[i].fd = -1;
                    conn -- ;
                    break;
                }

                unsigned long long use_time1 = (time_f.tv_sec - time_e.tv_sec) * 1000000 + (time_f.tv_usec - time_e.tv_usec);
                total_time1 += use_time1;
                g_count1++;

                if (nRet != g_len-readlen[i])//没读完
                {
                    readlen[i] += nRet;
                }
                else if (nRet == g_len-readlen[i])//读完
                {
                    if(g_verify == 1 && buff[i][0] != 'a')
                    {
                        printf("format error %s\n", buff[i]);
                        return -1;
                    }
                    readlen[i] = 0;
                    g_count2++;
                    memcpy(rbuff, buff[i], sizeof(buff[i]));
                    memset(buff[i], 0, sizeof(buff[i]));
                }

            }
        }

    }
    gettimeofday(&time_d, NULL);


    unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
    total_time2 += use_time2;

    return 0;
}

int Rpollset()
{
#ifdef  AIX
    int servfd = listen_net(g_port);
    if (servfd < 0)
    {
        printf("listen_net failed:%s \n", strerror(errno));
        return -1;
    }

    struct poll_ctl  pollset[CLIENT_NUM];

    for(int i = 0; i< CLIENT_NUM; i++)
    {
        pollset[i].fd = my_accept(servfd);
        if (pollset[i].fd < 0)
        {
            printf("my_accept failed %s\n", strerror(errno));
            return -1;
        }
        int buff = 1024*1024;
        socklen_t len = sizeof(int);
        if(setsockopt(pollset[i].fd , SOL_SOCKET,SO_RCVBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_RCVBUF failed:%s\n", strerror(errno));
        }
        if(setsockopt(pollset[i].fd , SOL_SOCKET,SO_SNDBUF , &buff , len) == -1)
        {
            printf("setsockopt SO_SNDBUF failed:%s\n", strerror(errno));
        }
    }

    pollset_t ps = pollset_create(-1);
    if(ps < 0)
    {
        printf("pollset_create failed %s\n",strerror(errno));
        return -1;
    }

    for(int i = 0; i< CLIENT_NUM; i++)
    {
        pollset[i].events = POLLIN;
        pollset[i].cmd = PS_ADD;
        printf("my_accept clifd[%d] %d\n", i , pollset[i].fd);
    }

    nRet = pollset_ctl(ps, pollset, CLIENT_NUM);
    if(nRet < 0)
    {
        printf("pollset_ctl failed %s\n",strerror(errno));
        return -1;
    }

    char  buff[MAX_FD][1024];
    int   readlen[MAX_FD];
    memset(buff, 0, MAX_FD*1024);
    memset(readlen, 0, sizeof(readlen));

    struct pollfd fds[CLIENT_NUM];
    memset(fds, 0, sizeof(fds));
    int conn = CLIENT_NUM;
    int nRet = 0;
    gettimeofday(&time_c, NULL);
    while (true)
    {
        if (_HandleSignal() < 0)
            break;

        if (conn == 0)
        {
            printf("no fd poll\n");
            break;
        }
        if (g_count2 >= loop_times && loop_times != 0)
        {
            printf("finish\n");
            break;
        }
        gettimeofday(&time_a, NULL);
        nRet = pollset_poll(ps, fds, CLIENT_NUM, -1);
        gettimeofday(&time_b, NULL);
        if (nRet < 0)
        {
            printf("poll failed %s\n", strerror(errno));
            break;
        }

        unsigned long long use_time0 = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
        total_time0 += use_time0;
        g_count0++;

        for(int i = 0; i< CLIENT_NUM; i++)
        {
            if(fds[i].fd > 0 && (fds[i].revents & (POLLIN | POLLERR)))
            {
                gettimeofday(&time_e, NULL);
                nRet = read(fds[i].fd, buff[fds[i].fd]+readlen[fds[i].fd], g_len-readlen[fds[i].fd]);
                gettimeofday(&time_f, NULL);
                if (nRet == 0)
                {
                    printf("%d fd %d close\n", i, fds[i].fd);
                    struct poll_ctl  delset;
                    delset.cmd = PS_DELETE;
                    delset.fd = fds[i].fd;
                    nRet = pollset_ctl(ps, &delset, 1);
                    if(nRet < 0)
                    {
                        printf("pollset_ctl failed %s\n",strerror(errno));
                        return -1;
                    }
                    fds[i].fd = -1;
                    conn -- ;
                    break;
                }
                if (nRet < 0)
                {
                    printf("%d fd %d err %s \n", i, fds[i].fd,strerror(errno));
                    fds[i].fd = -1;
                    conn -- ;
                    break;
                }

                unsigned long long use_time1 = (time_f.tv_sec - time_e.tv_sec) * 1000000 + (time_f.tv_usec - time_e.tv_usec);
                total_time1 += use_time1;
                g_count1++;

                if (nRet != g_len-readlen[fds[i].fd])//没读完
                {
                    readlen[fds[i].fd] += nRet;
                }
                else if (nRet == g_len-readlen[fds[i].fd])//读完
                {
                    if(g_verify == 1 && buff[fds[i].fd][0] != 'a')
                    {
                        printf("format error %s\n", buff[fds[i].fd]);
                        return -1;
                    }
                    readlen[fds[i].fd] = 0;
                    g_count2++;
                    memcpy(rbuff, buff[fds[i].fd], sizeof(buff[fds[i].fd]));
                    memset(buff[fds[i].fd], 0, sizeof(buff[fds[i].fd]));
                }

            }
        }

    }
    gettimeofday(&time_d, NULL);


    unsigned long long use_time2 = (time_d.tv_sec - time_c.tv_sec) * 1000000 + (time_d.tv_usec - time_c.tv_usec);
    total_time2 += use_time2;

    nRet = pollset_destroy(ps);
    if(nRet < 0)
    {
        printf("pollset_destroy failed %s\n",strerror(errno));
        return -1;
    }
#endif
    return 0;
}
