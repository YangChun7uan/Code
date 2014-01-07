#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <stddef.h>     /* for offsetof */
#include "Socket.h"


CSocket::CSocket():m_servfd(-1)
{

}

CSocket::~CSocket()
{

}

void CSocket::CloseFD(int fd)
{
    int err = errno;
    if (fd > 0)
        close(fd);
    errno = err;
}

int CSocket::Listen(char *sFile)
{
    int nRet = 0;

    m_servfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_servfd < 0)
    {
        return -1;
    }

    unlink(sFile);

    struct sockaddr_un un;
    memset(&un, 0, sizeof(un));

    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, sFile);

    int len = offsetof(struct sockaddr_un, sun_path) + strlen(sFile);
    nRet = bind(m_servfd, (struct sockaddr*) &un, len);
    if (nRet < 0)
    {
        CloseFD(m_servfd);
        return -2;
    }

    nRet = listen(m_servfd, LISTEN_QUEUE);
    if (nRet < 0)
    {
        CloseFD(m_servfd);
        return -3;
    }

    return m_servfd;

}

int CSocket::Accept()
{
    if (m_servfd <= 0)
        return -1;

    struct sockaddr_un un;
    memset(&un, 0, sizeof(un));
    socklen_t len = sizeof(un);
    int peerfd = accept(m_servfd, (struct sockaddr *) &un, &len);
    if (peerfd < 0)
        return -1;
    /*
    len -= offsetof(struct sockaddr_un, sun_path);
    un.sun_path[len] = 0;

    struct stat statbuf;
    if (stat(un.sun_path, &statbuf) < 0)
    {
        CloseFD(peerfd);
        return -2;
    }

#ifdef  S_ISSOCK    // not defined for SVR4
    if (S_ISSOCK(statbuf.st_mode) == 0)
    {
        CloseFD(peerfd);
        return -3;
    }
#endif

    if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) || (statbuf.st_mode & S_IRWXU) != S_IRWXU)
    {
        CloseFD(peerfd);
        return -4;
    }

    
     time_t staletime = time(NULL) - 30 ;// client's name can't be older than this (sec)
     if (statbuf.st_atime < staletime || statbuf.st_ctime < staletime || statbuf.st_mtime < staletime)
     {
     CloseFD(peerfd);
     return -5;
     }*/

    //if (uidptr != NULL)
    //    *uidptr = statbuf.st_uid; /* return uid of caller */

    //unlink(un.sun_path); /* we're done with pathname now */

    return peerfd;

}

int CSocket::Connect(char *sFile)
{
    /* create a UNIX domain stream socket */
    int peerfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (peerfd < 0)
        return -1;

    /* fill socket address structure with our address */
    struct sockaddr_un un;
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    sprintf(un.sun_path, "%s_%s_%05d", sFile, "CLIENT", getpid());
    int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
    unlink(un.sun_path); /* in case it already exists */
    
    int nRet = bind(peerfd, (struct sockaddr *) &un, len);
    if ( nRet < 0)
    {
        CloseFD(peerfd);
        return -2;
    }
    
    /*
    nRet = chmod(un.sun_path, S_IRWXU);
    if ( nRet < 0)    //rwx for user only
    {
        CloseFD(peerfd);
        return -3;
    }
    */
    
    //fill socket address structure with server's address
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, sFile);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(sFile);
    
    nRet = connect(peerfd, (struct sockaddr *) &un, len);
    if ( nRet < 0)
    {
        CloseFD(peerfd);
        return -4;
    }
    
    return peerfd;

}


int CSocket::Writen(int peerfd,const char *buff, int len)
{
    int nleft;
    int nwritten;
    const char *ptr;

    ptr = buff;
    nleft = len;
    
    while (nleft > 0) 
    {
        nwritten = write(peerfd, ptr, nleft);
        if ( nwritten <= 0 ) 
        {
            if(errno == EINTR)
            {
                return -2;
            }
            else
                return nwritten;
        }
        
        nleft -= nwritten;
        ptr += nwritten;
    }
    return len;
}


int CSocket::Readn(int peerfd, char *buff, int len)
{
    size_t  nleft;
    ssize_t nreaded;
    char   *ptr;

    ptr = buff;
    nleft = len;

    while (nleft > 0) 
    {
        nreaded = read(peerfd, ptr, nleft);
        if ( nreaded < 0) 
        {
            if ( errno == EINTR  )
            {
                return -2;
            }
            else
                return -1;
        } 
        else if (nreaded == 0)
            return 0;
        
        nleft -= nreaded;
        ptr += nreaded;
    }
    return (len - nleft);
}

int CSocket::SendFD(int pipefd, char *buff, int fd_to_send)
{
    union 
    {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;

    struct iovec iov[1];
    iov[0].iov_base = buff;
    iov[0].iov_len = MAX_COM_LENGTH;
    
    struct msghdr msg;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    struct cmsghdr *cmptr;
    cmptr = CMSG_FIRSTHDR(&msg);
    cmptr->cmsg_len = CMSG_LEN(sizeof(int));
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type = SCM_RIGHTS;
    *((int *) CMSG_DATA(cmptr)) = fd_to_send;

#ifdef AIX
    return(sendmsg(pipefd, &msg, MSG_NONBLOCK));
#else
    return(sendmsg(pipefd, &msg, MSG_DONTWAIT));
#endif
}

int CSocket::RecvFD(int pipefd, char *buff)
{
    int recvfd = -1;
    int n = 0;
    
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;

    struct iovec iov[1];
    iov[0].iov_base = buff;
    iov[0].iov_len = MAX_COM_LENGTH;

    struct msghdr msg;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if ((n=recvmsg(pipefd, &msg, 0)) <= 0)
        return n;

    struct cmsghdr *cmptr;
    if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL )
    {
        if (cmptr->cmsg_len == CMSG_LEN(sizeof(int)))
        {
            if (cmptr->cmsg_level != SOL_SOCKET)
            {
                //printf("control level != SOL_SOCKET\n");
            }
            if (cmptr->cmsg_type != SCM_RIGHTS)
            {
                //printf("control type != SCM_RIGHTS\n");
            }
            recvfd = *((int *) CMSG_DATA(cmptr));
        }
    }

    return recvfd;
}
