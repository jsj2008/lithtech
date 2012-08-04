//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : UsefulDib.cpp
//
//	PURPOSE	  : Implements the CUsefulDib class.
//
//	CREATED	  : October 12 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "usefuldib.h"
#include "dibmgr.h"




CUsefulDib::CUsefulDib()
{
	m_pDib = NULL;
	m_nSelectors = 0;

	m_CurPenColor = 0;
	m_CurBrushColor = 0;
	m_CurRopMode = R2_COPYPEN;
}


void CUsefulDib::StartPen( int penStyle, int penWidth, COLORREF penColor )
{
	ASSERT( m_nSelectors < MAX_DIB_SELECTORS );
	ASSERT( m_pDib );

	CDibSelector *pSelector = &m_Selectors[m_nSelectors];
	
	pSelector->m_PenColor = m_CurPenColor = COLOR24_TO_16( penColor );
	pSelector->m_RopMode = m_CurRopMode = R2_COPYPEN;
	m_nSelectors++;
}


void CUsefulDib::StartBrush( COLORREF brushColor )
{
	ASSERT( m_nSelectors < MAX_DIB_SELECTORS );
	ASSERT( m_pDib );

	CDibSelector *pSelector   = &m_Selectors[m_nSelectors];
	
	pSelector->m_BrushColor = m_CurBrushColor = COLOR24_TO_16( brushColor );
	pSelector->m_RopMode = m_CurRopMode = R2_COPYPEN;
	m_nSelectors++;
}


void CUsefulDib::StartPenBrush( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor )
{
	ASSERT( m_nSelectors < MAX_DIB_SELECTORS );
	ASSERT( m_pDib );

	CDibSelector *pSelector = &m_Selectors[m_nSelectors];
	
	pSelector->m_PenColor   = m_CurPenColor = COLOR24_TO_16( penColor );
	pSelector->m_BrushColor = m_CurBrushColor = COLOR24_TO_16( brushColor );

	m_nSelectors++;
}


void CUsefulDib::StartPenBrushROP( int penStyle, int penWidth, COLORREF penColor, COLORREF brushColor, int ropMode )
{
	StartPenBrush( penStyle, penWidth, penColor, brushColor );
	
	CDibSelector *pSelector = &m_Selectors[m_nSelectors-1];
	
	pSelector->m_RopMode = m_CurRopMode = ropMode;
}


void CUsefulDib::End( int count )
{
	int		i;

	
	for( i=0; i < count; i++ )
	{
		ASSERT( m_nSelectors != 0 );
		ASSERT( m_pDib );

		--m_nSelectors;
		
		if( m_nSelectors > 0 )
		{
			CDibSelector *pPrevSel = &m_Selectors[ m_nSelectors-1 ];
			m_CurPenColor = pPrevSel->m_PenColor;
			m_CurBrushColor = pPrevSel->m_BrushColor;
			m_CurRopMode = pPrevSel->m_RopMode;
		}
		else
		{
			m_CurPenColor = 0;
			m_CurBrushColor = 0;
			m_CurRopMode = R2_COPYPEN;
		}
	}
}
														   

void CUsefulDib::DrawLine( CPoint &pt1, CPoint &pt2 )
{
	CVector fPt1((CReal)pt1.x, (CReal)pt1.y, 0.0f);
	CVector	fPt2((CReal)pt2.x, (CReal)pt2.y, 0.0f);

	DrawLine( fPt1, fPt2 );
}


void CUsefulDib::DrawLine( CVector &pt1, CVector &pt2 )
{
	WORD		*pBuffer = m_pDib->GetBuf16();
	WORD		*pCurPos;
	DWORD		pitch = m_pDib->GetPitch();
	CReal		diffX, diffY;

	DWORD		startX, endX, startY, endY;
	DWORD		x, y, i;

	CReal		curX, curY, deltaX, deltaY;
	CVector		start, end;
	

	ASSERT( m_pDib );

	
	pt1.x = CLAMP( pt1.x, 0.0f, (CReal)(m_pDib->GetWidth()-1) );
	pt1.y = CLAMP( pt1.y, 0.0f, (CReal)(m_pDib->GetHeight()-1) );
	pt2.x = CLAMP( pt2.x, 0.0f, (CReal)(m_pDib->GetWidth()-1) );
	pt2.y = CLAMP( pt2.y, 0.0f, (CReal)(m_pDib->GetHeight()-1) );


	diffX = (pt1.x > pt2.x) ? (pt1.x - pt2.x) : (pt2.x - pt1.x);
	diffY = (pt1.y > pt2.y) ? (pt1.y - pt2.y) : (pt2.y - pt1.y);
	if( (int)diffX == 0 && (int)diffY == 0 )
	{
		return;
	}
	if( (int)diffX == 0 )
	{
		startY = (DWORD)((pt1.y < pt2.y) ? pt1.y : pt2.y);
		endY = (DWORD)((pt1.y > pt2.y) ? pt1.y : pt2.y);
		
		pCurPos = &pBuffer[ startY*pitch + (int)pt1.x ];

		if( m_CurRopMode == R2_COPYPEN )
		{
			for( i=startY; i < endY; i++ )
			{
				*pCurPos = m_CurPenColor;
				pCurPos += pitch;
			}
		}
		else if( m_CurRopMode == R2_NOT )
		{
			for( i=startY; i < endY; i++ )
			{
				*pCurPos ^= 0xFFFF;
				pCurPos += pitch;
			}
		}
	}
	else if( (int)diffY == 0 )
	{
		startX = (DWORD)((pt1.x < pt2.x) ? pt1.x : pt2.x);
		endX = (DWORD)((pt1.x > pt2.x) ? pt1.x : pt2.x);
		
		pCurPos = &pBuffer[ (int)pt1.y*pitch + startX ];
		
		if( m_CurRopMode == R2_COPYPEN )
		{
			for( i=startX; i < endX; i++ )
				*pCurPos++ = m_CurPenColor;
		}
		else if( m_CurRopMode == R2_NOT )
		{
			for( i=startX; i < endX; i++ )
			{
				*pCurPos ^= 0xFFFF;
				pCurPos++;
			}
		}
	}
	else
	{
		if( diffX > diffY )
		{
			if( pt1.x < pt2.x )
			{
				start = pt1;
				end = pt2;
			}
			else
			{
				start = pt2;
				end = pt1;
			}
			
			// Iterate over X, incrementing delta Y.
			deltaY = (end.y - start.y) / (end.x - start.x);
			curY = start.y + (deltaY * ( (CReal)ceil(start.x) - start.x ));

			if( m_CurRopMode == R2_COPYPEN )
			{
				for( x = (DWORD)start.x; x < (DWORD)end.x; x++ )
				{
					pBuffer[ (int)curY * pitch + x ] = m_CurPenColor;
					curY += deltaY;
				}				
			}
			else if( m_CurRopMode == R2_NOT )
			{
				for( x = (DWORD)start.x; x < (DWORD)end.x; x++ )
				{
					pBuffer[ (int)curY * pitch + x ] ^= 0xFFFF;
					curY += deltaY;
				}				
			}
		}
		else
		{
			// Iterate over Y, incrementing delta X.
			if( pt1.y < pt2.y )
			{
				start = pt1;
				end = pt2;
			}
			else
			{
				start = pt2;
				end = pt1;
			}
			
			deltaX = (end.x - start.x) / (end.y - start.y);
			curX = start.x + (deltaX * ((CReal)ceil(start.y) - start.y));

			if( m_CurRopMode == R2_COPYPEN )
			{
				for( y = (DWORD)start.y; y < (DWORD)end.y; y++ )
				{
					pBuffer[ y * pitch + (int)curX ] = m_CurPenColor;
					curX += deltaX;
				}				
			}
			else if( m_CurRopMode == R2_NOT )
			{
				for( y = (DWORD)start.y; y < (DWORD)end.y; y++ )
				{
					pBuffer[ y * pitch + (int)curX ] ^= 0xFFFF;
					curX += deltaX;
				}				
			}
		}
	}
}


void CUsefulDib::DrawEllipse( int x1, int y1, int x2, int y2 )
{
	int			temp, curY;
	CReal		angle, angleInc;
	CReal		halfWidth;

	CReal		fStartX, fEndX, fCenterX;
	int			startX, endX;

	WORD		*pCurPos;
	DWORD		len;

	DWORD		pitch = m_pDib->GetPitch();
	int			maxWidth = m_pDib->GetWidth()-1;
	DWORD		dwBrushColor = m_CurBrushColor;

	
	// Make sure the coordinates go the right way.
	if( x1 > x2 ) {	temp = x1; x1=x2; x2=temp; }
	if( y1 > y2 ) {	temp = y1; y1=y2; y2=temp; }

	if( ((y2-y1) == 0) || ((x2-x1) == 0) )
		return;

	halfWidth = ((CReal)(x2-x1)) * 0.5f;
	fCenterX = (CReal)x1 + halfWidth;

	angle = 0.0f;
	angleInc = MATH_PI / (CReal)(y2-y1);

	// Clip vertically.
	if( y1 < 0 )
	{
		angle = angleInc * -y1;
		y1 = 0;
	}

	if( y2 > m_pDib->GetHeight() )
		y2 = m_pDib->GetHeight();


	for( curY=y1; curY < y2; curY++ )
	{
		// Get the X coordinates.
		fEndX = (float)sin(angle) * halfWidth;
		fStartX = fCenterX - fEndX;
		fEndX += fCenterX;

		startX = (int)fStartX;
		endX = (int)fEndX;

		// Clip horizontally.
		if( (startX < maxWidth) && (endX >= 0) )
		{
			if( startX < 0 )		startX = 0;
			if( endX > maxWidth )	endX = maxWidth;
		
			pCurPos = &m_pDib->GetBuf16()[curY*pitch + startX];
			len = (DWORD)((endX-startX)+1);

			__asm
			{
				mov		edi, pCurPos;
				mov		ecx, len
				mov		eax, dwBrushColor

				repnz	stosw
			}
		}

		angle += angleInc;
	}
}


void CUsefulDib::DrawRectangle( int x1, int y1, int x2, int y2 )
{
	ASSERT( m_pDib );

	CRect		rect( x1, y1, x2, y2 );

	WORD		*pBuffer = m_pDib->GetBuf16();
	WORD		*pCurPos, *pCurPos2;
	DWORD		pitch = m_pDib->GetPitch();	
	int			x, y;
	DWORD		len;
	DWORD		dwBrushColor = m_CurBrushColor;

		
	rect.left   = CLAMP(rect.left, 0, (int)(m_pDib->GetWidth()-1));
	rect.top    = CLAMP(rect.top, 0, (int)(m_pDib->GetHeight()-1));
	rect.right  = CLAMP(rect.right, 0, (int)(m_pDib->GetWidth()-1));
	rect.bottom = CLAMP(rect.bottom, 0, (int)(m_pDib->GetHeight()-1));
	rect.NormalizeRect();

	if( rect.IsRectEmpty() )
		return;


	if( m_CurRopMode == R2_COPYPEN )
	{
		// Draw the filled portion.
		if( (rect.right-rect.left) > 2 )
		{
			for( y=rect.top+1; y <= rect.bottom-1; y++ )
			{
				pCurPos = &pBuffer[y*pitch + rect.left + 1];
				len = (rect.right - rect.left) - 2;

				__asm
				{
					mov		edi, pCurPos;
					mov		ecx, len
					mov		eax, dwBrushColor

					repnz	stosw
				}

				//for( x=rect.left+1; x <= rect.right-1; x++ )
					//*pCurPos++ = m_CurBrushColor;
			}
		}

		// Draw the border.
		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.bottom*pitch+rect.left];
		for( x=x1; x < rect.right; x++ )
		{
			*pCurPos++ = m_CurPenColor;
			*pCurPos2++ = m_CurPenColor;
		}

		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.top*pitch+rect.right-1];
		for( y=y1; y <= rect.bottom; y++ )
		{
			*pCurPos = m_CurPenColor;
			*pCurPos2 = m_CurPenColor;
			
			pCurPos += pitch;
			pCurPos2 += pitch;
		}
	}
	else if( m_CurRopMode == R2_XORPEN )
	{
		// Draw the filled portion.
		if( (rect.right-rect.left) > 2 )
		{
			for( y=rect.top+1; y <= rect.bottom-1; y++ )
			{
				pCurPos = &pBuffer[y*pitch + rect.left + 1];
				for( x=rect.left+1; x <= rect.right-1; x++ )
					*pCurPos++ ^= m_CurBrushColor;
			}
		}

		// Draw the border.
		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.bottom*pitch+rect.left];
		for( x=x1; x < rect.right; x++ )
		{
			*pCurPos++ ^= m_CurPenColor;
			*pCurPos2++ ^= m_CurPenColor;
		}

		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.top*pitch+rect.right-1];
		for( y=y1; y <= rect.bottom; y++ )
		{
			*pCurPos ^= m_CurPenColor;
			*pCurPos2 ^= m_CurPenColor;
			
			pCurPos += pitch;
			pCurPos2 += pitch;
		}
	}
	else if( m_CurRopMode == R2_NOT )
	{
		// Draw the filled portion.
		if( (rect.right-rect.left) > 2 )
		{
			for( y=rect.top+1; y <= rect.bottom-1; y++ )
			{
				pCurPos = &pBuffer[y*pitch + rect.left + 1];
				for( x=rect.left+1; x <= rect.right-1; x++ )
					*pCurPos++ ^= 0xFFFF;
			}
		}

		// Draw the border.
		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.bottom*pitch+rect.left];
		for( x=x1; x < rect.right; x++ )
		{
			*pCurPos++ ^= 0xFFFF;
			*pCurPos2++ ^= 0xFFFF;
		}

		pCurPos = &pBuffer[rect.top*pitch+rect.left];
		pCurPos2 = &pBuffer[rect.top*pitch+rect.right-1];
		for( y=y1; y <= rect.bottom; y++ )
		{
			*pCurPos ^= 0xFFFF;
			*pCurPos2 ^= 0xFFFF;
			
			pCurPos += pitch;
			pCurPos2 += pitch;
		}
	}
}






