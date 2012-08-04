// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_VOLUME_MGR_H__
#define __AI_VOLUME_MGR_H__

#include "AISpatialRepresentationMgr.h"

extern class CAIVolumeMgr* g_pAIVolumeMgr;

class CAIPath;
class AIVolume;
class AIVolumeNeighbor;

//----------------------------------------------------------------------------
//              
//	CLASS:		CAIVolumeMgr
//              
//	PURPOSE:	AI Pathing Volume Mgr class.  The AIVolumeMgr manages the AI
//				volumes used by the AI for pathfinding.  It specializes the
//				CAISpatialRepresentationMgr, and wraps some of its
//				functionality to provide pointers to/from AIVolumes
//              
//----------------------------------------------------------------------------
class CAIVolumeMgr : public CAISpatialRepresentationMgr
{
public:
	// Ctors/Dtors/etc

	CAIVolumeMgr();
	virtual ~CAIVolumeMgr();

	void Init();

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	LTBOOL StraightRadiusPathExists(CAI* pAI, const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fRadius, LTFLOAT fVerticalThreshold, uint32 dwExcludeVolumes, AIVolume* pStartVolumeHint);
	LTBOOL StraightPathExists(CAI* pAI, const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fVerticalThreshhold, uint32 dwExcludeVolumes, AIVolume* pStartVolumeHint);

	// Functions wrapping functionality supported in the base class,
	// CAISpatialRepresentationMgr.  Wrapping functions expose
	AIVolumeNeighbor* FindNeighbor(CAI* pAI, AIVolume* pVolume, AIVolume* pVolumeNeighbor);
	AIVolume* FindContainingVolumeBruteForce(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold);
	AIVolume* FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold = 0.0f, AISpatialRepresentation* pVolumeStart = LTNULL, LTBOOL bBruteForce = LTTRUE);
	AIVolume* FindNearestIntersectingVolume(const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fWidth, LTFLOAT fVerticalThreshhold, LTVector* pvIntersection);

	AIVolume* GetVolume(uint32 iVolume);
	AIVolume* GetVolume(const char* szVolume);

	// Methods
};


#endif // __AI_VOLUME_MGR_H__