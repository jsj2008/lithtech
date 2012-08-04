#if !defined(AFX_CONTINUOUSDLG_H__1B1481C2_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_)
#define AFX_CONTINUOUSDLG_H__1B1481C2_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ContinuousDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContinuousDlg dialog

class CContinuousDlg : public CDialog
{
// Construction
public:
	CContinuousDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CContinuousDlg)
	enum { IDD = IDD_CONTINUOUS_DIALOG };
	DWORD	m_nDelay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContinuousDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CContinuousDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTINUOUSDLG_H__1B1481C2_2F46_11D1_9462_0020AFF7CDC1__INCLUDED_)
