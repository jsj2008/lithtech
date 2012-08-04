#if !defined(AFX_GENLODDLG_H__4431AA22_482A_11D1_B4AC_00A024805738__INCLUDED_)
#define AFX_GENLODDLG_H__4431AA22_482A_11D1_B4AC_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GenLodDlg.h : header file
//

#include "newgenlod.h"

/////////////////////////////////////////////////////////////////////////////
// CGenLodDlg dialog

class CGenLodDlg : public CDialog
{
// Construction
public:
	CGenLodDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGenLodDlg)
	enum { IDD = IDD_GENLOD };
	CListBox	m_PieceList;
	CEdit	m_MinimumNodePolies;
	CEdit	m_MaxEdgeLength;
	CListBox	m_LODs;
	float	m_PieceWeight;
	//}}AFX_DATA

	// If they click Ok, then these will be set.
	// The LODs should be initialized before starting the dialog.
	BuildLODRequest	*m_pRequest;


	void		FillLODList();
	void		SortLODs();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenLodDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenLodDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEditLOD();
	afx_msg void OnRemoveLOD();
	afx_msg void OnAddLOD();
	afx_msg void OnSelchangePiecelist();
	afx_msg void OnUpdatePieceweight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENLODDLG_H__4431AA22_482A_11D1_B4AC_00A024805738__INCLUDED_)
