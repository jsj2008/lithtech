#if !defined(AFX_INTDLG_H__B1D95DA5_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
#define AFX_INTDLG_H__B1D95DA5_9058_11D2_9B4B_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntDlg dialog

class CIntDlg : public CDialog
{
// Construction
public:
	CIntDlg(int iInitial, CString sName, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIntDlg)
	enum { IDD = IDD_INTEGERDLG };
	int		m_int;
	//}}AFX_DATA

	CString							m_sName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIntDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTDLG_H__B1D95DA5_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
