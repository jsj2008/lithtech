#include "stdafx.h"
#include "World.h"
#include "Volume.h"
#include <math.h>
#include "Data.h"


CWorld* g_pWorld = NULL;

//-------------------------------------------------------------------

CWorld::CWorld()
{
//	CData1 data;
//	CData2 data;
	CData3 data;

//	m_nWidth = 10;
	m_nWidth = 5;

	data.Init( this );

	BuildPath();
}

//-------------------------------------------------------------------

void CWorld::AddVolume(long nLeft, long nTop, long nRight, long nBottom)
{
	CVolume* pVol = new CVolume( nLeft, nTop, nRight, nBottom );

	if( m_lstVolumes.size() )
	{
		pVol->SetPrev( m_lstVolumes.back() );
		m_lstVolumes.back()->SetNext( pVol );
		m_lstVolumes.back()->FindConnection();
	}

	m_lstVolumes.push_back( pVol );
}

//-------------------------------------------------------------------

void CWorld::Draw(HDC hdc)
{
	DrawGrid( hdc );
	DrawVolumes( hdc );
	DrawPath( hdc );
}

//-------------------------------------------------------------------

void CWorld::DrawGrid(HDC hdc)
{
	HPEN hPen = CreatePen( PS_SOLID, 0, RGB(55,211,244) ); 

	SelectObject( hdc, hPen );

	for( long x=10; x < 800; x += 10 )
	{
		MoveToEx( hdc, x, 0, NULL );
		LineTo(	hdc, x, 600 );
	}

	for( long y=10; y < 600; y += 10 )
	{
		MoveToEx( hdc, 0, y, NULL );
		LineTo(	hdc, 800, y );
	}
}

//-------------------------------------------------------------------

void CWorld::DrawVolumes(HDC hdc)
{
	CVolume* pVol;
	VOLUMELIST::iterator it;
	for( it = m_lstVolumes.begin(); it != m_lstVolumes.end(); ++it )
	{
		pVol = *it;
		pVol->Draw( hdc );
	}

	for( it = m_lstVolumes.begin(); it != m_lstVolumes.end(); ++it )
	{
		pVol = *it;
		pVol->DrawConnection( hdc );
	}
}

//-------------------------------------------------------------------

void CWorld::DrawPath(HDC hdc)
{
	HPEN hPen = CreatePen( PS_SOLID, 0, RGB(31,182,77) ); 

	SelectObject( hdc, hPen );

	MoveToEx( hdc, m_ptStart.x, m_ptStart.y, NULL );

	POINT pt;
	CVolume* pVol;
	VOLUMELIST::iterator it;
	for( it = m_lstVolumes.begin(); it != m_lstVolumes.end() - 1; ++it )
	{
		pVol = *it;
		pt = pVol->GetConnectMidPt();
		LineTo(	hdc, pt.x, pt.y );
	}

	LineTo(	hdc, m_ptEnd.x, m_ptEnd.y );

	//--

	hPen = CreatePen( PS_SOLID, 0, RGB(0,0,255) ); 

	SelectObject( hdc, hPen );

	MoveToEx( hdc, m_ptStart.x, m_ptStart.y, NULL );

long size = m_lstEvaluatedCurvePoints.size();

	FPOINTLIST::iterator fit;
	for( fit = m_lstEvaluatedCurvePoints.begin(); fit != m_lstEvaluatedCurvePoints.end(); ++fit )
	{
		pt.x = fit->x;
		pt.y = fit->y;

		LineTo(	hdc, pt.x, pt.y );
	}
}

//-------------------------------------------------------------------

void CWorld::BuildPath()
{
	POINTLIST lstControlPts;

	lstControlPts.push_back( m_ptStart );

	POINT pt;
	CVolume* pVol;
	VOLUMELIST::iterator it;
	for( it = m_lstVolumes.begin(); it != m_lstVolumes.end() - 1; ++it )
	{
		pVol = *it;
///		pt = pVol->GetConnectMidPt();

		if( it != m_lstVolumes.end() - 2 )
		{
			pt = pVol->FindNearest( lstControlPts.back(), m_nWidth * 3 );
		}
		else {
			pt = pVol->FindNearest( m_ptEnd, m_nWidth * 3 );
		}

		lstControlPts.push_back( pt );
	}

	lstControlPts.push_back( m_ptEnd );

	EvaluateCurveC2Interp( lstControlPts, m_lstEvaluatedCurvePoints );

	long size = m_lstEvaluatedCurvePoints.size();
}

//-------------------------------------------------------------------

void CWorld::EvaluateCurveC2Interp(const POINTLIST& lstControlPts, FPOINTLIST& lstEvaluatedCurvePoints)
{
	unsigned long cControlPoints = lstControlPts.size();

	lstEvaluatedCurvePoints.clear();

	FPoint fpt;
	std::vector<FPoint> copy;
	for(int i=0; i <cControlPoints; ++i) 
	{
		fpt.x = lstControlPts[i].x;
		fpt.y = lstControlPts[i].y;
		copy.push_back( fpt );
	}

//	lstEvaluatedCurvePoints.push_back( copy[0] );

	std::vector<double> Y;
	Y.push_back(0.5);

	long m = cControlPoints - 1;
	for(i=1; i<m; ++i)
	{
		Y.push_back(1.0 / (4.0 - Y.back()) );
	}
	Y.push_back(1.0 / (2.0-Y.back()) );

	FPoint p;
	FPOINTLIST S;
	p.x = 3.0 * (copy[1].x - copy[0].x) * Y[0];
	p.y = 3.0 * (copy[1].y - copy[0].y) * Y[0];
	S.push_back(p);

	for(i=1; i<m; i++) {
		p.x = (3.0 * (copy[i+1].x - copy[i-1].x) - S.back().x) * Y[i];
		p.y = (3.0 * (copy[i+1].y - copy[i-1].y) - S.back().y) * Y[i];
		S.push_back(p);
	}

	p.x = (3.0 * (copy[m].x - copy[m-1].x) - S.back().x) * Y[m];
	p.y = (3.0 * (copy[m].y - copy[m-1].y) - S.back().y) * Y[m];
	S.push_back(p);

	FPoint *D = new FPoint[m+1];
	D[m] = S[m];
	for(i = m-1; i >= 0; i--) {
		D[i].x = S[i].x - (Y[i] * D[i+1].x);
		D[i].y = S[i].y - (Y[i] * D[i+1].y);
	}

	long iStart;
	for(i=0; i<cControlPoints-1; i++) {
		FPoint V[4];
		V[0] = copy[i];
		V[1].x = copy[i].x + ((1.0/3.0) * D[i].x);
		V[1].y = copy[i].y + ((1.0/3.0) * D[i].y);
		V[2].x = copy[i + 1].x - ((1.0/3.0) * D[i + 1].x);
		V[2].y = copy[i + 1].y - ((1.0/3.0) * D[i + 1].y);
		V[3] = copy[i + 1];

		iStart = lstEvaluatedCurvePoints.size();
		if( iStart == 0 )
		{
			iStart = 1;
		}

char temp[100];
sprintf(temp, "Display: %d to %d\n", i, i+1);
OutputDebugString(temp);
		DisplayBezier(V[0], V[1], V[2], V[3], lstEvaluatedCurvePoints);		

		m_lstVolumes[i]->BoundPoints( lstEvaluatedCurvePoints, iStart, m_nWidth );
	}
}

//-------------------------------------------------------------------

void CWorld::DisplayBezier(const FPoint& p0, const FPoint& p1, 
							const FPoint& p2, const FPoint& p3, 
							FPOINTLIST& lstEvaluatedCurvePoints) 
{
//char temp[100];

	if(FlatEnough2(p0, p1, p2, p3)) 
	{
		if( lstEvaluatedCurvePoints.size() == 0 )
		{
			lstEvaluatedCurvePoints.push_back(p0);
//sprintf(temp, "Pushing: %.3f, %.3f\n", p0.x, p0.y);
//OutputDebugString(temp);
		}

		lstEvaluatedCurvePoints.push_back(p3);
//sprintf(temp, "Pushing: %.3f, %.3f\n", p3.x, p3.y);
//OutputDebugString(temp);
	}
	else {
		FPoint l0, l1, l2, l3;
		FPoint r0, r1, r2, r3;
		Subdivide(p0,p1,p2,p3,l0,l1,l2,l3,r0,r1,r2,r3);
		DisplayBezier(l0, l1, l2, l3, lstEvaluatedCurvePoints);
		DisplayBezier(r0, r1, r2, r3, lstEvaluatedCurvePoints);
	}
}

//-------------------------------------------------------------------

bool CWorld::FlatEnough(const FPoint& p0, const FPoint& p1, 
						const FPoint& p2, const FPoint& p3) 
{
	if(p0.x < 0.0 && p3.x > 0.0)
		return false;
	
	double lenSeg1 = Len(p0, p1);
	double lenSeg2 = Len(p1, p2);
	double lenSeg3 = Len(p2, p3);
	double lenEnds = Len(p0, p3);

	if( ((lenSeg1 + lenSeg2 + lenSeg3) / lenEnds) < 1.0 + 0.005)
//	if( ((lenSeg1 + lenSeg2 + lenSeg3) / lenEnds) < 1.0 + 0.05)
		return true;
	else
		return false;
}
 
//-------------------------------------------------------------------

double CWorld::Len(const FPoint& p0, const FPoint& p1) 
{
	return sqrt( ((p1.x - p0.x)*(p1.x - p0.x)) + ((p1.y - p0.y)*(p1.y - p0.y)) );
}

//-------------------------------------------------------------------

double CWorld::LenSqr(const FPoint& p0, const FPoint& p1) 
{
	return ( ((p1.x - p0.x)*(p1.x - p0.x)) + ((p1.y - p0.y)*(p1.y - p0.y)) );
}

//-------------------------------------------------------------------

bool CWorld::FlatEnough2(const FPoint& p0, const FPoint& p1, 
						const FPoint& p2, const FPoint& p3) 
{
	FPoint v0 = p1;
	v0.x -= p0.x;
	v0.y -= p0.y;
	v0 = Normalize( v0 );

	FPoint v1 = p3;
	v1.x -= p0.x;
	v1.y -= p0.y;
	v1 = Normalize( v1 );

	float fDot = DotProduct( v0, v1 );
	float fAngle = acos( fDot );
	float fDegrees = fAngle * ( 180.f / 3.14f );

	if( fDegrees < 5.f )
//	if( fDegrees < 180.f )
		return true;
	else
		return false;
}

//-------------------------------------------------------------------

float CWorld::DotProduct (const FPoint& v1, const FPoint& v2)
{
   return v1.x*v2.x + v1.y * v2.y;
}

//-------------------------------------------------------------------

FPoint CWorld::Normalize(const FPoint& v)
{
	float fMag = Magnitude( v );

	FPoint vResult;
	vResult.x = v.x / fMag;
	vResult.y = v.y / fMag;

	return vResult;
}

//-------------------------------------------------------------------

float CWorld::Magnitude(const FPoint& v)
{
   return (float)sqrt( v.x*v.x + v.y*v.y );
}
  
//-------------------------------------------------------------------

void CWorld::Subdivide(const FPoint& p0, const FPoint& p1,
						const FPoint& p2, const FPoint& p3,
						FPoint& l0, FPoint& l1, FPoint& l2, FPoint& l3,
						FPoint& r0, FPoint& r1, FPoint& r2, FPoint& r3) 
{
	FPoint leftSplitStart;
	FPoint leftSplitEnd;
	FPoint leftMid;
	leftSplitStart	= linearlyInterpolate(p0, p1, 0.5);
	leftSplitEnd	= linearlyInterpolate(p1, p2, 0.5);
	leftMid			= linearlyInterpolate(leftSplitStart, leftSplitEnd, 0.5);
	
	FPoint rightSplitStart;
	FPoint rightSplitEnd;
	FPoint rightMid;
	rightSplitStart	= linearlyInterpolate(p1, p2, 0.5);
	rightSplitEnd	= linearlyInterpolate(p2, p3, 0.5);
	rightMid		= linearlyInterpolate(rightSplitStart, rightSplitEnd, 0.5);

	FPoint mid = linearlyInterpolate(leftMid, rightMid, 0.5);

	l0 = p0;
	l1 = leftSplitStart;
	l2 = leftMid;
	l3 = mid;

	r0 = mid;
	r1 = rightMid;
	r2 = rightSplitEnd;
	r3 = p3;
}

//-------------------------------------------------------------------

FPoint CWorld::linearlyInterpolate(const FPoint& first_point, const FPoint& second_point, double t)
{
	double interpolated_x = (t) * first_point.x + (1 - t) * second_point.x;
	double interpolated_y = (t) * first_point.y + (1 - t) * second_point.y;
	
	FPoint result;
	result.x = interpolated_x;
	result.y = interpolated_y;

	return result;
}
