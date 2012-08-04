#if !defined(AFX_RENAMENODEDLG_H__B2AF6751_CFE9_11D1_A7F5_006097726515__INCLUDED_)
#define AFX_RENAMENODEDLG_H__B2AF6751_CFE9_11D1_A7F5_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RenameNodeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RenameNodeDlg dialog

class RenameNodeDlg : public CDialog
{
// Construction
public:
	RenameNodeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RenameNodeDlg)
	enum { IDD = IDD_RENAMENODE };
	CString	m_NewName;
	CString	m_OldName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RenameNodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RenameNodeDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMENODEDLG_H__B2AF6751_CFE9_11D1_A7F5_006097726515__INCLUDED_)
