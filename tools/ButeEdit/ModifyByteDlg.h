#if !defined(AFX_MODIFYBYTEDLG_H__0F292663_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYBYTEDLG_H__0F292663_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyByteDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyByteDlg dialog

class CModifyByteDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyByteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyByteDlg)
	enum { IDD = IDD_DIALOG_BYTE };
	BYTE	m_nValue;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyByteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyByteDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYBYTEDLG_H__0F292663_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
