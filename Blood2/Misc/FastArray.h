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


#ifndef __FASTDYNARRAY_H__
	#define __FASTDYNARRAY_H__


	#include "Memory.h"
	

	// Defines....
	#define CHECK_ARRAY_BOUNDS
	#define CACHE_DEFAULT_SIZE	5


	// Predefined types of arrays.
	#define CFastPtrArray	CFastMoArray<void*>
	#define CFastDWordArray	CFastMoArray<DWORD>
	#define CFastWordArray	CFastMoArray<WORD>
	#define CFastByteArray	CFastMoArray<unsigned char>

	
	template<class T>
	class CFastMoArray
	{
		public:

			// Constructors					
							CFastMoArray() { Init(); }
							CFastMoArray( WORD cacheSize ) { Init(0, cacheSize); }
						
			// Destructor
							~CFastMoArray() { Term(); }

			
			// Member functions
			BOOL			Init(DWORD size = 0, WORD cacheSize = CACHE_DEFAULT_SIZE);
			void			Term() { SetSize(0); }

			// Comparison
			BOOL			Compare( const CFastMoArray<T> &other );
			
			// Assignment
			
			// You should use CopyArray whenever possible.
			BOOL			operator=( const CFastMoArray<T> &other );
			BOOL			CopyArray( const CFastMoArray<T> &other );
			BOOL			AppendArray( const CFastMoArray<T> &other );

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
			DWORD			GetSize() const { return m_nElements; }

			// Sets the cache size
			void			SetCacheSize( WORD size ) { m_WantedCache = size; }

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


	private:

			// Member variables
			
			T		*m_pArray;
			DWORD	m_nElements;

			WORD	m_CacheSize;
			WORD	m_WantedCache;

	};

	
	template<class T>
	BOOL CFastMoArray<T>::Init( DWORD size, WORD cacheSize )
	{
		_InitArray(cacheSize);
		
		return SetSize(size);
	}

	template<class T>
	BOOL CFastMoArray<T>::Compare( const CFastMoArray<T> &other )
	{
		DWORD	i;

		if( m_nElements != other.m_nElements )
			return FALSE;

		for( i=0; i < m_nElements; i++ )
			if( m_pArray[i] != other.m_pArray[i] )
				return FALSE;

		return TRUE;
	}



	template<class T>
	BOOL CFastMoArray<T>::operator=( const CFastMoArray<T> &other )
	{
		return CopyArray(other);
	}


	template<class T>
	BOOL CFastMoArray<T>::CopyArray( const CFastMoArray<T> &other )
	{
		DWORD i;

		if( m_pArray )
			_DeleteAndDestroyArray();

		m_nElements = other.m_nElements;
		m_CacheSize = other.m_CacheSize;
		m_WantedCache = other.m_WantedCache;

		if( m_nElements + m_CacheSize > 0  )
		{
			m_pArray = _AllocateTArray( m_nElements + m_CacheSize );
		}
		else
		{
			m_nElements = 0;
			m_CacheSize = 0;
			m_pArray = NULL;
			return TRUE;
		}

		// Could it allocate the array?
		if( !m_pArray )
		{
			m_nElements = 0;
			m_CacheSize = 0;
			return FALSE;
		}

		for( i=0; i < m_nElements; i++ )
			m_pArray[i] = other.m_pArray[i];

		return TRUE;
	}


	template<class T>
	BOOL CFastMoArray<T>::AppendArray( const CFastMoArray<T> &other )
	{
		DWORD			i;

		for( i=0; i < other; i++ )
			if( !Append(other[i]) )
				return FALSE;

		return TRUE;
	}


	template<class T>
	DWORD CFastMoArray<T>::FindElement( const T &x )
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



	template<class T>
	BOOL CFastMoArray<T>::Insert( DWORD index, const T &toInsert )
	{
		T		*pNewArray;
		DWORD	newSize, i;

		ASSERT( index <= m_nElements );

		// Create a new array (possibly).
		newSize = m_nElements + 1;
		
		//if( newSize >= (m_nElements+m_CacheSize) || m_nElements == 0 )
		if( m_CacheSize == 0 )
		{
			pNewArray = _AllocateTArray( newSize + m_WantedCache );
			if( !pNewArray )
				return FALSE;

			// Copy the old array into the new one, start inserting at index.
			memcpy( pNewArray, m_pArray, sizeof(T)*index );
//			for( i=0; i < index; i++ )
//				pNewArray[i] = m_pArray[i];

			memcpy( &pNewArray[index+1], &m_pArray[index], sizeof(T)*(m_nElements-index) );
//			for( i=index; i < m_nElements; i++ )
//				pNewArray[i+1] = m_pArray[i];
			
			// Free the old array and set our pointer to the new one
			if( m_pArray )
				_DeleteAndDestroyArray();

			m_CacheSize = m_WantedCache;
			m_pArray = pNewArray;
		}
		else
		{
			for( i=m_nElements; i > index; i-- )
				m_pArray[i] = m_pArray[i-1];
			
			ASSERT( m_CacheSize != 0 );
			--m_CacheSize;
		}

		++m_nElements;
		m_pArray[index] = toInsert;

		return TRUE;
	}



	template<class T>
	void CFastMoArray<T>::Remove( DWORD index )
	{
		DWORD	newSize;
//		DWORD	i;
		T		*pNewArray;
		BOOL	bSlideDown = FALSE;
												
	
		ASSERT( index < m_nElements && m_pArray );

		if( m_CacheSize >= (m_WantedCache*2) )
		{
			newSize = m_nElements - 1;
			pNewArray = _AllocateTArray( newSize + m_WantedCache );

			// Make sure it allocated the array .. if it didn't, just have
			// it slide all the elements down (this guarantees that Remove() 
			// won't fail.)
			if( pNewArray )
			{
				memcpy( pNewArray, m_pArray, sizeof(T)*index );
//				for( i=0; i < index; i++ )
//					pNewArray[i] = m_pArray[i];

				memcpy( &pNewArray[index], &m_pArray[index+1], sizeof(T)*(m_nElements-index-1) );
//				for( i=index; i < m_nElements-1; i++ )
//					pNewArray[i] = m_pArray[i+1];

				_DeleteAndDestroyArray();
				m_pArray = pNewArray;

				m_CacheSize = m_WantedCache;
			}
			else
				bSlideDown = TRUE;
		}
		else
			bSlideDown = TRUE;


		if( bSlideDown )
		{
			// Slide them all down one.
			++m_CacheSize;
			
			memmove( &m_pArray[index], &m_pArray[index+1], sizeof(T)*(m_nElements-index-1) );
//			for( i=index; i < m_nElements-1; i++ )
//				m_pArray[i] = m_pArray[i+1];
		}

		--m_nElements;
	}



	template<class T>
	BOOL CFastMoArray<T>::SetSize( DWORD newSize )
	{
		if( m_pArray )
			_DeleteAndDestroyArray();

		m_nElements = newSize;
		if( newSize > 0 )
		{
			m_pArray = _AllocateTArray( newSize + m_WantedCache );
			if( !m_pArray )
			{
				m_nElements = 0;
				m_CacheSize = 0;
				return FALSE;
			}
			
			m_CacheSize = m_WantedCache;
		}

		return TRUE;
	}



	template<class T>
	void CFastMoArray<T>::_InitArray( WORD wantedCache )
	{
		m_pArray = NULL;
		m_nElements = 0;

		m_WantedCache = wantedCache;
		m_CacheSize = 0;
	}



	template<class T>
	T *CFastMoArray<T>::_AllocateTArray( DWORD nElements )
	{
		char	*ptr = new char[nElements * sizeof(T)];
		T		*tPtr = (T*)ptr;

		if( !tPtr )
			return tPtr;

		return tPtr;
	}



	template<class T>
	void CFastMoArray<T>::_DeleteAndDestroyArray()
	{
		void	*voidPointer;

		voidPointer = m_pArray;
		delete voidPointer;
		m_pArray = NULL;
		m_CacheSize = 0;
	}


	
	template<class T>
	void DeleteAndClearArray( T &theArray )
	{
		for( DWORD i=0; i < theArray.GetSize(); i++ )
			delete theArray.Get(i);

		theArray.SetSize(0);
	}


#endif 




