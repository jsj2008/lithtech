#include "stdafx.h"
#include "memorypagemgr.h"

//---------------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// CMemoryPage
//---------------------------------------------------------------------------------

CMemoryPage::CMemoryPage(uint8* pMemoryBlock, uint32 nMemoryBlockSize) :
	m_pPrevPage(NULL),
	m_pNextPage(NULL),
	m_pMemoryBlock(pMemoryBlock),
	m_pAllocationPtr(pMemoryBlock),
	m_nMemoryBlockSize(nMemoryBlockSize)
{
}

CMemoryPage::~CMemoryPage()
{
}

//allocates a block of memory from this page, this can return NULL if there is not
//enough room in the page
uint8* CMemoryPage::Allocate(uint32 nSize)
{
	//return NULL if there is no room
	if(GetMemoryBlockSize() - GetAllocationOffset() < nSize)
		return NULL;

	//there is room, so allocate a block
	uint8* pRV = m_pAllocationPtr;

	//move our allocation pointer ahead
	m_pAllocationPtr += nSize;
	return pRV;
}

//releases the last N bytes that are allocated from this page
void CMemoryPage::Free(uint32 nSize)
{
	//sanity check that there is enough already allocated
	LTASSERT(GetAllocationOffset() >= nSize, "Error: Freeing more memory from the page than is available");
	m_pAllocationPtr -= nSize;
}

//---------------------------------------------------------------------------------
// CMemoryPageMgr
//---------------------------------------------------------------------------------

CMemoryPageMgr::CMemoryPageMgr(uint32 nPageSize, uint32 nMaxMemory) :
	m_nAllocatedMemory(0),
	m_nMaxAllocatedMemory(nMaxMemory),
	m_pFreeList(NULL),
	m_nPageSize(nPageSize)
{
}

CMemoryPageMgr::~CMemoryPageMgr()
{
	//clean up all of our unused memory pages
	FreeUnusedMemoryPages();

	//now make sure that all the pages we handed out have been freed
	LTASSERT(GetCurrentMemoryUsage() == 0, "Warning: Memory pages were allocated and not freed");
}

//called to free all currently unused memory pages (a form of garbage collection)
void CMemoryPageMgr::FreeUnusedMemoryPages()
{
	CMemoryPage* pDeletePage = NULL;

	//just pop and delete until we are done
	while((pDeletePage = PopFreePage()) != NULL)
	{
		DeletePage(pDeletePage);
	}
}

//called to set the maximum amount of memory that can be used for memory
//system memory
void CMemoryPageMgr::SetMaximumMemoryUsage(uint32 nMaxMemoryUsage)
{
	//set this as our maximum threshold
	m_nMaxAllocatedMemory = nMaxMemoryUsage;

	//and delete free pages until we are either below the limit, or out of free pages
	while(GetCurrentMemoryUsage() > GetMaximumMemoryUsage())
	{
		//get a page to delete
		CMemoryPage* pPage = PopFreePage();

		//see if our list is empty
		if(!pPage)
			break;

		DeletePage(pPage);
	}
}

//called to access the maximum amount of memory that can be allocated
uint32 CMemoryPageMgr::GetMaximumMemoryUsage() const
{
	return m_nMaxAllocatedMemory;
}

//called to determine how much memory is currently being used by memory systems
uint32 CMemoryPageMgr::GetCurrentMemoryUsage() const
{
	return m_nAllocatedMemory;
}

//called to request a page from the memory page manager. This can return
//NULL if the memory limit for the memory page manager has been exceeded.
CMemoryPage* CMemoryPageMgr::AllocatePage()
{
	//see if we have a free one on the list
	CMemoryPage* pPage = PopFreePage();
	if(pPage)
		return pPage;

	//we don't have one on the list, make sure that we have enough room to allocate one
	if(GetMaximumMemoryUsage() - GetCurrentMemoryUsage() < m_nPageSize)
	{
		//we are out of our allotted memory
		return NULL;
	}

	//determine the memory block size that we want to allocate
	uint32 nBlockSize = sizeof(CMemoryPage) + m_nPageSize;

	//allocate that block of memory
	uint8* pMemBlock;
	LT_MEM_TRACK_ALLOC(pMemBlock = new uint8 [nBlockSize], LT_MEM_TYPE_CLIENTFX);

	//and now verify the allocation
	if(!pMemBlock)
		return NULL;

	//everything is valid, so construct the object on top of the memory block
	CMemoryPage* pNewPage = new (pMemBlock) CMemoryPage(pMemBlock + sizeof(CMemoryPage), m_nPageSize);

	//and now update our allocation count
	m_nAllocatedMemory += m_nPageSize;

	//and give this object back to the caller
	return pNewPage;
}

//called to free a memory page that is no longer in use.
void CMemoryPageMgr::FreePage(CMemoryPage* pPage)
{
	//if we are over our memory budget, then toss this page, otherwise push it onto our list
	if(m_nAllocatedMemory > m_nMaxAllocatedMemory)
	{
		//sanity check
		LTASSERT(m_nAllocatedMemory >= pPage->GetMemoryBlockSize(), "Error: Freed more memory than was allocated");

		//we are overallocated, just toss it
		DeletePage(pPage);
		return;
	}

	//reset our allocation pointer so that way we can reuse this page later
	pPage->FreeAll();

	//we have room left, so now just tack it onto the head of our free list
	pPage->m_pNextPage = m_pFreeList;
	pPage->m_pPrevPage = NULL;

	if(m_pFreeList)
		m_pFreeList->m_pPrevPage = pPage;	

	//update our list head
	m_pFreeList = pPage;
}

//this function will pop a page off of the free list, and return NULL if there are no
//more pages on the list
CMemoryPage* CMemoryPageMgr::PopFreePage()
{
	//see if our list is empty
	if(!m_pFreeList)
		return NULL;

	//Update the head element
	CMemoryPage* pPopNode = m_pFreeList;

	m_pFreeList = m_pFreeList->m_pNextPage;

	//break the link of the next item back
	if(m_pFreeList)
	{
		m_pFreeList->m_pPrevPage = NULL;
	}

	//and now break any links the node we are returning
	pPopNode->m_pNextPage = NULL;
	pPopNode->m_pPrevPage = NULL;

	//and return that node
	return pPopNode;	
}

//deletes the specified page. This assumes that the page has already been detatched
void CMemoryPageMgr::DeletePage(CMemoryPage* pPage)
{
	if(pPage)
	{
		m_nAllocatedMemory -= pPage->GetMemoryBlockSize();

		//call the destructor for the page
		pPage->~CMemoryPage();

		//and now free the memory as a block of uint8 data since that is how we allocated it
		delete [] ((uint8*)pPage);
	}
}

