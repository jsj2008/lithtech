// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStrategy.h"
#include "CharacterMgr.h"
#include "AIHuman.h"
#include "PlayerObj.h"
#include "RotatingDoor.h"
#include "AISense.h"
#include "AITarget.h"
#include "AIPathMgr.h"
#include "CharacterHitBox.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"

const static LTFLOAT s_fPerturbScale = 4.0f;

using namespace AnimationsHuman;

// Factories

IMPLEMENT_FACTORY(CAIHumanStrategyOneShotAni, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyFollowPath, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyDodge, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyCoverDuck, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyCoverBlind, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyCover1WayCorner, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyCover2WayCorner, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyGrenadeThrow, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyShootBurst, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyShootBurstBlind, 0);
IMPLEMENT_FACTORY(CAIHumanStrategyFlashlight, 0);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategy::Constructor()
{
	m_pAIHuman = LTNULL;
}

void CAIHumanStrategy::Destructor()
{
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
//	ROUTINE:	CAIHumanStrategyFollowPath::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Constructor()
{
	CAIHumanStrategy::Constructor();
			
	m_pStrategyShoot = FACTORY_NEW(CAIHumanStrategyShootBurstBlind);

	m_eState = eStateUnset;
	m_eMedium = eMediumGround;
	m_eUrgency = eUrgencyCourteous;
	m_AIMovement.Constructor();
	m_pPath = FACTORY_NEW(CAIPath);
	m_eDoorState = eDoorStateNone;
	m_bModifiedMovement = LTFALSE;
	m_fStuckOnDoorTimer = 0.0f;
	m_bDoorShootThroughable = LTFALSE;
}

void CAIHumanStrategyFollowPath::Destructor()
{
	FACTORY_DELETE(m_pStrategyShoot);
	FACTORY_DELETE(m_pPath);
	m_AIMovement.Destructor();

	CAIHumanStrategy::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::Init(CAIHuman* pAIHuman)
{
	if ( !CAIHumanStrategy::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_AIMovement.Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_pPath->Init(pAIHuman);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_pPath )
	{
		m_pPath->HandleBrokenLink(hObject);
	}
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

	m_pStrategyShoot->HandleModelString(pArgList);
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

	_ASSERT(m_eState == eStateSet);

	CAIPathWaypoint* pWaypointPrev = LTNULL;
	CAIPathWaypoint* pWaypoint;

	// Basically, if our waypoint update keeps advancing the waypoint, keep updating for the new waypoint.
	// That way we skip over degenerate waypoints that would cause animation pops.

	while ( m_pPath->HasRemainingWaypoints() && (pWaypointPrev != (pWaypoint = m_pPath->GetCurrentWaypoint())) )
	{
		switch ( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			{
				UpdateMoveTo(pWaypoint);
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

	m_pStrategyShoot->ClearFired();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext changes
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::UpdateAnimation()
{
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
				return;
			}

			CAIPathWaypoint* pWaypoint = m_pPath->GetCurrentWaypoint();

			switch ( pWaypoint->GetInstruction() )
			{
				case CAIPathWaypoint::eInstructionMoveTo:
				{
					m_AIMovement.UpdateAnimation();
					if ( m_bModifiedMovement )
					{
						GetAnimationContext()->SetProp(m_aniModifiedMovement);
					}
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

				case CAIPathWaypoint::eInstructionOpenDoors:
				{
					switch ( m_eDoorState )
					{
						case eDoorStateWaitingForAnimationToStart:
						{
							GetAnimationContext()->ClearProps();
							GetAnimationContext()->SetProp(aniStand);
							GetAnimationContext()->SetProp(aniLower);
							GetAnimationContext()->SetProp(aniOpenDoor);
						}
						break;

						case eDoorStateWaitingForModelString:
						case eDoorStateWaitingForAnimationToFinish:
						{
							if ( !GetAnimationContext()->IsLocked() )
							{
								m_eDoorState = eDoorStateWaitingForDoorToOpen;
							}
						}
						break;

						case eDoorStateNone:
						{
						}
						break;

						case eDoorStateWaitingForDoorToOpen:
						{
							if ( m_fStuckOnDoorTimer > 0.0f )
							{
								if ( m_bDoorShootThroughable && (eUrgencyAggressive == m_eUrgency) && GetAI()->GetCurrentWeapon() && GetAI()->HasTarget() )
								{
									m_pStrategyShoot->UpdateAnimation();
								}
								else
								{
									GetAnimationContext()->ClearProps();
									GetAnimationContext()->SetProp(aniStand);
									GetAnimationContext()->SetProp(aniLower);
									GetAnimationContext()->SetProp(aniKnockOnDoor);
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
	if ( m_AIMovement.IsSet() )
	{
		if ( !m_AIMovement.Update() )
		{
			return LTFALSE;
		}

		if ( m_AIMovement.IsDone() )
		{
			m_pPath->IncrementWaypointIndex();
		}
	}
	else if ( m_AIMovement.IsUnset() || m_AIMovement.IsDone() )
	{
		m_AIMovement.Set(pWaypoint->GetArgumentVector1());
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
	LTBOOL bDoorsOpen = LTTRUE;
	LTBOOL bDoorsStuck = LTFALSE;
	LTBOOL bDoorsLocked = LTFALSE;
	LTBOOL bDoorsOpening = LTFALSE;
	LTBOOL bDoorsShootable = LTFALSE;

	LTBOOL bCanOpenDoor = (eDoorStateWaitingForAnimationToFinish == m_eDoorState || eDoorStateWaitingForDoorToOpen == m_eDoorState);

	HOBJECT hDoors[2];
	hDoors[0] = pWaypoint->GetArgumentObject1();
	hDoors[1] = pWaypoint->GetArgumentObject2();

	for ( uint iDoor = 0 ; iDoor < 2 ; iDoor++ )
	{
		HOBJECT hDoor = hDoors[iDoor];
		if ( !hDoor ) continue;

		RotatingDoor* pDoor = (RotatingDoor*)g_pLTServer->HandleToObject(hDoor);

		if ( (pDoor->GetState() == DOORSTATE_CLOSED) || (pDoor->GetState() == DOORSTATE_CLOSING) )
		{
			if ( eDoorStateWaitingForDoorToOpen == m_eDoorState )
			{
				m_eDoorState = eDoorStateNone;
				return LTTRUE;
			}

			if ( bCanOpenDoor )
			{
				GetAI()->OpenDoor(hDoor);
			}
			else
			{
				m_eDoorState = eDoorStateWaitingForAnimationToStart;
			}
		}
		if ( pDoor->GetState() != DOORSTATE_OPEN )
		{
			bDoorsOpen = LTFALSE;
		}

		bDoorsOpening |= (pDoor->GetState() == DOORSTATE_OPENING);
		bDoorsLocked |= pDoor->IsLocked();
		bDoorsStuck |= pDoor->IsStuck();

		if ( bDoorsStuck || bDoorsLocked )
		{
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(hDoor);
			SURFACE* pSurface = g_pSurfaceMgr->GetSurface(UserFlagToSurface(dwUserFlags));
			
			m_bDoorShootThroughable = pSurface->bCanShootThrough;
		}
	}

	if ( bDoorsOpening )
	{
		m_eDoorState = eDoorStateWaitingForDoorToOpen;
	}

	if ( bDoorsOpen )
	{
		m_eDoorState = eDoorStateNone;
		m_pPath->IncrementWaypointIndex();
		m_fStuckOnDoorTimer = 0.0f;
		m_bDoorShootThroughable = LTFALSE;
	}
	else if ( bDoorsStuck )
	{
		m_fStuckOnDoorTimer = 2.0f;
	}
	else if ( bDoorsLocked )
	{

	}
	else
	{
		m_fStuckOnDoorTimer = Max<LTFLOAT>(m_fStuckOnDoorTimer-g_pLTServer->GetFrameTime(), -1.0f);
	}

	if ( m_fStuckOnDoorTimer > 0.0f )
	{
		if ( m_bDoorShootThroughable && (eUrgencyAggressive == m_eUrgency) && GetAI()->GetCurrentWeapon() && GetAI()->HasTarget() )
		{
			m_pStrategyShoot->Reload(LTTRUE);
			m_pStrategyShoot->Update();
		}
		else
		{
			// Courteously wait for the door
		}
	}

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

	HOBJECT hDoor;

	hDoor = pWaypoint->GetArgumentObject1();
	if ( hDoor )
	{
        Door* pDoor = (Door*)g_pLTServer->HandleToObject(hDoor);
		if ( pDoor->GetState() == DOORSTATE_CLOSED )
		{
			GetAI()->OpenDoor(hDoor);
		}
		if ( pDoor->GetState() != DOORSTATE_OPEN )
		{
			bDoorsOpen = LTFALSE;
		}

		bDoorsLocked |= pDoor->IsLocked();
	}

	hDoor = pWaypoint->GetArgumentObject2();
	if ( hDoor )
	{
        Door* pDoor = (Door*)g_pLTServer->HandleToObject(hDoor);
		if ( pDoor->GetState() == DOORSTATE_CLOSED )
		{
			GetAI()->OpenDoor(hDoor);
		}
		if ( pDoor->GetState() != DOORSTATE_OPEN )
		{
			bDoorsOpen = LTFALSE;
		}

		bDoorsLocked |= pDoor->IsLocked();
	}

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
//	ROUTINE:	CAIHumanStrategyFollowPath::UpdateFaceDoor
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::UpdateFaceDoor(CAIPathWaypoint* pWaypoint)
{
	LTBOOL bDoorsOpen = LTTRUE;

	HOBJECT hDoors[2];
	hDoors[0] = pWaypoint->GetArgumentObject1();
	hDoors[1] = pWaypoint->GetArgumentObject2();

	for ( uint iDoor = 0 ; iDoor < 2 ; iDoor++ )
	{
		HOBJECT hDoor = hDoors[iDoor];
		if ( !hDoor ) continue;

		RotatingDoor* pDoor = (RotatingDoor*)g_pLTServer->HandleToObject(hDoor);

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
	m_eDoorState = eDoorStateNone;
	m_fStuckOnDoorTimer = 0.0f;
	m_bDoorShootThroughable = LTFALSE;
	m_AIMovement.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyFollowPath::Set(const LTVector& vDestination)
{
	Reset();

	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIHumanStrategyFollowPath::Set(CAINode* pNodeDestination)
{
	Reset();

	m_eState = g_pAIPathMgr->FindPath(GetAI(), pNodeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIHumanStrategyFollowPath::Set(CAIVolume* pVolumeDestination)
{
	Reset();

	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	m_pStrategyShoot->Load(hRead);
	m_pPath->Load(hRead);
	m_AIMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eDoorState, DoorState);
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eMedium, Medium);
	LOAD_DWORD_CAST(m_eUrgency, Urgency);

	m_aniModifiedMovement.Load(hRead);
	LOAD_BOOL(m_bModifiedMovement);
	LOAD_FLOAT(m_fStuckOnDoorTimer);
	LOAD_BOOL(m_bDoorShootThroughable);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);

	m_pStrategyShoot->Save(hWrite);
	m_pPath->Save(hWrite);
	m_AIMovement.Save(hWrite);

	SAVE_DWORD(m_eDoorState);
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eMedium);
	SAVE_DWORD(m_eUrgency);

	m_aniModifiedMovement.Save(hWrite);
	SAVE_BOOL(m_bModifiedMovement);
	SAVE_FLOAT(m_fStuckOnDoorTimer);
	SAVE_BOOL(m_bDoorShootThroughable);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Constructor()
{
	CAIHumanStrategy::Constructor();

	m_eState = eUnset;
}

void CAIHumanStrategyOneShotAni::Destructor()
{
	CAIHumanStrategy::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Set
//
//	PURPOSE:	Sets the one shot ani
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Set(const CAnimationProp& Prop)
{
	m_eState = eSet;
	m_Prop = Prop;
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

void CAIHumanStrategyOneShotAni::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eUnset:
		case eDone:
		{
		}
		break;

		case eSet:
		case eAnimating:
		{
			GetAnimationContext()->Lock();
			GetAnimationContext()->SetProp(m_Prop);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);

	m_Prop.Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyOneShotAni::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyOneShotAni::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);

	SAVE_DWORD(m_eState);

	m_Prop.Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Constructor()
{
	CAIHumanStrategy::Constructor();

	m_eDodgeStatus = eDodgeStatusOk;
	m_eState = eStateChecking;

	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
	m_dwNode = CAINode::kInvalidNodeID;

	m_fAnimTimePrev = 0.0f;
}

void CAIHumanStrategyDodge::Destructor()
{
	CAIHumanStrategy::Destructor();
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

void CAIHumanStrategyDodge::UpdateAnimation()
{
	if ( m_eState == eStateDodging )
	{
		switch ( m_eDodgeAction )
		{
			case eDodgeActionShuffle:
			{
				switch ( m_eDirection )
				{
					case eDirectionRight:
						GetAnimationContext()->SetProp(aniShuffleRight);
						break;
					case eDirectionLeft:
					default:
						GetAnimationContext()->SetProp(aniShuffleLeft);
						break;
				}

				GetAnimationContext()->Lock();
			}
			break;

			case eDodgeActionRoll:
			{
				GetAnimationContext()->SetProp(aniCrouch);

				switch ( m_eDirection )
				{
					case eDirectionRight:
						GetAnimationContext()->SetProp(aniRollRight);
						break;
					case eDirectionLeft:
					default:
						GetAnimationContext()->SetProp(aniRollLeft);
						break;
				}

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
	uint32 dwNode;
	GetAI()->GetBrain()->GetDodgeStatus(&m_eDodgeStatus, &m_eDirection, &m_eDodgeAction, &dwNode);

	if ( eDodgeStatusVector == m_eDodgeStatus )
	{
		if ( eDodgeActionShuffle == m_eDodgeAction )
		{
			if ( eDirectionRight == m_eDirection )
			{
				m_vDir = GetAI()->GetRightVector();
			}
			else
			{
				m_vDir = -GetAI()->GetRightVector();
			}
		}
		else if ( eDodgeActionRoll == m_eDodgeAction )
		{
			if ( eDirectionRight == m_eDirection )
			{
				m_vDir = GetAI()->GetRightVector();
			}
			else
			{
				m_vDir = -GetAI()->GetRightVector();
			}
		}
		else if ( eDodgeActionCover == m_eDodgeAction )
		{
			m_dwNode = dwNode;
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
			UpdateDodgeShuffle();
			break;
		case eDodgeActionRoll:
			UpdateDodgeRoll();
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

// H A C K

LTFLOAT CAIHumanStrategyDodge::GetMovementData()
{
	MovementData mdDirection = m_eDirection == eDirectionRight ? mdRight : mdLeft;
	MovementData mdAction = m_eDodgeAction == eDodgeActionShuffle ? mdShuffle : mdRoll;

	LTFLOAT fTime = 0.0f;
	LTAnimTracker* pTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(GetAI()->GetObject(), pTracker) )
	{
		uint32 nLength;
		if ( LT_OK == g_pModelLT->GetCurAnimLength(pTracker, nLength) )
		{
			uint32 nTime;
			if ( LT_OK == g_pModelLT->GetCurAnimTime(pTracker, nTime) )
			{
				fTime = (LTFLOAT)nTime/(LTFLOAT)nLength;
			}
		}
	}
		
	LTFLOAT fMovement = 0.0f;

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon )
	{
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (pWeaponData)
		{
			switch ( pWeaponData->nAniType )
			{
				case 0:
					fMovement = ::GetMovementData(mdRifle, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
				case 1:
					fMovement = ::GetMovementData(mdPistol, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
			}
		}
	}

	m_fAnimTimePrev = fTime;

	return fMovement;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodgeShuffle
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodgeShuffle()
{
	if ( GetAnimationContext()->IsLocked() )
	{
		switch ( m_eDirection )
		{
			case eDirectionRight:
				if ( GetAnimationContext()->IsPropSet(aniShuffleRight) )
					GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
				break;
			case eDirectionLeft:
				if ( GetAnimationContext()->IsPropSet(aniShuffleLeft) )
					GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
				break;
		}
	}
	else
	{
		Check();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::UpdateDodgeRoll
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::UpdateDodgeRoll()
{
	if ( GetAnimationContext()->IsLocked() )
	{
		switch ( m_eDirection )
		{
			case eDirectionRight:
				if ( GetAnimationContext()->IsPropSet(aniRollRight) )
					GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
				break;
			case eDirectionLeft:
				if ( GetAnimationContext()->IsPropSet(aniRollLeft) )
					GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
				break;
		}
	}
	else
	{
		Check();
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
	LTVector vScatterPosition;
	CAIVolume* pVolume = GetAI()->GetLastVolume();

	if ( pVolume && g_pAIVolumeMgr->FindDangerScatterPosition(pVolume, GetAI()->GetPosition(), GetAI()->GetBrain()->GetDodgeProjectilePosition(), 40000.0f, &vScatterPosition) )
	{
		char szBuffer[1024];
		sprintf(szBuffer, "FLEE DANGER=%s PT=%f,%f,%f MOVE=RUN NEXT=ATTACKONSIGHT", g_pLTServer->GetStringData(GetAI()->GetBrain()->GetDodgeProjectileName()), VEC_EXPAND(vScatterPosition));
		GetAI()->ChangeState(szBuffer);
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
	CAINode* pNode = g_pAINodeMgr->GetNode(m_dwNode);

	if ( pNode )
	{
		GetAI()->ChangeState("ATTACKFROMCOVER DEST=%s", g_pLTServer->GetStringData(pNode->GetName()));
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

		m_fAnimTimePrev = 0.0f;

		switch ( m_eDodgeAction )
		{
			case eDodgeActionShuffle:
				Shuffle();
				break;
			case eDodgeActionRoll:
				Roll();
				break;
			case eDodgeActionDive:
				return;
				Dive();
				break;
			case eDodgeActionFlee:
				Flee();
				break;
			case eDodgeActionCover:
				Cover();
				break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Shuffle
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Shuffle()
{
	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
	m_eState = eStateDodging;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Roll
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Roll()
{
	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
	m_eState = eStateDodging;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Dive
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Dive()
{
	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
	m_eState = eStateDodging;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Dodge
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Flee()
{
	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
	m_eState = eStateDodging;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Dodge
//
//	PURPOSE:	Dodge by doing the current action
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Cover()
{
	g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDodge);
	m_eState = eStateDodging;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	LOAD_DWORD(m_dwNode);
	LOAD_FLOAT(m_fCheckTimeVector);
	LOAD_FLOAT(m_fCheckTimeProjectile);
	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eDodgeStatus, DodgeStatus);
	LOAD_DWORD_CAST(m_eDodgeAction, DodgeAction);
	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
	LOAD_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyDodge::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyDodge::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);
	
	SAVE_DWORD(m_dwNode);
	SAVE_FLOAT(m_fCheckTimeVector);
	SAVE_FLOAT(m_fCheckTimeProjectile);
	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eDodgeStatus);
	SAVE_DWORD(m_eDodgeAction);
	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
	SAVE_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Constructor()
{
	CAIHumanStrategy::Constructor();

	m_fCoverTime = 1.0f;
	m_fUncoverTime = 1.0f;

	Clear();
}

void CAIHumanStrategyCover::Destructor()
{
	CAIHumanStrategy::Destructor();
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

			if ( m_fCoverTimer+fTimeDelta > m_fCoverTime )
			{
				m_eState = eWillUncover;
			}

			if ( m_bWantUncover )
			{
				if ( m_fCoverTimer + fTimeDelta > m_fCoverDelay )
				{
					m_eState = eWillUncover;
				}
			}
		}
		break;

		case eUncovered:
		{
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

void CAIHumanStrategyCover::UpdateAnimation()
{
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

void CAIHumanStrategyCover::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);

	LOAD_BOOL(m_bWantCover);
	LOAD_FLOAT(m_fCoverTimer);
	LOAD_FLOAT(m_fCoverTime);
	LOAD_FLOAT(m_fCoverDelay);

	LOAD_BOOL(m_bWantUncover);
	LOAD_FLOAT(m_fUncoverTimer);
	LOAD_FLOAT(m_fUncoverTime);
	LOAD_FLOAT(m_fUncoverDelay);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);

	SAVE_DWORD(m_eState);

	SAVE_BOOL(m_bWantCover);
	SAVE_FLOAT(m_fCoverTimer);
	SAVE_FLOAT(m_fCoverTime);
	SAVE_FLOAT(m_fCoverDelay);

	SAVE_BOOL(m_bWantUncover);
	SAVE_FLOAT(m_fUncoverTimer);
	SAVE_FLOAT(m_fUncoverTime);
	SAVE_FLOAT(m_fUncoverDelay);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverDuck::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCoverDuck::Constructor()
{
	CAIHumanStrategyCover::Constructor();
}

void CAIHumanStrategyCoverDuck::Destructor()
{
	CAIHumanStrategyCover::Destructor();
}

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
			if ( GetAnimationContext()->IsPropSet(aniPopup) )
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
			if ( !GetAnimationContext()->IsPropSet(aniPopup) )
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

void CAIHumanStrategyCoverDuck::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eWillCover:
		case eCovering:
		case eCovered:
		{
			GetAnimationContext()->SetProp(aniCrouch);
		}
		break;

		case eWillUncover:
		case eUncovering:
		case eUncovered:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniPopup);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover1WayCorner::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Constructor()
{
	CAIHumanStrategyCover::Constructor();

	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
	m_fAnimTimePrev = 0.0f;
}

void CAIHumanStrategyCover1WayCorner::Destructor()
{
	CAIHumanStrategyCover::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover1WayCorner::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategyCover::Save(hWrite);

	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
	SAVE_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover1WayCorner::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategyCover::Load(hRead);

	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
	LOAD_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //

LTFLOAT CAIHumanStrategyCover1WayCorner::GetMovementData()
{
	MovementData mdDirection = m_eDirection == eDirectionRight ? mdRight : mdLeft;
	MovementData mdAction = mdCorner;

	LTFLOAT fTime = 0.0f;
	LTAnimTracker* pTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(GetAI()->GetObject(), pTracker) )
	{
		uint32 nLength;
		if ( LT_OK == g_pModelLT->GetCurAnimLength(pTracker, nLength) )
		{
			uint32 nTime;
			if ( LT_OK == g_pModelLT->GetCurAnimTime(pTracker, nTime) )
			{
				fTime = (LTFLOAT)nTime/(LTFLOAT)nLength;
			}
		}
	}
		
	LTFLOAT fMovement = 0.0f;

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon )
	{
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (pWeaponData)
		{
			switch ( pWeaponData->nAniType )
			{
				case 0:
					fMovement = ::GetMovementData(mdRifle, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
				case 1:
					fMovement = ::GetMovementData(mdPistol, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
			}
		}
	}

	m_fAnimTimePrev = fTime;

	return fMovement;
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

			if ( m_vDir.Dot(GetAI()->GetRightVector()) > 0.0f )
			{
				m_eDirection = eDirectionRight;
			}
			else
			{
				m_eDirection = eDirectionLeft;
			}

			m_eState = eUncovering;
			m_fAnimTimePrev = 0.0f;
		}
		break;

		case eUncovering:
		{
			if ( GetAnimationContext()->IsLocked() )
			{
				switch ( m_eDirection )
				{
					case eDirectionRight:
						if ( GetAnimationContext()->IsPropSet(aniCornerRight) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
					case eDirectionLeft:
						if ( GetAnimationContext()->IsPropSet(aniCornerLeft) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
				}
			}
			else
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
			m_fAnimTimePrev = 0.0f;
		}
		break;

		case eCovering:
		{
			if ( GetAnimationContext()->IsLocked() )
			{
				switch ( m_eDirection )
				{
					case eDirectionRight:
						if ( GetAnimationContext()->IsPropSet(aniCornerRight) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
					case eDirectionLeft:
						if ( GetAnimationContext()->IsPropSet(aniCornerLeft) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
				}
			}
			else
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

void CAIHumanStrategyCover1WayCorner::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eCovered:
		{
 			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniCovered);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		case eUncovered:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniUncovered);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
		}
		break;

		case eWillCover:
		case eCovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniCovering);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
			GetAnimationContext()->Lock();
		}
		break;

		case eWillUncover:
		case eUncovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniUncovering);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
			GetAnimationContext()->Lock();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover2WayCorner::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Constructor()
{
	CAIHumanStrategyCover::Constructor();

	m_eDirection = eDirectionRight;
	m_vDir = LTVector(1,0,0);
	m_fAnimTimePrev = 0.0f;
}

void CAIHumanStrategyCover2WayCorner::Destructor()
{
	CAIHumanStrategyCover::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCover2WayCorner::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategyCover::Save(hWrite);

	SAVE_DIRECTION(m_eDirection);
	SAVE_VECTOR(m_vDir);
	SAVE_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //

void CAIHumanStrategyCover2WayCorner::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategyCover::Load(hRead);

	LOAD_DIRECTION(m_eDirection);
	LOAD_VECTOR(m_vDir);
	LOAD_FLOAT(m_fAnimTimePrev);
}

// ----------------------------------------------------------------------- //

LTFLOAT CAIHumanStrategyCover2WayCorner::GetMovementData()
{
	MovementData mdDirection = m_eDirection == eDirectionRight ? mdRight : mdLeft;
	MovementData mdAction = mdCorner;

	LTFLOAT fTime = 0.0f;
	LTAnimTracker* pTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(GetAI()->GetObject(), pTracker) )
	{
		uint32 nLength;
		if ( LT_OK == g_pModelLT->GetCurAnimLength(pTracker, nLength) )
		{
			uint32 nTime;
			if ( LT_OK == g_pModelLT->GetCurAnimTime(pTracker, nTime) )
			{
				fTime = (LTFLOAT)nTime/(LTFLOAT)nLength;
			}
		}
	}
		
	LTFLOAT fMovement = 0.0f;

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();
	if ( pWeapon )
	{
		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (pWeaponData)
		{
			switch ( pWeaponData->nAniType )
			{
				case 0:
					fMovement = ::GetMovementData(mdRifle, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
				case 1:
					fMovement = ::GetMovementData(mdPistol, mdAction, mdDirection, m_fAnimTimePrev, fTime);
					break;
			}
		}
	}

	m_fAnimTimePrev = fTime;

	return fMovement;
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
			vDir.Norm();

			if ( vDir.Dot(m_pCoverNode->GetRight()) > 0.0f )
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
			m_fAnimTimePrev = 0.0f;
		}
		break;

		case eUncovering:
		{
			if ( GetAnimationContext()->IsLocked() )
			{
				switch ( m_eDirection )
				{
					case eDirectionRight:
						if ( GetAnimationContext()->IsPropSet(aniCornerRight) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
					case eDirectionLeft:
						if ( GetAnimationContext()->IsPropSet(aniCornerLeft) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
				}
			}
			else
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
			m_vDir.Norm();

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
			m_fAnimTimePrev = 0.0f;
		}
		break;

		case eCovering:
		{
			if ( GetAnimationContext()->IsLocked() )
			{
				switch ( m_eDirection )
				{
					case eDirectionRight:
						if ( GetAnimationContext()->IsPropSet(aniCornerRight) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
					case eDirectionLeft:
						if ( GetAnimationContext()->IsPropSet(aniCornerLeft) )
							GetAI()->Move(GetAI()->GetPosition() + m_vDir*GetMovementData());
						break;
				}
			}
			else
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

void CAIHumanStrategyCover2WayCorner::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eCovered:
		{
 			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniCovered);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		case eUncovered:
		{
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniUncovered);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
		}
		break;

		case eWillCover:
		case eCovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniCovering);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
			GetAnimationContext()->Lock();
		}
		break;

		case eWillUncover:
		case eUncovering:
		{
			GetAnimationContext()->ClearProps();
			GetAnimationContext()->SetProp(aniStand);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniUncovering);
			GetAnimationContext()->SetProp(m_eDirection == eDirectionRight ? aniCornerRight : aniCornerLeft);
			GetAnimationContext()->Lock();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyCoverBlind::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyCoverBlind::Constructor()
{
	CAIHumanStrategyCover::Constructor();
}

void CAIHumanStrategyCoverBlind::Destructor()
{
	CAIHumanStrategyCover::Destructor();
}

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
			if ( GetAnimationContext()->IsPropSet(aniBlind) )
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
			if ( !GetAnimationContext()->IsPropSet(aniBlind) )
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

void CAIHumanStrategyCoverBlind::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eWillCover:
		case eCovering:
		case eCovered:
		{
			GetAnimationContext()->SetProp(aniCrouch);
		}
		break;

		case eWillUncover:
		case eUncovering:
		case eUncovered:
		{
			GetAnimationContext()->SetProp(aniCrouch);
			GetAnimationContext()->SetProp(aniUp);
			GetAnimationContext()->SetProp(aniBlind);
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenade::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenade::Constructor()
{
	CAIHumanStrategy::Constructor();
}

void CAIHumanStrategyGrenade::Destructor()
{
	CAIHumanStrategy::Destructor();
}

CAIHumanStrategyGrenade* CAIHumanStrategyGrenade::Create(AIHumanStrategyType eType)
{
	switch ( eType )
	{
		case eStrategyGrenadeThrow:
			return FACTORY_NEW(CAIHumanStrategyGrenadeThrow);
		default:
			return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyGrenadeThrow::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyGrenadeThrow::Constructor()
{
	CAIHumanStrategyGrenade::Constructor();

	m_eState = eStateNone;
	m_fHangtime = 0.5f;
}

void CAIHumanStrategyGrenadeThrow::Destructor()
{
	CAIHumanStrategyGrenade::Destructor();
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

		CWeapon* pWeapon = GetAI()->GetWeapon(1);
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

		WFireInfo fireInfo;
		fireInfo.hFiredFrom = GetAI()->GetObject();
		fireInfo.vPath		= vDir;
		fireInfo.bOverrideVelocity = LTTRUE;
		fireInfo.fOverrideVelocity = fVelocity;
		fireInfo.vFirePos	= vFirePos;
		fireInfo.vFlashPos	= vFirePos;
		fireInfo.hTestObj	= hTarget;
		fireInfo.fPerturbR	= 1.0f*(1.0f - GetAI()->GetAccuracy());
		fireInfo.fPerturbU	= 1.0f*(1.0f - GetAI()->GetAccuracy());

		pWeapon->ReloadClip(LTFALSE);
		pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);
		pWeapon->UpdateWeapon(fireInfo, LTTRUE);

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

void CAIHumanStrategyGrenadeThrow::UpdateAnimation()
{
	CAIHumanStrategyGrenade::UpdateAnimation();

	GetAnimationContext()->SetProp(aniStand);
	GetAnimationContext()->SetProp(aniThrow);
	GetAnimationContext()->Lock();
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

void CAIHumanStrategyGrenadeThrow::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategyGrenade::Load(hRead);

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

void CAIHumanStrategyGrenadeThrow::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategyGrenade::Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_FLOAT(m_fHangtime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Constructor()
{
	CAIHumanStrategy::Constructor();

	m_eState = eStateAiming;
	m_bFired = LTFALSE;
	m_bNeedsReload = LTFALSE;
	m_bOutOfAmmo = LTFALSE;
	m_bFirstUpdate = LTTRUE;
	m_bIgnoreFOV = LTFALSE;
}

void CAIHumanStrategyShoot::Destructor()
{
	CAIHumanStrategy::Destructor();
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
		pWeapon->ReloadClip(LTFALSE);
		pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);

		if ( !bInstant )
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
		HandleFired();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::HandleFired
//
//	PURPOSE:	Handle the fire key
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::HandleFired()
{
	m_bFired = LTTRUE;
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

	// If we don't/no longer have a target, this strategy cannot work

	if ( !hTarget )
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
				vTargetPos = GetAI()->GetTarget()->GetShootPosition();

				// Get the target's dims

				LTVector vTargetDims;
                g_pLTServer->GetObjectDims(hTarget, &vTargetDims);

				// Shoot halfway between torso and head

				vTargetPos.y += vTargetDims.y/2.0f;
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

void CAIHumanStrategyShoot::UpdateAnimation()
{
	switch ( m_eState )
	{
		case eStateNone:
		{

		}
		break;

		case eStateAiming:
		{
			m_eState = eStateNone;

			GetAnimationContext()->SetProp(aniAim);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		case eStateFiring:
		{
			m_eState = eStateNone;

			GetAnimationContext()->SetProp(aniFire);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		case eStateReloading:
		{
			GetAnimationContext()->Lock();
			GetAnimationContext()->SetProp(aniReload);
			GetAnimationContext()->SetProp(aniUp);
		}
		break;

		default:
		{
			_ASSERT(LTFALSE);
		}
		break;
	}
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
	// Get the weapon state

	if ( 0 == pWeapon->GetAmmoInClip() )
	{
		if ( 0 == pWeapon->GetParent()->GetAmmoCount(pWeapon->GetAmmoId()) )
		{
			pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);
			m_bNeedsReload = LTTRUE;
		}
		else
		{
			m_bNeedsReload = LTTRUE;
		}
	}
	else
	{
		m_bNeedsReload = LTFALSE;
	}
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
	m_eState = eStateFiring;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_BOOL(m_bFired);
	LOAD_BOOL(m_bNeedsReload);
	LOAD_BOOL(m_bOutOfAmmo);
	LOAD_BOOL(m_bFirstUpdate);
	LOAD_BOOL(m_bIgnoreFOV);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShoot::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_BOOL(m_bFired);
	SAVE_BOOL(m_bNeedsReload);
	SAVE_BOOL(m_bOutOfAmmo);
	SAVE_BOOL(m_bFirstUpdate);
	SAVE_BOOL(m_bIgnoreFOV);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Constructor
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Constructor()
{
	CAIHumanStrategyShoot::Constructor();

	m_bIgnoreFOV = LTFALSE;
	m_fBurstInterval = 0;
	m_nBurstShots = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Destructor
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Destructor()
{
	CAIHumanStrategyShoot::Destructor();
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

	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();

	if ( pWeapon && pWeapon->GetWeaponData() )
	{
		m_fBurstInterval = 0;
		m_nBurstShots = GetRandom(pWeapon->GetWeaponData()->nAIMinBurstShots, pWeapon->GetWeaponData()->nAIMaxBurstShots);
	}
	else
	{
		m_fBurstInterval = (LTFLOAT)INT_MAX;
		m_nBurstShots = 0;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::HandleFired
//
//	PURPOSE:	Handle the fire key
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::HandleFired()
{
	CAIHumanStrategyShoot::HandleFired();

	m_nBurstShots--;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategyShoot::Load(hRead);

	LOAD_FLOAT(m_fBurstInterval);
	LOAD_INT(m_nBurstShots);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurst::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurst::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategyShoot::Save(hWrite);

	SAVE_FLOAT(m_fBurstInterval);
	SAVE_INT(m_nBurstShots);
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
	// Get our fire position

	LTVector vFirePos = GetAI()->GetWeaponPosition(pWeapon);

	// Get our firing vector

	LTVector vDir = !IsBlind() ? vTargetPos - vFirePos : GetAI()->GetForwardVector();

	// Now fire the weapon

	WFireInfo fireInfo;
	fireInfo.hFiredFrom = GetAI()->GetObject();
	fireInfo.vPath		= vDir;
	fireInfo.vFirePos	= vFirePos;
	fireInfo.vFlashPos	= vFirePos;
	fireInfo.hTestObj	= hTarget;

	if ( !IsBlind() )
	{
		fireInfo.fPerturbR	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
		fireInfo.fPerturbU	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
	}
	else
	{
		fireInfo.fPerturbR	= 10.0f*LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
		fireInfo.fPerturbU	= 10.0f*LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
	}

	pWeapon->UpdateWeapon(fireInfo, LTTRUE);

	if ( m_nBurstShots <= 0 )
	{
		// We just finished our burst. Start waiting.

		CalculateBurst();

		// And just aim.

		Aim();
	}
	else
	{
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
	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();

	if ( pWeapon && pWeapon->GetWeaponData() )
	{
		m_fBurstInterval = GetRandom(pWeapon->GetWeaponData()->fAIMinBurstInterval, pWeapon->GetWeaponData()->fAIMaxBurstInterval);
		m_nBurstShots = GetRandom(pWeapon->GetWeaponData()->nAIMinBurstShots, pWeapon->GetWeaponData()->nAIMaxBurstShots);
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
	if ( m_nBurstShots <= 0 )
	{
		CalculateBurst();

		Aim();
	}
	else if ( m_fBurstInterval > 0.0f )
	{
		// We are still waiting to fire.

        m_fBurstInterval -= g_pLTServer->GetFrameTime();

		// So just aim.

		Aim();
	}
	else
	{
		// We're done waiting, fire if we're at a reasonable angle

		if ( m_bIgnoreFOV || IsBlind() )
		{
			Fire();
		}
		else
		{
			LTVector vTargetPos;
			g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

			LTVector vDir = vTargetPos - GetAI()->GetPosition();
			vDir.y = 0.0f;
			vDir.Norm();

			if ( vDir.Dot(GetAI()->GetForwardVector()) < 0.70f )
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
//	ROUTINE:	CAIHumanStrategyShootBurstBlind::Constructor
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurstBlind::Constructor()
{
	CAIHumanStrategyShootBurst::Constructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShootBurstBlind::Destructor
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyShootBurstBlind::Destructor()
{
	CAIHumanStrategyShootBurst::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::Constructor()
{
	CAIHumanStrategy::Constructor();

	m_hFlashlightModel = LTNULL;
}

void CAIHumanStrategyFlashlight::Destructor()
{
	FlashlightHide();
	FlashlightOff();
	FlashlightDestroy();

	CAIHumanStrategy::Destructor();
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
		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hFlashlightModel);
		g_pLTServer->SetObjectFlags(m_hFlashlightModel, dwFlags | FLAG_VISIBLE);
	}
}

void CAIHumanStrategyFlashlight::FlashlightHide()
{
	if ( m_hFlashlightModel )
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hFlashlightModel);
		g_pLTServer->SetObjectFlags(m_hFlashlightModel, dwFlags & ~FLAG_VISIBLE);
	}
}

void CAIHumanStrategyFlashlight::FlashlightOn()
{
	uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(GetAI()->GetObject());
	g_pLTServer->SetObjectUserFlags(GetAI()->GetObject(), dwUsrFlags | USRFLG_AI_FLASHLIGHT);
}

void CAIHumanStrategyFlashlight::FlashlightOff()
{
	uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(GetAI()->GetObject());
	g_pLTServer->SetObjectUserFlags(GetAI()->GetObject(), dwUsrFlags & ~USRFLG_AI_FLASHLIGHT);
}

void CAIHumanStrategyFlashlight::FlashlightCreate()
{
	if ( !m_hFlashlightModel )
	{
		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);
		theStruct.m_Pos = LTVector(0,100,0);

		SAFE_STRCPY(theStruct.m_Filename, "Guns\\Models_HH\\Flashlight_hh.abc");
		SAFE_STRCPY(theStruct.m_SkinName, "Guns\\Skins_HH\\Flashlight_hh.dtx");

		theStruct.m_Flags = 0;//FLAG_VISIBLE;
		theStruct.m_ObjectType = OT_MODEL;

		HCLASS hClass = g_pLTServer->GetClass("BaseClass");
		LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
		m_hFlashlightModel = pModel->m_hObject;

		GetAI()->Link(m_hFlashlightModel);

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->CreateAttachment(GetAI()->GetObject(), m_hFlashlightModel, "LeftHand", &LTVector(0,0,0), &LTRotation(0,0,0,1), &hAttachment) )
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

		GetAI()->Unlink(m_hFlashlightModel);

		g_pLTServer->RemoveObject(m_hFlashlightModel);
		m_hFlashlightModel = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::HandleBrokenLink(HOBJECT hObject)
{
	if ( hObject == m_hFlashlightModel )
	{
		FlashlightHide();
		FlashlightOff();

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(GetAI()->GetObject(), m_hFlashlightModel, &hAttachment) )
		{
			if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
			{

			}
		}

//		FlashlightDestroy();

		m_hFlashlightModel = LTNULL;
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

void CAIHumanStrategyFlashlight::Load(HMESSAGEREAD hRead)
{
	CAIHumanStrategy::Load(hRead);

	LOAD_HOBJECT(m_hFlashlightModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyFlashlight::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyFlashlight::Save(HMESSAGEREAD hWrite)
{
	CAIHumanStrategy::Save(hWrite);

	SAVE_HOBJECT(m_hFlashlightModel);
}
