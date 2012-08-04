
// These work just like StructBanks but are for C++ objects that need their
// constructors and destructors called.

#ifndef __OBJECT_BANK_H__
#define __OBJECT_BANK_H__


	#include "struct_bank.h"


	#define DEFAULT_OBJECT_CACHE_SIZE 32
	

	class DummyThingForConstructor
	{
		public:
	};


	inline void* operator new(size_t size, DummyThingForConstructor *pThing, void *ptr)
	{
		return ptr;
	}

	#if _MSC_VER != 1100
		inline void operator delete(void *pDataPtr, DummyThingForConstructor *pThing, void *ptr)
		{
		}
	#endif


	class NullCS
	{
	public:
		void	Enter() {}
		void	Leave() {}
	};


	template<class T, class CS=NullCS>
	class ObjectBank
	{
	public:

					ObjectBank();
					ObjectBank(unsigned long cacheSize, unsigned long preAllocate=0);
					~ObjectBank();

		void		Init(unsigned long cacheSize, unsigned long preAllocate=0);
		void		Term();

		// Set the cache size (in numbers of objects).  Default is DEFAULT_OBJECT_CACHE_SIZE.
		void		SetCacheSize(unsigned long size);

		T*			Allocate();
		void		Free(T *pObj);

	public:

		StructBank	m_Bank;
		CS			m_CS;
	};


	template<class T, class CS>
	inline ObjectBank<T, CS>::ObjectBank()
	{
		sb_Init(&m_Bank, sizeof(T), DEFAULT_OBJECT_CACHE_SIZE);
	}

	template<class T, class CS>
	inline ObjectBank<T, CS>::ObjectBank(unsigned long cacheSize, unsigned long preAllocate)
	{
		sb_Init2(&m_Bank, sizeof(T), cacheSize, preAllocate);
	}

	template<class T, class CS>
	inline ObjectBank<T, CS>::~ObjectBank()
	{
		sb_Term2(&m_Bank, 1);
	}

	template<class T, class CS>
	inline void ObjectBank<T, CS>::Init(unsigned long cacheSize, unsigned long preAllocate)
	{
		m_CS.Enter();
			sb_Term2(&m_Bank, 1);
			sb_Init2(&m_Bank, sizeof(T), cacheSize, preAllocate);
		m_CS.Leave();
	}

	template<class T, class CS>
	inline void ObjectBank<T, CS>::Term()
	{
		m_CS.Enter();
			sb_Term2(&m_Bank, 1);
			sb_Init(&m_Bank, sizeof(T), DEFAULT_OBJECT_CACHE_SIZE);
		m_CS.Leave();
	}

	template<class T, class CS>
	inline void ObjectBank<T, CS>::SetCacheSize(unsigned long size)
	{
		m_CS.Enter();
			m_Bank.m_CacheSize = size;
		m_CS.Leave();
	}

	template<class T, class CS>
	inline T* ObjectBank<T, CS>::Allocate()
	{
		T *pRet;

		m_CS.Enter();
			if(pRet = (T*)sb_Allocate(&m_Bank))
			{
				::new((DummyThingForConstructor*)0, pRet) T;
			}
		m_CS.Leave();

		return pRet;
	}

	template<class T, class CS>
	inline void ObjectBank<T, CS>::Free(T *pObj)
	{
		m_CS.Enter();
			pObj->~T();
			sb_Free(&m_Bank, pObj);
		m_CS.Leave();
	}



#endif  // __OBJECT_BANK_H__




