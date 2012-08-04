
//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStrategyShootStream.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	04.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Handles firing when it comes in a constant stream instead of
//				in a burst.  A stream has a Start, Hold and Finish animation
//				IsFiring() is true as long as the beam is being emitted.
//              
//				TODO:	Currently, the first anim and part of the second
//						anim are garenteed.  The third anim may be skipped due
//						to a goal or state change.
//				
//						Hook streaming fire up to work with running characters.
//              
//----------------------------------------------------------------------------

// Includes
#include "stdafx.h"

#include "AIHumanStrategyShootStream.h"		
#include "AIHuman.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "AIMovement.h"

// Forward declarations

// Globals

// Statics
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyShootStream, kStrat_HumanShootStream);
const static LTFLOAT s_fPerturbScale = 4.0f;


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::CAIHumanStrategyShootStream()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CAIHumanStrategyShootStream::CAIHumanStrategyShootStream() : 
			m_bFiringStream( LTFALSE ),
			m_flStreamTime( 0 )
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::Save(ILTMessage_Write *pMsg)
{
	CAIHumanStrategyShoot::Save(pMsg);

	SAVE_BOOL(m_bFiringStream);
	SAVE_TIME(m_flStreamTime);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::Load(ILTMessage_Read *pMsg)
{
	CAIHumanStrategyShoot::Load(pMsg);

	LOAD_BOOL(m_bFiringStream);
	LOAD_TIME(m_flStreamTime);
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::Init()
//              
//	PURPOSE:	Handle the initialization of the StreamTime code
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStrategyShootStream::Init(CAIHuman* pAIHuman)
{
	AIASSERT( pAIHuman, NULL, "CAIHumanStrategyShootStream::Init: No human specified" );

	if ( !CAIHumanStrategyShoot::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_eFireState = eFireStateInvalid;

	// All recalculations of the burst firing now handled
	// by this function to reduce duplication
	CalculateStreamTime();

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::Fire()
//              
//	PURPOSE:	Hook into the Fire() function so that we can start our special
//				handling of the firing. 
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::Fire()
{
	CAIHumanStrategyShoot::Fire();
	
	if ( eFireStateInvalid == m_eFireState )
	{
		m_eFireState = eFireStateStart;
		m_bFiringStream = LTTRUE;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::UpdateFiring()
//              
//	PURPOSE:	Updates the firing state, first allowing the base class to
//				handle the basics, then extending it to check the animation 
//				lock state for handling progression from eFireStateStart to 
//				eFireStateFiring.
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::UpdateFiring(HOBJECT hTarget,const LTVector& vTargetPos, CWeapon* pWeapon)
{
	AIASSERT( LTTRUE == m_bFired, GetAI()->GetHOBJECT(), "Expected Fired to be true.  Check logic.");

	switch ( m_eFireState )
	{
		case eFireStateStart:
		{
			if ( !GetAI()->GetAnimationContext()->IsLocked() )
			{
				m_eFireState = eFireStateFiring;
			}
		}
		break;

		case eFireStateFiring:
		{
			// Get our fire position

			AIASSERT( GetAI()->GetCurrentWeapon(), GetAI()->m_hObject, "UpdateFiring without a weapon" );
			LTVector vFirePos = GetFirePosition(GetAI()->GetCurrentWeapon());

			// Now fire the weapon

			WeaponFireInfo weaponFireInfo;

			weaponFireInfo.nDiscTrackingType = MPROJ_DISC_TRACKING_STEADY;
			weaponFireInfo.hSocket		= m_hFiringSocket!=INVALID_MODEL_SOCKET ? m_hFiringSocket : GetAI()->GetWeaponSocket(GetAI()->GetCurrentWeapon());
			weaponFireInfo.hFiredFrom  = GetAI()->GetObject();
			weaponFireInfo.vPath       = vTargetPos - vFirePos;
			weaponFireInfo.vFirePos    = vFirePos;
			weaponFireInfo.vFlashPos   = vFirePos;
			weaponFireInfo.hTestObj    = hTarget;
			weaponFireInfo.fPerturbR	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());
			weaponFireInfo.fPerturbU	= LOWER_BY_DIFFICULTY(s_fPerturbScale)*(1.0f - GetAI()->GetAccuracy());

			pWeapon->UpdateWeapon(weaponFireInfo, LTTRUE);

			Fire();
			
			// Check to see our fire timer expired
			if ( m_flStreamTime < g_pLTServer->GetTime() || !GetAI()->GetAnimationContext()->IsLocked())
			{
				m_eFireState = eFireStateEnding;
			}
		}
		break;

		case eFireStateEnding:
		{
			if ( !GetAI()->GetAnimationContext()->IsLocked() )
			{
				m_eFireState = eFireStateInvalid;
				m_bFiringStream = LTFALSE;
				m_eState = eStateAiming;
				return;
			}
		}
		break;
	
		default:
		{
			AIASSERT( 0, m_pAIHuman->GetHOBJECT(), "CAIHumanStrategyShootStream::UpdateAnimation: Unexpected m_eFireState");
		}
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::UpdateAiming()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::UpdateAiming(HOBJECT hTarget)
{
	if ( m_flStreamTime < g_pLTServer->GetTime() )
	{
		// Don't calculate new stream time until finished firing animation.
		if( !GetAnimationContext()->IsLocked() )
		{
			CalculateStreamTime();
		}

		Aim();
	}
	else
	{
		// We're done waiting, fire if we're at a reasonable angle

		if ( m_bIgnoreFOV )
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

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::HandleFired()
//              
//	PURPOSE:	Turn on the stream.  Unlike standard firing, it does not go off
//				until either we run out of ammo, or until we decide to cut it
//				off based on time or some other such consideration.
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::HandleFired(const char* const pszSocketName)
{
	CAIHumanStrategyShoot::HandleFired(pszSocketName);
	Fire();	
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::HandleFired()
//              
//	PURPOSE:	Allow the base class to handle the Fire string.  Handle the
//				stream nature of the firing through the overridden HandleFired
//				function.
//
//				Handle the 'StopFiring' command here.
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStrategyShootStream::HandleModelString(ArgList* pArgList)
{
	CAIHumanStrategyShoot::HandleModelString(pArgList);
	
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyStopFireWeapon) )
	{
		m_eFireState = eFireStateEnding;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyShoot::UpdateAnimation
//
//	PURPOSE:	Handles any pending AnimationContext specializations.  
//				Extends the base class and makes the following assumptions:
//				1) Animations are being locked by the base class.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIHumanStrategyShootStream::UpdateAnimation()
{
	if( !CAIHumanStrategyShoot::UpdateAnimation() )
	{
		return LTFALSE;
	}

	switch ( m_eFireState )
	{
		case eFireStateStart:
		{
			// If we are in the fire state, and if our animation is not locked, 
			// then make sure it is cleared fully.  Rely on the base class to 
			// handle the initial locking.
			
			if( GetAI()->GetCurrentWeapon() == GetAI()->GetPrimaryWeapon() )
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireStreamStart);
			}
			else
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireSecondaryStreamStart);
			}
			GetAnimationContext()->Lock();
		}
		break;

		case eFireStateFiring:
		{
			// Streaming attacks are NEVER garenteed to play all the way through -- we want
			// tighter control over them than the animation will give us. BUT we want to also
			// have the ability to kick out when the animation ends UNLESS the animation is
			// looping.

			if( GetAI()->GetCurrentWeapon() == GetAI()->GetPrimaryWeapon() )
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireStream);
			}
			else
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireSecondaryStream);
			}
			GetAnimationContext()->Lock();
		}
		break;

		case eFireStateEnding:
		{
			if( GetAI()->GetCurrentWeapon() == GetAI()->GetPrimaryWeapon() )
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireStreamEnd);
			}
			else
			{
				GetAnimationContext()->SetProp(kAPG_WeaponAction, kAP_FireSecondaryStreamEnd);
			}
			GetAnimationContext()->Lock();
		}
		break;

		case eFireStateInvalid:
		{
			// we are not in one of the basic firing states, so don't do anything special
			;
		}
		break;

		default:
		{
			AIASSERT( 0, GetAI()->GetHOBJECT(), "CAIHumanStrategyShootStream::UpdateAnimation: Unexpected m_eFireState" );
		}
		break;
	}

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyShootStream::CalculateStreamTime()
//              
//	PURPOSE:	Calculate how long we should next fire a stream for.
//              
//----------------------------------------------------------------------------
void CAIHumanStrategyShootStream::CalculateStreamTime()
{
	CWeapon* pWeapon = GetAI()->GetCurrentWeapon();

	AIASSERT( pWeapon && pWeapon->GetWeaponData(), m_pAIHuman->GetHOBJECT(), "" );
	if ( !pWeapon || ( pWeapon && !pWeapon->GetWeaponData()) )
	{
		return;
	}

	m_flStreamTime = g_pLTServer->GetTime() + GetRandom(pWeapon->GetWeaponData()->fAIMinBurstInterval, pWeapon->GetWeaponData()->fAIMaxBurstInterval);
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStrategyFollowPath::DelayChangeState()
//              
//	PURPOSE:	Delay a state change as long as 
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIHumanStrategyShootStream::DelayChangeState()
{
	if ( m_eFireState == eFireStateStart || m_eFireState == eFireStateFiring )
	{
		return LTTRUE;
	}

	return CAIHumanStrategyShoot::DelayChangeState();
}


