#include "Headdefine.h"
#include "signalhandler.h"
#include "utility.h"

CSignal g_signal;

extern long msgrcv_time = 0;
extern long msgsnd_time = 0;
extern long read_time = 0;
extern long write_time = 0;
extern int msgsnd_count = 0;
extern int msgrcv_count = 0;
extern int read_count = 0;
extern int write_count = 0;

int main(int argc, char** argv)
{
    daemon();

    g_signal.RegisterSignals();

    int msgid = conn_q("./tmp");
    if (msgid < 0)
    {
        printf("connq err\n");
        return -1;
    }

    char a[512] =
    { 0 };
    for (int i = 0; i < 512; i++)
    {
        a[i] = 'a';
    }

    struct msgform read;
    memset(&read, 0, sizeof(read));
    
    int i = 0;
    while (true)
    {
        if (g_signal.HandleSignals() < 0)
            break;
        
        i++;
        
        struct msgform msg;
        memset(&msg, 0, sizeof(msg));
        msg.mtype = i;
        memcpy(msg.mtext, a, sizeof(a));

        int nRet = writeq(msgid, &msg, sizeof(a), IPC_NOWAIT);
        if (nRet < 0)
        {
            printf("writeq  err  %s\n", strerror(errno));
            break;
        }
    }
    
    
    i = 0;
    while (true)
    {
        i++;
        
        if (g_signal.HandleSignals() < 0)
            break;
        int nRet = readq(msgid, &read, sizeof(read.mtext), i, IPC_NOWAIT);
        if (nRet < 0)
        {
            printf("readq  err  %s\n", strerror(errno));
            break;
        }

        if (memcmp(read.mtext, a, sizeof(a)) != 0)
        {
            printf("format err:%s\n", read.mtext);
            break;
        }

    }
    float msgsnd = (float) msgsnd_time / (float) msgsnd_count;
    printf("msgsnd analyse number:%ld , avgage time %f us\n", msgsnd_count, msgsnd);

    float msgrcv = (float) msgrcv_time / (float) msgrcv_count;
    printf("msgrcv analyse number:%ld , avgage time %f us\n", msgrcv_count, msgrcv);

    printf("exit\n");
    return 0;
}
