#if !defined(AFX_MODELINFODLG_H__49102B72_4764_11D1_B4AC_00A024805738__INCLUDED_)
#define AFX_MODELINFODLG_H__49102B72_4764_11D1_B4AC_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ModelInfoDlg.h : header file
//


#include "model.h"


/////////////////////////////////////////////////////////////////////////////
// CModelInfoDlg dialog

class CModelInfoDlg : public CDialog
{
// Construction
public:
	CModelInfoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModelInfoDlg)
	enum { IDD = IDD_MODELINFO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	Model	*m_pModel;
	void	DrawToTextThing(char *pStr, ...);
	DWORD	ShowPieceMem(DWORD iLOD);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModelInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModelInfoDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELINFODLG_H__49102B72_4764_11D1_B4AC_00A024805738__INCLUDED_)
