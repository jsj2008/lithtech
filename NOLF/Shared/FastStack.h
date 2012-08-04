
#ifndef _FASTSTACK_H_
#define _FASTSTACK_H_

// Classes

template <class TYPE, int SIZE>
class CFastStack
{
	public :

		inline CFastStack();

		inline void Push(TYPE* pTYPE);
		inline TYPE* Pop();

		inline void Clear() { m_iTYPE = 0; }

		inline bool IsEmpty() { return m_iTYPE == 0; }

		inline int GetSize() { return m_iTYPE; }

	protected :

		int		m_iTYPE;			// The next free TYPE
		TYPE*	m_aTYPEs[SIZE];	// The array of types
};

// Methods

template <class TYPE, int SIZE>
inline CFastStack<TYPE, SIZE>::CFastStack()
{
	m_iTYPE = 0;
}

template <class TYPE, int SIZE>
inline void CFastStack<TYPE, SIZE>::Push(TYPE* pTYPE)
{
	m_aTYPEs[m_iTYPE++] = pTYPE;
}

template <class TYPE, int SIZE>
inline TYPE* CFastStack<TYPE, SIZE>::Pop()
{
	return m_aTYPEs[--m_iTYPE];
}

#endif
