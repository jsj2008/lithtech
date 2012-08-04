//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_WORLDSDLG_H__D7EBDD92_4653_11D1_A408_006097098780__INCLUDED_)
#define AFX_WORLDSDLG_H__D7EBDD92_4653_11D1_A408_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WorldsDlg.h : header file
//

#include "resourcemgr.h"
#include "baserezdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CWorldsDlg dialog

class CWorldsDlg : public CBaseRezDlg
{
// Construction
public:
	CWorldsDlg();   // standard constructor
	virtual ~CWorldsDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(CWorldsDlg)
	enum { IDD = IDD_WORLDS_TABDLG };
	CTreeCtrl	m_WorldTree;
	CListCtrl	m_WorldList;
	//}}AFX_DATA

	CImageList	*m_pIconList;
	CImageList	*m_pIconList2;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//gets the filename of the currently selected list item
	CString GetSelectedItemFileName();


	BOOL OnInitDialogBar();

	// Generated message map functions
	//{{AFX_MSG(CWorldsDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkWorldsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFileNewWorld();
	afx_msg void OnWorldListOpen();
	afx_msg void OnWorldDelete();
	afx_msg void OnWorldCompress();
	afx_msg void OnWorldDecompress();
	afx_msg void OnWorldRun();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDSDLG_H__D7EBDD92_4653_11D1_A408_006097098780__INCLUDED_)
