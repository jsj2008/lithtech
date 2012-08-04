#if !defined(AFX_MODIFYDWORDDLG_H__0F292665_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYDWORDDLG_H__0F292665_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyDWordDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyDWordDlg dialog

class CModifyDWordDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyDWordDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyDWordDlg)
	enum { IDD = IDD_DIALOG_DWORD };
	DWORD	m_dwValue;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyDWordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyDWordDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYDWORDDLG_H__0F292665_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
