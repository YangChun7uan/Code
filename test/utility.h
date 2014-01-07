#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "Headdefine.h"


struct msgform
{
    long mtype;
    char mtext[1024];
};

void daemon();

int readn(int sockfd, char *vptr, int n, int timeout);

int writen(int sockfd, const char *buf, int n);
                     
int conn_q(const char *pathname);

ssize_t readq(int msqid, struct msgform *msgp, size_t msgsz, long msgtyp, int msgflg);

int writeq(int msqid, struct msgform *msgp, size_t msgsz, int msgflg);

int read_msg(int sock,char *rcvbuf,int &rcvlen);

int listen_net(int port);

int my_accept(int fd);

int select_timeout(int *fd, int fdnum, int *fdout, struct timeval *timeout);

int set_fd_nonblock(int fd);

int ConnectServer(const char *ip, int nServport);
#endif
