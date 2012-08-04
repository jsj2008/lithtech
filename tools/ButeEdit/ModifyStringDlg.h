#if !defined(AFX_MODIFYSTRINGDLG_H__0F29266C_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYSTRINGDLG_H__0F29266C_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyStringDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyStringDlg dialog

class CModifyStringDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyStringDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyStringDlg)
	enum { IDD = IDD_DIALOG_STRING };
	CString	m_sValue;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyStringDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyStringDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYSTRINGDLG_H__0F29266C_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
