// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIAnimalStrategy.h"
#include "CharacterMgr.h"
#include "AIAnimal.h"
#include "PlayerObj.h"
#include "AIDog.h"
#include "AIPoodle.h"
#include "AIShark.h"
#include "AISense.h"
#include "AITarget.h"
#include "AIPath.h"
#include "Door.h"
#include "AIPathMgr.h"

// Factories

IMPLEMENT_FACTORY(CAIDogStrategyFollowPath, 0);
IMPLEMENT_FACTORY(CAIDogStrategyOneShotAni, 0);

IMPLEMENT_FACTORY(CAIPoodleStrategyFollowPath, 0);

IMPLEMENT_FACTORY(CAISharkStrategyFollowPath, 0);
IMPLEMENT_FACTORY(CAISharkStrategyOneShotAni, 0);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimalStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIAnimalStrategy::Constructor()
{
    m_pAIAnimal = LTNULL;
}

void CAIAnimalStrategy::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIAnimalStrategy::GetAnimator
//
//	PURPOSE:	Gets our animator
//
// ----------------------------------------------------------------------- //

CAnimatorAIAnimal* CAIAnimalStrategy::GetAnimator()
{
	return GetAI()->GetAnimator();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStrategy::Constructor()
{
	super::Constructor();

    m_pAIDog = LTNULL;
}

void CAIDogStrategy::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategy::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategy::Init(AI_Dog* pAIDog)
{
	super::Init(pAIDog);

	m_pAIDog = pAIDog;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyFollowPath::Constructor()
{
	super::Constructor();

	m_eState = eStateUnset;
	m_AIMovement.Constructor();
	m_pPath = FACTORY_NEW(CAIPath);
	m_eMovement = CAnimatorAIAnimal::eWalking;
}

void CAIDogStrategyFollowPath::Destructor()
{
	FACTORY_DELETE(m_pPath);
	m_AIMovement.Destructor();

	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyFollowPath::Init(AI_Dog* pAIDog)
{
	if ( !super::Init(pAIDog) )
	{
        return LTFALSE;
	}

	if ( !m_AIMovement.Init(pAIDog) )
	{
        return LTFALSE;
	}

	m_pPath->Init(pAIDog);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyFollowPath::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_pPath )
	{
		m_pPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyFollowPath::Update()
{
	if ( !super::Update() )
	{
        return LTFALSE;
	}

	_ASSERT(m_eState == eStateSet);

	if ( m_pPath->HasRemainingWaypoints() )
	{
		CAIPathWaypoint* pWaypoint = m_pPath->GetCurrentWaypoint();

		switch ( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			{
				UpdateMoveTo(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionOpenDoors:
			{
				UpdateOpenDoors(pWaypoint);
			}
			break;

			default:
			{
                g_pLTServer->CPrint("CAIDogStrategyFollowPath::Update - unrecognized waypoint instruction");
				_ASSERT(!"CAIDogStrategyFollowPath::Update - unrecognized waypoint instruction");
                return LTFALSE;
			}
			break;
		}
	}

	if ( !m_pPath->HasRemainingWaypoints() )
	{
		m_eState = eStateDone;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyFollowPath::UpdateMoveTo(CAIPathWaypoint* pWaypoint)
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
	else // Stuck
	{

	}

	GetAnimator()->SetMain(m_eMovement);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::UpdateOpenDoors
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyFollowPath::UpdateOpenDoors(CAIPathWaypoint* pWaypoint)
{
    LTBOOL bDoorsOpen = LTTRUE;
	HOBJECT hDoor1, hDoor2;

	hDoor1 = pWaypoint->GetArgumentObject1();
	hDoor2 = pWaypoint->GetArgumentObject2();

	if ( hDoor1 || hDoor2  )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyFollowPath::Set(const LTVector& vDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIDogStrategyFollowPath::Set(CAINode* pNodeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pNodeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIDogStrategyFollowPath::Set(CAIVolume* pVolumeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::SetMovement
//
//	PURPOSE:	Sets our "default" movement type (sometimes we may be forced
//				to do a particular style of movement, like a crawl, jump, or
//				climb)
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyFollowPath::SetMovement(CAnimatorAIAnimal::Main eMovement)
{
	switch ( eMovement )
	{
		case CAnimatorAIAnimal::eWalking:		GetAI()->Walk();	break;
		case CAnimatorAIAnimal::eRunning:		GetAI()->Run();		break;
        default:                                _ASSERT(LTFALSE);    break;
	}

	m_eMovement = eMovement;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pPath->Load(hRead);
	m_AIMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eMovement, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pPath->Save(hWrite);
	m_AIMovement.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eMovement);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyOneShotAni::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyOneShotAni::Constructor()
{
	super::Constructor();

    m_bAnimating = LTFALSE;
	m_eMain = CAnimatorAIAnimal::eIdle;
}

void CAIDogStrategyOneShotAni::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyOneShotAni::Set
//
//	PURPOSE:	Sets the one shot ani
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyOneShotAni::Set(CAnimatorAIAnimal::Main eMain)
{
	GetAnimator()->SetMain(eMain);

	m_eMain = eMain;
    m_bAnimating = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyOneShotAni::Update
//
//	PURPOSE:	Updates the strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIDogStrategyOneShotAni::Update()
{
	super::Update();

    if ( !m_bAnimating )
	{
        return LTFALSE;
	}

	if ( GetAnimator()->IsAnimatingMainDone(m_eMain) )
	{
        m_bAnimating = LTFALSE;
	}

	GetAnimator()->SetMain(m_eMain);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyOneShotAni::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyOneShotAni::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_BOOL(m_bAnimating);
	LOAD_DWORD_CAST(m_eMain, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIDogStrategyOneShotAni::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIDogStrategyOneShotAni::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_BOOL(m_bAnimating);
	SAVE_DWORD(m_eMain);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategy::Constructor()
{
	super::Constructor();

    m_pAIPoodle = LTNULL;
}

void CAIPoodleStrategy::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategy::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategy::Init(AI_Poodle* pAIPoodle)
{
	super::Init(pAIPoodle);

	m_pAIPoodle = pAIPoodle;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategyFollowPath::Constructor()
{
	super::Constructor();

	m_eState = eStateUnset;
	m_AIMovement.Constructor();
	m_pPath = FACTORY_NEW(CAIPath);
	m_eMovement = CAnimatorAIAnimal::eWalking;
}

void CAIPoodleStrategyFollowPath::Destructor()
{
	FACTORY_DELETE(m_pPath);
	m_AIMovement.Destructor();

	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategyFollowPath::Init(AI_Poodle* pAIPoodle)
{
	if ( !super::Init(pAIPoodle) )
	{
        return LTFALSE;
	}

	if ( !m_AIMovement.Init(pAIPoodle) )
	{
        return LTFALSE;
	}

	m_pPath->Init(pAIPoodle);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategyFollowPath::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_pPath )
	{
		m_pPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategyFollowPath::Update()
{
	if ( !super::Update() )
	{
        return LTFALSE;
	}

	_ASSERT(m_eState == eStateSet);

	if ( m_pPath->HasRemainingWaypoints() )
	{
		CAIPathWaypoint* pWaypoint = m_pPath->GetCurrentWaypoint();

		switch ( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			{
				UpdateMoveTo(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionOpenDoors:
			{
				UpdateOpenDoors(pWaypoint);
			}
			break;

			default:
			{
                g_pLTServer->CPrint("CAIPoodleStrategyFollowPath::Update - unrecognized waypoint instruction");
				_ASSERT(!"CAIPoodleStrategyFollowPath::Update - unrecognized waypoint instruction");
                return LTFALSE;
			}
			break;
		}
	}

	if ( !m_pPath->HasRemainingWaypoints() )
	{
		m_eState = eStateDone;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategyFollowPath::UpdateMoveTo(CAIPathWaypoint* pWaypoint)
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
		if  ( !GetAI()->FacePos(pWaypoint->GetArgumentVector1()) )
		{
			// If we're not yet facing the point, don't move, and start to face it.
			// We need to clear in order to disable the steerings.

			GetAI()->Stop();
		}
		else
		{
			// We're facing the point, so we can start moving.

			SetMovement(m_eMovement);
			m_AIMovement.Set(pWaypoint->GetArgumentVector1());
		}
	}
	else // Stuck
	{

	}

	GetAnimator()->SetMain(m_eMovement);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::UpdateOpenDoors
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategyFollowPath::UpdateOpenDoors(CAIPathWaypoint* pWaypoint)
{
    LTBOOL bDoorsOpen = LTTRUE;
	HOBJECT hDoor1, hDoor2;

	hDoor1 = pWaypoint->GetArgumentObject1();
	hDoor2 = pWaypoint->GetArgumentObject2();

	if ( hDoor1 || hDoor2  )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIPoodleStrategyFollowPath::Set(const LTVector& vDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIPoodleStrategyFollowPath::Set(CAINode* pNodeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pNodeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIPoodleStrategyFollowPath::Set(CAIVolume* pVolumeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::SetMovement
//
//	PURPOSE:	Sets our "default" movement type (sometimes we may be forced
//				to do a particular style of movement, like a crawl, jump, or
//				climb)
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategyFollowPath::SetMovement(CAnimatorAIAnimal::Main eMovement)
{
	switch ( eMovement )
	{
		case CAnimatorAIAnimal::eWalking:		GetAI()->Walk();	break;
		case CAnimatorAIAnimal::eRunning:		GetAI()->Run();		break;
        default:                                _ASSERT(LTFALSE);    break;
	}

	m_eMovement = eMovement;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pPath->Load(hRead);
	m_AIMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eMovement, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPoodleStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIPoodleStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pPath->Save(hWrite);
	m_AIMovement.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eMovement);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStrategy::Constructor()
{
	super::Constructor();

    m_pAIShark = LTNULL;
}

void CAISharkStrategy::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategy::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategy::Init(AI_Shark* pAIShark)
{
	super::Init(pAIShark);

	m_pAIShark = pAIShark;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyFollowPath::Constructor()
{
	super::Constructor();

	m_eState = eStateUnset;
	m_AIMovement.Constructor();
	m_pPath = FACTORY_NEW(CAIPath);
	m_eMovement = CAnimatorAIAnimal::eWalking;
}

void CAISharkStrategyFollowPath::Destructor()
{
	FACTORY_DELETE(m_pPath);
	m_AIMovement.Destructor();

	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyFollowPath::Init(AI_Shark* pAIShark)
{
	if ( !super::Init(pAIShark) )
	{
        return LTFALSE;
	}

	if ( !m_AIMovement.Init(pAIShark) )
	{
        return LTFALSE;
	}

	m_pPath->Init(pAIShark);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyFollowPath::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_pPath )
	{
		m_pPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyFollowPath::Update()
{
	if ( !super::Update() )
	{
        return LTFALSE;
	}

	_ASSERT(m_eState == eStateSet);

	if ( m_pPath->HasRemainingWaypoints() )
	{
		CAIPathWaypoint* pWaypoint = m_pPath->GetCurrentWaypoint();

		switch ( pWaypoint->GetInstruction() )
		{
			case CAIPathWaypoint::eInstructionMoveTo:
			{
				UpdateMoveTo(pWaypoint);
			}
			break;

			case CAIPathWaypoint::eInstructionOpenDoors:
			{
				UpdateOpenDoors(pWaypoint);
			}
			break;

			default:
			{
                g_pLTServer->CPrint("CAISharkStrategyFollowPath::Update - unrecognized waypoint instruction");
				_ASSERT(!"CAISharkStrategyFollowPath::Update - unrecognized waypoint instruction");
                return LTFALSE;
			}
			break;
		}
	}

	if ( !m_pPath->HasRemainingWaypoints() )
	{
		m_eState = eStateDone;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyFollowPath::UpdateMoveTo(CAIPathWaypoint* pWaypoint)
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
	else // Stuck
	{

	}

	GetAnimator()->SetMain(m_eMovement);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::UpdateOpenDoors
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyFollowPath::UpdateOpenDoors(CAIPathWaypoint* pWaypoint)
{
    LTBOOL bDoorsOpen = LTTRUE;
	HOBJECT hDoor1, hDoor2;

	hDoor1 = pWaypoint->GetArgumentObject1();
	hDoor2 = pWaypoint->GetArgumentObject2();

	if ( hDoor1 || hDoor2  )
	{
		GetAnimator()->SetMain(CAnimatorAIAnimal::eIdle);
	}

	m_pPath->IncrementWaypointIndex();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyFollowPath::Set(const LTVector& vDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAISharkStrategyFollowPath::Set(CAINode* pNodeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pNodeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAISharkStrategyFollowPath::Set(CAIVolume* pVolumeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

void CAISharkStrategyFollowPath::Stop()
{
	m_AIMovement.Clear();
	GetAI()->Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::SetMovement
//
//	PURPOSE:	Sets our "default" movement type (sometimes we may be forced
//				to do a particular style of movement, like a crawl, jump, or
//				climb)
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyFollowPath::SetMovement(CAnimatorAIAnimal::Main eMovement)
{
	switch ( eMovement )
	{
		case CAnimatorAIAnimal::eSwimming:		GetAI()->Swim();	break;
        default:                                _ASSERT(LTFALSE);    break;
	}

	m_eMovement = eMovement;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	m_pPath->Load(hRead);
	m_AIMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_DWORD_CAST(m_eMovement, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	m_pPath->Save(hWrite);
	m_AIMovement.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_DWORD(m_eMovement);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyOneShotAni::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyOneShotAni::Constructor()
{
	super::Constructor();

    m_bAnimating = LTFALSE;
	m_eMain = CAnimatorAIAnimal::eIdle;
}

void CAISharkStrategyOneShotAni::Destructor()
{
	super::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyOneShotAni::Set
//
//	PURPOSE:	Sets the one shot ani
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyOneShotAni::Set(CAnimatorAIAnimal::Main eMain)
{
	GetAnimator()->SetMain(eMain);

	m_eMain = eMain;
    m_bAnimating = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyOneShotAni::Update
//
//	PURPOSE:	Updates the strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAISharkStrategyOneShotAni::Update()
{
	super::Update();

    if ( !m_bAnimating )
	{
        return LTFALSE;
	}

	if ( GetAnimator()->IsAnimatingMainDone(m_eMain) )
	{
        m_bAnimating = LTFALSE;
	}

	GetAnimator()->SetMain(m_eMain);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyOneShotAni::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyOneShotAni::Load(HMESSAGEREAD hRead)
{
	super::Load(hRead);

	LOAD_BOOL(m_bAnimating);
	LOAD_DWORD_CAST(m_eMain, CAnimatorAIAnimal::Main);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISharkStrategyOneShotAni::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAISharkStrategyOneShotAni::Save(HMESSAGEREAD hWrite)
{
	super::Save(hWrite);

	SAVE_BOOL(m_bAnimating);
	SAVE_DWORD(m_eMain);
}