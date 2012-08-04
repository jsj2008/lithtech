#if !defined(AFX_MODIFYPOINTDLG_H__0F292668_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_MODIFYPOINTDLG_H__0F292668_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyPointDlg.h : header file
//

#include "ModifyDlgBase.h"

/////////////////////////////////////////////////////////////////////////////
// CModifyPointDlg dialog

class CModifyPointDlg : public CDialog, public CModifyDlgBase
{
// Construction
public:
	CModifyPointDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CModifyPointDlg)
	enum { IDD = IDD_DIALOG_POINT };
	CString	m_sName;
	CString	m_sPoint;
	BOOL	m_bReplaceAll;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModifyPointDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModifyPointDlg)
	afx_msg void OnChangeEditPoint();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Set/Get the point value
	void		SetPoint(CPoint point)	{ m_point=point; }
	CPoint		GetPoint()				{ return m_point; }

protected:
	CPoint		m_point;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYPOINTDLG_H__0F292668_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
	
