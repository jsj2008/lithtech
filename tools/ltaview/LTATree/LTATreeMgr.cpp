#include "ltatreemgr.h"
#include "ltwintreeitem.h"

// lta
#include "ltamgr.h"


CLTATreeMgr::CLTATreeMgr() :
	m_pLTAIconList(NULL),
	m_nNumLTAIcons(0)
{
}

CLTATreeMgr::~CLTATreeMgr()
{
	FreeLTAIcons();
}

//loads an LTA file, and adds it to the tree from the specified node,
//or from the root if the key is invalid. Each created item will have an
//icon associated to it from the list specified in AddLTAIcons
BOOL CLTATreeMgr::LoadLTA(const char* pszFileName, CLTWinTreeKey Parent)
{

	CLTANode* pRoot = new CLTANode;

	//make sure that the root could be allocated successfully
	if(pRoot == NULL)
	{
		return FALSE;
	}
	
	CLTALoadOnlyAlloc Allocator(2048);

	if(CLTANodeReader::LoadEntireFile(pszFileName, CLTAUtil::IsFileCompressed(pszFileName), pRoot, &Allocator) == false)
	{
		return FALSE;
	}

	//recursively run through all the children in 
	//the parse tree and build up the local tree
	for(uint32 nChild = 0; nChild < pRoot->GetNumElements(); nChild++)
	{
		RecurseBuildTree(pRoot->GetElement(nChild), Parent, DEFAULTICON);
	}

	//delete the LTA tree we no longer need
	Allocator.FreeNode(pRoot);
	Allocator.FreeAllMemory();

	return TRUE;
}

//adds an icon to the list, and associates it with a keyword. The first
//icon is the one that will be associated with the item itself, the other
//is the default icon for its children, both are windows icon IDs. 
BOOL CLTATreeMgr::AddLTAIcon(const char* pszKeyword, DWORD nIcon, DWORD nChildIcon)
{
	//get the current number of icons (important since resizing will change this)
	DWORD nNumIcons = GetNumLTAIcons();

	//resize the list to add room for the new icon
	if(ResizeLTAIcons(nNumIcons + 1,  TRUE) == FALSE)
	{
		//major failure
		return FALSE;
	}

	//set the new last item to this
	m_pLTAIconList[nNumIcons].m_sKeyword = pszKeyword;
	m_pLTAIconList[nNumIcons].m_sKeyword.MakeUpper();
	m_pLTAIconList[nNumIcons].m_Icon = AddIcon(MAKEINTRESOURCE(nIcon));
	m_pLTAIconList[nNumIcons].m_ChildDefIcon = AddIcon(MAKEINTRESOURCE(nChildIcon));

	//success
	return TRUE;
}



//clears the LTA icon list
void CLTATreeMgr::ClearLTAIcons()
{
	ResizeLTAIcons(0, FALSE);
}

//gets the number of icons in the list
DWORD CLTATreeMgr::GetNumLTAIcons() const
{
	return m_nNumLTAIcons;
}

//resizes the LTA Icon array, keeping as much data as possible
//if bKeepData is true. If nNumItems is 0, it will free the list
BOOL CLTATreeMgr::ResizeLTAIcons(DWORD nNumItems, BOOL bKeepData)
{
	//see if it is just a free
	if(nNumItems == 0)
	{
		FreeLTAIcons();
		return TRUE;
	}

	//allocate the new list
	CLTAIcon* pNewList = new CLTAIcon[nNumItems];

	//verify
	if(pNewList == NULL)
	{
		return FALSE;
	}

	//copy over data if needed
	if(m_pLTAIconList && bKeepData)
	{
		for(DWORD nCurrIcon = 0; nCurrIcon < LTMIN(nNumItems, GetNumLTAIcons()); nCurrIcon++)
		{
			pNewList[nCurrIcon] = m_pLTAIconList[nCurrIcon];
		}
	}

	//delete the old data
	FreeLTAIcons();

	//setup the new pointers
	m_pLTAIconList = pNewList;
	m_nNumLTAIcons = nNumItems;

	return TRUE;
}

//frees the list of LTA icons associated with this manager
void CLTATreeMgr::FreeLTAIcons()
{
	delete [] m_pLTAIconList;
	m_pLTAIconList = NULL;

	m_nNumLTAIcons = 0;
}

BOOL CLTATreeMgr::RecurseBuildTree(	CLTANode* pCurrNode, 
									CLTWinTreeKey Parent,
									CLTWinTreeIcon DefIcon)
{
	//make sure the node passed in is correct
	if((pCurrNode == NULL) || ((pCurrNode->IsList() && pCurrNode->GetNumElements() == 0)))
	{
		return FALSE;
	}


	//number of children in the list (cache it)
	int nNumChildren = pCurrNode->GetNumElements();
	//looping var through child list
	int nCurrChild;

	//the text to put on the item
	CString sItemText;

	//the keyword (first word in the text)
	CString sKeyword;

	//build up the name of the item
	for(nCurrChild = 0; nCurrChild < nNumChildren; nCurrChild++)
	{
		//cache the child
		CLTANode* pCurrChild = pCurrNode->GetElement(nCurrChild);

		//make sure it is valid
		if(pCurrChild && pCurrChild->IsAtom())
		{
			if(sItemText.IsEmpty())
			{
				//put the first word in keyword, and don't use quotes
				sKeyword = pCurrChild->GetValue();
				sItemText += pCurrChild->GetValue();
				sItemText += " ";
			}
			else
			{
				//put quotes around the value otherwise
				if(pCurrChild->IsString())
					sItemText += '\"';

				sItemText += pCurrChild->GetValue();

				if(pCurrChild->IsString())
					sItemText += '\"';

				sItemText += ' ';
			}			
		}
	}

	//create the item if there is text to associate with it
	CLTWinTreeKey NewKey = Parent;
	
	//get the default icon if appropriate
	CLTWinTreeIcon NewDefaultIcon = DefIcon;

	//if the item text is emtpy, we didn't find any valid items
	if(!sItemText.IsEmpty())
	{
		//find the LTA icon for this keyword
		sKeyword.MakeUpper();
		CLTAIcon* pLTAIcon = GetLTAIcon(sKeyword);

		//get the appropriate icon for the item
		CLTWinTreeIcon CurrIcon = DefIcon;

		//if we found the icon, we need to set it
		if(pLTAIcon)
		{
			CurrIcon		= pLTAIcon->m_Icon;
			NewDefaultIcon	= pLTAIcon->m_ChildDefIcon;
		}
	
		NewKey = AddItem(new CLTWinTreeItem(sItemText, CurrIcon), Parent, FALSE);

		if(NewKey == NULLITEM)
		{
			return FALSE;
		}
	}

	//recurse on all children that are lists
	for(nCurrChild = 0; nCurrChild < nNumChildren; nCurrChild++)
	{
		CLTANode* pCurrChild = pCurrNode->GetElement(nCurrChild);
	
		if(pCurrChild->IsList() && (pCurrChild->GetNumElements() > 0))
		{
			RecurseBuildTree(pCurrChild, NewKey, NewDefaultIcon);
		}
	}


	return TRUE;
}

//gets an LTA icon associated with the specified keyword. Will return
//NULL if it cannot find one. NOTE: pszKeyword must be uppercase,
//since all LTAIcons have the names capitalized for speed in finding
CLTAIcon* CLTATreeMgr::GetLTAIcon(const char* pszKeyword)
{
	//go through all icons
	for(uint32 nCurrIcon = 0; nCurrIcon < GetNumLTAIcons(); nCurrIcon++)
	{
		if(strcmp(pszKeyword, m_pLTAIconList[nCurrIcon].m_sKeyword) == 0)
		{
			return &m_pLTAIconList[nCurrIcon];
		}
	}

	//didn't find it
	return NULL;
}
