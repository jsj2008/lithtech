//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ColorWell.cpp : implementation file
//
#include "stdafx.h"
#include "colorwell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorWell

CColorWell::CColorWell()
{
}

CColorWell::~CColorWell()
{
}


BEGIN_MESSAGE_MAP(CColorWell, CButton)
	//{{AFX_MSG_MAP(CColorWell)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorWell message handlers

void CColorWell::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	CDC *pDC = GetDC( );
	CRect rect;

	GetClientRect( &rect );

	CBrush brush, *pOldBrush;

	brush.CreateSolidBrush( PALETTERGB( m_red, m_green, m_blue ));
	pOldBrush = pDC->SelectObject( &brush );

	pDC->PatBlt( rect.left, rect.top, rect.Width( ), rect.Height( ),
		  PATCOPY);

	pDC->SelectObject( pOldBrush );

	pDC->DrawEdge( &rect, EDGE_RAISED, BF_RECT );

	ReleaseDC( pDC );	
}

BOOL CColorWell::Create( LPCTSTR lpszCaption, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	dwStyle |= 	BS_OWNERDRAW;
	return CButton::Create( lpszCaption, dwStyle, rect, pParentWnd, nID );
}
