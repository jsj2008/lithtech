#if !defined(AFX_INTSPINCTRL_H__B5B13DC5_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_INTSPINCTRL_H__B5B13DC5_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntSpinCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntSpinCtrl window

class CIntSpinCtrl : public CSpinButtonCtrl
{
// Construction
public:
	CIntSpinCtrl(CEdit *pBuddy);

	CEdit							*m_pEdit;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntSpinCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIntSpinCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIntSpinCtrl)
	afx_msg void OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTSPINCTRL_H__B5B13DC5_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_)
