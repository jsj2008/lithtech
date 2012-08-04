// ----------------------------------------------------------------------- //
//
// MODULE  : ControlPoint.cpp
//
// PURPOSE : ControlPoint object to place in Control Point level.
//
// CREATED : 02/09/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "ControlPoint.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "PropsDB.h"
#include "GameModeMgr.h"
#include "StateMachine.h"
#include "TeamMgr.h"
#include "SFXMsgIds.h"
#include "ServerConnectionMgr.h"
#include "ServerMissionMgr.h"
#include "ltintersect.h"
#include "GameStartPointMgr.h"
#include "ServerMissionMgr.h"
#include "SurfaceFlagsOverrideHelpers.h"
#include "CollisionsDB.h"
#include "SurfaceFunctions.h"

LINKFROM_MODULE( ControlPoint )

class ControlPoint;

// Tracks which control point id's have been used.
uint8 ControlPoint::m_nAllocatedControlPointIds = 0;

// Control level value when team 0 has 100% control.
static const float kControlLevel_Team0 = -1.0f;
// Control level value when Neutral has 100% control.
static const float kControlLevel_Neutral = 0.0f;
// Control level value when team 1 has 100% control.
static const float kControlLevel_Team1 = 1.0f;
// Time period to send the control level to client.
static const float kControlPoint_ControlLevelSendPeriod = 0.25f;

// List of existing control point objects.
ControlPoint::TControlPointList ControlPoint::m_lstControlPoints;


// Statemachine to handle controlpoint states.
class ControlPointStateMachine : public MacroStateMachine
{
public:

	ControlPointStateMachine( );

	~ControlPointStateMachine( )
	{
		Term( );
	}

	// Initialize to a ControlPoint.
	bool Init( ControlPoint& controPoint )
	{
		m_pControlPoint = &controPoint;

		// Listen for player kill events.
		m_delegatePlayerKillEvent.Attach( this, NULL, CPlayerObj::PlayerScoredKillEvent );

		m_lstCharactersInZone.resize( 0 );
		m_nCharactersInZoneDiff = 0;

		// Add a gravity source object.
		if( DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPoint->m_hControlPointRec, AttractSpawns ))
		{
			GameStartPointMgr::GravitySource gravitySource;
			gravitySource.m_eTeamId = INVALID_TEAM;
			gravitySource.m_hObject = m_pControlPoint->m_hObject;
			GameStartPointMgr::Instance().AddManagedGravitySource( gravitySource );
		}

		return true;
	}

	void Term( )
	{
		// No longer listen for player kill events.
		m_delegatePlayerKillEvent.Detach();

		// Remove our managed gravitysource object.
		if( m_pControlPoint && DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPoint->m_hControlPointRec, AttractSpawns ))
		{
			GameStartPointMgr::Instance().RemoveManagedGravitySource( m_pControlPoint->m_hObject );
		}
	}

	// Send touch event to state.
	bool DoTouchEvent( HOBJECT hToucher )
	{
		ControlPointEventParams eventParams;
		eventParams.m_hObject = hToucher;
		return DoUserEvent( eControlPointEvent_Touched, eventParams );
	}

	// Send player kill event to state.
	bool DoPlayerKillEvent( HOBJECT hKiller, HOBJECT hVictim )
	{
		PlayerKillEventParams eventParams;
		eventParams.m_hKiller = hKiller;
		eventParams.m_hVictim = hVictim;
		return DoUserEvent( eControlPointEvent_PlayerKilled, eventParams );
	}

	// Gets the team that owns the controlpoint.
	ETeamId GetTeamId( ) const;

	// override of MacroStateMachine.
	using MacroStateMachine::SetState;
	virtual bool SetState( uint32 nNewState, ILTMessage_Write* pMsg );

protected:

	// Statemachine event handlers.
	bool Team0_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Team0_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Team0_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Team1_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Team1_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Team1_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Neutral_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Neutral_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Neutral_OnExit( MacroStateMachine::EventParams& eventParams );

	// Generalized functions for event handlers.
	bool TeamX_OnEnter( float fControlLevel, ETeamId eOwningTeamId, char const* pszTeamCapturedControlCmd );
	bool TeamX_OnExit( ETeamId eOwningTeamId );
	bool TeamX_OnPlayerKill( MacroStateMachine::EventParams& eventParams );
	bool OnTouched( MacroStateMachine::EventParams& eventParams );

	// ControlPoint event paramaters.
	struct ControlPointEventParams : public EventParams
	{
		ControlPointEventParams( )
		{
			m_hObject = NULL;
		}

		HOBJECT m_hObject;
	};

	// Params sent with player kill event.
	struct PlayerKillEventParams : public EventParams
	{
		PlayerKillEventParams( )
		{
			m_hKiller = NULL;
			m_hVictim = NULL;
		}

		HOBJECT m_hKiller;
		HOBJECT m_hVictim;
	};

	// Statemachine events for ControlPoint.
	enum EControlPointEvent
	{
		// Player killed event.
		eControlPointEvent_PlayerKilled = EVENT_User,
		// ControlPoint touched.
		eControlPointEvent_Touched,
		// ControlPoint capture.
		eControlPointEvent_Capture,
	};

	// State table.
	MSM_BeginTable( ControlPointStateMachine )
		MSM_BeginState( kControlPointState_Team0 )
			MSM_OnEnter( Team0_OnEnter )
			MSM_OnUpdate( Team0_OnUpdate )
			MSM_OnEvent( eControlPointEvent_PlayerKilled, TeamX_OnPlayerKill )
			MSM_OnEvent( eControlPointEvent_Touched, OnTouched )
			MSM_OnExit( Team0_OnExit )
		MSM_EndState( )
		MSM_BeginState( kControlPointState_Team1 )
			MSM_OnEnter( Team1_OnEnter )
			MSM_OnUpdate( Team1_OnUpdate )
			MSM_OnEvent( eControlPointEvent_PlayerKilled, TeamX_OnPlayerKill )
			MSM_OnEvent( eControlPointEvent_Touched, OnTouched )
			MSM_OnExit( Team1_OnExit )
		MSM_EndState( )
		MSM_BeginState( kControlPointState_Neutral )
			MSM_OnEnter( Neutral_OnEnter )
			MSM_OnUpdate( Neutral_OnUpdate )
			MSM_OnEvent( eControlPointEvent_Touched, OnTouched )
			MSM_OnExit( Neutral_OnExit )
		MSM_EndState( )
	MSM_EndTable( )

	// Declare delegate to listen for player kill events.
	static void OnPlayerKillEvent( ControlPointStateMachine* pControlPointStateMachine, CPlayerObj* pPlayerObj, EventCaster::NotifyParams& notifyParams )
	{
		CPlayerObj::PlayerScoredKillEventParams& playerScoredKillNotifyParams = ( CPlayerObj::PlayerScoredKillEventParams& )notifyParams;
		pControlPointStateMachine->DoPlayerKillEvent( playerScoredKillNotifyParams.m_hKiller, playerScoredKillNotifyParams.m_hVictim );
	}
	Delegate< ControlPointStateMachine, CPlayerObj, ControlPointStateMachine::OnPlayerKillEvent > m_delegatePlayerKillEvent;

	// Cleans up the character in zone list to only contain valid characters.
	void CleanCharactersInZoneList( );

	// Record the work done by a team influencing control.
	void RecordTeamWork( uint8 nTeamId, float fWorkTime );

	// Record the work done by a character influencing control.
	void RecordCharacterWork( CCharacter& character, float fWorkTime );

	// Give points to team.
	void AwardTeamPoints( int32 nPoints, uint8 nTeamId );

	// Give the contributing characters the work award.
	void AwardCharacterWorkPoints( EControlPointFXMsgId eControlPointFXMsgId, int32 nPlayerPoints, uint8 nTeamId );

	// Adds character influence to timed captures.
	bool AddCharacterInZone( HOBJECT hCharacter );

	// Processes the inzone list.
	bool ProcessCharactersInZone( );

	// Check for instant capture.
	bool CheckInstantCapture( HOBJECT hObject );

	// Sends the client an event message.
	void SendClientEventMessage( HCLIENT hClient, uint32 nState, ILTMessage_Write* pEventMsg, bool bGuaranteed );

	// Send the control level to the client.
	void SendControlLevel( bool bGuaranteed );

	// Check if we are getting points for owning the CP.
	void UpdateOwnershipPoints( ETeamId eOwningTeamId );

	// Data type to track the history of the character within the zone.
	struct CharacterWorkHistory
	{
		CharacterWorkHistory( )
		{
			m_fTimeCapturing = 0.0f;
		}

		// Character reference.
		LTObjRef	m_hCharacter;
		// Time spent doing work to capture cp.
		float		m_fTimeCapturing;
	};

private:

	// The controlpoint that owns us.
	ControlPoint* m_pControlPoint;

	// List of characters that are within the zonedims.
	ObjRefVector m_lstCharactersInZone;
	// Effective number of characters, taking into account multiple teams in the zone together.
	int32 m_nCharactersInZoneDiff;

	// Data used to track character's history in zone.
	typedef std::vector< CharacterWorkHistory > TCharacterWorkHistoryList;
	TCharacterWorkHistoryList m_lstCharacterWorkHistory;

	// Timer to track when to send control level updates.
	StopWatchTimer m_tmrControlLevelUpdate;

	// Timer to track when to award team points for owning CP.
	StopWatchTimer m_tmrOwningControlPointScoring;

	// Control level.  Varies from [-1,+1] based on controlling team.
	float m_fControlLevel;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::ControlPointStateMachine
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
ControlPointStateMachine::ControlPointStateMachine( )
{
	m_pControlPoint = NULL;
	m_fControlLevel = 0.0f;
	RealTimeTimer& realTimeTimer = RealTimeTimer::Instance( );
	m_tmrControlLevelUpdate.SetEngineTimer( realTimeTimer );
	m_tmrOwningControlPointScoring.SetEngineTimer( realTimeTimer );
	m_nCharactersInZoneDiff = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::SendClientEventMessage
//
//	PURPOSE:	Sends event message to client.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::SendClientEventMessage( HCLIENT hClient, uint32 nState, ILTMessage_Write* pEventMsg, bool bGuaranteed )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CONTROLPOINT_ID );
	cMsg.WriteObject( m_pControlPoint->m_hObject );
	cMsg.WriteBits( nState, FNumBitsExclusive<kControlPointState_NumStates>::k_nValue );
	// If there are any event data to go with it write it out here.
	if( pEventMsg )
	{
		cMsg.Writebool( true );
		cMsg.WriteMessageRaw( pEventMsg->Read( ));
	}
	else
	{
		cMsg.Writebool( false );
	}
	g_pLTServer->SendToClient( cMsg.Read( ), hClient, bGuaranteed ? MESSAGE_GUARANTEED : 0 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::SetState
//
//	PURPOSE:	Override of MacroStateMachine.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::SetState( uint32 nNewState, ILTMessage_Write* pEventMsg )
{
	// Let parent handle it first.
	if( !MacroStateMachine::SetState( nNewState ))
		return false;

	// Send the event message to client.
	SendClientEventMessage( NULL, nNewState, pEventMsg, true );

	// Update the main sfx message.
	m_pControlPoint->CreateSpecialFX( false );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::GetTeamId
//
//	PURPOSE:	Gets the team that owns the control point.
//
// ----------------------------------------------------------------------- //
inline ETeamId ControlPointStateMachine::GetTeamId( ) const
{
	if( GetState( ) == kControlPointState_Team0 )
		return kTeamId0;
	else if( GetState( ) == kControlPointState_Team1 )
		return kTeamId1;
	else
		return INVALID_TEAM;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team0_OnEnter
//
//	PURPOSE:	Handle a enter Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team0_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	return TeamX_OnEnter( kControlLevel_Team0, kTeamId0, m_pControlPoint->m_pszTeam0CapturedControlCmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team0_OnUpdate
//
//	PURPOSE:	Handle a update Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team0_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Process the inzone list.
	ProcessCharactersInZone( );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// See if we have been neutralized.
	if( m_fControlLevel >= kControlLevel_Neutral )
	{
		// Go to the neutralized state.
		SetState( kControlPointState_Neutral, NULL );
		return true;
	}

	// Give any fixup points for regaining full control.
	if( m_fControlLevel == kControlLevel_Team0 && m_lstCharacterWorkHistory.size( ) > 0 )
	{
		AwardCharacterWorkPoints( kControlPointFXMsg_PlayerFixup, gameModeMgr.m_grnCPScoreCapturePlayer, kTeamId0 );
	}

	// Update our ownership points.
	UpdateOwnershipPoints( kTeamId0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team0_OnExit
//
//	PURPOSE:	Handle a exit Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team0_OnExit( MacroStateMachine::EventParams& eventParams )
{
	TeamX_OnExit( kTeamId0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team1_OnEnter
//
//	PURPOSE:	Handle a enter Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team1_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	return TeamX_OnEnter( kControlLevel_Team1, kTeamId1, m_pControlPoint->m_pszTeam1CapturedControlCmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team1_OnUpdate
//
//	PURPOSE:	Handle a update Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team1_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Process the inzone list.
	ProcessCharactersInZone( );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// See if we have been neutralized.
	if( m_fControlLevel <= kControlLevel_Neutral )
	{
		// Go to the neutralized state.
		SetState( kControlPointState_Neutral, NULL );
		return true;
	}

	// Give any fixup points for regaining full control.
	if( m_fControlLevel == kControlLevel_Team1 && m_lstCharacterWorkHistory.size( ) > 0 )
	{
		AwardCharacterWorkPoints( kControlPointFXMsg_PlayerFixup, gameModeMgr.m_grnCPScoreCapturePlayer, kTeamId1 );
	}

	// Update our ownership points.
	UpdateOwnershipPoints( kTeamId1 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Team1_OnExit
//
//	PURPOSE:	Handle a exit Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Team1_OnExit( MacroStateMachine::EventParams& eventParams )
{
	TeamX_OnExit( kTeamId1 );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::TeamX_OnEnter
//
//	PURPOSE:	Handle a enter Team0/Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::TeamX_OnEnter( float fControlLevel, ETeamId eOwningTeamId, char const* pszTeamCapturedControlCmd )
{
	// Don't allow caps after end round condition met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet( ))
		return false;

	// Control level is fully in control when first entering state.
	m_fControlLevel = fControlLevel;

	// Don't need to send period control updates if we're fully controlled.
	m_tmrControlLevelUpdate.Stop( );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Give the team the capture points.
	AwardTeamPoints( gameModeMgr.m_grnCPScoreCaptureTeam, eOwningTeamId );

	// Give the contributing characters the work award.
	AwardCharacterWorkPoints( kControlPointFXMsg_PlayerCapture, gameModeMgr.m_grnCPScoreCapturePlayer, eOwningTeamId );

	// Check if we need to award points to the team for owning the CP.
	float fOwnedScorePeriod = gameModeMgr.m_grfCPOwnedScorePeriod;
	m_tmrOwningControlPointScoring.Stop();
	if( fOwnedScorePeriod > 0.0f )
	{
		uint32 nOwnedScoreAmountTeam = gameModeMgr.m_grnCPOwnedScoreAmountTeam;
		if( nOwnedScoreAmountTeam )
		{
			m_tmrOwningControlPointScoring.Start( fOwnedScorePeriod );

			// Tell our parent object to update.
			m_pControlPoint->SetNextUpdate( UPDATE_NEXT_FRAME );
		}
	}

	// Update the spawnpoint gravity.
	if( DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPoint->m_hControlPointRec, AttractSpawns ))
	{
		GameStartPointMgr::GravitySource* pGravitySource = GameStartPointMgr::Instance().GetManagedGravitySource( m_pControlPoint->m_hObject );
		if( pGravitySource )
		{
			pGravitySource->m_eTeamId = GetTeamId();
		}
	}

	// Send the command that team 0 captures the CP.
	if( !LTStrEmpty( pszTeamCapturedControlCmd ))
	{
		g_pCmdMgr->QueueCommand( pszTeamCapturedControlCmd, m_pControlPoint->m_hObject, m_pControlPoint->m_hObject );
	}

	// Checks to see if conquest has been achieved.
	ControlPoint::CheckForConquestWin( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::TeamX_OnExit
//
//	PURPOSE:	Handle a exit Team0/1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::TeamX_OnExit( ETeamId eOwningTeamId )
{
	// Give the losing team the lost points.
	AwardTeamPoints( -(( int32 )GameModeMgr::Instance().m_grnCPScoreLoseTeam ), eOwningTeamId );

	// No longer getting points for owning.
	m_tmrOwningControlPointScoring.Stop( );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::TeamX_OnPlayerKill
//
//	PURPOSE:	Handle a playerkilled event for Team0 or Team1 states.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::TeamX_OnPlayerKill( MacroStateMachine::EventParams& eventParams )
{
#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: Entry", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

	PlayerKillEventParams& playerKilledEventParams = ( PlayerKillEventParams& )eventParams;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Make sure they can get points for this kill
	int32 nDefendScorePlayer = gameModeMgr.m_grnCPDefendScorePlayer;
	int32 nDefendScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCPDefendScoreTeam : 0;
	if( !nDefendScorePlayer && nDefendScoreTeam )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No defend score", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
	int32 nDefendRadius = gameModeMgr.m_grnCPDefendRadius;
	if( !nDefendRadius )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No defend radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Only care about killers that are on our team.
	CPlayerObj* pKillerPlayerObj = CPlayerObj::DynamicCast( playerKilledEventParams.m_hKiller );
	if( !pKillerPlayerObj )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: no killer object", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		g_pLTBase->CPrint( "%s: pKillerGameClientData->GetUniqueName(%s), pKillerPlayerObj->GetTeamID(%d)", __FUNCTION__, 
			MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), pKillerPlayerObj->GetTeamID());
	}
#endif // DEBUG_DEFEND_MESSAGES
	if( pKillerPlayerObj->GetTeamID() != GetTeamId( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: killer on different team than controlpoint", __FUNCTION__);
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Only care about victims that are on the other team.
	CPlayerObj* pVictimPlayerObj = CPlayerObj::DynamicCast( playerKilledEventParams.m_hVictim );
	if( !pVictimPlayerObj )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: no victim object", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
		g_pLTBase->CPrint( "%s: pVictimGameClientData->GetUniqueName(%s), pVictimPlayerObj->GetTeamID(%d)", __FUNCTION__, 
			MPW2A( pVictimGameClientData->GetUniqueName( )).c_str(), pVictimPlayerObj->GetTeamID());
	}
#endif // DEBUG_DEFEND_MESSAGES
	if( pVictimPlayerObj->GetTeamID() == GetTeamId( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: victim on same team as controlpoint", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Get the controlpoint position.
	LTVector vControlPointPos;
	g_pLTServer->GetObjectPos( m_pControlPoint->m_hObject, &vControlPointPos );

	// Get the position of the victim.
	LTVector vVictimPos;
	g_pLTServer->GetObjectPos( pVictimPlayerObj->m_hObject, &vVictimPos );

#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: vVictimPos(%.3f,%.3f,%.3f), vControlPointPos(%.3f,%.3f,%.3f),dist(%.3f), nDefendRadius(%d)", __FUNCTION__,
		VEC_EXPAND( vVictimPos ), VEC_EXPAND( vControlPointPos ), vControlPointPos.Dist( vVictimPos ), nDefendRadius );
#endif // DEBUG_DEFEND_MESSAGES

	// See if the victim is beyond radius.
	if( vControlPointPos.DistSqr( vVictimPos ) > nDefendRadius * nDefendRadius )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: victim beyond radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
		g_pLTBase->CPrint( "%s: %s defended his controlpoint killing %s", __FUNCTION__, 
			MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), MPW2A( pVictimGameClientData->GetUniqueName( )).c_str());
	}
#endif // DEBUG_DEFEND_MESSAGES

	// Give the killer some objective points.
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
	if( nDefendScorePlayer )
	{
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nDefendScorePlayer );
	}
	if( nDefendScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pKillerPlayerObj->GetTeamID(), nDefendScoreTeam );
	}

	if( nDefendScorePlayer || nDefendScoreTeam )
	{
		// Prepare the event message.
		CAutoMessage cEventMsg;
		cEventMsg.WriteBits( kControlPointFXMsg_PlayerControlPointDefend, FNumBitsExclusive<kControlPointFXMsg_NumIds>::k_nValue );
		HCLIENT hKillerClient = pKillerPlayerObj->GetClient();
		uint8 nKillerClientId = ( uint8 )g_pLTServer->GetClientID( hKillerClient );
		cEventMsg.Writeuint8( nKillerClientId );

		// Send event message.
		SendClientEventMessage( pGameClientData->GetClient(), GetState( ), cEventMsg, true );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::OnTouched
//
//	PURPOSE:	Handle a touched event for Team0/1/Neutral.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::OnTouched( MacroStateMachine::EventParams& eventParams )
{
	ControlPointEventParams& controlPointEventParams = static_cast< ControlPointEventParams& >( eventParams );

	// Add the character to the zone if capturing takes time.
	if( GameModeMgr::Instance().m_grfCPCapturingTime > 0.0f )
	{
		AddCharacterInZone( controlPointEventParams.m_hObject );
	}
	// Do instant capturing.
	else
	{
		CheckInstantCapture( controlPointEventParams.m_hObject );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Neutral_OnEnter
//
//	PURPOSE:	Handle a enter Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Neutral_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Don't allow neutral after end round condition met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet( ))
		return false;

	// Make sure the control level is clamped within range.
	m_fControlLevel = LTCLAMP( m_fControlLevel, kControlLevel_Team0, kControlLevel_Team1 );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Check if we've been neutralized from the Team0 state.
	if(( EControlPointState )eventParams.m_nPreviousState == kControlPointState_Team0 )
	{
		// Give the neutralizing team the neutralize points.
		AwardTeamPoints( gameModeMgr.m_grnCPScoreNeutralizeTeam, kTeamId1 );

		// Give the contributing characters the work award.
		AwardCharacterWorkPoints( kControlPointFXMsg_PlayerNeutralize, gameModeMgr.m_grnCPScoreNeutralizePlayer, kTeamId1 );

		// Send the command that team 1 neutralized the CP.
		if( !LTStrEmpty( m_pControlPoint->m_pszTeam1NeutralizedControlCmd ))
		{
			g_pCmdMgr->QueueCommand( m_pControlPoint->m_pszTeam1NeutralizedControlCmd, m_pControlPoint->m_hObject, m_pControlPoint->m_hObject );
		}
	}
	// Check if we've been neutralized from the Team1 state.
	else if(( EControlPointState )eventParams.m_nPreviousState == kControlPointState_Team1 )
	{
		// Give the neutralizing team the neutralize points.
		AwardTeamPoints( gameModeMgr.m_grnCPScoreNeutralizeTeam, kTeamId0 );

		// Give the contributing characters the work award.
		AwardCharacterWorkPoints( kControlPointFXMsg_PlayerNeutralize, gameModeMgr.m_grnCPScoreNeutralizePlayer, kTeamId0 );

		// Send the command that team 0 neutralized the CP.
		if( !LTStrEmpty( m_pControlPoint->m_pszTeam0NeutralizedControlCmd ))
		{
			g_pCmdMgr->QueueCommand( m_pControlPoint->m_pszTeam0NeutralizedControlCmd, m_pControlPoint->m_hObject, m_pControlPoint->m_hObject );
		}
	}

	// Update the spawnpoint gravity.
	if( DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPoint->m_hControlPointRec, AttractSpawns ))
	{
		GameStartPointMgr::GravitySource* pGravitySource = GameStartPointMgr::Instance().GetManagedGravitySource( m_pControlPoint->m_hObject );
		if( pGravitySource )
		{
			pGravitySource->m_eTeamId = GetTeamId();
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Neutral_OnUpdate
//
//	PURPOSE:	Handle a update Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Neutral_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Process the inzone list.
	ProcessCharactersInZone( );

	// See if we have been captured to team0.
	if( m_fControlLevel <= kControlLevel_Team0 )
	{
		SetState( kControlPointState_Team0, NULL );
		return true;
	}
	// See if we have been captured to team1.
	else if( m_fControlLevel >= kControlLevel_Team1 )
	{
		SetState( kControlPointState_Team1, NULL );
		return true;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::Neutral_OnExit
//
//	PURPOSE:	Handle a exit Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::Neutral_OnExit( MacroStateMachine::EventParams& eventParams )
{
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::CheckInstantCapture
//
//	PURPOSE:	Check for instant capture.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::CheckInstantCapture( HOBJECT hObject )
{
	// Clear the work history, since we're playing with instant captures.
	ltstd::reset_vector( m_lstCharacterWorkHistory );

	// Get the character.
	CCharacter* pCharacter = static_cast< CCharacter* >( g_pLTServer->HandleToObject( hObject ));

	// Check if the character is invalid.
	if( !pCharacter->IsAlive( ) || pCharacter->GetTeamID() == INVALID_TEAM )
		return true;

	// Default to no points to award.
	bool bAwardCapturePoints = false;

	// Turn the CP in the team0 favor if not already fully controlled by team0.
	if( pCharacter->GetTeamID() == 0 && m_fControlLevel > kControlLevel_Team0 )
	{
		// Make sure we're in the team0 state.
		SetState( kControlPointState_Team0, NULL );

		bAwardCapturePoints = true;
	}
	// Turn the CP in the team1 favor if not already fully controlled by team1.
	else if( pCharacter->GetTeamID() == 1 && m_fControlLevel < kControlLevel_Team1 )
	{
		// Make sure we're in the team0 state.
		SetState( kControlPointState_Team1, NULL );

		bAwardCapturePoints = true;
	}

	// See if the player gets capture points.
	if( bAwardCapturePoints )
	{
		GameModeMgr& gameModeMgr = GameModeMgr::Instance();
		int32 nCPScoreCapturePlayer = gameModeMgr.m_grnCPScoreCapturePlayer;
		if( !nCPScoreCapturePlayer )
			return true;

		// Give the player some objective points.
		CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( hObject );
		if( !pPlayerObj )
			return true;
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( !pGameClientData )
			return true;
		pGameClientData->GetPlayerScore()->AddObjectiveScore( nCPScoreCapturePlayer );

		// Tell the client of their event if points were awarded.
		CAutoMessage cEventMsg;
		cEventMsg.WriteBits( kControlPointFXMsg_PlayerCapture, FNumBitsExclusive<kControlPointFXMsg_NumIds>::k_nValue );
		uint8 nPlayerId = ( uint8 )g_pLTServer->GetClientID( pGameClientData->GetClient( ));
		cEventMsg.Writeuint8( nPlayerId );
		SendClientEventMessage( pGameClientData->GetClient(), GetState( ), cEventMsg, true );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::UpdateOwnershipPoints
//
//	PURPOSE:	Check if we are getting points for owning the CP.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::UpdateOwnershipPoints( ETeamId eOwningTeamId )
{
	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Check if we are getting points for owning the CP.
	if( m_tmrOwningControlPointScoring.IsStarted( ))
	{
		if( m_tmrOwningControlPointScoring.IsTimedOut( ))
		{
			// Award the points.
			AwardTeamPoints( gameModeMgr.m_grnCPOwnedScoreAmountTeam, eOwningTeamId );

			// Start the next period.
			m_tmrOwningControlPointScoring.Start( gameModeMgr.m_grfCPOwnedScorePeriod );
		}

		// Tell our parent object to update.
		m_pControlPoint->SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::AddCharacterInZone
//
//	PURPOSE:	Handle a touched event.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::AddCharacterInZone( HOBJECT hObject )
{
	// Only applicable if capturing takes time.
	if( GameModeMgr::Instance().m_grfCPCapturingTime <= 0.0f )
		return true;

	CCharacter* pCharacter = CCharacter::DynamicCast( hObject );
	if( !pCharacter )
		return true;

	// Check if we already have this object in the list.
	for( ObjRefVector::iterator iter = m_lstCharactersInZone.begin(); iter != m_lstCharactersInZone.end( ); iter++ )
	{
		if( *iter == pCharacter->m_hObject )
			return true;
	}

	// Add it to the list.
	m_lstCharactersInZone.push_back( pCharacter->m_hObject );

	// Tell our parent object to update.
	m_pControlPoint->SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::CleanCharactersInZoneList
//
//	PURPOSE:	Cleans up the character in zone list to only contain valid characters.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::CleanCharactersInZoneList( )
{
	// Clear our calculation of the difference in characters in zone.
	m_nCharactersInZoneDiff = 0;

	// Check if there are no characters in zone.
	if( m_lstCharactersInZone.size( ) == 0 )
		return;

	// Get the AABB for the zone.
	LTVector vZonePos;
	g_pLTServer->GetObjectPos( m_pControlPoint->m_hObject, &vZonePos );
	LTVector const& vZoneDims = m_pControlPoint->GetZoneDims();
	LTRect3f rectZone( vZonePos - vZoneDims, vZonePos + vZoneDims );

	// Check if we already have this object in the list.
	ObjRefVector::iterator iter = m_lstCharactersInZone.begin();
	while( iter != m_lstCharactersInZone.end( ))
	{
		HOBJECT hToucher = *iter;
		if( !hToucher )
		{
			iter = m_lstCharactersInZone.erase( iter );
			continue;
		}

		// Get the character this is.
		CCharacter* pCharacter = static_cast< CCharacter* >( g_pLTServer->HandleToObject( hToucher ));

		// Ignore characters that aren't alive.
		if( !pCharacter->IsAlive( ) || pCharacter->GetTeamID() == INVALID_TEAM )
		{
			iter = m_lstCharactersInZone.erase( iter );
			continue;
		}

		// Check if character has left the zone.
		LTVector vCharacterPos;
		LTVector vCharacterDims;
		g_pLTServer->GetObjectPos( pCharacter->m_hObject, &vCharacterPos );
		g_pLTServer->Physics()->GetObjectDims( pCharacter->m_hObject, &vCharacterDims );
		if( !LTIntersect::AABB_AABB( LTRect3f( vCharacterPos - vCharacterDims, vCharacterPos + vCharacterDims ), rectZone ))
		{
			iter = m_lstCharactersInZone.erase( iter );
			continue;
		}

		iter++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::RecordTeamWork
//
//	PURPOSE:	Record the work done by a team influencing control.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::RecordTeamWork( uint8 nTeamId, float fWorkTime )
{
	// Find all the team characters.
	for( ObjRefVector::iterator iter = m_lstCharactersInZone.begin( ); iter != m_lstCharactersInZone.end( ); iter++ )
	{
		HOBJECT hCharacter = *iter;
		CCharacter* pCharacter = static_cast< CCharacter* >( g_pLTServer->HandleToObject( hCharacter ));

		// Ignore characters on other teams.
		if( pCharacter->GetTeamID() != nTeamId )
			continue;

		// Give the character credit.
		RecordCharacterWork( *pCharacter, fWorkTime );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::RecordCharacterWork
//
//	PURPOSE:	Record the work done by a character influencing control.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::RecordCharacterWork( CCharacter& character, float fWorkTime )
{
	// Check if they are already part of the work history list.
	for( TCharacterWorkHistoryList::iterator iter = m_lstCharacterWorkHistory.begin( ); iter != m_lstCharacterWorkHistory.end( ); iter++ )
	{
		CharacterWorkHistory& characterWorkHistory = *iter;
		if( character.m_hObject == characterWorkHistory.m_hCharacter )
		{
			characterWorkHistory.m_fTimeCapturing += fWorkTime;
			return;
		}
	}

	// If they weren't part of the work history, then add them.
	CharacterWorkHistory characterWorkHistory;
	characterWorkHistory.m_hCharacter = character.m_hObject;
	characterWorkHistory.m_fTimeCapturing += fWorkTime;
	m_lstCharacterWorkHistory.push_back( characterWorkHistory );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::AwardTeamPoints
//
//	PURPOSE:	Give points to team.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::AwardTeamPoints( int32 nPoints, uint8 nTeamId )
{
	// Check if no points to award.
	if( nPoints == 0 )
		return;

	CTeamMgr::Instance().AddToScore( nTeamId, nPoints );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::AwardTeamPoints
//
//	PURPOSE:	Give the contributing characters the work award.
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::AwardCharacterWorkPoints( EControlPointFXMsgId eControlPointFXMsgId, int32 nPlayerPoints, 
														uint8 nTeamId )
{
	// Make sure it takes time to capture.  No work points if it takes no work.
	GameModeMgr& gameModeMgr = GameModeMgr::Instance();
	if( gameModeMgr.m_grfCPCapturingTime <= 0.0f )
		return;

	CAutoMessage cEventMsg;

	// Award points to each teammate that worked on control.
	for( TCharacterWorkHistoryList::iterator iter = m_lstCharacterWorkHistory.begin( ); iter != m_lstCharacterWorkHistory.end( ); iter++ )
	{
		// Get the character object.
		CharacterWorkHistory& characterWorkHistory = *iter;
		if( !characterWorkHistory.m_hCharacter )
			continue;

		// They have to be a player.
		CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( characterWorkHistory.m_hCharacter );
		if( !pPlayerObj )
			continue;

		// Make sure they match the team.
		if( pPlayerObj->GetTeamID() != nTeamId )
			continue;

		// Get the gameclientdata for this player.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( !pGameClientData )
			continue;

		// Create an award that is proportional to the amount of time capturing divided by the amount of 
		// time it requires to capture.
		float fAward = ( float )nPlayerPoints * ( characterWorkHistory.m_fTimeCapturing / gameModeMgr.m_grfCPCapturingTime );
		// Round up.
		uint32 nAward = ( uint32 )( fAward + 0.5f );

		// If they won't get any points, then skip them.
		if( nAward == 0 )
			continue;

		// Give the player some objective points.
		pGameClientData->GetPlayerScore()->AddObjectiveScore( nAward );

		// Tell the client of their event.
		cEventMsg.Reset();
		cEventMsg.WriteBits( eControlPointFXMsgId, FNumBitsExclusive<kControlPointFXMsg_NumIds>::k_nValue );
		uint8 nPlayerId = ( uint8 )g_pLTServer->GetClientID( pGameClientData->GetClient( ));
		cEventMsg.Writeuint8( nPlayerId );
		SendClientEventMessage( pGameClientData->GetClient(), GetState( ), cEventMsg, true );
	}

	// Clear the work history.
	ltstd::reset_vector( m_lstCharacterWorkHistory );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::ProcessCharactersInZone
//
//	PURPOSE:	Processes the inzone list.
//
// ----------------------------------------------------------------------- //
bool ControlPointStateMachine::ProcessCharactersInZone( )
{
	// Don't allow action after end round condition met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet( ))
		return false;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Nothing to do if captures are instant.
	if( gameModeMgr.m_grfCPCapturingTime <= 0.0f )
		return true;

	// Clean up the list to only have valid characters.
	CleanCharactersInZoneList( );

	// Check if we don't have any characters in the zone.
	if( m_lstCharactersInZone.size( ) == 0 )
	{
		// See if we were waiting to tell the client about the control level.
		if( m_tmrControlLevelUpdate.IsStarted( ))
		{
			// Stop timing, since there's no one in the zone to tell the client about.
			m_tmrControlLevelUpdate.Stop();

			// Send the control level to the client to make them aware no more clients are around.
			SendControlLevel( true );
		}

		return true;
	}

	uint32 nNumTeam0 = 0;
	uint32 nNumTeam1 = 0;

	// Go through character in the zone and figure out what influence on the control they have.
	for( ObjRefVector::iterator iter = m_lstCharactersInZone.begin( ); iter != m_lstCharactersInZone.end( ); iter++ )
	{
		HOBJECT hCharacter = *iter;
		CCharacter* pCharacter = static_cast< CCharacter* >( g_pLTServer->HandleToObject( hCharacter ));

		// Count the team0 character.
		if( pCharacter->GetTeamID() == 0 )
		{
			nNumTeam0++;
		}
		// Count the team1 character.
		else
		{
			nNumTeam1++;
		}
	}

	// Get the difference between teams.
	m_nCharactersInZoneDiff = nNumTeam1 - nNumTeam0;

	// Get the time for this frame.
	float fElapsedTime = GameTimeTimer::Instance().GetTimerElapsedS( );

	// Will be set if actual work being done to the control level.
	uint8 nTeamIdDoingWork = INVALID_TEAM;

	// Get the effective team count, where negative numbers means more team0 and postive numbers
	// means more team1.
	int32 nEffectiveTeamCount = 0;

	// Will be set to a +/-1 multiplier based on direction control point is getting captured.
	float fDirectionMultiplier = 0.0f;

	// Will contain the amount of control left to cap.
	float fControlDeltaMag = 0.0f;

	// If team0 has more influence and doesn't have full control, then they are doing work.  
	if( m_nCharactersInZoneDiff < 0 && m_fControlLevel > kControlLevel_Team0 )
	{
		nTeamIdDoingWork = kTeamId0;
		fDirectionMultiplier = -1.0f;
		nEffectiveTeamCount = -m_nCharactersInZoneDiff;
		fControlDeltaMag = m_fControlLevel - kControlLevel_Team0;
	}
	// If team1 has more influence and doesn't have full control, then they are doing work.  
	else if( m_nCharactersInZoneDiff > 0 && m_fControlLevel < kControlLevel_Team1 )
	{
		nTeamIdDoingWork = kTeamId1;
		fDirectionMultiplier = 1.0f;
		nEffectiveTeamCount = m_nCharactersInZoneDiff;
		fControlDeltaMag = kControlLevel_Team1 - m_fControlLevel;
	}

	// Check if we did actual work to the controllevel.
	if( nTeamIdDoingWork != INVALID_TEAM )
	{
		// Adjust the control level.  The delta to the control level is the amount of time spent
		// this frame divided by the total time needed.  Since multiple people can capture, the time delta
		// is multiplied by the number of people.  So the captures don't go too fast, the speed up
		// per person is multiplied by group capture factor.  
		float fGroupMultiplier = gameModeMgr.m_grfCPGroupCaptureFactor * ( nEffectiveTeamCount - 1 ) + 1.0f;
		float fModifiedElapsedTime = fElapsedTime * fGroupMultiplier;

		// If we spent more time this frame than is needed, clamp to the time required.
		float fTimeDeltaLeft = fControlDeltaMag * gameModeMgr.m_grfCPCapturingTime;
		fModifiedElapsedTime = LTMIN( fModifiedElapsedTime, fTimeDeltaLeft );

		// Give the team players credit for doing work.
		RecordTeamWork( nTeamIdDoingWork, fModifiedElapsedTime );

		m_fControlLevel += fDirectionMultiplier * fModifiedElapsedTime / gameModeMgr.m_grfCPCapturingTime;
		m_fControlLevel = LTCLAMP( m_fControlLevel, kControlLevel_Team0, kControlLevel_Team1 );

		// If we've reached a full control to one team, then don't need to keep sending.
		if( m_fControlLevel == kControlLevel_Team0 || m_fControlLevel == kControlLevel_Team1 )
		{
			// Stop sending periodic control level updates since we're at the limit.
			m_tmrControlLevelUpdate.Stop();

			// Tell all clients about it.
			SendControlLevel( true );
		}
		// Check if it's time to send the control level to the client.
		else if( m_tmrControlLevelUpdate.IsStarted() && m_tmrControlLevelUpdate.IsTimedOut())
		{
			// Start next period.
			m_tmrControlLevelUpdate.Start( kControlPoint_ControlLevelSendPeriod );
			// Send unguaranteed, since we'll be sending it all the time.
			SendControlLevel( false );
		}
		// Check if we haven't told the client yet about new characters in the zone.
		else if( !m_tmrControlLevelUpdate.IsStarted())
		{
			// Start next period.
			m_tmrControlLevelUpdate.Start( kControlPoint_ControlLevelSendPeriod );
			// Send guaranteed so they get the start.
			SendControlLevel( true );
		}
	}
	// No one is doing work.  Make sure client knows to stop if it was told to start.
	else if( m_tmrControlLevelUpdate.IsStarted( ))
	{
		// Stop sending periodic control level.
		m_tmrControlLevelUpdate.Stop();

		// Tell all clients about it.
		SendControlLevel( true );
	}

	// Need updates to happen to poll for players.
	m_pControlPoint->SetNextUpdate( UPDATE_NEXT_FRAME );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::SendControlLevel
//
//	PURPOSE:	Sends client the control level
//
// ----------------------------------------------------------------------- //
void ControlPointStateMachine::SendControlLevel( bool bGuaranteed )
{
	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kControlPointFXMsg_ControlLevel, FNumBitsExclusive<kControlPointFXMsg_NumIds>::k_nValue );
	// Make sure the control level sent is clamped to the valid range.
	float fClampedControlLevel = LTCLAMP( m_fControlLevel, kControlLevel_Team0, kControlLevel_Team1 );
	// Map control level to 8 bits for optimization.
	uint8 nMappedControlLevel = ( uint8 )(( fClampedControlLevel + 1.0f ) * 127.5f );
	cEventMsg.Writeuint8( nMappedControlLevel );
	// Send whether characters are in the zone.
	cEventMsg.Writebool( abs( m_nCharactersInZoneDiff ) > 0 );
	SendClientEventMessage( NULL, GetState( ), cEventMsg, bGuaranteed );
}



// Plugin class for hooking into the level editor for displaying entries in listboxes and displaying the model...
class ControlPointPlugin : public IObjectPlugin
{
public: // Methods...

	virtual LTRESULT PreHook_EditStringList( 
		const char *szRezPath,
		const char *szPropName,
		char **aszStrings,
		uint32 *pcStrings,
		const uint32 cMaxStrings,
		const uint32 cMaxStringLen );

	virtual LTRESULT PreHook_Dims(
		const char* szRezPath,
		const char* szPropName, 
		const char* szPropValue,
		char* szModelFilenameBuf,
		int	  nModelFilenameBufLen,
		LTVector & vDims,
		const char* pszObjName, 
		ILTPreInterface *pInterface);

	virtual LTRESULT PreHook_PropChanged( 
		const	char		*szObjName,
		const	char		*szPropName,
		const	int			nPropType,
		const	GenericProp	&gpPropValue,
		ILTPreInterface		*pInterface,
		const	char		*szModifiers );

private: // Members...

	CCommandMgrPlugin			m_CommandMgrPlugin;
};

BEGIN_CLASS( ControlPoint )
	ADD_STRINGPROP_FLAG( Filename, "", PF_HIDDEN | PF_MODEL, "This hidden property is needed in order to get the model visible within WorldEdit." )
	ADD_STRINGPROP_FLAG( Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Record within the game database GameModes/CP/Types to use for defining control point." )
	ADD_VECTORPROP_FLAG( ZoneDims, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "The override dims to use for zone if OverrideZoneDims is true.  Does not set the physical dims of the object.  Override zonedims will be used if all components are greater than 0, otherwise the default zonedims from database will be used.")
	ADD_STRINGPROP_FLAG( Team, "NoTeam", PF_STATICLIST, "The initial owning team of the object.")
	ADD_STRINGPROP_FLAG( ControlPointId, "1", PF_STATICLIST, "Unique ID of this control point.  All CP's in world must have different ID's.")
	ADD_BOOLPROP( MoveToFloor, true, "If true the object is moved to the floor when created in the game." )
	PROP_DEFINEGROUP(Commands, PF_GROUP(1), "This is a subset that lets you write commands for specific states that the object will carry out when in that state.")
	ADD_COMMANDPROP_FLAG(Team0NeutralizedControlCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "Command sent if Team0 neutralizes control of the control point.")
	ADD_COMMANDPROP_FLAG(Team1NeutralizedControlCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "Command sent if Team1 neutralizes control of the control point.")
	ADD_COMMANDPROP_FLAG(Team0CapturedControlCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "Command sent if Team0 captures control of the control point.")
	ADD_COMMANDPROP_FLAG(Team1CapturedControlCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "Command sent if Team1 captures control of the control point.")
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH( ControlPoint, GameBase, CF_DONTSAVE, ControlPointPlugin, DefaultPrefetch<ControlPoint>, "Places a ControlPoint within the level." )

// Register with the CommandMgr...
CMDMGR_BEGIN_REGISTER_CLASS( ControlPoint )
CMDMGR_END_REGISTER_CLASS( ControlPoint, GameBase )


LTRESULT ControlPointPlugin::PreHook_EditStringList( const char *szRezPath,
												   const char *szPropName,
												   char **aszStrings,
												   uint32 *pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLen )
{
	if( !aszStrings || !pcStrings )
	{
		LTERROR( "Invalid input parameters" );
		return LT_UNSUPPORTED;
	}

	static CParsedMsg::CToken s_cTok_Type( "Type" );
	static CParsedMsg::CToken s_cTok_Team( "Team" );
	static CParsedMsg::CToken s_cTok_ControlPointId( "ControlPointId" );

	// Tokenize the input for fast searching.
	CParsedMsg::CToken token( szPropName );

	if( token == s_cTok_Type )
	{
		// Fill the first string in the list with a <none> selection...
		LTStrCpy( aszStrings[(*pcStrings)++], "", cMaxStringLen );

		// Add an entry for each type.
		uint8 nNumRecords = DATABASE_CATEGORY( CPTypes ).GetNumRecords();
		for( uint8 nRecordIndex = 0; nRecordIndex < nNumRecords; ++nRecordIndex )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many controlpoint types to fit in the list.  Enlarge list size?" );

			HRECORD hRecord = DATABASE_CATEGORY( CPTypes ).GetRecordByIndex( nRecordIndex );
			if( !hRecord )
				continue;

			const char *pszRecordName = DATABASE_CATEGORY( CPTypes ).GetRecordName( hRecord );
			if( !pszRecordName )
				continue;

			if( (LTStrLen( pszRecordName ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], pszRecordName, cMaxStringLen );
			}
		}

		// Sort the list so things are easier to find.  Skip the first item, since it's the <none> selection.
		qsort( aszStrings + 1, *pcStrings - 1, sizeof(char *), CaseInsensitiveCompare );

		return LT_OK;
	}
	// Handle team...
	else if( token == s_cTok_Team )
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLen );
		return LT_OK;
	}
	else if( token == s_cTok_ControlPointId )
	{
		// Add an entry for each type.
		char szText[64];
		for( uint8 nId = 1; nId <= MAX_CONTROLPOINT_OBJECTS; ++nId )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many controlpoint types to fit in the list.  Enlarge list size?" );
			
			LTSNPrintF( szText, LTARRAYSIZE( szText ), "%d", nId );

			if( (LTStrLen( szText ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], szText, cMaxStringLen );
			}
		}

		return LT_OK;
	}


	return LT_UNSUPPORTED;
}

//
// NOTE TO INTEGRATIONS
//
// This function depends on the updates to PreHook_Dims made to the tools that will
// not be propogated forward (changelist 59304).  This will need to be updated to the new method upon integration.
LTRESULT ControlPointPlugin::PreHook_Dims( const char* szRezPath,
										  const char* szPropName, 
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector &vDims,
										 const char* pszObjName, 
										 ILTPreInterface *pInterface)
{
	if( LTStrIEquals( szPropName, "Type" ))
	{
		// Don't proceed without a value value.
		if( LTStrEmpty( szPropValue ))
			return LT_OK;

		// Get type used.
		HRECORD hRecord = DATABASE_CATEGORY( CPTypes ).GetRecordByName( szPropValue );
		if( !hRecord )
			return LT_UNSUPPORTED;

		// Get the prop used.
		HRECORD hProp = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( hRecord, Prop );
		if( !hProp )
			return LT_UNSUPPORTED;

		// Get the model from the props category.
		const char *pszModel = g_pPropsDB->GetPropFilename( hProp );
		if( !pszModel )
			return LT_UNSUPPORTED;

		LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );

	}
	else if( LTStrIEquals( szPropName, "ZoneDims" ))
	{
		GenericProp genProp;
		if( pInterface->GetPropGeneric( pszObjName, "Type", &genProp ) != LT_OK )
			return LT_UNSUPPORTED;

		szPropValue = genProp.GetString();

		// Don't proceed without a value value.
		if( LTStrEmpty( szPropValue ))
			return LT_OK;

		// Get type used.
		HRECORD hRecord = DATABASE_CATEGORY( CPTypes ).GetRecordByName( szPropValue );
		if( !hRecord )
			return LT_UNSUPPORTED;

		// Get the prop used.
		HRECORD hProp = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( hRecord, Prop );
		if( !hProp )
			return LT_UNSUPPORTED;

		// See if there are override dims for this instance.
		bool bGotOverrideDims = false;
		if( pInterface->GetPropGeneric( pszObjName, "ZoneDims", &genProp ) == LT_OK )
		{
			LTVector const& vZoneDims = genProp.GetVector();
			if( vZoneDims.x > 0.0f && vZoneDims.y > 0.0f && vZoneDims.z > 0.0f )
			{
				bGotOverrideDims = true;
				vDims = vZoneDims;
			}
		}

		// See if we need to use the default dims.
		if( !bGotOverrideDims )
		{
			vDims = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( hRecord, DefaultZoneDims );
		}
	}

	return LT_OK;
}

LTRESULT ControlPointPlugin::PreHook_PropChanged( const char *szObjName, 
												const char *szPropName,
												const int nPropType,
												const GenericProp &gpPropValue,
												ILTPreInterface *pInterface,
												const char *szModifiers )
{

	// Only our command is marked for change notification so just send it to the CommandMgr..
	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
		szPropName, 
		nPropType, 
		gpPropValue,
		pInterface,
		szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ControlPointTouchMonitor
//
//  Special class created by ControlPoint to monitor for touchnotifies.
//
// ----------------------------------------------------------------------- //
class ControlPointTouchMonitor : public GameBase
{
public:

	// Accessors to controlpoint object.
	void SetControlPoint( HOBJECT hControlPoint )
	{
		m_hControlPoint = hControlPoint;
	}
	HOBJECT GetControlPoint( ) const { return m_hControlPoint; }

protected: // Methods... 

	// Handle messages from the engine...
	uint32 EngineMessageFn( uint32 messageID, void *pData, float fData );

private:
	LTObjRef m_hControlPoint;
};

// Special worker class for ControlPoint only.  Used to receive
// touch notifies.
BEGIN_CLASS( ControlPointTouchMonitor )
END_CLASS_FLAGS( ControlPointTouchMonitor, GameBase, CF_DONTSAVE | CF_HIDDEN, "ControlPoint touchmonitor object for receiving touch notifies." )


//
// ControlPoint class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPoint::ControlPoint
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ControlPoint::ControlPoint( ) : GameBase( OT_MODEL )
{
	m_pControlPointStateMachine = NULL;
	m_bMoveToFloor = true;
	m_hControlPointRec = NULL;
	m_nInitialTeamId = INVALID_TEAM;
	m_nControlPointId = CONTROLPOINT_INVALID_ID;
	m_vZoneDims.Init( );

	m_pszTeam0NeutralizedControlCmd = NULL;
	m_pszTeam1NeutralizedControlCmd = NULL;
	m_pszTeam0CapturedControlCmd = NULL;
	m_pszTeam1CapturedControlCmd = NULL;

	// Add ourselves to the global list.
	m_lstControlPoints.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPoint::~ControlPoint
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ControlPoint::~ControlPoint( )
{
	if( m_pControlPointStateMachine )
	{
		delete m_pControlPointStateMachine;
		m_pControlPointStateMachine = NULL;
	}

	if( m_hTouchMonitor )
	{
		g_pLTServer->RemoveObject( m_hTouchMonitor );
		m_hTouchMonitor = NULL;
	}

	// Clear out our controlpoint id from the allocated ones.
	if( m_nControlPointId != CONTROLPOINT_INVALID_ID )
	{
		uint32 nControlPointBit = 1 << m_nControlPointId;
		m_nAllocatedControlPointIds &= ~nControlPointBit;
	}

	delete[] m_pszTeam0NeutralizedControlCmd;
	delete[] m_pszTeam1NeutralizedControlCmd;
	delete[] m_pszTeam0CapturedControlCmd;
	delete[] m_pszTeam1CapturedControlCmd;

	// Erase this instance from the list.
	TControlPointList::iterator it = std::find( m_lstControlPoints.begin( ), m_lstControlPoints.end( ), this );
	if( it != m_lstControlPoints.end( ))
	{
		m_lstControlPoints.erase( it );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //
uint32 ControlPoint::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				if( !ReadProp( &pStruct->m_cProperties ))
					return 0;
			}

			if( !PostReadProp( pStruct ))
				return 0;

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_UPDATE:
		{
			Update( );
		}
		break;

		default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //
bool ControlPoint::ReadProp( const GenericPropList *pProps )
{
	char const* pszType = pProps->GetString( "Type", "" );
	if( LTStrEmpty( pszType ))
		return false;
	m_hControlPointRec = DATABASE_CATEGORY( CPTypes ).GetRecordByName( pszType );
	if( !m_hControlPointRec )
		return false;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Don't allow CP's if this isn't CP game.
	if( !gameModeMgr.GetCPRulesRecord( ))
		return false;
	
	m_bMoveToFloor = pProps->GetBool( "MoveToFloor", m_bMoveToFloor );

	const char *pszTeam = pProps->GetString( "Team", "" );
	m_nInitialTeamId = TeamStringToTeamId( pszTeam );

	const char *pszControlPointId = pProps->GetString( "ControlPointId", "" );
	uint16 nControlPointId = ( uint16 )LTCLAMP( LTStrToLong( pszControlPointId ), 0, USHRT_MAX );
	if( nControlPointId == CONTROLPOINT_INVALID_ID || nControlPointId > MAX_CONTROLPOINT_OBJECTS )
	{
		LTERROR( "Invalid ControlPoint id" );
		return false;
	}

	// Figure out what bit value this controlpoint id is.  Make sure the id isn't already in use.
	// Control point id's must be unique.
	uint32 nControlPointBit = 1 << nControlPointId;
	if( m_nAllocatedControlPointIds & nControlPointBit )
	{
		LTERROR( "ControlPoint id already used" );
		return false;
	}

	// ControlPoint id checks out, we can use it.
	m_nControlPointId = nControlPointId;
	m_nAllocatedControlPointIds |= nControlPointBit;

	// Check if this object is overriding the zonedims.  If not, then take the zone dims from the db.
	m_vZoneDims = pProps->GetVector( "ZoneDims", m_vZoneDims );
	if( m_vZoneDims.x <= 0.0f || m_vZoneDims.y <= 0.0f || m_vZoneDims.z <= 0.0f )
	{
		m_vZoneDims = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_hControlPointRec, DefaultZoneDims );
	}

	// Read in the commands for events.  Note, team0 in code corresponds to team1 in worldedit.  Team1 in 
	// code corresponds to team2 in worldedit.
	delete[] m_pszTeam0NeutralizedControlCmd;
	m_pszTeam1NeutralizedControlCmd	= LTStrDup( pProps->GetCommand( "Team0NeutralizedControlCmd", NULL ));
	delete[] m_pszTeam1NeutralizedControlCmd;
	m_pszTeam1NeutralizedControlCmd	= LTStrDup( pProps->GetCommand( "Team1NeutralizedControlCmd", NULL ));
	delete[] m_pszTeam0CapturedControlCmd;
	m_pszTeam0CapturedControlCmd	= LTStrDup( pProps->GetCommand( "Team0CapturedControlCmd", NULL ));
	delete[] m_pszTeam1CapturedControlCmd;
	m_pszTeam1CapturedControlCmd	= LTStrDup( pProps->GetCommand( "Team1CapturedControlCmd", NULL ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::PostReadProp
//
//	PURPOSE:	Configure the ObjectCreateStruct for creating the object
//
// ----------------------------------------------------------------------- //
bool ControlPoint::PostReadProp( ObjectCreateStruct *pStruct )
{
	// Get the prop used.
	HRECORD hProp = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_hControlPointRec, Prop );
	if( !hProp )
		return false;

	// Fill in the model and material names.
	char const* pszPropFilename = g_pPropsDB->GetPropFilename( hProp );
	if( LTStrEmpty( pszPropFilename ))
		return false;
	LTStrCpy( pStruct->m_Filename, pszPropFilename, LTARRAYSIZE( pStruct->m_Filename ));

	g_pPropsDB->CopyMaterialFilenames( hProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ),
		LTARRAYSIZE( pStruct->m_Materials[0] ));

	// Make the object visible.
	pStruct->m_Flags |= FLAG_VISIBLE;

	// Turn on solid flag if desired.
	if( DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_hControlPointRec, Solid ))
	{
		pStruct->m_Flags |= ( FLAG_SOLID | FLAG_RAYHIT );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::InitialUpdate
//
//	PURPOSE:	Handle a MID_INITIALUPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void ControlPoint::InitialUpdate( )
{
	// Set the model diminsions...
	LTVector vDims;
	HMODELANIM hAnim = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hAnim );
	g_pModelLT->GetModelAnimUserDims (m_hObject, hAnim, &vDims);
	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

	// Make sure object starts on floor if the flag is set...
	if( m_bMoveToFloor )
	{
		MoveObjectToFloor( m_hObject );
	}

	// Create the touchmonitor.
	CreateTouchMonitor( );

	// Set the collisionproperty.
	HRECORD hCollisionPropertyRec = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_hControlPointRec, CollisionProperty );
	if( hCollisionPropertyRec )
	{
		uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionPropertyRec );
		g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );
	}

	// Set the surface type.
	HSURFACE hSurfaceRec = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_hControlPointRec, SurfaceType );
	if( hSurfaceRec )
	{
		SurfaceType eSurfType = g_pSurfaceDB->GetSurfaceType(hSurfaceRec);
		uint32 dwSurfUsrFlgs = SurfaceToUserFlag(eSurfType);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwSurfUsrFlgs, USRFLG_SURFACEMASK);
	}

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pControlPointStateMachine = new ControlPointStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pControlPointStateMachine->Init( *this );
	
	// Start off based on initial team.
	EControlPointState eInitialState;
	switch( m_nInitialTeamId )
	{
	case 0:
		eInitialState = kControlPointState_Team0;
		break;
	case 1:
		eInitialState = kControlPointState_Team1;
		break;
	default:
		eInitialState = kControlPointState_Neutral;
		break;
	}
	m_pControlPointStateMachine->SetState( eInitialState, NULL );

	CreateSpecialFX( false );

	// Don't need updates unless we're getting captured.
	SetNextUpdate( UPDATE_NEVER );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::Update
//
//	PURPOSE:	Handle a MID_UPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void ControlPoint::Update( )
{
	if( !m_pControlPointStateMachine )
		return;

	// Update the statemachine.
	m_pControlPointStateMachine->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::CreateSpecialFX
//
//	PURPOSE:	Send relevant information to clients...
//
// ----------------------------------------------------------------------- //

void ControlPoint::CreateSpecialFX( bool bUpdateClients )
{
	if( !m_pControlPointStateMachine )
		return;

	CONTROLPOINTCREATESTRUCT cs;
	cs.m_eControlPointState = ( EControlPointState )m_pControlPointStateMachine->GetState( );
	cs.m_hControlPointRec = m_hControlPointRec;
	cs.m_nControlPointId = m_nControlPointId;
	cs.m_vZoneDims = m_vZoneDims;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_CONTROLPOINT_ID );
		cs.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::CreateTouchMonitor
//
//	PURPOSE:	Creates a touch monitor object to receive MID_TOUCHNOTIFY's.
//
// ----------------------------------------------------------------------- //
bool ControlPoint::CreateTouchMonitor( )
{
	HCLASS hClass = g_pLTServer->GetClass("ControlPointTouchMonitor");
	if( !hClass ) 
		return false;

	// Create the object to monitor for touches.
	ObjectCreateStruct theStruct;
	g_pLTServer->GetObjectPos( m_hObject, &theStruct.m_Pos );
	g_pLTServer->GetObjectRotation( m_hObject, &theStruct.m_Rotation );
	theStruct.m_UserData = reinterpret_cast< uint32 >( m_hObject );
	GameBase* pTouchMonitor = ( GameBase* )g_pLTServer->CreateObject( hClass, &theStruct );
	if( !pTouchMonitor )
		return false;
	m_hTouchMonitor = pTouchMonitor->m_hObject;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::OnTouched
//
//	PURPOSE:	Called by the touch monitor object.
//
// ----------------------------------------------------------------------- //
void ControlPoint::OnTouched( HOBJECT hToucher )
{
	if( !m_pControlPointStateMachine )
		return;

	// Make sure this is a character.  Other objects are not tracked.
	CCharacter* pCharacter = CCharacter::DynamicCast( hToucher );
	if( !pCharacter )
		return;

	// Relay the touch event to the statemachine.
	m_pControlPointStateMachine->DoTouchEvent( hToucher );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::CheckForConquestWin
//
//	PURPOSE:	Checks for acheiving conquest win. Happens when all CP's are owned
//				by one team.
//
// ----------------------------------------------------------------------- //
void ControlPoint::CheckForConquestWin( )
{
	// Check if end round condition already met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet( ))
		return;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Conquest win not an option.
	if( !gameModeMgr.m_grbCPConquestWin )
		return;

	// Make sure we have some control points.
	uint32 nNumControlPoints = m_lstControlPoints.size( );
	if( nNumControlPoints == 0 )
		return;

	// Iterate through the control points and count the number controlled by each team.
	uint32 nNumTeam0 = 0;
	uint32 nNumTeam1 = 0;
	for( TControlPointList::iterator iter = m_lstControlPoints.begin( ); iter != m_lstControlPoints.end( ); iter++ )
	{
		ControlPoint* pControlPoint = *iter;
		if( !pControlPoint->m_pControlPointStateMachine )
			continue;

		switch( pControlPoint->m_pControlPointStateMachine->GetTeamId())
		{
		case kTeamId0:
			nNumTeam0++;
			break;
		case kTeamId1:
			nNumTeam1++;
			break;
		}
	}

	// See if either team one.
	ETeamId eWinningTeam = INVALID_TEAM;
	if( nNumTeam0 == nNumControlPoints )
	{
		eWinningTeam = kTeamId0;
	}
	else if( nNumTeam1 == nNumControlPoints )
	{
		eWinningTeam = kTeamId1;
	}
	else
	{
		// Neither team has reached conquest.
		return;
	}

	// Set the winner.
	g_pServerMissionMgr->SetTeamWon( eWinningTeam );
	g_pServerMissionMgr->SetEndRoundCondition( GameModeMgr::eEndRoundCondition_Conquest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPoint::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void ControlPoint::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the CP record
	char szType[256];
	pInterface->GetPropString(pszObjectName, "Type", szType, LTARRAYSIZE(szType), NULL);
	if( LTStrEmpty( szType ))
		return;
	HRECORD hControlPointRec = DATABASE_CATEGORY( CPTypes ).GetRecordByName( szType );
	if( !hControlPointRec )
		return;

	GetRecordResources( Resources, hControlPointRec, true );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointTouchMonitor::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //
uint32 ControlPointTouchMonitor::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			// Need to get touch notifies to send to the ControlPoint.
			pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;

			// Remember our parent controlpoint.
			SetControlPoint( reinterpret_cast< HOBJECT >( pStruct->m_UserData ));

			return dwRet;
		}
		break;

	case MID_INITIALUPDATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );
			
			ControlPoint* pControlPoint = static_cast< ControlPoint* >( g_pLTServer->HandleToObject( GetControlPoint( )));
			if( pControlPoint )
			{
				// Set our object dims.
				LTVector vDims = pControlPoint->GetZoneDims( );
				g_pLTServer->Physics()->SetObjectDims( m_hObject, &vDims, 0 );
			}

			return dwRet;
		}
		break;

	case MID_TOUCHNOTIFY:
		{
			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ControlPoint* pControlPoint = static_cast< ControlPoint* >( g_pLTServer->HandleToObject( m_hControlPoint ));
			if( pControlPoint )
			{
				// Give our parent object the touch.
				HOBJECT hToucher = reinterpret_cast< HOBJECT >( pData );
				pControlPoint->OnTouched( hToucher );
			}

			return dwRet;
		}
		break;

	default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}
