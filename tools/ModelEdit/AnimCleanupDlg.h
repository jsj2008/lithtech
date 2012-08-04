#if !defined(AFX_ANIMCLEANUPDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_)
#define AFX_ANIMCLEANUPDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AnimCleanupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimCleanupDlg dialog

class CAnimCleanupDlg : public CDialog
{
// Construction
public:
	CAnimCleanupDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimCleanupDlg)
	enum { IDD = IDD_ANIMATIONCLEANUP };
	BOOL	m_bPreserveRot;
	CString	m_VarianceString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimCleanupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimCleanupDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMCLEANUPDLG_H__298F2841_9438_11D1_A7D8_006097726515__INCLUDED_)
