#if !defined(AFX_GAMEINFODLG_H__BF17E2A4_4919_11D3_9B55_0060971BDAD8__INCLUDED_)
#define AFX_GAMEINFODLG_H__BF17E2A4_4919_11D3_9B55_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GameInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGameInfoDlg dialog

class CGameInfoDlg : public CDialog
{
// Construction
public:
	CGameInfoDlg(CString sDll, CString sRez, CString sApp, CString sCmdLine, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGameInfoDlg)
	enum { IDD = IDC_GAMEINFO };
	CString	m_sApp;
	CString	m_sRez;
	CString	m_sCmdLine;
	CString	m_sDll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGameInfoDlg)
	afx_msg void OnBrowseAppPath();
	afx_msg void OnBrowseProjectPath();
    afx_msg void OnBrowseDllPath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMEINFODLG_H__BF17E2A4_4919_11D3_9B55_0060971BDAD8__INCLUDED_)
