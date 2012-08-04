
#ifndef _FASTHEAP_H_
#define _FASTHEAP_H_

/* 

  ...to use a CFastHeap for heaping CFoo*'s , define a class like:

struct CompareFunction
{
	inline bool operator()(const CFoo* pFoo1, const CFoo* pFoo2) const
	{
		return pFoo1->GetHeapPriority() > pFoo2->GetHeapPriority();
	}
};

  ...the overloaded() operator on CompareFunction should return true if
  pFoo1's heap key is greater than pFoo2's heap key.
 
  ...and declare the fast heap like:

CFastHeap<CFoo, 1024, CompareFunction> heap;

  ...you may not push more than 1024 CFoo*'s onto the heap. 

*/


// Classes

template <class TYPE, int SIZE, class COMPARE>
class CFastHeap
{
	public :

		inline CFastHeap();

		inline void Push(TYPE* pTYPE);
		inline TYPE* Pop();
		inline void Update(TYPE* pTYPE);

		inline void Clear() { m_iTYPE = 0; }

		inline bool IsEmpty() { return m_iTYPE == 0; }

		inline int GetSize() { return m_iTYPE; }

	protected :

		inline void Heapify(int iTYPE);

	protected :

		int		m_iTYPE;			// The next free TYPE
		TYPE*	m_aTYPEs[SIZE+1];	// The array of types
};

// Macros

#define FASTHEAP_PARENT(x)	(x >> 1)
#define FASTHEAP_LEFT(x)	(x << 1)
#define FASTHEAP_RIGHT(x)	((x << 1) + 1)

// Methods

template <class TYPE, int SIZE, class COMPARE>
inline CFastHeap<TYPE, SIZE, COMPARE>::CFastHeap()
{
	m_iTYPE = 0;
}

template <class TYPE, int SIZE, class COMPARE>
inline void CFastHeap<TYPE, SIZE, COMPARE>::Push(TYPE* pTYPE)
{
	COMPARE comp;
	int iTYPE = ++m_iTYPE;
	while ( iTYPE > 1 && comp(pTYPE, m_aTYPEs[FASTHEAP_PARENT(iTYPE)]) )
	{
		m_aTYPEs[iTYPE] = m_aTYPEs[FASTHEAP_PARENT(iTYPE)];
		iTYPE = FASTHEAP_PARENT(iTYPE);
	}
	m_aTYPEs[iTYPE] = pTYPE;
}

template <class TYPE, int SIZE, class COMPARE>
inline TYPE* CFastHeap<TYPE, SIZE, COMPARE>::Pop()
{
	TYPE* pTYPE = m_aTYPEs[1];
	m_aTYPEs[1] = m_aTYPEs[m_iTYPE];
	m_iTYPE--;
	Heapify(1);
	return pTYPE;
}

template <class TYPE, int SIZE, class COMPARE>
inline void CFastHeap<TYPE, SIZE, COMPARE>::Update(TYPE* pTYPE)
{
	for ( int iTYPE = 1 ; iTYPE < m_iTYPE ; iTYPE++ )
	{
		if ( m_aTYPEs[iTYPE] == pTYPE )
		{
			m_aTYPEs[iTYPE] = m_aTYPEs[m_iTYPE];
			m_iTYPE--;
			Heapify(iTYPE);
		}
	}
	
	Push(pTYPE);
}

template <class TYPE, int SIZE, class COMPARE>
inline void CFastHeap<TYPE, SIZE, COMPARE>::Heapify(int iTYPE)
{
	COMPARE comp;
	int iLeft = FASTHEAP_LEFT(iTYPE);
	int iRight = FASTHEAP_RIGHT(iTYPE);
	int iLargest;
	if ( iLeft <= m_iTYPE && comp(m_aTYPEs[iLeft], m_aTYPEs[iTYPE]) )
	{
		iLargest = iLeft;
	}
	else
	{
		iLargest = iTYPE;
	}
	if ( iRight <= m_iTYPE && comp(m_aTYPEs[iRight], m_aTYPEs[iLargest]) )
	{
		iLargest = iRight;
	}
	if ( iLargest != iTYPE )
	{
		TYPE* pTemp;
		pTemp = m_aTYPEs[iLargest];
		m_aTYPEs[iLargest] = m_aTYPEs[iTYPE];
		m_aTYPEs[iTYPE] = pTemp;
		Heapify(iLargest);
	}
}

#endif
