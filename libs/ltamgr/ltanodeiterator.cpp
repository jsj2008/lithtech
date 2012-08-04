#include "ltanodeiterator.h"
#include "ltadefaultalloc.h"

//allocator to be used in creating the root node. This is thread safe since
//it is just new and delete
CLTADefaultAlloc g_DefaultAlloc;


//create an iterator starting at this root
CLTANodeIterator::CLTANodeIterator(CLTANode* pRoot) :
	m_nStackPos(0)
{
	//create a new node to be the parent
	CLTANode* pParent = g_DefaultAlloc.AllocateNode();

	//add this item as a child
	pParent->AppendElement(pRoot, &g_DefaultAlloc);

	//add the node to the stack
	m_pNodeStack[m_nStackPos] = pParent;
	m_nElemStack[m_nStackPos] = 0;

}

CLTANodeIterator::~CLTANodeIterator()
{
	//detach the root from the parent node, since we don't want to delete that
	if(m_pNodeStack[0])
	{
		m_pNodeStack[0]->SetElement(NULL, 0);
		g_DefaultAlloc.FreeNode(m_pNodeStack[0]);
	}
}

//pushes a specified node onto the stack. Identical to stepping
//into that node
bool CLTANodeIterator::PushNode(CLTANode* pNode)
{
	ASSERT(pNode);
	ASSERT(m_nStackPos < MAX_LTA_DEPTH - 1);

	//should possibly perform a test here, but the assert should catch it
	//in debug anyway

	//add the node to the stack
	m_nStackPos++;
	m_pNodeStack[m_nStackPos] = pNode;
	m_nElemStack[m_nStackPos] = 0;

	//success
	return true;
}


//pops a node from the stack. Identical to traversing back up to 
//the node's parent. Returns the node that was popped, or NULL if
//it fails
CLTANode* CLTANodeIterator::PopNode()
{
	//return the head element
	if(m_nStackPos > 0)
	{
		return m_pNodeStack[m_nStackPos--];
	}

	//nothing on the stack
	return NULL;
}


//finds the next list with the first element being a value
//with a string that matches the specified string. Updates
//the position of the cursor to point to that node
CLTANode* CLTANodeIterator::FindNextList(const char* pszValue)
{
	//go through all the lists
	do
	{
		CLTANode* pCurr = NextList();

		//make sure we didn't hit the end of the list
		if(pCurr == NULL)
		{
			return NULL;
		}

		//see if this list has elements
		if(pCurr->GetNumElements() > 0)
		{
			//see if this first element is a 0
			CLTANode* pValue = pCurr->GetElement(0);

			if(pValue->IsAtom())
			{
				//now we can see if the strings match
				if(stricmp(pValue->GetValue(), pszValue) == 0)
				{
					return pCurr;
				}
			}
		}

	}while(1);

	return NULL;
}

//moves onto the next value in the current node's list. If the current
//element is a list, it will be searched through recursively. Calling this continually
//will iterate through all values in the tree. Returns NULL if no more
CLTANode* CLTANodeIterator::NextValue()
{
	//just run through the elements until we hit a value one
	do
	{
		CLTANode* pCurr = NextElement();

		//see if we hit the end
		if(pCurr == NULL)
		{
			return NULL;
		}

		//otherwise, see if it is an atom
		if(pCurr->IsAtom())
		{
			return pCurr;
		}

	}while(1); //keep going til we have to bail or find a value
}

//moves onto the next list in the current node's list. If the current
//element is a list, it will be searched through recursively. Calling this continually
//will iterate through all lists in the tree. Returns NULL if no more
CLTANode* CLTANodeIterator::NextList()
{
	//just run through the elements until we hit a list one
	do
	{
		CLTANode* pCurr = NextElement();

		//see if we hit the end
		if(pCurr == NULL)
		{
			return NULL;
		}

		//otherwise, see if it is a list
		if(pCurr->IsList())
		{
			return pCurr;
		}

	}while(1); //keep going til we have to bail or find a list
}

//moves onto the next element (either a value or list). If the current
//element is a list, it will be searched through recursively. Calling this continually
//will iterate through all elements in the tree. Returns NULL if no more
CLTANode* CLTANodeIterator::NextElement()
{
	//sanity checks
	ASSERT(GetHead());

	//get the current element

	CLTANode* pCurr = NULL;

	if(GetElementIndex() < GetHead()->GetNumElements())
	{
		pCurr = GetHead()->GetElement(GetElementIndex());
	}
	else
	{
		//pop the stack if we can
		if(m_nStackPos == 0)
		{
			//we can't pop!
			return NULL;
		}

		//pop the node
		PopNode();

		return NextElement();
	}

	//okay, we have successfully found an item, we need to move onto the next one

	SetElementIndex(GetElementIndex() + 1);

	//if the current is a list, we need to pop into it
	if(pCurr->IsList())
	{
		PushNode(pCurr);
	}

	return pCurr;
}


//finds the next list with the appropriate first element being
//a value with the specified text, but does not go deeper than
//one level
CLTANode* CLTANodeIterator::FindNextListShallow(const char* pszValue)
{
	CLTANode* pCurrList;

	do
	{
		//get the next list
		pCurrList = NextListShallow();

		//see if we are out of lists
		if(pCurrList == NULL)
		{
			return NULL;
		}

		//see if this list has any elements
		if(pCurrList->GetNumElements() > 0)
		{
			//see if the first element is a value
			CLTANode* pValue = pCurrList->GetElement(0);

			if(pValue->IsAtom())
			{
				//now we can check the value
				if(strcmp(pValue->GetValue(), pszValue) == 0)
				{
					//we have hit the item
					return pCurrList;
				}
			}
		}

	}while(1); //broken out of when no more lists are found

	return NULL;
}


//moves onto the next value in the current node's list. Returns NULL if no more
CLTANode* CLTANodeIterator::NextValueShallow()
{
	ASSERT(m_nStackPos > 0);

	for(	uint32 nCurrElem = GetElementIndex(); 
			nCurrElem < GetHead()->GetNumElements(); 
			nCurrElem++)
	{
		if(GetHead()->GetElement(nCurrElem)->IsAtom())
		{
			//found it

			//save this new offset
			SetElementIndex(nCurrElem + 1);

			//return the element
			return GetHead()->GetElement(nCurrElem);
		}
	}

	//didn't find it

	//set the offset to the end
	SetElementIndex(GetHead()->GetNumElements());

	//indicate failure
	return NULL;
}


//moves onto the next list in the current node's list. Returns NULL if no more
CLTANode* CLTANodeIterator::NextListShallow()
{

	for(	uint32 nCurrElem = GetElementIndex(); 
			nCurrElem < GetHead()->GetNumElements(); 
			nCurrElem++)
	{
		if(GetHead()->GetElement(nCurrElem)->IsList())
		{
			//found it

			//save this new offset
			SetElementIndex(nCurrElem + 1);

			//return the element
			return GetHead()->GetElement(nCurrElem);
		}
	}

	//didn't find it

	//set the offset to the end
	m_nElemStack[m_nStackPos] = GetHead()->GetNumElements();

	//indicate failure
	return NULL;
}


//moves onto the next element (either a value or list). Returns NULL if no more
CLTANode* CLTANodeIterator::NextElementShallow()
{
	if(GetHead()->GetNumElements() == 0)
	{
		return NULL;
	}

	if(GetElementIndex() < (GetHead()->GetNumElements() - 1))
	{
		//get the item and update the position
		SetElementIndex(GetElementIndex() + 1);
		CLTANode* pRV = GetHead()->GetElement(GetElementIndex());
		return pRV;
	}

	//at end of list
	SetElementIndex(GetHead()->GetNumElements());
	return NULL;
}


//resets the cursor so that the elements being searched return back to the head
//of the current node
void CLTANodeIterator::ResetCursorElement()
{
	SetElementIndex(0);
}

//gets the current element being pointed at
CLTANode* CLTANodeIterator::GetCursorElement()
{
	//make sure this is really a list
	ASSERT(GetHead()->IsList());

	//get that list's child element
	return GetHead()->GetElement(GetElementIndex());
}

//gets the head of the stack
CLTANode* CLTANodeIterator::GetHead()
{
	return m_pNodeStack[m_nStackPos];
}


//get the current element index
uint32 CLTANodeIterator::GetElementIndex() const
{
	return m_nElemStack[m_nStackPos];
}

//sets the element index
void CLTANodeIterator::SetElementIndex(uint32 nVal)
{
	m_nElemStack[m_nStackPos] = nVal;
}

//resets the entire iterator, so it begins again at the root
void CLTANodeIterator::Reset()
{
	m_nStackPos = 0;
	ResetCursorElement();
}