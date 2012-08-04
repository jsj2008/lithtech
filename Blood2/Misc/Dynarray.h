//------------------------------------------------------------------
//
//	FILE	  : DynArray.h
//
//	PURPOSE	  : Caching dynamic arrays used everywhere.
//
//	CREATED	  : 5/1/96
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------


#ifndef __MODYNARRAY_H__
	#define __MODYNARRAY_H__


	#include "Memory.h"
	

	// Defines....
	#define CHECK_ARRAY_BOUNDS
	#define CACHE_DEFAULT_SIZE	5


	// Predefined types of arrays.
	#define CMoPtrArray		CMoArray<void*>
	#define CMoDWordArray	CMoArray<DWORD>
	#define CMoWordArray	CMoArray<WORD>
	#define CMoByteArray	CMoArray<unsigned char>
	#define DYNA_TEMPLATE template<class T, class C>


	// This can be used if you don't want the extra 4 bytes of caching info in the array.
	class NoCache
	{
	public:
		WORD	GetCacheSize() const		{return 0;}
		void	SetCacheSize(WORD size)		{}
		WORD	GetWantedCache() const		{return 0;}
		void	SetWantedCache(WORD size)	{}
	};


	class DefaultCache
	{
	public:
		WORD	GetCacheSize() const	{return m_CacheSize;}
		void	SetCacheSize(WORD val)	{m_CacheSize = val;}
		WORD	GetWantedCache() const	{return m_WantedCache;}
		void	SetWantedCache(WORD val){m_WantedCache = val;}

	private:		
		WORD	m_CacheSize;
		WORD	m_WantedCache;
	};

	
	template<class T, class C=DefaultCache>
	class CMoArray
	{
		public:

			// Constructors					
							CMoArray(CMoArray<T, C> &copyFrom, const T &toAdd)
							{
								Clear();
								Init();
								CopyArray(copyFrom);
								Append(toAdd);
							}

							CMoArray()
							{ 
								Clear();
								Init(); 
							}
							
							CMoArray( WORD cacheSize )
							{
								Clear();
								Init(0, cacheSize); 
							}
						
			// Destructor
							~CMoArray() { Term(); }

			
			// Member functions
			BOOL			Init(DWORD size = 0, WORD cacheSize = CACHE_DEFAULT_SIZE);
			void			Term() { SetSize(0); }

			// Comparison
			BOOL			Compare( const CMoArray<T, C> &other );
			
			// Assignment
			
			// You should use CopyArray whenever possible.
			CMoArray<T, C>&		operator=(const CMoArray<T, C> &other);
			CMoArray<T, C>		operator+(const T &toAdd);
			
			
			BOOL			CopyArray( const CMoArray<T, C> &other );
		
			BOOL			AppendArray( const CMoArray<T, C> &other );

			DWORD			FindElement( const T &x );
			
			BOOL 			Append( const T &toAdd ) { return Insert(m_nElements, toAdd); }
			BOOL 			Insert( DWORD index, const T &toInsert );
			void 			Remove( DWORD index );

			// You can use it like a stack with these...
			BOOL			Push( const T &toAdd )  { return Append(toAdd); }
			void			Pop()					{ ASSERT(m_nElements>0); Remove( m_nElements-1 ); }



			// Accessors
			BOOL			IsValid() { return TRUE; }
			
			// Helpers for if you want to wrap around...
			T&				Last()		const	{ ASSERT(m_nElements>0); return m_pArray[ m_nElements-1 ]; }
			DWORD			LastI()		const	{ ASSERT(m_nElements>0); return m_nElements-1; }
			
			T&				Next( DWORD index ) const { return m_pArray[ NextI(index) ]; }
			T&				Prev( DWORD index ) const { return m_pArray[ PrevI(index) ]; }
			DWORD			NextI( DWORD i ) const
			{
				#ifdef CHECK_ARRAY_BOUNDS
					ASSERT( m_nElements > 0 );
				#endif

				if( i < (m_nElements-1) )
					return i+1;
				else
					return 0;
			}

			DWORD			PrevI( DWORD i ) const
			{
				#ifdef CHECK_ARRAY_BOUNDS
					ASSERT( m_nElements > 0 );
				#endif
				
				if( i == 0 )
					return m_nElements - 1;
				else
					return i-1;
			}

			// Number of elements
							operator unsigned long() const { return (unsigned long)m_nElements; }

			// Array-like access.
			T&				operator[]( const DWORD index ) const { return Get(index); }
			
			// Returns FALSE if there isn't enough memory.
			BOOL 			SetSize( DWORD newSize );
			BOOL 			SetSizeInit( DWORD newSize, T &val );
			BOOL 			SetSizeInit2( DWORD newSize, T val );

			// Same as SetSize but preserves the old contents (ie: sizing from 8 to 4 preserves
			// the first 4 and sizing from 4 to 8 preserves the first 4).
			BOOL			NiceSetSize(DWORD newSize) {return InternalNiceSetSize(newSize, FALSE);}
			BOOL			Fast_NiceSetSize(DWORD newSize) {return InternalNiceSetSize(newSize, TRUE);}

			DWORD			GetSize() const { return m_nElements; }

			// Sets the cache size
			void			SetCacheSize( WORD size )
			{
				m_Cache.SetWantedCache(size);
			}

			// Get and set
			T&				Get( DWORD index ) const
			{
				#ifdef CHECK_ARRAY_BOUNDS
					ASSERT( index < m_nElements );
				#endif
				return m_pArray[index];
			}

			void 			Set( DWORD index, T &data )
			{
				#ifdef CHECK_ARRAY_BOUNDS
					ASSERT( index < m_nElements );
				#endif
				m_pArray[index] = data;
			}

			// Returns a pointer to the internal array..
			T*				GetArray()	{ return m_pArray; }


	// Accessors for MFC compatibility.
	public:

			T&				GetAt( DWORD index ) const		{ return Get(index); }
			void			SetAt( DWORD index, T data )	{ Set(index, data); }

			void			RemoveAll()						{ SetSize(0); }
			BOOL 			Add( const T &toAdd )			{ return Insert(m_nElements, toAdd); }


	private:

			void 	_InitArray( WORD wantedCache );
			void	_DeleteAndDestroyArray();
			T*		_AllocateTArray( DWORD nElements );

			BOOL	InternalNiceSetSize( DWORD newSize, BOOL bFast );


	private:

			void	Clear()
			{
				m_pArray = 0;
				m_nElements = 0;
				m_Cache.SetCacheSize(0);
				m_Cache.SetWantedCache(0);
			}

			// Member variables
			
			T		*m_pArray;
			DWORD	m_nElements;

			C		m_Cache;

	};

	
	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::Init( DWORD size, WORD cacheSize )
	{
		Term();
		_InitArray(cacheSize);
		
		return SetSize(size);
	}

	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::Compare( const CMoArray<T, C> &other )
	{
		DWORD	i;

		if( m_nElements != other.m_nElements )
			return FALSE;

		for( i=0; i < m_nElements; i++ )
			if( m_pArray[i] != other.m_pArray[i] )
				return FALSE;

		return TRUE;
	}



	template<class T, class C>
	CMoArray<T, C>& CMoArray<T, C>::operator=( const CMoArray<T, C> &other )
	{
		CopyArray(other);
		return *this;
	}

	
	DYNA_TEMPLATE
	CMoArray<T, C> CMoArray<T, C>::operator+(const T &toAdd)
	{
		return CMoArray<T, C>(*this, toAdd);
	}


	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::CopyArray(const CMoArray<T,C> &other)
	{
		DWORD i;

		if( m_pArray )
			_DeleteAndDestroyArray();

		m_nElements = other.m_nElements;
		m_Cache.SetCacheSize(other.m_Cache.GetCacheSize());
		m_Cache.SetWantedCache(other.m_Cache.GetWantedCache());

		if( m_nElements + m_Cache.GetCacheSize() > 0  )
		{
			m_pArray = _AllocateTArray( m_nElements + m_Cache.GetCacheSize() );
		}
		else
		{
			m_nElements = 0;
			m_Cache.SetCacheSize(0);
			m_pArray = NULL;
			return TRUE;
		}

		// Could it allocate the array?
		if( !m_pArray )
		{
			m_nElements = 0;
			m_Cache.SetCacheSize(0);
			return FALSE;
		}

		for( i=0; i < m_nElements; i++ )
			m_pArray[i] = other.m_pArray[i];

		return TRUE;
	}


	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::AppendArray( const CMoArray<T, C> &other )
	{
		DWORD			i;

		for( i=0; i < other; i++ )
			if( !Append(other[i]) )
				return FALSE;

		return TRUE;
	}


	DYNA_TEMPLATE
	DWORD CMoArray<T, C>::FindElement( const T &x )
	{
		DWORD i, ret = BAD_INDEX;

		for( i=0; i < m_nElements; i++ )
		{
			if( m_pArray[i] == x )
			{
				ret = i;
				break;
			}
		}

		return ret;
	}



	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::Insert( DWORD index, const T &toInsert )
	{
		T		*pNewArray;
		DWORD	newSize, i;

		ASSERT( index <= m_nElements );
		if(index > m_nElements)
			return FALSE;

		// Create a new array (possibly).
		newSize = m_nElements + 1;
		
		//if( newSize >= (m_nElements+m_CacheSize) || m_nElements == 0 )
		if( m_Cache.GetCacheSize() == 0 )
		{
			pNewArray = _AllocateTArray( newSize + m_Cache.GetWantedCache() );
			if( !pNewArray )
				return FALSE;

			// Copy the old array into the new one, start inserting at index.
			for( i=0; i < index; i++ )
				pNewArray[i] = m_pArray[i];

			for( i=index; i < m_nElements; i++ )
				pNewArray[i+1] = m_pArray[i];
			
			// Insert the new item into the array
			pNewArray[index] = toInsert;

			// Free the old array and set our pointer to the new one
			if( m_pArray )
				_DeleteAndDestroyArray();

			m_Cache.SetCacheSize(m_Cache.GetWantedCache());
			m_pArray = pNewArray;
		}
		else
		{
			for( i=m_nElements; i > index; i-- )
				m_pArray[i] = m_pArray[i-1];
			
			m_Cache.SetCacheSize(m_Cache.GetCacheSize() - 1);

			m_pArray[index] = toInsert;
		}

		++m_nElements;		

		return TRUE;
	}



	DYNA_TEMPLATE
	void CMoArray<T, C>::Remove( DWORD index )
	{
		DWORD	newSize;
		DWORD	i;
		T		*pNewArray;
		BOOL	bSlideDown = FALSE;
												
	
		ASSERT( index < m_nElements && m_pArray );

		if( m_Cache.GetCacheSize() >= (m_Cache.GetWantedCache()*2) )
		{
			newSize = m_nElements - 1;
			pNewArray = _AllocateTArray( newSize + m_Cache.GetWantedCache() );

			// Make sure it allocated the array .. if it didn't, just have
			// it slide all the elements down (this guarantees that Remove() 
			// won't fail.)
			if( pNewArray )
			{
				for( i=0; i < index; i++ )
					pNewArray[i] = m_pArray[i];

				for( i=index; i < m_nElements-1; i++ )
					pNewArray[i] = m_pArray[i+1];

				_DeleteAndDestroyArray();
				m_pArray = pNewArray;

				m_Cache.SetCacheSize(m_Cache.GetWantedCache());
			}
			else
				bSlideDown = TRUE;
		}
		else
			bSlideDown = TRUE;


		if( bSlideDown )
		{
			// Slide them all down one.
			m_Cache.SetCacheSize(m_Cache.GetCacheSize() + 1);
			
			for( i=index; i < m_nElements-1; i++ )
				m_pArray[i] = m_pArray[i+1];
		}

		--m_nElements;
	}



	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::SetSize( DWORD newSize )
	{
		if( m_pArray )
			_DeleteAndDestroyArray();

		m_nElements = newSize;
		if( newSize > 0 )
		{
			m_pArray = _AllocateTArray( newSize + m_Cache.GetWantedCache() );
			if( !m_pArray )
			{
				m_nElements = 0;
				m_Cache.SetCacheSize(0);
				return FALSE;
			}
			
			m_Cache.SetCacheSize(m_Cache.GetWantedCache());
		}

		return TRUE;
	}


	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::SetSizeInit( DWORD newSize, T &val )
	{
		DWORD i;

		if(!SetSize(newSize))
			return FALSE;

		for(i=0; i < GetSize(); i++)
		{
			m_pArray[i] = val;
		}
		
		return TRUE;
	}


	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::SetSizeInit2( DWORD newSize, T val )
	{
		return SetSizeInit(newSize, val);
	}


	DYNA_TEMPLATE
	BOOL CMoArray<T, C>::InternalNiceSetSize( DWORD newSize, BOOL bFast )
	{
		T *pNewArray;
		DWORD i, nToCopy;

		// Trivial reject..
		if(newSize < m_nElements && ((DWORD)m_Cache.GetCacheSize() + (m_nElements - newSize)) <= 0xFFFF)
		{
			m_Cache.SetCacheSize(m_Cache.GetCacheSize() + (WORD)(m_nElements - newSize));
			m_nElements = newSize;
			return TRUE;
		}
		else if(newSize > m_nElements && (m_nElements + (DWORD)m_Cache.GetCacheSize()) >= newSize)
		{
			m_Cache.SetCacheSize(m_Cache.GetCacheSize() - (WORD)(newSize - m_nElements));
			m_nElements = newSize;
			return TRUE;
		}

		pNewArray = _AllocateTArray(newSize + m_Cache.GetWantedCache());
		if(!pNewArray)
			return FALSE;

		nToCopy = m_nElements;
		if(nToCopy > newSize)
			nToCopy = newSize;

		// Copy as many elements as we can.
		if(bFast)
		{
			memcpy(pNewArray, m_pArray, sizeof(T)*nToCopy);
		}
		else
		{
			for(i=0; i < nToCopy; i++)
			{
				pNewArray[i] = m_pArray[i];
			}
		}
		
		// Get rid of the old array and point at the new one.
		_DeleteAndDestroyArray();
		m_pArray = pNewArray;
		m_nElements = newSize;
		m_Cache.SetCacheSize(m_Cache.GetWantedCache());

		return TRUE;
	}


	DYNA_TEMPLATE
	void CMoArray<T, C>::_InitArray( WORD wantedCache )
	{
		m_pArray = NULL;
		m_nElements = 0;

		m_Cache.SetWantedCache(wantedCache);
		m_Cache.SetCacheSize(0);
	}



	DYNA_TEMPLATE
	T *CMoArray<T, C>::_AllocateTArray( DWORD nElements )
	{
		T *tPtr = new T[nElements];

		return tPtr;
	}



	DYNA_TEMPLATE
	void CMoArray<T, C>::_DeleteAndDestroyArray()
	{
		if(m_pArray)
		{
			delete [] m_pArray;
			m_pArray = NULL;
		}

		m_Cache.SetCacheSize(0);
	}


	template<class T, class C, class ToAlloc>
	BOOL AllocateArray(CMoArray<T, C> &theArray, ToAlloc *pToAlloc)
	{
		DWORD i;

		for(i=0; i < theArray; i++)
		{
			theArray[i] = new ToAlloc;
			if(!theArray[i])
				return FALSE;
		}

		return TRUE;
	}


	template<class T>
	void DeleteAndClearArray( T &theArray )
	{
		DWORD i;

		for( i=0; i < theArray.GetSize(); i++ )
		{
			if(theArray[i])
				delete theArray[i];
		}

		theArray.SetSize(0);
	}


#endif 




