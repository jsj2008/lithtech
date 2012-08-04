#if !defined(AFX_KEYFRAMESTRINGDLG_H__4B4E13B2_49D2_11D1_B4AC_00A024805738__INCLUDED_)
#define AFX_KEYFRAMESTRINGDLG_H__4B4E13B2_49D2_11D1_B4AC_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyframeStringDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyframeStringDlg dialog

class CKeyframeStringDlg : public CDialog
{
// Construction
public:
	CKeyframeStringDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyframeStringDlg)
	enum { IDD = IDD_KEYFRAMESTRING };
	CString	m_KeyframeString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyframeStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyframeStringDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYFRAMESTRINGDLG_H__4B4E13B2_49D2_11D1_B4AC_00A024805738__INCLUDED_)
