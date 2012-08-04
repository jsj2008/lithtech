#if !defined(AFX_MODIFYRECTANGLEDLG_H__0F29266A_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYRECTANGLEDLG_H__0F29266A_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyRectangleDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyRectangleDlg dialog

class CModifyRectangleDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyRectangleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyRectangleDlg)
	enum { IDD = IDD_DIALOG_RECTANGLE };
	CString	m_sName;
	CString	m_sRectangle;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyRectangleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyRectangleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditRectangle();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Get/Set the rectangle
	CRect	GetRect()			{ return m_rcRect; }
	void	SetRect(CRect rc)	{ m_rcRect=rc; }

protected:
	CRect	m_rcRect;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYRECTANGLEDLG_H__0F29266A_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
