#ifndef __ABM_RTB_TOOL_ARRAYBUFFER_H
#define __ABM_RTB_TOOL_ARRAYBUFFER_H

#include <new>
using namespace std;

#include "string.h"

#include "ArrayGuard.h"

#define THROW_BAD_ALLOC throw bad_alloc("ArrayBuffer: bad allocation")

// 此类用于简单的记录数组大小和空间
template<class BufferType>
	class ArrayBuffer
{
public:

	typedef BufferType type;

	// 构造函数
	explicit ArrayBuffer(llsize_t stCapacity = 0) : m_size(0), m_capacity(stCapacity)
	{
		if (stCapacity)
		{
			m_data.reset(new type [stCapacity]);
			if ( ! m_data.get() )
			{
				THROW_BAD_ALLOC;
			}
		}
	}
	
	// 析构函数
	~ArrayBuffer() {}

	// 元素访问
	type &operator[] (llsize_t stPos)
	{
		return m_data[stPos];
	}

	const type &operator[] (llsize_t stPos) const
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
	void reset(llsize_t stCapacity = 0)
	{
		m_size = 0;

		if (capacity() == stCapacity) return;

		m_capacity = stCapacity;
		if (stCapacity)
		{
			m_data.reset(new type [stCapacity]);
			if ( ! m_data.get() )
			{
				THROW_BAD_ALLOC;
			}
		}
		else m_data.reset();
	}

	// 返回数组指针
	type * get() const
	{
		return m_data.get();
	}

	// 纠正有效元素数量
	void setsize(llsize_t stSize)
	{
		m_size = stSize;
	}

	// 返回有效元素个数
	llsize_t size() const { return m_size; }
	
	// 元素数量加 1
	ArrayBuffer &operator++()
	{
		++m_size;
		return *this;
	}

    //添加元素
    bool push_back(const type &t)
    {
        if( m_capacity <= m_size)
            return false;

        (*this)[m_size] = t;

        ++m_size;

        return true;
    }

	// 返回空间大小
	llsize_t capacity() const { return m_capacity; }

	llsize_t space() const { return (m_capacity - m_size); }

	bool autoresize()
	{
		bool isResize = false;

		if( m_size >= m_capacity )
		{
			llsize_t oldsize = m_size;

			m_capacity = m_capacity*2;

			type* p = new type[m_capacity];
			if( p == NULL )
			{
				THROW_BAD_ALLOC;
			}

			memcpy( p, m_data.get(), m_size*sizeof(type) );

			m_data.reset( p );

			m_size = oldsize;

			isResize = true;
		}

		return isResize;
	}

private:
	ArrayGuard<type> m_data;
	llsize_t           m_capacity;
	llsize_t           m_size;

};

#endif
