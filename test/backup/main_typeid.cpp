#include "Headdefine.h"


class Base
{
public:

    Base(){};
    virtual ~Base(){};
};

class Child:public Base
{
public:
    Child(){};
    ~Child(){};
};

int main(int argc, char** argv)
{

    cout << typeid(int).name() << endl;

    Base *p = new Child();

    cout << typeid(p).name() << endl;
}


