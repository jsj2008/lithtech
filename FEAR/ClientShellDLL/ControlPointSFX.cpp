// ----------------------------------------------------------------------- //
//
// MODULE  : ControlPointSFX.cpp
//
// PURPOSE : Client side representation of ControlPoint object
//
// CREATED : 02/10/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ControlPointSFX.h"
#include "GameModeMgr.h"
#include "StateMachine.h"
#include "ControlPointDB.h"
#include "TeamClientFXSFX.h"
#include "NavMarkerFX.h"
#include "HUDMessageQueue.h"
#include "HUDTransmission.h"
#include "HUDMgr.h"

ControlPointSFX::ControlPointSFXList ControlPointSFX::m_lstControlPointSFX;

typedef ControlPointSFX* ControlPointSFXPtr;
struct ControlPointSFXSort
{
	bool operator()( const ControlPointSFXPtr& x, const ControlPointSFXPtr& y) const
	{
		return ( x->GetCS().m_nControlPointId < y->GetCS().m_nControlPointId );
	}
};


static const float kControlLevel_Team0 = -1.0f;
static const float kControlLevel_Neutral = 0.0f;
static const float kControlLevel_Team1 = 1.0f;

// Statemachine to handle controlpointsfx states.
class ControlPointSFXStateMachine : public MacroStateMachine
{
public:

	ControlPointSFXStateMachine( )
	{
		m_pControlPointSFX = NULL;
		m_pFriendlyNavMarker = NULL;
		m_pEnemyNavMarker = NULL;
		m_pNeutralNavMarker = NULL;
		m_fControlLevel = 0.0f;
		m_bCharactersInZone = false;
		m_hCapturingSound = NULL;
		m_nLastStage = ( uint32 )-1;
		m_eLastTeamId = INVALID_TEAM;
	}

	~ControlPointSFXStateMachine( )
	{
		Term( );
	}

	// Initialize to a object.
	bool Init( ControlPointSFX& controlPointSfx );

	// Terminate the object.
	void Term( );

	// Get the current team the control point is owned by.
	ETeamId GetTeamId( ) const;

	// Handles event messages from server.
	void OnEventMessage( ILTMessage_Read *pMsg );

	// Get the control level.
	float GetControlLevel( ) const { return m_fControlLevel; }

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
	bool TeamX_OnEnter( float fControlLevel );

	// State table.
	MSM_BeginTable( ControlPointSFXStateMachine )
		MSM_BeginState( kControlPointState_Team0 )
			MSM_OnEnter( Team0_OnEnter )
			MSM_OnUpdate( Team0_OnUpdate )
			MSM_OnExit( Team0_OnExit )
		MSM_EndState( )
		MSM_BeginState( kControlPointState_Team1 )
			MSM_OnEnter( Team1_OnEnter )
			MSM_OnUpdate( Team1_OnUpdate )
			MSM_OnExit( Team1_OnExit )
		MSM_EndState( )
		MSM_BeginState( kControlPointState_Neutral )
			MSM_OnEnter( Neutral_OnEnter )
			MSM_OnUpdate( Neutral_OnUpdate )
			MSM_OnExit( Neutral_OnExit )
		MSM_EndState( )
	MSM_EndTable( )

	// Sets up the clientfx to show who owns the CP.
	void SetOwnerClientFx( );
	// Sets up the navmarker to show who owns the CP.
	void SetNavMarkerFX( );
	// Sets up the clientfx to show what stage the ownership is at.
	void SetStageClientFx( );

	//	Play/stop the sound when characters are capturing
	bool PlayCapturingSound( );
	bool StopCapturingSound( );

private:

	// The controlpointsfx that owns us.
	ControlPointSFX* m_pControlPointSFX;

	// Control level sent from server.
	float m_fControlLevel;

	// Used by stage clientfx to track if stages have changed.
	uint32 m_nLastStage;
	ETeamId m_eLastTeamId;

	// Set when some characters are in the zone to capture.
	bool m_bCharactersInZone;

	// Sound played while being captured.
	HLTSOUND m_hCapturingSound;

	TeamClientFXSFX m_CapturedTeamClientFx;
	TeamClientFXSFX m_StageTeamClientFx;
	CClientFXLink m_NeutralClientFxLink;
	CNavMarkerFX* m_pFriendlyNavMarker;
	CNavMarkerFX* m_pEnemyNavMarker;
	CNavMarkerFX* m_pNeutralNavMarker;
};



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Init
//
//	PURPOSE:	Initialize the object.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Init( ControlPointSFX& controlPointSfx )
{
	m_pControlPointSFX = &controlPointSfx;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Init
//
//	PURPOSE:	Terminate the object.
//
// ----------------------------------------------------------------------- //
void ControlPointSFXStateMachine::Term( )
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
	
	// Shutdown the old captured teamclientfx.
	m_CapturedTeamClientFx.Term( );

	// Shutdown the old stage teamclientfx.
	m_StageTeamClientFx.Term( );
	m_nLastStage = ( uint32 )-1;
	m_eLastTeamId = INVALID_TEAM;

	// Shutdown the old neutral clientfx.
	pClientFXMgr->ShutdownClientFX( &m_NeutralClientFxLink );
	m_NeutralClientFxLink.ClearLink();

	// Shutdown the nammarkers.
	if( m_pFriendlyNavMarker )
	{
		m_pFriendlyNavMarker->WantRemove( true );
		m_pFriendlyNavMarker = NULL;
	}
	if( m_pEnemyNavMarker )
	{
		m_pEnemyNavMarker->WantRemove( true );
		m_pEnemyNavMarker = NULL;
	}
	if( m_pNeutralNavMarker )
	{
		m_pNeutralNavMarker->WantRemove( true );
		m_pNeutralNavMarker = NULL;
	}

	// Make sure capturing sound is stopped.
	StopCapturingSound( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoCPEventMessage
//
//  PURPOSE:	Common function used to post messages due to a control point event.
//
// ----------------------------------------------------------------------- //
inline bool DoCPEventMessage( eChatMsgType eType, char const* pszTransmissionStringId )
{
	// Do the transmission.
	if( LTStrEmpty( pszTransmissionStringId ))
		return false;

	const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId( "Dialogue", pszTransmissionStringId );
	if( !LTStrEmpty( pszFilename ))
	{
		g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
	}

	g_pTransmission->Show( LoadString( pszTransmissionStringId ));

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::TeamX_OnEnter
//
//	PURPOSE:	Handle a enter Team0/1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::TeamX_OnEnter( float fControlLevel )
{
	// Control level is fully in control when first entering state.
	m_fControlLevel = fControlLevel;

	SetOwnerClientFx( );
	SetNavMarkerFX( );
	SetStageClientFx( );

	// Don't need capturing sound anymore now that it's captured.
	StopCapturingSound( );

	// Check if the CP is under our control.
	bool bLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( GetTeamId( ));

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Play the captured animation.
	bool bPlayedAni = false;
	char const* pszAniName = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, AniCaptured );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pControlPointSFX->GetServerObj(), pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, hAni, true );
			bPlayedAni = true;
		}
	}
	// If we don't have an animation, then make sure we're not playing an old one.
	if( !bPlayedAni && g_pModelLT->GetPlaying( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER ) == LT_YES )
	{
		g_pModelLT->ResetAnim( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER );
	}

	// Notify player of event.
	char const* pszTransmissionId = bLocalCP ? 
		DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), WeCapturedCPMessage ) : 
		DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), TheyCapturedCPMessage );
	DoCPEventMessage( bLocalCP ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );


	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team0_OnEnter
//
//	PURPOSE:	Handle a update Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team0_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	g_pHUDMgr->QueueUpdate(kHUDControlPoint);
	return TeamX_OnEnter( kControlLevel_Team0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team0_OnUpdate
//
//	PURPOSE:	Handle a update Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team0_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	if( m_fControlLevel > kControlLevel_Team0 && m_bCharactersInZone )
	{
		PlayCapturingSound( );
	}
	else
	{
		StopCapturingSound( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team0_OnExit
//
//	PURPOSE:	Handle a exit Team0 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team0_OnExit( MacroStateMachine::EventParams& eventParams )
{
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team1_OnEnter
//
//	PURPOSE:	Handle a update Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team1_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	g_pHUDMgr->QueueUpdate(kHUDControlPoint);
	return TeamX_OnEnter( kControlLevel_Team1 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team1_OnUpdate
//
//	PURPOSE:	Handle a update Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team1_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	if( m_fControlLevel < kControlLevel_Team1 && m_bCharactersInZone )
	{
		PlayCapturingSound( );
	}
	else
	{
		StopCapturingSound( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team1_OnExit
//
//	PURPOSE:	Handle a exit Team1 state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Team1_OnExit( MacroStateMachine::EventParams& eventParams )
{
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Neutral_OnEnter
//
//	PURPOSE:	Handle a enter Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Neutral_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	SetOwnerClientFx( );
	SetNavMarkerFX( );
	SetStageClientFx( );
	g_pHUDMgr->QueueUpdate(kHUDControlPoint);


	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Figure out the previous teamid.
	ETeamId ePreviousTeamId;
	if( eventParams.m_nPreviousState == kControlPointState_Team0 )
		ePreviousTeamId = kTeamId0;
	else if( eventParams.m_nPreviousState == kControlPointState_Team1 )
		ePreviousTeamId = kTeamId1;
	else
		ePreviousTeamId = INVALID_TEAM;

	// Play the neutral animation.
	bool bPlayedAni = false;
	char const* pszAniName = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, AniNeutral );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pControlPointSFX->GetServerObj(), pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER, hAni, true );
			bPlayedAni = true;
		}
	}
	// If we don't have an animation, then make sure we're not playing an old one.
	if( !bPlayedAni && g_pModelLT->GetPlaying( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER ) == LT_YES )
	{
		g_pModelLT->ResetAnim( m_pControlPointSFX->GetServerObj(), MAIN_TRACKER );
	}

	// If the CP was controlled by a team, then show message.
	if( ePreviousTeamId != INVALID_TEAM )
	{
		bool bPreviousLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( ePreviousTeamId );
		// They neutralized our CP.
		if( bPreviousLocalCP )
		{
			DoCPEventMessage( kMsgOtherTeam, DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), TheyNeutralizedCPMessage ));
		}
		// We neutralized their CP.
		else
		{
			DoCPEventMessage( kMsgTeam, DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), WeNeutralizedCPMessage ));
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Team0_OnUpdate
//
//	PURPOSE:	Handle a update Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Neutral_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	if( m_bCharactersInZone )
	{
		PlayCapturingSound( );
	}
	else
	{
		StopCapturingSound( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::Neutral_OnExit
//
//	PURPOSE:	Handle a exit Neutral state.
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::Neutral_OnExit( MacroStateMachine::EventParams& eventParams )
{
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointStateMachine::GetTeamId
//
//	PURPOSE:	Gets the team that owns the control point.
//
// ----------------------------------------------------------------------- //
inline ETeamId ControlPointSFXStateMachine::GetTeamId( ) const
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
//  ROUTINE:	ControlPointSFXStateMachine::SetOwnerClientFx
//
//  PURPOSE:	Sets up the clientfx to show who owns the CP.
//
// ----------------------------------------------------------------------- //
void ControlPointSFXStateMachine::SetOwnerClientFx( )
{
	switch( GetTeamId( ))
	{
	// Use a teamclientfx for when the control point is captured.
	case kTeamId0:
	case kTeamId1:
		{
			CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

			// Shutdown the old captured teamclientfx.
			m_CapturedTeamClientFx.Term();

			// Shutdown the old neutral clientfx.
			pClientFXMgr->ShutdownClientFX( &m_NeutralClientFxLink );
			m_NeutralClientFxLink.ClearLink();

			// Create the new captured teamclientfx.
			TEAMCLIENTFXCREATESTRUCT cs;
			cs.m_hParentObject = m_pControlPointSFX->GetServerObj( );
			cs.m_hTeamClientFXRec = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, TeamClientFxCaptured );
			cs.m_nTeamId = GetTeamId();
			m_CapturedTeamClientFx.Init( &cs );
		}
		break;
	// Just use a regular clientfx when the control point is neutral.
	case INVALID_TEAM:
	default:
		{
			CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

			// Shutdown the old captured teamclientfx.
			m_CapturedTeamClientFx.Term();

			// Shutdown the old neutral clientfx.
			pClientFXMgr->ShutdownClientFX( &m_NeutralClientFxLink );
			m_NeutralClientFxLink.ClearLink();

			// Create the new neutral clientfx.
			char const* pszClientFx = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, ClientFxNeutral );
			if( !LTStrEmpty( pszClientFx ))
			{
				CLIENTFX_CREATESTRUCT fxInit( pszClientFx, FXFLAG_LOOP, m_pControlPointSFX->GetServerObj( )); 
				pClientFXMgr->CreateClientFX( &m_NeutralClientFxLink, fxInit, true );
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFXStateMachine::SetStageClientFx
//
//  PURPOSE:	Sets up the clientfx to show what stage of ownership the CP is at.
//
// ----------------------------------------------------------------------- //
void ControlPointSFXStateMachine::SetStageClientFx( )
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

	// Get the attribute describing the stages.
	HATTRIBUTE hStagesStruct = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( 
		m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, Stages );
	if( !hStagesStruct )
	{
		LTERROR( "Invalid ControlPoint stages specified" );
		return;
	}

	// Get the number of stages defined.
	uint32 nNumStages = DATABASE_CATEGORY( CPTypes ).GetNumValues( hStagesStruct );
	if( nNumStages == 0 )
		return;

	// Figure out what team currently has some control.
	ETeamId eControlLevelTeamId = ( m_fControlLevel < 0.0f ) ? kTeamId0 : ( m_fControlLevel > 0.0f ) ? kTeamId1 : INVALID_TEAM;

	TEAMCLIENTFXCREATESTRUCT cs;
	cs.m_hParentObject = m_pControlPointSFX->GetServerObj( );

	// The stage used rounds down towards the current owner.  This is done to make sure that taking a CP away from
	// its current owner has the last stage reached at 100%.
	uint32 nStage = 0;
	float fAbsControlLevel = fabs( m_fControlLevel );
	uint32 nNumStagesBase0 = nNumStages - 1;
	switch( GetTeamId( ))
	{
	case kTeamId0:
	case kTeamId1:
		// Round toward positive values.
		nStage = ( nNumStagesBase0 ) - ( uint32 )(( 1.0f - fAbsControlLevel ) * nNumStagesBase0 );
		break;
	default:
	case INVALID_TEAM:
		// Round toward 0.
		nStage = ( uint32 )( fAbsControlLevel * nNumStagesBase0 );
		break;
	}

	// Check if we haven't changed anything.
	if( nStage == m_nLastStage && eControlLevelTeamId == m_eLastTeamId )
		return;

	// Shutdown the old stage teamclientfx.
	m_StageTeamClientFx.Term();

	// Save the current as the values for to check next time.
	m_nLastStage = nStage;
	m_eLastTeamId = eControlLevelTeamId;

	// Use the stage to find out what teamclientfx to use.
	switch( eControlLevelTeamId )
	{
	case kTeamId0:
	case kTeamId1:
		cs.m_hTeamClientFXRec = DATABASE_CATEGORY( CPTypes ).GETSTRUCTATTRIB( Stages, hStagesStruct, nStage, TeamClientFxCapturedStage );
		break;
	default:
	case INVALID_TEAM:
		cs.m_hTeamClientFXRec = DATABASE_CATEGORY( CPTypes ).GETSTRUCTATTRIB( Stages, hStagesStruct, nStage, TeamClientFxNeutralStage );
		break;
	}

	// Check if no clientfx specified for this stage.
	if( !cs.m_hTeamClientFXRec )
		return;

	// Create the new captured teamclientfx.
	cs.m_nTeamId = eControlLevelTeamId;
	m_StageTeamClientFx.Init( &cs );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFXStateMachine::SetNavMarkerFX
//
//  PURPOSE:	Sets up the navmarker fx.
//
// ----------------------------------------------------------------------- //
void ControlPointSFXStateMachine::SetNavMarkerFX( )
{
	NAVMARKERCREATESTRUCT navMarker;
	navMarker.m_bInstant = false;
	navMarker.m_bIsActive = true;
	navMarker.m_hTarget = m_pControlPointSFX->GetServerObj();

	wchar_t wszNavMarkerString[32] = L"";
	LTSNPrintF( wszNavMarkerString, LTARRAYSIZE( wszNavMarkerString ), L"%d", m_pControlPointSFX->GetCS().m_nControlPointId );
	navMarker.m_wsString = wszNavMarkerString;

	switch( GetTeamId( ))
	{
	case 0:
	case 1:
		{
			// Remove the neutral navmarker.  When captured by either team, we only show the team context navmarkers.
			if( m_pNeutralNavMarker )
			{
				m_pNeutralNavMarker->WantRemove( true );
				m_pNeutralNavMarker = NULL;
			}

			// Make a friendly navmarker for this object.
			if( !m_pFriendlyNavMarker )
			{
				CAutoMessage cMsg;
				navMarker.m_nTeamId = GetTeamId();
				navMarker.m_hType = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, FriendlyNavMarker );
				cMsg.Writeuint8( SFX_NAVMARKER_ID );
				navMarker.Write( cMsg );
				m_pFriendlyNavMarker = static_cast< CNavMarkerFX* >( g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( navMarker.m_hTarget, cMsg.Read()));
			}
			else
			{
				m_pFriendlyNavMarker->SetTeamId( GetTeamId( ));
			}

			// Make an enemy navmarker for this object.
			if( !m_pEnemyNavMarker )
			{
				CAutoMessage cMsg;
				navMarker.m_nTeamId = !GetTeamId();
				navMarker.m_hType = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, EnemyNavMarker );
				cMsg.Writeuint8( SFX_NAVMARKER_ID );
				navMarker.Write( cMsg );
				m_pEnemyNavMarker = static_cast< CNavMarkerFX* >( g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( navMarker.m_hTarget, cMsg.Read()));
			}
			else
			{
				m_pEnemyNavMarker->SetTeamId( !GetTeamId( ));
			}
		}
		break;
	case INVALID_TEAM:
	default:
		{
			// Remove the captured navmarkers.  When neutral, we don't have team context navmarkers.
			if( m_pFriendlyNavMarker )
			{
				m_pFriendlyNavMarker->WantRemove( true );
				m_pFriendlyNavMarker = NULL;
			}
			if( m_pEnemyNavMarker )
			{
				m_pEnemyNavMarker->WantRemove( true );
				m_pEnemyNavMarker = NULL;
			}

			// Make an enemy navmarker for this object.
			if( !m_pNeutralNavMarker )
			{
				CAutoMessage cMsg;
				navMarker.m_nTeamId = INVALID_TEAM;
				navMarker.m_hType = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, NeutralNavMarker );
				cMsg.Writeuint8( SFX_NAVMARKER_ID );
				navMarker.Write( cMsg );
				m_pNeutralNavMarker = static_cast< CNavMarkerFX* >( g_pGameClientShell->GetSFXMgr()->HandleSFXMsg( navMarker.m_hTarget, cMsg.Read()));
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DoPlayerEventMessage
//
//  PURPOSE:	Common function used to post messages due to a player event received by OnServerMessage.
//
// ----------------------------------------------------------------------- //
inline bool DoPlayerEventMessage( ILTMessage_Read* pMsg, char const* pszGameMsgFormat, eChatMsgType eType, char const* pszTransmissionStringId )
{
	// Read in the player doing the event.
	uint8 nClientId = pMsg->Readuint8( );
	// If this is the local player, then let him know his event.
	CLIENT_INFO *pClient = g_pInterfaceMgr->GetClientInfoMgr( )->GetClientByID( nClientId );
	if( pClient && pClient == g_pInterfaceMgr->GetClientInfoMgr( )->GetLocalClient( ) && !LTStrEmpty( pszGameMsgFormat ))
	{
		// Write out the event to the message log.
		wchar_t wszTmp[128] = L"";
		FormatString( pszGameMsgFormat, wszTmp, LTARRAYSIZE( wszTmp ), pClient->sName.c_str( ));
		g_pGameMsgs->AddMessage( wszTmp, eType );
	}

	// Do the transmission.
	if( !LTStrEmpty( pszTransmissionStringId ))
	{
		const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId( "Dialogue", pszTransmissionStringId );
		if( !LTStrEmpty( pszFilename ))
		{
			g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
		}

		g_pTransmission->Show( LoadString( pszTransmissionStringId ));
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFXStateMachine::OnEventMessage
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //
void ControlPointSFXStateMachine::OnEventMessage( ILTMessage_Read *pMsg )
{
	GameModeMgr& gameModeMgr = GameModeMgr::Instance( );

	// Handle the event.
	EControlPointFXMsgId eMsgId = ( EControlPointFXMsgId )pMsg->ReadBits( FNumBitsExclusive<kControlPointFXMsg_NumIds>::k_nValue );
	switch( eMsgId )
	{
	case kControlPointFXMsg_ControlLevel:
		{
			// Control level is mapped to 8 bits for optimization.
			m_fControlLevel = ( pMsg->Readuint8() / 127.5f ) - 1.0f;

			m_bCharactersInZone = pMsg->Readbool( );

			// Play the capturing sounds or stop it if no one is capturing.
			if( m_bCharactersInZone )
			{
				PlayCapturingSound( );
			}
			else
			{
				StopCapturingSound( );
			}

			// Update our stage level.
			SetStageClientFx( );
		}
		break;
	case kControlPointFXMsg_PlayerCapture:
		{
			// Check if the CP is under our control.
			bool bLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( GetTeamId( ));

			// Notify player of event.
			char const* pszTransmissionId = bLocalCP ? 
				DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), TheyCapturedCPMessage ) : 
				DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), WeCapturedCPMessage );
			char const* pszGameMsgFormat = DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), PlayerCaptureStringId );
			DoPlayerEventMessage( pMsg, pszGameMsgFormat, bLocalCP ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
		}
		break;
	case kControlPointFXMsg_PlayerFixup:
		{
			// Check if the CP is under our control.
			bool bLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( GetTeamId( ));

			// Notify player of event.
			char const* pszGameMsgFormat = DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), PlayerFixupStringId );
			DoPlayerEventMessage( pMsg, pszGameMsgFormat, bLocalCP ? kMsgTeam : kMsgOtherTeam, NULL );
		}
		break;
	case kControlPointFXMsg_PlayerNeutralize:
		{
			// Check if the CP is under our control.
			bool bLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( GetTeamId( ));

			// Notify player of event.
			char const* pszTransmissionId = bLocalCP ? 
				DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), TheyNeutralizedCPMessage ) : 
				DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), WeNeutralizedCPMessage );
			char const* pszGameMsgFormat = DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), PlayerNeutralizeStringId );
			DoPlayerEventMessage( pMsg, pszGameMsgFormat, bLocalCP ? kMsgOtherTeam : kMsgTeam, pszTransmissionId );
		}
		break;
	case kControlPointFXMsg_PlayerControlPointDefend:
		{
			// Check if the CP is under our control.
			bool bLocalCP = g_pInterfaceMgr->GetClientInfoMgr( )->IsLocalTeam( GetTeamId( ));

			// Notify player of event.
			char const* pszGameMsgFormat = DATABASE_CATEGORY( CPRules ).GETRECORDATTRIB( gameModeMgr.GetCPRulesRecord(), PlayerDefendStringId );
			DoPlayerEventMessage( pMsg, pszGameMsgFormat, bLocalCP ? kMsgTeam : kMsgOtherTeam, NULL );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::PlayCapturingSound
//
//	PURPOSE:	Play the sound when characters are capturing
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::PlayCapturingSound( )
{
	// See if already playing.
	if( m_hCapturingSound )
		return true;

	// Get the sound record to use.
	HRECORD hSoundRec = DATABASE_CATEGORY( CPTypes ).GETRECORDATTRIB( m_pControlPointSFX->m_csControlPoint.m_hControlPointRec, CapturingSoundFX );
	if( !hSoundRec )
		return false;

	// Play the sound looping and save the handle.
	m_hCapturingSound = g_pClientSoundMgr->PlayDBSoundFromObject( m_pControlPointSFX->GetServerObj(), hSoundRec, SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP) );
	if( !m_hCapturingSound )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ControlPointSFXStateMachine::StopCapturingSound
//
//	PURPOSE:	Stop the playing sound when characters are capturing
//
// ----------------------------------------------------------------------- //
bool ControlPointSFXStateMachine::StopCapturingSound( )
{
	// See if already playing.
	if( !m_hCapturingSound )
		return true;

	g_pLTClient->SoundMgr()->KillSound( m_hCapturingSound );
	m_hCapturingSound = NULL;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::ControlPointSFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

ControlPointSFX::ControlPointSFX( )
{
	m_pControlPointSFXStateMachine = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::~ControlPointSFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

ControlPointSFX::~ControlPointSFX( )
{
	if( m_pControlPointSFXStateMachine )
	{
		delete m_pControlPointSFXStateMachine;
		m_pControlPointSFXStateMachine = NULL;
	}

	// Erase this instance from the list.
	ControlPointSFXList::iterator it = std::find( m_lstControlPointSFX.begin( ), m_lstControlPointSFX.end( ), this );
	if( it != m_lstControlPointSFX.end( ))
	{
		m_lstControlPointSFX.erase( it );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool ControlPointSFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	// Make sure there's a cp rules defined.
	if( !GameModeMgr::Instance( ).GetCPRulesRecord( ))
		return false;

	m_csControlPoint.Read( pMsg );

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pControlPointSFXStateMachine = new ControlPointSFXStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pControlPointSFXStateMachine->Init( *this );
	m_pControlPointSFXStateMachine->SetState( m_csControlPoint.m_eControlPointState );

	// insert this instance to a list of all of this type.
	ControlPointSFXList::iterator it = m_lstControlPointSFX.begin();
	while (it != m_lstControlPointSFX.end() && ((*it)->GetCS().m_nControlPointId < GetCS().m_nControlPointId))
	{
		it++;
	}
	m_lstControlPointSFX.insert( it, this );
	


	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::Update
//
//  PURPOSE:	Update the weapon associated with the turret...
//
// ----------------------------------------------------------------------- //

bool ControlPointSFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	m_pControlPointSFXStateMachine->Update();

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::OnServerMessage
//
//  PURPOSE:	Handle a message recieved from the server side Turret object...
//
// ----------------------------------------------------------------------- //

bool ControlPointSFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	m_csControlPoint.m_eControlPointState = ( EControlPointState )pMsg->ReadBits( FNumBitsExclusive<kControlPointState_NumStates>::k_nValue );

	// See if there is any event message info.
	if( pMsg->Readbool( ))
	{
		m_pControlPointSFXStateMachine->OnEventMessage( pMsg );
	}

	// Update our state.
	if( !m_pControlPointSFXStateMachine->SetState( m_csControlPoint.m_eControlPointState ))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::GetControlPoint
//
//  PURPOSE:	retrieve the control point associated with a particular id
//
// ----------------------------------------------------------------------- //
ControlPointSFX* ControlPointSFX::GetControlPoint(uint16 nID)
{
	// Erase this instance from the list.
	ControlPointSFXList::iterator it = m_lstControlPointSFX.begin( );
	while( it != m_lstControlPointSFX.end( ))
	{
		ControlPointSFX* pCP = *it;
		if (pCP && pCP->GetCS().m_nControlPointId == nID)
		{
			return pCP;
		}
		it++;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::GetControlLevel
//
//  PURPOSE:	Get the level of the control.
//
// ----------------------------------------------------------------------- //
float ControlPointSFX::GetControlLevel( ) const
{
	if( !m_pControlPointSFXStateMachine )
		return 0.0f;

	return m_pControlPointSFXStateMachine->GetControlLevel();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ControlPointSFX::GetTeamId
//
//  PURPOSE:	Get the ETeamId that owns the CP.
//
// ----------------------------------------------------------------------- //
ETeamId ControlPointSFX::GetTeamId( ) const
{
	if( !m_pControlPointSFXStateMachine )
		return INVALID_TEAM;

	switch( m_pControlPointSFXStateMachine->GetState( ))
	{
	case kControlPointState_Team0:
		return kTeamId0;
		break;
	case kControlPointState_Team1:
		return kTeamId1;
		break;
	default:
		return INVALID_TEAM;
	};
}
