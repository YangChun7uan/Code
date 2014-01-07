

#ifndef _SOCKET_H
#define _SOCKET_H

#define MAX_CLINET_NUMBER     1024
#define LISTEN_QUEUE    1024
#define MAX_COM_LENGTH   32

class CSocket
{
public:
    CSocket();
    ~CSocket();
    
    int Listen(char *sFile);
    
    int Accept();
    
    int Connect(char *sFile);
    
    int Writen(int peerfd, const char *pBuff, int len);
    
    int Readn(int peerfd, char *pBuff, int len);
    
    int SendFD(int pipefd, char *buff, int fd_to_send);
    
    int RecvFD(int pipefd, char *buff);
private:
    void CloseFD(int fd);
    
private:
    
    int m_servfd;
};


#endif

