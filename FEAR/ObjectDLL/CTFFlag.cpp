// ----------------------------------------------------------------------- //
//
// MODULE  : CTFFlag.cpp
//
// PURPOSE : CTFFlag object to place in CTF level.
//
// CREATED : 05/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "CTFFlag.h"
#include "CTFFlagBase.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "PropsDB.h"
#include "GameModeMgr.h"
#include "PlayerInventory.h"
#include "StateMachine.h"
#include "TeamMgr.h"
#include "ServerMissionMgr.h"
#include "ServerConnectionMgr.h"

LINKFROM_MODULE( CTFFlag )

#ifndef _FINAL
#define DEBUG_DEFEND_MESSAGES
#endif

// Statemachine to handle Flag states.
class FlagStateMachine : public MacroStateMachine
{
public:

	FlagStateMachine( )
	{
		m_pFlag = NULL;
		m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}

	~FlagStateMachine( )
	{
		Term( );
	}

	// Initialize to a flagbase.
	bool Init( CTFFlag& flag )
	{
		m_pFlag = &flag;
		m_StateTimer.SetEngineTimer( RealTimeTimer::Instance( ));
		m_CaptureAssistTimer.SetEngineTimer( RealTimeTimer::Instance( ));

		// Listen for player kill events.
		m_delegatePlayerKillEvent.Attach( this, NULL, CPlayerObj::PlayerScoredKillEvent );

		return true;
	}

	void Term( )
	{
		// Cleanup the rigidbody.
		ReleaseRigidBody( );

		// No longer listen for player kill events.
		m_delegatePlayerKillEvent.Detach();
	}

	// Send touch event to state.
	bool DoStealEvent( HOBJECT hStealingPlayer )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hStealingPlayer;
		return DoUserEvent( eFlagEvent_Steal, eventParams );
	}

	// Send touch event to state.
	bool DoTouchEvent( HOBJECT hToucher )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hToucher;
		return DoUserEvent( eFlagEvent_Touched, eventParams );
	}

	// Send pickup event to state.
	bool DoPickedUpEvent( HOBJECT hPlayer )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hPlayer;
		return DoUserEvent( eFlagEvent_PickedUp, eventParams );
	}

	// Send return event to state.
	bool DoReturnEvent( HOBJECT hPlayer )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hPlayer;
		return DoUserEvent( eFlagEvent_Return, eventParams );
	}

	// Send auto return event to state.
	bool DoAutoReturnEvent( )
	{
		EventParams cParams;
		return DoUserEvent( eFlagEvent_AutoReturn, cParams );
	}

	// Send drop event to state.
	bool DoDropEvent( HOBJECT hDropper )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hDropper;
		return DoUserEvent( eFlagEvent_Dropped, eventParams );
	}

	// Send capture event to state.
	bool DoCaptureEvent( )
	{
		EventParams cParams;
		return DoUserEvent( eFlagEvent_Capture, cParams );
	}

	// Send capture assist event to state.
	bool DoCaptureAssistEvent( HOBJECT hFlagCapPlayer )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hFlagCapPlayer;
		return DoUserEvent( eFlagEvent_CaptureAssist, eventParams );
	}

	// Send activated event to state.
	bool DoActivatedEvent( HOBJECT hPlayer )
	{
		FlagEventParams eventParams;
		eventParams.m_hObject = hPlayer;
		return DoUserEvent( eFlagEvent_Activated, eventParams );
	}

	// Send player kill event to state.
	bool DoPlayerKillEvent( HOBJECT hKiller, HOBJECT hVictim )
	{
		PlayerKillEventParams eventParams;
		eventParams.m_hKiller = hKiller;
		eventParams.m_hVictim = hVictim;
		return DoUserEvent( eFlagEvent_PlayerKilled, eventParams );
	}

	// Sends the client an event message.
	void SendClientEventMessage( uint32 nState, ILTMessage_Write* pEventMsg );

	// override of MacroStateMachine.
	using MacroStateMachine::SetState;
	virtual bool SetState( uint32 nNewState, ILTMessage_Write* pMsg );

protected:

	// Handles rigidbody for flag.
	bool CreateRigidBody( HOBJECT hDropper );
	void ReleaseRigidBody( );
	bool UpdateRigidBody( );

	// Statemachine event handlers.
	bool InBase_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnSteal( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnPlayerKill( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnCaptureAssist( MacroStateMachine::EventParams& eventParams );
	bool InBase_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnDropped( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnCapture( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnPlayerKill( MacroStateMachine::EventParams& eventParams );
	bool Carried_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnTouched( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnPickedUp( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnReturn( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnAutoReturn( MacroStateMachine::EventParams& eventParams );
	bool Loose_OnExit( MacroStateMachine::EventParams& eventParams );

	// Flag event paramaters.
	struct FlagEventParams : public EventParams
	{
		FlagEventParams( )
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

	// Statemachine events for flagbase.
	enum EFlagEvent
	{
		// Flag stolen.
		eFlagEvent_Steal = EVENT_User,
		// Flag touched.
		eFlagEvent_Touched,
		// Flag pickedup.
		eFlagEvent_PickedUp,
		// Flag return.
		eFlagEvent_Return,
		// Flag auto return.
		eFlagEvent_AutoReturn,
		// Flag dropped.
		eFlagEvent_Dropped,
		// Flag capture.
		eFlagEvent_Capture,
		// Flag Activated.
		eFlagEvent_Activated,
		// Player killed event.
		eFlagEvent_PlayerKilled,
		// Flag capture assist.
		eFlagEvent_CaptureAssist,
	};

	// State table.
	MSM_BeginTable( FlagStateMachine )
		MSM_BeginState( kCTFFlagState_InBase )
			MSM_OnEnter( InBase_OnEnter )
			MSM_OnEvent( eFlagEvent_Steal, InBase_OnSteal )
			MSM_OnEvent( eFlagEvent_Activated, InBase_OnSteal )
			MSM_OnEvent( eFlagEvent_PlayerKilled, InBase_OnPlayerKill )
			MSM_OnEvent( eFlagEvent_CaptureAssist, InBase_OnCaptureAssist )
			MSM_OnExit( InBase_OnExit )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagState_Carried )
			MSM_OnEnter( Carried_OnEnter )
			MSM_OnEvent( eFlagEvent_Dropped, Carried_OnDropped )
			MSM_OnEvent( eFlagEvent_Capture, Carried_OnCapture )
			MSM_OnEvent( eFlagEvent_PlayerKilled, Carried_OnPlayerKill )
			MSM_OnExit( Carried_OnExit )
		MSM_EndState( )
		MSM_BeginState( kCTFFlagState_Loose )
			MSM_OnEnter( Loose_OnEnter )
			MSM_OnUpdate( Loose_OnUpdate )
			MSM_OnEvent( eFlagEvent_Touched, Loose_OnTouched )
			MSM_OnEvent( eFlagEvent_PickedUp, Loose_OnPickedUp )
			MSM_OnEvent( eFlagEvent_Return, Loose_OnReturn )
			MSM_OnEvent( eFlagEvent_AutoReturn, Loose_OnAutoReturn )
			MSM_OnEvent( eFlagEvent_Activated, Loose_OnTouched )
			MSM_OnExit( Loose_OnExit )
		MSM_EndState( )
	MSM_EndTable( )

	// Declare delegate to listen for player to do dropinventory event.
	static void OnPlayerDropInventory( FlagStateMachine* pFlagStateMachine, CPlayerObj* pPlayerObj, EventCaster::NotifyParams& notifyParams )
	{
		pFlagStateMachine->DoDropEvent( pPlayerObj->m_hObject );
	}
	Delegate< FlagStateMachine, CPlayerObj, FlagStateMachine::OnPlayerDropInventory > m_delegatePlayerDropInventory;

	// Declare delegate to listen for player kill events.
	static void OnPlayerKillEvent( FlagStateMachine* pFlagStateMachine, CPlayerObj* pPlayerObj, EventCaster::NotifyParams& notifyParams )
	{
		CPlayerObj::PlayerScoredKillEventParams& playerScoredKillNotifyParams = ( CPlayerObj::PlayerScoredKillEventParams& )notifyParams;
		pFlagStateMachine->DoPlayerKillEvent( playerScoredKillNotifyParams.m_hKiller, playerScoredKillNotifyParams.m_hVictim );
	}
	Delegate< FlagStateMachine, CPlayerObj, FlagStateMachine::OnPlayerKillEvent > m_delegatePlayerKillEvent;

	// Declare our delegate to receive playerswitched events.
	static void OnPlayerSwitched( FlagStateMachine* pFlagStateMachine, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams )
	{
		// Switch our assist player to his new player.
		pFlagStateMachine->m_hCaptureAssistPlayer = pGameClientData->GetPlayer( );
	}
	Delegate< FlagStateMachine, GameClientData, FlagStateMachine::OnPlayerSwitched > m_delegatePlayerSwitched;

private:

	// The flag that owns us.
	CTFFlag* m_pFlag;

	// Timer used by states.
	StopWatchTimer m_StateTimer;

	// Capture assist timer.
	StopWatchTimer m_CaptureAssistTimer;
	LTObjRef m_hCaptureAssistPlayer;

	// Rigidbody for flag.
	HPHYSICSRIGIDBODY	m_hRigidBody;
	LTRigidTransform	m_tfRigidBodyOffset;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::CreateRigidBody
//
//	PURPOSE:	create the shape and body for the physics sim...
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::CreateRigidBody( HOBJECT hDropper )
{
	// Start fresh.
	ReleaseRigidBody();

	HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	if( g_pLTServer->PhysicsSim()->GetModelRigidBody( m_pFlag->m_hObject, 0, hRigidBody ) != LT_OK )
	{
		return false;
	}

	// Get info from the model rigidbody.
	LTRigidTransform tfBody;
	g_pLTServer->PhysicsSim()->GetRigidBodyTransform(hRigidBody,tfBody);

	EPhysicsGroup eGroup;
	uint32 nSystem;
	g_pLTServer->PhysicsSim()->GetRigidBodyCollisionInfo(hRigidBody, eGroup, nSystem);

	float fFriction;
	g_pLTServer->PhysicsSim()->GetRigidBodyFriction(hRigidBody,fFriction);
	float fCOR;
	g_pLTServer->PhysicsSim()->GetRigidBodyCOR(hRigidBody,fCOR);

	HPHYSICSSHAPE hShape;
	g_pLTServer->PhysicsSim()->GetRigidBodyShape(hRigidBody,hShape);

	// Copy the rigidbody.
	m_hRigidBody = g_pLTServer->PhysicsSim()->CreateRigidBody(hShape,tfBody,false,eGroup,nSystem,fFriction,fCOR);

	// Release the references to the model rigidbody.
	g_pLTServer->PhysicsSim()->ReleaseShape(hShape);
	hShape = NULL;
	g_pLTServer->PhysicsSim()->ReleaseRigidBody(hRigidBody);
	hRigidBody = INVALID_PHYSICS_RIGID_BODY;

	// Get a transform of the game object
	LTRigidTransform tfFlag;
	g_pLTServer->GetObjectTransform( m_pFlag->m_hObject, &tfFlag );
	tfFlag.m_rRot = tfBody.m_rRot;

	// Get the offest transform.  We'll use this during the update.
	m_tfRigidBodyOffset.Difference( tfBody, tfFlag );

	// Clear our velocity.
	LTVector vVel(0, 0, 0);
	g_pPhysicsLT->SetVelocity( m_pFlag->m_hObject, vVel );
	g_pLTServer->PhysicsSim()->SetRigidBodyVelocity(m_hRigidBody, vVel);

	// Give the flag a little push.
	LTVector vImpulse = tfFlag.m_rRot.Forward() + tfFlag.m_rRot.Up();
	vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( hDropper );
	if( pPlayerObj && pPlayerObj->GetDestructible()->IsDead() && pPlayerObj->GetDestructible()->GetDeathType() == DT_EXPLODE)
	{
		vImpulse += pPlayerObj->GetDestructible()->GetDeathDir() * pPlayerObj->GetDestructible()->GetDeathImpulseForce();
	}
	g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(m_hRigidBody, tfBody.m_vPos, vImpulse);

	// Spin the flag a little.
	LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
	g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity( m_hRigidBody, vAng );

	g_pLTServer->PhysicsSim()->SetRigidBodySolid(m_hRigidBody,true);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::ReleaseRigidBody
//
//	PURPOSE:	free the shape and body for the physics sim...
//
// ----------------------------------------------------------------------- //
void FlagStateMachine::ReleaseRigidBody()
{
	if( m_hRigidBody == INVALID_PHYSICS_RIGID_BODY )
		return;

	g_pLTServer->PhysicsSim()->RemoveRigidBodyFromSimulation(m_hRigidBody);
	g_pLTServer->PhysicsSim()->ReleaseRigidBody(m_hRigidBody);
	m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::UpdateRigidBody
//
//	PURPOSE:	Updates the rigidbody position.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::UpdateRigidBody()
{
	if( m_hRigidBody == INVALID_PHYSICS_RIGID_BODY )
		return false;

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_pFlag->m_hObject, &vPos );

	bool bMoving = false;
	LTVector vVel;
	LTVector vAngVel;
	g_pLTServer->PhysicsSim()->GetRigidBodyVelocity(m_hRigidBody,vVel);
	g_pLTServer->PhysicsSim()->GetRigidBodyAngularVelocity(m_hRigidBody,vAngVel);

	// Check if we're still moving beyond a threshold.
	bMoving = vVel.MagSqr() > 5.0f || vAngVel.MagSqr() > 0.5f;
	if( bMoving )
	{
		LTRigidTransform tRTrans;
		g_pLTServer->PhysicsSim()->GetRigidBodyTransform(m_hRigidBody,tRTrans);
		LTRigidTransform tNewPos = tRTrans * m_tfRigidBodyOffset;
		g_pLTServer->SetObjectRotation( m_pFlag->m_hObject, tNewPos.m_rRot );

		// Update the main objects velocity based on the rigidbody.
		g_pLTServer->Physics()->SetVelocity( m_pFlag->m_hObject, vVel );

		// Try to move the main object to the rigidbody position.
		g_pLTServer->Physics()->MoveObject( m_pFlag->m_hObject, tNewPos.m_vPos, 0);

		// See where we got to.
		g_pLTServer->GetObjectPos(m_pFlag->m_hObject, &vPos);

		// If we didn't get there, see if we hit the world.
		if( !vPos.NearlyEquals( tNewPos.m_vPos, 10.0f ))
		{
			//only test against the main world model
			HOBJECT hMainWorld = g_pLTBase->GetMainWorldModel();
			IntersectQuery query;
			IntersectInfo info;
			query.m_From		= vPos;
			query.m_To			= tNewPos.m_vPos;
			query.m_Flags		= INTERSECT_HPOLY;

			//test this segment against the world to see what we bounce against
			if( g_pLTBase->IntersectSegmentAgainst(query, &info, hMainWorld ))
			{
				// get the normal of the plane we impacted with
				LTPlane plane = info.m_Plane;
				if (plane.m_Normal.MagSqr() == 0.0f)
				{
					vVel.Normalize();
					vVel *= -10.0f;
				}
				else
				{
					float f = vVel.Mag();
					LTVector vNormal = plane.m_Normal;
					vNormal.Normalize();
					//reflect the velocity over the normal
					vVel -= vNormal * (2.0f * vVel.Dot(vNormal));
					// Dampen the reflection by a little.
					vVel *= 0.3f;
				}
			}

			tRTrans.m_vPos = vPos;
			tRTrans.m_rRot.Init();
			tRTrans *= m_tfRigidBodyOffset.GetInverse();
			g_pLTServer->PhysicsSim()->TeleportRigidBody(m_hRigidBody,tRTrans);

			g_pLTServer->Physics()->SetVelocity( m_pFlag->m_hObject, vVel );
			g_pLTServer->PhysicsSim()->SetRigidBodyVelocity(m_hRigidBody,vVel);
			g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity(m_hRigidBody,LTVector(0.0f,0.0f,0.0f));
		}
	}
	else
	{
		g_pLTServer->Physics()->SetVelocity( m_pFlag->m_hObject, LTVector(0,0,0));
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::SendClientEventMessage
//
//	PURPOSE:	Sends event message to client.
//
// ----------------------------------------------------------------------- //
void FlagStateMachine::SendClientEventMessage( uint32 nState, ILTMessage_Write* pEventMsg )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CTFFLAG_ID );
	cMsg.WriteObject( m_pFlag->m_hObject );
	cMsg.WriteBits( nState, FNumBitsExclusive<kCTFFlagState_NumStates>::k_nValue );
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
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::SetState
//
//	PURPOSE:	Override of MacroStateMachine.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::SetState( uint32 nNewState, ILTMessage_Write* pEventMsg )
{
	// Let parent handle it first.
	if( !MacroStateMachine::SetState( nNewState ))
		return false;

	// Send the event message to client.
	SendClientEventMessage( nNewState, pEventMsg );

	// Update the main sfx message.
	m_pFlag->CreateSpecialFX( false );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::InBase_OnEnter
//
//	PURPOSE:	Handle a enter InBase state.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::InBase_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Tell base it has its flag back.
	pFlagBase->ReturnFlag();

	// Make sure there is no flagcarrier.
	if( m_pFlag->m_hFlagCarrier )
	{
		CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( m_pFlag->m_hFlagCarrier );
		if( pPlayerObj )
		{
			pPlayerObj->GetInventory()->SetCTFFlag( NULL );
		}

		m_pFlag->m_hFlagCarrier = NULL;
	}

	// Don't allow touch notifies while in the base.
	g_pCommonLT->SetObjectFlags( m_pFlag->m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY );

	// Play the inbase animation.
	char const* pszAniName = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagAniInBase );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pFlag->m_hObject, pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pFlag->m_hObject, MAIN_TRACKER, hAni, true );
		}
	}

	// Attach the flag to the flagbase.
	char const* pszBaseSocket = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), BaseSocket );
	HATTACHMENT hAttachment = NULL;
	if( g_pLTServer->CreateAttachment( pFlagBase->m_hObject, m_pFlag->m_hObject, pszBaseSocket, NULL, NULL, &hAttachment) != LT_OK ||
		hAttachment == NULL )
	{
		LTERROR( "Could not attach flag to flagbase." );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::InBase_OnSteal
//
//	PURPOSE:	Handle a InBase steal event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::InBase_OnSteal( MacroStateMachine::EventParams& eventParams )
{
	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Stealing object must be a player.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Don't allow non-alive players to steal the flag.
	if( !pPlayerObj->IsAlive( ))
		return false;

	// Make sure the player doesn't aleady have a flag.
	if( pPlayerObj->GetInventory()->GetCTFFlag())
	{
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Not allowed to steal own flag.
	if( pPlayerObj->GetTeamID() == pFlagBase->GetTeamId( ))
		return false;

	// Give the player the flag.
	pPlayerObj->GetInventory()->SetCTFFlag( m_pFlag->m_hObject );

	// Remember the flagcarrier.
	m_pFlag->m_hFlagCarrier = pPlayerObj->m_hObject;

	// Tell the base to remove it's flag.
	if( !pFlagBase->RemoveFlag())
		return false;

	// Give points for Steal.
	GameModeMgr& gameModeMgr = GameModeMgr::Instance();
	int32 nStealFlagScorePlayer = gameModeMgr.m_grnCTFStealFlagScorePlayer;
	if( nStealFlagScorePlayer )
	{
		// Give the player some objective points.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nStealFlagScorePlayer );
	}
	int32 nStealFlagScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFStealFlagScoreTeam : 0;
	if( nStealFlagScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pPlayerObj->GetTeamID(), nStealFlagScoreTeam );
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_Steal, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	cEventMsg.WriteObject( m_pFlag->m_hFlagCarrier );
	HCLIENT hPlayerClient = pPlayerObj->GetClient();
	uint8 nPlayerClientId = ( uint8 )g_pLTServer->GetClientID( hPlayerClient );
	cEventMsg.Writeuint8( nPlayerClientId );

	// Change to the carried state.
	if( !SetState( kCTFFlagState_Carried, cEventMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::InBase_OnPlayerKill
//
//	PURPOSE:	Handle a InBase playerkilled event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::InBase_OnPlayerKill( MacroStateMachine::EventParams& eventParams )
{
#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: Entry", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

	PlayerKillEventParams& playerKilledEventParams = ( PlayerKillEventParams& )eventParams;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Make sure they can get points for this kill
	int32 nDefendFlagBaseScorePlayer = gameModeMgr.m_grnCTFDefendFlagBaseScorePlayer;
	int32 nDefendFlagBaseScoreTeam = gameModeMgr.m_grbUseTeams ?  gameModeMgr.m_grnCTFDefendFlagBaseScoreTeam : 0;
	if( !nDefendFlagBaseScorePlayer && !nDefendFlagBaseScoreTeam )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No defend flag base score", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
	int32 nDefendFlagBaseRadius = GameModeMgr::Instance().m_grnCTFDefendFlagBaseRadius;
	if( !nDefendFlagBaseRadius )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No defend flag base radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No flagbase object", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		LTERROR( "Missing flagbase object." );
		return false;
	}

#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: pFlagBase->GetTeamId(%d)", __FUNCTION__, pFlagBase->GetTeamId());
#endif // DEBUG_DEFEND_MESSAGES

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
	if( pKillerPlayerObj->GetTeamID() != pFlagBase->GetTeamId( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: killer on different team than flag", __FUNCTION__);
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
	if( pVictimPlayerObj->GetTeamID() == pFlagBase->GetTeamId( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: victim on same team as flag", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Get the flag position.
	LTVector vFlagPos;
	g_pLTServer->GetObjectPos( m_pFlag->m_hObject, &vFlagPos );

	// Get the position of the victim.
	LTVector vVictimPos;
	g_pLTServer->GetObjectPos( pVictimPlayerObj->m_hObject, &vVictimPos );

	// Get the position of the killer.
	LTVector vKillerPos;
	g_pLTServer->GetObjectPos( pKillerPlayerObj->m_hObject, &vKillerPos );

#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: vFlagPos(%.3f,%.3f,%.3f), vVictimPos(%.3f,%.3f,%.3f)dist(%.3f), vKillerPos(%.3f,%.3f,%.3f)dist(%.3f), nDefendFlagBaseRadius(%d)", __FUNCTION__,
		VEC_EXPAND( vFlagPos ), VEC_EXPAND( vVictimPos ), vFlagPos.Dist( vVictimPos ), VEC_EXPAND( vKillerPos ), vFlagPos.Dist( vKillerPos ), nDefendFlagBaseRadius );
#endif // DEBUG_DEFEND_MESSAGES

	// See if the victim is beyond radius.
	uint32 nDefendFlagBaseRadiusSqr = nDefendFlagBaseRadius * nDefendFlagBaseRadius;
	if( vFlagPos.DistSqr( vVictimPos ) > nDefendFlagBaseRadiusSqr )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: victim beyond radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		// See if the killer is beyond the radius.
		if( vFlagPos.DistSqr( vKillerPos ) > nDefendFlagBaseRadiusSqr )
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: killer beyond radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
			return false;
		}
	}

#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
		g_pLTBase->CPrint( "%s: %s defended his base killing %s", __FUNCTION__, 
			MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), MPW2A( pVictimGameClientData->GetUniqueName( )).c_str());
	}
#endif // DEBUG_DEFEND_MESSAGES

	// Give the killer some objective points.
	if( nDefendFlagBaseScorePlayer )
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nDefendFlagBaseScorePlayer );
	}
	if( nDefendFlagBaseScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pKillerPlayerObj->GetTeamID(), nDefendFlagBaseScoreTeam);
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_FlagDefend, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	HCLIENT hKillerClient = pKillerPlayerObj->GetClient();
	uint8 nKillerClientId = ( uint8 )g_pLTServer->GetClientID( hKillerClient );
	cEventMsg.Writeuint8( nKillerClientId );

	// Send event message.
	SendClientEventMessage( kCTFFlagState_InBase, cEventMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::InBase_OnCaptureAssist
//
//	PURPOSE:	Handle a InBase capture assist event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::InBase_OnCaptureAssist( MacroStateMachine::EventParams& eventParams )
{
#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: Entry", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Make sure we have a capture assist player.
	if( !m_hCaptureAssistPlayer )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No capture assist player", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Don't allow flag capture player to assist self.
	if( m_hCaptureAssistPlayer == flagEventParams.m_hObject )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Flag capturer can't assist self.", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// See if the time has expired.
	if( !m_CaptureAssistTimer.IsStarted( ) || m_CaptureAssistTimer.IsTimedOut( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Timer timed out", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Make sure they can get points for this assist.
	int32 nCaptureAssistScorePlayer = gameModeMgr.m_grnCTFCaptureAssistScorePlayer;
	int32 nCaptureAssistScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFCaptureAssistScoreTeam : 0;
#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: nCaptureAssistScorePlayer(%d) nCaptureAssistScoreTeam(%d)", __FUNCTION__, nCaptureAssistScorePlayer, nCaptureAssistScoreTeam );
#endif // DEBUG_DEFEND_MESSAGES
	if( !nCaptureAssistScorePlayer && !nCaptureAssistScoreTeam )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No capture assist score", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No flagbase object", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Only care about players on our team.
	CPlayerObj* pPlayerobj = CPlayerObj::DynamicCast( m_hCaptureAssistPlayer );
	if( !pPlayerobj )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Invalid capture assist player.", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerobj->GetClient( ));
		g_pLTBase->CPrint( "%s: pGameClientData->GetUniqueName(%s), pPlayerobj->GetTeamID(%d)", __FUNCTION__, 
			MPW2A( pGameClientData->GetUniqueName( )).c_str(), pPlayerobj->GetTeamID());
	}
#endif // DEBUG_DEFEND_MESSAGES
	if( pPlayerobj->GetTeamID() != pFlagBase->GetTeamId( ))
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Player not on this flag's team.", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerobj->GetClient( ));
		g_pLTBase->CPrint( "%s: %s got capture assist", __FUNCTION__, MPW2A( pGameClientData->GetUniqueName( )).c_str());
	}
#endif // DEBUG_DEFEND_MESSAGES

	// Give the player some objective points.
	if( nCaptureAssistScorePlayer )
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerobj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nCaptureAssistScorePlayer );
	}
	if( nCaptureAssistScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pPlayerobj->GetTeamID(), nCaptureAssistScoreTeam );
	}

	if( nCaptureAssistScorePlayer || nCaptureAssistScoreTeam )
	{
		// Prepare the event message.
		CAutoMessage cEventMsg;
		cEventMsg.WriteBits( kCTFFlagFXMsg_CaptureAssist, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
		HCLIENT hClient = pPlayerobj->GetClient();
		uint8 nClientId = ( uint8 )g_pLTServer->GetClientID( hClient );
		cEventMsg.Writeuint8( nClientId );

		// Send event message.
		SendClientEventMessage( kCTFFlagState_InBase, cEventMsg );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::InBase_OnExit
//
//	PURPOSE:	Handle a exit InBase state.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::InBase_OnExit( MacroStateMachine::EventParams& eventParams )
{
	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Remove the attachment.
	HATTACHMENT hAttachment = NULL;
	if( g_pLTServer->FindAttachment(pFlagBase->m_hObject, m_pFlag->m_hObject, &hAttachment) == LT_OK && hAttachment != NULL )
	{
		g_pLTServer->RemoveAttachment( hAttachment );
		hAttachment = NULL;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Carried_OnEnter
//
//	PURPOSE:	Handle a carried enter.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Carried_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Make sure we have a flag carrier.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( m_pFlag->m_hFlagCarrier );
	if( !pPlayerObj )
	{
		LTERROR( "Invalid flag carrier." );
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Don't allow touch notifies while being carried.
	g_pCommonLT->SetObjectFlags( m_pFlag->m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY );

	// Attach the flag to the flagcarrier.
	char const* pszFlagCarrierSocket = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagCarrierSocket );
	HATTACHMENT hAttachment = NULL;
	if( g_pLTServer->CreateAttachment( pPlayerObj->m_hObject, m_pFlag->m_hObject, pszFlagCarrierSocket, NULL, NULL, &hAttachment) != LT_OK ||
		hAttachment == NULL )
	{
		LTERROR( "Could not attach flag to player." );
		return false;
	}

	// Attach to the player's drop inventory event.
	m_delegatePlayerDropInventory.Attach( this, pPlayerObj, pPlayerObj->DropInventoryEvent );

	// Play the carried animation.
	char const* pszAniName = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagAniCarried );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pFlag->m_hObject, pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pFlag->m_hObject, MAIN_TRACKER, hAni, true );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Carried_OnDropped
//
//	PURPOSE:	Handle a dropped event.
//
// ----------------------------------------------------------------------- //

inline bool DropFilterFn(HOBJECT hTest, void *pUserData)
{
	// Ignore all objects that aren't solid.
	uint32 nFlags = 0;
	g_pCommonLT->GetObjectFlags( hTest, OFT_Flags, nFlags );
	if( !( nFlags & FLAG_SOLID ))
		return false;

	return ObjListFilterFn( hTest, pUserData );
}

bool FlagStateMachine::Carried_OnDropped( MacroStateMachine::EventParams& eventParams )
{
	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Make the rigidbody for the flag.  We can't just use the flagcarrier object
	// because it could be getting deleted due to disconnect, which means the 
	// flagcarrier ltobjref would have been cleared out.
	if( !CreateRigidBody( flagEventParams.m_hObject ))
	{
		// Couldn't make a rigidbody, just move it to the floor.
		HOBJECT hFilterList[]	= { flagEventParams.m_hObject, NULL };
		MoveObjectToFloor( m_pFlag->m_hObject, hFilterList, DropFilterFn );
	}

	// Make sure we have a flag carrier.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagEventParams.m_hObject );
	if( !pPlayerObj )
	{
		LTERROR( "Invalid flag carrier." );
		return false;
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_Dropped, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	HCLIENT hPlayerClient = pPlayerObj->GetClient();
	uint8 nPlayerClientId = ( uint8 )g_pLTServer->GetClientID( hPlayerClient );
	cEventMsg.Writeuint8( nPlayerClientId );

	// Go to the loose state.
	if( !SetState( kCTFFlagState_Loose, cEventMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Carried_OnCapture
//
//	PURPOSE:	Handle a dropped event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Carried_OnCapture( MacroStateMachine::EventParams& eventParams )
{
	// Get the player that has the flag.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( m_pFlag->m_hFlagCarrier );
	if( !pPlayerObj )
	{
		LTERROR( "Invalid flag carrier." );
		return false;
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_Capture, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	HCLIENT hPlayerClient = pPlayerObj->GetClient();
	uint8 nPlayerClientId = ( uint8 )g_pLTServer->GetClientID( hPlayerClient );
	cEventMsg.Writeuint8( nPlayerClientId );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// See if there are some objective points for the player.
	int32 nCaptureScorePlayer = gameModeMgr.m_grnCTFCaptureFlagScorePlayer;
	if( nCaptureScorePlayer )
	{
		// Give the player some objective points.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nCaptureScorePlayer );
	}

	// Add to the team score.
	int32 nCaptureScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFCaptureFlagScoreTeam : 0;
	if( nCaptureScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pPlayerObj->GetTeamID(), nCaptureScoreTeam );
	}

	g_pServerMissionMgr->CheckScoreLimitWin();

	// Go back to the base.
	if( !SetState( kCTFFlagState_InBase, cEventMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Carried_OnPlayerKill
//
//	PURPOSE:	Handle a Carried playerkilled event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Carried_OnPlayerKill( MacroStateMachine::EventParams& eventParams )
{
#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: Entry", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

	PlayerKillEventParams& playerKilledEventParams = ( PlayerKillEventParams& )eventParams;

	// Don't allow the flagcarrier to defend/kill himself.
	if( playerKilledEventParams.m_hKiller == m_pFlag->m_hFlagCarrier )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Flag carrier can't defend himself", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No flag base", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Get the killer and victim objects.
	CPlayerObj* pKillerPlayerObj = CPlayerObj::DynamicCast( playerKilledEventParams.m_hKiller );
	if( !pKillerPlayerObj )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No killer obj", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}
	CPlayerObj* pVictimPlayerObj = CPlayerObj::DynamicCast( playerKilledEventParams.m_hVictim );
	if( !pVictimPlayerObj )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: No victim obj", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
		return false;
	}

#ifdef DEBUG_DEFEND_MESSAGES
	g_pLTBase->CPrint( "%s: pFlagBase->GetTeamId(%d)", __FUNCTION__, pFlagBase->GetTeamId());
#endif // DEBUG_DEFEND_MESSAGES

#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		if( pKillerGameClientData )
		{
			g_pLTBase->CPrint( "%s: pKillerGameClientData->GetUniqueName(%s), pKillerPlayerObj->GetTeamID(%d)", __FUNCTION__, 
				MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), pKillerPlayerObj->GetTeamID());
		}
	}
#endif // DEBUG_DEFEND_MESSAGES

#ifdef DEBUG_DEFEND_MESSAGES
	{
		GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
		if( pVictimGameClientData )
		{
			g_pLTBase->CPrint( "%s: pVictimGameClientData->GetUniqueName(%s), pVictimPlayerObj->GetTeamID(%d)", __FUNCTION__, 
				MPW2A( pVictimGameClientData->GetUniqueName( )).c_str(), pVictimPlayerObj->GetTeamID());
		}
	}
#endif // DEBUG_DEFEND_MESSAGES

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Check if this is a flag carrier kill
	if( playerKilledEventParams.m_hVictim == m_pFlag->m_hFlagCarrier )
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Checking for killing a flagcarrier", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

		// Only care about flag carrier kills if the killer is on the the same team as the flag.
		if( pKillerPlayerObj->GetTeamID() != pFlagBase->GetTeamId( ))
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: Flag carrier killed by a teammate", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

			// Give the killer some penalty
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
			if( !pGameClientData )
			{
#ifdef DEBUG_DEFEND_MESSAGES
				g_pLTBase->CPrint( "%s: Invalid killer gameclientdata", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
				return false;
			}

			// Make sure they can get points for this kill
			int32 nTeamKillFlagCarrierPenalty = -(int32)(gameModeMgr.m_grnCTFTeamKillFlagCarrierPenalty);
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: nTeamKillFlagCarrierPenalty(%d)", __FUNCTION__, nTeamKillFlagCarrierPenalty );
#endif // DEBUG_DEFEND_MESSAGES
			if( nTeamKillFlagCarrierPenalty )
			{
				pGameClientData->GetPlayerScore()->AddObjectiveScore( nTeamKillFlagCarrierPenalty );
				return true;
			}

			return false;
		}

		// Only care about flag carrier kills if the victim is on the opposite team of the flag.
		if( pVictimPlayerObj->GetTeamID() == pFlagBase->GetTeamId( ))
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: Flag carrier killed is same team as this flag", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
			return false;
		}

		// Give the killer some objective points.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
		if( !pGameClientData )
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: Invalid killer gameclientdata", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
			return false;
		}

		// Make sure they can get points for this kill
		int32 nKillFlagCarrierScorePlayer = gameModeMgr.m_grnCTFKillFlagCarrierScorePlayer;
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: nKillFlagCarrierScorePlayer(%d)", __FUNCTION__, nKillFlagCarrierScorePlayer );
#endif // DEBUG_DEFEND_MESSAGES
		if( nKillFlagCarrierScorePlayer )
		{
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nKillFlagCarrierScorePlayer );
		}

		// Make sure team can get points for this kill
		int32 nKillFlagCarrierScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFKillFlagCarrierScoreTeam : 0;
		if( nKillFlagCarrierScoreTeam )
		{
			CTeamMgr::Instance( ).AddToScore( pKillerPlayerObj->GetTeamID(), nKillFlagCarrierScoreTeam );
		}

		// Set a timer for this player to be able to get a capture assist.
		float fCaptureAssistTimeout = gameModeMgr.m_grfCTFCaptureAssistTimeout;
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: fCaptureAssistTimeout(%.3f)", __FUNCTION__, fCaptureAssistTimeout );
#endif // DEBUG_DEFEND_MESSAGES
		if( fCaptureAssistTimeout > 0.0f )
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: Started capture assist timer for (%s)", __FUNCTION__, MPW2A( pGameClientData->GetUniqueName()).c_str());
#endif // DEBUG_DEFEND_MESSAGES
			m_CaptureAssistTimer.Start( fCaptureAssistTimeout );
			m_hCaptureAssistPlayer = pKillerPlayerObj->m_hObject;
			m_delegatePlayerSwitched.Attach( this, pGameClientData, pGameClientData->PlayerSwitched );
		}
		else
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: No capture assist time, clearing capture assist info", __FUNCTION__);
#endif // DEBUG_DEFEND_MESSAGES
			m_hCaptureAssistPlayer = NULL;
			m_CaptureAssistTimer.Stop( );
		}

#ifdef DEBUG_DEFEND_MESSAGES
		{
			GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
			GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
			g_pLTBase->CPrint( "%s: %s killed flagcarrier %s", __FUNCTION__, 
				MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), MPW2A( pVictimGameClientData->GetUniqueName( )).c_str());
		}
#endif // DEBUG_DEFEND_MESSAGES

		// Prepare the event message.
		CAutoMessage cEventMsg;
		cEventMsg.WriteBits( kCTFFlagFXMsg_FlagCarrierKill, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
		HCLIENT hKillerClient = pKillerPlayerObj->GetClient();
		uint8 nKillerClientId = ( uint8 )g_pLTServer->GetClientID( hKillerClient );
		cEventMsg.Writeuint8( nKillerClientId );

		// Change to the carried state.
		SendClientEventMessage( kCTFFlagState_Carried, cEventMsg );
	}
	else
	{
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: Checking for defending a flagcarrier", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

		// Only care about flag carrier defends if the killer is on the the opposite team of the flag.
		if( pKillerPlayerObj->GetTeamID() == pFlagBase->GetTeamId( ))
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: killer is on same team as this flag", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
			return false;
		}

		// Only care about flag carrier defends if the victim is on the same team of the flag.
		if( pVictimPlayerObj->GetTeamID() != pFlagBase->GetTeamId( ))
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: victim is on different team than this flag", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
			return false;
		}

		// Make sure they can get points for this kill
		int32 nDefendFlagCarrierScorePlayer = gameModeMgr.m_grnCTFDefendFlagCarrierScorePlayer;
		int32 nDefendFlagCarrierScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFDefendFlagCarrierScoreTeam : 0;
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: nDefendFlagCarrierScorePlayer(%d), nDefendFlagCarrierScoreTeam(%d)", 
			__FUNCTION__, nDefendFlagCarrierScorePlayer, nDefendFlagCarrierScoreTeam );
#endif // DEBUG_DEFEND_MESSAGES
		if( !nDefendFlagCarrierScorePlayer && !nDefendFlagCarrierScoreTeam )
			return false;
		int32 nDefendFlagCarrierRadius = gameModeMgr.m_grnCTFDefendFlagCarrierRadius;
#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: nDefendFlagCarrierRadius(%d)", __FUNCTION__, nDefendFlagCarrierRadius );
#endif // DEBUG_DEFEND_MESSAGES
		if( !nDefendFlagCarrierRadius )
			return false;

		// Get the flag position.
		LTVector vFlagPos;
		g_pLTServer->GetObjectPos( m_pFlag->m_hObject, &vFlagPos );

		// Get the position of the victim.
		LTVector vVictimPos;
		g_pLTServer->GetObjectPos( pVictimPlayerObj->m_hObject, &vVictimPos );

		// Get the position of the killer.
		LTVector vKillerPos;
		g_pLTServer->GetObjectPos( pKillerPlayerObj->m_hObject, &vKillerPos );

#ifdef DEBUG_DEFEND_MESSAGES
		g_pLTBase->CPrint( "%s: vFlagPos(%.3f,%.3f,%.3f), vKillerPos(%.3f,%.3f,%.3f) dist(%.3f), vVictimPos(%.3f,%.3f,%.3f) dist(%.3f)", __FUNCTION__, 
			VEC_EXPAND( vFlagPos ), VEC_EXPAND( vKillerPos ), vFlagPos.Dist( vKillerPos ), VEC_EXPAND( vVictimPos ), vFlagPos.Dist( vVictimPos ));
#endif // DEBUG_DEFEND_MESSAGES

		// See if the killer is beyond radius.
		uint32 nDefendFlagCarrierRadiusSqr = nDefendFlagCarrierRadius * nDefendFlagCarrierRadius;
		if( vFlagPos.DistSqr( vKillerPos ) > nDefendFlagCarrierRadiusSqr )
		{
#ifdef DEBUG_DEFEND_MESSAGES
			g_pLTBase->CPrint( "%s: Killer outside radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES

			// See if the victim is beyond the radius
			if( vFlagPos.DistSqr( vVictimPos ) > nDefendFlagCarrierRadiusSqr )
			{
#ifdef DEBUG_DEFEND_MESSAGES
				g_pLTBase->CPrint( "%s: Victim outside radius", __FUNCTION__ );
#endif // DEBUG_DEFEND_MESSAGES
				return false;
			}
		}

#ifdef DEBUG_DEFEND_MESSAGES
		{
			GameClientData* pKillerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
			GameClientData* pVictimGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pVictimPlayerObj->GetClient( ));
			g_pLTBase->CPrint( "%s: %s defended flag carrier by killing %s", __FUNCTION__, 
				MPW2A( pKillerGameClientData->GetUniqueName( )).c_str(), MPW2A( pVictimGameClientData->GetUniqueName( )).c_str());
		}
#endif // DEBUG_DEFEND_MESSAGES

		if( nDefendFlagCarrierScorePlayer )
		{
			// Give the killer some objective points.
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pKillerPlayerObj->GetClient( ));
			if( pGameClientData )
				pGameClientData->GetPlayerScore()->AddObjectiveScore( nDefendFlagCarrierScorePlayer );
		}
		if( nDefendFlagCarrierScoreTeam )
		{
			CTeamMgr::Instance().AddToScore( pKillerPlayerObj->GetTeamID(), nDefendFlagCarrierScoreTeam );
		}

		// Prepare the event message.
		CAutoMessage cEventMsg;
		cEventMsg.WriteBits( kCTFFlagFXMsg_FlagCarrierDefend, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
		HCLIENT hKillerClient = pKillerPlayerObj->GetClient();
		uint8 nKillerClientId = ( uint8 )g_pLTServer->GetClientID( hKillerClient );
		cEventMsg.Writeuint8( nKillerClientId );

		// Change to the carried state.
		SendClientEventMessage( kCTFFlagState_Carried, cEventMsg );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Carried_OnExit
//
//	PURPOSE:	Handle a carried exit.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Carried_OnExit( MacroStateMachine::EventParams& eventParams )
{
	// Remove flag from flag carrier.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( m_pFlag->m_hFlagCarrier );
	if( pPlayerObj )
	{
		// Get our flagbase.
		CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
		if( !pFlagBase )
		{
			LTERROR( "Missing flagbase object." );
			return false;
		}

		// Remove the attachment.
		HATTACHMENT hAttachment = NULL;
		if( g_pLTServer->FindAttachment(pPlayerObj->m_hObject, m_pFlag->m_hObject, &hAttachment) == LT_OK && hAttachment != NULL )
		{
			g_pLTServer->RemoveAttachment( hAttachment );
			hAttachment = NULL;
		}

		// Take flag away from carrier.
		pPlayerObj->GetInventory()->SetCTFFlag( NULL );

		// Get the transform to the player's socket to use as the starting spot of the flag.
		LTTransform tfBack;
		g_pLTServer->GetObjectTransform( pPlayerObj->m_hObject, &tfBack );
		char const* pszFlagCarrierSocket = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagCarrierSocket );
		if( !LTStrEmpty( pszFlagCarrierSocket ))
		{
			HMODELSOCKET hSocket = (HMODELSOCKET)NULL;
			if( g_pModelLT->GetSocket( pPlayerObj->m_hObject, pszFlagCarrierSocket, hSocket ) == LT_OK && hSocket )
			{
				g_pModelLT->GetSocketTransform( pPlayerObj->m_hObject, hSocket, tfBack, true );
			}
		}
		g_pLTServer->SetObjectTransform( m_pFlag->m_hObject, tfBack );

		// Detach from the player's drop inventory event.
		m_delegatePlayerDropInventory.Detach( );
	}

	// No more flagcarrier.
	m_pFlag->m_hFlagCarrier = NULL;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnEnter
//
//	PURPOSE:	Handle a loose enter.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Allow touch notifies while loose.
	g_pCommonLT->SetObjectFlags( m_pFlag->m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY );

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return false;
	}

	// Play the inbase animation.
	char const* pszAniName = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagAniLoose );
	if( !LTStrEmpty( pszAniName ))
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_pFlag->m_hObject, pszAniName, hAni );
		if( INVALID_MODEL_ANIM != hAni )
		{
			g_pModelLT->SetPlaying( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_pFlag->m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetCurAnim( m_pFlag->m_hObject, MAIN_TRACKER, hAni, true );
		}
	}

	// Set the timeout if it's a value above zero, otherwise stay loose forever.
	float fLooseTimeout = GameModeMgr::Instance().m_grfCTFFlagLooseTimeout;
	if( fLooseTimeout > 0.0f )
	{
		m_StateTimer.Start( fLooseTimeout );
	}
	else
	{
		m_StateTimer.Stop( );
	}

	// Always need update so we can update the rigidbody.
	g_pLTServer->SetNextUpdate( m_pFlag->m_hObject, UPDATE_NEXT_FRAME );

	// Go unguaranteed since we'll be moving a lot.  Only need this if we have a rigidbody, becuase
	// because the without it we don't move.  Also, setting this on an attachment seems to stall the parent.
	if( m_hRigidBody )
	{
		g_pLTServer->SetNetFlags( m_pFlag->m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED);
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnUpdate
//
//	PURPOSE:	Handle a loose update.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// See if we are timing the loose state for an auto return and time ran out.
	if( m_StateTimer.IsStarted( ) && m_StateTimer.IsTimedOut())
	{
		if( !DoAutoReturnEvent())
			return false;
	}

	// Update to our rigidbody position.
	UpdateRigidBody( );

	// Keep getting updates.
	g_pLTServer->SetNextUpdate( m_pFlag->m_hObject, UPDATE_NEXT_FRAME );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnTouched
//
//	PURPOSE:	Handle a touch event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnTouched( MacroStateMachine::EventParams& eventParams )
{
	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Check if we were touched by a player.  Ignore all others.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Don't allow non-alive players to touch the flag.
	if( !pPlayerObj->IsAlive( ))
		return false;

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
		return false;

	// If touched by enemy, see if they can grab our flag.
	if( pPlayerObj->GetTeamID() != pFlagBase->GetTeamId( ))
	{
		// Player already carrying a flag, can't grab ours.
		if( pPlayerObj->GetInventory()->GetCTFFlag( ))
			return false;

		// Enemy pickedup the flag.
		if( !DoPickedUpEvent( pPlayerObj->m_hObject ))
			return false;
	}
	else
	{
		// Friendly is capturing enemy flag.
		if( !DoReturnEvent( pPlayerObj->m_hObject ))
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnPickedUp
//
//	PURPOSE:	Handle a pickedup event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnPickedUp( MacroStateMachine::EventParams& eventParams )
{
	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Get the player that is picking the flag up.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagEventParams.m_hObject );
	if( !pPlayerObj )
		return false;

	// Make sure they don't already have a flag.
	if( pPlayerObj->GetInventory()->GetCTFFlag( ))
		return false;

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
		return false;

	// Can't pickup own team's flag.
	if( pPlayerObj->GetTeamID() == pFlagBase->GetTeamId( ))
		return false;

	// Give the player the flag.
	pPlayerObj->GetInventory()->SetCTFFlag( m_pFlag->m_hObject );

	// Set our flag carrier.
	m_pFlag->m_hFlagCarrier = pPlayerObj->m_hObject;

	// Give points for pickup.
	GameModeMgr& gameModeMgr = GameModeMgr::Instance();
	int32 nPickupFlagScorePlayer = gameModeMgr.m_grnCTFPickupFlagScorePlayer;
	if( nPickupFlagScorePlayer )
	{
		// Give the player some objective points.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nPickupFlagScorePlayer );
	}
	int32 nPickupFlagScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFPickupFlagScoreTeam : 0;
	if( nPickupFlagScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pPlayerObj->GetTeamID(), nPickupFlagScoreTeam );
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_PickedUp, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	cEventMsg.WriteObject( m_pFlag->m_hFlagCarrier );
	HCLIENT hPlayerClient = pPlayerObj->GetClient();
	uint8 nPlayerClientId = ( uint8 )g_pLTServer->GetClientID( hPlayerClient );
	cEventMsg.Writeuint8( nPlayerClientId );

	// Now being carried.
	SetState( kCTFFlagState_Carried, cEventMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnReturn
//
//	PURPOSE:	Handle a return event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnReturn( MacroStateMachine::EventParams& eventParams )
{
	FlagEventParams& flagEventParams = ( FlagEventParams& )eventParams;

	// Get the player that is picking the flag up.
	CPlayerObj* pPlayerObj = CPlayerObj::DynamicCast( flagEventParams.m_hObject );
	if( !pPlayerObj )
	{
		LTERROR( "Invalid player doing return." );
		return false;
	}

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_pFlag->m_hFlagBase );
	if( !pFlagBase )
		return false;

	// Can't return other team's flag.
	if( pPlayerObj->GetTeamID() != pFlagBase->GetTeamId( ))
		return false;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	int32 nReturnFlagScorePlayer = gameModeMgr.m_grnCTFReturnFlagScorePlayer;
	if( nReturnFlagScorePlayer )
	{
		// Give the player some objective points.
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore()->AddObjectiveScore( nReturnFlagScorePlayer );
	}
	int32 nReturnFlagScoreTeam = gameModeMgr.m_grbUseTeams ? gameModeMgr.m_grnCTFReturnFlagScoreTeam : 0;
	if( nReturnFlagScoreTeam )
	{
		CTeamMgr::Instance().AddToScore( pPlayerObj->GetTeamID(), nReturnFlagScoreTeam );
	}

	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_Return, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );
	HCLIENT hPlayerClient = pPlayerObj->GetClient();
	uint8 nPlayerClientId = ( uint8 )g_pLTServer->GetClientID( hPlayerClient );
	cEventMsg.Writeuint8( nPlayerClientId );

	// Go back to the base.
	if( !SetState( kCTFFlagState_InBase, cEventMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnAutoReturn
//
//	PURPOSE:	Handle a auto return event.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnAutoReturn( MacroStateMachine::EventParams& eventParams )
{
	// Prepare the event message.
	CAutoMessage cEventMsg;
	cEventMsg.WriteBits( kCTFFlagFXMsg_AutoReturn, FNumBitsExclusive<kCTFFlagFXMsg_NumIds>::k_nValue );

	// Go back to the base.
	if( !SetState( kCTFFlagState_InBase, cEventMsg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FlagStateMachine::Loose_OnExit
//
//	PURPOSE:	Handle a loose exit.
//
// ----------------------------------------------------------------------- //
bool FlagStateMachine::Loose_OnExit( MacroStateMachine::EventParams& eventParams )
{
	// Don't need updates any longer.
	g_pLTServer->SetNextUpdate( m_pFlag->m_hObject, UPDATE_NEVER );

	// Don't need rigidbody anymore.
	ReleaseRigidBody( );

	// Remove unguaranteed flags.
	g_pLTServer->SetNetFlags( m_pFlag->m_hObject, 0);

	return true;
}

BEGIN_CLASS( CTFFlag )
END_CLASS_FLAGS( CTFFlag, GameBase, CF_HIDDEN | CF_DONTSAVE, "CTFFlag used exclusively with CTFFlagBase." )

// Register with the CommandMgr...
CMDMGR_BEGIN_REGISTER_CLASS( CTFFlag )
	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( CTFFlag, HandleActivateMsg ),	"ACTIVATE", "Grabs flag.", "activate" )
CMDMGR_END_REGISTER_CLASS( CTFFlag, GameBase )

//
// CTFFlag class implementation...
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlag::CTFFlag
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTFFlag::CTFFlag( ) : GameBase( OT_MODEL )
{
	m_pFlagStateMachine = NULL;
	m_hFlagCarrier.SetReceiver( *this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTFFlag::~CTFFlag
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTFFlag::~CTFFlag( )
{
	if( m_pFlagStateMachine )
	{
		delete m_pFlagStateMachine;
		m_pFlagStateMachine = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //
uint32 CTFFlag::EngineMessageFn( uint32 messageID, void *pData, float fData )
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

		case MID_TOUCHNOTIFY:
		{
			HandleTouchNotify(( HOBJECT )pData);
		}
		break;

		default : 
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::ReadProp
//
//	PURPOSE:	Read in the properties of the object... 
//
// ----------------------------------------------------------------------- //
bool CTFFlag::ReadProp( const GenericPropList *pProps )
{
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::PostReadProp
//
//	PURPOSE:	Configure the ObjectCreateStruct for creating the object
//
// ----------------------------------------------------------------------- //
bool CTFFlag::PostReadProp( ObjectCreateStruct *pStruct )
{
	// Get the flagbase object passed in.
	CTFFlagBase* pFlagBase = ( CTFFlagBase* )pStruct->m_UserData;
	if( !pFlagBase )
		return false;

	m_hFlagBase = pFlagBase->m_hObject;

	// Get the prop used.
	HRECORD hProp = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec(), FlagProp );
	if( !hProp )
		return false;

	// Fill in the model and material names.
	char const* pszPropFilename = g_pPropsDB->GetPropFilename( hProp );
	if( LTStrEmpty( pszPropFilename ))
		return false;
	LTStrCpy( pStruct->m_Filename, pszPropFilename, LTARRAYSIZE( pStruct->m_Filename ));

	g_pPropsDB->CopyMaterialFilenames( hProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ),
		LTARRAYSIZE( pStruct->m_Materials[0] ));

	// Make the flagbase get touch notifies and be visible.
	pStruct->m_Flags |= FLAG_VISIBLE;

	// Setup or physics group to pickups.
	pStruct->m_eGroup = PhysicsUtilities::ePhysicsGroup_UserPickup;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::InitialUpdate
//
//	PURPOSE:	Handle a MID_INITIALUPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void CTFFlag::InitialUpdate( )
{
	// Set the base model diminsions...
	LTVector vDims;
	HMODELANIM hAnimBase = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hAnimBase );
	g_pModelLT->GetModelAnimUserDims (m_hObject, hAnimBase, &vDims);
	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

	// Create our statemachine object.
	LT_MEM_TRACK_ALLOC(m_pFlagStateMachine = new FlagStateMachine, LT_MEM_TYPE_GAMECODE);
	m_pFlagStateMachine->Init( *this );
	m_pFlagStateMachine->SetState( kCTFFlagState_InBase, NULL );

	// Get our flagbase.
	CTFFlagBase* pFlagBase = CTFFlagBase::DynamicCast( m_hFlagBase );
	if( !pFlagBase )
	{
		LTERROR( "Missing flagbase object." );
		return;
	}
	// Setup our collision properties
	HRECORD hCollisionProperty = DATABASE_CATEGORY( CTFFlagBase ).GETRECORDATTRIB( pFlagBase->GetFlagBaseRec( ), FlagCollisionProperty );
	uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionProperty );
	g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );

	// Take us out of the physics simulation, we'll use our own rigid body
	PhysicsUtilities::SetPhysicsWeightSet(m_hObject, PhysicsUtilities::WEIGHTSET_NONE, false);

	CreateSpecialFX( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::Update
//
//	PURPOSE:	Handle a MID_UPDATE message from the engine....
//
// ----------------------------------------------------------------------- //
void CTFFlag::Update( )
{
	if( !m_pFlagStateMachine )
		return;

	// Update the statemachine.
	m_pFlagStateMachine->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::CreateSpecialFX
//
//	PURPOSE:	Send relevant information to clients...
//
// ----------------------------------------------------------------------- //

void CTFFlag::CreateSpecialFX( bool bUpdateClients )
{
	if( !m_pFlagStateMachine )
		return;

	CTFFLAGCREATESTRUCT cs;
	cs.m_eCTFFlagState = ( CTFFlagState )m_pFlagStateMachine->GetState( );
	cs.m_hFlagBase = m_hFlagBase;

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_CTFFLAG_ID );
		cs.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::HandleTouchNotify
//
//	PURPOSE:	Handle a MID_TOUCHNOTIFY message from the engine....
//
// ----------------------------------------------------------------------- //

void CTFFlag::HandleTouchNotify( HOBJECT hToucher )
{
	if( !m_pFlagStateMachine )
		return;

	m_pFlagStateMachine->DoTouchEvent( hToucher );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::OnLinkBroken
//
//	PURPOSE:	Handle links getting broken.
//
// ----------------------------------------------------------------------- //
void CTFFlag::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( !m_pFlagStateMachine )
		return;

	// Handle the flagcarrier getting deleted.
	if( &m_hFlagCarrier == pRef )
	{
		m_pFlagStateMachine->DoDropEvent( hObj );
		return;
	}

	GameBase::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::FlagStolen
//
//	PURPOSE:	Tells flag that it has been stolen.
//
// ----------------------------------------------------------------------- //
bool CTFFlag::FlagStolen( HOBJECT hStealingPlayer )
{
	if( !m_pFlagStateMachine )
		return false;

	return m_pFlagStateMachine->DoStealEvent( hStealingPlayer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::FlagCapture
//
//	PURPOSE:	Tells flag that it has been captured.
//
// ----------------------------------------------------------------------- //
bool CTFFlag::FlagCapture( )
{
	if( !m_pFlagStateMachine )
		return false;

	return m_pFlagStateMachine->DoCaptureEvent( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::CaptureAssist
//
//	PURPOSE:	Tells flag that it can do a capture assist.
//
// ----------------------------------------------------------------------- //
bool CTFFlag::CaptureAssist( HOBJECT hFlagCapPlayer )
{
	if( !m_pFlagStateMachine )
		return false;

	return m_pFlagStateMachine->DoCaptureAssistEvent( hFlagCapPlayer );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTFFlag::HandleActivateMsg
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void CTFFlag::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !m_pFlagStateMachine )
		return;

	m_pFlagStateMachine->DoActivatedEvent( hSender );
}


