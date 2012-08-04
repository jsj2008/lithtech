// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIVehicleStrategy.h"
#include "CharacterMgr.h"
#include "AIVehicle.h"
#include "PlayerObj.h"
#include "AIHelicopter.h"
#include "AISense.h"
#include "AITarget.h"
#include "AIPath.h"
#include "Door.h"
#include "AIPathMgr.h"

// Factories

IMPLEMENT_FACTORY(CAIHelicopterStrategyFollowPath, 0);
IMPLEMENT_FACTORY(CAIHelicopterStrategyShoot, 0);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicleStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIVehicleStrategy::Constructor()
{
    m_pAIVehicle = LTNULL;
}

void CAIVehicleStrategy::Destructor()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIVehicleStrategy::GetAnimator
//
//	PURPOSE:	Gets our animator
//
// ----------------------------------------------------------------------- //

CAnimatorAIVehicle* CAIVehicleStrategy::GetAnimator()
{
	return GetAI()->GetAnimator();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategy::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategy::Constructor()
{
	CAIVehicleStrategy::Constructor();

    m_pAIHelicopter = LTNULL;
}

void CAIHelicopterStrategy::Destructor()
{
	CAIVehicleStrategy::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategy::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategy::Init(AI_Helicopter* pAIHelicopter)
{
	CAIVehicleStrategy::Init(pAIHelicopter);

	m_pAIHelicopter = pAIHelicopter;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyFollowPath::Constructor()
{
	CAIHelicopterStrategy::Constructor();

	m_eState = eStateUnset;
	m_AIMovement.Constructor();
	m_pPath = FACTORY_NEW(CAIPath);
}

void CAIHelicopterStrategyFollowPath::Destructor()
{
	FACTORY_DELETE(m_pPath);
	m_AIMovement.Destructor();

	CAIHelicopterStrategy::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyFollowPath::Init(AI_Helicopter* pAIHelicopter)
{
	if ( !CAIHelicopterStrategy::Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	if ( !m_AIMovement.Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	m_pPath->Init(pAIHelicopter);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyFollowPath::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_pPath )
	{
		m_pPath->HandleBrokenLink(hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyFollowPath::Update()
{
	if ( !CAIHelicopterStrategy::Update() )
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

			default:
			{
                g_pLTServer->CPrint("CAIHelicopterStrategyFollowPath::Update - unrecognized waypoint instruction");
				_ASSERT(!"CAIHelicopterStrategyFollowPath::Update - unrecognized waypoint instruction");
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
//	ROUTINE:	CAIHelicopterStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyFollowPath::UpdateMoveTo(CAIPathWaypoint* pWaypoint)
{
	if ( m_AIMovement.IsSet() )
	{
		GetAI()->Start();

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

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyFollowPath::Set(const LTVector& vDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), vDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIHelicopterStrategyFollowPath::Set(CAINode* pNodeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pNodeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

LTBOOL CAIHelicopterStrategyFollowPath::Set(CAIVolume* pVolumeDestination)
{
	m_AIMovement.Clear();
	m_eState = g_pAIPathMgr->FindPath(GetAI(), pVolumeDestination, m_pPath) ? eStateSet : eStateUnset;
	return m_eState == eStateSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	CAIHelicopterStrategy::Load(hRead);

	m_pPath->Load(hRead);
	m_AIMovement.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	CAIHelicopterStrategy::Save(hWrite);

	m_pPath->Save(hWrite);
	m_AIMovement.Save(hWrite);

	SAVE_DWORD(m_eState);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::Constructor()
{
	CAIHelicopterStrategy::Constructor();

    m_hTarget = LTNULL;
	m_eFire = CAnimatorAIVehicle::eFireFull;
}

void CAIHelicopterStrategyShoot::Destructor()
{
	CAIHelicopterStrategy::Destructor();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyShoot::Init(AI_Helicopter* pAIHelicopter)
{
	if ( !CAIHelicopterStrategy::Init(pAIHelicopter) )
	{
        return LTFALSE;
	}

	// For each weapon, setup burst info

	for ( int iWeapon = 0 ; iWeapon < GetAI()->GetNumWeapons() ; iWeapon++ )
	{
		CWeapon* pWeapon = GetAI()->GetWeapon(iWeapon);
		if ( pWeapon )
		{
			m_aBursts[iWeapon].m_bActive = LTTRUE;
			m_aBursts[iWeapon].m_fBurstInterval = 0;
			m_aBursts[iWeapon].m_nBurstShots = GetRandom(pWeapon->GetWeaponData()->nAIMinBurstShots, pWeapon->GetWeaponData()->nAIMaxBurstShots);
		}
		else
		{
			m_aBursts[iWeapon].m_bActive = LTFALSE;
			m_aBursts[iWeapon].m_fBurstInterval = (float)INT_MAX;
			m_aBursts[iWeapon].m_nBurstShots = 0;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::HandleBrokenLink
//
//	PURPOSE:	Handles a link to the AI being broken
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::HandleBrokenLink(HOBJECT hObject)
{
	if ( m_hTarget == hObject )
	{
		m_hTarget = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHelicopterStrategyShoot::Update()
{
	if ( !CAIHelicopterStrategy::Update() )
	{
        return LTFALSE;
	}

	if ( !m_hTarget )
	{
        return LTFALSE;
	}

	for ( int iWeapon = 0 ; iWeapon < GetAI()->GetNumWeapons() ; iWeapon++ )
	{
		CWeapon* pWeapon = GetAI()->GetWeapon(iWeapon);

		if ( m_aBursts[iWeapon].m_bFired && m_aBursts[iWeapon].m_bActive )
		{
			UpdateFiring(pWeapon, &m_aBursts[iWeapon]);

			m_aBursts[iWeapon].m_bFired = LTFALSE;
		}
		else
		{
			UpdateAiming(pWeapon, &m_aBursts[iWeapon]);
		}

		if ( 0 == pWeapon->GetAmmoInClip() )
		{
            pWeapon->ReloadClip(LTFALSE);
		}

		if ( 0 == pWeapon->GetParent()->GetAmmoCount(pWeapon->GetAmmoId()) )
		{
			pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Aim
//
//	PURPOSE:	Makes us Aim
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::Aim(CWeapon* pWeapon, BURSTSTRUCT* pBurst)
{
	GetAnimator()->SetFire(CAnimatorAIVehicle::eFireAim);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Fire
//
//	PURPOSE:	Makes us fire
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::Fire(CWeapon* pWeapon, BURSTSTRUCT* pBurst)
{
	GetAnimator()->SetFire(m_eFire);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::Load(HMESSAGEREAD hRead)
{
	CAIHelicopterStrategy::Load(hRead);

	LOAD_DWORD_CAST(m_eFire, CAnimatorAIVehicle::Fire);
	LOAD_HOBJECT(m_hTarget);

	for ( int iWeapon = 0 ; iWeapon < AI_MAX_WEAPONS ; iWeapon++ )
	{
		m_aBursts[iWeapon].Load(hRead);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::Save(HMESSAGEREAD hWrite)
{
	CAIHelicopterStrategy::Save(hWrite);

	SAVE_DWORD(m_eFire);
	SAVE_HOBJECT(m_hTarget);

	for ( int iWeapon = 0 ; iWeapon < AI_MAX_WEAPONS ; iWeapon++ )
	{
		m_aBursts[iWeapon].Save(hWrite);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::UpdateFiring
//
//	PURPOSE:	Handles firing
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::UpdateFiring(CWeapon* pWeapon, BURSTSTRUCT* pBurst)
{
	// Get our fire position

	LTVector vFirePos = GetAI()->GetAttachmentPosition(pWeapon->GetModelObject());
    LTRotation rFireRot = GetAI()->GetAttachmentRotation(pWeapon->GetModelObject());
	LTVector vFireRight, vFireUp, vFireForward;
	g_pMathLT->GetRotationVectors(rFireRot, vFireUp, vFireRight, vFireForward);

	// Get target's position

	LTVector vTargetPos;
    g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

	// Get our firing vector

	LTVector vFireDir = vTargetPos - vFirePos;
	vFireDir.Norm();

	// Make sure it's in our field of firce

	LTFLOAT fDp = vFireDir.Dot(vFireForward);
//    g_pLTServer->CPrint("fireangle = %f", (acos(fDp)/MATH_PI)*180.0f);
	if ( fDp <= c_fFOV90 )
	{
		Aim(pWeapon, pBurst);
		return;
	}

	// Now fire the weapon

	WFireInfo fireInfo;
	fireInfo.hFiredFrom = GetAI()->GetObject();
	fireInfo.vPath		= vFireDir;
	fireInfo.vFirePos	= vFirePos;
	fireInfo.vFlashPos	= vFirePos;
	fireInfo.hTestObj	= m_hTarget;
	fireInfo.fPerturbR	= 4.0f*(1.0f - GetAI()->GetAccuracy());
	fireInfo.fPerturbU	= 4.0f*(1.0f - GetAI()->GetAccuracy());

    pWeapon->UpdateWeapon(fireInfo, LTTRUE);

	// Decrement our burst counter and update the last ammo count

	pBurst->m_nBurstShots -= Max<int>(0, pBurst->m_nLastAmmoInClip - pWeapon->GetAmmoInClip());
	pBurst->m_nLastAmmoInClip = pWeapon->GetAmmoInClip();

	if ( pBurst->m_nBurstShots <= 0 )
	{
		// We just finished our burst. Start waiting.

		CalculateBurst(pWeapon, pBurst);

		// And just aim.

		Aim(pWeapon, pBurst);
	}
	else
	{
		// Keep firing

		Fire(pWeapon, pBurst);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::UpdateAiming
//
//	PURPOSE:	Handles Aiming
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::UpdateAiming(CWeapon* pWeapon, BURSTSTRUCT* pBurst)
{
	if ( pBurst->m_fBurstInterval > 0.0f )
	{
		// We are still waiting to fire.

        pBurst->m_fBurstInterval -= g_pLTServer->GetFrameTime();

		// So just aim.

		Aim(pWeapon, pBurst);
	}
	else
	{
		// We're done waiting, start firing again

		Fire(pWeapon, pBurst);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHelicopterStrategyShoot::CalculateBurst
//
//	PURPOSE:	Calculate all our burst parameters
//
// ----------------------------------------------------------------------- //

void CAIHelicopterStrategyShoot::CalculateBurst(CWeapon* pWeapon, BURSTSTRUCT* pBurst)
{
	pBurst->m_fBurstInterval = GetRandom(pWeapon->GetWeaponData()->fAIMinBurstInterval, pWeapon->GetWeaponData()->fAIMaxBurstInterval);
	pBurst->m_nBurstShots = GetRandom(pWeapon->GetWeaponData()->nAIMinBurstShots, pWeapon->GetWeaponData()->nAIMaxBurstShots);
}


void CAIHelicopterStrategyShoot::SetTarget(HOBJECT hTarget)
{
	GetAI()->Unlink(m_hTarget);
	m_hTarget = hTarget;
	GetAI()->Link(m_hTarget);
}
