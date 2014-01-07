#ifndef __HEAD_DEFINE_H__
#define __HEAD_DEFINE_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>
#include <limits.h>
#include <string>
#include <string.h>
#include <iostream> 
#include <set>
#include <map>
#include <vector>
#include <exception>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <aio.h>
#include <signal.h>
#ifdef AIX
#include <sys/pollset.h>
#endif 
using namespace std;

/**
*  ��ͬƽ̨�ϵĺ궨��
*/

#ifdef HP_UNIX
typedef int SOCKETLEN_T;
#endif

#ifdef LINUX
typedef socklen_t SOCKETLEN_T;
#endif //__WM_LINUX

#ifdef AIX
typedef unsigned int SOCKETLEN_T;
#endif

typedef unsigned char        UINT8;         //1�ֽ�
typedef unsigned short int   UINT16;        //2�ֽ�
typedef unsigned int         UINT32;        //4�ֽ�
typedef unsigned long long   UINT64;        //8�ֽ�
typedef short int            INT16;         //2�ֽ�
typedef int                  INT32;         //4�ֽ�
typedef long long            INT64;         //8�ֽ�


#define LENGTH_OF_LISTEN_QUEUE 100 //length of listen queue in server

#define SOCK_BUF               1024000

//#define p(...)   printf(__VA_ARGS__)

#endif
