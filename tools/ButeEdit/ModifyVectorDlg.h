#if !defined(AFX_MODIFYVECTORDLG_H__0F29266E_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYVECTORDLG_H__0F29266E_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyVectorDlg.h : header file
//

#include "ButeMgr.h"
#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyVectorDlg dialog

class CModifyVectorDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyVectorDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyVectorDlg)
	enum { IDD = IDD_DIALOG_VECTOR };
	CString	m_sVector;
	CString	m_sName;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyVectorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyVectorDlg)
	afx_msg void OnChangeEditVector();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Call this before DoModal is called
	void		SetVector(CAVector v)	{ m_vVector=v; }

	// Call this after DoModal
	CAVector	GetVector()				{ return m_vVector; }

protected:
	// The current vector
	CAVector	m_vVector;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYVECTORDLG_H__0F29266E_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
