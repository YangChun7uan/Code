#include "Headdefine.h"

#define THROW_BAD_ALLOC throw bad_alloc("ArrayBuffer: bad allocation")

class A
{
public:
    A()
    {
        ll = 1;
        l = 1;
        ui = 1;
        ul = 1;
        f = 1.1;
        d = 1.1;
        i = 1;
        c = 1;
    }

    long long ll;
    long l;
    unsigned int ui;
    unsigned long ul;
    float f;
    double d;
    int i;
    char c;

};

// 保护动态分配的数组自动释放
template<class ArrayType>
class ArrayGuard
{
public:
    typedef ArrayType type;

    explicit ArrayGuard(type *pArray = NULL)
            : m_pArray(pArray)
    {
    }

    ArrayGuard(ArrayGuard<type> &array)
    {
        m_pArray = array.release();
    }

    ~ArrayGuard()
    {
        delete[] m_pArray;
    }

    void reset(type *pArray = NULL)
    {
        if (NULL != m_pArray)
        {
            delete[] m_pArray;
        }

        m_pArray = pArray;
    }

    type * release()
    {
        type *p = m_pArray;
        m_pArray = NULL;
        return p;
    }

    type * get() const
    {
        return m_pArray;
    }

    type & operator[](unsigned long long stPos)
    {
        return *(m_pArray + stPos);
    }
    const type & operator[](unsigned long long stPos) const
    {
        return *(m_pArray + stPos);
    }

    ArrayGuard<type> & operator=(ArrayGuard<type> &array)
    {
        if ((&array) != this)
        {
            reset(array.release());
        }
        return (*this);
    }

private:
    type * m_pArray;
};

template<class BufferType>
class ArrayBuffer
{
public:

    typedef BufferType type;

    // 构造函数
    explicit ArrayBuffer(unsigned long long stCapacity = 0)
            : m_size(0), m_capacity(stCapacity)
    {
        if (stCapacity)
        {
            m_data.reset(new type[stCapacity]);
            if (!m_data.get())
            {
                THROW_BAD_ALLOC;
            }
        }
    }

    // 析构函数
    ~ArrayBuffer()
    {
    }

    // 元素访问
    type &operator[](unsigned long long stPos)
    {
        return m_data[stPos];
    }

    const type &operator[](unsigned long long stPos) const
    {
        return m_data[stPos];
    }

    // 释放控制
    type * release()
    {
        m_size = 0;
        m_capacity = 0;
        return m_data.release();
    }

    // 赋值操作
    ArrayBuffer<type> &operator=(ArrayBuffer<type> &array)
    {
        if ((&array) != this)
        {
            m_size = array.size();
            m_capacity = array.capacity();
            m_data.reset(array.release());
        }
        return *this;
    }

    ArrayBuffer<type> &mem_assign(ArrayBuffer<type>& array)
    {
        if ((&array) != this)
        {
            if (capacity() < array.size())
            {
                reset(array.size());
            }

            memcpy(get(), array.get(), sizeof(type) * array.size());
            setsize(array.size());
        }
        return *this;
    }

    // 复位
    void clear()
    {
        setsize(0);
    }

    // 重新分配空间
    void reset(unsigned long long stCapacity = 0)
    {
        m_size = 0;

        if (capacity() == stCapacity)
            return;

        m_capacity = stCapacity;
        if (stCapacity)
        {
            m_data.reset(new type[stCapacity]);
            if (!m_data.get())
            {
                THROW_BAD_ALLOC;
            }
        }
        else
            m_data.reset();
    }

    // 返回数组指针
    type * get() const
    {
        return m_data.get();
    }

    // 纠正有效元素数量
    void setsize(unsigned long long stSize)
    {
        m_size = stSize;
    }

    // 返回有效元素个数
    unsigned long long size() const
    {
        return m_size;
    }

    // 元素数量加 1
    ArrayBuffer &operator++()
    {
        ++m_size;
        return *this;
    }

    // 返回空间大小
    unsigned long long capacity() const
    {
        return m_capacity;
    }

    unsigned long long space() const
    {
        return (m_capacity - m_size);
    }

    bool autoresize()
    {
        bool isResize = false;

        if (m_size >= m_capacity)
        {
            unsigned long long oldsize = m_size;

            m_capacity = m_capacity * 2;

            type* p = new type[m_capacity];
            if (p == NULL)
            {
                THROW_BAD_ALLOC;
            }

            memcpy(p, m_data.get(), m_size * sizeof(type));

            m_data.reset(p);

            m_size = oldsize;

            isResize = true;
        }

        return isResize;
    }

private:
    ArrayGuard<type> m_data;
    unsigned long long m_capacity;
    unsigned long long m_size;

};

int main(int argc, char** argv)
{
    A a;

    ArrayBuffer<A>  aa1;
    aa1.reset(1024);

    A aa2[1024];
    memset(aa2, 0, sizeof(aa2));

    vector<A>  aa3;
    aa3.clear();

    if(1 == atoi(argv[1]) )
    {
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            aa1[pos] = a;
        }
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            long long tmp = aa1[pos].ll;
        }
    }

    if(2 == atoi(argv[1]) )
    {
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            aa2[pos] = a;
        }
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            long long tmp = aa2[pos].ll;
        }
    }

    if(3 == atoi(argv[1]) )
    {
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            aa2[pos] = a;
        }
        for (long i = 0; i < atol(argv[2]); ++i)
        {
            int pos = i%1024;
            long long tmp = aa2[pos].ll;
        }
    }

    return 0;
}
