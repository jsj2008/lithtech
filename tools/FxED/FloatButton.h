#if !defined(AFX_FLOATBUTTON_H__B1D95DA7_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
#define AFX_FLOATBUTTON_H__B1D95DA7_9058_11D2_9B4B_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FloatButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFloatButton window

class CFloatButton : public CButton
{
// Construction
public:
	CFloatButton();

	void					SetValue(float *pfVal);
	
	float				   *m_pFloat;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFloatButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFloatButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFloatButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLOATBUTTON_H__B1D95DA7_9058_11D2_9B4B_0060971BDAD8__INCLUDED_)
