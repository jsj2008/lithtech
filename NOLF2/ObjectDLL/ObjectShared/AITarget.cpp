// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseRecorderAbstract.h"
#include "AIUtils.h"
#include "AIBrain.h"
#include "ObjectRelationMgr.h" 
#include "CharacterMgr.h" 

DEFINE_AI_FACTORY_CLASS(CAITarget);


#define MAX_PHASE				7

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Constructor/Destructor
//
//	PURPOSE:	Factory con/destructor
//
// ----------------------------------------------------------------------- //

CAITarget::CAITarget()
{
    m_bVisibleFromEye = LTFALSE;
    m_bVisibleFromWeapon = LTFALSE;

	m_hRelationNotifier.SetObserver( this );
	m_hVisionBlocker = LTNULL;

	m_fTargetTime = 0.f;
	m_fCurMovementInaccuracy = 0.f;

	m_iHit = 0;
	m_iMiss = 0;
	m_cHits = 0;
	m_cMisses = 0;

	VEC_INIT(m_vTargetVelocity);
	VEC_INIT(m_vTargetPosition);
	VEC_INIT(m_vVisiblePosition);

	m_fTargetDistSqr = 0.f;

	m_bCanUpdateVisibility = LTTRUE;

	m_fPushSpeed = 0.f;
	m_fPushMinDist = 0.f;
	m_fPushMinDistSqr = 0.f;
	m_fPushThreshold = 64.f;

    m_bAttacking = LTFALSE;

	m_nPhase = 2;
	m_nResetPhase = 2;
	m_fPhaseStep = 0.f;

	m_hObject = NULL;

	m_bPrimaryOnLeft = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Save(ILTMessage_Write *pMsg)
{
    if ( !g_pLTServer || !pMsg ) return;

	SAVE_BOOL(m_bCanUpdateVisibility);
	SAVE_BOOL(m_bVisibleFromEye);
	SAVE_BOOL(m_bVisibleFromWeapon);
	SAVE_HOBJECT(m_hObject);
	SAVE_HOBJECT(m_hVisionBlocker);

	SAVE_DWORD(m_iHit);
	SAVE_DWORD(m_iMiss);
	SAVE_DWORD(m_cHits);
	SAVE_DWORD(m_cMisses);
	SAVE_FLOAT(m_fCurMovementInaccuracy);

	SAVE_TIME(m_fTargetTime);
	SAVE_VECTOR(m_vTargetVelocity);
	SAVE_VECTOR(m_vTargetPosition);
	SAVE_VECTOR(m_vVisiblePosition);
	SAVE_VECTOR(m_vPosition);
	SAVE_FLOAT(m_fTargetDistSqr);
	SAVE_VECTOR(m_vTargetDims);
	SAVE_BOOL(m_bAttacking);
	SAVE_INT(m_nPhase);
	SAVE_INT(m_nResetPhase);
	SAVE_FLOAT(m_fPhaseStep);

	SAVE_FLOAT(m_fPushSpeed);
	SAVE_FLOAT(m_fPushMinDist);
	SAVE_FLOAT(m_fPushMinDistSqr);
	SAVE_FLOAT(m_fPushThreshold);

	SAVE_BOOL(m_bPrimaryOnLeft);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Load
//
//	PURPOSE:	Loads our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Load(ILTMessage_Read *pMsg)
{
    if ( !g_pLTServer || !pMsg ) return;

	LOAD_BOOL(m_bCanUpdateVisibility);
	LOAD_BOOL(m_bVisibleFromEye);
	LOAD_BOOL(m_bVisibleFromWeapon);
	LOAD_HOBJECT(m_hObject);
	LOAD_HOBJECT(m_hVisionBlocker);

	LOAD_DWORD(m_iHit);
	LOAD_DWORD(m_iMiss);
	LOAD_DWORD(m_cHits);
	LOAD_DWORD(m_cMisses);
	LOAD_FLOAT(m_fCurMovementInaccuracy);

	LOAD_TIME(m_fTargetTime);
	LOAD_VECTOR(m_vTargetVelocity);
	LOAD_VECTOR(m_vTargetPosition);
	LOAD_VECTOR(m_vVisiblePosition);
	LOAD_VECTOR(m_vPosition);
	LOAD_FLOAT(m_fTargetDistSqr);
	LOAD_VECTOR(m_vTargetDims);
	LOAD_BOOL(m_bAttacking);
	LOAD_INT(m_nPhase);
	LOAD_INT(m_nResetPhase);
	LOAD_FLOAT(m_fPhaseStep);

	LOAD_FLOAT(m_fPushSpeed);
	LOAD_FLOAT(m_fPushMinDist);
	LOAD_FLOAT(m_fPushMinDistSqr);
	LOAD_FLOAT(m_fPushThreshold);

	LOAD_BOOL(m_bPrimaryOnLeft);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Init
//
//	PURPOSE:	Intialized targeting for an AI.
//
// ----------------------------------------------------------------------- //

void CAITarget::Init(CAI* pAI)
{
	m_pAI = pAI;
	
	m_hRelationNotifier.SetSubject( pAI->GetRelationMgr() );

	if( pAI->GetBrain() && pAI->GetBrain()->GetAIDataExist( kAIData_PrimaryWeaponOnLeft ) )
	{
		m_bPrimaryOnLeft = ( GetAI()->GetBrain()->GetAIData( kAIData_PrimaryWeaponOnLeft ) > 0.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::SetObject
//
//	PURPOSE:	Set target object.
//
// ----------------------------------------------------------------------- //

void CAITarget::SetObject(HOBJECT hObject)
{
	// HACK:  for TO2 to make AI shutup after killing someone one co-op.

	if( IsPlayer( m_hObject ) )
	{	
		CCharacter* pPlayer = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
		if( pPlayer->IsDead() )
		{
			GetAI()->MuteAISounds( LTTRUE );
		}
	}

	// END HACK

	m_hObject = hObject;
	if( m_hObject )
	{
		if( IsPlayer( m_hObject ) )
		{
			CCharacter* pPlayer = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
			if( !pPlayer->IsDead() )
			{
				GetAI()->MuteAISounds( LTFALSE );
			}
		}

		// Re-initialize target tracking data.

		m_fTargetTime = g_pLTServer->GetTime();
		g_pLTServer->GetObjectPos( m_hObject, &m_vTargetPosition );
		m_fCurMovementInaccuracy = m_pAI->GetMaxMovementAccuracyPerturb();
		m_vTargetVelocity = LTVector( 0.f, 0.f, 0.f );

		g_pPhysicsLT->GetObjectDims( m_hObject, &m_vTargetDims );
		m_fPhaseStep = ( m_vTargetDims.y * 2.f ) / ( MAX_PHASE + 1 );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::GetShootPosition
//
//	PURPOSE:	Get shoot position, including some random
//				inaccuracy due to target movement.
//
// ----------------------------------------------------------------------- //

void CAITarget::GetShootPosition(LTVector* pvShootPos)
{
	if( !pvShootPos )
	{
		return;
	}

	// Initially aim for the target's visible position.

	*pvShootPos = m_vVisiblePosition;

	// If Target is within the FullAccuracy radius, we are done.

	if( m_fTargetDistSqr < m_pAI->GetFullAccuracyRadiusSqr() )
	{
		return;
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

	LTFLOAT fAccuracy = m_pAI->GetAccuracy();
	if( fAccuracy <= 0.f )
	{
		m_cMisses = 1;
		m_cHits = 0;
	}
	else if( fAccuracy >= 1.f )
	{
		m_cMisses = 0;
		m_cHits = 1;
	}
	else if( fAccuracy < 0.5f )
	{
		m_cMisses = (uint32)( ( ( 1.f - fAccuracy ) / fAccuracy ) + 0.5f );
		m_cHits = 1;
	}
	else {
		m_cMisses = 1;
		m_cHits = (uint32)( ( fAccuracy / ( 1.f - fAccuracy ) ) + 0.5f );
	}

	// If we have met or exceeded the required number of misses, 
	// reset the counters.

	if( m_iMiss >= m_cMisses )
	{
		m_iHit = 0;
		m_iMiss = 0;
	}

	//
	// First take care of hits, then take care of misses.
	//

	// Hit.

	if( m_iHit < m_cHits )
	{
		++m_iHit;

		// If target has started moving or change directions recently,
		// factor in some inaccuracy.

		LTFLOAT fInnaccuracy = Max( 0.f, m_fCurMovementInaccuracy );
		if( fInnaccuracy > 0.f )
		{
			LTVector vShootOffset = LTVector(	GetRandom( -fInnaccuracy, fInnaccuracy ),
												GetRandom( -fInnaccuracy * 0.5f, fInnaccuracy * 0.5f ),
												GetRandom( -fInnaccuracy, fInnaccuracy ) );
			vShootOffset.Normalize();

			*pvShootPos += vShootOffset * 100.0f;
		}
	}

	// Miss.

	else {
		++m_iMiss;

		// Calculate a position to the right or left of the target.

		LTVector vDir = m_vVisiblePosition - m_pAI->GetPosition();
		vDir.Normalize();

		LTVector vRight = vDir.Cross( LTVector( 0.f, 1.f, 0.f ) );

		if( GetRandom( -1.f, 1.f ) > 0.f )
		{
			vRight *= m_pAI->GetAccuracyMissPerturb();
		}
		else {
			vRight *= -m_pAI->GetAccuracyMissPerturb();
		}
		vRight.y = GetRandom( -5.f, 15.f );

		// Apply the offset to miss the target.

		*pvShootPos += vRight;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdateVisibility
//
//	PURPOSE:	Updates the target's visibility
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdateVisibility()
{
	_ASSERT(IsValid());

	// Do not update visibility again until something external says it's OK.
	// AI sets this to TRUE when it has had a sense update.

	if( !m_bCanUpdateVisibility )
	{
		return;
	}
	m_bCanUpdateVisibility = LTFALSE;

	// Do not see dead targets.
	// This is important for AI behavior in multiplayer, where a 
	// dead players stick around so the HOBJECTs do not get NULLed.

	if( IsCharacter( m_hObject ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hObject );
		if( pChar->IsDead() )
		{
			// HACK:  for TO2 to make AI shutup after killing someone one co-op.

			if( IsPlayer( m_hObject ) )
			{
				GetAI()->MuteAISounds( LTTRUE );
			}
			
			// END HACK

			// Invalidate a dead target.

			m_hObject = LTNULL;
			m_hVisionBlocker = LTNULL;
			SetVisibleFromEye( LTFALSE );
			SetVisibleFromWeapon( LTFALSE );
			return;
		}
	}

	LTVector vPosition;
	g_pLTServer->GetObjectPos( m_hObject, &vPosition );

	//
	// Predict the target's next position based on target's observed velocity.
	//

	LTFLOAT fCurTime = g_pLTServer->GetTime();
	LTFLOAT fTimeDelta = fCurTime - m_fTargetTime;
	m_fTargetTime = fCurTime;

	if( fTimeDelta > 0.f )
	{
		LTVector vPredictedPos = m_vTargetPosition + ( m_vTargetVelocity * fTimeDelta);
		LTVector vDiff = vPosition - vPredictedPos;
		
		LTFLOAT fInaccuracyDist = Min( vDiff.Mag(), m_pAI->GetMaxMovementAccuracyPerturb() );
		m_fCurMovementInaccuracy = Max( fInaccuracyDist, m_fCurMovementInaccuracy - ( m_pAI->GetMovementAccuracyPerturbDecay() * fTimeDelta ) );

		m_vTargetVelocity = ( vPosition - m_vTargetPosition ) / fTimeDelta;
	}

	m_vTargetPosition = vPosition;

	// Keep track of the distance to the target.

	m_fTargetDistSqr = m_pAI->GetPosition().DistSqr( m_vTargetPosition );

	// Incrementally scan the target vertically until he can be seen.

	if ( m_nPhase == m_nResetPhase )
	{
		m_vVisiblePosition = vPosition;
	}

	vPosition.y += ( m_vTargetDims.y - ( LTFLOAT(m_nPhase) * m_fPhaseStep ) );

    LTFLOAT fSeeEnemyDistanceSqr = GetAI()->GetSenseRecorder()->GetSenseDistanceSqr(kSense_SeeEnemy);

	LTBOOL bTargetVisibleFromEye = ( GetAI()->GetSenseRecorder()->HasFullStimulation(kSense_SeeEnemy) || 
									 GetAI()->GetSenseRecorder()->HasFullStimulation(kSense_SeeEnemyLean) );
	LTBOOL bTargetVisibleFromWeapon = LTFALSE;
	HOBJECT hVisionBlocker = LTNULL;

	if( bTargetVisibleFromEye )
	{
		// Check visibility from a position dependent on the current weapon.
		// Primary weapon is on the right side, secondary on the left.
		// If no weapon, check from the eyes.

		LTVector vCheckPos;
		if( GetAI()->GetCurrentWeapon() )
		{
			if( ( GetAI()->GetCurrentWeapon() == GetAI()->GetPrimaryWeapon() ) &&
				( !m_bPrimaryOnLeft ) )
			{
				vCheckPos = GetAI()->GetPosition() + ( GetAI()->GetTorsoRight() * GetAI()->GetRadius() );
				vCheckPos.y += GetAI()->GetDims().y * 0.5f;
			}
			else {
				vCheckPos = GetAI()->GetPosition() + ( -GetAI()->GetTorsoRight() * GetAI()->GetRadius() );
				vCheckPos.y += GetAI()->GetDims().y * 0.5f;
			}
		}
		else {
			vCheckPos = GetAI()->GetEyePosition();
		}

		// Check visibility.

		if ( !GetAI()->CanShootThrough() )
		{
			if ( GetAI()->IsObjectPositionVisible(CAI::DefaultFilterFn, NULL, vCheckPos, m_hObject, vPosition, fSeeEnemyDistanceSqr, !GetAI()->IsAlert(), LTFALSE, &hVisionBlocker ) )
			{
				bTargetVisibleFromWeapon = LTTRUE;
			}
		}
		else
		{
			if( GetAI()->IsObjectPositionVisible(CAI::ShootThroughFilterFn, CAI::ShootThroughPolyFilterFn, vCheckPos, m_hObject, vPosition, fSeeEnemyDistanceSqr, !GetAI()->IsAlert(), LTTRUE, &hVisionBlocker ) )
			{
				CAI* pTargetAI = LTNULL;
				if( IsAI( m_hObject ) )
				{
					pTargetAI = (CAI*)g_pLTServer->HandleToObject( m_hObject );
				}

				CAI* pVisionBlocker = LTNULL;
				if( !g_pCharacterMgr->RayIntersectAI( vCheckPos, vPosition, GetAI(), pTargetAI, &pVisionBlocker ) )
				{
					bTargetVisibleFromWeapon = LTTRUE;
				}
				else if( pVisionBlocker )
				{
					hVisionBlocker = pVisionBlocker->m_hObject;
				}
			}

			// Ignore AI that are not currently client-solid.
			// (e.g. AI that are knocked out).

			else if( IsAI( hVisionBlocker ) )
			{
				uint32 dwFlags;
				g_pCommonLT->GetObjectFlags( hVisionBlocker, OFT_User, dwFlags);
				if( ! ( dwFlags & USRFLG_AI_CLIENT_SOLID ) )
				{
					hVisionBlocker = LTNULL;
				}
			}
		}
	}

	if( bTargetVisibleFromWeapon )
	{
		m_vVisiblePosition = vPosition;
	
		m_hVisionBlocker = LTNULL;
		SetVisibleFromEye( bTargetVisibleFromEye );
		SetVisibleFromWeapon( bTargetVisibleFromWeapon );
	}
	else {
		SetVisibleFromEye( bTargetVisibleFromEye );
		SetVisibleFromWeapon( bTargetVisibleFromWeapon );
		m_hVisionBlocker = hVisionBlocker;

		// Only change the phase if target is not visible.

		m_nPhase = (m_nPhase+1) & MAX_PHASE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::UpdatePush
//
//	PURPOSE:	Push the target back if it is too close.
//
// ----------------------------------------------------------------------- //

void CAITarget::UpdatePush( CCharacter* pChar )
{
	_ASSERT(IsValid());

	if( m_fPushSpeed > 0.f )
	{
		// Handle collisions by pushing the target back.

		LTVector vPos = GetAI()->GetPosition();
		if( vPos.DistSqr( m_vVisiblePosition ) < m_fPushMinDistSqr )
		{
			pChar->PushCharacter(vPos, m_fPushMinDist + m_fPushThreshold, 0.f, GetAI()->GetSenseUpdateRate(), m_fPushSpeed );
		}
	}
}


/*virtual*/ int CAITarget::OnRelationChange(HOBJECT hObj)
{
	if( hObj == m_hObject.operator HOBJECT() )
		m_hObject = LTNULL;

	return 0;
}
