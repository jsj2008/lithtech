#if !defined(__ROTATIONDLG_H)
#define __ROTATIONDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RotationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RotationDlg dialog

class RotationDlg : public CDialog
{
// Construction
public:
	RotationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RotationDlg)
	enum { IDD = IDD_ROTATIONDLG };
	CString	m_sRotX;
	CString	m_sRotY;
	CString	m_sRotZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RotationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RotationDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined __ROTATIONDLG_H
