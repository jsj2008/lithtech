//------------------------------------------------------------
// ILTAAllocator
// 
// Provides an interface for allocating memory for LTA nodes
// so that custom allocation can be done
//
//------------------------------------------------------------
#ifndef __ILTAALLOCATOR_H__
#define __ILTAALLOCATOR_H__

#include "ltbasedefs.h"

class CLTANode;

class ILTAAllocator
{
public:

	ILTAAllocator()						{}
	virtual ~ILTAAllocator()			{}

	//allocators for a node
	virtual CLTANode*		AllocateNode()					= 0;
	virtual void			FreeNode(CLTANode* pNode)		= 0;

	//allocators for a block of memory
	virtual void*			AllocateBlock(uint32 nSize)		= 0;
	virtual void			FreeBlock(void* pBlock)			= 0;

private:
};



#endif

