//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_NEWPROJECTDLG_H__E5F3B9C3_6AD7_11D1_A7BD_006097726515__INCLUDED_)
#define AFX_NEWPROJECTDLG_H__E5F3B9C3_6AD7_11D1_A7BD_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NewProjectDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDlg dialog

class CNewProjectDlg : public CDialog
{
// Construction
public:
	CNewProjectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewProjectDlg)
	enum { IDD = IDD_NEWPROJECT };
	CString	m_ProjectDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewProjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewProjectDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPROJECTDLG_H__E5F3B9C3_6AD7_11D1_A7BD_006097726515__INCLUDED_)
