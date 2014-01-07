#include "Headdefine.h"
#include "signalhandler.h"
#include "utility.h"
#include "sem.h"

#define FIFO_SERVER "./tmp"

CSignal g_signal;



int main(int argc, char** argv)
{
    g_signal.RegisterSignals();

    
    int port = 7777;
    int servfd = ConnectServer("10.200.50.247", port);
    if(servfd < 0)
    {
        printf("ConnectServer failed\n");
        return -1;
    }
    
    while(true)
    {
        char buff[128] = {0};
        read(STDOUT_FILENO, buff, 1);
        
        if( strcmp(buff, "a") == 0 || strcmp(buff, "b") == 0)
        {
            write(servfd, buff, 1);
        }
    }
    
    return 0;
}

