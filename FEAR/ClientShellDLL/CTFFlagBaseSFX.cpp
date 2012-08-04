// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlagBaseSFX.cpp
//
// PURPOSE : Client side representation on CTFFlagBase
//
// CREATED : 05/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "CTFFlagBaseSFX.h"
#include "CTFFlagSFX.h"
#include "GameModeMgr.h"
#include "StateMachine.h"
#include "CTFDB.h"
#include "TeamClientFXSFX.h"

CTFFlagBaseSFX::CTFFlagBaseSFXList CTFFlagBaseSFX::m_lstCTFFlagBaseSFX;

// Statemachine to handle ctfflagsfx states.
class FlagBaseSFXStateMachine : public MacroStateMachine
{
public:

	FlagBaseSFXStateMachine( )
	{
		m_pFlagBaseSFX = NULL;
		m_delegatePlayerChangedTeamsEvent.Attach( this, g_pInterfaceMgr->GetClientInfoMgr( ), g_pInterfaceMgr->GetClientInfoMgr( )->PlayerChangedTeamsEvent );
	}

	~FlagBaseSFXStateMachine( )
	{
		Term( );
	}

	// Initialize to a flagbase.
	bool Init( CTFFlagBaseSFX& flag )
	{
		m_pFlagBaseSFX = &flag;
		return true;
	}

	void Term( )
	{
		m_delegatePlayerChangedTeamsEvent.Detach();
		CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
		pClientFXMgr->ShutdownClientFX( &m_ClientFxLink );
		m_ClientFxLink.ClearLink();
	}

protected:

	// Statemachine event handlers.
	bool HasFlag_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool HasFlag_OnPlayerChanged( MacroStateMachine::EventParams& eventParams );
	bool NoFlag_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool NoFlag_OnPlayerChanged( MacroStateMachine::EventParams& eventParams );

	enum FlagBaseEvent
	{
		eFlagBaseEvent_PlayerChanged = EVENT_User,
	};

	// State table.
	MSM_BeginTable( FlagBaseSFXStateMachine )
		MSM_BeginState( kCTFFlagBaseState_HasFlag )
			MSM_OnEnter( HasFlag_OnEnter )
			MSM_OnEvent( eFlagBaseEvent_PlayerChanged, HasFlag_OnEnter )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagBaseState_NoFlag )
			MSM_OnEnter( NoFlag_OnEnter )
			MSM_OnEvent( eFlagBaseEvent_PlayerChanged, NoFlag_OnEnter )
		MSM_EndState( )
	MSM_EndTable( )

	// Declare delegate to listen for player kill events.
	static void OnPlayerChangedTeamsEvent( FlagBaseSFXStateMachine* pFlagBaseSFXStateMachine, CClientInfoMgr* pClientInfoMgr, EventCaster::NotifyParams& notifyParams )
	{
		pFlagBaseSFXStateMachine->DoUserEvent( eFlagBaseEvent_PlayerChanged, EventParams( ));
	}
	Delegate< FlagBaseSFXStateMachine, CClientInfoMgr, OnPlayerChangedTeamsEvent > m_delegatePlayerChangedTeamsEvent;

	// Creates a team clientfx.
	void CreateTeamClientFX( HRECORD hTeamClientFxRec );

private:

	// The flag that owns us.
	CTFFlagBaseSFX* m_pFlagBaseSFX;

	CClientFXLink m_ClientFxLink;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagBaseSFXStateMachine::HasFlag_OnEnter
//
//	PURPOSE:	Handle a enter HasFlag state.
//
// ----------------------------------------------------------------------- //
bool FlagBaseSFXStateMachine::HasFlag_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, BaseTeamClientFxHasFlag );
	CreateTeamClientFX( hTeamClientFxRec );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagBaseSFXStateMachine::NoFlag_OnEnter
//
//	PURPOSE:	Handle a enter NoFlag state.
//
// ----------------------------------------------------------------------- //
bool FlagBaseSFXStateMachine::NoFlag_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	HRECORD hFlagBaseRec = m_pFlagBaseSFX->GetCS().m_hFlagBaseRec;
	HRECORD hTeamClientFxRec = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( hFlagBaseRec, BaseTeamClientFxNoFlag );
	CreateTeamClientFX( hTeamClientFxRec );
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	FlagBaseSFXStateMachine::CreateClientFX
//
//  PURPOSE:	Creates the clientfx for the team.
//
// ----------------------------------------------------------------------- //
void FlagBaseSFXStateMachine::CreateTeamClientFX( HRECORD hTeamClientFxRec )
{
	uint8 nTeamId = m_pFlagBaseSFX->GetCS().m_nTeamId;
	TeamClientFXSFX::CreateTeamClientFX( m_pFlagBaseSFX->GetServerObj( ), hTeamClientFxRec, nTeamId, m_ClientFxLink );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::CTFFlagBaseSFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTFFlagBaseSFX::CTFFlagBaseSFX( )
{
	m_pCTFFlagSFX = NULL;
	m_pFlagBaseSFXStateMachine = NULL;

	// Add this instance to a list of all of this type.
	m_lstCTFFlagBaseSFX.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::~CTFFlagBaseSFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTFFlagBaseSFX::~CTFFlagBaseSFX( )
{
	if( m_csCTFFlagBase.m_hFlag && m_pCTFFlagSFX )
	{
		m_pCTFFlagSFX->SetFlagBase( NULL, NULL );
	}

	if( m_pFlagBaseSFXStateMachine )
	{
		delete m_pFlagBaseSFXStateMachine;
		m_pFlagBaseSFXStateMachine = NULL;
	}

	// Erase this instance from the list.
	CTFFlagBaseSFXList::iterator it = std::find( m_lstCTFFlagBaseSFX.begin( ), m_lstCTFFlagBaseSFX.end( ), this );
	if( it != m_lstCTFFlagBaseSFX.end( ))
	{
		m_lstCTFFlagBaseSFX.erase( it );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool CTFFlagBaseSFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	// Make sure there's a ctf rules defined.
	if( !GameModeMgr::Instance( ).GetCTFRulesRecord( ))
		return false;

	m_csCTFFlagBase.Read( pMsg );

	// Make sure flag knows about us.  Because the order of the objects arriving on the client is
	// not easily managed, each interdependent object will set itself on the other.
	m_pCTFFlagSFX = reinterpret_cast< CTFFlagSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAG_ID, m_csCTFFlagBase.m_hFlag ));

	// Make a friendly navmarker for this base.
	NAVMARKERCREATESTRUCT navMarker;
	navMarker.m_bInstant = false;
	navMarker.m_bIsActive = true;
	navMarker.m_hTarget = GetServerObj();
	navMarker.m_nTeamId = m_csCTFFlagBase.m_nTeamId;
	navMarker.m_hType = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( GetCS().m_hFlagBaseRec, FriendlyBaseNavMarker );

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_NAVMARKER_ID );
	navMarker.Write( cMsg );
	g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( GetServerObj(), cMsg.Read());

	// Make an enemy navmarker for this base.
	navMarker.m_nTeamId = !m_csCTFFlagBase.m_nTeamId;
	navMarker.m_hType = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( GetCS().m_hFlagBaseRec, EnemyBaseNavMarker );
	cMsg.Reset();
	cMsg.Writeuint8( SFX_NAVMARKER_ID );
	navMarker.Write( cMsg );
	g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( GetServerObj(), cMsg.Read());

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pFlagBaseSFXStateMachine = new FlagBaseSFXStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pFlagBaseSFXStateMachine->Init( *this );
	if( m_pCTFFlagSFX )
	{
		m_pFlagBaseSFXStateMachine->SetState( m_csCTFFlagBase.m_eCTFFlagBaseState );
		m_pCTFFlagSFX->SetFlagBase( GetServerObj( ), this );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::SetFlag
//
//  PURPOSE:	Sets the flag object.
//
// ----------------------------------------------------------------------- //
void CTFFlagBaseSFX::SetFlag( HOBJECT hObject, CTFFlagSFX* pCTFFlagSFX ) 
{ 
	m_csCTFFlagBase.m_hFlag = hObject; 
	m_pCTFFlagSFX = pCTFFlagSFX;
	m_pFlagBaseSFXStateMachine->SetState( m_csCTFFlagBase.m_eCTFFlagBaseState );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::Update
//
//  PURPOSE:	Update the weapon associated with the turret...
//
// ----------------------------------------------------------------------- //

bool CTFFlagBaseSFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::OnServerMessage
//
//  PURPOSE:	Handle a message recieved from the server side Turret object...
//
// ----------------------------------------------------------------------- //

bool CTFFlagBaseSFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlagBaseSFX::SetHasFlag
//
//  PURPOSE:	Sets whether base has the flag.
//
// ----------------------------------------------------------------------- //
bool CTFFlagBaseSFX::SetHasFlag( bool bValue )
{
	return m_pFlagBaseSFXStateMachine->SetState( bValue ? kCTFFlagBaseState_HasFlag : kCTFFlagBaseState_NoFlag );
}
