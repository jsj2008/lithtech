#if !defined(AFX_STRINGDLG_H__E0AF2141_701A_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_STRINGDLG_H__E0AF2141_701A_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StringDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStringDlg dialog

class CStringDlg : public CDialog
{
// Construction
public:
	CStringDlg(CString sName, BOOL bNullStringOkay = FALSE, CWnd* pParent = NULL);   // standard constructor

	public :

		// Member Variables

		CString					m_sName;
		BOOL					m_bNullStringOkay;

// Dialog Data
	//{{AFX_DATA(CStringDlg)
	enum { IDD = IDD_STRINGDLG };
	CString	m_sText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGDLG_H__E0AF2141_701A_11D2_9B4A_0060971BDAD8__INCLUDED_)
