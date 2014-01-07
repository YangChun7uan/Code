#include "utility.h"

long msgrcv_time = 0;
long msgsnd_time = 0;

int msgsnd_count = 0;
int msgrcv_count = 0;


extern int g_nSigTerm;  //SIGTERM信号标记
extern int g_nSigChild; //SIGCHILD信号标记
extern int g_nSigHup;   //SIGHUP信号标记
extern int g_nSigUsr2;  //SIGUSR2信号标记
extern int g_nSigAlarm; //SIGALARM信号标记
extern int g_nSigPipe;  //SIGPIPE信号标记

void daemon()
{
    //建立守护进程
    int pid = 0;
    if ((pid = fork()) > 0)
    {
        exit(0);
    }

    setsid();
    if ((pid = fork()) > 0)
    {
        exit(0);
    }
}

int writen(int sockfd, const char *buff, int n)
{
    int nleft;
    int nwritten;
    const char *ptr;

    ptr = buff;
    nleft = n;

    while (nleft > 0)
    {
        alarm(10);

        nwritten = write(sockfd, ptr, nleft);

        alarm(0);
        if (nwritten <= 0)
        {
            if (errno == EINTR)
            {
                if (g_nSigTerm || g_nSigAlarm || g_nSigPipe)
                    return nwritten;
                else
                    nwritten = 0; /* and call write() again */
            }
            else
                return nwritten; /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int readn(int sockfd, char *vptr, int n, int timeout)
{
    size_t nleft;
    ssize_t nreaded;
    char *ptr;

    ptr = vptr;
    nleft = n;

    while (nleft > 0)
    {
        if (timeout > 0)
            alarm(timeout);
        
        nreaded = read(sockfd, ptr, nleft);

        if (timeout > 0)
            alarm(0);

        if (nreaded < 0)
        {
            if (errno == EINTR)
            {
                if (g_nSigTerm || (g_nSigAlarm && timeout > 0))
                {
                    return -1;
                }
                else
                    nreaded = 0; /* and call read() again */
            }
            else
                return -1;
        }
        else if (nreaded == 0)
            return 0; /* EOF */

        nleft -= nreaded;
        ptr += nreaded;
    }
    return (n - nleft); /* return >= 0 */
}

int writeq(int msqid, struct msgform *msgp, size_t msgsz, int msgflg)
{
    struct timeval time_bf, time_af;
    while (true)
    {
        gettimeofday(&time_bf, NULL);
        int ret = msgsnd(msqid, msgp, msgsz, msgflg);
        gettimeofday(&time_af, NULL);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                if (g_nSigTerm || g_nSigAlarm || g_nSigPipe)
                    return ret;
                else
                    continue;
            }
            else if (errno == EAGAIN)
            {
                return ret;
            }
            else
                return ret;
        }
        else
        {
            //UTL_DEBUG("msgsnd use time:%ldus\n",(time_af.tv_sec-time_bf.tv_sec)*1000000 + (time_af.tv_usec - time_bf.tv_usec));
            long use_time = (time_af.tv_sec - time_bf.tv_sec) * 1000000 + (time_af.tv_usec - time_bf.tv_usec);
            if (use_time < 1000)
            {
                msgsnd_time += use_time;
                msgsnd_count++;
            }
            return ret;
        }
    }

    return 0;
}

ssize_t readq(int msqid, struct msgform *msgp, size_t msgsz, long msgtyp, int msgflg)
{
    struct timeval time_bf, time_af;
    while (true)
    {
        gettimeofday(&time_bf, NULL);
        ssize_t ret = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
        gettimeofday(&time_af, NULL);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                if (g_nSigTerm || g_nSigAlarm || g_nSigPipe)
                    return ret;
                else
                    continue;
            }
            else
                return ret;
        }
        else
        {
            long use_time = (time_af.tv_sec - time_bf.tv_sec) * 1000000 + (time_af.tv_usec - time_bf.tv_usec);
            //UTL_DEBUG("msgrcv use time:%ldus\n",use_time);
            if (use_time < 1000)
            {
                msgrcv_time += use_time;
                msgrcv_count++;
            }
            return ret;
        }
    }

    return 0;
}

int conn_q(const char *pathname)
{
    key_t key = ftok(pathname, 1);
    if (key == -1)
    {
        return -1;
    }

    int q_id = msgget(key, 0666 | IPC_CREAT);
    return q_id;
}

int read_msg(int sock, char *rcvbuf, int &rcvlen)
{
    int ret = 0;

    //读长度
    ret = readn(sock, rcvbuf, 4, 0);
    if (ret == 0)
    {
        struct sockaddr_in peer_addr;
        memset(&peer_addr, 0, sizeof(peer_addr));
        socklen_t addr_len = sizeof(peer_addr);
        getpeername(sock, (struct sockaddr*) &peer_addr, &addr_len);

        printf("peer socket close.[%s:%d:%d].[%s][%d]\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), sock, __FILE__, __LINE__);
        return -1;
    }
    if (ret < 0)
    {
        printf("read header failed:%s.[%s][%d]\n", strerror(errno), __FILE__, __LINE__);
        return -2;
    }

    //长度检验
    rcvlen = ntohl(*((UINT32 *) (rcvbuf)));
    if (rcvlen < 0 || rcvlen > 1000)
    {
        printf("hearder len invalid:%d.[%s][%d]\n", rcvlen, __FILE__, __LINE__);
        return -3;
    }

    //读消息体
    ret = readn(sock, rcvbuf + 4, rcvlen - 4, 0);
    if (ret == 0)
    {
        struct sockaddr_in peer_addr;
        memset(&peer_addr, 0, sizeof(peer_addr));
        socklen_t addr_len = sizeof(peer_addr);
        getpeername(sock, (struct sockaddr*) &peer_addr, &addr_len);

        printf("peer socket close.[%s:%d:%d].[%s][%d]\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), sock, __FILE__, __LINE__);
        return -1;
    }
    if (ret < 0)
    {
        printf("read body failed:%s.[%s][%d]\n", strerror(errno), __FILE__, __LINE__);
        return -4;
    }

    return 0;
}

int listen_net(int port)
{
    struct sockaddr_in servaddr;
    int servfd = -1;
    if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create tcp socket failed:%s\n", strerror(errno));
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    int one = 1;
    if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)))
    {
        printf("setsockopt failed:%s\n", strerror(errno));
        return -1;
    }
    if (bind(servfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        printf("tcp bind failed:%s\n", strerror(errno));
        return -1;
    }

    if (listen(servfd, LENGTH_OF_LISTEN_QUEUE) < 0)
    {
        printf("tcp listen port failed:%s\n", strerror(errno));
        return -1;
    }

    return servfd;
}

int my_accept(int fd)
{
    int clifd;

    struct sockaddr_in cliaddr;
    socklen_t length = sizeof(cliaddr);
    memset(&cliaddr, 0, sizeof(cliaddr));

    if ((clifd = accept(fd, (struct sockaddr*) &cliaddr, &length)) < 0)
    {
        if (errno == EINTR) //被信号中断
        {
            if (g_nSigTerm == 1)
            {
                return -1;
            }
            else
                return -2;
        }
        printf("my_accept() failed:%s. at \"%s\" line %d\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    else
    {
        printf("accept client success.clifd:%d\n", clifd);
        return clifd;

    }

}

int select_timeout(int *fdin, int fdnum, int *fdout, struct timeval *timeout)
{
    fd_set rset;
    while (true)
    {
        FD_ZERO(&rset);
        int maxfd = 0;
        for (int i = 0; i < fdnum; i++)
        {
            if (fdin[i] > maxfd)
                maxfd = fdin[i];
            FD_SET(fdin[i], &rset);
        }

        int nRet = select(maxfd + 1, &rset, NULL, NULL, timeout);
        if (nRet < 0)
        {
            if (errno == EINTR)
            {
                if (g_nSigTerm || g_nSigAlarm || g_nSigPipe)
                {
                    printf("select failed:%d,%s\n", nRet, strerror(errno));
                    return nRet;
                }
                continue;
            }
            else
            {
                printf("select failed:%d,%s\n", nRet, strerror(errno));
                return -1;
            }
        }
        else if (nRet == 0)//指定时间内没有描述符准备好
        {
            printf("select failed:%d,%s\n", nRet, strerror(errno));
            return -1;
        }
        else
        {
            int total = 0;
            for (int i = 0; i < fdnum; i++)
            {
                if (FD_ISSET(fdin[i], &rset) != 0)
                {
                    fdout[total] = fdin[i];
                    total++;
                }

            }
            return total;
        }
    }
    return 0;
}

int set_fd_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);

    return 0;
}

int ConnectServer(const char *ip, int nServport)
{
    int nServfd;
    struct sockaddr_in nServaddr;
    if ((nServfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error:%s\n", strerror(errno));
        return -1;
    }
    memset(&nServaddr, 0, sizeof(nServaddr));
    nServaddr.sin_family = AF_INET;
    nServaddr.sin_port = htons(nServport);

    if (inet_pton(AF_INET, ip, &nServaddr.sin_addr) <= 0)
    {
        printf("inet_pton error：%s\n", strerror(errno));
        return -1;
    }

    if (connect(nServfd, (sockaddr *) &nServaddr, sizeof(nServaddr)) < 0)
    {
        close(nServfd);
        printf("Connect Error:%s\n", strerror(errno));
        return -1;
    }

    return nServfd;
}
