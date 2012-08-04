#if !defined(AFX_COMMANDSTRINGDLG_H__4431AA23_482A_11D1_B4AC_00A024805738__INCLUDED_)
#define AFX_COMMANDSTRINGDLG_H__4431AA23_482A_11D1_B4AC_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CommandStringDlg.h : header file
//

#include "model.h"


/////////////////////////////////////////////////////////////////////////////
// CCommandStringDlg dialog

class CCommandStringDlg : public CDialog
{
// Construction
public:
	CCommandStringDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCommandStringDlg)
	enum { IDD = IDD_COMMANDSTRINGEDIT };
	CEdit	m_CommandString;
	//}}AFX_DATA


	// Set this before running the dialog.
	Model *m_pModel;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommandStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommandStringDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDSTRINGDLG_H__4431AA23_482A_11D1_B4AC_00A024805738__INCLUDED_)
