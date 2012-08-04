//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_DEBUGDLG_H__847BFC81_6525_11D1_A7BD_006097726515__INCLUDED_)
#define AFX_DEBUGDLG_H__847BFC81_6525_11D1_A7BD_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DebugDlg.h : header file
//

#include <stdarg.h>
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg dialog

class CDebugDlg : public CDialog
{
// Construction
public:
	CDebugDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDebugDlg)
	enum { IDD = IDD_DEBUGDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void AddMessage(const char *pStr, ...);
	void AddMessage2(const char *pStr, va_list marker);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CDebugDlg)
	afx_msg void OnHide();
	afx_msg void Clear();
	afx_msg void OnRunCommand();
	afx_msg void OnUpdateProperties();
	//}}AFX_MSG

protected:

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGDLG_H__847BFC81_6525_11D1_A7BD_006097726515__INCLUDED_)
