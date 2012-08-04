#if !defined(AFX_FLOATSPINCTRL_H__B5B13DC4_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_FLOATSPINCTRL_H__B5B13DC4_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FloatSpinCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFloatSpinCtrl window

class CFloatSpinCtrl : public CSpinButtonCtrl
{
// Construction
public:
	CFloatSpinCtrl(CEdit *pBuddy);

	CEdit							*m_pEdit;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFloatSpinCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFloatSpinCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFloatSpinCtrl)
	afx_msg void OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLOATSPINCTRL_H__B5B13DC4_8A0F_11D2_9B4A_0060971BDAD8__INCLUDED_)
