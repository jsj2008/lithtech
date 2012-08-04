
#ifndef _FASTSTACK_H_
#define _FASTSTACK_H_

// Classes

template <class TYPE, int SIZE>
class CFastStack
{
	public :

		inline CFastStack();

		inline void Push(TYPE type);
		inline TYPE Pop();

		inline TYPE Top();
		inline TYPE At(int iTYPE);

		inline void Clear() { m_iTYPE = 0; }

		inline bool IsEmpty() { return m_iTYPE == 0; }

		inline int GetSize() { return m_iTYPE; }

		inline void Load(ILTMessage_Read *pMsg, void (*)(ILTMessage_Read *pMsg, TYPE& type));
		inline void Save(ILTMessage_Write *pMsg, void (*)(ILTMessage_Write *pMsg, TYPE& type));

	protected :

		int		m_iTYPE;			// The next free TYPE
		TYPE	m_aTYPEs[SIZE];	// The array of types
};

// Methods

template <class TYPE, int SIZE>
inline CFastStack<TYPE, SIZE>::CFastStack()
{
	m_iTYPE = 0;
}

template <class TYPE, int SIZE>
inline void CFastStack<TYPE, SIZE>::Push(TYPE type)
{
	m_aTYPEs[m_iTYPE++] = type;
}

template <class TYPE, int SIZE>
inline TYPE CFastStack<TYPE, SIZE>::Pop()
{
	return m_aTYPEs[--m_iTYPE];
}

template <class TYPE, int SIZE>
inline TYPE CFastStack<TYPE, SIZE>::Top()
{
	return m_aTYPEs[m_iTYPE-1];
}

template <class TYPE, int SIZE>
inline TYPE CFastStack<TYPE, SIZE>::At(int iTYPE)
{
	return m_aTYPEs[iTYPE];
}

template <class TYPE, int SIZE>
inline void CFastStack<TYPE, SIZE>::Load(ILTMessage_Read *pMsg, void (*FnLoad)(ILTMessage_Read *pMsg, TYPE& type))
{
	m_iTYPE = pMsg->Readint32();
	for ( uint32 iType = 0 ; iType < SIZE ; iType++ )
	{
		FnLoad(pMsg, m_aTYPEs[iType]);
	}
}

template <class TYPE, int SIZE>
inline void CFastStack<TYPE, SIZE>::Save(ILTMessage_Write *pMsg, void (*FnSave)(ILTMessage_Write *pMsg, TYPE& type))
{
	pMsg->Writeint32( m_iTYPE );
	for ( uint32 iType = 0 ; iType < SIZE ; iType++ )
	{
		FnSave(pMsg, m_aTYPEs[iType]);
	}
}

#endif
