//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_RESNEWDIR_H__C2074344_CD09_11D0_99E3_0060970987C3__INCLUDED_)
#define AFX_RESNEWDIR_H__C2074344_CD09_11D0_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ResNewDir.h : header file
//
#include "resourcemgr.h"

/////////////////////////////////////////////////////////////////////////////
// CResNewDir dialog

class CResNewDir : public CDialog
{
// Construction
public:
	CResNewDir(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResNewDir)
	enum { IDD = IDD_NEWDIR };
	CComboBox	m_cbDirType;
	CString	m_szNewDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResNewDir)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:

protected:

	CStringHolder		m_StringHolder;
	resource_type		m_cDirType;

	// Generated message map functions
	//{{AFX_MSG(CResNewDir)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESNEWDIR_H__C2074344_CD09_11D0_99E3_0060970987C3__INCLUDED_)
