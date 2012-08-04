//----------------------------------------------------------------------------
//              
//	MODULE:		AISpatialRepresentationMgr.h
//              
//	PURPOSE:	CAISpatialRepresentationMgr declaration
//              
//	CREATED:	19.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AISPATIALREPRESENTATIONMGR_H__
#define __AISPATIALREPRESENTATIONMGR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes

// Forward declarations

// Globals

// Statics

class AISpatialNeighbor;
class AISpatialRepresentation;
class CAI;

//----------------------------------------------------------------------------
//              
//	CLASS:		CAISpatialRepresentationMgr
//              
//	PURPOSE:	Base class for VolumeMgrs.  Supports the underlying
//				functionality of a Volume system.  Any potentially general
//				functionality which multiple Volume Mgr systems may use
//				belongs in this class.  The specializaing VolumeMgrs can
//				selectively expose any functionality desired.
//              
//----------------------------------------------------------------------------
class CAISpatialRepresentationMgr
{
public:
		enum
		{
			kMaxNeighbors = 16,
		};

public:
	typedef std::vector<AISpatialRepresentation*> _listVolume;
	typedef std::vector<AISpatialRepresentation*>::iterator _VolumeIterator;

	// Ctors/Dtors/etc
	CAISpatialRepresentationMgr();
	virtual ~CAISpatialRepresentationMgr();

	void	Init(const char* const szName);
	int		SetupVolumesNeighbors( AISpatialRepresentation* pVolume );

	void Term();

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Methods
	AISpatialRepresentation* FindContainingVolume(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold = 0.0f, AISpatialRepresentation* pVolumeStart = LTNULL, LTBOOL bBruteForce = LTTRUE);
	AISpatialNeighbor* FindNeighbor(CAI* pAI, AISpatialRepresentation* pVolume, AISpatialRepresentation* pVolumeNeighbor);
	LTBOOL FindDangerScatterPosition(AISpatialRepresentation* pVolume, const LTVector& vAIPos, const LTVector& vDangerPos, LTFLOAT fDangerDistanceSqr, LTVector* pvScatterPosition, LTBOOL bNeighbor = LTFALSE);
	AISpatialRepresentation* FindNearestIntersectingVolume(const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fWidth, LTFLOAT fVerticalThreshhold, LTVector* pvIntersection);

	// Simple accessors
	LTBOOL	IsInitialized() const { return m_bInitialized; }
	uint32	GetNumVolumes() const { return m_listpVolumes.size(); }

	AISpatialRepresentation* GetVolume(uint32 iVolume);
	AISpatialRepresentation* GetVolume(const char* szVolume);

	_listVolume* GetContainer() { return &m_listpVolumes; }

	// Debugging
	void	UpdateDebugRendering(LTFLOAT fVarTrack);
	void	DrawVolumes();
	void	HideVolumes();

protected:
	
	LTBOOL	m_bDrawingVolumes;

	// Implementation
	AISpatialRepresentation*	FindContainingVolumeBruteForce(HOBJECT hObject, const LTVector& vPos, int iAxisMask, LTFLOAT fVerticalThreshhold);
	LTBOOL	RayIntersectVolume(AISpatialRepresentation* pVolume, const LTVector& vOrigin, const LTVector& vDest, LTFLOAT fVerticalThreshhold, LTVector* pvIntersection);
	int		CountInstances(const char* const szClass) const;
	void	SetupInstanceArray(const char* const szClass);

private:
	LTBOOL		m_bInitialized;

	_listVolume m_listpVolumes;
};

#endif // __AISPATIALREPRESENTATIONMGR_H__

