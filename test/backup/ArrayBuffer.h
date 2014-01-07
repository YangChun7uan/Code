#ifndef __ABM_RTB_TOOL_ARRAYBUFFER_H
#define __ABM_RTB_TOOL_ARRAYBUFFER_H

#include <new>
using namespace std;

#include "string.h"

#include "ArrayGuard.h"

#define THROW_BAD_ALLOC throw bad_alloc("ArrayBuffer: bad allocation")

// �������ڼ򵥵ļ�¼�����С�Ϳռ�
template<class BufferType>
	class ArrayBuffer
{
public:

	typedef BufferType type;

	// ���캯��
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
	
	// ��������
	~ArrayBuffer() {}

	// Ԫ�ط���
	type &operator[] (llsize_t stPos)
	{
		return m_data[stPos];
	}

	const type &operator[] (llsize_t stPos) const
	{
		return m_data[stPos];
	}

	// �ͷſ���
	type * release()
	{
		m_size = 0;
		m_capacity = 0;
		return m_data.release();
	}

	// ��ֵ����
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

	// ��λ
	void clear()
	{
		setsize(0);
	}

	// ���·���ռ�
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

	// ��������ָ��
	type * get() const
	{
		return m_data.get();
	}

	// ������ЧԪ������
	void setsize(llsize_t stSize)
	{
		m_size = stSize;
	}

	// ������ЧԪ�ظ���
	llsize_t size() const { return m_size; }
	
	// Ԫ�������� 1
	ArrayBuffer &operator++()
	{
		++m_size;
		return *this;
	}

    //���Ԫ��
    bool push_back(const type &t)
    {
        if( m_capacity <= m_size)
            return false;

        (*this)[m_size] = t;

        ++m_size;

        return true;
    }

	// ���ؿռ��С
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
