// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.h
//
// PURPOSE : Definition of global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_MGR_H__
#define __GLOBAL_MGR_H__

#include "WeaponMgr.h"
#include "ModelButeMgr.h"
#include "SurfaceMgr.h"
#include "MissionMgr.h"

#include "SoundMgr.h"
#include "DebrisMgr.h"
#include "SoundFilterMgr.h"
#include "VersionMgr.h"

class CGlobalMgr
{
	protected :

		CGlobalMgr() {}
		virtual ~CGlobalMgr();

        LTBOOL Init(ILTCSBase *pInterface);

		CFXButeMgr		m_FXButeMgr;		// Same as g_pFXButeMgr
		CDebrisMgr		m_DebrisMgr;		// Same as g_pDebrisMgr
		CSoundFilterMgr	m_SoundFilterMgr;	// Same as g_pSoundFilterMgr

		CWeaponMgr		m_WeaponMgr;		// Same as g_pWeaponMgr
		CModelButeMgr	m_ModelButeMgr;		// Same as g_pModelButeMgr
		CSurfaceMgr		m_SurfaceMgr;		// Same as g_pSurfaceMgr
		CMissionMgr		m_MissionMgr;		// Same as g_pMissionMgr
		CVersionMgr		m_VersionMgr;		// Same as g_pVersionMgr

		virtual void	ShutdownWithError(char* pMgrName, char* pButeFilePath) = 0;
};

#endif // __GLOBAL_MGR_H__