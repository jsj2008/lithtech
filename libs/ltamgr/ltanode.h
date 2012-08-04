//-------------------------------------------------------------------
// LTANode.h
//
// Provides the definition for CLTANode which constitutes a LTA
// parse tree. This structure can be manipulated in a tree like
// manner to parse the tree. 
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------

#ifndef __LTANODE_H__
#define __LTANODE_H__

#include "ltbasedefs.h"

//masks used for isolating parts of the flag field
#define MASK_VALUE				((uint32)0x80000000)
#define MASK_STRING				((uint32)0x40000000)
#define MASK_NUMELEMENTS		((uint32)0x3FFFFFFF)

//forward declaration for the allocator interface
class ILTAAllocator;

class CLTANode
{
public:

	//default constructor. Creates an atom with no value
	CLTANode();
	~CLTANode();

	//function constructor. This is so that allocators can initialize a node
	//that they create from their own memory banks
	void NodeConstructor();

	//allocates room for the specified number of elements. Doing this will
	//make the node a list and not an atom
	bool AllocateElements(uint32 nNumElements, ILTAAllocator* pAllocator);

	//appends an element onto the end of the element list
	bool AppendElement(CLTANode* pElement, ILTAAllocator* pAllocator);

	//frees the memory associated with the node. If it is a list, it 
	//will also free all the children nodes
	void Free(ILTAAllocator* pAllocator);

	//determines if the node is a list or not (no value, only children)
	inline bool IsList() const;

	//determines if the node is an atom (no children, only a value)
	inline bool IsAtom() const;

	//determines if the item is a value, and if it is, if it was originally
	//in quotes
	inline bool IsString() const;

	//gets the number of elements if the item is a list
	inline uint32 GetNumElements() const;

	//sets a specified element, assumes it is within range of its list of elements
	inline void SetElement(CLTANode* pElement, uint32 nIndex);

	//gets a specified child
	inline CLTANode* GetElement(uint32 nIndex);

	//gets a specified child (const version)
	inline const CLTANode* GetElement(uint32 nIndex) const;

	//sets the value of the atom. Will not work if this already has children
	bool SetValue(const char* pszValue, bool bString, ILTAAllocator* pAllocator);

	//sets the value of the atom. Will not work if this already has children.
	//this version is for if you already know the length of the string
	bool SetValue(const char* pszValue, bool bString, uint32 nStrLen, ILTAAllocator* pAllocator);

	//sets the value to a specific type of value (converts to a string)
	bool SetValue(int32 nVal, ILTAAllocator* pAllocator);
	bool SetValue(double fVal, ILTAAllocator* pAllocator);
	bool SetValue(bool bVal, ILTAAllocator* pAllocator);

	//gets the value of the atom
	inline const char* GetValue() const;

private:

	//this variable holds the flag for whether or not it is an atom or list.
	//If it is an atom, the 2nd highest bit will determine if it was in quotes
	//origninally. If it is a list, the bottom 30 bits will hold the number of
	//children in the list
	uint32		m_nFlags;

	//this pointer either holds a list of children, or a string holding the
	//value for the atom
	void*		m_pData;

};

//------------------------------------
// Inlines
//------------------------------------

//sets a specified element
void CLTANode::SetElement(CLTANode* pElement, uint32 nIndex)
{
	ASSERT(IsList());
	ASSERT(nIndex < m_nFlags);

	((CLTANode**)m_pData)[nIndex] = pElement;
}

//determines if the node is a list or not (no value, only children)
bool CLTANode::IsList() const
{
	return (m_nFlags & MASK_VALUE) ? false : true;
}

//determines if the node is an atom (no children, only a value)
bool CLTANode::IsAtom() const
{
	return (m_nFlags & MASK_VALUE) ? true : false;
}

//determines if the item is a value, and if it is, if it was originally
//in quotes
bool CLTANode::IsString() const
{
	return (m_nFlags & MASK_STRING) ? true : false;
}

//gets the number of elements if the item is a list
uint32 CLTANode::GetNumElements() const
{
	ASSERT(IsList());

	//maybe this should be masked with MASK_NUMELEMENTS, but the upper 2 bits
	//should never be set
	return m_nFlags;
}

//gets a specified child
CLTANode* CLTANode::GetElement(uint32 nIndex)
{
	ASSERT(IsList());
	ASSERT(nIndex < m_nFlags);

	return ((CLTANode**)m_pData)[nIndex];
}

//gets a specified child
const CLTANode* CLTANode::GetElement(uint32 nIndex) const
{
	ASSERT(IsList());
	ASSERT(nIndex < m_nFlags);

	return ((const CLTANode**)m_pData)[nIndex];
}

//gets the value of the atom
const char* CLTANode::GetValue() const
{
	ASSERT(IsAtom());

	return (const char*)m_pData;
}


#endif
