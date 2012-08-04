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

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Simple accessors

		uint32 GetNumRegions() { return m_cRegions; }
		LTBOOL IsInitialized() { return m_bInitialized; }

	protected :

		LTBOOL		m_bInitialized;
		uint32		m_cRegions;
		AIRegion**	m_apRegions;
};

#endif // __AI_REGION_MGR_H__