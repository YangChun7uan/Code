#include "Headdefine.h"

int readfile(char *file, char *buff)
{
    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
        printf("open failed:%s\n", file);
        return -1;
    }

    char *ptr = buff;
    int     n = 0;
    int     count = 0;
    while ((n = read(fd, ptr, 1)) > 0)
    {
        ptr   += n;
        count += n;
    }

    close(fd);

    *ptr = '\0';

    return count;
}

int isCsStruct(char *p)
{
    CS_CDR_INFO*   a;

    a = (CS_CDR_INFO*)p;

    int time = a->list_info.nBeginEventTimestamp ;

    //20121127000000
    //20121130000000
    if(time < 3562934400 || time > 3563193600)
        retunr -1;

    if( a->list_info.sSessionID[0] != 'S')
        return -1;

    return 0;
}


int main(int argc, char ** argv)
{
    char buff[1024*1024*100] = {0};

    //打开输入文件
    int filelen = readfile(argv[1], buff);
    if(filelen < 0)
    {
        printf("readfile failed\n");
        return -1;
    }

    //打开输出文件
    char outfile[256] = {0};
    sprintf(outfile, "%s.out", argv[1]);
    FILE* outfd = fopen(outfile, "wb");
    if(outfd == NULL)
    {
        printf("fopen failed %s\n",  strerror(errno));
        return -1;
    }

    int pos = 0;
    int okcout = 0;
    while( pos < filelen - sizeof(CS_CDR_INFO) )
    {
        if( isCsStruct(buff+pos) == 0)
        {
            // 写outfile
            if(fwrite(buff+pos, sizeof(CS_CDR_INFO), 1, outfd) != 1)
            {
                printf("fwrite faild %s\n", strerror(errno));
                return -1;
            }

            okcout++;

            pos += sizeof(CS_CDR_INFO);
            continue;
        }

        pos++;
    }

    printf("out put record %d\n", okcout);
    return 0;

}


