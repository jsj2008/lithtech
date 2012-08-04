#ifndef __STACK_H
#define __STACK_H

template <class T>
class CStack
{
public:

	CStack (int bMemCopyable = 0, long nGrowBy = 128);
	~CStack();

	int		Push (T& item);		// returns zero on failure, nonzero on success
	int		Pop (T& item);		// returns nonzero if stack not empty, zero if stack is empty after pop
	void	Flush();

protected:

	int		Grow();

	T*		m_pItems;
	long	m_nArraySize;
	long	m_nNextItem;
	long	m_nGrowBy;
	int		m_bMemCopyable;
};

template <class T>
CStack<T>::CStack (int bMemCopyable, long nGrowBy)
{
	m_bMemCopyable = bMemCopyable;
	m_nGrowBy = nGrowBy;

	m_pItems = NULL;
	m_nArraySize = 0;
	m_nNextItem = 0;
}

template <class T>
CStack<T>::~CStack()
{
	Flush();
}

template <class T>
int CStack<T>::Push (T& item)
{
	if (m_nNextItem == m_nArraySize)
	{
		if (!Grow()) return 0;
	}

	if (m_bMemCopyable)
	{
		memcpy (&m_pItems[m_nNextItem++], &item, sizeof(T));
	}
	else
	{
		m_pItems[m_nNextItem++] = item;
	}

	return 1;
}

template <class T>
int CStack<T>::Pop (T& item)
{
	if (m_nNextItem == 0)
	{
		return 0;
	}

	if (m_bMemCopyable)
	{
		memcpy (&item, &m_pItems[--m_nNextItem], sizeof(T));
	}
	else
	{
		item = m_pItems[--m_nNextItem];
	}

	return 1;
}

template <class T>
void CStack<T>::Flush()
{
	debug_deletea(m_pItems);
	m_pItems = NULL;
	m_nArraySize = 0;
	m_nNextItem = 0;
}

template <class T>
int CStack<T>::Grow()
{
	T* pNewArray = debug_newa(T, m_nArraySize + m_nGrowBy);
	if (!pNewArray) return 0;

	if (m_bMemCopyable)
	{
		memcpy (pNewArray, m_pItems, m_nNextItem * sizeof(T));
	}
	else
	{
		for (long i = 0; i < m_nNextItem; i++)
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
