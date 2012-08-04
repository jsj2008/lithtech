//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __COLORWELL_H__
#define __COLORWELL_H__


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


#endif 
