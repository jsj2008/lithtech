// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_REGION_MGR_H__
#define __AI_REGION_MGR_H__

#include "AIRegion.h"

class CAIPath;

// Externs

extern class CAIRegionMgr* g_pAIRegionMgr;

// Classes

class CAIRegionMgr
{
	public :

		// Ctors/Dtors/etc

		CAIRegionMgr();
		~CAIRegionMgr();

		void Init();
		void Term();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		// Simple accessors

		int32 GetNumRegions() { return m_cRegions; }
		CAIRegion* GetRegionByIndex(int32 iRegion);
		LTBOOL IsInitialized() { return m_bInitialized; }

	protected :

		LTBOOL		m_bInitialized;
		int32		m_cRegions;
		CAIRegion*	m_aRegions;
};

#endif // __AI_REGION_MGR_H__