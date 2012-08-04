#if !defined(AFX_VECTORBUTTON_H__7DE1CCE2_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_)
#define AFX_VECTORBUTTON_H__7DE1CCE2_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VectorButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVectorButton window

class CVectorButton : public CButton
{
// Construction
public:
	CVectorButton(DWORD dwID);

// Attributes
public:

	DWORD						m_dwID;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVectorButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVectorButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVectorButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VECTORBUTTON_H__7DE1CCE2_D56C_11D2_9B4F_0060971BDAD8__INCLUDED_)
