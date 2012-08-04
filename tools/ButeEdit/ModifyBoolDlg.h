#if !defined(AFX_MODIFYBOOLDLG_H__0F292662_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYBOOLDLG_H__0F292662_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyBoolDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyBoolDlg dialog

class CModifyBoolDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyBoolDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyBoolDlg)
	enum { IDD = IDD_DIALOG_BOOL };
	int		m_nBoolValue;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyBoolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyBoolDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Call this to set the initial state of the dialog control
	void	SetValue(bool bValue);

	// Call this to get the value of the control
	bool	GetValue();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYBOOLDLG_H__0F292662_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
