// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponAbstract.cpp
//
// PURPOSE : AIWeaponAbstract class implementation
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponAbstract.h"
#include "AIWeaponMgr.h"
#include "AIUtils.h"
#include "Weapon.h"
#include "Arsenal.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorldState.h"
#include "AITarget.h"
#include "WeaponFireInfo.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AnimationContext.h"

CAIWeaponAbstract::CAIWeaponAbstract() : 
	m_pWeapon(NULL),
	m_eFiringState(kAIFiringState_None),
	m_iAnimRandomSeed((uint32)-1),
	m_fRandomSeedSelectionTime(0.f),
	m_pAIWeaponRecord(NULL),
	m_flWeaponContextInaccuracyScalar(1.0f),
	m_hWeaponSocket(INVALID_MODEL_SOCKET),
	m_bCanDropWeapon(true)
{
}

void CAIWeaponAbstract::Save(ILTMessage_Write *pMsg)
{
	HOBJECT hOwner = m_pWeapon->GetObject();
	ASSERT(IsAI(hOwner));

	SAVE_HOBJECT(hOwner);
	SAVE_HRECORD( m_pWeapon->GetWeaponRecord() );
	SAVE_STDSTRING(m_szFireSocketName);
	SAVE_INT(m_eFiringState);
	SAVE_DWORD(m_iAnimRandomSeed);
	SAVE_TIME(m_fRandomSeedSelectionTime);
	SAVE_FLOAT(m_flWeaponContextInaccuracyScalar);
	SAVE_DWORD(m_hWeaponSocket);
	SAVE_bool(m_bCanDropWeapon);
}

void CAIWeaponAbstract::Load(ILTMessage_Read *pMsg)
{
	HOBJECT hOwner = NULL;
	HWEAPON hWeapon = NULL;

	LOAD_HOBJECT(hOwner);
	LOAD_HRECORD( hWeapon, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_STDSTRING(m_szFireSocketName);
	LOAD_INT_CAST(m_eFiringState, ENUM_AIFiringState);
	LOAD_DWORD(m_iAnimRandomSeed);
	LOAD_TIME(m_fRandomSeedSelectionTime);
	LOAD_FLOAT(m_flWeaponContextInaccuracyScalar);
	LOAD_DWORD(m_hWeaponSocket);
	LOAD_bool(m_bCanDropWeapon);

	ASSERT(IsAI(hOwner));
	CAI* pAI = (CAI*)g_pLTServer->HandleToObject(hOwner);

	ASSERT(pAI->GetArsenal());
	if (pAI)
	{
		ASSERT(pAI->GetArsenal());
		if (CArsenal* pArsenal = pAI->GetArsenal())
		{
			m_pWeapon = pArsenal->GetWeapon(hWeapon);
		}
	}

	if( m_pWeapon )
	{
		m_pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
			m_pWeapon->GetWeaponRecord(), 
			pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::DefaultHandleModelString
//
//	PURPOSE:	Default model string handling function for any weapons not
//				needing any special function.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponAbstract::DefaultInit(CWeapon* pWeapon, CAI* pAI)
{
	ASSERT(pWeapon);
	if (!pWeapon)
	{
		return false;
	}

	m_pWeapon = pWeapon;

	HWEAPON hWeapon = m_pWeapon ? m_pWeapon->GetWeaponRecord() : NULL;

	m_pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
		hWeapon, 
		pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

	// Set up weapon-AI type specific data.

	if ( m_pAIWeaponRecord )
	{
		// Adjust the damage factor

		m_pWeapon->SetDamageFactor( GetRandom( m_pAIWeaponRecord->fDamageScalarMin, m_pAIWeaponRecord->fDamageScalarMax ) );
	
		// Store an inaccuracy for this weapon.  This allows different AIs to have 
		// different abilities with different weapon types

		m_flWeaponContextInaccuracyScalar = GetRandom( m_pAIWeaponRecord->fAccuracyScalarMin, m_pAIWeaponRecord->fAccuracyScalarMax );
	}
	else
	{
		m_pWeapon->SetDamageFactor( 1.0f );
		m_flWeaponContextInaccuracyScalar = 1.0f;
	}

	// Cache a flag indicating if this weapon can be dropped.

	m_bCanDropWeapon = true;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
	bool bNotForPlayer = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bNotForPlayer );
	if ( bNotForPlayer || !g_pWeaponDB->IsPlayerWeapon( hWeapon ) )
	{
		m_bCanDropWeapon = false;
	}

	// Cache socket weapon is attached to.

	CActiveWeapon* pActiveWeapon = pAI->GetAIWeaponMgr()->FindActiveWeaponOfType( m_pAIWeaponRecord->eAIWeaponType );
	if( pActiveWeapon )
	{
		g_pModelLT->GetSocket( pAI->m_hObject, pActiveWeapon->GetSocketName(), m_hWeaponSocket );
	}

	// If this weapon was dynamically added, make sure the animation is 
	// updated the first frame.

	UpdateAnimation( pAI );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::GetWeaponRecord
//
//	PURPOSE:	Returns the weapon record associated wtih this AIWeapon.
//
// ----------------------------------------------------------------------- //

HWEAPON CAIWeaponAbstract::GetWeaponRecord()
{
	if (m_pWeapon)
	{
		return m_pWeapon->GetWeaponRecord();
	}
	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::DefaultHandleModelString
//
//	PURPOSE:	Default model string handling function for any weapons not
//				needing any special function.
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::DefaultHandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg )
{
	if (!pAI)
	{
		return;
	}

	static CParsedMsg::CToken s_cTok_CineFireWeapon("CINEFIRE");
	static CParsedMsg::CToken s_cTok_FireWeapon(c_szKeyFireWeapon);
	static CParsedMsg::CToken s_cTok_Throw(c_szKeyThrow);

	// FIRE and THROW 

	// When an AI is throwing a grenade, the thrown weapon ought to be
	// the current weapon.  If this changes, and they firing needs to
	// operate differently at a low level, either this default 
	// implementation can change, or the grenade can provide its own 
	// implementation.
	if (cParsedMsg.GetArg(0) == s_cTok_FireWeapon ||
		cParsedMsg.GetArg(0) == s_cTok_Throw )
	{
		if (cParsedMsg.GetArgCount() > 1)
		{
			const char* const pszSocketName = cParsedMsg.GetArg(1).c_str();
			m_szFireSocketName = pszSocketName ? pszSocketName : "";
		}
		else
		{
			m_szFireSocketName = "";
		}

		Fire(pAI);
		return;
	}

	// Cinematic firing.

	if( cParsedMsg.GetArg(0) == s_cTok_CineFireWeapon )
	{
		Cinefire(pAI);
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::DefaultAim
//
//	PURPOSE:	Aims the weapon
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::DefaultAim(CAI* pAI)
{
	if (pAI)
	{
		pAI->GetAnimationContext()->SetProp( kAPG_Action, kAP_ACT_Aim );
	}
	
	UpdateWeaponAnimation(pAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::DefaultGetFirePosition
//
//	PURPOSE:	Returns the source position a shot will be fired from.
//
// ----------------------------------------------------------------------- //

LTVector CAIWeaponAbstract::DefaultGetFirePosition(CAI* pAI)
{
	if (!pAI)
	{
		return LTVector(0,0,0);
	}

	if ( !m_szFireSocketName.empty() )
	{
		HMODELSOCKET hFiringSocket = INVALID_MODEL_SOCKET;

		// Set the socket to fire from to the socket named if it exists
		// check to see if we already have the socket so we can try to
		// avoid annoying lookups.
		g_pModelLT->GetSocket( 
			pAI->m_hObject, 
			m_szFireSocketName.c_str(), 
			hFiringSocket);

		LTTransform transform;
		LTRESULT SocketTransform = g_pModelLT->GetSocketTransform( pAI->m_hObject, hFiringSocket, transform, true );
		AIASSERT( SocketTransform == LT_OK, pAI->m_hObject, "Unable to get socket for transform" );
		return transform.m_vPos;
	}
	else 
	{
		return pAI->GetWeaponPosition( m_pWeapon, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Reload
//
//	PURPOSE:	Reloads the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::DefaultReload(CAI* pAI)
{
	if (!m_pWeapon)
	{
		return;
	}

	if (pAI)
	{
		pAI->GetAIWorldState()->SetWSProp( kWSK_WeaponLoaded, pAI->GetHOBJECT(), kWST_bool, true );
	}

	m_eFiringState = kAIFiringState_Reloading;
	UpdateWeaponAnimation(pAI);

	if( m_pWeapon->GetAmmoInClip() > 0 )
	{
		return;
	}

	m_pWeapon->ReloadClip( false );

	// Keep track of reload counts on the blackboard.

	uint32 nRounds = pAI->GetAIBlackBoard()->GetBBRoundsFired();
	pAI->GetAIBlackBoard()->SetBBRoundsFired( nRounds + 1 );

	// After a reload we may not have any ammo in our reserves.
	if (m_pAIWeaponRecord && m_pAIWeaponRecord->bAllowAmmoGeneration)
	{
		m_pWeapon->GetArsenal()->AddAmmo( m_pWeapon->GetAmmoRecord(), 999999 );
	}
}

bool CAIWeaponAbstract::DefaultFire(CAI* pAI, const LTVector& vTargetPos, bool bAnimatesReload)
{
	if( !( m_pWeapon && m_pAIWeaponRecord ) )
	{
		return false;
	}

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();

	// Get our fire position

	LTVector vFirePos = GetFirePosition(pAI);

	// Get our firing vector

	LTVector vDir = vTargetPos - vFirePos;
	vDir.Normalize();

	// Now fire the weapon

	WeaponFireInfo weaponFireInfo;
	static uint8 s_nCount = GetRandom( 0, 255 );
	s_nCount++;

	weaponFireInfo.hFiredFrom  = pAI->GetHOBJECT();
	weaponFireInfo.vPath       = vDir;
	weaponFireInfo.vFirePos    = vFirePos;
	weaponFireInfo.vFlashPos   = vFirePos;
	weaponFireInfo.hTestObj    = hTarget;
	weaponFireInfo.hFiringWeapon = m_pWeapon->GetModelObject();
	weaponFireInfo.nSeed		= (uint8)GetRandom( 2, 255 );
	weaponFireInfo.nPerturbCount = s_nCount;
	weaponFireInfo.nFireTimestamp = g_pLTServer->GetRealTimeMS( );
	weaponFireInfo.bLeftHandWeapon = ( LTStrIEquals( m_szFireSocketName.c_str( ), "LEFTHAND" ) );

	if( pAI->GetAIBlackBoard()->GetBBPerfectAccuracy() )
	{
		weaponFireInfo.fPerturb = 0.0f;
	}
	else 
	{
		weaponFireInfo.fPerturb = 1.f;
	}

	WeaponState eWeaponState = m_pWeapon->UpdateWeapon( weaponFireInfo, true );

	if( eWeaponState == W_FIRED )
	{
		UpdateWeaponAnimation( pAI );
	}

	if( m_pWeapon->GetAmmoInClip() == 0 )
	{
		// Automatically reload if:
		// 1) The AI has more ammo for this weapon and
		// 2) Either the AI is flagged to autoreload, or the weapon is flagged to not animation reloads.
		if( AIWeaponUtils::HasAmmo(pAI, m_pAIWeaponRecord->eAIWeaponType, !AIWEAP_CHECK_HOLSTER) 
			&& ( ( pAI->GetAIBlackBoard()->GetBBAutoReload() && m_pAIWeaponRecord->bAllowAutoReload ) || !bAnimatesReload) )
		{
			Reload(pAI);
		}
		else 
		{
			pAI->GetAIWorldState()->SetWSProp( kWSK_WeaponLoaded, pAI->GetHOBJECT(), kWST_bool, false );
		}
	}
	else 
	{
		pAI->GetAIWorldState()->SetWSProp( kWSK_WeaponLoaded, pAI->GetHOBJECT(), kWST_bool, true );
	}

	// If the primary weapon is now out of ammo, set the WeaponArmed 
	// worldstate to false.

	if( pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() == m_pAIWeaponRecord->eAIWeaponType
		&& !AIWeaponUtils::HasAmmo(pAI, m_pAIWeaponRecord->eAIWeaponType, !AIWEAP_CHECK_HOLSTER))
	{
		pAI->GetAIBlackBoard()->SetBBSelectAction(true);
	}

	return true;
}

bool CAIWeaponAbstract::DefaultThrow(CAI* pAI)
{
	// Make sure the basic pointers are valid.
	ASSERT(m_pWeapon);
	ASSERT(pAI);
	if (!pAI || !m_pWeapon || !m_pAIWeaponRecord)
	{
		return false;
	}

	// Don't fire if AI has no target.

	if( !pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		return false;
	}

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();

	// Throw at the last known position.

	LTVector vTargetPos;
	CAIWMFact factTargetQuery;
	factTargetQuery.SetFactType( kFact_Character );
	factTargetQuery.SetTargetObject( hTarget );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factTargetQuery );
	if( pFact )
	{
		vTargetPos = pFact->GetPos();
	}
	else {
		g_pLTServer->GetObjectPos(hTarget, &vTargetPos);
	}


	// Offset the target pos a little so projectile lands in front of the target.

	LTVector vOffsetDir;
	vOffsetDir = pAI->GetPosition() - vTargetPos;
	vOffsetDir.y = 0.f;
	vOffsetDir.Normalize();
	vTargetPos += vOffsetDir * 384.f; 

	// Get our fire position

	LTVector vFirePos = GetFirePosition(pAI);

	// Velocity Vo

	LTVector vGravity;
	g_pPhysicsLT->GetGlobalForce( vGravity );

	// Vo = (S - R - 1/2*G*t^2) / t         
	// Vo = initial velocity
	// S = destination
	// R = origin
	// G = gravity
	// t = hangtime

	float fHangtime = 0.5f;
	LTVector vVelocity = ( vTargetPos - vFirePos - vGravity * .5f * fHangtime * fHangtime ) / fHangtime;
	float fVelocity = vVelocity.Mag();
	LTVector vDir( vVelocity / fVelocity );

	// Now fire the weapon

	WeaponFireInfo weaponFireInfo;
	static uint8 s_nCount = GetRandom( 0, 255 );
	s_nCount++;

	weaponFireInfo.hFiredFrom = pAI->GetHOBJECT();
	weaponFireInfo.vPath = vDir;
	weaponFireInfo.bOverrideVelocity = LTTRUE;
	weaponFireInfo.fOverrideVelocity = fVelocity;
	weaponFireInfo.vFirePos	= vFirePos;
	weaponFireInfo.vFlashPos = vFirePos;
	weaponFireInfo.hTestObj	= hTarget;
	weaponFireInfo.fPerturb = 1.0f * (1.0f - pAI->GetAccuracy() );
	weaponFireInfo.nSeed = (uint8)GetRandom( 2, 255 );
	weaponFireInfo.nPerturbCount = s_nCount;
	weaponFireInfo.nFireTimestamp = g_pLTServer->GetRealTimeMS( );

	m_pWeapon->ReloadClip( LTFALSE );

	if (m_pAIWeaponRecord->bAllowAmmoGeneration)
	{
		m_pWeapon->GetArsenal()->AddAmmo( m_pWeapon->GetAmmoRecord(), 999999 );
	}

	m_pWeapon->UpdateWeapon( weaponFireInfo, LTTRUE );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::GetShootPosition
//
//	PURPOSE:	Get shoot position, including some random
//				inaccuracy due to target movement.
//				Return true if the position is a Hit.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponAbstract::GetShootPosition( CAI* pAI, AimContext& Context,LTVector& outvShootPos )
{
	ASSERT(pAI);

	// Cineractive firing.

	if( m_eFiringState == kAIFiringState_CineFiring )
	{
		LTVector vDir = pAI->GetWeaponForward( m_pWeapon );
		vDir.Normalize();
		outvShootPos = pAI->GetPosition() + ( vDir * 5000.f );
		return true;
	}

	// If perfect accuracy is enabled, we are done.

	if( pAI->GetAIBlackBoard()->GetBBPerfectAccuracy() )
	{
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		g_pLTServer->GetObjectPos( hTarget, &outvShootPos );
		return true;
	}

	// Initially aim for the target's visible position.
	// If the target is not visible at all, use his actual position.
	// This is a failsafe for AI shooting at the origin if they have
	// not yet seen the target ever.

	LTVector vVisiblePosition = pAI->GetTarget()->GetVisiblePosition();
	if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		vVisiblePosition = pAI->GetAIBlackBoard()->GetBBTargetPosition();
	}
	outvShootPos = vVisiblePosition;

	// If Target is within the FullAccuracy radius, we are done.

	if( pAI->GetTarget()->GetTargetDistSqr() < pAI->GetFullAccuracyRadiusSqr() )
	{
		return true;
	}

	// The following code forces the AI to intenionally miss every x
	// number of shots, depending on their accuracy. This gives players
	// the excitement of getting shot at without killing them too fast.

	// For example, if accuracy = 0.5 there will be a guaranteed sequence
	// of HIT, MISS, HIT, MISS, ...
	// If accuracy = 0.25, then HIT, MISS, MISS, MISS, HIT, MISS, MISS, MISS, etc.
	// If accuracy = 0.75, then HIT, HIT, HIT, MISS, HIT, HIT, HIT, MISS, etc.

	// Calculate the ratio of hits to misses based on the current 
	// accuracy.  This needs to be recalculated for every shot, 
	// because accuracy may change at any time.

	float fAccuracy = m_flWeaponContextInaccuracyScalar * pAI->GetAccuracy();
	if( fAccuracy <= 0.f )
	{
		Context.m_cMisses = 1;
		Context.m_cHits = 0;
	}
	else if( fAccuracy >= 1.f )
	{
		Context.m_cMisses = 0;
		Context.m_cHits = 1;
	}
	else if( fAccuracy < 0.5f )
	{
		Context.m_cMisses = (uint32)( ( ( 1.f - fAccuracy ) / fAccuracy ) + 0.5f );
		Context.m_cHits = 1;
	}
	else 
	{
		Context.m_cMisses = 1;
		Context.m_cHits = (uint32)( ( fAccuracy / ( 1.f - fAccuracy ) ) + 0.5f );
	}

	// If we have met or exceeded the required number of misses, 
	// reset the counters.

	if( Context.m_iMiss >= Context.m_cMisses )
	{
		Context.m_iHit = 0;
		Context.m_iMiss = 0;
	}

	//
	// First take care of hits, then take care of misses.
	//

	// Hit.

	if( Context.m_iHit < Context.m_cHits )
	{
		++Context.m_iHit;

		// Blind fire.

		if( pAI->GetAIBlackBoard()->GetBBBlindFire() )
		{
			GetBlindFirePosition( pAI, outvShootPos, !FIRE_MISS );
			return false;
		}

		// Suppression fire at last known pos.

		if( pAI->GetAIBlackBoard()->GetBBSuppressionFire() )
		{
			HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();

			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Character );
			factQuery.SetTargetObject( hTarget );
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( pFact )
			{
				outvShootPos = pFact->GetPos();
			}
		}

		// Default fire.

		// If target has started moving or change directions recently,
		// factor in some inaccuracy.

		float fInnaccuracy = LTMAX( 0.f, pAI->GetTarget()->GetCurMovementInaccuracy() );
		if( fInnaccuracy > 0.f )
		{
			LTVector vShootOffset = LTVector(	GetRandom( -fInnaccuracy, fInnaccuracy ),
				GetRandom( -fInnaccuracy * 0.5f, fInnaccuracy * 0.5f ),
				GetRandom( -fInnaccuracy, fInnaccuracy ) );
			vShootOffset.Normalize();

			outvShootPos += vShootOffset * 100.0f;
		}
		
		return true;
	}

	// Miss.

	else 
	{
		++Context.m_iMiss;

		// Blind fire.

		if( pAI->GetAIBlackBoard()->GetBBBlindFire() )
		{
			GetBlindFirePosition( pAI, outvShootPos, FIRE_MISS );
			return false;
		}

		// Default fire.

		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if( !IsCharacter( hTarget ) )
		{
			return false;
		}

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
		if( !pChar )
		{
			return false;
		}

		// Intentionally shoot a little short of the target.

		LTVector vPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();;

		// Suppression fire at last known pos.

		if( pAI->GetAIBlackBoard()->GetBBSuppressionFire() )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Character );
			factQuery.SetTargetObject( hTarget );
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( pFact )
			{
				vPos = pFact->GetPos();
			}
		}

		float fDist = sqrt( pAI->GetTarget()->GetTargetDistSqr() );

		float fRadius = pChar->GetRadius();

		float fRand = GetRandom( 0.f, 1.f );
		fDist -= ( fRadius * 2.f ) + ( fRand * pAI->GetAccuracyMissPerturb() );

		// Calculate a position to the right or left of the target.

		LTVector vDir = vPos - pAI->GetPosition();
		if( vDir != LTVector::GetIdentity() )
		{
			vDir.Normalize();
		}

		vPos = pAI->GetPosition() + ( vDir * fDist );

		LTVector vRight = vDir.Cross( LTVector( 0.f, 1.f, 0.f ) );

		fRand = GetRandom( 0.f, 1.f );
		float fPerturb = ( ( pAI->GetAccuracyMissPerturb() * 2.f ) * fRand ) - pAI->GetAccuracyMissPerturb();
		vRight *= fPerturb;

		// Apply the offset to miss the target.

		outvShootPos = vPos + vRight;

		// Force bullets to land in front of the target, on the floor.

		if( m_pAIWeaponRecord->bForceMissToFloor )
		{
			float fFloor = pAI->GetAIBlackBoard()->GetBBTargetPosition().y;
			fFloor -= pAI->GetAIBlackBoard()->GetBBTargetDims().y;
			outvShootPos.y = fFloor;
		}

		return false;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::GetBlindFirePosition
//
//	PURPOSE:	Fire in front of the player.
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::GetBlindFirePosition(CAI* pAI, LTVector& outvShootPos, bool bMiss )
{
	ASSERT(pAI);

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !IsCharacter( hTarget ) )
	{
		return;
	}

	CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
	if( !pChar )
	{
		return;
	}

	// Intentionally shoot a little short of the target.

	float fDist = sqrt( pAI->GetTarget()->GetTargetDistSqr() );

	float fRadius = pChar->GetRadius();
	float fRand = GetRandom( 0.f, 1.f );
	fDist -= ( fRadius * 2.f ) + ( fRand * pAI->GetAccuracyMissPerturb() );

	// Aim wherever the weapon is aiming.

	LTVector vDir = pAI->GetWeaponForward( m_pWeapon );
	outvShootPos = pAI->GetPosition() + ( vDir * fDist );

	// Force bullets to land in front of the target, on the floor.

	if( bMiss && m_pAIWeaponRecord->bForceMissToFloor )
	{
		float fFloor = pAI->GetAIBlackBoard()->GetBBTargetPosition().y;
		fFloor -= pAI->GetAIBlackBoard()->GetBBTargetDims().y;
		outvShootPos.y = fFloor;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::SyncWeaponAnimation
//
//	PURPOSE:	Plays an animation matching the name of the animation 
//				playing on the owners main tracker.  If a matching 
//				animation is not found, the animation specified in the 
//				WeaponDatas DefaultAnimationName property is used.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIWeaponAbstract::SyncWeaponAnimation( CAI* pAI, HMODELANIM hLastUserAnimation )
{
	// Only update the animation this way if the weapon is flagged to sync to 
	// the users animation.

	if ( !m_pAIWeaponRecord || !m_pAIWeaponRecord->bSyncToUserAnimation )
	{
		return INVALID_ANI;
	}

	// Determine if the users animation has changed.

	HMODELANIM hCurrentUserAnim = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( pAI->GetHOBJECT(), pAI->GetAnimationContext()->GetTrackerID(), hCurrentUserAnim );
	if ( hCurrentUserAnim != hLastUserAnimation )
	{
		// Attempt to play the matching animation by name of the owners animation.

		char szAnimName[64] = { '\0' };
		if ( LT_OK == g_pModelLT->GetAnimName( pAI->GetHOBJECT(), hCurrentUserAnim, szAnimName, LTARRAYSIZE(szAnimName) ) )
		{
			HMODELANIM hWeaponAnim = INVALID_MODEL_ANIM;
			if( g_pModelLT->GetAnimIndex( m_pWeapon->GetModelObject(), szAnimName, hWeaponAnim ) == LT_OK )
			{
				bool bSemiAuto = g_pWeaponDB->GetBool(m_pWeapon->GetWeaponData(), WDB_WEAPON_bSemiAuto);
				if ( m_pWeapon->PlayAnimation( hWeaponAnim, true, bSemiAuto ) )
				{
#ifndef _FINAL
					// Skip validation if this is the default animation (this
					// animation is meant as a fallback), though it may happen 
					// to match in name.

					if ( !LTStrIEquals( m_pAIWeaponRecord->szDefaultAnimationName, szAnimName ) )
					{
						// In non final builds, print out a warning message if 
						// the AI animation and weapon animation are different 
						// lengths.  This is likely caused by tweaking the AI 
						// animation timing without maintaining the weapon timing.

						uint32 nAIAnimLength = 0;
						g_pModelLT->GetAnimLength( pAI->GetHOBJECT(), hCurrentUserAnim, nAIAnimLength );

						uint32 nWeaponAnimLength = 0;
						g_pModelLT->GetAnimLength( m_pWeapon->GetModelObject(), hWeaponAnim, nWeaponAnimLength );

						if ( nAIAnimLength != nWeaponAnimLength )
						{
							char szAIModelName[80];
							g_pModelLT->GetModelFilename( pAI->GetHOBJECT(), szAIModelName, LTARRAYSIZE( szAIModelName ) ); 

							char szWeaponModelName[80];
							g_pModelLT->GetModelFilename( m_pWeapon->GetModelObject(), szWeaponModelName, LTARRAYSIZE( szWeaponModelName ) ); 

							ObjectCPrint( pAI->GetHOBJECT(), "Weapon animation length does not match AI animation length:\n"
															" Anim: '%s'\n"
															" AI Model: %s\n"
															" AI Anim Length: %d\n"
															" Weapon Model: %s\n"
															" Weapon Anim Length: %d",
								szAnimName, szAIModelName, nAIAnimLength, szWeaponModelName, nWeaponAnimLength );
						}
					}
#endif 
					return hCurrentUserAnim;
				}
			}
		}
		
		// Failed to play the matching animation; play the default animation instead.

		HMODELANIM hWeaponAnim = INVALID_MODEL_ANIM;
		if( g_pModelLT->GetAnimIndex( 
			m_pWeapon->GetModelObject(), 
			m_pAIWeaponRecord->szDefaultAnimationName, 
			hWeaponAnim ) == LT_OK )
		{
			bool bSemiAuto = g_pWeaponDB->GetBool(m_pWeapon->GetWeaponData(), WDB_WEAPON_bSemiAuto);
			if ( m_pWeapon->PlayAnimation( hWeaponAnim, true, LOOP ) )
			{
				return hCurrentUserAnim;
			}
		}
	}

	// Make sure the contexts animation rate matches the weapons animation rate.

	float flAnimation = pAI->GetAnimationContext()->GetAnimRate();
	m_pWeapon->SetAnimRate( flAnimation );

	return hCurrentUserAnim;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::UpdateWeaponAnimation
//
//	PURPOSE:	Play appropriate weapon animations, and handle looping sounds.
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::UpdateWeaponAnimation( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Don't use the old animation system if the weapon should instead use
	// the animation matching the name of the owners animation.  Technically
	// this branch cound be done here, but UpdateWeaponAnimation is only 
	// called when certain events happen.  Changing this may break existing
	// behavior FEAR is relying on.

	if ( m_pAIWeaponRecord && m_pAIWeaponRecord->bSyncToUserAnimation )
	{
		return;
	}

	HOBJECT hWeaponModel = m_pWeapon->GetModelObject();

	uint32 dwAnimState		= g_pLTServer->GetModelPlaybackState( hWeaponModel );
	uint32 dwAni			= g_pLTServer->GetModelAnimation( hWeaponModel );

	uint32 dwPreFireAni		= m_pWeapon->GetPreFireAni();
	uint32 dwFireAni		= m_pWeapon->GetFireAni();
	uint32 dwPostFireAni	= m_pWeapon->GetPostFireAni();
	uint32 dwReloadAni		= m_pWeapon->GetReloadAni();

	// Play a new animation.

	switch( m_eFiringState )
	{
		//
		// Firing.
		//

		case kAIFiringState_Firing:
		case kAIFiringState_CineFiring:
		{
			// Currently playing a weapon animation.

			if( ( dwAni == dwFireAni ) ||
				( dwAni == dwPreFireAni ) ||
				( dwAni == dwPostFireAni ) ||
				( dwAni == dwReloadAni ) )
			{
				if( !( dwAnimState & MS_PLAYDONE ) )
				{
					return;
				}
			}

			// Play Pre-fire animation.

			if( ( dwAni != dwFireAni ) &&
				( dwPreFireAni != INVALID_ANI ) )
			{
				m_pWeapon->PlayAnimation( dwPreFireAni, true, false );
				return;
			}

			// Play fire animation.

			if( dwFireAni != INVALID_ANI )
			{
				m_pWeapon->PlayAnimation( dwFireAni, true, false );
				return;
			}

			// Play post-fire animation.

			if( dwPostFireAni != INVALID_ANI )
			{
				m_pWeapon->PlayAnimation( dwPostFireAni, true, false );
				return;
			}
		}
		break;

		//
		// Aiming.
		//

		case kAIFiringState_None:
		case kAIFiringState_Aiming:
		case kAIFiringState_Throwing:
		{
			// Play post-fire animation.

			if( ( dwAni == dwFireAni ) || 
				( dwAni == dwPreFireAni ) )
			{
				if( dwPostFireAni != INVALID_ANI )
				{
					m_pWeapon->PlayAnimation( dwPostFireAni, true, false );
					return;
				}
			}
			
			// No post-fire exists, so just kill the loop.

			if ( (dwAni != dwPostFireAni) || (dwAni == INVALID_ANI) )
			{
				// don't kill the loop sound if the current animation
				// is the PostFire, since the PostFire is supposed to
				// turn off the looping sounds (if it isn't doing it, it's
				// a content bug). Otherwise, kill the loop. -- Terry
				m_pWeapon->KillLoopSound();
			}
		}
		break;

		//
		// Reloading.
		//

		case kAIFiringState_Reloading:
		{
			// Played a reload animation.

			if( dwAni == dwReloadAni )
			{
				return;
			}

			// Play a post-fire animation.

			if(	( dwAni != dwPostFireAni ) &&
				( dwPostFireAni != INVALID_ANI ) ) 
			{
				m_pWeapon->PlayAnimation( dwPostFireAni, true, false );
				return;
			}

			// Play a reload animation.

			if(	dwReloadAni != INVALID_ANI )
			{
				m_pWeapon->PlayAnimation( dwReloadAni, true, false );
				return;
			}

			// No reload exists, so just kill the loop.

			m_pWeapon->KillLoopSound();
		}
		break;
	}
}

void CAIWeaponAbstract::DefaultCalculateBurst( CAI* pAI, double* pfOutBurstInterval, int* pnOutBurstShots )
{
	// Sanity check.

	if ( !m_pWeapon 
		|| !m_pWeapon->GetWeaponRecord() 
		|| !pfOutBurstInterval
		|| !pnOutBurstShots )
	{
		return;
	}

	// Determine the number of shots in the burst, and the number of 

	if ( m_pAIWeaponRecord )
	{
		*pfOutBurstInterval = g_pLTServer->GetTime() + 
			GetRandom( 
				m_pAIWeaponRecord->fAIMinBurstInterval, 
				m_pAIWeaponRecord->fAIMaxBurstInterval );
		
		*pnOutBurstShots = 
			GetRandom( 
				(int)m_pAIWeaponRecord->nAIMinBurstShots, 
				(int)m_pAIWeaponRecord->nAIMaxBurstShots );
	}
	else
	{
		*pfOutBurstInterval = DBL_MAX;
		*pnOutBurstShots = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponAbstract::Cinefire
//
//	PURPOSE:	Fire the weapon from a scripted command.
//
// ----------------------------------------------------------------------- //

void CAIWeaponAbstract::Cinefire(CAI* pAI)
{
	m_eFiringState = kAIFiringState_CineFiring;
	Fire(pAI);	
}
