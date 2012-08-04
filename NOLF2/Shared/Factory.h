// ----------------------------------------------------------------------- //
//
// MODULE  : Factory.h
//
// PURPOSE : Factory definition
//
// CREATED : 05.12.1999
//
// ----------------------------------------------------------------------- //

#ifndef __FACTORY_H__
#define __FACTORY_H__

#define FACTORY_NEW(fact)\
	CFactory<##fact##>::Create();

#define FACTORY_DELETE(fact)\
	fact->Destroy();

#define DEFINE_FACTORY_CLASS(fact)\
	public CFactory<##fact##>

#define	DEFINE_ABSTRACT_FACTORY_METHODS(fact) \
	public:\
		virtual void Constructor();\
		virtual void Destructor();\
		virtual void Destroy() {}\
	private:

#define	DEFINE_FACTORY_METHODS(fact) \
	public:\
	virtual void Constructor();\
	virtual void Destructor();\
	virtual void Destroy() { CFactory<##fact##>::Destroy((##fact##*)this); }\
	private:

#define IMPLEMENT_FACTORY(fact, size)\
	CFactory<##fact##>::CCleaner CFactory<##fact##>::s_Cleaner;\
	int CFactory<##fact##>::s_cTYPE;\
	int CFactory<##fact##>::s_iCursor;\
	##fact##** CFactory<##fact##>::s_aPTYPE = CFactory<##fact##>::Initialize(size);

template<class TYPE> class CFactory
{
	typedef TYPE* PTYPE;	// For clarity

	public : // Public methods

		// Methods on the shared factory data

		static inline TYPE* Create();

		// Methods per instance of a factory instantiated class

		virtual void Constructor() = 0;
		virtual void Destructor() = 0;

		static void Destroy(TYPE* pTYPE);

	private : // Private methods

		inline int GetFactoryIndex() { return m_iPTYPE; }
		inline void SetFactoryIndex(int iPTYPE) { m_iPTYPE = iPTYPE; }

		static inline bool Resize(int cTYPE);
		static inline PTYPE* Initialize(int cTYPE);
		static inline void AssertValid();

	private : // Private inner classes

		class CCleaner
		{
			public :

				inline ~CCleaner()
				{
					if ( m_aPTYPE )
					{
						// Delete all the allocated TYPEs and the TYPE* array

						for ( int iPTYPE = 0 ; iPTYPE < m_cTYPE ; iPTYPE++ )
						{
							debug_delete(m_aPTYPE[iPTYPE]);
							m_aPTYPE[iPTYPE] = NULL;
						}

						debug_deletea(m_aPTYPE);
						m_aPTYPE = NULL;
						m_cTYPE = 0;
					}
				}

				PTYPE*	m_aPTYPE;
				int		m_cTYPE;
		};

	private : // Private member variables

		static PTYPE*	s_aPTYPE;
		static int		s_cTYPE;
		static int		s_iCursor;
		static CCleaner	s_Cleaner;
		int				m_iPTYPE;
};

template<class TYPE>
TYPE* CFactory<TYPE>::Create()
{
	if ( 0 == s_cTYPE )
	{
		// We started with 0 types, so allocate one

		if ( !Resize(1) )
		{
			// Couldn't grow the array, allocation failed.

			return NULL;
		}
	}
	else if ( s_iCursor >= s_cTYPE )
	{
		// We ran out of TYPE*s, and need to grow the array (we grow by 2x)

		if ( !Resize(s_cTYPE*2) )
		{
			// Couldn't grow the array, allocation failed.

			return NULL;
		}
	}

	// Clear it out before we return it

	((CFactory<TYPE>*)s_aPTYPE[s_iCursor])->Constructor();

	// Return the next available TYPE*

#ifdef _DEBUG
	AssertValid();
#endif

	return s_aPTYPE[s_iCursor++];
}

template<class TYPE>
void CFactory<TYPE>::Destroy(TYPE* pTYPE)
{
	if ( !s_aPTYPE ) return;

	int iPTYPE = ((CFactory<TYPE>*)pTYPE)->GetFactoryIndex();

	_ASSERT(s_iCursor > 0);
	_ASSERT(iPTYPE >= 0 && iPTYPE <= s_cTYPE);

	TYPE* pTemp = s_aPTYPE[--s_iCursor];
	s_aPTYPE[s_iCursor] = s_aPTYPE[iPTYPE];
	((CFactory<TYPE>*)s_aPTYPE[s_iCursor])->SetFactoryIndex(s_iCursor);
	s_aPTYPE[iPTYPE] = pTemp;
	((CFactory<TYPE>*)s_aPTYPE[iPTYPE])->SetFactoryIndex(iPTYPE);

	((CFactory<TYPE>*)pTYPE)->Destructor();

#ifdef _DEBUG
	AssertValid();
#endif

	return;
}

template<class TYPE>
TYPE** CFactory<TYPE>::Initialize(int cTYPE)
{
	typedef TYPE* PTYPE;	// For clarity

	// If we were already initted then either someone explicitly Init'd us (bad)
	// or two classes are sharing the same factory (WORSE).

	_ASSERT(!s_aPTYPE);
	if ( s_aPTYPE ) return s_aPTYPE;

	PTYPE* aPTYPE;

	// Allocate a TYPE* array and creat the TYPEs if a valid size is specified

	if ( cTYPE > 0 )
	{
		aPTYPE = debug_newa(PTYPE, cTYPE);

		for ( int iPTYPE = 0 ; iPTYPE < cTYPE ; iPTYPE++ )
		{
			aPTYPE[iPTYPE] = debug_new(TYPE);
			((CFactory<TYPE>*)aPTYPE[iPTYPE])->SetFactoryIndex(iPTYPE);
		}
	}
	else
	{
		aPTYPE = NULL;
		cTYPE = 0;
	}

	// Set the Cleaner information

	s_Cleaner.m_aPTYPE = aPTYPE;
	s_Cleaner.m_cTYPE = cTYPE;

	// Set our internal information

	s_aPTYPE = aPTYPE;
	s_cTYPE = cTYPE;
	s_iCursor = 0;

	return aPTYPE;
}

template<class TYPE>
bool CFactory<TYPE>::Resize(int cTYPE)
{
	// We can't go smaller than our current amount of allocated TYPE*s

	if ( cTYPE < s_iCursor ) return false;

	// If we're changing to the same size, it's trivial.

	if ( cTYPE == s_cTYPE ) return true;

	typedef TYPE* PTYPE;	// For clarity

	// Allocate a new TYPE* array of the appropriate size

	PTYPE* aPTYPE = debug_newa(PTYPE, cTYPE);

	// If we're growing...

	if ( cTYPE > s_cTYPE )
	{
		// Copy all the existing TYPE* array into the new one

        int iPTYPE;
        for ( iPTYPE = 0 ; iPTYPE < s_cTYPE ; iPTYPE++ )
		{
			aPTYPE[iPTYPE] = s_aPTYPE[iPTYPE];
			((CFactory<TYPE>*)aPTYPE[iPTYPE])->SetFactoryIndex(iPTYPE);
		}

		// Allocate all the additional TYPEs in the new, larger array

		for ( iPTYPE = s_cTYPE ; iPTYPE < cTYPE ; iPTYPE++ )
		{
			aPTYPE[iPTYPE] = debug_new(TYPE);
			((CFactory<TYPE>*)aPTYPE[iPTYPE])->SetFactoryIndex(iPTYPE);
		}
	}
	else
	{
		// Copy the number of TYPE*s into the new array as requested

        int iPTYPE;
        for ( iPTYPE = 0 ; iPTYPE < cTYPE ; iPTYPE++ )
		{
			aPTYPE[iPTYPE] = s_aPTYPE[iPTYPE];
			((CFactory<TYPE>*)aPTYPE[iPTYPE])->SetFactoryIndex(iPTYPE);
		}

		// Delete all the old TYPEs past the end of the new TYPE* array

		for ( iPTYPE = s_cTYPE ; iPTYPE < cTYPE ; iPTYPE++ )
		{
			debug_delete(aPTYPE[iPTYPE]);
			aPTYPE[iPTYPE] = NULL;
		}
	}

	// Delete our old TYPE* array

	debug_deletea(s_aPTYPE);

	// Set the new Cleaner information

	s_Cleaner.m_aPTYPE = aPTYPE;
	s_Cleaner.m_cTYPE = cTYPE;

	// Set our new internal information. The cursor will not change.

	s_aPTYPE = aPTYPE;
	s_cTYPE = cTYPE;

	return true;
}

template<class TYPE>
void CFactory<TYPE>::AssertValid()
{
	for ( int iPTYPE = 0 ; iPTYPE < s_cTYPE ; iPTYPE++ )
	{
		_ASSERT(((CFactory<TYPE>*)s_aPTYPE[iPTYPE])->GetFactoryIndex() == iPTYPE);
	}
}

#endif