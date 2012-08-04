#if !defined(AFX_REZBUTTON_H__234A03E4_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_REZBUTTON_H__234A03E4_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RezButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRezButton window

class CRezButton : public CButton
{
// Construction
public:
	CRezButton(CString sExt, CString sInitial);

	CString						m_sExt;
	CString						m_sRez;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRezButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRezButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRezButton)
	afx_msg void OnClicked();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REZBUTTON_H__234A03E4_7FBF_11D2_9B4A_0060971BDAD8__INCLUDED_)
