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
#include "WeaponMgr.h"
#include "ModelButeMgr.h"
#include "FXButeMgr.h"
#include "UberAssert.h"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CGlobalMgr::CGlobalMgr()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CGlobalMgr::CGlobalMgr()
{
	m_pFXButeMgr = debug_new(CFXButeMgr);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CGlobalMgr::~CGlobalMgr()
//              
//	PURPOSE:	Call Term BEFORE deleting allocated aggregates, as doing this
//				the other way causes engine crashes on exit.
//              
//----------------------------------------------------------------------------
/*virtual*/ CGlobalMgr::~CGlobalMgr()
{
	Term( );

	if ( m_pFXButeMgr )
	{
		debug_delete( m_pFXButeMgr );
		m_pFXButeMgr = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalMgr::Init()
{
    if (!m_DebrisMgr.Init())
	{
		ShutdownWithError("DebrisMgr", DEBRISMGR_DEFAULT_FILE);
        return LTFALSE;
	}

	UBER_ASSERT( m_pFXButeMgr, "Failed to allocate m_pFXButeMgr" );
    if (!m_pFXButeMgr->Init())
	{
		ShutdownWithError("FXButeMgr", FXBMGR_DEFAULT_FILE);
        return LTFALSE;
	}

    if (!m_SoundFilterMgr.Init())
	{
		ShutdownWithError("SoundFilterMgr", SFM_DEFAULT_FILE);
        return LTFALSE;
	}

	// Get the singleton instance of the weapon mgr and intialize it.
	if( !g_pWeaponMgr )
	{
		CWeaponMgr& weaponMgr = CWeaponMgr::Instance( );
		if (!weaponMgr.Init())
		{
			ShutdownWithError("WeaponMgr", WEAPON_DEFAULT_FILE);
			return LTFALSE;
		}
	}

	//the surfaces need to be initialized before the models, because models contain
	//surfaces
    if (!m_SurfaceMgr.Init())
	{
		ShutdownWithError("SurfaceMgr", SRFMGR_DEFAULT_FILE);
        return LTFALSE;
	}

	// Get the singleton instance of the weapon mgr and intialize it.
	if( !g_pModelButeMgr )
	{
		CModelButeMgr& modelButeMgr = CModelButeMgr::Instance( );
		if (!modelButeMgr.Init())
		{
			ShutdownWithError("ModelButeMgr", MBMGR_DEFAULT_FILE);
			return LTFALSE;
		}
	}

	if (!m_SkillsButeMgr.Init())
	{
		ShutdownWithError("SkillsButeMgr", SMGR_DEFAULT_FILE);
        return LTFALSE;
	}

	if( !m_SoundButeMgr.Init() )
	{
		ShutdownWithError( "SoundButeMgr", SOUND_BUTES_DEFAULT_FILE );
		return LTFALSE;
	}

    if (!m_DTButeMgr.Init())
	{
		ShutdownWithError("DTButeMgr", DTMGR_DEFAULT_FILE);
        return LTFALSE;
	}
	
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalMgr::Term()
//
//	PURPOSE:	Term
//
// ----------------------------------------------------------------------- //

void CGlobalMgr::Term()
{
	m_pFXButeMgr->Term();
	m_DebrisMgr.Term();
	m_SoundFilterMgr.Term();

	// Terminate the weaponmgr singleton.
	if( g_pWeaponMgr )
	{
		g_pWeaponMgr->Term();
	}

	// Terminate the modelbutemgr singleton.
	if( g_pModelButeMgr )
	{
		g_pModelButeMgr->Term();
	}

	m_SurfaceMgr.Term();
	
	m_SkillsButeMgr.Term();
	m_SoundButeMgr.Term();
	m_DTButeMgr.Term();
}