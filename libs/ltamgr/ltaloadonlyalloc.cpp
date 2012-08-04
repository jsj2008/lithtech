#include "ltaloadonlyalloc.h"
#include "ltanode.h"

class CLoadMemBlock
{
public:
	
	CLoadMemBlock(uint8* pMem, CLoadMemBlock* pNext) : 
		m_pMemory(pMem), m_pNext(pNext), m_pCurrMemHead(pMem)			
	{
	}

	~CLoadMemBlock()
	{
		//delete our block
		delete [] m_pMemory;

		//delete the children
		delete m_pNext;
	}	

	//the memory block
	uint8*				m_pMemory;

	//the current location to grab memory from
	uint8*				m_pCurrMemHead;

	//the next item in the list
	CLoadMemBlock*		m_pNext;

private:
};



CLTALoadOnlyAlloc::CLTALoadOnlyAlloc(uint32 nBlockSize) :
	m_pHead(NULL),
	m_nBlockSize(nBlockSize),
	m_nMemLeft(0)
{
	//the blocks really need to be more than a K to be useful (ideally they would
	//be about 64k to a meg)
	ASSERT(nBlockSize > 1024);
}

CLTALoadOnlyAlloc::~CLTALoadOnlyAlloc()
{
	FreeAllMemory();
}

//allocators for a node
CLTANode* CLTALoadOnlyAlloc::AllocateNode()
{
	//lets first off get the memory for a node
	CLTANode* pNewNode = (CLTANode*)AllocateBlock(sizeof(CLTANode));

	//check the alloc
	if(pNewNode)
	{
		pNewNode->NodeConstructor();
	}

	return pNewNode;
}

void CLTALoadOnlyAlloc::FreeNode(CLTANode* pNode)
{
	//we don't support freeing
}

//allocators for a block of memory
void* CLTALoadOnlyAlloc::AllocateBlock(uint32 nSize)
{
	//see if we have enough room left in this block
	if(m_nMemLeft < nSize)
	{
		uint32 nBlockSize = LTMAX(m_nBlockSize, nSize);

		//we need to allocate the memory for a block. If the size is bigger than
		//a block, we need to allocate the block to be that big
		uint8* pMem;
		LT_MEM_TRACK_ALLOC(pMem = new uint8[nBlockSize],LT_MEM_TYPE_MISC);

		//check the allocation
		if(pMem == NULL)
			return NULL;

		//now allocate the block structure that will maintain the mem
		CLoadMemBlock* pNewBlock;
		LT_MEM_TRACK_ALLOC(pNewBlock = new CLoadMemBlock(pMem, m_pHead),LT_MEM_TYPE_MISC);

		//check the allocation
		if(pNewBlock == NULL)
		{
			delete [] pMem;
			return NULL;
		}

		m_pHead = pNewBlock;

		//update the block size
		m_nMemLeft = nBlockSize;
	}

	//ok, now we know that the allocation will fit inside of this block, so lets
	//go ahead and return the pointer and update our counts
	ASSERT(m_nMemLeft >= nSize);
	ASSERT(m_pHead);

	void* pRV = (void*)m_pHead->m_pCurrMemHead;

	//update our counts
	m_pHead->m_pCurrMemHead += nSize;
	m_nMemLeft -= nSize;

	return pRV;
}

void CLTALoadOnlyAlloc::FreeBlock(void* pBlock)
{
	//we don't support freeing
}

void CLTALoadOnlyAlloc::FreeAllMemory()
{
	//only need to delete the first one. It will delete its children
	delete m_pHead;
	m_pHead		= NULL;
	m_nMemLeft	= 0;
}


