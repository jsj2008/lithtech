#include "stdafx.h"
#include "Volume.h"


//-------------------------------------------------------------------

CVolume::CVolume(long nLeft, long nTop, long nRight, long nBottom)
{
	m_rRect.left = nLeft;
	m_rRect.top = nTop;
	m_rRect.right = nRight;
	m_rRect.bottom = nBottom;
	
	m_nConnectionLen = 0;

	m_pVolPrev = NULL;
	m_pVolNext = NULL;
}

//-------------------------------------------------------------------

void CVolume::FindConnection()
{
	bool bMatched = false;

	if( m_rRect.left == m_pVolNext->m_rRect.right )
	{
		m_ptConnect1.x = m_rRect.left;
		m_ptConnect2.x = m_rRect.left;
		m_eConnectionType = kLeft;
		bMatched = true;
	}
	else if( m_rRect.right == m_pVolNext->m_rRect.left )
	{
		m_ptConnect1.x = m_rRect.right;
		m_ptConnect2.x = m_rRect.right;
		m_eConnectionType = kRight;
		bMatched = true;
	}

	if( bMatched )
	{
		m_ptConnect1.y = max( m_rRect.top, m_pVolNext->m_rRect.top );
		m_ptConnect2.y = min( m_rRect.bottom, m_pVolNext->m_rRect.bottom );

		m_nConnectionLen = m_ptConnect2.y - m_ptConnect1.y;

		return;
	}

	if( m_rRect.top == m_pVolNext->m_rRect.bottom )
	{
		m_ptConnect1.y = m_rRect.top;
		m_ptConnect2.y = m_rRect.top;
		m_eConnectionType = kTop;
		bMatched = true;
	}
	else if( m_rRect.bottom == m_pVolNext->m_rRect.top )
	{
		m_ptConnect1.y = m_rRect.bottom;
		m_ptConnect2.y = m_rRect.bottom;
		m_eConnectionType = kBottom;
		bMatched = true;
	}

	if( bMatched )
	{
		m_ptConnect1.x = max( m_rRect.left, m_pVolNext->m_rRect.left );
		m_ptConnect2.x = min( m_rRect.right, m_pVolNext->m_rRect.right );

		m_nConnectionLen = m_ptConnect2.x - m_ptConnect1.x;

		return;
	}
}

//-------------------------------------------------------------------

POINT CVolume::GetConnectMidPt()
{
	POINT ptMid;
	ptMid.x = m_ptConnect1.x + ( ( m_ptConnect2.x - m_ptConnect1.x ) / 2 );
	ptMid.y = m_ptConnect1.y + ( ( m_ptConnect2.y - m_ptConnect1.y ) / 2 );

	return ptMid;
}

//-------------------------------------------------------------------

POINT CVolume::FindNearest( POINT ptLast, long nWidth )
{
	long nGates = m_nConnectionLen / nWidth;

	if( nGates <= 1 )
	{
		return GetConnectMidPt();
	}

	POINT ptNearest;
	long iGate;

	switch( m_eConnectionType )
	{
		case kLeft:
		case kRight:
			iGate = ( ptLast.y - m_ptConnect1.y ) / nWidth;
			if( iGate < 0 )
			{
				iGate = 1;
			}
			else if( iGate >= nGates )
			{
				iGate = nGates - 1;
			}
	
			ptNearest.x = m_ptConnect1.x;
			ptNearest.y = m_ptConnect1.y + ( iGate * nWidth ) + ( nWidth / 2 ); 
			break;

		case kTop:
		case kBottom:
			iGate = ( ptLast.x - m_ptConnect1.x ) / nWidth;
			if( iGate < 0 )
			{
				iGate = 0;
			}
			else if( iGate >= nGates )
			{
				iGate = nGates - 1;
			}
	
			ptNearest.y = m_ptConnect1.y;
			ptNearest.x = m_ptConnect1.x + ( iGate * nWidth ) + ( nWidth / 2 ); 
			break;
	}

	return ptNearest;
}

//-------------------------------------------------------------------

void CVolume::BoundPoints( FPOINTLIST& lstEvaluatedCurvePoints, long iStart, long nWidth ) 
{
	float fThreshold = (float)nWidth;

	bool bInEnterance = (bool)(m_pVolPrev != NULL);
	bool bInExit = false;

	FPOINTLIST::iterator it;
	for( it = lstEvaluatedCurvePoints.begin() + iStart; it != lstEvaluatedCurvePoints.end(); ++it )
	{
		FPoint ptNew;
		FPoint ptCur = *it;
		FPoint ptLast = *(it - 1);

		//-------

		if( bInEnterance )
		{
			if( AvoidConnectionEdges( bInEnterance, m_pVolPrev->m_eConnectionType,
								    m_pVolPrev->m_ptConnect1, m_pVolPrev->m_ptConnect2,
									ptLast, ptCur, 
									ptNew, fThreshold, false ) )
			{
				FPOINTLIST::iterator it2;				
				it2 = lstEvaluatedCurvePoints.erase( lstEvaluatedCurvePoints.begin() + iStart, it );
				it = lstEvaluatedCurvePoints.insert( it2, ptNew );
				++it;
			}
		}

		//------

		if( m_pVolNext)
		{
			if( AvoidConnectionEdges( bInExit, m_eConnectionType,
								    m_ptConnect1, m_ptConnect2,
									ptLast, ptCur, 
									ptNew, fThreshold, true ) )
			{
				lstEvaluatedCurvePoints.erase( it, lstEvaluatedCurvePoints.end() - 1 );
				lstEvaluatedCurvePoints.insert( lstEvaluatedCurvePoints.end() - 1, ptNew );
				return;
			}
		}

		if( it == lstEvaluatedCurvePoints.end() - 1 )
		{
			return;
		}

		//------

		long dwCheck = 0xffffffff;
		if( bInEnterance )
		{
			switch( m_pVolPrev->m_eConnectionType )
			{
				case kLeft:
				case kRight:
					dwCheck &= ~( ( m_pVolPrev->m_eConnectionType == kLeft ) ? kRight : kLeft );
					break;

				case kTop:
				case kBottom:
					dwCheck &= ~( ( m_pVolPrev->m_eConnectionType == kTop ) ? kBottom : kTop );
					break;
			}
		}
		if( bInExit )
		{
			dwCheck &= ~m_eConnectionType;
		}

		//------

		if( ( dwCheck & kLeft ) && ( it->x < ((float)m_rRect.left) + fThreshold ) )
		{
			it->x = ((float)m_rRect.left) + fThreshold;
		}
		else if( ( dwCheck & kRight ) && ( it->x > ((float)m_rRect.right) - fThreshold ) )
		{
			it->x = ((float)m_rRect.right) - fThreshold;
		}

		if( ( dwCheck & kTop ) && ( it->y < ((float)m_rRect.top) + fThreshold ) )
		{
			it->y = ((float)m_rRect.top) + fThreshold;
		}
		else if( ( dwCheck & kBottom ) && ( it->y > ((float)m_rRect.bottom) - fThreshold ) )
		{
			it->y = ((float)m_rRect.bottom) - fThreshold;
		}
	}
}

//-------------------------------------------------------------------

bool CVolume::AvoidConnectionEdges( bool& bInConnection, ENUMCONNECTIONTYPE eConnectionType,
								    POINT ptConnect1, POINT ptConnect2,
									FPoint ptLast, FPoint ptCur, 
									FPoint& ptNew, float fThreshold,
									bool bReverse ) 
{
	bool bOrigInConnection = bInConnection;

	switch( eConnectionType )
	{
		case kLeft:
		case kRight:
			{
				float y;
				if( ptCur.y <= ptConnect1.y )
				{
					y = ptConnect1.y;
					bInConnection = false;
				}
				else if( ptCur.y >= ptConnect2.y )
				{
					y = ptConnect2.y;
					bInConnection = false;
				}
				else {
					bInConnection = true;

					if( ptLast.y <= ptConnect1.y )
					{
						y = ptConnect1.y;
					}
					else {
						y = ptConnect2.y;
					}
				}

				if( bOrigInConnection != bInConnection )
				{
					if( bReverse )
					{
						FPoint ptTemp = ptCur;
						ptCur = ptLast;
						ptLast = ptTemp;

						eConnectionType = ( eConnectionType == kLeft ) ? kRight : kLeft;
					}

					float fBoundary;
					if( eConnectionType == kLeft )
					{
						fBoundary = ptConnect1.x - fThreshold;
					}
					else {
						fBoundary = ptConnect1.x + fThreshold;
					}

					if( ( ( eConnectionType == kLeft ) && ( ptLast.x > fBoundary ) )
						|| ( ( eConnectionType == kRight ) && ( ptLast.x < fBoundary ) ) )
					{
						float fSlope = ( ptCur.y - ptLast.y ) / ( ptCur.x - ptLast.x );
						float x = ( ( y - ptLast.y ) / fSlope ) + ptLast.x;

						if( ( ( eConnectionType == kLeft ) && ( x > fBoundary ) )
							|| ( ( eConnectionType == kRight ) && ( x < fBoundary ) ) )
						{
							ptNew.x = fBoundary;
							ptNew.y = y;
							return true;
						}
					}
				}
			}
			break;

		case kTop:
		case kBottom:
			{
				float x;
				if( ptCur.x <= ptConnect1.x )
				{
					x = ptConnect1.x;
					bInConnection = false;
				}
				else if( ptCur.x >= ptConnect2.x )
				{
					x = ptConnect2.x;
					bInConnection = false;
				}
				else {
					bInConnection = true;

					if( ptLast.x <= ptConnect1.x )
					{
						x = ptConnect1.x;
					}
					else {
						x = ptConnect2.x;
					}
				}

				if( bOrigInConnection != bInConnection )
				{
					if( bReverse )
					{
						FPoint ptTemp = ptCur;
						ptCur = ptLast;
						ptLast = ptTemp;

						eConnectionType = ( eConnectionType == kTop ) ? kBottom : kTop;
					}

					float fBoundary;
					if( eConnectionType == kTop )
					{
						fBoundary = ptConnect1.y - fThreshold;
					}
					else {
						fBoundary = ptConnect1.y + fThreshold;
					}

					if( ( ( eConnectionType == kTop ) && ( ptLast.y > fBoundary ) )
						|| ( ( eConnectionType == kBottom ) && ( ptLast.y < fBoundary ) ) )
					{
						float fSlope = ( ptCur.y - ptLast.y ) / ( ptCur.x - ptLast.x );
						float y = ( ( x - ptLast.x ) * fSlope ) + ptLast.y;

						if( ( ( eConnectionType == kTop ) && ( y > fBoundary ) )
							|| ( ( eConnectionType == kBottom ) && ( y < fBoundary ) ) )
						{
							ptNew.x = x;
							ptNew.y = fBoundary;
							return true;
						}
					}
				}
			}
			break;
	}

	return false;
}

//-------------------------------------------------------------------

void CVolume::Draw(HDC hdc)
{
	HPEN hPen = CreatePen( PS_SOLID, 0, RGB(0,0,0) ); 

	SelectObject( hdc, hPen );

	SelectObject( hdc, GetStockObject(HOLLOW_BRUSH) ); 

	Rectangle( hdc, m_rRect.left, m_rRect.top, m_rRect.right, m_rRect.bottom );
}

//-------------------------------------------------------------------

void CVolume::DrawConnection(HDC hdc)
{
	HPEN hPen = CreatePen( PS_SOLID, 3, RGB(255,0,0) ); 

	SelectObject( hdc, hPen );

	MoveToEx( hdc, m_ptConnect1.x, m_ptConnect1.y, NULL );
	LineTo(	hdc, m_ptConnect2.x, m_ptConnect2.y );
}

//-------------------------------------------------------------------
