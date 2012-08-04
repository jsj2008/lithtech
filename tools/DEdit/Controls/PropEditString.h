#if !defined(AFX_PROPEDITSTRING_H__9C8B9303_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_)
#define AFX_PROPEDITSTRING_H__9C8B9303_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropEditString.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropEditString window	

class CPropEditString : public CEdit
{
// Construction
public:
	CPropEditString();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropEditString)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropEditString();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropEditString)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:

	// Sets the button width in the control (this defaults to 12)
	void		SetButtonWidth(int nWidth)	{ m_nButtonWidth=nWidth; }

	// Add a button.  This returns the created button.  Note that it is
	// deleted by the control in the constructor.
	CButton		*AddButton(CString sButtonText, CWnd *pParent, int nID, CFont *pFont=NULL);

	// Position the control and its buttons
	void		Position(CRect rcControl);

protected:
	// The button width
	int								m_nButtonWidth;
	// The array of buttons
	CArray<CButton *, CButton *>	m_buttonArray;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPEDITSTRING_H__9C8B9303_03DA_11D3_BE1D_0060971BDC6D__INCLUDED_)
