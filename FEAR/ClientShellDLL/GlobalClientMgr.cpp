// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalClientMgr.cpp
//
// PURPOSE : Implementations of client global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalClientMgr.h"
#include "ClientSoundMgr.h"
#include "ClientDB.h"
#include "PerformanceDB.h"
#include "PerformanceMgr.h"
#include "sys/win/mpstrconv.h"
#include "CharacterDisplay.h"
#include "WeaponDisplay.h"
#include "CursorsDB.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::CGlobalClientMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CGlobalClientMgr::CGlobalClientMgr( )
{
	m_pClientSoundMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

bool CGlobalClientMgr::Init()
{
	wchar_t errorBuf[256];
	bool bOk = true;

    if (!CGlobalMgr::Init(CErrorHandler()))
		return false;

	if( !ClientDB::Instance().Init())
		return false;

	if (!DATABASE_CATEGORY( PerformanceGlobal ).Init() )
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize PerformanceDB." ));
		bOk = false;			
	}
	if (!DATABASE_CATEGORY( PerformanceGroup ).Init() )
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize PerformanceDB." ));
		bOk =  false;			
	}
	if (!DATABASE_CATEGORY( PerformanceOption ).Init() )
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize PerformanceDB." ));
		bOk =  false;			
	}

	if( !DATABASE_CATEGORY( CharacterDisplayDB ).Init( ))
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "Couldn't initialize CharacterDisplayDB." ));
		return false;
	}
	if( !DATABASE_CATEGORY( CharacterDisplayLayoutDB ).Init( DB_Default_Localized_File ))
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "Couldn't initialize CharacterDisplayLayoutDB." ));
		return false;
	}

	if( !DATABASE_CATEGORY( WeaponDisplayDB ).Init( ))
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "Couldn't initialize WeaponDisplayDB." ));
		return false;
	}
	if( !DATABASE_CATEGORY( WeaponDisplayLayoutDB ).Init( DB_Default_Localized_File ))
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "Couldn't initialize WeaponDisplayLayoutDB." ));
		return false;
	}
	if( !DATABASE_CATEGORY( Cursors ).Init( DB_Default_Localized_File ))
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "Couldn't initialize Cursors database." ));
		return false;
	}


	if (!CPerformanceMgr::Instance().Init()) 
	{
		LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CPerformanceMgr::Init():  Could not initialize PerformanceMgr!" ));
		bOk =  false;
	}


	if( bOk )
	{
		m_pClientSoundMgr = debug_new( CClientSoundMgr );
		if( !m_pClientSoundMgr || !m_pClientSoundMgr->Init())
		{
			bOk = false;
			LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize ClientSoundMgr." ));
		}
	}

	if( !bOk )
	{
		Term();
		g_pLTClient->ShutdownWithMessage(errorBuf);
	}

    return bOk;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::~CGlobalClientMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGlobalClientMgr::~CGlobalClientMgr()
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::Term()
//
//	PURPOSE:	Terminates object.
//
// ----------------------------------------------------------------------- //

void CGlobalClientMgr::Term()
{
	if( m_pClientSoundMgr )
	{
		debug_delete( m_pClientSoundMgr );
		m_pClientSoundMgr = NULL;
	}

	CGlobalMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::ShutdownWithError()
//
//	PURPOSE:	Shutdown the client with an error
//
// ----------------------------------------------------------------------- //

void CGlobalClientMgr::CErrorHandler::ShutdownWithError(char* pMgrName, const char* pButeFilePath) const
{
	wchar_t errorBuf[256];
	LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), LT_WCHAR_T( "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!" ), MPA2W(pMgrName).c_str(), MPA2W(pButeFilePath).c_str());
    g_pLTClient->ShutdownWithMessage(errorBuf);
}