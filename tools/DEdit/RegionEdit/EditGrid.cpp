//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditGrid.cpp
//
//	PURPOSE	  : Implements the CEditGrid class.
//
//	CREATED	  : October 3 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editgrid.h"
#include "regiondoc.h"
#include "regionview.h"



CEditGrid::CEditGrid()
{
	Pos().Init( 0.0f, 0.0f, 0.0f );
	Forward().Init( 0.0f, 1.0f, 0.0f );
	Right().Init( 0.0f, 0.0f, 1.0f );
	Up().Init( 1.0f, 0.0f, 0.0f );

	m_DrawSize = 24 * 256.0f;
}


CEditGrid::CEditGrid( LTVector pos, LTVector forward, LTVector up, CReal drawSize )
{
	Pos() = pos;
	Forward() = forward;
	Up() = up;
	MakeRight();

	m_DrawSize = drawSize;
}


BOOL CEditGrid::IntersectRay( DWORD snapSize, CEditRay ray, LTVector &intersection, CReal *pT, BOOL bSnapToGrid )
{
	CReal	dot1, dot2, t;
	CReal	fSnapSize = (CReal)snapSize;

	
	// Trivial reject.
	if( Normal().Dot(ray.m_Pos - Pos()) >= 0.0 )
	{
		if( Forward().Dot(ray.m_Dir) >= 0.0 )
			return FALSE;
	}
	else
	{
		if( Normal().Dot(ray.m_Dir) <= 0.0 )
			return FALSE;
	}

	// Get the intersection.
	dot1 = Normal().Dot( ray.m_Pos - Pos() );
	dot2 = Normal().Dot( (ray.m_Pos+ray.m_Dir) - Pos() );
	t = -dot1 / (dot2 - dot1);
	
	intersection = ray.m_Pos + (ray.m_Dir * t);
	if( pT )
		*pT = t;

	//if the user doesn't want to snap to the grid, just return here
	if(!bSnapToGrid)
	{
		return TRUE;
	}

	// Snap to grid.
	LTVector	temp = intersection - Pos();
	CReal		xLen = Right().Dot( temp );
	CReal		yLen = Up().Dot( temp );

	SDWORD		xLenInt = (SDWORD)( xLen / fSnapSize );
	SDWORD		yLenInt = (SDWORD)( yLen / fSnapSize );

	SDWORD		xLenInt2 = (SDWORD)( (xLen * 2.0) / fSnapSize );
	SDWORD		yLenInt2 = (SDWORD)( (yLen * 2.0) / fSnapSize );
	
	SDWORD		xLenMod = xLenInt2 % 2;
	SDWORD		yLenMod = yLenInt2 % 2;


	xLen = (xLenMod==0) ? (fSnapSize*xLenInt) : (fSnapSize * (xLenInt+xLenMod));
	yLen = (yLenMod==0) ? (fSnapSize*yLenInt) : (fSnapSize * (yLenInt+yLenMod));

	intersection = Pos() + (Right()*xLen + Up()*yLen);
	return TRUE;
}





