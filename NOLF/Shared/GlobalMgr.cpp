// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalMgr.cpp
//
// PURPOSE : Implementations of global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalMgr::Init(ILTCSBase *pInterface)
{
    if (!m_DebrisMgr.Init(pInterface))
	{
		ShutdownWithError("DebrisMgr", DEBRISMGR_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_FXButeMgr.Init(pInterface))
	{
		ShutdownWithError("FXButeMgr", FXBMGR_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_SoundFilterMgr.Init(pInterface))
	{
		ShutdownWithError("SoundFilterMgr", SFM_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_WeaponMgr.Init(pInterface))
	{
		ShutdownWithError("WeaponMgr", WEAPON_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_ModelButeMgr.Init(pInterface))
	{
		ShutdownWithError("ModelButeMgr", MBMGR_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_SurfaceMgr.Init(pInterface))
	{
		ShutdownWithError("SurfaceMgr", SRFMGR_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_MissionMgr.Init(pInterface))
	{
		ShutdownWithError("MissionMgr", MISSION_DEFAULT_FILE);
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::~CGlobalMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGlobalMgr::~CGlobalMgr()
{
	m_FXButeMgr.Term();
	m_DebrisMgr.Term();
	m_SoundFilterMgr.Term();
	m_WeaponMgr.Term();
	m_ModelButeMgr.Term();
	m_SurfaceMgr.Term();
	m_MissionMgr.Term();
}