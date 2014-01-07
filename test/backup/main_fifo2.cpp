#include "Headdefine.h"
#include "sem.h"

#define FIFO_SERVER "./tmp"

int main(int argc, char** argv)
{
    int fd;
    char r_buf[1024] = {0};
    int real_wnum;


    fd = open(FIFO_SERVER,  O_RDONLY);
    if (fd == -1)
    {
            printf("open error; no reading process  err:%s \n",strerror(errno));
            return -1;
    }
    
    printf("read start==============\n");

    int count = 0;
    while (1)
    {
        
        count++;
        real_wnum = read(fd, r_buf, 1024);
        if (real_wnum == -1)
        {
            
            printf("read to fifo error; try later  count:%d err:%s  \n", count,strerror(errno));
            break;
        }
        printf("read len %d, count:%d\n %s\n",real_wnum,count, r_buf);

    }

        

    return 0;
}

