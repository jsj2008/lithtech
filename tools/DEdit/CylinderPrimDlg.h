//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_CYLINDERPRIMDLG_H__78716A41_6B41_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_CYLINDERPRIMDLG_H__78716A41_6B41_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CylinderPrimDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCylinderPrimDlg dialog

class CCylinderPrimDlg : public CDialog
{
// Construction
public:
	CCylinderPrimDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCylinderPrimDlg)
	enum { IDD = IDD_PRIMCYLINDER };
	UINT	m_nHeight;
	UINT	m_nNumSides;
	UINT	m_nRadius;
	//}}AFX_DATA

	CString	m_sCaption;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCylinderPrimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCylinderPrimDlg)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CYLINDERPRIMDLG_H__78716A41_6B41_11D1_99E4_0060970987C3__INCLUDED_)
