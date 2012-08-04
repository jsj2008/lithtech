#if !defined(AFX_MODIFYRANGEDLG_H__0F292669_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYRANGEDLG_H__0F292669_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyRangeDlg.h : header file
//

#include "ButeMgr.h"
#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyRangeDlg dialog
class CModifyRangeDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyRangeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyRangeDlg)
	enum { IDD = IDD_DIALOG_RANGE };
	CString	m_sName;
	CString	m_sRange;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyRangeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyRangeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditRange();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Get/Set the range
	CARange		GetRange()					{ return m_range; }
	void		SetRange(CARange range)		{ m_range=range; }

protected:
	CARange		m_range;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYRANGEDLG_H__0F292669_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
