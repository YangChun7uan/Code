#include "Headdefine.h"


#define BUFSIZE   1024



int aioread(int fd)
{
    struct aiocb my_aiocb;


    /* Zero out the aiocb structure (recommended) */
    bzero( (char *)&my_aiocb, sizeof(struct aiocb) );

    /* Allocate a data buffer for the aiocb request */
    my_aiocb.aio_buf = malloc(BUFSIZE+1);
    if (!my_aiocb.aio_buf)
    {
        perror("malloc");
        return -1;
    }

    /* Initialize the necessary fields in the aiocb */
    my_aiocb.aio_fildes = fd;
    my_aiocb.aio_nbytes = BUFSIZE;
    my_aiocb.aio_offset = 0;

    int ret = aio_read( &my_aiocb );
    if (ret < 0)
    {
        perror("aio_read");
        return -1;
    }

    while ( aio_error( &my_aiocb ) == EINPROGRESS ) ;

    if ((ret = aio_return( &my_aiocb )) > 0) {
        printf("%s\n", (char *)my_aiocb.aio_buf);
    } else {
        perror("aio_read");
        return -1;
    }
}


int main()
{
    int fd = open( "file.txt", O_CREAT|O_RDWR );
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    int ret = write(fd, "aaaaaaaaaaa", 10);
    if( ret < 0 )
    {
        perror("write");
        return -1;
    }

    aioread(fd);


}




