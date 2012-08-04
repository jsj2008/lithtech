#if !defined(AFX_FXDLG_H__99D9CF27_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_FXDLG_H__99D9CF27_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FxDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFxDlg dialog

class CFxDlg : public CDialog
{
// Construction
public:
	CFxDlg(CWnd* pParent = NULL);   // standard constructor

	public :

		// Member Variables

		FX_REF					   *m_pFxRef;














// Dialog Data
	//{{AFX_DATA(CFxDlg)
	enum { IDD = IDD_FX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFxDlg)
	afx_msg void OnDblclkFxlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FXDLG_H__99D9CF27_7A26_11D2_9B4A_0060971BDAD8__INCLUDED_)
