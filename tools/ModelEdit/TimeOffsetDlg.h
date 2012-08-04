#if !defined(AFX_TIMEOFFSETDLG_H__CD6D49B1_9246_11D3_99C8_00A0C9696F4D__INCLUDED_)
#define AFX_TIMEOFFSETDLG_H__CD6D49B1_9246_11D3_99C8_00A0C9696F4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TimeOffsetDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TimeOffsetDlg dialog

class TimeOffsetDlg : public CDialog
{
// Construction
public:
	TimeOffsetDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TimeOffsetDlg)
	enum { IDD = IDD_TIMEOFFSET };
	UINT	m_TimeOffset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TimeOffsetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TimeOffsetDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMEOFFSETDLG_H__CD6D49B1_9246_11D3_99C8_00A0C9696F4D__INCLUDED_)
