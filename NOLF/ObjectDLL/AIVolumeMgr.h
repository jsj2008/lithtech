// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_VOLUME_MGR_H__
#define __AI_VOLUME_MGR_H__

#include "AIVolume.h"

class CAIPath;

// Externs

extern class CAIVolumeMgr* g_pAIVolumeMgr;

// Classes

class CAIVolumeMgr
{
	public :

		// Ctors/Dtors/etc

		CAIVolumeMgr();
		~CAIVolumeMgr();

		void Init();
		void Term();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Methods

        CAIVolume* FindContainingVolume(const LTVector& vPos, LTFLOAT fVerticalThreshhold = 0.0f, CAIVolume* pVolumeStart = LTNULL, LTBOOL bBruteForce = LTTRUE);
		CAIVolumeNeighbor* FindNeighbor(CAIVolume* pVolume, CAIVolume* pVolumeNeighbor);
		LTBOOL FindDangerScatterPosition(CAIVolume* pVolume, const LTVector& vAIPos, const LTVector& vDangerPos, LTFLOAT fDangerDistanceSqr, LTVector* pvScatterPosition, LTBOOL bNeighbor = LTFALSE);

		// Link methods for volumes

		void HandleBrokenLink(HOBJECT hObject);
		void Link(HOBJECT hObject);
		void Unlink(HOBJECT hObject);

		// Simple accessors

		int32 GetNumVolumes() const { return m_cVolumes; }
		CAIVolume* GetVolumeByIndex(int32 iVolume);
		CAIVolume* GetVolumeByName(const char* szVolume);
		LTBOOL IsInitialized() const { return m_bInitialized; }

	protected :

        CAIVolume* FindContainingVolumeBruteForce(const LTVector& vPos, LTFLOAT fVerticalThreshhold);

	protected :

		LTBOOL		m_bInitialized;
		int32		m_cVolumes;
		CAIVolume*	m_aVolumes;
};

#endif // __AI_VOLUME_MGR_H__