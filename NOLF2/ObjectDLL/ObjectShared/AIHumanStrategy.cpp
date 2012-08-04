// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStrategy.h"
#include "CharacterMgr.h"
#include "AIHuman.h"
#include "PlayerObj.h"
#include "RotatingDoor.h"
#include "AITarget.h"
#include "AIPathMgr.h"
#include "CharacterHitBox.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AnimationMovement.h"
#include "DebugLineSystem.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "AIMovement.h"
#include "AICentralKnowledgeMgr.h"
#include "MusicMgr.h"

const static LTFLOAT s_fPerturbScale = 4.0f;
static CVarTrack g_ShowAIPath;
static CVarTrack g_ShowAIRemainingPath;

//DEFINE_SPECIFIC_AI_FACTORY(Strategy);


// Factories

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyOneShotAni, kStrat_HumanOneShotAni);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyFollowPath, kStrat_HumanFollowPath);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyDodge, kStrat_HumanDodge);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyTaunt, kStrat_HumanTaunt);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCoverDuck, kStrat_HumanCoverDuck);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCoverBlind, kStrat_HumanCoverBlind);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCover1WayCorner, kStrat_HumanCover1WayCorner);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyCover2WayCorner, kStrat_HumanCover2WayCorner);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyGrenadeThrow, kStrat_HumanGrenadeThrow);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyShootBurst, kStrat_HumanShootBurst);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyFlashlight, kStrat_HumanFlashlight);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategy::CAIHumanStrategy
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategy::CAIHumanStrategy()
{
	m_pAIHuman = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategy::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategy::Init(CAIHuman* pAIHuman)
{
	m_pAIHuman = pAIHuman;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategy::GetAnimationContext
//
//	PURPOSE:	Gets our AnimationContext
//
// ----------------------------------------------------------------------- //

CAnimationContext* CAIHumanStrategy::GetAnimationContext()
{
	return GetAI()->GetAnimationContext();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategy::UpdateAnimation
//
//	PURPOSE:	Update animation
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategy::UpdateAnimation()
{
	// Do not try to animate if movement in controlling animation.
	// (e.g. jumping)

	if( GetAI()->GetAIMovement()->IsMovementLocked() )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::CAIHumanStrategyFollowPath/~CAIHumanStrategyFollowPath
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyFollowPath::CAIHumanStrategyFollowPath()
{
	m_pStrategyShoot = LTNULL;

	m_eState = eStateUnset;
	m_eMedium = eMediumGround;
	m_pPath = AI_FACTORY_NEW(CAIPath);
	m_eDoorState = eDoorStateNone;
	m_eDoorAction = kAP_OpenDoor;
	m_fLastDoor1Yaw = 0.f;
	m_fLastDoor2Yaw = 0.f;
	m_bModifiedMovement = LTFALSE;
	m_cStuckOnDoorUpdates = 0;
	m_bDoorShootThroughable = LTFALSE;
	m_bDrawingPath = LTFALSE;

	if( !g_ShowAIPath.IsInitted() )
	{
		g_ShowAIPath.Init(g_pLTServer, "ShowAIPath", LTNULL, 0.0f);
	}

	if( !g_ShowAIRemainingPath.IsInitted() )
	{
		g_ShowAIRemainingPath.Init(g_pLTServer, "ShowAIRemainingPath", LTNULL, 0.0f);
	}

	m_bCheckAnimStatus = LTFALSE;
}

CAIHumanStrategyFollowPath::~CAIHumanStrategyFollowPath()
{
	AI_FACTORY_DELETE(m_pPath);

	// Make sure movement is unlocked when exiting the strategy.

	if( GetAI()->GetAIMovement()->IsMovementLocked() )
	{
		GetAI()->GetAIMovement()->UnlockMovement();
	}

	if( m_bDrawingPath )
	{
		LineSystem::RemoveSystem(this, "ShowPath");
	}

	LineSystem::RemoveSystem(this, "ShowRemainingPath");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::Init(CAIHuman* pAIHuman, CAIHumanStrategyShoot* pStrategyShoot)
{
	if ( !CAIHumanStrategy::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pStrategyShoot = pStrategyShoot;
	
	m_pAIHuman->GetAIMovement()->ClearAnimations();
	m_pAIHuman->GetAIMovement()->PushAnimation(kAP_Walk);

	m_pPath->Init(pAIHuman);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::SetMovement
//
//	PURPOSE:	Set movement animation.
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::SetMovement(EnumAnimProp eMovement)
{
	if( m_pAIHuman->GetAIMovement()->TopAnimation() != eMovement )
	{
		m_pAIHuman->GetAIMovement()->PopAnimation();
		m_pAIHuman->GetAIMovement()->PushAnimation(eMovement); 
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::GetMovement
//
//	PURPOSE:	Get movement animation.
//
// ----------------------------------------------------------------------- //

EnumAnimProp CAIHumanStrategyFollowPath::GetMovement()
{
	return m_pAIHuman->GetAIMovement()->TopAnimation();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::SetMedium
//
//	PURPOSE:	Set medium AI is moving over.
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::SetMedium(Medium eMedium)
{
	m_eMedium = eMedium; 
	m_pAIHuman->GetAIMovement()->SetUnderwater(eMedium == eMediumUnderwater); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::HandleModelString(ArgList* pArgList)
{
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, "DOOR") )
	{
		m_eDoorState = eDoorStateWaitingForAnimationToFinish;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::DelayChangeState()
{
	if ( !m_pPath || !m_pPath->GetCurrentWaypoint() ) return super::DelayChangeState();

	switch ( m_pPath->GetCurrentWaypoint()->GetInstruction() )
	{
		case CAIPathWaypoint::eInstructionClimbDownTo:
		case CAIPathWaypoint::eInstructionClimbUpTo:
		{
			return LTTRUE;
		}
		break;

		default:
		{
			return super::DelayChangeState();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Update()
{
	CAIHumanStrategy::Update();

	AIASSERT(m_eState == eStateSet, m_pAIHuman->m_hObject, "CAIHumanStrategyFollowPath::Update: Path not set.");

	// Handle closing doors.

	if( m_pAIHuman->HasLastDoors() )
	{
		if( m_pAIHuman->GetAwareness() == kAware_Relaxed )
		{
			if( ( m_eDoorState != eDoorStateWalkingThroughDoor ) &&
				m_pAIHuman->CanCloseDoors() )
			{
				m_pAIHuman->CloseDoors();
			}
		}
		else 
		{	
			m_pAIHuman->UnmarkDoors();
		}
	}


	CAIPathWaypoint* pWaypointPrev = LTNULL;
	CAIPathWaypoint* pWaypoint;

	// Draw from where remaining path 
	if( g_ShowAIRemainingPath.GetFloat() > 0.0f )
	{
		DebugRemainingDrawPath();
	}

	// Basically, if our waypoint update keeps advancing the waypoint, keep updating for the new waypoint.
	// That way we skip over degenerate waypoints that would cause animation pops.

	while ( m_pPath->HasRemainingWaypoints() && (pWaypointPrev != (pWaypoint = m_pPath->GetCurrentWaypoint())) )
	{
		switch ( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			{
				UpdateMoveTo(pWaypoint);
				GetAI()->GetAIMovement()->UnlockMovement();
			}
			break;

			case CAIPathWaypoint::eInstructionLockedMoveTo:
			{
				UpdateMoveTo(pWaypoint);
				GetAI()->GetAIMovement()->LockMovement();
			}
			break;

			case CAIPathWaypoint::eInstructionJumpDownTo:
			case CAIPathWaypoint::eInstructionJumpUpTo:
			case CAIPathWaypoint::eInstructionJumpOver:
			{
				UpdateJumpTo(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionFaceJumpLand:
			{
				UpdateFaceJumpLand(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionClimbDownTo:
			case CAIPathWaypoint::eInstructionClimbUpTo:
			case CAIPathWaypoint::eInstructionGetOnLadder:
			case CAIPathWaypoint::eInstructionGetOffLadder:
			{
				UpdateClimbTo(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionFaceLadder:
			{
				UpdateFaceLadder(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionFaceDoor:
			{
				UpdateFaceDoor(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionOpenDoors:
			{
				UpdateOpenDoors(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionWaitForDoors:
			{
				UpdateWaitForDoors(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionReleaseGate:
			{
				UpdateReleaseGate(pWaypoint);
				m_eDoorState = eDoorStateNone; 
			}
			break;

			case CAIPathWaypoint::eInstructionMoveFromTeleport:
			case CAIPathWaypoint::eInstructionMoveToTeleport:
			{
				UpdateMoveTeleport(pWaypoint);
			}
			break;

			default:
			{
                g_pLTServer->CPrint("CAIHumanStrategyFollowPath::Update - unrecognized waypoint instruction");
				_ASSERT(!"CAIHumanStrategyFollowPath::Update - unrecognized waypoint instruction");
			}
			break;
		}

		pWaypointPrev = pWaypoint;
	}

	if ( !m_pPath->HasRemainingWaypoints() )
	{
		m_eState = eStateDone;
		m_eDoorState = eDoorStateNone;
	}


	// Debug path rendering.

	if( g_ShowAIPath.GetFloat() > 0.0f )
	{
		if( !m_bDrawingPath )
		{
			DebugDrawPath();
		}
	}
	else
	{
		if( m_bDrawingPath )
		{
			LineSystem::RemoveSystem(this, "ShowPath");

			m_bDrawingPath = LTFALSE;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateAnimation()
{
	// Intentionally do NOT call super::UpdateAnimation().

	switch ( m_eState )
	{
		case eStateUnset:
		{
		}
		break;

		case eStateSet:
		{
			if ( !m_pPath->HasRemainingWaypoints() )
			{
				return LTFALSE;
			}

			CAIPathWaypoint* pWaypoint = m_pPath->GetCurrentWaypoint();

			switch ( pWaypoint->GetInstruction() )
			{
				case CAIPathWaypoint::eInstructionLockedMoveTo:
				case CAIPathWaypoint::eInstructionMoveTo:
				{
					m_pAIHuman->GetAIMovement()->UpdateAnimation();
					if ( m_bModifiedMovement )
					{
						GetAnimationContext()->SetProp(m_aniModifiedMovement);
					}
				}
				break;
				
				case CAIPathWaypoint::eInstructionMoveToTeleport:
				case CAIPathWaypoint::eInstructionMoveFromTeleport:
				{
					m_pAIHuman->GetAIMovement()->UpdateAnimation();
				}
				break;

				case CAIPathWaypoint::eInstructionJumpDownTo:
				{
					GetAnimationContext()->SetProp(kAPG_Awareness, pWaypoint->GetAnimProp() );
					m_pAIHuman->GetAIMovement()->UpdateAnimation();
				}
				break;

				case CAIPathWaypoint::eInstructionJumpUpTo:
				case CAIPathWaypoint::eInstructionJumpOver:
				{
					GetAnimationContext()->SetProp( kAPG_Awareness, kAP_None );
					m_pAIHuman->GetAIMovement()->UpdateAnimation();
				}
				break;

				case CAIPathWaypoint::eInstructionFaceJumpLand:
				{
				}
				break;

				case CAIPathWaypoint::eInstructionClimbUpTo:
				case CAIPathWaypoint::eInstructionClimbDownTo:
				case CAIPathWaypoint::eInstructionGetOnLadder:
				case CAIPathWaypoint::eInstructionGetOffLadder:
				{
					GetAnimationContext()->SetProp(kAPG_Awareness, kAP_None);
					m_pAIHuman->GetAIMovement()->UpdateAnimation();
				}
				break;

				case CAIPathWaypoint::eInstructionFaceLadder:
				{
				}
				break;

				case CAIPathWaypoint::eInstructionFaceDoor:
				{
				}
				break;

				case CAIPathWaypoint::eInstructionWaitForDoors:
				{
					_ASSERT(eDoorStateWaitingForDoorToOpen == m_eDoorState);
				}
				break;

				case CAIPathWaypoint::eInstructionReleaseGate:
				{
				}
				break;

				case CAIPathWaypoint::eInstructionOpenDoors:
				{
					switch ( m_eDoorState )
					{
						case eDoorStateNone:
						{
							if( m_pStrategyShoot &&
								m_bDoorShootThroughable && 
								(GetAI()->GetAwareness() == kAware_Alert) && 
								GetAI()->GetCurrentWeapon() && 
								GetAI()->HasTarget() )
							{
								m_pStrategyShoot->UpdateAnimation();
							}
						}
						break;

						case eDoorStateWaitingForAnimationToStart:
						{
							GetAnimationContext()->ClearProps();
							GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
							GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
							GetAnimationContext()->SetProp(kAPG_Action, m_eDoorAction);
							GetAnimationContext()->Lock();
						}
						break;

						case eDoorStateWaitingForDoorToOpen:
						{
							if ( m_cStuckOnDoorUpdates > 2 )
							{
								GetAnimationContext()->ClearLock();

								if ( m_pStrategyShoot
									&& m_bDoorShootThroughable 
									&& GetAI()->GetCurrentWeapon() 
									&& GetAI()->HasTarget() )
								{
									m_pStrategyShoot->UpdateAnimation();
								}

								else
								{
									GetAnimationContext()->ClearProps();
									GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
									GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Lower);
									GetAnimationContext()->SetProp(kAPG_Action, kAP_KnockOnDoor);
								}
							}
						}
						break;
					}
				}
				break;
			}
		}
		break;

		case eStateDone:
		{
		}
		break;

		default:
			_ASSERT(LTFALSE);
			break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateMoveTo(CAIPathWaypoint* pWaypoint)
{
	if ( m_pAIHuman->GetAIMovement()->IsDone() )
	{
		m_pPath->IncrementWaypointIndex();
		m_pAIHuman->GetAIMovement()->Clear();
	}
	else if ( m_pAIHuman->GetAIMovement()->IsUnset() )
	{
		m_pAIHuman->GetAIMovement()->SetMovementDest(pWaypoint->GetArgumentVector1());
	}

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateMoveTeleport()
//              
//	PURPOSE:	AI is leaving a Teleporter.  Until they have 'finished'
//				leaving, they are not allowed to change goals.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStrategyFollowPath::UpdateMoveTeleport(CAIPathWaypoint* pWaypoint)
{
	CAIMovement* pMovement = m_pAIHuman->GetAIMovement();

	switch( pMovement->TopAnimation() )
	{
		case kAP_ExitTeleport:
		{
			if ( m_pAIHuman->GetAIMovement()->IsDone() )
			{
				m_pPath->IncrementWaypointIndex();
				m_pAIHuman->GetAIMovement()->Clear();

				pMovement->PopAnimation();
				pMovement->Clear();
				pMovement->UnlockMovement();
			}
		}
		break;

		case kAP_EnterTeleport:
		{
			if ( m_pAIHuman->GetAIMovement()->IsDone() )
			{
				// AI is at the teleport point.  They now should now walk to a door and
				// reenter the gameplay area.  They should not reevaluate and do
				// something else so lock the movement just like we do for jumping.
				AIASSERT( pWaypoint->GetArgumentObject1(), m_pAIHuman->m_hObject, "UpdateMoveTeleport: No waypoint/volume argument" );
				AIVolumeTeleport* pSourceVolume = (AIVolumeTeleport*)pWaypoint->GetArgumentObject1();
				AIVolumeTeleport* pDestVolume = (AIVolumeTeleport*)pWaypoint->GetArgumentObject2();
				pSourceVolume->DoTeleportObject( m_pAIHuman, pDestVolume );

				m_pPath->IncrementWaypointIndex();
				m_pAIHuman->GetAIMovement()->Clear();

				pMovement->PopAnimation();
				pMovement->Clear();
				pMovement->UnlockMovement();
			}
		}
		break;

		default:
		{
			// By default, set the movement type as this must be the first
			// time we are entering this waypoint.
			switch ( pWaypoint->GetInstruction() )
			{
			case CAIPathWaypoint::eInstructionMoveToTeleport:
				{
					// Face the next waypoint instantly.
					GetAI()->FacePos( pWaypoint->GetArgumentVector1() );
					GetAI()->FaceTargetRotImmediately();

					pMovement->SetMovementDest( pWaypoint->GetArgumentVector1() );
					pMovement->PushAnimation(kAP_EnterTeleport);
					pMovement->LockMovement();
				}
				break;

			case CAIPathWaypoint::eInstructionMoveFromTeleport:
				{
					// Face the next waypoint instantly.
					GetAI()->FacePos( pWaypoint->GetArgumentVector1() );
					GetAI()->FaceTargetRotImmediately();

					pMovement->SetMovementDest( pWaypoint->GetArgumentVector1() );
					pMovement->PushAnimation(kAP_ExitTeleport);
					pMovement->LockMovement();

				}
				break;

			default:
				AIASSERT( 0, m_pAIHuman->m_hObject, "UpdateMoveTeleport: Unexpected waypoint instruction" );
				break;
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateClimbTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateClimbTo(CAIPathWaypoint* pWaypoint)
{
	// Check for completion of get on/off animations.

	CAIMovement* pMovement = m_pAIHuman->GetAIMovement();
	switch( pMovement->TopAnimation() )
	{
		case kAP_GetOnLadderRight:
		case kAP_GetOnLadderLeft:
			if( !m_pAIHuman->GetAnimationContext()->IsLocked() )
			{
				// Face the ladder.

				CAIPathWaypoint* pNextWaypoint = m_pPath->GetNextWaypoint();
				LTVector vForward = pNextWaypoint->GetArgumentVector1();
				m_pAIHuman->FaceDir( vForward );
				m_pAIHuman->FaceTargetRotImmediately();

				// Increment waypoint, but skip the next one because the animation
				// took care of facing the ladder.

				m_pPath->IncrementWaypointIndex();
				m_pPath->IncrementWaypointIndex();
				pMovement->PopAnimation();
				pMovement->Clear();
				pMovement->UnlockMovement();
				return LTTRUE;
			}
			break;

		case kAP_GetOffLadderRight:
		case kAP_GetOffLadderLeft:
			if( !m_pAIHuman->GetAnimationContext()->IsLocked() )
			{
				m_pPath->IncrementWaypointIndex();
				pMovement->PopAnimation();
				pMovement->Clear();
				pMovement->UnlockMovement();
				return LTTRUE;
			}
			break;
	}


	if ( m_pAIHuman->GetAIMovement()->IsDone() )
	{
		m_pPath->IncrementWaypointIndex();
		m_pAIHuman->GetAIMovement()->PopAnimation();
		m_pAIHuman->GetAIMovement()->Clear();

		GetAI()->GetAIMovement()->UnlockMovement();
	}

	else if ( m_pAIHuman->GetAIMovement()->IsUnset() )
	{
		LTVector vDest = pWaypoint->GetArgumentVector1();
		m_pAIHuman->GetAIMovement()->SetMovementDest(vDest);

		// Locking the movement prevents the goal from changing until 
		// ladder climbing has completed, and prevents the AI from rotating.

		GetAI()->GetAIMovement()->LockMovement();

		switch( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionClimbUpTo:
				m_pAIHuman->FaceTargetRotImmediately();

				if( m_pAIHuman->GetAwareness() == kAware_Alert )
				{
					m_pAIHuman->GetAIMovement()->PushAnimation(kAP_ClimbUpFast);
				}
				else {
					m_pAIHuman->GetAIMovement()->PushAnimation(kAP_ClimbUp);
				}
				break;

			case CAIPathWaypoint::eInstructionClimbDownTo:
				// Do not sink below the floor.
				vDest.y += m_pAIHuman->GetDims().y;

				if( m_pAIHuman->GetAwareness() == kAware_Alert )
				{
					m_pAIHuman->GetAIMovement()->PushAnimation(kAP_ClimbDownFast);
				}
				else {
					m_pAIHuman->GetAIMovement()->PushAnimation(kAP_ClimbDown);
				}
				break;

			case CAIPathWaypoint::eInstructionGetOnLadder:
				{
					CAIPathWaypoint* pNextWaypoint = m_pPath->GetNextWaypoint();
					LTVector vForward = pNextWaypoint->GetArgumentVector1();
					if( m_pAIHuman->GetRightVector().Dot( vForward ) > 0.f )
					{
						m_pAIHuman->GetAIMovement()->PushAnimation( kAP_GetOnLadderRight );
					}
					else {
						m_pAIHuman->GetAIMovement()->PushAnimation( kAP_GetOnLadderLeft );
					}
					m_pAIHuman->GetAnimationContext()->Lock();
				}
				break;

			case CAIPathWaypoint::eInstructionGetOffLadder:
				{
					LTVector vDir = pWaypoint->GetArgumentVector1() - m_pAIHuman->GetPosition();
					if( m_pAIHuman->GetRightVector().Dot( vDir ) > 0.f )
					{
						m_pAIHuman->GetAIMovement()->PushAnimation( kAP_GetOffLadderRight );
					}
					else {
						m_pAIHuman->GetAIMovement()->PushAnimation( kAP_GetOffLadderLeft );
					}
					m_pAIHuman->GetAnimationContext()->Lock();
				}
				break;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateJumpTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateJumpTo(CAIPathWaypoint* pWaypoint)
{
	if( ( !m_pAIHuman->GetAnimationContext()->IsLocked() ) || m_bCheckAnimStatus )
	{
		CAIMovement* pMovement = m_pAIHuman->GetAIMovement();
		LTVector vDest = pWaypoint->GetArgumentVector1();

		switch( pMovement->TopAnimation() )
		{
			case kAP_JumpStart:
				if ( pMovement->IsUnset() )
				{
					pMovement->PopAnimation();

					// Find the actual landing height.

					IntersectQuery IQuery;
					IntersectInfo IInfo;

					LTFLOAT fPeakHeight = vDest.y;

					IQuery.m_From = LTVector(vDest.x, vDest.y, vDest.z);
					IQuery.m_To = LTVector(vDest.x, vDest.y - 99999.f, vDest.z);
					IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
					IQuery.m_FilterFn = GroundFilterFn;

					g_cIntersectSegmentCalls++;
					if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
					{
						vDest.y = IInfo.m_Point.y;
					}					

					vDest.y += m_pAIHuman->GetDims().y;

					pMovement->SetMovementDest(vDest);
					pMovement->SetParabola( fPeakHeight - m_pAIHuman->GetPosition().y );
					pMovement->PushAnimation(kAP_JumpFly);
					m_pAIHuman->GetAnimationContext()->Lock();
					m_bCheckAnimStatus = LTTRUE;
					pMovement->IgnoreVolumes( LTTRUE );
				}
				break;

			case kAP_JumpFly:
				if ( pMovement->IsDone() )
				{
					m_bCheckAnimStatus = LTFALSE;
					pMovement->PopAnimation();
					pMovement->PushAnimation(kAP_JumpLand);
					m_pAIHuman->GetAnimationContext()->ClearLock();
					m_pAIHuman->GetAnimationContext()->Lock();
				}
				break;

			case kAP_JumpUpStart:
				if ( pMovement->IsUnset() )
				{
					pMovement->PopAnimation();
					pMovement->SetMovementDest(vDest);
					pMovement->PushAnimation(kAP_JumpUpFly);
					m_pAIHuman->GetAnimationContext()->Lock();
					m_bCheckAnimStatus = LTTRUE;
					pMovement->IgnoreVolumes( LTTRUE );
				}
				break;

			case kAP_JumpUpFly:
				if ( pMovement->IsDone() )
				{
					m_bCheckAnimStatus = LTFALSE;
					pMovement->PopAnimation();
					pMovement->PushAnimation(kAP_JumpUpLand);
					pMovement->IgnoreVolumes( LTTRUE );
					m_pAIHuman->GetAnimationContext()->ClearLock();
					m_pAIHuman->GetAnimationContext()->Lock();
				}
				break;

			case kAP_JumpDownStart:
				if ( pMovement->IsUnset() )
				{
					pMovement->PopAnimation();					
					pMovement->SetMovementDest(vDest);
					pMovement->PushAnimation(kAP_JumpDownFly);
					m_pAIHuman->GetAnimationContext()->Lock();
					m_bCheckAnimStatus = LTTRUE;
					pMovement->IgnoreVolumes( LTTRUE );
				}
				break;

			case kAP_JumpDownFly:
				if ( pMovement->IsDone() )
				{
					m_bCheckAnimStatus = LTFALSE;
					pMovement->PopAnimation();

					// Random chance of a bad landing.

					if( m_pAIHuman->GetBrain()->GetAIDataExist( kAIData_AccidentChance ) &&
						( GetRandom(0.0f, 1.0f) <= m_pAIHuman->GetBrain()->GetAIData( kAIData_AccidentChance ) ) )
					{
						m_pAIHuman->PlaySound( kAIS_Accident, LTTRUE );
						pMovement->PushAnimation(kAP_JumpDownLandBad);
					}
					else {
						pMovement->PushAnimation(kAP_JumpDownLand);
					}
					m_pAIHuman->GetAnimationContext()->ClearLock();
					m_pAIHuman->GetAnimationContext()->Lock();
				}
				break;

			case kAP_JumpLand:
			case kAP_JumpUpLand:
			case kAP_JumpDownLand:
			case kAP_JumpDownLandBad:
				m_pPath->IncrementWaypointIndex();
				pMovement->PopAnimation();
				pMovement->Clear();
				pMovement->UnlockMovement();
				break;

			default:
				if(pWaypoint->GetInstruction() == CAIPathWaypoint::eInstructionJumpUpTo)
				{
					pMovement->PushAnimation(kAP_JumpUpStart);
				}
				else if(pWaypoint->GetInstruction() == CAIPathWaypoint::eInstructionJumpDownTo)
				{
					pMovement->PushAnimation(kAP_JumpDownStart);
				}
				else {
					pMovement->PushAnimation(kAP_JumpStart);
				}

				m_pAIHuman->GetAnimationContext()->ClearLock();
				m_pAIHuman->GetAnimationContext()->Lock();
				break;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateOpenDoors
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateOpenDoors(CAIPathWaypoint* pWaypoint)
{
	// Clear any object AI was interacting with.

	GetAI()->SetAnimObject( LTNULL );

	HOBJECT hDoor1 = pWaypoint->GetArgumentHObject1();
	HOBJECT hDoor2 = pWaypoint->GetArgumentHObject2();

	RotatingDoor* pDoor1 = LTNULL;
	if( hDoor1 )
	{
		pDoor1 = (RotatingDoor*)pWaypoint->GetArgumentObject1();
	}
	RotatingDoor* pDoor2 = LTNULL;
	if( hDoor2 )
	{
		pDoor2 = (RotatingDoor*)pWaypoint->GetArgumentObject2();
	}

	// Breaking doors is a special case.  If AIBrain specifies BreakDoors,
	// AI will break any door encountered, whether it is open or shut.

	if( GetAI()->GetBrain()->GetAIDataExist( kAIData_BreaksDoors ) 
		&& ( GetAI()->GetBrain()->GetAIData( kAIData_BreaksDoors ) > 0.f ) )
	{
		switch( m_eDoorState )
		{
			case eDoorStateNone:
				m_eDoorAction = kAP_KickDoor;
				m_eDoorState = eDoorStateWaitingForAnimationToStart;
				return LTTRUE;

			case eDoorStateWaitingForAnimationToFinish:
				GetAI()->KickDoors( hDoor1, hDoor2 );
				m_eDoorState = eDoorStateWaitingForDoorToOpen;
				return LTTRUE;
		}
	}

	GetAI()->MarkDoors( hDoor1, hDoor2 );

	// Door(s) already open, so walk through.

	if( ( ( !pDoor1 ) || ( pDoor1 && ( pDoor1->GetState() == DOORSTATE_OPEN ) ) ) &&
		( ( !pDoor2 ) || ( pDoor2 && ( pDoor2->GetState() == DOORSTATE_OPEN ) ) ) )
	{
		if( hDoor1 )
		{
			AITRACE( AIShowVolumes, ( hDoor1, "Door opened by %s", GetAI()->GetName() ) );
		}

		m_eDoorState = eDoorStateWalkingThroughDoor;
		m_pPath->IncrementWaypointIndex();
		m_cStuckOnDoorUpdates = 0;
		m_bDoorShootThroughable = LTFALSE;
		return LTTRUE;
	}


	// Door(s) already opening, so wait for them to be fully open.
	// This prevents one AI from shutting a door immediately after another opens it.

	if( ( ( !pDoor1 ) || ( pDoor1 && ( pDoor1->GetState() == DOORSTATE_OPENING ) ) ) &&
		( ( !pDoor2 ) || ( pDoor2 && ( pDoor2->GetState() == DOORSTATE_OPENING ) ) ) )
	{
		m_eDoorState = eDoorStateWaitingForDoorToOpen;
	}

	// Retry the door if stuck.

	if( m_cStuckOnDoorUpdates > 2 )
	{
		m_eDoorState = eDoorStateNone;
	}

	// Check the progress of the AI trying to open the door(s).

	switch( m_eDoorState )
	{
		//
		// Just arrived at the door, so start an animation,
		// or start shooting till the door's unblocked.
		//

		case eDoorStateNone:
			{
				// Are doors blocked?

				if( ( m_pAIHuman->GetAwareness() != kAware_Relaxed ) &&
					( ( hDoor1 && DoorsBlocked( hDoor1 ) ) ||
					  ( hDoor2 && DoorsBlocked( hDoor2 ) ) ) )
				{
					// Is door1 shoot-throughable?

					uint32 dwUserFlags;
					SURFACE* pSurface;
					if( !m_bDoorShootThroughable )
					{
						g_pCommonLT->GetObjectFlags( hDoor1, OFT_User, dwUserFlags );
						pSurface = g_pSurfaceMgr->GetSurface( UserFlagToSurface( dwUserFlags ) );
						m_bDoorShootThroughable = pSurface->bCanShootThrough;
					}

					// Is door2 shoot-throughable?
	
					if( !m_bDoorShootThroughable )
					{
						g_pCommonLT->GetObjectFlags( hDoor2, OFT_User, dwUserFlags );
						pSurface = g_pSurfaceMgr->GetSurface( UserFlagToSurface( dwUserFlags ) );
						m_bDoorShootThroughable = pSurface->bCanShootThrough;
					}
				}



				// Decide how to open the door.

				switch( m_pAIHuman->GetAwareness() )	
				{
					case kAware_Relaxed:
						m_eDoorAction = kAP_OpenDoor;
						break;

					case kAware_Alert:
						{
							// If a door is shoot-throughable, randomly decide to shoot or kick it.

							if( m_bDoorShootThroughable )
							{
								m_eDoorAction = ( GetRandom(0.f, 1.f) > 0.5 ) ? kAP_KickDoor : kAP_None;
							}
							else {
								m_eDoorAction = kAP_KickDoor;
							}
						}
						break;

					case kAware_Suspicious:
						if( GetAI()->GetAlarmLevel() >= GetAI()->GetBrain()->GetImmediateAlarmThreshold() )
						{
							m_eDoorAction = kAP_KickDoor;
						}
						else {
							m_eDoorAction = ( GetRandom(0.f, 1.f) > 0.5 ) ? kAP_KickDoor : kAP_OpenDoor;
						}
						break;
				}

				if( m_eDoorAction != kAP_None )
				{
					m_eDoorState = eDoorStateWaitingForAnimationToStart;
				}
			}
			break;


		//
		// Animation started, and got the DOOR modelstring,
		// so trigger the door to open.
		//

		case eDoorStateWaitingForAnimationToFinish:
			{
				if( m_eDoorAction == kAP_OpenDoor )
				{
					GetAI()->OpenDoors( hDoor1, hDoor2 );
				}
				else {
					GetAI()->KickDoors( hDoor1, hDoor2 );
				}

				m_cStuckOnDoorUpdates = 0;
				if( hDoor1 )
				{
					m_fLastDoor1Yaw = pDoor1->GetYaw();
				}
				if( hDoor2 )
				{
					m_fLastDoor2Yaw = pDoor2->GetYaw();
				}

				m_eDoorState = eDoorStateWaitingForDoorToOpen;
			}
			break;

		//
		// Wait for the door to be fully open, and check if it is stuck.
		//

		case eDoorStateWaitingForDoorToOpen:
			{
				LTBOOL bStuck = LTFALSE;
				if( hDoor1 )
				{
					if( pDoor1->GetYaw() == m_fLastDoor1Yaw )
					{
						++m_cStuckOnDoorUpdates;
						bStuck = LTTRUE;
					}
					m_fLastDoor1Yaw = pDoor1->GetYaw();
				}

				if( hDoor2 && ( !bStuck ) )
				{
					if( pDoor2->GetYaw() == m_fLastDoor2Yaw )
					{
						++m_cStuckOnDoorUpdates;
						bStuck = LTTRUE;
					}
					m_fLastDoor2Yaw = pDoor2->GetYaw();
				}

				if( !bStuck )
				{
					m_cStuckOnDoorUpdates = 0;
				}
			}
			break;
	}

	return LTTRUE;
}

LTBOOL CAIHumanStrategyFollowPath::DoorsBlocked( HOBJECT hDoor )
{
	if( !m_pAIHuman->HasTarget() )
	{
		return LTFALSE;
	}

	CCharacter* pChar = m_pAIHuman->GetTarget()->GetCharacter();
	if( pChar )
	{
		LTVector vDoorPos;
		g_pLTServer->GetObjectPos(hDoor, &vDoorPos);		

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(hDoor, &vDims);
		LTFLOAT fDoorWidth = (vDims.x > vDims.z) ? vDims.x : vDims.z;

		g_pPhysicsLT->GetObjectDims(pChar->m_hObject, &vDims);

		if( vDoorPos.Dist( m_pAIHuman->GetTarget()->GetVisiblePosition() ) < (fDoorWidth * 2.f) + vDims.y )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateReleaseGate
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateReleaseGate(CAIPathWaypoint* pWaypoint)
{
	pWaypoint->ReleaseGate();
	m_pPath->IncrementWaypointIndex();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateWaitForDoors
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateWaitForDoors(CAIPathWaypoint* pWaypoint)
{
	LTBOOL bDoorsOpen = LTTRUE;
	LTBOOL bDoorsLocked = LTFALSE;

	m_eDoorState = eDoorStateWaitingForDoorToOpen;

	// See if we timed out on this door

	Door* pDoor1 = (Door*)pWaypoint->GetArgumentObject1();
	Door* pDoor2 = (Door*)pWaypoint->GetArgumentObject2();

	HOBJECT hDoor1, hDoor2;

	hDoor1 = (pDoor1) ? pDoor1->m_hObject : LTNULL;
	hDoor2 = (pDoor2) ? pDoor2->m_hObject : LTNULL;

	// Open doors if closed.

	if( ( pDoor1 && ( pDoor1->GetState() == DOORSTATE_CLOSED ) ) ||
		( pDoor2 && ( pDoor2->GetState() == DOORSTATE_CLOSED ) ) )
	{
		GetAI()->OpenDoors( hDoor1, hDoor2 );
	}

	// Check if doors are open.

	if( ( pDoor1 && ( pDoor1->GetState() != DOORSTATE_OPEN ) ) ||
		( pDoor2 && ( pDoor2->GetState() != DOORSTATE_OPEN ) ) )
	{
		bDoorsOpen = LTFALSE;
	}

	// Check if doors are locked.

	if( ( pDoor1 && pDoor1->IsLockedForCharacter( GetAI()->m_hObject) ) ||
		( pDoor2 && pDoor2->IsLockedForCharacter( GetAI()->m_hObject) ) )
	{
		bDoorsLocked = LTTRUE;
	}

	// Advance path when doors are open.

	if ( bDoorsOpen )
	{
		m_eDoorState = eDoorStateNone;
		m_pPath->IncrementWaypointIndex();
	}
	else if ( bDoorsLocked )
	{

	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateFaceJumpLand
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateFaceJumpLand(CAIPathWaypoint* pWaypoint)
{
	GetAI()->FaceDir(pWaypoint->GetArgumentVector1());
	GetAI()->FaceTargetRotImmediately();
	m_pPath->IncrementWaypointIndex();

	// Locking the movement prevents the goal from changing until 
	// the jump has completed, and prevents the AI from rotating.

	GetAI()->GetAIMovement()->LockMovement();

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateFaceLadder
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateFaceLadder(CAIPathWaypoint* pWaypoint)
{
	GetAI()->FaceDir(pWaypoint->GetArgumentVector1());

	LTFLOAT fDp = pWaypoint->GetArgumentVector1().Dot(GetAI()->GetForwardVector());

	if ( fDp > .9f )
	{
		m_pPath->IncrementWaypointIndex();
	}

	// Lock movement, but allow rotation.

	GetAI()->GetAIMovement()->LockMovement( LTFALSE );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateFaceDoor
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateFaceDoor(CAIPathWaypoint* pWaypoint)
{
	LTBOOL bDoorsOpen = LTTRUE;

	RotatingDoor* pDoors[2];
	pDoors[0] = (RotatingDoor*)pWaypoint->GetArgumentObject1();
	pDoors[1] = (RotatingDoor*)pWaypoint->GetArgumentObject2();

	for ( uint iDoor = 0 ; iDoor < 2 ; iDoor++ )
	{
		RotatingDoor* pDoor = pDoors[iDoor];
		if ( !pDoor ) continue;

		if ( pDoor->GetState() != DOORSTATE_OPEN )
		{
			bDoorsOpen = LTFALSE;
		}
	}

	if ( bDoorsOpen || GetAI()->FaceDir(pWaypoint->GetArgumentVector1()) )
	{
		m_pPath->IncrementWaypointIndex();
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Reset
//
//	PURPOSE:	Gets us ready for a new path
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Reset()
{
	if( m_bDrawingPath )
	{
		LineSystem::RemoveSystem(this,"ShowPath");
	}

	LineSystem::RemoveSystem(this, "ShowRemainingPath");

	m_eDoorState = eDoorStateNone;
	m_cStuckOnDoorUpdates = 0;
	m_bDoorShootThroughable = LTFALSE;
	m_pAIHuman->GetAIMovement()->Clear();
	m_eState = eStateUnset;
	m_bDrawingPath = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::Set(const LTVector& vDestination, LTBOOL bDivergePaths)
{
	ClearReservedPath();
	Reset();

	m_vDest = vDestination;

	// Fail to set a path to the exact current location.
	if(m_pAIHuman->GetAIMovement()->IsAtDest(vDestination))
	{
		m_eState = eStateDone;
		return LTTRUE;
	}

	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, bDivergePaths, m_pPath) ? eStateSet : eStateUnset;
	if ( IsSet() )
	{
		// IMPORTANT!!  Update the path if one is found to prevent weird sync
		// issues due other AI systems being out of sync with the pathing system.
		Update();

		if( bDivergePaths )
		{
			ReservePath();
		}
	}

	DebugDrawPath();
	return m_eState == eStateSet;
}

LTBOOL CAIHumanStrategyFollowPath::Set(const LTVector& vDestination, const LTVector& vDir, LTBOOL bDivergePaths)
{
	ClearReservedPath();
	Reset();

	m_vDest = vDestination;

	// Fail to set a path to the exact current location.
	if(m_pAIHuman->GetAIMovement()->IsAtDest(vDestination))
	{
		m_eState = eStateDone;
		return LTTRUE;
	}

	// Call the appropriate FindPath depending if a direction was supplied.

	if( (vDir.x != 0.f) || (vDir.y != 0.f) || (vDir.z != 0.f) )
	{
		m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, vDir, bDivergePaths, m_pPath) ? eStateSet : eStateUnset;
	}
	else {
		m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, bDivergePaths, m_pPath) ? eStateSet : eStateUnset;
	}

	if ( IsSet() )
	{
		// IMPORTANT!!  Update the path if one is found to prevent weird sync
		// issues due other AI systems being out of sync with the pathing system.
		Update();

		if( bDivergePaths )
		{
			ReservePath();
		}
	}

	DebugDrawPath();
	return m_eState == eStateSet;
}

LTBOOL CAIHumanStrategyFollowPath::Set(AINode* pNodeDestination, LTBOOL bDivergePaths)
{
	if( !pNodeDestination )
	{
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStrategyFollowPath::Set: Desitination node is NULL." );
		return LTFALSE;
	}

	ClearReservedPath();
	Reset();

	m_vDest = pNodeDestination->GetPos();
	return Set( m_vDest, bDivergePaths );
}

LTBOOL CAIHumanStrategyFollowPath::Set(AIVolume* pVolumeDestination, LTBOOL bDivergePaths)
{
	if( !pVolumeDestination )
	{
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStrategyFollowPath::Set: Desitination volume is NULL." );
		return LTFALSE;
	}

	ClearReservedPath();
	Reset();

	m_vDest = pVolumeDestination->GetCenter();

	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, bDivergePaths, m_pPath) ? eStateSet : eStateUnset;

	if ( IsSet() )
	{
		// IMPORTANT!!  Update the path if one is found to prevent weird sync
		// issues due other AI systems being out of sync with the pathing system.
		Update();

		if( bDivergePaths )
		{
			ReservePath();
		}
	}

	DebugDrawPath();

	return m_eState == eStateSet;
}

void CAIHumanStrategyFollowPath::ReservePath()
{
	if( !m_pPath )
	{
		return;
	}

	// An AI can reserve a path to make it less preferable for other AIs to use.
	// A path is reserved by marking the second to last volume as reserved.
	// The second to last volume is chosen, because AIs have no choice on the last
	// volume if they are heading to the same destination.

	AIVolume* pSecondToLastVolume = m_pPath->GetLastVolume( 1 );
	if( pSecondToLastVolume )
	{
		HOBJECT hAI = GetAI()->m_hObject;

		// Reserve the new volume.

		pSecondToLastVolume->ReserveVolume( hAI );
		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_ReservedVolume, GetAI(), pSecondToLastVolume, LTTRUE );
	}
}

void CAIHumanStrategyFollowPath::ClearReservedPath()
{
	HOBJECT hAI = GetAI()->m_hObject;

	// Clear any previous reservation.
	// AIs may only reserve one volume at a time.

	AIVolume* pReservedVolume = (AIVolume*)g_pAICentralKnowledgeMgr->GetKnowledgeTarget( kCK_ReservedVolume, GetAI() );
	while( pReservedVolume )
	{
		pReservedVolume->ClearVolumeReservation( GetAI()->m_hObject );

		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_ReservedVolume, GetAI(), pReservedVolume );
		
		// To be safe, remove all reserved volumes.

		AIASSERT( g_pAICentralKnowledgeMgr->CountMatches( kCK_ReservedVolume, GetAI() ) == 0, hAI,
			"CAIHumanStrategyFollowPath::ReservePath: AI is already reserving another path." );
		pReservedVolume = (AIVolume*)g_pAICentralKnowledgeMgr->GetKnowledgeTarget( kCK_ReservedVolume, GetAI() );
	}
}

LTBOOL CAIHumanStrategyFollowPath::SetRandom(AIVolume* pVolumeSrcPrev, AIVolume* pVolumeSrc, AIVolume* pVolumeSrcNext)
{
	Reset();

	m_eState = g_pAIPathMgr->RandomPath(GetAI(), pVolumeSrcPrev, pVolumeSrc, pVolumeSrcNext, 2000.0f, m_pPath) ? eStateSet : eStateUnset;
	DebugDrawPath();

	return m_eState == eStateSet;
}

void CAIHumanStrategyFollowPath::GetInitialDir(LTVector* pvDir)
{
	if( !pvDir )
	{
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStrategyFollowPath::GetInitialDir: Dir is NULL." );
		return;
	}

	m_pPath->GetInitialDir( pvDir ); 
}

void CAIHumanStrategyFollowPath::GetFinalDir(LTVector* pvDir)
{
	if( !pvDir )
	{
		AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStrategyFollowPath::GetFinalDir: Dir is NULL." );
		return;
	}

	m_pPath->GetFinalDir( pvDir ); 
}

AIVolume* CAIHumanStrategyFollowPath::GetNextVolume(AIVolume* pVolume, AIVolume::EnumVolumeType eVolumeType)
{
	return m_pPath->GetNextVolume( pVolume, eVolumeType );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyFollowPath::DebugRemainingDrawPath()
//              
//	PURPOSE:	Draws an AIs path from where it is to the end.  Does not draw
//				waypoints already reached.
//              
//----------------------------------------------------------------------------
void CAIHumanStrategyFollowPath::DebugRemainingDrawPath()
{
	DebugLineSystem& system = LineSystem::GetSystem(this, "ShowRemainingPath");
	system.Clear( );

	// If there is no path, there is nothing to draw.

	if( m_pPath == NULL )
	{
		return;
	}

	LTVector vLastPos;
	g_pLTServer->GetObjectPos(m_pAIHuman->m_hObject, &vLastPos);

	// Use this version of the iterator to draw starting from where
	// the AI is in his waypoint list
	
	for(int i = m_pPath->GetCurrentWaypointIndex(); i < m_pPath->GetNumWaypoints(); ++i )
	{
		if( m_pPath->GetWaypoint(i) )
		{
			CAIPathWaypoint::Instruction eInstruction;
			eInstruction = m_pPath->GetWaypoint(i)->GetInstruction();

			switch( eInstruction )
			{
				case CAIPathWaypoint::eInstructionMoveTo:
				case CAIPathWaypoint::eInstructionCrawlTo:
				case CAIPathWaypoint::eInstructionClimbUpTo:
				case CAIPathWaypoint::eInstructionClimbDownTo:
				case CAIPathWaypoint::eInstructionJumpUpTo:
				case CAIPathWaypoint::eInstructionJumpDownTo:
				case CAIPathWaypoint::eInstructionJumpOver:
				{
					LTVector vPos = m_pPath->GetWaypoint(i)->GetArgumentVector1();
					vPos.y += m_pAIHuman->GetDims().y;

					system << LineSystem::Arrow( vLastPos, vPos, Color::Red );
					vLastPos = vPos;
				}
				break;

				case CAIPathWaypoint::eInstructionMoveToTeleport:
				case CAIPathWaypoint::eInstructionMoveFromTeleport:
				{
					LTVector vPos = m_pPath->GetWaypoint(i)->GetArgumentVector1();
					vPos.y += m_pAIHuman->GetDims().y;

					system << LineSystem::Arrow( vLastPos, vPos, Color::White );
					vLastPos = vPos;
				}
				break;

				case CAIPathWaypoint::eInstructionFaceLadder:
				{
					LTVector vPos = m_pPath->GetWaypoint(i)->GetArgumentVector1();
					vPos.y += m_pAIHuman->GetDims().y;

					system << LineSystem::Arrow( vLastPos, vPos, Color::Yellow );
					vLastPos = vPos;
				}
				break;
			}
		}
	}
}



void CAIHumanStrategyFollowPath::DebugDrawPath()
{
	if( g_ShowAIPath.GetFloat() > 0.0f )
	{
   		DebugLineSystem & system = LineSystem::GetSystem(this, "ShowPath");
   		system.Clear( );

   		if( m_pPath )
   		{
   			LTVector vLastPos;
   			g_pLTServer->GetObjectPos(m_pAIHuman->m_hObject, &vLastPos);

  			for(int i = 0; i < m_pPath->GetNumWaypoints(); ++i )
   			{
   				if( m_pPath->GetWaypoint(i) )
   				{
   					CAIPathWaypoint::Instruction eInstruction;
   					eInstruction = m_pPath->GetWaypoint(i)->GetInstruction();

   					switch( eInstruction )
   					{
   						case CAIPathWaypoint::eInstructionMoveTo:
   						case CAIPathWaypoint::eInstructionCrawlTo:
   						case CAIPathWaypoint::eInstructionClimbUpTo:
   						case CAIPathWaypoint::eInstructionClimbDownTo:
   						case CAIPathWaypoint::eInstructionJumpUpTo:
   						case CAIPathWaypoint::eInstructionJumpDownTo:
   						case CAIPathWaypoint::eInstructionJumpOver:
   						case CAIPathWaypoint::eInstructionMoveToTeleport:
   						case CAIPathWaypoint::eInstructionMoveFromTeleport:
   						{
   							LTVector vPos = m_pPath->GetWaypoint(i)->GetArgumentVector1();
   							vPos.y += m_pAIHuman->GetDims().y;

							system << LineSystem::Arrow( vLastPos, vPos, Color::Blue );
   							vLastPos = vPos;
   						}
   						break;
   					}
   				}
   			}
   		}

   		m_bDrawingPath = LTTRUE;
	}
}
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	m_pPath->Load(pMsg);

	LOAD_DWORD_CAST(m_eDoorState, DoorState);
	LOAD_DWORD_CAST(m_eDoorAction, EnumAnimProp);
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eMedium, Medium);

	m_aniModifiedMovement.Load(pMsg);
	LOAD_BOOL(m_bModifiedMovement);
	LOAD_DWORD(m_cStuckOnDoorUpdates);
	LOAD_BOOL(m_bDoorShootThroughable);
	LOAD_FLOAT(m_fLastDoor1Yaw);
	LOAD_FLOAT(m_fLastDoor2Yaw);

	LOAD_BOOL(m_bCheckAnimStatus);

	LOAD_VECTOR(m_vDest);

	m_bDrawingPath = LTFALSE;

	// If we're loading from a transition, then our positional information is invalid.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION )
	{
		Reset();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	m_pPath->Save(pMsg);

	SAVE_DWORD(m_eDoorState);
	SAVE_DWORD(m_eDoorAction);
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eMedium);

	m_aniModifiedMovement.Save(pMsg);
	SAVE_BOOL(m_bModifiedMovement);
	SAVE_DWORD(m_cStuckOnDoorUpdates);
	SAVE_BOOL(m_bDoorShootThroughable);
	SAVE_FLOAT(m_fLastDoor1Yaw);
	SAVE_FLOAT(m_fLastDoor2Yaw);

	SAVE_BOOL(m_bCheckAnimStatus);

	SAVE_VECTOR(m_vDest);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::CAIHumanStrategyOneShotAni
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyOneShotAni::CAIHumanStrategyOneShotAni()
{
	m_eState = eUnset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Set
//
//	PURPOSE:	Sets the one shot ani
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Set(EnumAnimPropGroup eGroup, EnumAnimProp eProp)
{
	m_eState = eSet;
	m_Prop.Set(eGroup, eProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Update
//
//	PURPOSE:	Updates the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Update()
{
	CAIHumanStrategy::Update();

	switch ( m_eState )
	{
		case eUnset:
		{
			//_ASSERT(LTFALSE);
		}
		break;

		case eSet:
		{
			if ( GetAnimationContext()->IsLocked() )
			{
				m_eState = eAnimating;
			}
		}
		break;

		case eAnimating:
		{
			if ( !GetAnimationContext()->IsLocked() )
			{
				m_eState = eDone;
			}
		}
		break;

		case eDone:
		{
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyOneShotAni::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eState )
	{
		case eUnset:
/////		case eDone:
		{
		}
		break;

		case eSet:
		case eAnimating:
		case eDone:
		{
			GetAnimationContext()->Lock();
			GetAnimationContext()->SetProp(m_Prop);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_DWORD_CAST(m_eState, State);

	m_Prop.Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	SAVE_DWORD(m_eState);

	m_Prop.Save(pMsg);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::CAIHumanStrategyDodge
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyDodge::CAIHumanStrategyDodge()
{
	m_eDodgeStatus = eDodgeStatusOk;
	m_eState = eStateChecking;

	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
	m_pNode = LTNULL;

	m_bForceDodge = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Update()
{
	CAIHumanStrategy::Update();

	if ( m_eState == eStateChecking )
	{
		UpdateCheck();
	}
	else if ( m_eState == eStateDodging )
	{
		UpdateDodge();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyDodge::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	// Dodge animation is in progress.

	if( GetAnimationContext()->IsLocked() )
	{
		return LTTRUE;
	}

	if ( m_eState == eStateDodging )
	{
		switch ( m_eDodgeAction )
		{
			case eDodgeActionShuffle:
			{
				GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
				switch ( m_eDirection )
				{
					case eDirectionRight:
						GetAnimationContext()->SetProp(kAPG_Evasive, kAP_ShuffleRight);
						break;
					case eDirectionLeft:
					default:
						GetAnimationContext()->SetProp(kAPG_Evasive, kAP_ShuffleLeft);
						break;
				}

				// Allow a dodge to be played right after a locked attack.

				GetAnimationContext()->ClearLock();
				GetAnimationContext()->Lock();
			}
			break;

			case eDodgeActionRoll:
			{
				// Crouch if brain says we can.
				if( ( GetAI()->GetBrain()->GetAttackPoseCrouchChance() > 0.f ) &&
					( GetAI()->GetPrimaryWeaponType() == kAIWeap_Ranged ) )
				{
					GetAnimationContext()->SetProp(kAPG_Posture, kAP_Crouch);
				}
				else {
					GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
				}
				switch ( m_eDirection )
				{
					case eDirectionRight:
						GetAnimationContext()->SetProp(kAPG_Evasive, kAP_RollRight);
						break;
					case eDirectionLeft:
					default:
						GetAnimationContext()->SetProp(kAPG_Evasive, kAP_RollLeft);
						break;
				}

				// Allow a dodge to be played right after a locked attack.

				GetAnimationContext()->ClearLock();
				GetAnimationContext()->Lock();
			}
			break;

			case eDodgeActionBlock:
			{
				GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
				GetAnimationContext()->SetProp(kAPG_Evasive, kAP_Block);

				// Allow a dodge to be played right after a locked attack.

				GetAnimationContext()->ClearLock();
				GetAnimationContext()->Lock();
			}
			break;

			case eDodgeActionDive:
			{
			}
			break;

			case eDodgeActionFlee:
			{
			}
			break;

			case eDodgeActionCover:
			{
			}
			break;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateCheck
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateCheck()
{
	HOBJECT hNode = LTNULL;
	GetAI()->GetBrain()->GetDodgeStatus(&m_eDodgeStatus, &m_eDirection, &m_eDodgeAction, &hNode, m_bForceDodge);
	m_bForceDodge = LTFALSE;

	m_vDir = GetAI()->GetTorsoForward();

	if ( eDodgeStatusVector == m_eDodgeStatus )
	{
		if ( eDodgeActionShuffle == m_eDodgeAction )
		{
			if ( eDirectionRight == m_eDirection )
			{
///				m_vDir = GetAI()->GetTorsoRight();
			}
			else
			{
///				m_vDir = -GetAI()->GetTorsoRight();
			}
		}
		else if ( eDodgeActionRoll == m_eDodgeAction )
		{
			if ( eDirectionRight == m_eDirection )
			{
///				m_vDir = GetAI()->GetTorsoRight();
			}
			else
			{
///				m_vDir = -GetAI()->GetTorsoRight();
			}
		}
		else if ( eDodgeActionCover == m_eDodgeAction )
		{
			m_pNode = AINode::HandleToObject(hNode);
		}
	}
	else if ( eDodgeStatusProjectile == m_eDodgeStatus )
	{
		// We will flee
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodge
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodge()
{
	switch ( m_eDodgeAction )
	{
		case eDodgeActionShuffle:
		case eDodgeActionRoll:
		case eDodgeActionBlock:
			if ( !GetAnimationContext()->IsLocked() )
			{
				Check();
			}
			break;
		case eDodgeActionDive:
			UpdateDodgeDive();
			break;
		case eDodgeActionFlee:
			UpdateDodgeFlee();
			break;
		case eDodgeActionCover:
			UpdateDodgeCover();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodgeDive
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodgeDive()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodgeFlee
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodgeFlee()
{
	AIVolume* pVolume = GetAI()->GetLastVolume();

	if ( pVolume && g_pAIVolumeMgr->FindDangerScatterPosition(pVolume, GetAI()->GetPosition(), GetAI()->GetBrain()->GetDodgeProjectilePosition(), 40000.0f, &m_vScatterPosition) )
	{
		m_eState = eStateFleeing;
	}
	else
	{
		// Not going to work...

		m_eState = eStateChecking;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodgeCover
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodgeCover()
{
	if ( m_pNode )
	{
		m_eState = eStateCovering;
//		GetAI()->ChangeState("ATTACKFROMCOVER DEST=%s", g_pLTServer->GetStringData(m_pNode->GetName()));
	}
	else
	{
		// Not going to work...

		m_eState = eStateChecking;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCheck::Check
//
//	PURPOSE:	Go back to the checking state (reset all check times)
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Check()
{
	GetAI()->GetBrain()->DodgeDelay();

	m_eState = eStateChecking;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Dodge
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Dodge()
{
	_ASSERT(m_eState == eStateChecking && (m_eDodgeStatus != eDodgeStatusOk));

	if ( m_eState == eStateChecking && (m_eDodgeStatus != eDodgeStatusOk) )
	{
		m_eDodgeStatus = eDodgeStatusOk;

		// Snap AI to face the direction of his torso.

		GetAI()->FaceDir( m_vDir );
		GetAI()->FaceTargetRotImmediately();

		g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
		m_eState = eStateDodging;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_COBJECT(m_pNode, AINode);
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eDodgeStatus, DodgeStatus);
	LOAD_DWORD_CAST(m_eDodgeAction, DodgeAction);
	LOAD_VECTOR(m_vScatterPosition);
	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
	LOAD_BOOL(m_bForceDodge);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);
	
	SAVE_COBJECT(m_pNode);
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eDodgeStatus);
	SAVE_DWORD(m_eDodgeAction);
	SAVE_VECTOR(m_vScatterPosition);
	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
	SAVE_BOOL(m_bForceDodge);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::CAIHumanStrategyTaunt
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyTaunt::CAIHumanStrategyTaunt()
{
	m_eState = eStateChecking;
	m_fCheckTime = 0.f;
	m_fMinTauntDistSqr = 128.f * 128.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::Init
//
//	PURPOSE:	Initialize the strategy.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyTaunt::Init(CAIHuman* pAIHuman)
{
	if( !super::Init( pAIHuman ) )
	{
		return LTFALSE;
	}

	m_fCheckTime = g_pLTServer->GetTime() + pAIHuman->GetBrain()->GetTauntDelay();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyTaunt::Update()
{
	CAIHumanStrategy::Update();

	switch( m_eState )
	{
		case eStateChecking:
			if( ( !GetAnimationContext()->IsLocked() ) && 
				( g_pLTServer->GetTime() > m_fCheckTime ) )
		{
			if( GetAI()->GetTarget()->GetVisiblePosition().DistSqr( GetAI()->GetPosition() ) > m_fMinTauntDistSqr )
			{
				LTFLOAT fRandom = GetRandom(0.0f, 1.0f);
				if( fRandom < GetAI()->GetBrain()->GetTauntChance() )
				{
					m_eState = eStateTaunting;
				}
			}

			if( m_eState != eStateTaunting )
			{
				Check();
			}
		}
		break;
	
		case eStateTaunting:
			if( !GetAnimationContext()->IsLocked() )
			{
				Check();
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::Check
//
//	PURPOSE:	Set next check time.
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyTaunt::Check()
{
	m_fCheckTime = g_pLTServer->GetTime() + GetAI()->GetBrain()->GetTauntDelay();
	m_eState = eStateChecking;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyTaunt::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	if ( m_eState == eStateTaunting )
	{
		GetAnimationContext()->SetProp(kAPG_Action, kAP_Taunt);
		GetAnimationContext()->Lock();
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyTaunt::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_TIME(m_fCheckTime);
	LOAD_DWORD_CAST(m_eState, State);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyTaunt::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyTaunt::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	SAVE_TIME(m_fCheckTime);
	SAVE_DWORD(m_eState);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::CAIHumanStrategyCover
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyCover::CAIHumanStrategyCover()
{
	m_fCoverTime = 1.0f;
	m_fUncoverTime = 1.0f;
	m_bOneAnimCover = LTFALSE;
	m_bOneAnimFiring = LTFALSE;

	Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Init
//
//	PURPOSE:	Initializes strategy.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCover::Init(CAIHuman* pAIHuman)
{
	// AIData OneAnimCover tells an AI to animate attacking from cover with one
	// animation, instead of an out, attack/aim, and in.
	// Each strategy can override OneAnimCover() if it does not allow this feauture.

	if( pAIHuman )
	{
		if( pAIHuman->GetBrain() &&
			pAIHuman->GetBrain()->GetAIDataExist( kAIData_OneAnimCover ) &&
			( pAIHuman->GetBrain()->GetAIData( kAIData_OneAnimCover ) > 0.f ) )
		{
			m_bOneAnimCover = LTTRUE;
		}
	}

	return super::Init( pAIHuman );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Clear
//
//	PURPOSE:	Resets us to our initial state
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Clear()
{
	m_eState = eCovered;

	m_bWantCover = LTFALSE;
	m_fCoverTimer = 0.0f;
	m_fCoverDelay = 0.0f;

	m_bWantUncover = LTFALSE;
	m_fUncoverTimer = 0.0f;
	m_fUncoverDelay = 0.0f;

	m_bOneAnimFiring = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::CanBlindFire
//
//	PURPOSE:	Ais with projectile weapons can always blindfire.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCover::CanBlindFire() 
{
	// AIs with 1 anim attacks can always blind fire.  otherwise they'll never choose to play 
	// the anim that will bring them out to attack.

	return m_bOneAnimCover; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Update()
{
	CAIHumanStrategy::Update();

    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	switch ( m_eState )
	{
		case eCovered:
		{
			if ( GetAI()->HasTarget() )
			{
				CAITarget *pTarget = GetAI()->GetTarget();

				if ( pTarget->IsAttacking() )
				{
					// Target is shooting at us, stay covered

					m_fCoverTimer = 0.0f;

					return;
				}
			}

			m_fCoverTimer += fTimeDelta;

			// AIs with projectile weapons come out of cover and fire in one animation. 

			if ( m_fCoverTimer+fTimeDelta > m_fCoverTime )
			{
				m_eState = eWillUncover;

				// If attacking all in 1 anim, skip the intermediate steps.

				if( OneAnimCover() )
				{
					Update();
					SwitchToUncover();
					m_bOneAnimFiring = LTFALSE;
				}
			}

			if ( m_bWantUncover )
			{
				if ( m_fCoverTimer + fTimeDelta > m_fCoverDelay )
				{
					m_eState = eWillUncover;

					// If attacking all in 1 anim, skip the intermediate steps.

					if( OneAnimCover() )
					{
						Update();
						SwitchToUncover();
						m_bOneAnimFiring = LTFALSE;
					}
				}
			}
		}
		break;

		case eUncovered:
		{
			// If attacked all in 1 anim, skip the intermediate steps.

			if( OneAnimCover() )
			{
				if( ( m_bWantCover ) || 
					( m_bOneAnimFiring && ( !GetAI()->GetAnimationContext()->IsLocked() ) ) )
				{
					m_eState = eWillCover;
					Update();
					SwitchToCover();
				}
				return;
			}


			if ( GetAI()->HasTarget() && !m_bWantCover )
			{
				CAITarget *pTarget = GetAI()->GetTarget();

				if ( pTarget->IsAttacking() )
				{
					// Target is shooting at us, go back to cover

					m_bWantCover = LTTRUE;
					m_fUncoverDelay = m_fUncoverTimer + GetAI()->GetBrain()->GetAttackFromCoverReactionTime();

					return;
				}
			}

			m_fUncoverTimer += fTimeDelta;

			// AIs with projectile weapons come out of cover and fire in one animation. 

			if ( m_fUncoverTimer > m_fUncoverTime )
			{
				m_eState = eWillCover;
			}

			if ( m_bWantCover )
			{
				if ( m_fUncoverTimer + fTimeDelta > m_fUncoverDelay )
				{
					m_eState = eWillCover;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCover::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	// Only play the fire animation once per uncovering.

	if( OneAnimCover() && ( m_eState == eUncovered ) )
	{
		if( ( GetAI()->GetAnimationContext()->GetProp( kAPG_WeaponAction ) == kAP_Fire ) ||
			( GetAI()->GetAnimationContext()->GetProp( kAPG_WeaponAction ) == kAP_FireSecondary ) )
		{
			if( !m_bOneAnimFiring )
			{
				GetAnimationContext()->ClearLock();
				GetAI()->GetAnimationContext()->Lock();
				m_bOneAnimFiring = LTTRUE;
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::SwitchToCover
//
//	PURPOSE:	Changes our state to Covered
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::SwitchToCover()
{
	m_eState = eCovered;

	m_bWantCover = LTFALSE;
	m_fCoverTimer = 0.0f;
	m_fCoverDelay = 0.0f;

	m_bWantUncover = LTFALSE;
	m_fUncoverTimer = 0.0f;
	m_fUncoverDelay = 0.0f;

	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::SwitchToUncover
//
//	PURPOSE:	Changes our state to Uncovered
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::SwitchToUncover()
{
	m_eState = eUncovered;

	m_bWantCover = LTFALSE;
	m_fCoverTimer = 0.0f;
	m_fCoverDelay = 0.0f;

	m_bWantUncover = LTFALSE;
	m_fUncoverTimer = 0.0f;
	m_fUncoverDelay = 0.0f;

	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Cover
//
//	PURPOSE:	Requests that the strategy Covers
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Cover(LTFLOAT fDelay /* = 0.0f */)
{
	m_fUncoverDelay = m_fUncoverTimer + fDelay;
	m_bWantCover = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Uncover
//
//	PURPOSE:	Requests that the strategy Uncovers
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Uncover(LTFLOAT fDelay /* = 0.0f */)
{
	m_fCoverDelay = m_fCoverTimer + fDelay;
	m_bWantUncover = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_DWORD_CAST(m_eState, State);

	LOAD_BOOL(m_bWantCover);
	LOAD_FLOAT(m_fCoverTimer);
	LOAD_FLOAT(m_fCoverTime);
	LOAD_FLOAT(m_fCoverDelay);

	LOAD_BOOL(m_bWantUncover);
	LOAD_FLOAT(m_fUncoverTimer);
	LOAD_FLOAT(m_fUncoverTime);
	LOAD_FLOAT(m_fUncoverDelay);

	LOAD_BOOL(m_bOneAnimCover);
	LOAD_BOOL(m_bOneAnimFiring);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	SAVE_DWORD(m_eState);

	SAVE_BOOL(m_bWantCover);
	SAVE_FLOAT(m_fCoverTimer);
	SAVE_FLOAT(m_fCoverTime);
	SAVE_FLOAT(m_fCoverDelay);

	SAVE_BOOL(m_bWantUncover);
	SAVE_FLOAT(m_fUncoverTimer);
	SAVE_FLOAT(m_fUncoverTime);
	SAVE_FLOAT(m_fUncoverDelay);

	SAVE_BOOL(m_bOneAnimCover);
	SAVE_BOOL(m_bOneAnimFiring);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverDuck::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCoverDuck::Update()
{
	CAIHumanStrategyCover::Update();

	switch ( m_eState )
	{
		case eCovered:
		{

		}
		break;

		case eWillUncover:
		{
			m_eState = eUncovering;
		}
		break;

		case eUncovering:
		{
			if ( GetAnimationContext()->IsPropSet(kAPG_Evasive, kAP_Popup) )
			{
				SwitchToUncover();
			}
		}
		break;

		case eUncovered:
		{

		}
		break;

		case eWillCover:
		{
			m_eState = eCovering;
		}
		break;

		case eCovering:
		{
			if ( !GetAnimationContext()->IsPropSet(kAPG_Evasive, kAP_Popup) )
			{
				// TODO: this doesn't actually until the out-transition is done.
				// the above is true as soon as you start transitioning
				SwitchToCover();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverDuck::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCoverDuck::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eState )
	{
		case eWillCover:
		case eCovering:
		case eCovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Crouch);
		}
		break;

		case eWillUncover:
		case eUncovering:
		case eUncovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Evasive, kAP_Popup);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover1WayCorner::CAIHumanStrategyCover1WayCorner
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyCover1WayCorner::CAIHumanStrategyCover1WayCorner()
{
	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover1WayCorner::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategyCover::Save(pMsg);

	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategyCover::Load(pMsg);

	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Update()
{
	CAIHumanStrategyCover::Update();

	switch ( m_eState )
	{
		case eCovered:
		{

		}
		break;

		case eWillUncover:
		{
			m_vDir = m_pCoverNode->GetRight();

			if ( m_vDir.Dot(GetAI()->GetRightVector()) < 0.0f )
			{
				m_eDirection = eDirectionLeft;
			}
			else
			{
				m_eDirection = eDirectionRight;
			}

			m_eState = eUncovering;
		}
		break;

		case eUncovering:
		{
			if ( !GetAnimationContext()->IsLocked() )
			{
				SwitchToUncover();
			}
		}
		break;

		case eUncovered:
		{

		}
		break;

		case eWillCover:
		{
			m_vDir = -m_pCoverNode->GetRight();
			m_eState = eCovering;
		}
		break;

		case eCovering:
		{
			if ( !GetAnimationContext()->IsLocked() )
			{
				SwitchToCover();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover1WayCorner::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCover1WayCorner::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eState )
	{
		case eCovered:
		{
 			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Covered);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		}
		break;

		case eUncovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Uncovered);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
		}
		break;

		case eWillCover:
		case eCovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Covering);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
			GetAnimationContext()->Lock();
		}
		break;

		case eWillUncover:
		case eUncovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Uncovering);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
			GetAnimationContext()->Lock();
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover2WayCorner::CAIHumanStrategyCover2WayCorner
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyCover2WayCorner::CAIHumanStrategyCover2WayCorner()
{
	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover2WayCorner::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategyCover::Save(pMsg);

	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategyCover::Load(pMsg);

	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Update()
{
	CAIHumanStrategyCover::Update();

	if ( !GetAI()->HasTarget() ) return;

	switch ( m_eState )
	{
		case eCovered:
		{

		}
		break;

		case eWillUncover:
		{
			CAITarget* pTarget = GetAI()->GetTarget();
			HOBJECT hTarget = pTarget->GetObject();

			LTVector vTargetPos;
			g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

			LTVector vDir = vTargetPos - m_pCoverNode->GetPos();
			vDir.Normalize();

			if ( vDir.Dot(m_pCoverNode->GetRight()) < 0.0f )
			{
				// Target is right of forward vector, so Step right

				m_eDirection = eDirectionRight;
				m_vDir = m_pCoverNode->GetRight();
			}
			else
			{
				// Target is left of or dead on forward vector, Step left

				m_eDirection = eDirectionLeft;
				m_vDir = -m_pCoverNode->GetRight();
			}

			m_eState = eUncovering;
		}
		break;

		case eUncovering:
		{
			if ( !GetAnimationContext()->IsLocked() )
			{
				SwitchToUncover();
			}
		}
		break;

		case eUncovered:
		{

		}
		break;

		case eWillCover:
		{
			// Step in the direction of the node

/*			m_vDir = m_pCoverNode->GetPos() - GetAI()->GetPosition();
			m_vDir.Normalize();

			if ( m_vDir.Dot(GetAI()->GetRightVector()) > 0.0f )
			{
				// Target is right of forward vector, so Step right

				m_eDirection = eDirectionRight;
			}
			else
			{
				// Target is left of or dead on forward vector, Step left

				m_eDirection = eDirectionLeft;
			}
*/
			m_vDir = -m_vDir;
			m_eState = eCovering;
		}
		break;

		case eCovering:
		{
			if ( !GetAnimationContext()->IsLocked() )
			{
				SwitchToCover();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover2WayCorner::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCover2WayCorner::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eState )
	{
		case eCovered:
		{
 			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Covered);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		}
		break;

		case eUncovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Uncovered);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
		}
		break;

		case eWillCover:
		case eCovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Covering);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
			GetAnimationContext()->Lock();
		}
		break;

		case eWillUncover:
		case eUncovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Uncovering);
			GetAnimationContext()->SetProp(kAPG_Evasive, m_eDirection == eDirectionRight ? kAP_CornerRight : kAP_CornerLeft);
			GetAnimationContext()->Lock();
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverBlind::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCoverBlind::Update()
{
	CAIHumanStrategyCover::Update();

	switch ( m_eState )
	{
		case eCovered:
		{

		}
		break;

		case eWillUncover:
		{
			m_eState = eUncovering;
		}
		break;

		case eUncovering:
		{
			if ( GetAnimationContext()->IsPropSet(kAPG_Evasive, kAP_Blind) )
			{
				SwitchToUncover();
			}
		}
		break;

		case eUncovered:
		{

		}
		break;

		case eWillCover:
		{
			m_eState = eCovering;
		}
		break;

		case eCovering:
		{
			if ( !GetAnimationContext()->IsPropSet(kAPG_Evasive, kAP_Blind) )
			{
				// TODO: this doesn't actually until the out-transition is done.
				// the above is true as soon as you start transitioning
				SwitchToCover();
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverBlind::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyCoverBlind::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eState )
	{
		case eWillCover:
		case eCovering:
		case eCovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Crouch);
		}
		break;

		case eWillUncover:
		case eUncovering:
		case eUncovered:
		{
			GetAnimationContext()->SetProp(kAPG_Posture, kAP_Crouch);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
			GetAnimationContext()->SetProp(kAPG_Evasive, kAP_Blind);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::CAIHumanStrategyGrenadeThrow
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyGrenadeThrow::CAIHumanStrategyGrenadeThrow()
{
	m_eState = eStateNone;
	m_fHangtime = 0.5f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenadeThrow::HandleModelString(ArgList* pArgList)
{
	CAIHumanStrategyGrenade::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 || !pArgList->argv[0] ) return;

	if ( !_stricmp(pArgList->argv[0], "THROW") )
	{
		if ( eStateThrowing == m_eState )
		{
			m_eState = eStateThrow;
		}
		else
		{
			_ASSERT(LTFALSE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::Update
//
//	PURPOSE:	Updates our state
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenadeThrow::Update()
{
	CAIHumanStrategyGrenade::Update();

	if ( !GetAI()->HasTarget() ) return;

	switch ( m_eState )
	{
		case eStateNone:
			_ASSERT(LTFALSE);
			return;
			break;
		case eStateThrowing:
			break;
		case eStateThrow:
			break;
		case eStateThrown:
			if ( !GetAnimationContext()->IsLocked() )
			{
				m_eState = eStateDone;
			}
			break;
		case eStateDone:
			m_eState = eStateNone;
			break;
	}

	GetAI()->FaceTarget();

	if ( eStateThrow == m_eState )
	{
		HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
		LTVector vTargetPos;
		g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

		CWeapon* pWeapon = GetAI()->GetWeapon( kAIWeap_Thrown );
		if ( !pWeapon ) return;

		// Get our fire position

		LTVector vFirePos = GetAI()->GetWeaponPosition(pWeapon);

		// Velocity Vo

		LTVector vGravity;
		g_pPhysicsLT->GetGlobalForce(vGravity);

		// Vo = (S - R - 1/2*G*t^2) / t         Vo = initial velocity, S = destination, R = origin, G = gravity, t = hangtime

		LTVector vVelocity = (vTargetPos - vFirePos - vGravity*.5f*m_fHangtime*m_fHangtime)/m_fHangtime;
		LTFLOAT fVelocity = vVelocity.Mag();
		LTVector vDir(vVelocity/fVelocity);

		// Now fire the weapon

		WeaponFireInfo weaponFireInfo;
		weaponFireInfo.hFiredFrom = GetAI()->GetObject();
		weaponFireInfo.vPath		= vDir;
		weaponFireInfo.bOverrideVelocity = LTTRUE;
		weaponFireInfo.fOverrideVelocity = fVelocity;
		weaponFireInfo.vFirePos	= vFirePos;
		weaponFireInfo.vFlashPos	= vFirePos;
		weaponFireInfo.hTestObj	= hTarget;
		weaponFireInfo.fPerturbR	= 1.0f*(1.0f - GetAI()->GetAccuracy());
		weaponFireInfo.fPerturbU	= 1.0f*(1.0f - GetAI()->GetAccuracy());

		pWeapon->ReloadClip(LTFALSE);

		pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999 );

		pWeapon->UpdateWeapon(weaponFireInfo, LTTRUE);

		m_eState = eStateThrown;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::UpdateAnimation
//
//	PURPOSE:	Handles any pending animator changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyGrenadeThrow::UpdateAnimation()
{
	if( !CAIHumanStrategyGrenade::UpdateAnimation() )
	{
		return LTFALSE;
	}

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Throw);
	GetAnimationContext()->Lock();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::*Throw*
//
//	PURPOSE:	Throwing related methods
//
// ----------------------------------------------------------------------- //
	
void CAIHumanStrategyGrenadeThrow::Throw(LTFLOAT fTime /*= 0.5f*/)
{
	m_eState = eStateThrowing;
	m_fHangtime = fTime;
}

LTBOOL CAIHumanStrategyGrenadeThrow::ShouldThrow()
{
	if ( eGrenadeStatusDontThrow == GetAI()->GetBrain()->GetGrenadeStatus() )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

LTBOOL CAIHumanStrategyGrenadeThrow::IsThrowing()
{
	return eStateDone != m_eState && eStateNone != m_eState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenadeThrow::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategyGrenade::Load(pMsg);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_FLOAT(m_fHangtime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenadeThrow::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategyGrenade::Save(pMsg);

	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fHangtime);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::CAIHumanStrategyShoot
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyShoot::CAIHumanStrategyShoot()
{
	m_eState = eStateAiming;
	m_bFired = LTFALSE;
	m_bNeedsReload = LTFALSE;
	m_bOutOfAmmo = LTFALSE;
	m_bFirstUpdate = LTTRUE;
	m_bIgnoreFOV = LTFALSE;
	m_bShootBlind = LTFALSE;
	m_hFiringSocket = INVALID_MODEL_SOCKET;
	m_iAnimRandomSeed = -1;
}

CAIHumanStrategyShoot::~CAIHumanStrategyShoot()
{
	Aim();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyShoot::Init(CAIHuman* pAIHuman)
{
	if( !super::Init( pAIHuman ) )
	{
		return LTFALSE;
	}

	// Get a random seed to use when playing aim and fire.
	// Only do this for rifles.

	if( GetAI()->GetCurrentWeaponProp() == kAP_Weapon1 )
	{
		CAnimationProps Props;
		Props.Set(kAPG_Posture, kAP_Stand);
		Props.Set(kAPG_WeaponPosition, kAP_Up);
		Props.Set(kAPG_WeaponAction, kAP_Aim);
		Props.Set(kAPG_Weapon, kAP_Weapon1);
		m_iAnimRandomSeed = GetAnimationContext()->ChooseRandomSeed( Props );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Clear
//
//	PURPOSE:	Resets us to our initial state
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Clear()
{
	m_eState = eStateAiming;
	m_bFired = LTFALSE;
	m_bNeedsReload = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Reload
//
//	PURPOSE:	Reloads our weapon
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Reload(LTBOOL bInstant /* = LTFALSE */)
{
	CWeapon *pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon && pWeapon->GetParent() )
	{
		Aim();
		pWeapon->ReloadClip(LTFALSE);
		
		// After a reload we may not have any ammo in our reserves.  If we 
		// have no ammo, and if we are allowed to regenerate amo, then do so
		// IF we are allowed to generate this type of ammo
		if ( 0 == pWeapon->GetParent()->GetAmmoCount(pWeapon->GetAmmoId()) &&
			CanGenerateAmmo(pWeapon->GetAmmoId()) )
		{
			pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999 );
		}

		if ( !bInstant && pWeapon->GetWeaponData()->bAIAnimatesReloads )
		{
			m_eState = eStateReloading;
		}

		m_bNeedsReload = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::HandleModelString(ArgList* pArgList)
{
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyFireWeapon) )
	{
		HandleFired( (pArgList->argc>=2 ? pArgList->argv[1] : NULL) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::HandleFired
//
//	PURPOSE:	Handle the fire key
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::HandleFired( const char* const pszSocketName )
{
	if ( pszSocketName == NULL )
	{
		// set the socket to fire from to NULL so we know to
		// use normal firing
		m_hFiringSocket = INVALID_MODEL_SOCKET;
	}
	else
	{
		// Set the socket to fire from to the socket named if it exists

		// check to see if we already have the socket so we can try to
		// avoid annoying lookups.
		if ( LT_OK != g_pModelLT->GetSocket( GetAI()->m_hObject,
			const_cast<char*>(pszSocketName), m_hFiringSocket) )
		{
			m_hFiringSocket = INVALID_MODEL_SOCKET;
		}
	}

	m_bFired = LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShoot::GetFirePosition()
//              
//	PURPOSE:	Returns the location shots should come from, using either the
//				over ride specified socket, or the weapon position if the
//				socket is invalid.
//              
//----------------------------------------------------------------------------
LTVector CAIHumanStrategyShoot::GetFirePosition(CWeapon* pWeapon)
{
	if ( m_hFiringSocket != INVALID_MODEL_SOCKET )
	{
		LTransform transform;
		LTRESULT SocketTransform = g_pModelLT->GetSocketTransform(GetAI()->m_hObject, m_hFiringSocket, transform, LTTRUE);
		AIASSERT( SocketTransform == LT_OK, GetAI()->m_hObject, "Unable to get socket for transform" );
		return transform.m_Pos;
	}
	else
	{
		return GetAI()->GetWeaponPosition(pWeapon);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShoot::CanGenerateAmmo()
//              
//	PURPOSE:	Query whether or not this AI is allowed to generate ammo when
//				they are out.  Previously, the AI gave itself 999999 ammo if 
//				it ran out.  Some AIs should not be able to refresh their ammo
//				in this manner.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStrategyShoot::CanGenerateAmmo( int nAmmoID )
{
	// Test our brain to see if we are allowed to regenerate this type of
	// ammo
	if ( GetAI()->GetBrain()->IsAmmoGenerationRestricted( nAmmoID ) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShoot::ShouldReload()
//              
//	PURPOSE:	Check to determine if the AI ought to start reloading.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStrategyShoot::ShouldReload()
{
	// We should reload if we Need to reload and if we Can reload
	return (NeedsReload() && CanReload() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShoot::ForceFire()
//              
//	PURPOSE:	Force AI to shoot with the current weapon.
//              
//----------------------------------------------------------------------------
void CAIHumanStrategyShoot::ForceFire(HOBJECT hTargetObject)
{
	LTVector vTargetPos;

	// Aim at the target object's position.

	if( hTargetObject )
	{
		g_pLTServer->GetObjectPos( hTargetObject, &vTargetPos );
	}

	// If no target object is provided, use the AITarget.

	else if( GetAI()->HasTarget() )
	{
		hTargetObject = GetAI()->GetTarget()->GetObject();
		GetAI()->GetTarget()->GetShootPosition( &vTargetPos );
	}

	if( hTargetObject )
	{
		CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
		if( pWeapon )
		{
			UpdateFiring( hTargetObject, vTargetPos, pWeapon );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Update
//
//	PURPOSE:	Updates the Strategy
//
//	NOTES:		This is NOT a virtual function. Derived shooting strategies
//				should implement UpdateFiring/UpdateAiming().
//
//  NOTES:		If hTarget is supplied, that means we're firing at a prop or
//				something. If it's not, it means to use the AI's target.
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Update(HOBJECT hTarget /* = LTNULL */)
{
	CAIHumanStrategy::Update();

	LTBOOL bTargetProp = LTFALSE;

	if ( hTarget )
	{
		bTargetProp = LTTRUE;
	}
	else
	{
		hTarget = GetAI()->HasTarget() ? GetAI()->GetTarget()->GetObject() : LTNULL;
	}

	// If we are out of ammo and change strategies, then we need the
	// strategy to test being out of ammo before it attempts to act.
	// this FirstUpdate is nasty, but accomplishes this.
	if ( m_bFirstUpdate )
	{
		CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
		if ( !pWeapon || !pWeapon->GetParent() )
		{
			return;
		}

		UpdateNeedsReload( pWeapon);		
	}


	// If we don't/no longer have a target, this strategy cannot work
	// If we need to reload because our clip is empty, and if we cannot reload
	// then this strategy is completely broken
	if ( !hTarget || ( NeedsReload() && !CanReload() ) )
	{
		return;
	}

	// Get our current weapon

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( !pWeapon || !pWeapon->GetParent() ) return;


	if ( !IsReloading() )
	{
		// Fire

		if ( m_bFired )
		{
			// Get the target's position

			LTVector vTargetPos;
			if ( !bTargetProp )
			{
				GetAI()->GetTarget()->GetShootPosition( &vTargetPos );
			}
			else
			{
				g_pLTServer->GetObjectPos(hTarget, &vTargetPos);
			}

			UpdateFiring(hTarget, vTargetPos, pWeapon);
		}
		else
		{
			UpdateAiming(hTarget);
		}

		// Update whether or not we're going to need to reload

		UpdateNeedsReload(pWeapon);
	}
	else
	{
		// Continue reloading

		UpdateReloading(pWeapon);
	}

	m_bFirstUpdate = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyShoot::UpdateAnimation()
{
	if( !super::UpdateAnimation() )
	{
		return LTFALSE;
	}

	if(m_pAIHuman->GetNumWeapons() && m_pAIHuman->GetWeapon(0) == LTNULL)
	{
		return LTFALSE;
	}

	GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );

	switch ( m_eState )
	{
		case eStateNone:
		{
		}
		break;

		case eStateAiming:
		{
			GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Aim);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		}
		break;

		case eStateFiring:
		{
			if(!GetAnimationContext()->IsLocked())
			{
				// Fire with primary or secondary weapon.

				if( GetAI()->GetCurrentWeapon() == GetAI()->GetPrimaryWeapon() )
				{
					GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Fire);
				}
				else {
					GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireSecondary);
				}
				GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
	
				// AIs who cannot attack while moving need to play their
				// attacks all the way through.
				// (e.g. fancy ninja and supersoldier attacks)
				// Secondary attacks always play all the way through
				// (e.g. shuriken and headbutts. )

				if( ( !GetAI()->GetBrain()->AttacksWhileMoving() ) ||
					( GetAI()->GetCurrentWeapon() != GetAI()->GetPrimaryWeapon() ) )
				{
					// ClearLock is necessary to allow the same locked animation
					// to be played consecutively.
					GetAnimationContext()->ClearLock();
					GetAnimationContext()->Lock();
				}
			}
		}
		break;

		case eStateReloading:
		{
			GetAnimationContext()->Lock();
			GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_Reload);
			GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);
		}
		break;

		default:
		{
			_ASSERT(LTFALSE);
		}
		break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::UpdateReloading
//
//	PURPOSE:	Update our reloading
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::UpdateReloading(CWeapon* pWeapon)
{
	if ( !GetAnimationContext()->IsLocked() )
	{
		m_eState = eStateAiming;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::UpdateNeedsReload
//
//	PURPOSE:	Update whether or not we need to reload
//
// ----------------------------------------------------------------------- //
void CAIHumanStrategyShoot::UpdateNeedsReload(CWeapon* pWeapon)
{
	// Get the weapon state.  If no weapon, we need to reload.  If we do 
	// have a weapon, then check to see if its clip is empty.
	if ( pWeapon == NULL || ( pWeapon && 0 == pWeapon->GetAmmoInClip() ) )
	{
		m_bNeedsReload = LTTRUE;
	}
	else
	{
		m_bNeedsReload = LTFALSE;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShoot::CanReload()
//              
//	PURPOSE:	Query whether we can reload or not.  Assuming either a full 
//				clip can be reloaded, or that a full clip reload will not be
//				attempted.
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStrategyShoot::CanReload()
{
	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon && pWeapon->GetParent() )
	{
		// If we have ammo in our reserves, we can reload
		if ( pWeapon->GetParent()->GetAmmoCount( pWeapon->GetAmmoId() ) != 0 )
		{
			return LTTRUE;
		}

		// If we can generate ammo, then we can definitely reload
		if ( CanGenerateAmmo( pWeapon->GetAmmoId() ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Aim
//
//	PURPOSE:	Makes us Aim
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Aim()
{
	m_eState = eStateAiming;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Fire
//
//	PURPOSE:	Makes us fire
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Fire()
{
	// Melee weapons push targets back.

	AIASSERT( GetAI()->GetCurrentWeapon() && GetAI()->GetCurrentWeapon()->GetWeaponData(), GetAI()->m_hObject, "CAIHumanStrategyShoot::Fire: No weapon or weapon data.");

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon->GetWeaponData()->nAIWeaponType == kAIWeap_Melee )
	{
		GetAI()->GetTarget()->SetPushSpeed(800.f);
		GetAI()->GetTarget()->SetPushMinDist( GetAI()->GetBrain()->GetAIData( kAIData_MeleeKnockBackDist ) );
	}
	
	m_eState = eStateFiring;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bFired);
	LOAD_BOOL(m_bNeedsReload);
	LOAD_BOOL(m_bOutOfAmmo);
	LOAD_BOOL(m_bFirstUpdate);
	LOAD_BOOL(m_bIgnoreFOV);
	LOAD_BOOL(m_bShootBlind);
	LOAD_INT(m_hFiringSocket);
	LOAD_DWORD(m_iAnimRandomSeed);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bFired);
	SAVE_BOOL(m_bNeedsReload);
	SAVE_BOOL(m_bOutOfAmmo);
	SAVE_BOOL(m_bFirstUpdate);
	SAVE_BOOL(m_bIgnoreFOV);
	SAVE_BOOL(m_bShootBlind);
	SAVE_INT(m_hFiringSocket);
	SAVE_DWORD(m_iAnimRandomSeed);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::CAIHumanStrategyShootBurst
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyShootBurst::CAIHumanStrategyShootBurst()
{
	m_bIgnoreFOV = LTFALSE;
	m_fBurstInterval = 0.f;
	m_nBurstShots = 0;
	m_bUseIntervals = LTTRUE;
	m_bPreFire = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyShootBurst::Init(CAIHuman* pAIHuman)
{
	_ASSERT(pAIHuman);
	if ( !pAIHuman ) return LTFALSE;

	if ( !CAIHumanStrategyShoot::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Weapons.txt can define some weapons to ignore intervals,
	// and just fire continuously.

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon && pWeapon->GetWeaponData() )
	{
		if( ( pWeapon->GetWeaponData()->fAIMinBurstInterval == 0.f ) &&
			( pWeapon->GetWeaponData()->fAIMaxBurstInterval == 0.f ) &&
			( pWeapon->GetWeaponData()->nAIMinBurstShots == 0 ) &&
			( pWeapon->GetWeaponData()->nAIMaxBurstShots == 0 ) )
		{
			m_bUseIntervals = LTFALSE;
		}
	}

	// All recalculations of the burst firing now handled
	// by this function to reduce duplication
	CalculateBurst();

	// Do not wait before first shot.

	m_fBurstInterval = 0.f;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::HandleFired
//
//	PURPOSE:	Handle the fire key
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::HandleFired(const char* const pszSocketName)
{
	CAIHumanStrategyShoot::HandleFired(pszSocketName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategyShoot::Load(pMsg);

	LOAD_TIME(m_fBurstInterval);
	LOAD_INT(m_nBurstShots);
	LOAD_BOOL(m_bUseIntervals);
	LOAD_BOOL(m_bPreFire);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategyShoot::Save(pMsg);

	SAVE_TIME(m_fBurstInterval);
	SAVE_INT(m_nBurstShots);
	SAVE_BOOL(m_bUseIntervals);
	SAVE_BOOL(m_bPreFire);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::UpdateFiring
//
//	PURPOSE:	Handles firing
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon)
{
	// Do not fire if we're in the middle of a burst interval.

	if( m_bUseIntervals && 
		( ( m_nBurstShots <= 0 ) || ( m_fBurstInterval > g_pLTServer->GetTime() ) ) )
	{
		// We just finished our burst. Start waiting.
		// And just aim.

		Aim();
		return;
	}

	// Decrement burst count for this shot.

	m_nBurstShots--;

	// Get our fire position

	AIASSERT( GetAI()->GetCurrentWeapon(), GetAI()->m_hObject, "UpdateFiring without a weapon" );
	LTVector vFirePos = GetFirePosition(GetAI()->GetCurrentWeapon());

	// Get our firing vector

	LTVector vDir = !IsBlind() ? vTargetPos - vFirePos : GetAI()->GetTorsoForward();

	// Now fire the weapon

	WeaponFireInfo weaponFireInfo;

	// This info is currently used only bu the disc.  Setting it for other
	// weapons doesn't hurt currently, so until we have a better solution, 
	// or a need to change it, I am leaving it in.
//	To be enabled ones the disc homing works
//	if ( hTarget )
//	{
//		weaponFireInfo.nDiscTrackingType = MPROJ_DISC_TRACKING_HOMING;
//		weaponFireInfo.hObjectTarget = hTarget;
//	}
//	else
	{
		weaponFireInfo.nDiscTrackingType = MPROJ_DISC_TRACKING_STEADY;
	}

	weaponFireInfo.hSocket		= m_hFiringSocket!=INVALID_MODEL_SOCKET ? m_hFiringSocket : GetAI()->GetWeaponSocket(GetAI()->GetCurrentWeapon());
	weaponFireInfo.hFiredFrom  = GetAI()->GetObject();
	weaponFireInfo.vPath       = vDir;
	weaponFireInfo.vFirePos    = vFirePos;
	weaponFireInfo.vFlashPos   = vFirePos;
	weaponFireInfo.hTestObj    = hTarget;

	if ( !IsBlind() )
	{
		weaponFireInfo.fPerturbR	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
		weaponFireInfo.fPerturbU	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
	}
	else
	{
		weaponFireInfo.fPerturbR	= 10.0f*LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
		weaponFireInfo.fPerturbU	= 10.0f*LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
	}

	WeaponState eWeaponState = pWeapon->UpdateWeapon(weaponFireInfo, LTTRUE);

	if( eWeaponState == W_FIRED )
	{
		// Play the HHWeapon ani when we actually fire the weapon...

		// Set the proper weapon animations...
	
		if( m_bPreFire )
		{
			pWeapon->PlayAnimation( pWeapon->GetPreFireAni(), true, false, true );
			m_bPreFire = LTFALSE;
		}
		else
		{
			// Loop the fire animation
			pWeapon->PlayAnimation( pWeapon->GetFireAni(), false, true, false );
		}
		
		// Keep firing

		Fire();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::CalculateBurst
//
//	PURPOSE:	Calculate all our burst parameters
//
// ----------------------------------------------------------------------- //
void CAIHumanStrategyShootBurst::CalculateBurst()
{
	if( !m_bUseIntervals )
	{
		return;
	}

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();

	if ( pWeapon && pWeapon->GetWeaponData() )
	{
		m_fBurstInterval = g_pLTServer->GetTime() + GetRandom(pWeapon->GetWeaponData()->fAIMinBurstInterval, pWeapon->GetWeaponData()->fAIMaxBurstInterval);
		m_nBurstShots = GetRandom(pWeapon->GetWeaponData()->nAIMinBurstShots, pWeapon->GetWeaponData()->nAIMaxBurstShots);

		// Having a ShotsPerClip of 0 is a special case used when we want 
		// animations dictate the number of shots in a clip.  For example, a 
		// ninja throwing stars for example will have several stars thrown 
		// through an animation.  We want to play that animation and not worry
		// about the exact number of shots in his burst.
		if ( pWeapon->GetWeaponData()->nShotsPerClip > 0 )
		{
			// Limit burst shots to the number of ammo we have in our clip.  This
			// WON'T protect us from an animation with more fire commands than 
			// BurstShots set here.  If that happens, all we can do is either allow
			// this to happen, or block the fire commands from being processed.
			if ( m_nBurstShots > pWeapon->GetAmmoInClip() )
			{
				m_nBurstShots = pWeapon->GetAmmoInClip();
			}
		}
	}
	else
	{
		m_fBurstInterval = (LTFLOAT)INT_MAX;
		m_nBurstShots = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::UpdateAiming
//
//	PURPOSE:	Handles Aiming
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::UpdateAiming(HOBJECT hTarget)
{
	if ( m_bUseIntervals && ( m_nBurstShots <= 0 ) )
	{
		// Don't calculate new burst until finished firing animation.
		if( !GetAnimationContext()->IsLocked() )
		{
			CalculateBurst();
		}

		Aim();
	}
	else if ( m_bUseIntervals && ( m_fBurstInterval > g_pLTServer->GetTime() ) )
	{
		// We are still waiting to fire, so just aim.
		
		Aim();
	}
	else
	{
		// We're done waiting, fire if we're at a reasonable angle
		// If we are not using node tracking, IsNodeTrackingAtLimit will return true.

		if ( m_bIgnoreFOV || IsBlind() || ( !GetAI()->IsNodeTrackingAtLimit() ) )
		{
			Fire();
		}
		else
		{
			LTVector vTargetPos;
			g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

			LTVector vDir = vTargetPos - GetAI()->GetPosition();
			vDir.y = 0.0f;
			vDir.Normalize();

			if ( vDir.Dot(GetAI()->GetTorsoForward()) < 0.70f )
			{
				Aim();
			}
			else
			{
				Fire();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Aim
//
//	PURPOSE:	Makes us Aim
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Aim()
{
	if( m_eState != eStateAiming )
	{
		CWeapon *pWeapon = GetAI()->GetCurrentWeapon();
		if( pWeapon )
		{
			// Make sure our loop sound is killed when switching to aim

			m_eState == eStateFiring ? pWeapon->PlayAnimation( pWeapon->GetPostFireAni(), true, false, true ) :
									   pWeapon->KillLoopSound();
		}
	}

	super::Aim();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Fire
//
//	PURPOSE:	Makes us fire
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Fire()
{
	m_bPreFire = LTTRUE;

	super::Fire();
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::CAIHumanStrategyFlashlight/~CAIHumanStrategyFlashlight
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyFlashlight::CAIHumanStrategyFlashlight()
{
	m_hFlashlightModel = LTNULL;
}

CAIHumanStrategyFlashlight::~CAIHumanStrategyFlashlight()
{
	FlashlightHide();
	FlashlightOff();
	FlashlightDestroy();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::Flashlight*
//
//	PURPOSE:	Helpers for flashlight operations
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::FlashlightShow()
{
	if ( m_hFlashlightModel )
	{
		g_pCommonLT->SetObjectFlags(m_hFlashlightModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
}

void CAIHumanStrategyFlashlight::FlashlightHide()
{
	if ( m_hFlashlightModel )
	{
		g_pCommonLT->SetObjectFlags(m_hFlashlightModel, OFT_Flags, 0, FLAG_VISIBLE);
	}
}

void CAIHumanStrategyFlashlight::FlashlightOn()
{
	GetAI()->CreateFlashLight();
}

void CAIHumanStrategyFlashlight::FlashlightOff()
{
	if( GetAI()->HasFlashLight() )
	{
		GetAI()->DestroyFlashLight();
	}
}

void CAIHumanStrategyFlashlight::FlashlightCreate()
{
	if ( !m_hFlashlightModel )
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);
		theStruct.m_Pos = LTVector(0,100,0);

		SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Flashlight_hh.ltb");
		SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Flashlight_hh.dtx");

		theStruct.m_Flags = 0;//FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

		HCLASS hClass = g_pLTServer->GetClass("BaseClass");
		LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		m_hFlashlightModel = pModel->m_hObject;

		// Don't eat ticks please...
		::SetNextUpdate(m_hFlashlightModel, UPDATE_NEVER);

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->CreateAttachment(GetAI()->GetObject(), m_hFlashlightModel, "Light", &LTVector(0,0,0), &LTRotation(), &hAttachment) )
		{

		}
	}
}

void CAIHumanStrategyFlashlight::FlashlightDestroy()
{
	if ( m_hFlashlightModel )
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(GetAI()->GetObject(), m_hFlashlightModel, &hAttachment) )
		{
			if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
			{

			}
		}

		g_pLTServer->RemoveObject(m_hFlashlightModel);
		m_hFlashlightModel = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::OnLinkBroken
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if ( pRef == &m_hFlashlightModel )
	{
		FlashlightHide();
		FlashlightOff();

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(GetAI()->GetObject(), hObj, &hAttachment) )
		{
			if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
			{

			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::HandleModelString(ArgList* pArgList)
{
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, "FL_SHOW") )
	{
		if ( !m_hFlashlightModel ) 
		{
			FlashlightCreate();
		}

		FlashlightShow();
	}
	else if ( !_stricmp(szKey, "FL_HIDE") )
	{
		FlashlightHide();
	}
	else if ( !_stricmp(szKey, "FL_ON") )
	{
		if ( !m_hFlashlightModel ) 
		{
			FlashlightCreate();
		}

		FlashlightOn();
	}
	else if ( !_stricmp(szKey, "FL_OFF") )
	{
		FlashlightOff();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategy::Load(pMsg);

	LOAD_HOBJECT(m_hFlashlightModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategy::Save(pMsg);

	SAVE_HOBJECT(m_hFlashlightModel);
}
