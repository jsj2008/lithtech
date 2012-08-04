//--------------------------------------------------------------------------
// MemoryPageMgr.h
//
// Provides the definition for the page manager for the effect systems. This
// manager is in charge of allocating and holding onto pages of memory that
// can be requested by effect systems for use. These pages can then be
// returned back to the page manager when no longer needed. 
// 
//--------------------------------------------------------------------------
#ifndef __MEMORYPAGEMGR_H__
#define __MEMORYPAGEMGR_H__

//defines a single memory page which at its simplest is a block of memory in a doubly
//linked list
class CMemoryPage
{
public:

	//provides access to the front of the memory block representing this page
	uint8*				GetMemoryBlock()			{ return m_pMemoryBlock; }
	const uint8*		GetMemoryBlock() const		{ return m_pMemoryBlock; }

	//determines the size of the available memory block
	uint32				GetMemoryBlockSize() const	{ return m_nMemoryBlockSize; }

	//determines the allocation offset
	uint32				GetAllocationOffset() const	{ return (m_pAllocationPtr - m_pMemoryBlock); }

	//pointer to the memory where allocations will occur
	const uint8*		GetAllocationPtr() const	{ return m_pAllocationPtr; }

	//determines if the current page is empty or not
	bool				IsEmpty() const				{ return GetAllocationOffset() == 0; }

	//allocates a block of memory from this page, this can return NULL if there is not
	//enough room in the page
	uint8*				Allocate(uint32 nSize);

	//releases the last N bytes that are allocated from this page
	void				Free(uint32 nSize);

	//releases all memory allocated from this page, effectively resetting the allocation pointer
	void				FreeAll()					{ m_pAllocationPtr = m_pMemoryBlock; }

	//the linked list pointers
	CMemoryPage*		m_pPrevPage;
	CMemoryPage*		m_pNextPage;

private:

	//we don't allow copying of this object
	PREVENT_OBJECT_COPYING(CMemoryPage);

	//allow the manager to access our internals
	friend class CMemoryPageMgr;

	//all of this data is declared private to prevent anything but the memory page
	//manager from creating or destroying it. The memory block is provided to the page during
	//construction, and the memory page will not free that block (that is up to the manager)
	CMemoryPage(uint8* pMemoryBlock, uint32 nBlockSize);
	~CMemoryPage();

	//the memory block associated with this page
	uint8*				m_pMemoryBlock;

	//the size of the memory block
	uint32				m_nMemoryBlockSize;

	//the pointer to the area of the memory block where we are to allocate memorys from
	uint8*				m_pAllocationPtr;
};

//defines the actual page manager which handles management of the pages
class CMemoryPageMgr
{
public:

	static const uint32 knInfiniteMemory = 0xFFFFFFFF;

	//constructs the memory page manager. As the only parameter, it takes the size in bytes
	//that the allocated pages should be
	CMemoryPageMgr(uint32 nPageSize, uint32 nMaxMemoryUsage = knInfiniteMemory);
	~CMemoryPageMgr();

	//called to get how large the memory pages that are created will be
	uint32				GetPageSize() const			{ return m_nPageSize; }

	//called to free all currently unused memory pages (a form of garbage collection)
	void				FreeUnusedMemoryPages();

	//called to set the maximum amount of memory that can be used for memory
	//system memory
	void				SetMaximumMemoryUsage(uint32 nMaxMemoryUsage);

	//called to access the maximum amount of memory that can be allocated
	uint32				GetMaximumMemoryUsage() const;

	//called to determine how much memory is currently being used by memory systems
	uint32				GetCurrentMemoryUsage() const;

	//called to request a page from the memory page manager. This can return
	//NULL if the memory limit for the memory page manager has been exceeded.
	CMemoryPage*		AllocatePage();

	//called to free a memory page that is no longer in use.
	void				FreePage(CMemoryPage* pPage);

private:

	//we don't allow copying of this object
	PREVENT_OBJECT_COPYING(CMemoryPageMgr);

	//deletes the specified page. This assumes that the page has already been detatched
	void				DeletePage(CMemoryPage* pPage);

	//this function will pop a page off of the free list, and return NULL if there are no
	//more pages on the list
	CMemoryPage*		PopFreePage();

	//the current amount of memory that has been allocated
	uint32				m_nAllocatedMemory;

	//the maximum amount of memory that we can allocate before forced failure
	uint32				m_nMaxAllocatedMemory;

	//the size of each particle page that will be created in bytes
	uint32				m_nPageSize;

	//the head of our free list
	CMemoryPage*		m_pFreeList;
};


#endif
