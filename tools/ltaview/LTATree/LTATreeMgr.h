#ifndef __LTATREEMGR_H__
#define __LTATREEMGR_H__

#ifndef __LTWINTREEMGR_H__
#	include "LTWinTreeMgr.h"
#endif

#ifndef __LTAICON_H__
#	include "LTAIcon.h"
#endif

//forward declare the parse node class, since callees need not
//worry about the actual definition
class CLTANode;

class CLTATreeMgr : public CLTWinTreeMgr
{
public:

	CLTATreeMgr();
	~CLTATreeMgr();

	//loads an LTA file, and adds it to the tree from the specified node,
	//or from the root if the key is invalid. Each created item will have an
	//icon associated to it from the list specified in AddLTAIcons
	BOOL	LoadLTA(const char* pszFileName, CLTWinTreeKey Parent = NULLITEM);

	//adds an icon to the list, and associates it with a keyword. The first
	//icon is the one that will be associated with the item itself, the other
	//is the default icon for its children, both are windows icon IDs.
	BOOL	AddLTAIcon(const char* pszKeyword, DWORD nIcon, DWORD nChildIcon);

	//clears the LTA icon list
	void	ClearLTAIcons();

	//gets the number of icons in the list
	DWORD	GetNumLTAIcons() const;

private:

	//gets an LTA icon associated with the specified keyword. Will return
	//NULL if it cannot find one. NOTE: pszKeyword must be uppercase,
	//since all LTAIcons have the names capitalized for speed in finding
	CLTAIcon*	GetLTAIcon(const char* pszKeyword);

	//resizes the LTA Icon array, keeping as much data as possible
	//if bKeepData is true. If nNumItems is 0, it will free the list
	BOOL	ResizeLTAIcons(DWORD nNumItems, BOOL bKeepData);

	//frees the list of LTA icons associated with this manager
	void	FreeLTAIcons();

	//recursively builds up the tree from a CParseNode class, which the
	//LTA files are loaded into
	BOOL	RecurseBuildTree(	CLTANode* pCurrNode, 
								CLTWinTreeKey Parent,
								CLTWinTreeIcon DefIcon);

	//number of LTA icons in the list
	DWORD			m_nNumLTAIcons;

	//the LTA icon list
	CLTAIcon*		m_pLTAIconList;

};

#endif
