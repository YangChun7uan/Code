
#include "Headdefine.h"

int main()
{
    
    char *error = NULL;
    void* hm = NULL;
    char file[128] = "./libtest.so";
    
    dlerror();
    hm = dlopen(file , RTLD_LAZY);
    if (hm == NULL || (error = dlerror()) != NULL)
    {
        printf("Open library %s failed\n error:%s\n%s\n" , file, strerror(errno), error);
        return -1;
    }
    
    int (*fun)(int i) = NULL;
    dlerror();
    fun = (int(*)(int))::dlsym(hm , "Print");
    if ((error = dlerror()) != NULL)
    {
        printf("Can't find Print() function in %s\n" , file);
        printf("Error: %s\n" , error);
        dlclose(hm);
        return 0;
    }
    
    (*fun)(5);
    printf("-----\n");
    
    
    char *a = "abc";
    char *b = "abc";
    
    printf("----a %s\n",a);
    printf("----b %s\n",b);
    
    char *p = a+1;
    printf("----p %c\n", *p);
    
    *p = 'd';
    printf("----a %s\n",a);
    printf("----b %s\n",b);

    return 0;
}
