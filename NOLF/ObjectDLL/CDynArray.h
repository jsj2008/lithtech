#ifndef __DYNARRAY_H
#define __DYNARRAY_H

template <class T>
class CDynArray
{
public:

	CDynArray (int bMemCopyable = 0, long nGrowBy = 128);
	~CDynArray();

	T&		operator[] (int nIndex);
	void	Flush();
	
	long	ArraySize()							{ return m_nArraySize; }
	void	SetMemCopyable (int bMemCopyable)	{ m_bMemCopyable = bMemCopyable; }
	void	SetGrowBy (long nGrowBy)			{ m_nGrowBy = nGrowBy; }

protected:

	int		Grow();

	T*		m_pItems;
	long	m_nArraySize;
	long	m_nGrowBy;
	int		m_bMemCopyable;
};

template <class T>
CDynArray<T>::CDynArray (int bMemCopyable, long nGrowBy)
{
	m_bMemCopyable = bMemCopyable;
	m_nGrowBy = nGrowBy;

	m_pItems = NULL;
	m_nArraySize = 0;
}

template <class T>
CDynArray<T>::~CDynArray()
{
	Flush();
}

template <class T>
T& CDynArray<T>::operator[] (int nIndex)
{
	_ASSERT(nIndex >= 0);

	while (nIndex >= m_nArraySize)
	{
		Grow();
	}

	return m_pItems[nIndex];
}

template <class T>
void CDynArray<T>::Flush()
{
	debug_deletea(m_pItems);
	m_pItems = NULL;
	m_nArraySize = 0;
}

template <class T>
int CDynArray<T>::Grow()
{
	T* pNewArray = debug_newa(T, m_nArraySize + m_nGrowBy);
	if (!pNewArray) return 0;

	if (m_bMemCopyable)
	{
		memcpy (pNewArray, m_pItems, m_nArraySize * sizeof(T));
	}
	else
	{
		for (long i = 0; i < m_nArraySize; i++)
		{
			pNewArray[i] = m_pItems[i];
		}
	}

	debug_deletea(m_pItems);
	m_pItems = pNewArray;
	m_nArraySize += m_nGrowBy;

	return 1;
}

#endif
