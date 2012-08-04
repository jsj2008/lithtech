#if !defined(AFX_SPELLCTRL_H__0EFD8081_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLCTRL_H__0EFD8081_6F5A_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SpellCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpellCtrl window

// Includes....

#include "Spell.h"

class CSpellCtrl : public CTreeCtrl
{
// Construction
public:
	CSpellCtrl();


	public :

		// Member Functions

		BOOL						AddGroup(HTREEITEM hItem, char *sName);
		BOOL						AddSpellToGroup(HTREEITEM hItem, CSpell *pSpell) { return TRUE; }
		BOOL						RemoveSpellFromGroup(CSpell *pSpell) { return TRUE; }

		BOOL						FullUpdate(HTREEITEM hItem);


		void						InsertSpell(HTREEITEM hItem, CSpell *pSpell)
									{
										TV_INSERTSTRUCT tvItem;

										tvItem.hParent			   = hItem;
										tvItem.hInsertAfter		   = TVI_LAST;
										tvItem.item.mask		   = TVIF_TEXT | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
										tvItem.item.pszText		   = (char *)(LPCSTR)pSpell->GetName();
										tvItem.item.cchTextMax	   = strlen(tvItem.item.pszText);
										tvItem.item.iImage		   = 2;
										tvItem.item.iSelectedImage = 2;
										tvItem.item.lParam		   = (LPARAM)pSpell;

										InsertItem(&tvItem);
									}

		BOOL						IsSpell(HTREEITEM hItem)
									{
										int iImage, iSelectedImage;
										GetItemImage(hItem, iImage, iSelectedImage);

										if (iImage == IM_SPELL) return TRUE;

										return FALSE;
									}

		void						FullSortTree(HTREEITEM hItem);

		// Accessors

	private :

		// Member Variables
	
		CSpell					   *m_pCopySpell;










// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpellCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpellCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnAddSpell();
	afx_msg void OnDeleteSpell();
	afx_msg void OnDeleteSpellFromGroup();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnAddSpellsByReference();
	afx_msg void OnRenameSpell();
	afx_msg void OnRenameGroup();
	afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditSpell();
	afx_msg void OnGroupMakeCopy();
	afx_msg void OnImportSpell();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLCTRL_H__0EFD8081_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
