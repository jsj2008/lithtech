// ----------------------------------------------------------------------- //
//
// MODULE  : generalheapgroup.h
//
// PURPOSE : this class has the same functionality as the CGeneralHeap class
//			 except that it will grow allocating additional general heaps
//			 as needed
//
// CREATED : Jan-5-2001
//
// ----------------------------------------------------------------------- //

#ifndef __GENERALHEAPGROUP_H__
#define __GENERALHEAPGROUP_H__

#ifndef __GENERALHEAP_H__
#include "generalheap.h"
#endif

class CGeneralHeapGroup 
{
public:
	// simple constructor
	CGeneralHeapGroup() 
	{ 
// these can't be initialized because they sometimes get initialized
// after the class is set up
//		m_bInitialized = false; 
//		m_pHeapList = NULL;
	};

	// constructor which also initializes the class
	CGeneralHeapGroup(
		uint32 nInitialHeapSize, 
		uint32 nGrowHeapSize,
		uint32 nHeapAlign)
	{ 
		m_bInitialized = false; 
		Init(nInitialHeapSize,nGrowHeapSize,nHeapAlign); 
	};

	// desctructor
	~CGeneralHeapGroup() 
	{ 
		Term(); 
	};

	// function to initialize the class
	// this must be called before any other calls 
	inline bool Init(
		uint32 nInitialHeapSize, 
		uint32 nGrowHeapSize,
		uint32 nHeapAlign);

	// terminate the class
	inline void Term();

	// allocate a piece of memory from the fixed heap
	inline void* Alloc(uint32 nSize);

	// free a piece of memory from the fixed heap
	inline bool Free(void* pFreeMem);

	// check if this memory is in this heap
	inline bool InHeap(void* pMem);

	// get the size of a piece of allocated memory
	inline uint32 GetSize(void* pMem);

	// get the general heap that this memory is in
	inline CGeneralHeap* GetGeneralHeap(void* pMem);

	// memory allocations for this class will go though system malloc and free
    void* operator new(size_t size) {	return malloc(size); }
    void operator delete(void* p) { free(p); };

private:

	// struct to hold each of the little fixed heaps in this group
	struct CGeneralHeapItem
	{
		CGeneralHeap m_heap;
		void* m_pMem;
		CGeneralHeapItem* m_pNext;

		// memory allocations for this class will go though system malloc and free
	    void* operator new(size_t size) {	return malloc(size); }
	    void operator delete(void* p) { free(p); };
	};

	// add a new heap
	inline bool AddHeap(uint32 nHeapSize, uint32 nHeapAlign);

	// true if this class is initialized
	bool m_bInitialized;

	// contains the number of items that will be in each little fixed heap
	// that is allocated after the first one
	uint32 m_nGrowHeapSize;

	// alignment for heap
	uint32 m_nHeapAlign;

	// contains a pointer to the first item in the heap list
	CGeneralHeapItem* m_pHeapList;
};


inline bool CGeneralHeapGroup::Init(
		uint32 nInitialHeapSize, 
		uint32 nGrowHeapSize,
		uint32 nHeapAlign)
{
	// check if we are supposed to allocate initial heap space
	if (nInitialHeapSize > 0)
	{
		if (!AddHeap(nInitialHeapSize, nHeapAlign)) 
		{
			ASSERT(false);
			return false;
		}
	}

	else
	{
		m_pHeapList = NULL;
	}
	
	// save memory grow size in bytes
	m_nGrowHeapSize = nGrowHeapSize;

	// alignment for heap
	m_nHeapAlign = nHeapAlign;

	// class is now initialized
	m_bInitialized = true;

	return true;
};


inline void CGeneralHeapGroup::Term()
{
	// make sure class was initialized
	if (!m_bInitialized) return;

	// go through list and delete all heaps
	while (m_pHeapList != NULL)
	{
		CGeneralHeapItem* pNextHeapList = m_pHeapList->m_pNext;
		m_pHeapList->m_heap.Term();
		free(m_pHeapList->m_pMem);
		delete m_pHeapList;
//		free(m_pHeapList);
		m_pHeapList = pNextHeapList;
	}

	// class is no longer initialized
	m_bInitialized = false;
};


// add a new heap
inline bool CGeneralHeapGroup::AddHeap(
					uint32 nHeapSize,
					uint32 nHeapAlign)
{
	// allocate initial heap item
	CGeneralHeapItem* pNewHeap = new CGeneralHeapItem;
//	CGeneralHeapItem* pNewHeap = (CGeneralHeapItem*)malloc(sizeof(CGeneralHeapItem));
	if (pNewHeap == NULL) 
	{
		ASSERT(false);
		return false;
	}

	// allocate memory for the general heap and round it to even 256 bytes
	pNewHeap->m_pMem = malloc(nHeapSize + 0xff);
	uint8* pHeapMem = (uint8*)(((uint32)((uint8*)pNewHeap->m_pMem) + 0xff) & (~0xff));
	if (pHeapMem == NULL) 
	{
		delete pNewHeap;
		ASSERT(false);
//		free(pNewHeap);
		return false;
	}

	// initialize the general heap
	pNewHeap->m_heap.Init((uint32)pHeapMem, nHeapSize, nHeapAlign);

	// add to heap list
	pNewHeap->m_pNext = m_pHeapList;
	m_pHeapList = pNewHeap;

	return true;
}


inline void* CGeneralHeapGroup::Alloc(uint32 nSize)
{
	ASSERT(m_bInitialized);

	// memory we are going to allocate
	void* pMem = NULL;

	// current heap we are looking at
	CGeneralHeapItem* pCurHeap = m_pHeapList;

	// try to find space in one of the existing heaps
	while (pCurHeap != NULL)
	{
		// try to allocate from this heap
		pMem = pCurHeap->m_heap.Alloc(nSize);
		if (pMem != NULL) 
		{
			// return memory we allocated
			return pMem;
		}
		else
		{
			// get next heap
			pCurHeap = pCurHeap->m_pNext;
		}
	}

	// we found no free memory so allocate another heap
	AddHeap(m_nGrowHeapSize, m_nHeapAlign);

	// allocate memory from new heap
	pMem = m_pHeapList->m_heap.Alloc(nSize);

	// return memory we allocated
	return pMem;
};

inline bool CGeneralHeapGroup::Free(void* pFreeMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CGeneralHeapItem* pCurHeap = m_pHeapList;

	// find which heap this memory belongs to
	while (pCurHeap != NULL)
	{
		// check if memory is in this heap
		if (pCurHeap->m_heap.InHeap(pFreeMem))
		{
			// free memory from heap
			pCurHeap->m_heap.Free(pFreeMem);

			// we are done and we deleted the memory
			return true;
		}
		else
		{
			// get next heap
			pCurHeap = pCurHeap->m_pNext;
		}
	}

	// we didn't find memory so return false
	return false;
};

inline bool CGeneralHeapGroup::InHeap(void* pMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CGeneralHeapItem* pCurHeap = m_pHeapList;

	// find which heap this memory belongs to
	while (pCurHeap != NULL)
	{
		// check if memory is in this heap
		if (pCurHeap->m_heap.InHeap(pMem))
		{
			// we are done and we deleted the memory
			return true;
		}
		else
		{
			// get next heap
			pCurHeap = pCurHeap->m_pNext;
		}
	}

	// we didn't find memory so return false
	return false;
};


// get the size of a piece of allocated memory
inline uint32 CGeneralHeapGroup::GetSize(void* pMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CGeneralHeapItem* pCurHeap = m_pHeapList;

	// find which heap this memory belongs to
	while (pCurHeap != NULL)
	{
		// check if memory is in this heap
		if (pCurHeap->m_heap.InHeap(pMem))
		{
			// we are done and we deleted the memory
			return pCurHeap->m_heap.GetSize(pMem);
		}
		else
		{
			// get next heap
			pCurHeap = pCurHeap->m_pNext;
		}
	}

	// we didn't find memory so return false
	return 0;
};


// get the general heap that this memory is in
inline CGeneralHeap* CGeneralHeapGroup::GetGeneralHeap(void* pMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CGeneralHeapItem* pCurHeap = m_pHeapList;

	// find which heap this memory belongs to
	while (pCurHeap != NULL)
	{
		// check if memory is in this heap
		if (pCurHeap->m_heap.InHeap(pMem))
		{
			// we are done and we deleted the memory
			return &(pCurHeap->m_heap);
		}
		else
		{
			// get next heap
			pCurHeap = pCurHeap->m_pNext;
		}
	}

	// we didn't find memory so return false
	return NULL;
};

#endif
