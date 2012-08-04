#if !defined(AFX_MODIFYFLOATDLG_H__0F292666_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYFLOATDLG_H__0F292666_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyFloatDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyFloatDlg dialog

class CModifyFloatDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyFloatDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyFloatDlg)
	enum { IDD = IDD_DIALOG_FLOAT };
	CString	m_sName;
	float	m_fValue;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyFloatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyFloatDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYFLOATDLG_H__0F292666_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
