//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_MULTILINESTRINGDLG_H__291465E1_81C8_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_MULTILINESTRINGDLG_H__291465E1_81C8_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MultiLineStringDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiLineStringDlg dialog

class CMultiLineStringDlg : public CDialog
{
// Construction
public:
	BOOL m_bReadOnly;
	BOOL m_bLimitText;
	uint32 m_nTextLimit;
	CMultiLineStringDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiLineStringDlg)
	enum { IDD = IDD_MULTILINEEDIT };
	CString	m_String;
	CString m_Caption;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiLineStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiLineStringDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTILINESTRINGDLG_H__291465E1_81C8_11D1_99E4_0060970987C3__INCLUDED_)
