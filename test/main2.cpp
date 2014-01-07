#include "Headdefine.h"

int g_argc;
char** g_argv;

int array[1024] = {0};

int sortor(const void * left, const void * right)
{
    for(int i = 0; i< 20; ++i)
    {
        printf("%d ", array[i]);
    }

    static int i = 0;
    i++;
    int * leftdata = (int *)left;
    int * rightdata = (int *)right;

    printf("sortor %d %d\n", *(leftdata), *(rightdata));

    int tmp = *(leftdata) - *(rightdata);
    if( tmp > 0 )
        return 1;
    else if( tmp == 0 )
        return 0;
    else
        return -1;
}


int main(int argc, char** argv)
{
    int g_argc = argc;
    char** g_argv = argv;

    for(int i = 0; i< g_argc - 1; ++i)
    {
        array[i] = atoi(argv[i+1]);
        printf("%d ", array[i]);
    }
    printf("\n\n");

    qsort(array, g_argc-1, sizeof(int), sortor);

    for(int i = 0; i< g_argc - 1; ++i)
    {
        printf("%d ", array[i]);
    }

}
