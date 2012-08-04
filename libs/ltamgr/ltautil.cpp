#include "ltautil.h"
#include "ltanodeiterator.h"

//given a filename, this determines if the file is compressed or not
bool CLTAUtil::IsFileCompressed(const char* pszFilename)
{
	ASSERT(pszFilename);

	//go backwards through the filename until we find a period
	uint32 nNameLen = strlen(pszFilename);

	//see if we have a string
	if(nNameLen == 0)
	{
		return false;
	}

	for(uint32 nCurrPos = nNameLen; nCurrPos > 0; nCurrPos--)
	{
		if(pszFilename[nCurrPos - 1] == '.')
		{
			//we hit a period, we need to check the extension
			if(stricmp(pszFilename + nCurrPos, "ltc") == 0)
			{
				//found the correct extension
				return true;
			}
			else
			{
				break;
			}
		}
	}

	//didn't find the matching extension
	return false;
}

//given a node, it will determine the amount of memory being used
uint32 CLTAUtil::GetMemoryUsage(CLTANode* pNode)
{
	ASSERT(pNode);

	//count the memory for the node itself
	uint32 nSize = sizeof(CLTANode);
	
	//now figure out the size
	if(pNode->IsList())
	{
		nSize += pNode->GetNumElements() + sizeof(CLTANode*);

		//now figure out the size for each of the children
		for(uint32 nCurrChild = 0; nCurrChild < pNode->GetNumElements(); nCurrChild++)
		{
			nSize += GetMemoryUsage(pNode->GetElement(nCurrChild));
		}
	}
	else
	{
		//if it has a value, factor in the size of that into the total size
		if(pNode->GetValue())
		{
			nSize += strlen(pNode->GetValue()) * sizeof(char);
		}
	}

	return nSize;		
}

//searches through a node and looks for a list that has the first
//element set to the specified value
CLTANode* CLTAUtil::ShallowFindList(CLTANode* pNode, const char* pszValue)
{
	//must be a list
	if(!pNode->IsList())
	{
		return NULL;
	}

	//now run through all of its sub lists
	for(uint32 nCurrElem = 0; nCurrElem < pNode->GetNumElements(); nCurrElem++)
	{
		CLTANode* pElem = pNode->GetElement(nCurrElem);

		if(pElem->IsList())
		{
			//found a list

			//see if it's first item is a value
			if((pElem->GetNumElements() > 0) && (pElem->GetElement(0)->IsAtom()))
			{
				//now see if it matches
				if(strcmp(pElem->GetElement(0)->GetValue(), pszValue) == 0)
				{
					//success!
					return pElem;
				}
			}
		}
	}

	//failed to find it
	return NULL;
}

//searches through a node and looks for the first list to have an element
//as the first item with a string that matches the specified value. This
//will search into the tree
CLTANode* CLTAUtil::FindList(CLTANode* pNode, const char* pszValue)
{
	ASSERT(pNode);


	//we need to check and see if pNode is actually a valid list in itself,
	//because the loop down below will skip over the starting list
	if(pNode->IsList() && (pNode->GetNumElements() > 0))
	{
		CLTANode* pFirstChild = pNode->GetElement(0);
		if(pFirstChild->IsAtom() && (strcmp(pFirstChild->GetValue(), pszValue) == 0))
		{
			//it is a match
			return pNode;
		}
	}

	//the first node wasn't it, so have an iterator find it
	CLTANodeIterator	Iter(pNode);

	return Iter.FindNextList(pszValue);
}


//searches through a node and looks for all lists that have the first element
//set to the specified value. It will not search any deeper than the
//immediate children. It will return the number of nodes added to the list
uint32 CLTAUtil::ShallowFindAll(	std::vector<CLTANode*>& vNodeList,
									CLTANode* pStartNode,
									const char* pszValue)
{
	//sanity check
	ASSERT(pszValue);

	//no start, no children
	if(pStartNode == NULL)
	{
		return 0;
	}

	//build up the iterator
	CLTANodeIterator NodeIter(pStartNode);
	NodeIter.PushNode( NodeIter.GetCursorElement() );

	//count of all the nodes found
	uint32 nNumFound = 0;

	//now we build the list
	CLTANode* pCurrNode = NULL;

	do
	{
		pCurrNode = NodeIter.FindNextListShallow(pszValue);

		//see if we found one
		if(pCurrNode)
		{
			//add it to the vector
			vNodeList.push_back(pCurrNode);

			//increment the count
			nNumFound++;
		}

	}while(pCurrNode);

	//give them back the count
	return nNumFound;
}


//builds up a list of all the lists that have the first element set to
//the specified value. Returns the number of elements added
uint32 CLTAUtil::FindAll(std::vector<CLTANode*>& vNodeList, 
								CLTANode* pStart, 
								const char* pszValue)
{
	//sanity check
	ASSERT(pszValue);

	//if we don't have a starting node, just bail
	if(pStart == NULL)
	{
		return 0;
	}

	//build up the iterator
	CLTANodeIterator NodeIter(pStart);

	//count of all the nodes found
	uint32 nNumFound = 0;

	//we need to check and see if pStart is actually a valid list in itself,
	//because the loop down below will skip over the starting list
	if(pStart->IsList() && (pStart->GetNumElements() > 0))
	{
		CLTANode* pFirstChild = pStart->GetElement(0);
		if(pFirstChild->IsAtom() && (strcmp(pFirstChild->GetValue(), pszValue) == 0))
		{
			//it is a match
			vNodeList.push_back(pStart);
			nNumFound++;
		}
	}

	//now we build the list
	CLTANode* pCurrNode = NULL;

	do
	{
		pCurrNode = NodeIter.FindNextList(pszValue);

		//see if we found one
		if(pCurrNode)
		{
			//add it to the vector
			vNodeList.push_back(pCurrNode);

			//increment the count
			nNumFound++;
		}

	}while(pCurrNode);

	//give them back the count
	return nNumFound;
}
