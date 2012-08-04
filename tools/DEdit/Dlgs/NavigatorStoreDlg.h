//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_NAVIGATORSTOREDLG_H__625EE621_C83A_11D2_BDF3_0060971BDC6D__INCLUDED_)
#define AFX_NAVIGATORSTOREDLG_H__625EE621_C83A_11D2_BDF3_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NavigatorStoreDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNavigatorStoreDlg dialog

class CNavigatorStoreDlg : public CDialog
{
// Construction
public:
	CNavigatorStoreDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNavigatorStoreDlg)
	enum { IDD = IDD_NAVIGATORSTOREDLG };
	CString	m_sName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNavigatorStoreDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNavigatorStoreDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NAVIGATORSTOREDLG_H__625EE621_C83A_11D2_BDF3_0060971BDC6D__INCLUDED_)
