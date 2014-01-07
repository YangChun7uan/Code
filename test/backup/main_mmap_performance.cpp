#include "Headdefine.h"

struct timeval time_b, time_a, time_c;
unsigned long long use_time = 0;

#define  TEST_SIZE  1024*1024

int main()
{
    long len = sysconf(_SC_PAGE_SIZE);
    long size = (long)(len*TEST_SIZE) / (long)(1024*1024);
    printf("sysconf(_SC_PAGE_SIZE):%ld , total size:%ldM\n", len, size);

    unlink("./datatmp");

    ////////////////////////////////
    //文件方式操作
    int fd = open("./datatmp", O_RDWR|O_CREAT);
    if (fd == -1)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }

    char * data = new char[len];
    memset(data, 'a', len);

    gettimeofday(&time_a, NULL);
    for(int i = 0; i < TEST_SIZE; ++i)
    {
        write(fd, data, len);
    }
    gettimeofday(&time_b, NULL);

    use_time = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
    printf("write %ldM use_time:%lldus\n", size, use_time);

    close(fd);


    ////////////////////////////////
    //mmap方式操作
    fd = open("./datatmp", O_RDWR|O_CREAT);
    if (fd == -1)
    {
        printf("open failed:%s \n", strerror(errno));
        return -1;
    }

    memset(data, '\n', len);

    gettimeofday(&time_a, NULL);
    char *ptr = (char *)mmap(0, len*TEST_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    gettimeofday(&time_b, NULL);
    if(ptr == NULL)
    {
        printf("mmap error:%s\n", strerror(errno));
        return -1;
    }

    use_time = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
    printf("mmap %ldM use_time:%lldus\n", size, use_time);


    gettimeofday(&time_a, NULL);
    for(int i = 0; i < TEST_SIZE; ++i)
    {
        memcpy(ptr, data, len);
//        if( msync(ptr, len, MS_SYNC/*MS_ASYNC*/) < 0 )
//        {
//            printf("msync error:%s,%lld\n", strerror(errno),ptr);
//            break;
//        }
        ptr += len;
    }
    gettimeofday(&time_b, NULL);

    use_time = (time_b.tv_sec - time_a.tv_sec) * 1000000 + (time_b.tv_usec - time_a.tv_usec);
    printf("mcopy %ldM use_time:%lldus\n", size, use_time);


    sleep(10);
}
