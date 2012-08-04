// ----------------------------------------------------------------------- //
//
// MODULE  : RadarObjectFX.h
//
// PURPOSE : RadarObject special fx class - Definition
//
// CREATED : 6/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "RadarObjectFX.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::CRadarObjectFX
//
//	PURPOSE:	ctor
//
// ----------------------------------------------------------------------- //

CRadarObjectFX::CRadarObjectFX( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::~CRadarObjectFX
//
//	PURPOSE:	dtor
//
// ----------------------------------------------------------------------- //

CRadarObjectFX::~CRadarObjectFX( )
{
	g_pRadar->RemoveObject( m_hServerObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RadarObjectFX::Init
//
//	PURPOSE:	Init the RadarObject fx
//
// ----------------------------------------------------------------------- //

LTBOOL CRadarObjectFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !hServObj ) return LTFALSE;
	if( !CSpecialFX::Init( hServObj, pMsg )) return LTFALSE;
	
	if( !g_pGameClientShell->ShouldUseRadar() )
		return LTFALSE;

	m_cs.hServerObj = hServObj;
	m_cs.Read( pMsg );

	// If the object is supposed to start on add it to the radar immediately...

	if( m_cs.bOn )
	{
		g_pRadar->AddObject( m_hServerObject, m_cs.nRadarTypeId, m_cs.nTeamId );
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarObjectFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CRadarObjectFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg )) return LTFALSE;

	m_cs.Read( pMsg );

	// See if we should add the object to the radar or remove it...
	
	if( m_cs.bOn )
	{
		g_pRadar->RemoveObject( m_hServerObject );
		g_pRadar->AddObject( m_hServerObject, m_cs.nRadarTypeId, m_cs.nTeamId );
	}
	else
	{
		g_pRadar->RemoveObject( m_hServerObject );
	}

	return LTTRUE;
}

