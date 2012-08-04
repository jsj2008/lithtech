// ----------------------------------------------------------------------- //
//
// MODULE  : TeamClientFXSFX.cpp
//
// PURPOSE : Client side representation on TeamClientFX
//
// CREATED : 05/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "TeamClientFXSFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::TeamClientFXSFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

TeamClientFXSFX::TeamClientFXSFX( )
{
	m_delegatePlayerChangedTeamsEvent.Attach( this, g_pInterfaceMgr->GetClientInfoMgr( ), g_pInterfaceMgr->GetClientInfoMgr( )->PlayerChangedTeamsEvent );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::~TeamClientFXSFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

TeamClientFXSFX::~TeamClientFXSFX( )
{
	m_delegatePlayerChangedTeamsEvent.Detach();
	ShutdownClientFX( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool TeamClientFXSFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	TEAMCLIENTFXCREATESTRUCT cs;
	cs.Read( pMsg );
	if( cs.m_hParentObject == NULL )
		cs.m_hParentObject = hServObj;
	return Init( &cs );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool TeamClientFXSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if( !CSpecialFX::Init( psfxCreateStruct ))
		return false;

	TEAMCLIENTFXCREATESTRUCT* pTeamClientFxCs = static_cast< TEAMCLIENTFXCREATESTRUCT* >( psfxCreateStruct );
	m_cs = *pTeamClientFxCs;

	CreateClientFX();

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::Update
//
//  PURPOSE:	Update the weapon associated with the turret...
//
// ----------------------------------------------------------------------- //

bool TeamClientFXSFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::OnServerMessage
//
//  PURPOSE:	Handle a message recieved from the server side Turret object...
//
// ----------------------------------------------------------------------- //

bool TeamClientFXSFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::CreateClientFX
//
//  PURPOSE:	Creates the clientfx for the team.
//
// ----------------------------------------------------------------------- //
void TeamClientFXSFX::CreateClientFX( )
{
	CreateTeamClientFX( m_cs.m_hParentObject, m_cs.m_hTeamClientFXRec, m_cs.m_nTeamId, m_ClientFxLink );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TeamClientFXSFX::ShutdownClientFX
//
//  PURPOSE:	Shutdown any ClientFX currently playing...
//
// ----------------------------------------------------------------------- //

void TeamClientFXSFX::ShutdownClientFX( )
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

	pClientFXMgr->ShutdownClientFX( &m_ClientFxLink );
	m_ClientFxLink.ClearLink();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CreateTeamClientFX
//
//  PURPOSE:	Creates the clientfx for the team.
//
// ----------------------------------------------------------------------- //
bool TeamClientFXSFX::CreateTeamClientFX( HOBJECT hParent, HRECORD hTeamClientFxRec, uint8 nTeamId, CClientFXLink& clientFxLink )
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

	// Shutdown the previous clientfx.
	pClientFXMgr->ShutdownClientFX( &clientFxLink );
	clientFxLink.ClearLink();

	// Get the name of the new clientfx.
	char const* pszFXName = NULL;
	if( g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam( nTeamId ))
	{
		pszFXName = DATABASE_CATEGORY( TeamClientFX ).GETRECORDATTRIB( hTeamClientFxRec, FriendlyClientFX );
	}
	else
	{
		pszFXName = DATABASE_CATEGORY( TeamClientFX ).GETRECORDATTRIB( hTeamClientFxRec, EnemyClientFX );
	}

	if( LTStrEmpty( pszFXName ))
	{
		LTERROR_PARAM1( "Invalid fx name specified in TeamClientFX %s", DATABASE_CATEGORY( TeamClientFX ).GetRecordName( hTeamClientFxRec ));
		return false;
	}

	// Create the new clientfx.
	CLIENTFX_CREATESTRUCT fxInit( pszFXName, FXFLAG_LOOP, hParent ); 
	if( !pClientFXMgr->CreateClientFX( &clientFxLink, fxInit, true ))
		return false;

	return true;
}


