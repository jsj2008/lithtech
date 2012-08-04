//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_PALETTEEDIT_H__D8070F61_663B_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_PALETTEEDIT_H__D8070F61_663B_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PaletteEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPaletteEdit dialog

class CPaletteEdit : public CDialog
{
// Construction
public:
	CPaletteEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPaletteEdit)
	enum { IDD = IDD_PALETTE };
	int		m_nPaletteIndex;
	int		m_nPaletteNumberIndex;
	CString	m_sSelectedColor;
	CListBox m_lbColors;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPaletteEdit)
	afx_msg void OnPaletteAdd();
	afx_msg void OnPaletteRemove();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALETTEEDIT_H__D8070F61_663B_11D1_99E4_0060970987C3__INCLUDED_)
