//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_CLASSLISTDLG_H__DB123AA5_77FA_11D2_BFA3_00A0C9696F4D__INCLUDED_)
#define AFX_CLASSLISTDLG_H__DB123AA5_77FA_11D2_BFA3_00A0C9696F4D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ClassListDlg.h : header file
//

#include "mrcext.h"
#include "projecttabcontrolbar.h"

/////////////////////////////////////////////////////////////////////////////
// ClassListDlg dialog

class ClassListDlg : public CProjectTabControlBar
{
// Construction
public:
	ClassListDlg();   // standard constructor
	~ClassListDlg();	// destructor

// Dialog Data
	//{{AFX_DATA(ClassListDlg)
	enum { IDD = IDD_CLASSES_TABDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CListCtrl	m_ClassList;
	CImageList	*m_pImageList;

	// Updates the contents.
	void		Update();

	//clears out the list of classes
	void		ClearAll();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ClassListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL	OnInitDialogBar();

	// Repositions the controls
	void	RepositionControls();

	// Generated message map functions
	//{{AFX_MSG(ClassListDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblClickClass( NMHDR * pNMHDR, LRESULT * pResult );
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASSLISTDLG_H__DB123AA5_77FA_11D2_BFA3_00A0C9696F4D__INCLUDED_)
