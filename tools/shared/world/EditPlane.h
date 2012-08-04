//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditPlane.h
//
//	PURPOSE	  : Defines the CEditPlane class.  This is used in 
//              CBasePolies and has functions to split them.
//
//	CREATED	  : October 21 1996
//
//
//------------------------------------------------------------------

#ifndef __EDITPLANE_H__
	#define __EDITPLANE_H__


	// Includes....
	#include "editvert.h"
	#include "geometry.h"
	#include "ltbasedefs.h"


	// Defines....
	class CBasePoly;

	extern uint32					g_nEditIntersect, g_nEditBack, g_nEditFront;
	extern PolySide					g_EditPointSides[MAX_POLYSIDE_POINTS];
	extern CReal					g_EditPointDots[MAX_POLYSIDE_POINTS];


	class CEditPlane : public CPlane
	{
		public:

			PolySide		GetPolySide( CBasePoly *pPoly );
			void			SplitPoly( CBasePoly *pToSplit, CBasePoly *sides[2], CEditVertArray &verts );
			void			SplitPoly( CBasePoly *pToSplit, CBasePoly *sides[2], CEditVertArray &verts1, CEditVertArray &verts2 );

	};


#endif  // __EDITPLANE_H__
