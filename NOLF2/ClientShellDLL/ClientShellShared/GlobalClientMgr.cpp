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
#include "ClientButeMgr.h"
#include "ClientSoundMgr.h"
#include "KeyMgr.h"
#include "SearchItemMgr.h"
#include "PopupMgr.h"
#include "RadarTypeMgr.h"
#include "ActivateTypeMgr.h"
#include "TriggerTypeMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::CGlobalClientMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CGlobalClientMgr::CGlobalClientMgr( )
{
	m_pClientButeMgr = NULL;
	m_pClientSoundMgr = NULL;
	m_pKeyMgr = NULL;
	m_pSearchItemMgr = NULL;
	m_pPopupMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalClientMgr::Init()
{
	char errorBuf[256];
	bool bOk = true;

	// Start fresh.
	Term( );

    if (!CGlobalMgr::Init())
		return LTFALSE;

	m_pClientSoundMgr = debug_new( CClientSoundMgr );
    if( !bOk || !m_pClientSoundMgr || !m_pClientSoundMgr->Init())
	{
		bOk = false;
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize ClientSoundMgr.  Make sure the %s file is valid!", CSNDMGR_DEFAULT_FILE);
	}

	m_pClientButeMgr = debug_new( CClientButeMgr );
	if( !bOk || !m_pClientButeMgr || !m_pClientButeMgr->Init())
	{
		bOk = false;
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize ClientButeMgr.  Make sure the %s file is valid!", CBMGR_DEFAULT_FILE);
	}

	m_pKeyMgr = debug_new( CKeyMgr );
	if( !bOk || !m_pKeyMgr || !m_pKeyMgr->Init())
	{
		bOk = false;
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize CKeyMgr." );
	}

	m_pSearchItemMgr = debug_new( CSearchItemMgr );
	if( !bOk || !m_pSearchItemMgr || !m_pSearchItemMgr->Init())
	{
		bOk = false;
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize CSearchItemMgr." );
	}

	m_pPopupMgr = debug_new( CPopupMgr );
	if( !bOk || !m_pPopupMgr || !m_pPopupMgr->Init())
	{
		bOk = false;
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize CPopupMgr." );
	}

	if( g_pGameClientShell->ShouldUseRadar() )
	{
		// Get the singleton instance of the radar type mgr and initialize it...
		if( !g_pRadarTypeMgr )
		{
			CRadarTypeMgr &RadarTypeMgr = CRadarTypeMgr::Instance();
			if( !RadarTypeMgr.Init() )
			{
				ShutdownWithError( "RadarTypeMgr", RTMGR_DEFAULT_FILE );
				return LTFALSE;
			}
		}
	}
	
	if( !g_pActivateTypeMgr )
	{
		CActivateTypeMgr &ActivateTypeMgr = CActivateTypeMgr::Instance();
		if( !ActivateTypeMgr.Init() )
		{
			ASSERT( !"ERROR in CGlobalClientMgr::Init() Couldn't initialize CActivateTypeMgr." );
			g_pLTClient->CPrint( "ERROR in CGlobalClientMgr::Init()\nCouldn't initialize CActivateTypeMgr.\n\n" );
		}
	}

	if( !g_pTriggerTypeMgr )
	{
		CTriggerTypeMgr &TriggerTypeMgr = CTriggerTypeMgr::Instance();
		if( !TriggerTypeMgr.Init() )
		{
			ASSERT( !"ERROR in CGlobalClienMgr::Init() Couldn't initialize CTriggerTypeMge." );
			g_pLTClient->CPrint( "ERROR in CGlobalClientMgr::Init()\nCouldn't initialize CTriggerTypeMgr.\n\n" );
		}
	}

	if( !bOk )
		g_pLTClient->ShutdownWithMessage(errorBuf);

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
	if( m_pClientButeMgr )
	{
		debug_delete( m_pClientButeMgr );
		m_pClientButeMgr = NULL;
	}

	if( m_pClientSoundMgr )
	{
		debug_delete( m_pClientSoundMgr );
		m_pClientSoundMgr = NULL;
	}

	if( m_pKeyMgr )
	{
		debug_delete( m_pKeyMgr );
		m_pKeyMgr = NULL;
	}

	if( m_pSearchItemMgr )
	{
		debug_delete( m_pSearchItemMgr );
		m_pSearchItemMgr = NULL;
	}

	if( m_pPopupMgr )
	{
		debug_delete( m_pPopupMgr );
		m_pPopupMgr = NULL;
	}

	if( g_pRadarTypeMgr )
	{
		g_pRadarTypeMgr->Term();
	}

	if( g_pActivateTypeMgr )
	{
		g_pActivateTypeMgr->Term();
	}

	if( g_pTriggerTypeMgr )
	{
		g_pTriggerTypeMgr->Term();
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

void CGlobalClientMgr::ShutdownWithError(char* pMgrName, char* pButeFilePath)
{
	char errorBuf[256];
	sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTClient->ShutdownWithMessage(errorBuf);
}