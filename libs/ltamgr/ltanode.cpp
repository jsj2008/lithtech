#include "ltanode.h"
#include "ltaconverter.h"
#include "iltaallocator.h"

//the decimal accuracy for converting from double to str
#define LTA_DECIMAL_ACCURACY	6

//default constructor. Creates an atom with no value
CLTANode::CLTANode()
{
	NodeConstructor();
}

//function constructor. This is so that allocators can initialize a node
//that they create from their own memory banks
void CLTANode::NodeConstructor()
{
	m_nFlags	= 0;
	m_pData		= NULL;
}


CLTANode::~CLTANode()
{
	//make sure we aren't leaking memory. If we are, this node needs to have
	//Free called on it before it is deleted
	assert(m_pData == NULL);
}


//allocates room for the specified number of elements. Doing this will
//make the node a list and not an atom
bool CLTANode::AllocateElements(uint32 nNumElements, ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);

	//clean up old stuff
	Free(pAllocator);

	//allocate the array
	m_pData = pAllocator->AllocateBlock(sizeof(CLTANode*) * nNumElements);

	//sanity check
	if(m_pData == NULL)
	{
		return false;
	}

	//set up the size
	m_nFlags = nNumElements;

	//success
	return true;
}

//appends an element onto the end of the element list
bool CLTANode::AppendElement(CLTANode* pElement, ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);
	ASSERT(IsList());
	
	//resize the array
	CLTANode** pNewBuffer = (CLTANode**)pAllocator->AllocateBlock(sizeof(CLTANode*) * (GetNumElements() + 1));

	//make sure it worked
	if(pNewBuffer == NULL)
	{
		return false;
	}

	//copy over the old data
	memcpy(pNewBuffer, m_pData, sizeof(CLTANode*) * GetNumElements());

	//add the new element onto the end
	pNewBuffer[GetNumElements()] = pElement;

	//delete the old buffer
	pAllocator->FreeBlock(m_pData);

	//and set up the new buffer
	m_pData = pNewBuffer;
	m_nFlags++;

	return true;
}


//frees the memory associated with the node. If it is a list, it 
//will also free all the children nodes
void CLTANode::Free(ILTAAllocator* pAllocator)
{
	ASSERT(pAllocator);

	if(IsAtom())
	{
		pAllocator->FreeBlock(m_pData);
	}
	else
	{
		//first delete all the children
		for(uint32 nCurrElement = 0; nCurrElement < m_nFlags; nCurrElement++)
		{
			pAllocator->FreeNode(GetElement(nCurrElement));
		}

		//now clear the entire array
		pAllocator->FreeBlock(m_pData);
	}

	m_nFlags	= 0;
	m_pData		= NULL;
}

//sets the value of the atom. Will not work if this already has children
bool CLTANode::SetValue(const char* pszValue, bool bString, ILTAAllocator* pAllocator)
{
	return SetValue(pszValue, bString, strlen(pszValue), pAllocator);
}


//sets the value of the atom. Will not work if this already has children
bool CLTANode::SetValue(const char* pszValue, bool bString, uint32 nLen, ILTAAllocator* pAllocator)
{
	//clean it up
	Free(pAllocator);

	//allocate the new buffer
	m_pData = pAllocator->AllocateBlock(sizeof(char) * (nLen + 1));

	//memory check
	if(m_pData == NULL)
	{
		return false;
	}

	//copy over the string
	strcpy((char*)m_pData, pszValue);

	//now set the flag appropriately
	if(bString)
	{
		m_nFlags = MASK_VALUE | MASK_STRING;
	}
	else
	{
		m_nFlags = MASK_VALUE;
	}

	return true;
}

bool CLTANode::SetValue(int32 nVal, ILTAAllocator* pAllocator)
{
	//buffer used for setting the value
	static char pszBuffer[32];

	uint32 nLen = CLTAConverter::IntToStr(nVal, pszBuffer, 32);

	return SetValue(pszBuffer, false, nLen, pAllocator);
}

bool CLTANode::SetValue(double fVal, ILTAAllocator* pAllocator)
{
	//buffer used for setting the value
	static char pszBuffer[32];

	uint32 nLen = CLTAConverter::RealToStr(fVal, pszBuffer, 32);

	return SetValue(pszBuffer, false, nLen, pAllocator);
}

bool CLTANode::SetValue(bool bVal, ILTAAllocator* pAllocator)
{
	//buffer used for setting the value
	static char pszBuffer[32];

	uint32 nLen = CLTAConverter::BoolToStr(bVal, pszBuffer, 32);

	return SetValue(pszBuffer, false, nLen, pAllocator);
}
