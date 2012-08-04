//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : UsefulDC.cpp
//
//	PURPOSE	  : Implements the CUsefulDC class.
//
//	CREATED	  : October 4 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "usefuldc.h"




void CUsefulDC::StartPen( int penStyle, int penWidth, COLORREF penColor )
{
	ASSERT( m_nSelectors < MAX_SELECTORS );
	ASSERT( m_pDC );

	CSelector *pSelector = &m_Selectors[m_nSelectors];
	
	m_SelectorPens[m_nSelectors].CreatePen( penStyle, penWidth, penColor );
	pSelector->m_pPen    = &m_SelectorPens[m_nSelectors];
	pSelector->m_pOldPen = m_pDC->SelectObject( pSelector->m_pPen );

	m_nSelectors++;
}


void CUsefulDC::StartBrush( COLORREF brushColor )
{
	ASSERT( m_nSelectors < MAX_SELECTORS );
	ASSERT( m_pDC );

	CSelector *pSelector   = &m_Selectors[m_nSelectors];
	
	m_SelectorBrushes[m_nSelectors].CreateSolidBrush( brushColor );
	pSelector->m_pBrush    = &m_SelectorBrushes[m_nSelectors];
	pSelector->m_pOldBrush = m_pDC->SelectObject( pSelector->m_pBrush );

	m_nSelectors++;
}


void CUsefulDC::StartPenBrush( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor )
{
	ASSERT( m_nSelectors < MAX_SELECTORS );
	ASSERT( m_pDC );

	CSelector *pSelector = &m_Selectors[m_nSelectors];
	
	m_SelectorPens[m_nSelectors].CreatePen( penStyle, penWidth, penColor );
	m_SelectorBrushes[m_nSelectors].CreateSolidBrush( brushColor );

	pSelector->m_pPen      = &m_SelectorPens[m_nSelectors];
	pSelector->m_pBrush    = &m_SelectorBrushes[m_nSelectors];

	pSelector->m_pOldPen   = m_pDC->SelectObject( pSelector->m_pPen );
	pSelector->m_pOldBrush = m_pDC->SelectObject( pSelector->m_pBrush );

	pSelector->m_OldRopMode = m_pDC->GetROP2();

	m_nSelectors++;
}


void CUsefulDC::StartPenBrushROP( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor, int ropMode )
{
	StartPenBrush( penStyle, penWidth, penColor, brushColor );
	CSelector *pSelector = &m_Selectors[m_nSelectors-1];
	
	pSelector->m_OldRopMode = m_pDC->GetROP2();
	m_pDC->SetROP2( ropMode );
}


void CUsefulDC::End( int count )
{
	int		i;

	
	for( i=0; i < count; i++ )
	{
		ASSERT( m_nSelectors != 0 );
		ASSERT( m_pDC );

		CSelector *pSelector = &m_Selectors[--m_nSelectors];
		
		if( pSelector->m_pPen )
		{
			m_pDC->SelectObject( pSelector->m_pOldPen );
			pSelector->m_pPen->DeleteObject();
		}

		if( pSelector->m_pBrush )
		{
			m_pDC->SelectObject( pSelector->m_pOldBrush );
			pSelector->m_pBrush->DeleteObject();
		}

		m_pDC->SetROP2( pSelector->m_OldRopMode );
	}
}


void CUsefulDC::DrawLine( CPoint &pt1, CPoint &pt2 )
{
	ASSERT( m_pDC );

	m_pDC->MoveTo( (POINT)pt1 );
	m_pDC->LineTo( (POINT)pt2 );
}


void CUsefulDC::DrawLine( CVector &pt1, CVector &pt2 )
{
	ASSERT( m_pDC );

	CPoint		iPt1( (int)pt1.x, (int)pt1.y );
	CPoint		iPt2( (int)pt2.x, (int)pt2.y );

	DrawLine( iPt1, iPt2 );
}





