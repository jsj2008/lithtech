//------------------------------------------------------------------
//
//	FILE	  : Surface.cpp
//
//	PURPOSE	  : Implements the CSurface class.
//
//	CREATED	  : February 12 1997
//
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "presurface.h"
#include "de_world.h"
#include "lightmap_planes.h"
#include "preplane.h"


CPreSurface::CPreSurface()
{
	m_pPlane				= NULL;
	m_pTextureEffect		= NULL;
	m_Ambient[0]			= 0;
	m_Ambient[1]			= 0;
	m_Ambient[2]			= 0;
	m_fLightPenScale		= (PReal)1.0;
	m_bUsed					= FALSE;
	m_LMGridSize			= 0;
}


bool CPreSurface::ShouldLightmap() const
{
	return (m_Flags & SURF_LIGHTMAP) && 
		!(m_Flags & SURF_INVISIBLE) &&
		!(m_Flags & SURF_SKY);
}

void CPreSurface::SetupLMVectors()
{
	ASSERT(m_pPlane);

	SetupLMPlaneVectors(
		SelectLMPlaneVector(m_pPlane->m_Normal),
		m_pPlane->m_Normal,
		P, 
		Q);

	// Setup the inverses..
	InverseP = P / P.Dot(P);
	InverseQ = Q / Q.Dot(Q);
}


