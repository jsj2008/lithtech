// ----------------------------------------------------------------------- //
//
// MODULE  : lilfixedheap.h
//
// PURPOSE : Creating efficient fixed size heaps.
//
// CREATED : Sep-20-2000
//
// ----------------------------------------------------------------------- //

#ifndef __LILFIXEDHEAP_H__
#define __LILFIXEDHEAP_H__

#ifndef VOIDPTR
#define VOIDPTR void*
#endif

class CLilFixedHeap 
{
public:
	CLilFixedHeap() { m_bInitialized = false; };
	CLilFixedHeap(uint32  nElemSize, uint32  nNumElements) { m_bInitialized = false; Init(nElemSize,nNumElements); };
	~CLilFixedHeap() { Term(); };

	inline bool Init(uint32  nElemSize, uint32  nNumElements);
	inline void Term();

	inline void* Alloc();
	inline void Free(void* free);

	inline bool InHeap(void* mem);

	// memory allocations for this class will go though system malloc and free
    void* operator new(size_t size) {	return malloc(size); }
    void operator delete(void* p) { free(p); };

private:
	bool m_bInitialized;
	uint32 m_nNumElements;
	uint32 m_nElemSize;
	VOIDPTR* m_pHeap;
	VOIDPTR* m_pFreeList;
};


inline bool CLilFixedHeap::Init(uint32  nElemSizeBytes, uint32  nNumElements)
{
	// make sure number of elements is valid
	if (nNumElements <= 0) return false;

	// we can't make the memory any smaller than our free list pointer
	// so adjust it if necessary
	if (nElemSizeBytes <= sizeof(VOIDPTR)) nElemSizeBytes = sizeof(VOIDPTR);

	// set up member variables (element size is in VOIDPTR units, not bytes!)
	m_nNumElements = nNumElements;
	m_nElemSize = nElemSizeBytes / sizeof(VOIDPTR);

	// allocate our chunk of memory
//  m_pHeap = new VOIDPTR[m_nElemSize*nNumElements];
	m_pHeap = (VOIDPTR*)malloc(nElemSizeBytes*nNumElements);
	if (m_pHeap == NULL) return false;

	// set up free list with all elements
	m_pFreeList = m_pHeap;
	for (uint32 i = 1; i < m_nNumElements; i++)
	{
		*(void**)&(m_pHeap[m_nElemSize*(i-1)]) = &(m_pHeap[m_nElemSize*(i)]);
	}
	*(void**)&(m_pHeap[m_nElemSize*(m_nNumElements-1)]) = NULL;

	// set class to initialized
	m_bInitialized = true;

	return true;
};

inline void CLilFixedHeap::Term() 
{
	// make sure class is initialized
	if (!m_bInitialized) return;

	// delete memory for this heap
//	if (m_pHeap != NULL) delete [] m_pHeap;
	if (m_pHeap != NULL) free(m_pHeap);
	m_pHeap = NULL;

	// set class to not initialized
	m_bInitialized = false;
};

inline void* CLilFixedHeap::Alloc()
{
	ASSERT(m_bInitialized);
	if (m_pFreeList == NULL) return NULL;
	else
	{
		void* pNext = *(VOIDPTR*)m_pFreeList;
		void* pPtr = m_pFreeList;
		m_pFreeList = (VOIDPTR*)pNext;
		return pPtr;
	}
};

inline void CLilFixedHeap::Free(void* free)
{
	ASSERT(m_bInitialized);
	ASSERT(free != NULL);
	if (free == NULL) return;
	VOIDPTR* pOld = m_pFreeList;
	m_pFreeList = (VOIDPTR*)free;
	*(VOIDPTR*)m_pFreeList = pOld;
};

inline bool CLilFixedHeap::InHeap(void* mem)
{
	ASSERT(m_bInitialized);
	if (m_pHeap == NULL) return false;
	if ((mem >= m_pHeap) && (mem <= &(m_pHeap[m_nElemSize*(m_nNumElements-1)]))) return true;
	else return false;
};

#endif
