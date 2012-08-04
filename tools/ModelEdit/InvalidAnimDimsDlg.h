#ifndef __INVALIDANIMDIMSDLG_H__
#define __INVALIDANIMDIMSDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CommandStringDlg.h : header file
//

#include "model.h"


/////////////////////////////////////////////////////////////////////////////
// CInvalidAnimDimsDlg dialog

class CInvalidAnimDimsDlg : public CDialog
{
// Construction
public:
	CInvalidAnimDimsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInvalidAnimDimsDlg)
	enum { IDD = IDD_INVALIDANIMDIMS };
	CEdit	m_CommandString;
	//}}AFX_DATA

	//the animation info. This must be filled out prior to DoModal
	AnimInfo* m_pAnimInfo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvalidAnimDimsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	afx_msg void UpdateEnabled();
	afx_msg void OnButtonUseX();
	afx_msg void OnButtonUseZ();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInvalidAnimDimsDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}


#endif 
