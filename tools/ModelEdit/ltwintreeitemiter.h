////////////////////////////////////////////////////////////////
//
// ltwintreeitemiter.h
//
// This is the iterator class for the CLTWinTreeItem items. 
// This iterates through all siblings of an item, and then those
// items can then be used to iterate through children, etc, 
// allowing for tree navigation. The iterators can only be cloned
// not created by users, to avoid invalid iterators. For info
// on the purpose and how to use iterators, refer to the Design
// Patterns book section on iterators.
//
// Author: John O'Rorke
// Created: 7/21/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __LTWINTREEITEMITER_H__
#define __LTWINTREEITEMITER_H__

#include "stdafx.h"

class CLTWinTreeItem;

class CLTWinTreeItemIter
{
public:

	//allow the copying of iterators so that positions can be saved
	CLTWinTreeItemIter(const CLTWinTreeItemIter& rhs);

	//allow the copying of iterators so that positions can be saved
	const CLTWinTreeItemIter& operator=(const CLTWinTreeItemIter& rhs);

	//gets the item the iterator is currently on
	CLTWinTreeItem*	Current();

	//gets the next item in the list, and advances the iterator
	CLTWinTreeItem* Next();

	//determines if there are any more items to iterate through
	BOOL IsMore() const;

private:

	//make the items a friend so that they can create
	//these iterators
	friend class CLTWinTreeItem;

	//make the constructor private so that only items
	//can create iterators, this prevents invalid iterators
	CLTWinTreeItemIter(CLTWinTreeItem* pStart);

	//the current item we are iterating through
	CLTWinTreeItem* m_pCurrent;

};

#endif

