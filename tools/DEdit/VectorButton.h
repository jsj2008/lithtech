//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_VECTORBUTTON_H__3FE3DF81_17AB_11D1_99E4_0060970987C3__INCLUDED_)
#define AFX_VECTORBUTTON_H__3FE3DF81_17AB_11D1_99E4_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// VectorButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVectorButton window

class CVectorButton : public CButton, public CVector
{
// Construction
public:
	CVectorButton();

// Attributes
public:
	
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
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VECTORBUTTON_H__3FE3DF81_17AB_11D1_99E4_0060970987C3__INCLUDED_)
