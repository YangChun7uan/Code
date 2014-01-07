#include "Headdefine.h"

struct sss
{
    char a[8];
    char b[8];
};


bool is_big_endian()//如果字节序为big-endian，返回true;反之为little-endian，返回false
{
    unsigned short x = 0x1234;
    if(*(unsigned char*)&x == 0x12)
        return true;

    return false;
}

int main()
{
    struct sss s;
    strncpy(s.b, "123456000078", 7);
    strncpy(s.a, "123450000678", 7);

    printf("%s--\n", s.a);

    char *tmp = (char *)&s;
    for(int i = 0; i < sizeof(sss); i++)
    {

        printf("[%d] %c\n", i, *tmp);
        tmp++;
    }
    printf("\n\n");

    static char snp[1024] = {0};
    snprintf(snp, sizeof(snp), "%s", "abc");
    snprintf(snp, sizeof(snp), "%s", "abc");
    snprintf(snp, sizeof(snp), "%s", "abc");
    printf("snp:%s\n\n",snp);

    long long aaa;
    int bbb;
    aaa = -9223372036854775807;
    bbb = (int)aaa;
    printf("-9223372036854775807:%lld %d\n", aaa, bbb);
    aaa = -2147483648;
    bbb = (int)aaa;
    printf("-2147483648:%lld %d\n", aaa, bbb);
    aaa = -2147483647;
    bbb = (int)aaa;
    printf("-2147483647:%lld %d\n", aaa, bbb);
    aaa = -1;
    bbb = (int)aaa;
    printf("-1:%lld %d\n", aaa, bbb);
    aaa = 0;
    bbb = (int)aaa;
    printf("0:%lld %d\n", aaa, bbb);
    aaa = 1;
    bbb = (int)aaa;
    printf("1:%lld %d\n", aaa, bbb);
    aaa = 2147483647;
    bbb = (int)aaa;
    printf("2147483647:%lld %d\n", aaa, bbb);
    aaa = 2147483648;
    bbb = (int)aaa;
    printf("2147483648:%lld %d\n", aaa, bbb);
    aaa = 9223372036854775807;
    bbb = (int)aaa;
    printf("9223372036854775807:%lld %d\n", aaa, bbb);


    char str[10]= {0};
    memset(str, 'a', sizeof(str));
    for(int i = 0; i<sizeof(str); ++i)
    {
        printf("[%d]%c\n",i,str[i]);
    }

    strncpy(str, "abc", strlen("abc"));
    for(int i = 0; i<sizeof(str); ++i)
    {
        printf("[%d]%c\n",i,str[i]);
    }


}




