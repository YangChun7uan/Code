#include "Headdefine.h"
#include "ArrayBuffer.h"

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

    if( strcmp( argv[1], "vector1" ) == 0 )
    {
        vector<A> vec;
        for(int i = 0; i < 2000000; ++i)
        {
            for(int x = 0; x < 100; ++x)
            {
                A a;
                vec.push_back(a);
            }
            vec.clear();
        }
    }
    if( strcmp( argv[1], "vector2" ) == 0 )
    {
        vector<A> vec;
        for(int i = 0; i < 2000000; ++i)
        {
            for(int x = 0; x < 100; ++x)
            {
                A a;
                vec.push_back(a);
            }
            vec.clear();
        }
    }

    if( strcmp( argv[1], "array" ) == 0 )
    {
        A* array = new A[100];
        int size = 0;
        for(int i = 0; i < 2000000; ++i)
        {
            for(int x = 0; x < 100; ++x)
            {
                size = x+1;
                A a;
                array[x] = a;
            }
        }
        delete array;
    }


    if( strcmp( argv[1], "template" ) == 0 )
    {
        ArrayBuffer<A>   tem;
        tem.reset(100);
        for(int i = 0; i < 2000000; ++i)
        {
            for(int x = 0; x < 100; ++x)
            {
                A a;
                tem.push_back(a);
                //tem[x] = a;
            }
            tem.clear();
        }

    }

}
