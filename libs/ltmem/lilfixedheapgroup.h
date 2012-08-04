// ----------------------------------------------------------------------- //
//
// MODULE  : lilfixedheapgroup.h
//
// PURPOSE : this class has the same functionality as the CLilFixedHeap class
//			 except that it will grow allocating additional fixed heaps
//			 as needed
//
// CREATED : Nov-1-2001
//
// ----------------------------------------------------------------------- //

#ifndef __LILFIXEDHEAPGROUP_H__
#define __LILFIXEDHEAPGROUP_H__

#ifndef _LILFIXEDHEAP_H_
#include "lilfixedheap.h"
#endif

class CLilFixedHeapGroup 
{
public:
	// simple constructor
	CLilFixedHeapGroup() 
	{ 
// these can't be initialized because they sometimes get initialized
// after the class is set up
//		m_bInitialized = false; 
	};

	// constructor which also initializes the class
	CLilFixedHeapGroup(
		uint32 nElemSize, 
		uint32 nInitialNumElements, 
		uint32 nGrowNumElements)
	{ 
		m_bInitialized = false; 
		Init(nElemSize,nInitialNumElements,nGrowNumElements); 
	};

	// desctructor
	~CLilFixedHeapGroup() 
	{ 
		Term(); 
	};

	// function to initialize the class
	// this must be called before any other calls 
	inline bool Init(
		uint32 nElemSizeBytes, 
		uint32 nInitialNumElements, 
		uint32 nGrowNumElements);

	// terminate the class
	inline void Term();

	// allocate a piece of memory from the fixed heap
	inline void* Alloc();

	// free a piece of memory from the fixed heap
	inline bool Free(void* pFreeMem);

	// check if this memory is in this heap
	inline bool InHeap(void* pMem);

	// memory allocations for this class will go though system malloc and free
    void* operator new(size_t size) {	return malloc(size); }
    void operator delete(void* p) { free(p); };

private:

	// struct to hold each of the little fixed heaps in this group
	struct CLilFixedHeapItem
	{
		CLilFixedHeap m_heap;
		CLilFixedHeapItem* m_pNext;

		// memory allocations for this class will go though system malloc and free
	    void* operator new(size_t size) {	return malloc(size); }
		void operator delete(void* p) { free(p); };
	};

	// true if this class is initialized
	bool m_bInitialized;

	// contains the number of bytes in a piece of heap memory
	uint32 m_nElemSizeBytes;

	// contains the number of items that will be in each little fixed heap
	// that is allocated after the first one
	uint32 m_nGrowElemSize;

	// contains a pointer to the first item in the heap list
	CLilFixedHeapItem* m_pHeapList;
};


inline bool CLilFixedHeapGroup::Init(
	uint32 nElemSizeBytes, 
	uint32 nInitialNumElements,
	uint32 nGrowNumElements)
{
	// check if we are supposed to allocate initial heap space
	if (nInitialNumElements > 0)
	{
		// allocate initial heap item
		m_pHeapList = new CLilFixedHeapItem;
//		m_pHeapList = (CLilFixedHeapItem*)malloc(sizeof(CLilFixedHeapItem));
		m_pHeapList->m_pNext = NULL;
		if (m_pHeapList == NULL) return false;

		// initialize initial heap item
		if (!m_pHeapList->m_heap.Init(nElemSizeBytes, nInitialNumElements))
		{
			return false;
		}
	}

	else
	{
		m_pHeapList = NULL;
	}
	
	// save element size to grow by
	m_nGrowElemSize = nGrowNumElements;

	// save memory size in bytes
	m_nElemSizeBytes = nElemSizeBytes;

	// class is now initialized
	m_bInitialized = true;

	return true;
};


inline void CLilFixedHeapGroup::Term()
{
	// make sure class was initialized
	if (!m_bInitialized) return;

	// go through list and delete all heaps
	while (m_pHeapList != NULL)
	{
		CLilFixedHeapItem* pNextHeapList = m_pHeapList->m_pNext;
		m_pHeapList->m_heap.Term();
		delete m_pHeapList;
//		free(m_pHeapList);
		m_pHeapList = pNextHeapList;
	}

	// class is no longer initialized
	m_bInitialized = false;
};

inline void* CLilFixedHeapGroup::Alloc()
{
	ASSERT(m_bInitialized);

	// memory we are going to allocate
	void* pMem = NULL;

	// current heap we are looking at
	CLilFixedHeapItem* pCurHeap = m_pHeapList;

	// try to find space in one of the existing heaps
	while (pCurHeap != NULL)
	{
		// try to allocate from this heap
		pMem = pCurHeap->m_heap.Alloc();
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

	// we found no free memory allocate so another fixed heap
	pCurHeap = new CLilFixedHeapItem;
//	pCurHeap = (CLilFixedHeapItem*)malloc(sizeof(CLilFixedHeapItem));
	if (pCurHeap == NULL) return NULL;
	pCurHeap->m_pNext = m_pHeapList;
	if (!pCurHeap->m_heap.Init(m_nElemSizeBytes, m_nGrowElemSize))
	{
		delete pCurHeap;
//		free(pCurHeap);
		return NULL;
	}
	pMem = pCurHeap->m_heap.Alloc();
	m_pHeapList = pCurHeap;

	// return memory we allocated
	return pMem;
};

inline bool CLilFixedHeapGroup::Free(void* pFreeMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CLilFixedHeapItem* pCurHeap = m_pHeapList;

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

inline bool CLilFixedHeapGroup::InHeap(void* pMem)
{
	ASSERT(m_bInitialized);

	// current heap we are looking at
	CLilFixedHeapItem* pCurHeap = m_pHeapList;

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

#endif
