#if !defined(AFX_PROPEDITSTRINGLIST_H__D7B134C2_92BF_11D3_B55F_0050DA2031A6__INCLUDED_)
#define AFX_PROPEDITSTRINGLIST_H__D7B134C2_92BF_11D3_B55F_0050DA2031A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropEditStringList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropEditStringList window

class CPropEditStringList : public CComboBox
{
// Construction
public:
	CPropEditStringList();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropEditStringList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropEditStringList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropEditStringList)
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
	CArray<CButton *, CButton *>	m_buttonArray;};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPEDITSTRINGLIST_H__D7B134C2_92BF_11D3_B55F_0050DA2031A6__INCLUDED_)
