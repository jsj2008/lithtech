//----------------------------------------------------------------------------
//              
//	MODULE:		AIInformationVolumeMgr.cpp
//              
//	PURPOSE:	CAIInformationVolumeMgr implementation
//              
//	CREATED:	18.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIINFORMATIONVOLUMEMGR_H__
#include "AIInformationVolumeMgr.h"		
#endif

#include "AIVolume.h"

// Forward declarations

// Globals

// Statics
						 
CAIInformationVolumeMgr* g_pAIInformationVolumeMgr = LTNULL;

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
//              
//	PURPOSE:	Hooks the g_ppInformationVolumeMgr up to the InformationVolumeMgr
//              
//----------------------------------------------------------------------------
CAIInformationVolumeMgr::CAIInformationVolumeMgr()
{
	g_pAIInformationVolumeMgr = this;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
//              
//	PURPOSE:	Unhooks the g_pAIInformationVolumeMgr
//              
//----------------------------------------------------------------------------
CAIInformationVolumeMgr::~CAIInformationVolumeMgr()
{
	g_pAIInformationVolumeMgr = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIInformationVolumeMgr::Init()
//              
//	PURPOSE:	Inits the base class with the name of the Volumes to be used
//              
//----------------------------------------------------------------------------
void CAIInformationVolumeMgr::Init()
{
	CAISpatialRepresentationMgr::Init( "AIInformationVolume" );
}

AIInformationVolume* CAIInformationVolumeMgr::FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold, AISpatialRepresentation* pVolumeStart, LTBOOL bBruteForce)
{
	return (AIInformationVolume*)CAISpatialRepresentationMgr::FindContainingVolume( hObject, vPos, iAxisMask, fVerticalThreshhold, pVolumeStart, bBruteForce);
}