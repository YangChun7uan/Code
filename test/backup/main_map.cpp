#include "Headdefine.h"

struct A
{
public:
    A():i(0),ui(0),l(0),ll(0),f(0),d(0)
    {
        cs[0] = '\0';
    }
    ~A()
    {
    }

    int i;
    unsigned int ui;
    long l;
    long long ll;
    char cs[512];
    float f;
    double d;

};


int main(int argc, char** argv)
{
    multimap<int, A>   m;
    A a1,a2;
    a1.i = 1;
    a2.i = 2;

    m.insert(pair<int, A>( 1, a1));
    m.insert(pair<int, A>( 1, a2));

    multimap<int, A>::iterator it = m.find(1);

    printf("m[1].i %d\n", it->second.i);
    it++;
    printf("m[2].i %d\n", it->second.i);

}
