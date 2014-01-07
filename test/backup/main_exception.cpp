
class ExceptionBase
{
public:
    ExceptionBase(int code)
    {
        m_code = code;
    }
    int m_code;

};


class Exception: public ExceptionBase
{
public:

    Exception(int code):ExceptionBase(code)
    {

    }

};


int  test()
{
    throw Exception(1);

    return -1;
}

int main(int argc, char ** argv)
{

    int ret = -99;
    try
    {
        ret = test();

    }
    catch(int a)
    {
        cout << "main exception int " << a << endl;
    }
    catch(exception &ex)
    {
        cout << "main exception ex " << ex.what() << endl;
    }
    catch(ExceptionBase &eb)
    {
        cout << "main ExceptionBase %d"  << eb.m_code << endl;
    }
    catch(...)
    {
        cout << "main exception unknow "  << endl;
    }

    printf("ret %d\n", ret);




    return 0;
}









