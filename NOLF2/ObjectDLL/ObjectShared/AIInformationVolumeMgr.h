//----------------------------------------------------------------------------
//              
//	MODULE:		AIInformationVolumeMgr.h
//              
//	PURPOSE:	AIInformationVolumeMgr declaration
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

#ifndef __AIINFORMATIONVOLUME_H__
#define __AIINFORMATIONVOLUME_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AISpatialRepresentationMgr.h"

// Forward declarations

// Globals

// Statics

class AIInformationVolume;

//----------------------------------------------------------------------------
//              
//	CLASS:		CAIInformationVolumeMgr
//              
//	PURPOSE:	Mgr class for the InformationVolume system.  Used to query the
//				InformationVolume system for volumes/information as well as 	
//              
//----------------------------------------------------------------------------
class CAIInformationVolumeMgr : public CAISpatialRepresentationMgr
{
public:
	// Ctors/Dtors/etc

	CAIInformationVolumeMgr();
	virtual ~CAIInformationVolumeMgr();

	void Init();

	AIInformationVolume* FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold = 0.0f, AISpatialRepresentation* pVolumeStart = LTNULL, LTBOOL bBruteForce = LTTRUE);

protected:
	// Copy Constructor and Asignment Operator private to prevent 
	// automatic generation and inappropriate, unintentional use
	CAIInformationVolumeMgr(const CAIInformationVolumeMgr& rhs) {}
	CAIInformationVolumeMgr& operator=(const CAIInformationVolumeMgr& rhs ) {}
};


#endif // __AIINFORMATIONVOLUMEMGR_H__