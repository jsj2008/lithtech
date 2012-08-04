#if !defined(AFX_NEWTAGDLG_H__A181D764_19FA_11D3_BE2D_0060971BDC6D__INCLUDED_)
#define AFX_NEWTAGDLG_H__A181D764_19FA_11D3_BE2D_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewTagDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewTagDlg dialog

class CNewTagDlg : public CDialog
{
// Construction
public:
	CNewTagDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewTagDlg)
	enum { IDD = IDD_DIALOG_NEW_TAG };
	CString	m_sName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewTagDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewTagDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWTAGDLG_H__A181D764_19FA_11D3_BE2D_0060971BDC6D__INCLUDED_)
