#include "ltwintreeitem.h"
#include "ltwintreeitemiter.h"

//default constructor
CLTWinTreeItem::CLTWinTreeItem(const char* pszText, CLTWinTreeIcon Icon) :
	m_pNextSibling(NULL),
	m_pFirstChild(NULL),
	m_pPrevSibling(NULL),
	m_pParent(NULL),
	m_sText(pszText),
	m_Icon(Icon),
	m_bEditText(TRUE),
	m_bSelected(FALSE)
{
}

//deault destructor
CLTWinTreeItem::~CLTWinTreeItem()
{
}

//creates an iterator that iterates through all the siblings, starting
//with this current tree item. NOTE: the callee is responsible for
//deleting the iterator
CLTWinTreeItemIter* CLTWinTreeItem::CreateSiblingIter()
{
	return new CLTWinTreeItemIter(this);
}

//crates an iterator that iterates through all the children, starting
//with the first child. NOTE: the callee is responsible for
//deleting the iterator
CLTWinTreeItemIter* CLTWinTreeItem::CreateChildIter()
{
	return new CLTWinTreeItemIter(m_pFirstChild);
}

//used for finding equal items
BOOL CLTWinTreeItem::operator==(const CLTWinTreeItem& rhs) const
{
	return (GetType() == rhs.GetType()) ? TRUE : FALSE;
}

//gets the unique identifier for the type of item this is
DWORD CLTWinTreeItem::GetType() const
{
	return BASE_ITEM;
}

//determines if this item is selected or not
BOOL CLTWinTreeItem::IsSelected() const
{
	return m_bSelected;
}

//gets the text associated with the item
const char* CLTWinTreeItem::GetText() const
{
	return (const char*)m_sText;
}

//returns the index to the icon associated with this item
CLTWinTreeIcon	CLTWinTreeItem::GetIcon() const
{
	return m_Icon;
}

//determines if this is a valid drop target for the passed in item
BOOL CLTWinTreeItem::IsDropTarget(const CLTWinTreeItem* pItem) const
{
	return TRUE;
}


//determines if the text can be modified by the user
BOOL CLTWinTreeItem::IsTextEditable() const
{
	return m_bEditText;
}

//sets whether or not the text is modifiable
void CLTWinTreeItem::SetTextEditable(BOOL bEditable)
{
	m_bEditText = bEditable;
}

//determines if this object is, or contains the object as a child
BOOL CLTWinTreeItem::ContainsObject(const CLTWinTreeItem* pItem)
{
	//see if this object is the passed in one
	if(this == pItem)
	{
		return TRUE;
	}

	//it isn't, go through the children
	CLTWinTreeItemIter* pIter = CreateChildIter();

	//check to make sure iterator is valid
	if(pIter == FALSE)
	{
		return FALSE;
	}

	for(; pIter->IsMore(); pIter->Next())
	{
		//see if this child contains it
		if(pIter->Current()->ContainsObject(pItem))
		{
			//it does, so clean up and bail
			delete pIter;
			return TRUE;
		}
	}

	//couldn't find the item, clean up and bail
	delete pIter;
	return FALSE;
}
