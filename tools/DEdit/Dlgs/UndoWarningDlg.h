//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_UNDOWARNINGDLG_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_)
#define AFX_UNDOWARNINGDLG_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RotationEdit.h : header file
//


#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CUndoWarning dialog

class CUndoWarningDlg : public CDialog
{
// Construction
public:
	CUndoWarningDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUndoWarning)
	enum { IDD = IDD_UNDOWARNINGDLG };
	BOOL	m_bShowWarningDialog;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUndoWarning)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

	uint32 m_nWorldNodeCount;

	void SaveShowWarningDialog();


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUndoWarning)
	virtual BOOL OnInitDialog();
	afx_msg void OnYes();
	afx_msg void OnAlways();
	afx_msg void OnNo();
	afx_msg void OnNever();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNDOWARNINGDLG_H__5999A6E2_4297_11D1_B4A7_00A024805738__INCLUDED_)
