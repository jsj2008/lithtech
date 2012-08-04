//-------------------------------------------------------------------
// LTANodeBuilder.h
//
// Provides the definition for CLTANodeBuilder which allows the 
// building up of node trees through several different interfaces.
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTANODEBUILDER_H__
#define __LTANODEBUILDER_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

#ifndef __LTALIMITS_H__
#	include "ltalimits.h"
#endif


class CLTANodeBuilder
{
public:

	CLTANodeBuilder(ILTAAllocator* pAllocator);
	~CLTANodeBuilder();

	//initializes the builder for a new building
	void Init();

	//frees all associated memory with the node builder, for when
	//you just can't wait for the destructor
	void Free();

	//push a new node onto the stack
	bool Push();

	//pushes a new list onto the stack, and adds
	//a value to the list with the specified value
	bool Push(const char* pszValue, bool bString = false);

	//pop a node off of the stack
	bool Pop();

	//pops off a node, but returns a pointer to it so that it can be tracked and used
	//later on. Will return NULL if the pop fails.
	CLTANode* PopAndTrack();

	//adds an element to the current node on the stack. Assumes
	//that pElement was created with new
	bool AddElement(CLTANode* pElement);

	//adds a value to the current node on the stack. 
	bool AddValue(const char* pszValue, bool bString = false);
	bool AddValue(bool bVal);
	bool AddValue(int32 nVal);
	bool AddValue(double fVal);
	bool AddValue(float fVal);

	//adds arrays of the specified datatype to the tree. This acts the
	//same as if AddValue were called within a loop
	bool AddArray(const char** ppszValue, bool bString, uint32 nArrSize);
	bool AddArray(bool* pbVal, uint32 nArrSize);
	bool AddArray(int32* pnVal, uint32 nArrSize);
	bool AddArray(double* pfVal, uint32 nArrSize);
	bool AddArray(float* pfVal, uint32 nArrSize);

	//gets the current depth of the stack
	uint32 GetDepth() const			{ return m_nStackPos;}

	//pops all nodes off the stack, ending the building
	bool PopAll();

	//given a node builder, it will detach all heads from the builder and add
	//them as elements to this list. This will empty out the added list.
	bool MoveElements(CLTANodeBuilder& MoveFrom);

	//determines how many elements are currently cached. If all nodes have been
	//popped off of the stack, it will tell how many nodes you have in the root
	uint32	GetNumCacheElements() const;

	//detaches the tree currently being built. This will pop all nodes off the stack
	//and clear all memory associated with the builder. This will ONLY get the
	//first node in the stack, and should ONLY be used when you know that there
	//can't be more than one root element. All elements beyond the first will
	//be freed
	CLTANode*	DetachHead();

	//this function is the same as above, but allows the retreival of a list
	//of nodes. It will take an array of CLTANode pointers, and fill up to
	//until either the buffer is full, or it runs out of nodes to add. It will
	//return the number of elements added, and will free any nodes that did not
	//get added
	uint32 DetachHeads(CLTANode** ppNodeList, uint32 nMaxListSize);

	//given a node, it will detach all heads, adding them as children to the
	//passed in node. Returns the number of nodes added
	uint32 DetachHeadsTo(CLTANode* pNewParentNode);

	//aborts the build process, and delets all nodes
	//that had already been created. NOTE: This will destroy the entire
	//tree completely so any pointers still pointing to these elements
	//will be bad
	void		AbortBuild();

private:

	//gets an element out of the cache given a specific ID
	CLTANode* GetCacheElement(uint32 nID);

	//number of cache rows and the size of each one
	enum	{		CACHE_ROW_SIZE		= 1024,
					MAX_CACHE_ROWS		= (MAX_LTA_SPAN / CACHE_ROW_SIZE)					
			};

	//used for caching when building the node lists
	CLTANode**	m_pCache[MAX_CACHE_ROWS];
	
	//the cache position
	uint32 m_nCachePos;

	//the structure used for holding a stack frame
	struct CLTAStackFrame
	{
		uint32		m_nBaseNode;
		uint32		m_nNumNodes;
	};

	//the stack frame

	//the actual stack
	CLTAStackFrame	m_Stack[MAX_LTA_DEPTH];

	//the position in the stack
	uint32	m_nStackPos;

	//the allocator used to handle allocations of nodes
	ILTAAllocator*	m_pAllocator;
};

#endif
