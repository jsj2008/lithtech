#if !defined(AFX_ANIMNUMBERDLG_H__47600101_CE53_11D1_A7F5_006097726515__INCLUDED_)
#define AFX_ANIMNUMBERDLG_H__47600101_CE53_11D1_A7F5_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnimNumberDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AnimNumberDlg dialog

class AnimNumberDlg : public CDialog
{
// Construction
public:
	AnimNumberDlg(CWnd* pParent, int maxNumber);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AnimNumberDlg)
	enum { IDD = IDD_ANIMNUMBER };
	int		m_MoveNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AnimNumberDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	int m_MaxNumber;

	// Generated message map functions
	//{{AFX_MSG(AnimNumberDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMNUMBERDLG_H__47600101_CE53_11D1_A7F5_006097726515__INCLUDED_)
