#ifndef __LTALOADONLYALLOC_H__
#define __LTALOADONLYALLOC_H__

#ifndef __ILTAALLOCATOR_H__
#	include "ILTAAllocator.h"
#endif

//forward declarations
class CLoadMemBlock;

class CLTALoadOnlyAlloc :
	public ILTAAllocator
{
public:

	CLTALoadOnlyAlloc(uint32 nBlockSize);
	virtual ~CLTALoadOnlyAlloc();

	//allocators for a node
	virtual CLTANode*		AllocateNode();
	virtual void			FreeNode(CLTANode* pNode);

	//allocators for a block of memory
	virtual void*			AllocateBlock(uint32 nSize);
	virtual void			FreeBlock(void* pBlock);

	//frees all associated memory
	void					FreeAllMemory();

private:

	//memory left in current block
	uint32			m_nMemLeft;

	//the size of a block
	uint32			m_nBlockSize;

	//the list of blocks, with the head pointing to the currently active one
	CLoadMemBlock*	m_pHead;
};

#endif
