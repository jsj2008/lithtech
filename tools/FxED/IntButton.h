#if !defined(AFX_INTBUTTON_H__B1D95DA4_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
#define AFX_INTBUTTON_H__B1D95DA4_9058_11D2_9B4B_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IntButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntButton window

class CIntButton : public CButton
{
// Construction
public:
	CIntButton();

	void					SetValue(CString sDlgDisp, int *pInt);
	
	int					   *m_pInt;
	CString					m_sDlgDisp;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIntButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIntButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INTBUTTON_H__B1D95DA4_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
