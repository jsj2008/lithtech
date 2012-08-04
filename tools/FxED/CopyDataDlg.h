#if !defined(AFX_COPYDATADLG_H__7281F121_A59E_11D2_9B4D_0060971BDAD8__INCLUDED_)
#define AFX_COPYDATADLG_H__7281F121_A59E_11D2_9B4D_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyDataDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCopyDataDlg dialog

class CCopyDataDlg : public CDialog
{
// Construction
public:
	CCopyDataDlg(BOOL bColour, BOOL bMotion, BOOL bScale, CWnd* pParent = NULL);   // standard constructor

	BOOL					m_bEnableColour;
	BOOL					m_bEnableScale;
	BOOL					m_bEnableMotion;

	static BOOL				s_bCopyColour;
	static BOOL				s_bCopyMotion;
	static BOOL				s_bCopyScale;

// Dialog Data
	//{{AFX_DATA(CCopyDataDlg)
	enum { IDD = IDD_COPYDATA };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyDataDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCopyDataDlg)
	afx_msg void OnColourkeys();
	afx_msg void OnMotionkeys();
	afx_msg void OnScalekeys();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYDATADLG_H__7281F121_A59E_11D2_9B4D_0060971BDAD8__INCLUDED_)
