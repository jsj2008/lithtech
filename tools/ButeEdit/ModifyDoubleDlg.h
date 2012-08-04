#if !defined(AFX_MODIFYDOUBLEDLG_H__0F292664_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYDOUBLEDLG_H__0F292664_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyDoubleDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyDoubleDlg dialog

class CModifyDoubleDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyDoubleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyDoubleDlg)
	enum { IDD = IDD_DIALOG_DOUBLE };
	double	m_fValue;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyDoubleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyDoubleDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYDOUBLEDLG_H__0F292664_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
