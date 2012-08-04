//-------------------------------------------------------------------
// LTANodeIterator.h
//
// Provides the definition for CLTANodeIterator. This allows the
// creation of an iterator given a root node, and allows the
// searching through of the tree for specific elements.
//
// Created: 1/17/01
// Author: John O'Rorke
// Modification History:
//
//-------------------------------------------------------------------
#ifndef __LTANODEITERATOR_H__
#define __LTANODEITERATOR_H__

#ifndef __LTANODE_H__
#	include "ltanode.h"
#endif

#ifndef __LTALIMITS_H__
#	include "ltalimits.h"
#endif

class CLTANodeIterator
{
public:

	//create an iterator starting at this root
	CLTANodeIterator(CLTANode* pRoot);

	~CLTANodeIterator();

	//pushes a specified node onto the stack. Identical to stepping
	//into that node
	bool		PushNode(CLTANode* pNode);

	//pops a node from the stack. Identical to traversing back up to 
	//the node's parent. Returns the node that was popped, or NULL if
	//it fails
	CLTANode*	PopNode();

	//finds the next list with the first element being a value
	//with a string that matches the specified string. Updates
	//the position of the cursor to point to that node
	CLTANode*	FindNextList(const char* pszValue);

	//moves onto the next value in the current node's list. If the current
	//element is a list, it will be searched through recursively. Calling this continually
	//will iterate through all values in the tree. Returns NULL if no more
	CLTANode*	NextValue();

	//moves onto the next list in the current node's list. If the current
	//element is a list, it will be searched through recursively. Calling this continually
	//will iterate through all lists in the tree. Returns NULL if no more
	CLTANode*	NextList();

	//moves onto the next element (either a value or list). If the current
	//element is a list, it will be searched through recursively. Calling this continually
	//will iterate through all elements in the tree. Returns NULL if no more
	CLTANode*	NextElement();

	//finds the next list with the appropriate first element being
	//a value with the specified text, but does not go deeper than
	//one level
	CLTANode*	FindNextListShallow(const char* pszValue);

	//moves onto the next value in the current node's list. Returns NULL if no more
	CLTANode*	NextValueShallow();

	//moves onto the next list in the current node's list. Returns NULL if no more
	CLTANode*	NextListShallow();

	//moves onto the next element (either a value or list). Returns NULL if no more
	CLTANode*	NextElementShallow();

	//resets the cursor so that the elements being searched return back to the head
	//of the current node
	void		ResetCursorElement();

	//resets the entire iterator, so it begins again at the root
	void		Reset();

	//gets the head of the stack
	CLTANode*	GetHead();

	//gets the current element being pointed at
	CLTANode*	GetCursorElement();


private:

	//get the current element index
	uint32			GetElementIndex() const;

	//sets the element index
	void			SetElementIndex(uint32 nVal);

	//the node stack
	CLTANode*		m_pNodeStack[MAX_LTA_DEPTH];
	uint32			m_nElemStack[MAX_LTA_DEPTH];

	//the position in the stack (the head)
	uint32			m_nStackPos;

};

#endif

