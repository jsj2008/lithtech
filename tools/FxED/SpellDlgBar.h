#if !defined(AFX_SPELLDLGBAR_H__A74036B9_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLDLGBAR_H__A74036B9_6F17_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SpellDlgBar.h : header file
//

// Includes....

#include "SpellCtrl.h"
#include "SpellTabCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellDlgBar dialog

class CSpellDlgBar : public CDialogBar
{
// Construction
public:
	CSpellDlgBar(CWnd* pParent = NULL);   // standard constructor


	public :

		// Member Functions

		CSpellTabCtrl*				GetTabCtrl() { return &m_tabCtrl; }
		CSpellCtrl*					GetSpellCtrl() { return m_pSpellCtrl; }

	private :

		// Member Variables

		CSpellTabCtrl			    m_tabCtrl;
		CSpellCtrl				   *m_pSpellCtrl;










// Dialog Data
	//{{AFX_DATA(CSpellDlgBar)
	enum { IDD = IDD_DLGBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellDlgBar)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellDlgBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLDLGBAR_H__A74036B9_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
