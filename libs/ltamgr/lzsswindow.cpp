#include "lzsswindow.h"

CLZSSWindow::CLZSSWindow()
{
	Init();
}

CLZSSWindow::~CLZSSWindow()
{
}

//initializes the window
void CLZSSWindow::Init()
{
	//reset the writing position
	m_nLookAheadPos = 1;
	m_nLookAheadLen = 0;

	//clear out all of the nodes
	for(uint32 nCurrNode = 0; nCurrNode < WINDOW_SIZE + 1; nCurrNode++)
	{
		m_Tree[nCurrNode].m_nParent		= NULL_NODE;
		m_Tree[nCurrNode].m_nLarger		= NULL_NODE;
		m_Tree[nCurrNode].m_nSmaller	= NULL_NODE;
	}

	//reset the tree
	m_Tree[ROOT_NODE].m_nParent			= ROOT_NODE;
	m_Tree[ROOT_NODE].m_nLarger			= m_nLookAheadPos;
	m_Tree[ROOT_NODE].m_nSmaller		= NULL_NODE;

	//set up the first node
	m_Tree[m_nLookAheadPos].m_nParent	= ROOT_NODE;
	m_Tree[m_nLookAheadPos].m_nSmaller	= NULL_NODE;
	m_Tree[m_nLookAheadPos].m_nLarger	= NULL_NODE;

	m_nMatchLen = 0;
	m_nMatchPos = 0;

	m_bLookAheadInit = false;

}

//adds an element into the look ahead list. This will push other
//characters back into the window, and also push characters out
//of the window
bool CLZSSWindow::AddByte(uint8 nVal, CLTABitFile& BitFile)
{
	//DEBUG CODE
	static int32 nNumCalls = 0;
	nNumCalls++;

	//make sure that the look ahead buffer is initialized
	if(m_bLookAheadInit == false)
	{
		m_Window[m_nLookAheadPos + m_nLookAheadLen] = nVal;
		m_nLookAheadLen++;

		if(m_nLookAheadLen >= LOOK_AHEAD_SIZE)
		{
			m_bLookAheadInit = true;
		}
		return true;
	}

	//see if we have a full buffer
	if(IsLookAheadFull())
	{
		//the buffer is full, blast out a token
		if(WriteToken(BitFile) == false)
		{
			//disk must be full
			return false;
		}
	}

	ASSERT(m_nLookAheadLen < LOOK_AHEAD_SIZE);

	//now we need to fill in the look ahead buffer
	
	//remove the node we are just about to overwrite
	DeleteNode(WINDOW_POS(m_nLookAheadPos + LOOK_AHEAD_SIZE));

	//put the new value into the window

	//see if we are near the edge, if so, we need to write the ghost
	//buffer, and use protected calls
	if(m_nLookAheadPos >= (WINDOW_SIZE - LOOK_AHEAD_SIZE))
	{
		m_Window[WINDOW_POS(m_nLookAheadPos + LOOK_AHEAD_SIZE)] = nVal;

		//write to the ghost buffer
		m_Window[m_nLookAheadPos + LOOK_AHEAD_SIZE] = nVal;
		
		//advance our cursor through the window
		m_nLookAheadPos = WINDOW_POS(m_nLookAheadPos + 1);
	}
	else
	{
		//aren't near the edge, we can do all the unprotected accesses
		m_Window[m_nLookAheadPos + LOOK_AHEAD_SIZE] = nVal;
		m_nLookAheadPos++;
	}

	//add this new position into the tree
	AddNode(m_nLookAheadPos, m_nMatchPos, m_nMatchLen);

	//adjust our size
	m_nLookAheadLen++;

	//success
	return true;
}

//writes out tokens until the entire look ahead buffer has been written.
//should only be called when closing a file
bool CLZSSWindow::FlushLookAhead(CLTABitFile& BitFile)
{
	//if they have already filled the look ahead buffer, but the look ahead buffer
	//is not full now, that means we need to skip along in the window.
	if(m_bLookAheadInit)
	{
		m_nLookAheadPos = WINDOW_POS(m_nLookAheadPos + (LOOK_AHEAD_SIZE - m_nLookAheadLen));
	}

	//just write out tokens until there is nothing in the input buffer anymore
	while(m_nLookAheadLen > 0)
	{
		//keep track of the previous length
		//uint32 nOldLookAheadLen = m_nLookAheadLen;
		m_nMatchLen = 0;

		if(WriteToken(BitFile) == false)
		{
			//disk is full
			return false;
		}

		//advance our cursor through the window
		m_nLookAheadPos = WINDOW_POS(m_nLookAheadPos + 1);
	}

	//output the end of stream character
	WriteSpan(BitFile, 0, (BREAK_EVEN_POINT + 1));

	return true;
}

//writes a span with the specified offset and length out to the given bitfile
bool CLZSSWindow::WriteSpan(CLTABitFile& BitFile, uint32 nOffset, uint32 nLength)
{
	ASSERT(nLength >= (BREAK_EVEN_POINT + 1));

	//set a bit that indicates that it is a run
	BitFile.SetBit(0);

	//write out the offset
	uint32 nCurrBit;
	ASSERT(nOffset < (1 << NUM_OFFSET_BITS));

	for(nCurrBit = 1; nCurrBit <= NUM_OFFSET_BITS; nCurrBit++)
	{
		BitFile.SetBit((nOffset & (1 << (NUM_OFFSET_BITS - nCurrBit))) ? 1 : 0);
	}

	//write out the length

	//take out the break even point since we can never have a run less than that
	uint32 nFinalLength = nLength - (BREAK_EVEN_POINT + 1);
	ASSERT(nFinalLength < (1 << NUM_LENGTH_BITS));

	for(nCurrBit = 1; nCurrBit <= NUM_LENGTH_BITS; nCurrBit++)
	{
		BitFile.SetBit((nFinalLength & (1 << (NUM_LENGTH_BITS - nCurrBit))) ? 1 : 0);
	}
	
	return true;
}

//looks through the look ahead buffer, finds the largest match
//in the window, and returns the offset and writes out the token
//to the buffer
bool CLZSSWindow::WriteToken(CLTABitFile& BitFile)
{
	uint32 nMatchPos = m_nMatchPos;
	uint32 nMatchLen = m_nMatchLen;

	//make sure to clamp the value to the actual maximum size of the look ahead
	//buffer
	if(nMatchLen > m_nLookAheadLen)
	{
		nMatchLen = m_nLookAheadLen;
	}


	//now we know how many bytes we can shift out, etc

	//see if we can save space by just printing out bytes
	if(nMatchLen <= BREAK_EVEN_POINT)
	{
		//print the flag indicating that it is just a raw byte
		BitFile.SetBit(1);

		//now print out the byte
		uint8 nVal = m_Window[m_nLookAheadPos];
		for(uint32 nCurrBit = 1; nCurrBit <= 8; nCurrBit++)
		{
			BitFile.SetBit((nVal & (1 << (8 - nCurrBit))) ? 1 : 0);
		}

		//set the amount we need to print out
		nMatchLen = 1;
	}
	else
	{
		WriteSpan(BitFile, nMatchPos, nMatchLen);		
	}

	//shrink the size of the look ahead buffer
	ASSERT(m_nLookAheadLen >= nMatchLen);
	m_nLookAheadLen -= nMatchLen;

	return true;
}

//determines how many tokens are in the look ahead buffer
bool CLZSSWindow::IsLookAheadFull() const
{
	return (m_nLookAheadLen >= LOOK_AHEAD_SIZE) ? true : false;
}

//--------------------
//Tree manipulation

//collapses a node. This takes a node with only one child, and replaces itself
//in the tree with the children
void CLZSSWindow::CollapseNode(uint32 nNode, uint32 nChild)
{
	//sanity checks

	//make sure the node is indeed a child of its parent
	ASSERT(	(m_Tree[m_Tree[nNode].m_nParent].m_nSmaller == nNode) || 
			(m_Tree[m_Tree[nNode].m_nParent].m_nLarger == nNode));
	//make sure that child is indeed a child of node
	ASSERT((m_Tree[nNode].m_nSmaller == nChild) || (m_Tree[nNode].m_nLarger == nChild));
	//make sure that the node does indeed have one node null
	ASSERT((m_Tree[nNode].m_nSmaller == NULL_NODE) || (m_Tree[nNode].m_nLarger == NULL_NODE));
	//make sure it is a valid index
	ASSERT((nNode != NULL_NODE) && (nNode < WINDOW_SIZE));

	//get the parent node
	uint32 nParent = m_Tree[nNode].m_nParent;

	//see which branch of the parent we will modify
	if(m_Tree[nParent].m_nSmaller == nNode)
	{
		m_Tree[nParent].m_nSmaller = nChild;
	}
	else
	{
		m_Tree[nParent].m_nLarger = nChild;
	}

	//clean up the removed node
	m_Tree[nChild].m_nParent	= nParent;
	m_Tree[nNode].m_nParent		= NULL_NODE;
}

//replaces a node with the specified node
void CLZSSWindow::ReplaceNode(uint32 nNode, uint32 nReplaceWith)
{
	//make sure the node is indeed a child of its parent
	ASSERT(	(m_Tree[m_Tree[nNode].m_nParent].m_nSmaller == nNode) || 
			(m_Tree[m_Tree[nNode].m_nParent].m_nLarger == nNode));

	//get the parent
	uint32 nParent = m_Tree[nNode].m_nParent;

	//link the parent to the new node
	if(m_Tree[nParent].m_nLarger == nNode)
	{
		m_Tree[nParent].m_nLarger = nReplaceWith;
	}
	else
	{
		m_Tree[nParent].m_nSmaller = nReplaceWith;
	}

	//set up the new node
	m_Tree[nReplaceWith].m_nParent	= m_Tree[nNode].m_nParent;
	m_Tree[nReplaceWith].m_nLarger	= m_Tree[nNode].m_nLarger;
	m_Tree[nReplaceWith].m_nSmaller = m_Tree[nNode].m_nSmaller;

	//parentize the children correctly
	m_Tree[ m_Tree[nReplaceWith].m_nSmaller ].m_nParent = nReplaceWith;
	m_Tree[ m_Tree[nReplaceWith].m_nLarger  ].m_nParent = nReplaceWith;

	//mark the old node as invalid
	m_Tree[nNode].m_nParent = NULL_NODE;
}

//deletes a node from the tree
void CLZSSWindow::DeleteNode(uint32 nNode)
{
	if((m_Tree[nNode].m_nParent == NULL_NODE) || (nNode == NULL_NODE))
	{
		return;
	}

	//first check to see if we can simply collapse the node (meaning
	//it has only one child)
	if(m_Tree[nNode].m_nLarger == NULL_NODE)
	{
		CollapseNode(nNode, m_Tree[nNode].m_nSmaller);
	}
	else if(m_Tree[nNode].m_nSmaller == NULL_NODE)
	{
		CollapseNode(nNode, m_Tree[nNode].m_nLarger);
	}
	//can't collapse it, have to remove it
	else
	{
		//find the one to replace it with
		uint32 nReplaceWith = m_Tree[nNode].m_nSmaller;

		//continually move down the larger side until we have to stop
		while(m_Tree[nReplaceWith].m_nLarger != NULL_NODE)
		{
			nReplaceWith = m_Tree[nReplaceWith].m_nLarger;
		}

		//clean up that node (guranteed to be only a collapse on the smaller
		//side, since for it to be picked, larger had to be NULL_NODE)
		CollapseNode(nReplaceWith, m_Tree[nReplaceWith].m_nSmaller);
		//now we swap the two nodes
		ReplaceNode(nNode, nReplaceWith);
	}		
}

//adds a node into the tree, finding the largest match as it adds the node
void CLZSSWindow::AddNode(uint32 nWindowNode, uint32& nMatchPos, uint32& nMatchLen)
{

	//init the match to something invalid
	nMatchLen = 0;
	nMatchPos = NULL_NODE;

	//see if we are on a valid node
	if(nWindowNode == NULL_NODE)
	{
		//nope
		return;
	}

	//start out at the first node (stored in the root's larger branch)
	uint32 nCurrNode = m_Tree[ROOT_NODE].m_nLarger;

	ASSERT(nCurrNode != NULL_NODE);

	//pointer to the window where we will be comparing
	uint8* pWindowPos = m_Window + nWindowNode;
	uint8* pCurrWindowPos;
	uint32* pNextNode;

	//the ending address
	uint8* pEndWindowPos = pWindowPos + LOOK_AHEAD_SIZE;

	//the strcmp return value essentially
	int32 nDelta;

	//count up the match
	uint32 nCurrMatchLen;


	//now we run through the tree moving appropriately, counting the match lengths
	while(1)
	{
		//reset the match count
		nCurrMatchLen = 0;
		pCurrWindowPos = m_Window + nCurrNode;

		do
		{
			nDelta = *(pWindowPos + nCurrMatchLen) - *(pCurrWindowPos + nCurrMatchLen);

			//see if the strings are not equal
			if(nDelta != 0)
			{
				break;
			}

			//matched another character
			nCurrMatchLen++;
			

			//unroll the loop once

			nDelta = *(pWindowPos + nCurrMatchLen) - *(pCurrWindowPos + nCurrMatchLen);

			//see if the strings are not equal
			if(nDelta != 0)
			{
				break;
			}

			//matched another character
			nCurrMatchLen++;

		}while(nCurrMatchLen < LOOK_AHEAD_SIZE);

		//see if this match is the best one so far
		if(nCurrMatchLen > nMatchLen)
		{
			if(nCurrMatchLen >= LOOK_AHEAD_SIZE)
			{
				//save the settings
				nMatchLen = LOOK_AHEAD_SIZE;
				nMatchPos = nCurrNode;

				//since we have an exact match, we can simply replace the old node with
				//this one (since it would be pulled out of the buffer first) which
				//prevents buffer clutter
				ReplaceNode(nCurrNode, nWindowNode);

				//bail, since we couldn't possibly find a better match
				return;
			}

			//save the settings
			nMatchLen = nCurrMatchLen;
			nMatchPos = nCurrNode;
		}

		//see where we need to travel next
		if(nDelta > 0)
		{
			pNextNode = &(m_Tree[nCurrNode].m_nLarger);
		}
		else
		{
			pNextNode = &(m_Tree[nCurrNode].m_nSmaller);
		}

		//now we need to see if we hit a dead end
		if(*pNextNode == NULL_NODE)
		{
			//found a dead end, insert our node here
			*pNextNode = nWindowNode;

			//update the new node's links
			m_Tree[nWindowNode].m_nParent	= nCurrNode;
			m_Tree[nWindowNode].m_nLarger	= NULL_NODE;
			m_Tree[nWindowNode].m_nSmaller	= NULL_NODE;

			return;
		}

		nCurrNode = *pNextNode;
	}

	//should never get here
	ASSERT(false);		
}
