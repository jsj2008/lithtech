//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_COLORWELL_H__BA288E81_0342_11D1_99E3_0060970987C3__INCLUDED_)
#define AFX_COLORWELL_H__BA288E81_0342_11D1_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ColorWell.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorWell window

class CColorWell : public CButton
{
// Construction
public:
	CColorWell();

// Attributes
public:

	UINT m_red, m_green, m_blue;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorWell)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL Create( LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorWell();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorWell)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORWELL_H__BA288E81_0342_11D1_99E3_0060970987C3__INCLUDED_)
