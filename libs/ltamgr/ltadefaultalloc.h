#ifndef __LTADEFAULTALLOC_H__
#define __LTADEFAULTALLOC_H__

#ifndef __ILTAALLOCATOR_H__
#	include "ILTAAllocator.h"
#endif

class CLTADefaultAlloc :
	public ILTAAllocator
{
public:
	
	//allocators for a node
	virtual CLTANode*	AllocateNode()				{ return new CLTANode; }
	virtual void		FreeNode(CLTANode* pNode)	{ if(pNode) { pNode->Free(this); } delete pNode; }

	//allocators for a block of memory
	virtual void*		AllocateBlock(uint32 nSize)	 { return (void*)(new uint8[nSize]); }
	virtual void		FreeBlock(void* pBlock)		 { delete [] pBlock; }

private:

};

#endif