//-------------------------------------------------------------------
// LTAHuffmanTree.h
//
// Provides definition for CLTAHuffmanTree, which provides a means
// of managing a binary tree needed for the standard huffman 
// compression algorithm. It allows an initial tree to be constructed
// from a set of initial weights, and then to update the tree based
// upon incrementing weights.
//
// Created: 1/12/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------
#ifndef __LTAHUFFMANTREE_H__
#define __LTAHUFFMANTREE_H__

#include "ltbasedefs.h"

//hidden internal class
class CLTAHuffmanNode;

class CLTAHuffmanTree
{
public:

	CLTAHuffmanTree();
	~CLTAHuffmanTree();

	//creates a tree given the initial weights of each character. It is
	//assumed this passed in pointer points to 256 elements
	bool CreateTree(uint32* pnWeightsets);

	//increments the weight of the specified character, and maintains
	//the tree's defining properties
	bool IncrementWeight(uint8 nItem);

	//gets the weight of a specified item
	uint32 GetWeight(uint8 nItem) const;

	//gets the bits needed to encode a character
	bool GetBits(uint8 nItem, uint32& nBits, uint32& nNumBits);

	//used for decoding data, this function should be called for each bit.
	//it will return true when a leaf has been hit, and then the passed
	//in byte will hold the properly decoded byte, and the decoding
	//cursor will be reset
	bool DecodeBit(uint8 nBit, uint8& nOutByte);
	
private:

	//don't allow copying of this object
	CLTAHuffmanTree(const CLTAHuffmanTree&) {}

	//traverses through the tree and scales the weights of each object by cutting them in half
	//it will preserve all tree properties.
	bool ScaleTree();

	//clears the existing tree
	void Free();

	//the decoding cursor
	CLTAHuffmanNode*	m_pDecodePos;

	//the root node
	CLTAHuffmanNode*	m_pRoot;

	//the list of pointers to all the leaves
	//for quick encoding
	CLTAHuffmanNode*	m_pLeaves[256];

	//the list of all leaves in order of their weights, with the larger weights
	//being lower in the list
	CLTAHuffmanNode*	m_pWeights[512];
};

#endif
