#if !defined(AFX_REZDLG_H__234A03E1_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_REZDLG_H__234A03E1_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RezDlg.h : header file
//

// Includes....

#include "rezmgr.h"

/////////////////////////////////////////////////////////////////////////////
// CRezDlg dialog

class CRezDlg : public CDialog
{
// Construction
public:
	CRezDlg(CString sExt, CString sInitialChoice, CWnd* pParent = NULL);   // standard constructor

	public :
	
		// Member Functions

		void						ReadDirectory(HTREEITEM hItem, CRezDir *pDir);

	public :

		CString						m_sPath;

	private :

		// Member Variables

		CString						m_sExt;
		CString						m_sInitialChoice;














// Dialog Data
	//{{AFX_DATA(CRezDlg)
	enum { IDD = IDD_REZ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRezDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRezDlg)
	virtual void OnOK();
	afx_msg void OnDblclkRez(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnNone();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REZDLG_H__234A03E1_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_)
