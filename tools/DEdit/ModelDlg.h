//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_MODELDLG_H__8AA8DF22_55B9_11D1_A419_006097098780__INCLUDED_)
#define AFX_MODELDLG_H__8AA8DF22_55B9_11D1_A419_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ModelDlg.h : header file
//

#include "resourcemgr.h"
#include "baserezdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CModelDlg dialog

class CModelDlg : public CBaseRezDlg
{
// Construction
public:
	CModelDlg();   // standard constructor
	virtual ~CModelDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(CModelDlg)
	enum { IDD = IDD_MODEL_TABDLG };
	CTreeCtrl	m_ModelTree;
	CListCtrl	m_ModelList;
	//}}AFX_DATA

	CImageList	*m_pIconList;
	CImageList	*m_pIconList2;

	BOOL		DoImportOperation();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialogBar();

	// Generated message map functions
	//{{AFX_MSG(CModelDlg)	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkModel( NMHDR * pNMHDR, LRESULT * pResult );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELDLG_H__8AA8DF22_55B9_11D1_A419_006097098780__INCLUDED_)
