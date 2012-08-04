#if !defined(AFX_VECTORCOMBO_H__CB8B29C3_82CC_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_VECTORCOMBO_H__CB8B29C3_82CC_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VectorCombo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVectorCombo window

class CVectorCombo : public CComboBox
{
// Construction
public:
	CVectorCombo(DWORD dwID);

// Attributes
public:

	DWORD						m_dwID;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVectorCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CVectorCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(CVectorCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VECTORCOMBO_H__CB8B29C3_82CC_11D2_9B4A_0060971BDAD8__INCLUDED_)
