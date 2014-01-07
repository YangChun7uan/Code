#ifndef __ABM_RTB_TOOL_ARRAYGUARD_H
#define __ABM_RTB_TOOL_ARRAYGUARD_H

#define llsize_t unsigned long long 

// 保护动态分配的数组自动释放
template<class ArrayType> class ArrayGuard
{
public:
	typedef ArrayType type;

	explicit ArrayGuard(type *pArray = NULL) : m_pArray(pArray) {}
		
	ArrayGuard(ArrayGuard<type> &array)
	{
		m_pArray = array.release();
	}
	
	~ArrayGuard() { delete [] m_pArray; }
	
	void reset(type *pArray = NULL)
	{
	    if(NULL != m_pArray)
	    {
	        delete [] m_pArray;
	    }
	    
		m_pArray = pArray;
	}
	
	type * release()
	{
		type *p = m_pArray;
		m_pArray = NULL;
		return p;
	}
	
	type * get() const { return m_pArray; }
	
	type & operator[](llsize_t stPos) { return *(m_pArray + stPos); }
	const type & operator[](llsize_t stPos) const { return *(m_pArray + stPos); }
	
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

#endif
