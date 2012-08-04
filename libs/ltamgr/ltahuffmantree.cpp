#include "ltahuffmantree.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

//CLTAHuffmanNode, utility class the the CLTAHuffmanTree, which
//provides a binary tree representation with a weight set
//and utility functions
class CLTAHuffmanNode
{
public:

	//constructor
	CLTAHuffmanNode(	uint32 nWeight = 0,	uint8 nValue = 0,
						CLTAHuffmanNode* pLeft = NULL, 
						CLTAHuffmanNode* pRight = NULL,
						CLTAHuffmanNode* pParent = NULL) :
		m_pLeft(pLeft),
		m_pRight(pRight),
		m_pParent(pParent),
		m_nWeight(nWeight),
		m_nValue(nValue)
	{
	}

	//destructor
	~CLTAHuffmanNode()
	{
		FreeChildren();
	}

	//determines if this node is a leaf
	bool IsLeaf()
	{
		return !(GetLeft() || GetRight());
	}

	//deletes its children in a recursive fashion
	void FreeChildren()
	{
		//clear out the left branch
		if(m_pLeft)
		{
			m_pLeft->FreeChildren();
			delete m_pLeft;
			m_pLeft = NULL;
		}
		//clear out the right branch
		if(m_pRight)
		{
			m_pRight->FreeChildren();
			delete m_pRight;
			m_pRight = NULL;
		}
	}

	//accessors for the nodes
	CLTAHuffmanNode*		GetLeft()							{return m_pLeft;}
	void					SetLeft(CLTAHuffmanNode* pLeft)		{m_pLeft = pLeft;}

	CLTAHuffmanNode*		GetRight()							{return m_pRight;}
	void					SetRight(CLTAHuffmanNode* pRight)	{m_pRight = pRight;}

	CLTAHuffmanNode*		GetParent()							{return m_pParent;}
	void					SetParent(CLTAHuffmanNode* pParent)	{m_pParent = pParent;}


	//accessors for the weight
	uint32					GetWeight() const					{return m_nWeight;}
	void					SetWeight(uint32 nWeight)			{m_nWeight = nWeight;}

	//accessors for the weight index
	uint16					GetWeightIndex() const				{return m_nWeightIndex;}
	void					SetWeightIndex(uint16 nWeightIndex)	{m_nWeightIndex = nWeightIndex;}

	//accessors for the value
	uint8					GetValue() const					{return m_nValue;}
	void					SetValue(uint8 nValue)				{m_nValue = nValue;}

private:

	//index into the weight list
	uint16				m_nWeightIndex;

	uint32				m_nWeight;
	uint8				m_nValue;

	//nodes
	CLTAHuffmanNode*	m_pLeft;
	CLTAHuffmanNode*	m_pRight;

	CLTAHuffmanNode*	m_pParent;
};


CLTAHuffmanTree::CLTAHuffmanTree() :
	m_pRoot(NULL),
	m_pDecodePos(NULL)
{
	//clear out all the leaves
	memset(m_pLeaves, 0, sizeof(m_pLeaves));
}

CLTAHuffmanTree::~CLTAHuffmanTree()
{
	//clear the tree
	Free();
}


//creates a tree given the initial weights of each character. It is
//assumed this passed in pointer points to 256 elements
bool CLTAHuffmanTree::CreateTree(uint32* pnWeightsets)
{
	//clear the old tree
	Free();

	//list for the nodes that have yet to be build
	CLTAHuffmanNode* pNodes[256];

	//clear out the node list
	memset(m_pLeaves, 0, sizeof(pNodes));

	//clear out the weight list
	memset(m_pWeights, 0, sizeof(m_pWeights));

	//create all the initial leaves
	for(uint32 nCurrElem = 0; nCurrElem < 256; nCurrElem++)
	{
		LT_MEM_TRACK_ALLOC(pNodes[nCurrElem]		= new CLTAHuffmanNode(pnWeightsets[nCurrElem], nCurrElem),LT_MEM_TYPE_MISC);
		m_pLeaves[nCurrElem]	= pNodes[nCurrElem];
	}

	//now we build up the tree

	//start building up the weight list in reverse order
	uint32 nWeightIndex = 510;

	//continue as long as we have more than one leaf in the list
	for(uint32 nNodesLeft = 256; nNodesLeft > 1; nNodesLeft--)
	{
		//run through and find the two smallest leaves. The
		//smallest one is always in 0, the larger is in 1
		uint32 nSmallest[2];

		//init the smallest
		if(pNodes[0]->GetWeight() < pNodes[1]->GetWeight())
		{
			nSmallest[0] = 0;
			nSmallest[1] = 1;
		}
		else
		{
			nSmallest[0] = 1;
			nSmallest[1] = 0;
		}

		//loop through all the nodes
		for(uint32 nCurrNode = 2; nCurrNode < nNodesLeft; nCurrNode++)
		{
			//see if this one is smaller
			if(pNodes[nCurrNode]->GetWeight() < pNodes[nSmallest[1]]->GetWeight())
			{
				//found one smaller, so set it to the larger of the smaller
				nSmallest[1] = nCurrNode;

				//make sure that the second element stays the largest
				if(pNodes[nSmallest[0]]->GetWeight() > pNodes[nSmallest[1]]->GetWeight())
				{
					uint32 nTemp = nSmallest[0];
					nSmallest[0] = nSmallest[1];
					nSmallest[1] = nTemp;
				}
			}
		}

		//now that we have the two elements, we need to create a parent node for it
		uint32 nParentWeight = pNodes[nSmallest[0]]->GetWeight() + pNodes[nSmallest[1]]->GetWeight();
		LT_MEM_TRACK_ALLOC(CLTAHuffmanNode* pParent = new CLTAHuffmanNode(nParentWeight, 0, 
														pNodes[nSmallest[0]], pNodes[nSmallest[1]]),LT_MEM_TYPE_MISC);

		//parentize the children
		pNodes[nSmallest[0]]->SetParent(pParent);
		pNodes[nSmallest[1]]->SetParent(pParent);

		//save the weights in descending order
		m_pWeights[nWeightIndex] = pNodes[nSmallest[0]];
		pNodes[nSmallest[0]]->SetWeightIndex(nWeightIndex--);

		m_pWeights[nWeightIndex] = pNodes[nSmallest[1]];
		pNodes[nSmallest[1]]->SetWeightIndex(nWeightIndex--);

		//put the parent in the slot of the smallest child
		uint32 nMinSlot = min(nSmallest[0], nSmallest[1]);
		pNodes[nMinSlot] = pParent;

		//now move the last element into the larger slot to make the list
		//continuous
		uint32 nMaxSlot = max(nSmallest[0], nSmallest[1]);
		pNodes[nMaxSlot] = pNodes[nNodesLeft - 1];
		pNodes[nNodesLeft - 1] = NULL;
	}

	//the last node in our tree is our root
	ASSERT(pNodes[0]);
	m_pRoot = pNodes[0];

	//save the root as the big bad weight
	ASSERT(nWeightIndex == 0);
	m_pWeights[0] = m_pRoot;

	m_pRoot->SetWeightIndex(0);

	//set up decoding
	m_pDecodePos = m_pRoot;

	return true;
}


//increments the weight of the specified character, and maintains
//the tree's defining properties
bool CLTAHuffmanTree::IncrementWeight(uint8 nItem)
{
	//sanity checks
	ASSERT(m_pRoot);
	ASSERT(m_pLeaves[nItem]);

	CLTAHuffmanNode* pCurr = m_pLeaves[nItem];

	while(pCurr)
	{
		//increment the weight
		pCurr->SetWeight(pCurr->GetWeight() + 1);

		//see if we need to swap it
		for(uint32 nTestNode = pCurr->GetWeightIndex(); nTestNode > 0; nTestNode--)
		{
			if(m_pWeights[nTestNode - 1]->GetWeight() >= pCurr->GetWeight())
			{
				break;
			}
		}
		
		if(m_pWeights[nTestNode] != pCurr)
		{
			CLTAHuffmanNode* pSwap = m_pWeights[nTestNode];

			//swap the nodes

			//start off by changing the parents
			CLTAHuffmanNode*	pParents[2];
			pParents[0] = pCurr->GetParent();
			pParents[1] = pSwap->GetParent();
			pCurr->SetParent(pParents[1]);
			pSwap->SetParent(pParents[0]);

			//now swap the parent's links
			if(pParents[0]->GetLeft() == pCurr)
			{
				pParents[0]->SetLeft(pSwap);
			}
			else
			{
				pParents[0]->SetRight(pSwap);
			}

			if(pParents[1]->GetLeft() == pSwap)
			{
				pParents[1]->SetLeft(pCurr);
			}
			else
			{
				pParents[1]->SetRight(pCurr);
			}

			//make sure to cache this because it will be changing
			uint32 nOriginalWeight = pCurr->GetWeightIndex();

			//swap their weight indices
			uint16 nTemp = pSwap->GetWeightIndex();
			pSwap->SetWeightIndex(nOriginalWeight);
			pCurr->SetWeightIndex(nTemp);

			//now swap the order in the weight table
			CLTAHuffmanNode* pTemp				= m_pWeights[nTestNode];
			m_pWeights[nTestNode]				= m_pWeights[nOriginalWeight];
			m_pWeights[nOriginalWeight]			= pTemp;		

			//set this as the current
			pCurr = pSwap;
		}

		//move onto the parent
		pCurr = pCurr->GetParent();
	}

	return true;
}


//gets the weight of a specified item
uint32 CLTAHuffmanTree::GetWeight(uint8 nItem) const
{
	//make sure we have a tree
	ASSERT(m_pRoot);
	ASSERT(m_pLeaves[nItem]);

	//return the wright
	return m_pLeaves[nItem]->GetWeight();
}


//gets the bits needed to encode a character
bool CLTAHuffmanTree::GetBits(uint8 nItem, uint32& nBits, uint32& nNumBits)
{
	//make sure the tree is valid
	ASSERT(m_pRoot);
	ASSERT(m_pLeaves[nItem]);

	//first off, get the leaf of the bits we are looking for
	CLTAHuffmanNode* pCurr = m_pLeaves[nItem];

	//setup the mask
	uint32 nMask = 0x01;

	//init the counts
	nNumBits	= 0;
	nBits		= 0;
	
	//now we need to go up the tree, and determine the bit pattern to get this item
	do
	{
		//sanity check
		ASSERT(pCurr);
		ASSERT(pCurr->GetParent());

		//determine if we are the left or right child
		if(pCurr->GetParent()->GetRight() == pCurr)
		{
			//we are on the left hand side, need to set the bit
			nBits |= nMask;
		}

		//move on
		nMask <<= 1;
		nNumBits++;

		pCurr = pCurr->GetParent();

		//sanity check
		ASSERT(nNumBits < 32);

		//repeat this until we hit the root
	}while(pCurr != m_pRoot);

	return true;
}


//used for decoding data, this function should be called for each bit.
//it will return true when a leaf has been hit, and then the passed
//in byte will hold the properly decoded byte, and the decoding
//cursor will be reset
bool CLTAHuffmanTree::DecodeBit(uint8 nBit, uint8& nOutByte)
{
	//make sure the existing position is valid
	ASSERT(m_pDecodePos);
	//make sure the tree is valid
	ASSERT(m_pRoot);

	//move the cursor based upon the bit
	if(!nBit)
	{
		//make sure we are moving into a real node
		ASSERT(m_pDecodePos->GetLeft());
		m_pDecodePos = m_pDecodePos->GetLeft();
	}
	else
	{
		//make sure we are moving into a real node
		ASSERT(m_pDecodePos->GetRight());
		m_pDecodePos = m_pDecodePos->GetRight();
	}

	//see if we have hit a leaf
	if(m_pDecodePos->IsLeaf())
	{
		nOutByte		= m_pDecodePos->GetValue();
		m_pDecodePos	= m_pRoot;
		return true;
	}

	return false;
}


//utility function for scaling the tree
static void ScaleTreeRecurse(CLTAHuffmanNode* pNode)
{
	if(pNode)
	{
		if(pNode->IsLeaf())
		{
			//if this is a leaf, then just divide the weight
			pNode->SetWeight((pNode->GetWeight() + 1) / 2);
		}
		else
		{
			//scale the children
			ScaleTreeRecurse(pNode->GetLeft());
			ScaleTreeRecurse(pNode->GetRight());

			//now set the weight by adding up the children
			pNode->SetWeight(	(pNode->GetLeft()->GetWeight() + 
								 pNode->GetRight()->GetWeight()) / 2);
		}
	}
}


//traverses through the tree and scales the weights of each object by cutting them in half
//it will preserve all tree properties.
bool CLTAHuffmanTree::ScaleTree()
{
	if(m_pRoot == NULL)
	{
		return false;
	}

	ScaleTreeRecurse(m_pRoot);

	return true;
}


//clears the existing tree
void CLTAHuffmanTree::Free()
{
	if(m_pRoot)
	{
		//clear out the root's children
		m_pRoot->FreeChildren();

		//clear out the root
		delete m_pRoot;
		m_pRoot = NULL;
	}

	//clear out the other pointers
	m_pDecodePos = NULL;
	memset(m_pLeaves, 0, sizeof(m_pLeaves));
	memset(m_pWeights, 0, sizeof(m_pWeights));
}

