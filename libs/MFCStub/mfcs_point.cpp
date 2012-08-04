// mfcs_point.cpp - implementation for the MFC stub CPoint class

#include "stdafx.h"
#include "mfcs_point.h"

CPoint::CPoint()
{
}

CPoint::CPoint(const CPoint &cPt) 
{
	*(POINT *)this = cPt;
}

CPoint::CPoint(const POINT *pPt) 
{
	*(POINT *)this = *pPt;
}

CPoint::CPoint(int32 newx, int32 newy)
{
	x = newx;
	y = newy;
}

