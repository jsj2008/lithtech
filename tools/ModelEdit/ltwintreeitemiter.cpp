#include "ltwintreeitemiter.h"
#include "ltwintreeitem.h"

//allow the copying of iterators so that positions can be saved
CLTWinTreeItemIter::CLTWinTreeItemIter(const CLTWinTreeItemIter& rhs)
{
	*this = rhs;
}

//allow the copying of iterators so that positions can be saved
const CLTWinTreeItemIter& CLTWinTreeItemIter::operator=(const CLTWinTreeItemIter& rhs)
{
	m_pCurrent = rhs.m_pCurrent;
	return *this;
}

//gets the item the iterator is currently on
CLTWinTreeItem*	CLTWinTreeItemIter::Current()
{
	return m_pCurrent;
}

//gets the next item in the list, and advances the iterator
CLTWinTreeItem* CLTWinTreeItemIter::Next()
{
	if(m_pCurrent)
	{
		m_pCurrent = m_pCurrent->m_pNextSibling;
	}
	return m_pCurrent;
}

//determines if there are any more items to iterate through
BOOL CLTWinTreeItemIter::IsMore() const
{
	if(m_pCurrent)
	{
		return TRUE;
	}
	return FALSE;
}

//make the constructor private so that only items
//can create iterators, this prevents invalid iterators
CLTWinTreeItemIter::CLTWinTreeItemIter(CLTWinTreeItem* pStart)
{
	m_pCurrent = pStart;
}


