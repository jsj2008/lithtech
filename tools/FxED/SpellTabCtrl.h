#if !defined(AFX_SPELLTABCTRL_H__0EFD8084_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLTABCTRL_H__0EFD8084_6F5A_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SpellTabCtrl.h : header file
//

// Includes....

#include "SpellCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellTabCtrl window

class CSpellTabCtrl : public CTabCtrl
{
// Construction
public:
	CSpellTabCtrl();


	public :

		// Member Functions

		// Accessors

		CSpellCtrl*						GetSpellCtrl() { return &m_spellCtrl; }

	private :

		// Member Variables

		CSpellCtrl					     m_spellCtrl;







// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellTabCtrl)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpellTabCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpellTabCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLTABCTRL_H__0EFD8084_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
