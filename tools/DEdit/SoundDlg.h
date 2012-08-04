//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_SOUNDDLG_H__8AA8DF23_55B9_11D1_A419_006097098780__INCLUDED_)
#define AFX_SOUNDDLG_H__8AA8DF23_55B9_11D1_A419_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SoundDlg.h : header file
//

#include "resourcemgr.h"
#include "baserezdlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

class CSoundDlg : public CBaseRezDlg
{
// Construction
public:
	CSoundDlg();   // standard constructor
	virtual ~CSoundDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(CSoundDlg)
	enum { IDD = IDD_SOUND_TABDLG };
	CTreeCtrl	m_SoundTree;
	CListCtrl	m_SoundList;
	//}}AFX_DATA

	CImageList	*m_pIconList;
	CImageList	*m_pIconList2;


	BOOL DoImportOperation();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialogBar();

	// Generated message map functions
	//{{AFX_MSG(CSoundDlg)	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedDirectory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClickSound( NMHDR * pNMHDR, LRESULT * pResult );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDDLG_H__8AA8DF23_55B9_11D1_A419_006097098780__INCLUDED_)
