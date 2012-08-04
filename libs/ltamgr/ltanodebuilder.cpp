#include "ltanodebuilder.h"
#include "ltaconverter.h"
#include "iltaallocator.h"

CLTANodeBuilder::CLTANodeBuilder(ILTAAllocator* pAllocator) :
	m_pAllocator(pAllocator),
	m_nStackPos(0),
	m_nCachePos(0)
{
	//make sure that we have a valid allocator associated with this builder
	ASSERT(m_pAllocator);

	//need to clear out all the cache rows
	for(uint32 nCurrRow = 0; nCurrRow < MAX_CACHE_ROWS; nCurrRow++)
	{
		m_pCache[nCurrRow] = NULL;
	}
}

CLTANodeBuilder::~CLTANodeBuilder()
{
	AbortBuild();
}


//initializes the builder for a new building
void CLTANodeBuilder::Init()
{
	//clear up all the old stuff
	AbortBuild();
}


//frees all associated memory with the node builder, for when
//you just can't wait for the destructor
void CLTANodeBuilder::Free()
{
	//remove all the cache rows
	for(uint32 nCurrRow = 0; nCurrRow < MAX_CACHE_ROWS; nCurrRow++)
	{
		delete [] m_pCache[nCurrRow];
		m_pCache[nCurrRow] = NULL;
	}

	//reset the states
	m_nStackPos = 0;
	m_nCachePos = 0;

}

//pushes a new list onto the stack, and adds
//a value to the list with the specified value
bool CLTANodeBuilder::Push(const char* pszValue, bool bString)
{
	if(Push())
	{
		return AddValue(pszValue, bString);
	}
	return false;
}
	

//push a new node onto the stack
bool CLTANodeBuilder::Push()
{
	//make sure we can push
	if(GetDepth() >= MAX_LTA_DEPTH - 1)
	{
		return false;
	}

	//now modify the node
	m_Stack[GetDepth()].m_nBaseNode = m_nCachePos;
	m_Stack[GetDepth()].m_nNumNodes = 0;

	//next on the stack
	m_nStackPos++;

	return true;
}

//pop a node off of the stack
bool CLTANodeBuilder::Pop()
{
	return PopAndTrack() ? true : false;
}

//pops off a node, but returns a pointer to it so that it can be tracked and used
//later on. Will return NULL if the pop fails.
CLTANode* CLTANodeBuilder::PopAndTrack()
{
	//we need to build up the list of elements to be contained under this list,
	//create the list node, remove the children elements, and then add the
	//list element back into the cache


	//make sure they don't pop too many times
	if(GetDepth() == 0)
	{
		return NULL;
	}

	//pop the stack
	m_nStackPos--;

	//get the number of nodes
	uint32 nNumNodes = m_Stack[GetDepth()].m_nNumNodes;

	//move the cache position back
	m_nCachePos = m_Stack[GetDepth()].m_nBaseNode;


	//create the node
	CLTANode* pNewNode = m_pAllocator->AllocateNode();

	//make sure the allocation succeeded.
	if(pNewNode == NULL)
	{
		//there is a potential memory leak here where the children might
		//not get deleted, so run through and delete all the nodes that
		//would have been bound to this node. Note: at this point we are
		//in a critical state anyway, so it may not be a big concern
		for(uint32 nCurrElem = 0; nCurrElem < nNumNodes; nCurrElem++)
		{
			m_pAllocator->FreeNode(GetCacheElement(m_nCachePos + nCurrElem));
		}

		return NULL;
	}

	pNewNode->AllocateElements(nNumNodes, m_pAllocator);

	//now we read in all the elements into the new node
	for(uint32 nCurrElem = 0; nCurrElem < nNumNodes; nCurrElem++)
	{
		pNewNode->SetElement(GetCacheElement(m_nCachePos + nCurrElem), nCurrElem);
		m_pCache[(m_nCachePos + nCurrElem) / CACHE_ROW_SIZE][(m_nCachePos + nCurrElem) % CACHE_ROW_SIZE] = NULL;
	}

	//push this new node onto the stack
	if(AddElement(pNewNode) == false)
	{
		m_pAllocator->FreeNode(pNewNode);
		return NULL;
	}

	return pNewNode;

}


//adds an element to the current node on the stack. Assumes
//that pElement was created with new
bool CLTANodeBuilder::AddElement(CLTANode* pElement)
{
	//add this node to the next available spot in the cache
	uint32 nRow = m_nCachePos / CACHE_ROW_SIZE;

	//see if we need to allocate this row
	if(m_pCache[nRow] == NULL)
	{
		//we do
		LT_MEM_TRACK_ALLOC(m_pCache[nRow] = new CLTANode* [CACHE_ROW_SIZE],LT_MEM_TYPE_MISC);

		//make sure the allocation succeeded
		if(m_pCache[nRow] == NULL)
		{
			//it didn't
			return false;
		}

		//clear out the row
		//memset(m_pCache[nRow], 0, sizeof(CLTANode*) * CACHE_ROW_SIZE);
	}

	//now we add this element to the list
	m_pCache[nRow][m_nCachePos % CACHE_ROW_SIZE] = pElement;

	//need to update the counts of the parent of this item
	if(GetDepth() > 0)
	{
		m_Stack[GetDepth() - 1].m_nNumNodes++;
	}

	//update the cache position
	m_nCachePos++;

	return true;
}

//gets an element out of the cache given a specific ID
CLTANode* CLTANodeBuilder::GetCacheElement(uint32 nID)
{
	//get the row
	uint32 nRow = nID / CACHE_ROW_SIZE;

	//sanity check the row
	ASSERT(nRow < MAX_CACHE_ROWS);
	ASSERT(m_pCache[nRow]);

	//calculate the column
	uint32 nCol = nID % CACHE_ROW_SIZE;

	return m_pCache[nRow][nCol];
}


//adds a value to the current node on the stack. 
bool CLTANodeBuilder::AddValue(const char* pszValue, bool bString)
{
	//need to create the new node
	CLTANode* pNewNode = m_pAllocator->AllocateNode();

	//make sure the allocation succeeded
	if(pNewNode == NULL)
	{
		return false;
	}

	pNewNode->SetValue(pszValue, bString, m_pAllocator);

	//now we add this to the cache
	if(AddElement(pNewNode) == false)
	{
		//failed to add it to the cache
		delete pNewNode;
		return false;
	}

	//success
	return true;
}

//a buffer used for the building up of the value strings
#define BUILD_BUFFER_SIZE		128
static char g_pszBuildBuffer[BUILD_BUFFER_SIZE];

//adds a value to the current node on the stack. 
bool CLTANodeBuilder::AddValue(bool bVal)
{
	CLTAConverter::BoolToStr(bVal, g_pszBuildBuffer, BUILD_BUFFER_SIZE);
	return AddValue(g_pszBuildBuffer, false);
}

//adds a value to the current node on the stack. 
bool CLTANodeBuilder::AddValue(int32 nVal)
{
	CLTAConverter::IntToStr(nVal, g_pszBuildBuffer, BUILD_BUFFER_SIZE);
	return AddValue(g_pszBuildBuffer, false);
}

//adds a value to the current node on the stack. 
bool CLTANodeBuilder::AddValue(double fVal)
{
	CLTAConverter::RealToStr(fVal, g_pszBuildBuffer, BUILD_BUFFER_SIZE);
	return AddValue(g_pszBuildBuffer, false);
}

bool CLTANodeBuilder::AddValue(float fVal)
{
	return AddValue((double)fVal);
}


//pops all nodes off the stack, ending the building
bool CLTANodeBuilder::PopAll()
{
	while(GetDepth() > 0)
	{
		Pop();
	}

	return true;
}

//given a node builder, it will detach all heads from the builder and add
//them as elements to this list. This will empty out the added list.
bool CLTANodeBuilder::MoveElements(CLTANodeBuilder& MoveFrom)
{
	//pop all of the elements on the other list
	MoveFrom.PopAll();

	//now see how many items we need to move
	uint32 nNumToMove = MoveFrom.GetNumCacheElements();

	//allocate the list
	CLTANode** pList;
	LT_MEM_TRACK_ALLOC(pList = new CLTANode*[nNumToMove],LT_MEM_TYPE_MISC);

	//make sure we allocated memory ok
	if(pList == NULL)
	{
		return false;
	}

	//get the list
	uint32 nNumAdded = MoveFrom.DetachHeads(pList, nNumToMove);

	//assume success
	bool bRV = true;

	//now add all the elements
	for(uint32 nCurrElem = 0; nCurrElem < nNumAdded; nCurrElem++)
	{
		if(AddElement(pList[nCurrElem]) == false)
		{
			//hmmm....this is odd. At least make sure there isn't a memory leak
			m_pAllocator->FreeNode(pList[nCurrElem]);
			bRV = false;
		}
	}

	//make sure to clean up the list
	delete [] pList;

	return bRV;
}


//determines how many elements are currently cached. If all nodes have been
//popped off of the stack, it will tell how many nodes you have in the root
uint32	CLTANodeBuilder::GetNumCacheElements() const
{
	return m_nCachePos;
}



//detaches the tree currently being built. This will pop all nodes off the stack
//and clear all memory associated with the builder. This will ONLY get the
//first node in the stack, and should ONLY be used when you know that there
//can't be more than one root element. All elements beyond the first will
//be freed
CLTANode* CLTANodeBuilder::DetachHead()
{
	CLTANode* pHead;

	//detach the node
	uint32 nNumDetached = DetachHeads(&pHead, 1);

	//see if we got a node
	if(nNumDetached > 0)
	{
		return pHead;
	}
	
	return NULL;

}

//this function is the same as above, but allows the retreival of a list
//of nodes. It will take an array of CLTANode pointers, and fill up to
//until either the buffer is full, or it runs out of nodes to add. It will
//return the number of elements added, and will free any nodes that did not
//get added
uint32 CLTANodeBuilder::DetachHeads(CLTANode** ppNodeList, uint32 nMaxListSize)
{
	//sanity check
	ASSERT(ppNodeList);

	//pop all the nodes off
	PopAll();


	//copy over the list
	uint32 nNumNodes = m_nCachePos;

	for(uint32 nCurrNode = 0; nCurrNode < nNumNodes; nCurrNode++)
	{
		//if we can fit it in the list, add it, otherwise free it
		if(nCurrNode < nMaxListSize)
		{
			ppNodeList[nCurrNode] = GetCacheElement(nCurrNode);
		}
		else
		{
			m_pAllocator->FreeNode(GetCacheElement(nCurrNode));
		}
	}

	//free the internal cache
	Free();

	//pass back the number set to the user

	if(nNumNodes < nMaxListSize)
	{
		return nNumNodes;
	}
	return nMaxListSize;
}

//given a node, it will detach all heads, adding them as children to the
//passed in node. Returns the number of nodes added
uint32 CLTANodeBuilder::DetachHeadsTo(CLTANode* pNewParentNode)
{
	//make sure we have a valid parent
	if(pNewParentNode == NULL)
	{
		return 0;
	}

	//now we need to create a buffer
	CLTANode** ppChildList;
	LT_MEM_TRACK_ALLOC(ppChildList = new CLTANode*[GetNumCacheElements()],LT_MEM_TYPE_MISC);

	//make sure the memory allocation worked
	if(ppChildList == NULL)
	{
		return 0;
	}

	//get the list
	uint32 nNumChildren = DetachHeads(ppChildList, GetNumCacheElements());

	//now add the list to the parent
	for(uint32 nCurrChild = 0; nCurrChild < nNumChildren; nCurrChild++)
	{
		pNewParentNode->AppendElement(ppChildList[nCurrChild], m_pAllocator);
	}

	//free the list now
	delete [] ppChildList;

	return nNumChildren;
}


//aborts the build process, and delets all nodes
//that had already been created. NOTE: This will destroy the entire
//tree completely so any pointers still pointing to these elements
//will be bad
void CLTANodeBuilder::AbortBuild()
{
	for(uint32 nCurrElem = 0; nCurrElem < m_nCachePos; nCurrElem++)
	{
		//delete the node
		delete GetCacheElement(nCurrElem);
	}

	//now we clear the cache
	Free();
}

bool CLTANodeBuilder::AddArray(const char** ppszValue, bool bString, uint32 nArrSize)
{
	for(uint32 nCurrVal = 0; nCurrVal < nArrSize; nCurrVal++)
	{
		if(AddValue(ppszValue[nCurrVal], bString) == false)
		{
			return false;
		}
	}
	return true;
}


bool CLTANodeBuilder::AddArray(bool* pbVal, uint32 nArrSize)
{
	for(uint32 nCurrVal = 0; nCurrVal < nArrSize; nCurrVal++)
	{
		if(AddValue(pbVal[nCurrVal]) == false)
		{
			return false;
		}
	}
	return true;
}


bool CLTANodeBuilder::AddArray(int32* pnVal, uint32 nArrSize)
{
	for(uint32 nCurrVal = 0; nCurrVal < nArrSize; nCurrVal++)
	{
		if(AddValue(pnVal[nCurrVal]) == false)
		{
			return false;
		}
	}
	return true;
}


bool CLTANodeBuilder::AddArray(double* pfVal, uint32 nArrSize)
{
	for(uint32 nCurrVal = 0; nCurrVal < nArrSize; nCurrVal++)
	{
		if(AddValue(pfVal[nCurrVal]) == false)
		{
			return false;
		}
	}
	return true;
}

bool CLTANodeBuilder::AddArray(float* pfVal, uint32 nArrSize)
{
	for(uint32 nCurrVal = 0; nCurrVal < nArrSize; nCurrVal++)
	{
		if(AddValue(pfVal[nCurrVal]) == false)
		{
			return false;
		}
	}
	return true;
}


