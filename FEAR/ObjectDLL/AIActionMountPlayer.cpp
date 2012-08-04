// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountPlayer.cpp
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMountPlayer.h"
#include "AIDB.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIPathMgrNavMesh.h"
#include "AIBlackBoard.h"
#include "PlayerObj.h"

LINKFROM_MODULE(AIActionMountPlayer);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountPlayer, kAct_MountPlayer );

static int GetKickCount( CAIDB* pAIDB, CAI* pAI )
{
	HRECORD hMisc = pAIDB->GetMiscRecord();
	
	if ( NULL == hMisc )
	{
		return 0;
	}

	// Look for an override specialized for this model.

	HATTRIBUTE hAtt = pAIDB->GetAttribute( hMisc, "BerserkerKickCountOverride" );
	for ( uint32 iEachOverride = 0; ; ++iEachOverride )
	{
		HATTRIBUTE hModelTypeAtt = pAIDB->GetStructAttribute( hAtt, iEachOverride, "Model" );

		// End of the list; no match found.

		if ( NULL == hModelTypeAtt )
		{
			break;
		}

		// Not a match.

		if ( pAI->GetModel() != pAIDB->GetRecordLink( hModelTypeAtt ) )
		{
			continue;
		}

		// Found a match!  Return the kick count.

		HATTRIBUTE hKickCountAtt = pAIDB->GetStructAttribute( hAtt, iEachOverride, "KickCount" );
		return pAIDB->GetInt32( hKickCountAtt );
	}

	// Get the default.

	return pAIDB->GetInt32( hMisc, "BerserkerKickCountDefault" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionMountPlayer::CAIActionMountPlayer()
{
}

CAIActionMountPlayer::~CAIActionMountPlayer()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNodeMounted::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionMountPlayer::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	//
	// AI has mounted the object.
	//
	// AI is using the object specified in the parents worldstate's 
	// kWSK_UsingObject field.

	m_wsWorldStateEffects.SetWSProp( kWSK_MountedObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionMountPlayer::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning  )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	if ( bIsPlanning )
	{
		// Fail if the targeted object is not a player or if it is not the 
		// currently targeted object.

		SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject );
		if ( !pProp 
			|| !IsPlayer( pProp->hWSValue ) 
			|| pProp->hWSValue != pAI->GetAIBlackBoard()->GetBBTargetObject() )
		{
			return false;
		}

		// Fail if we fail to get the player object.

		CPlayerObj* pPlayer = CPlayerObj::DynamicCast( pProp->hWSValue );
		if ( !pPlayer )
		{
			return false;
		}

		// Fail if the AI is armed.

		if ( AIWeaponUtils::HasWeaponType( pAI,  kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) 
			|| AIWeaponUtils::HasWeaponType( pAI,  kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER ) )
		{
			return false;
		}

		// Perform all tests ignoring Y.

		LTVector vAIPos2D = pAI->GetPosition();
		vAIPos2D.y = 0.0f;

		LTVector vPlayerPos2D;
		g_pLTServer->GetObjectPos( pAI->GetAIBlackBoard()->GetBBTargetObject(), &vPlayerPos2D );
		vPlayerPos2D.y = 0;

		LTVector vDirNorm2D = vPlayerPos2D - vAIPos2D;
		if ( vDirNorm2D == LTVector::GetIdentity() )
		{
			return false;
		}
		vDirNorm2D.Normalize();

		float flDistance2D = vAIPos2D.Dist( vPlayerPos2D );

		// Fail if the AI is running a path and the direction to the berserker 
		// target is significantly different than the path.
		// Only apply this test if the AI is at least some minimum distance 
		// away from the player.

		if ( flDistance2D > 100 )
		{
			LTVector vForwardNorm2D = pAI->GetAIMovement()->GetForward();
			vForwardNorm2D.y = 0.f;
			vForwardNorm2D.Normalize();

			float fHorizontalDp = vDirNorm2D.Dot( vForwardNorm2D );
			if ( fHorizontalDp < 0.98f ) 
			{
				return false;
			}
		}

		// Fail if there is not smartobject record for this action.

		AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
		if( !pSmartObjectRecord )
		{
			return false;
		}

		// Fail if the targeted object is too far away.

		if ( flDistance2D > pSmartObjectRecord->fMaxDist )
		{
			return false;
		}

		// No straight path to the target (geometry may obstruct the mounting, so don't do it).

		float flLungeDistance = LTMAX( flDistance2D, flDistance2D - ( pAI->GetRadius() + pPlayer->GetRadius() ) ); 
		LTVector vDestPosition = pAI->GetPosition() + vDirNorm2D * flLungeDistance;
		if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDestPosition, pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly(), pAI->GetRadius() ) )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountPlayer::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Fail if there is not smartobject record for this action.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionMountPlayer::ActivateAction : No SmartObjectRecord specified.");
	if ( !pSmartObjectRecord )
	{
		return;
	}

	// Set the state to animate, and configure the state.

	pAI->SetState( kState_Animate );
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( pSmartObjectRecord->Props, !LOOP );

	// Turn on touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_Notify );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountPlayer::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Turn off touch handling

	pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_None );

	// Clear the locked animation.  If this isn't done and the AI goes 
	// into idle, the AI will loop the mount player animation.

	pAI->GetAnimationContext()->ClearLock();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionMountPlayer::IsActionComplete( CAI* pAI )
{
	// Mounting is complete when the targeted player is touched this frame.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Touch );
	factQuery.SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	factQuery.SetTime( g_pLTServer->GetTime() );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	if ( pFact )
	{
		return true;
	}

	// AI has not yet touched its enemy.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::ValidateAction
//
//	PURPOSE:	Returns true if AI should keep attempting to mount the 
//				player.
//
// ----------------------------------------------------------------------- //

bool CAIActionMountPlayer::ValidateAction( CAI* pAI )
{
	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	// If the animation is done, the AI failed to mount the player.  Return 
	// false.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return false;
	}

	// Mounting attempt has not yet failed.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionMountPlayer::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	// Set the player as the UsingObject.  This MUST be kept in sync with the 
	// mounted flag.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_UsingObject, pAI->GetHOBJECT() );
	AIASSERT(pProp, pAI->GetHOBJECT(), "CAIActionMountPlayer::ApplyContextEffect : No UsingObject prop specified in the goal state.  Verify that this property is tested in the goal state" );

	// Lock the player in place.
	
	CPlayerObj* pPlayer = CPlayerObj::DynamicCast( pProp->hWSValue );
	if ( pPlayer )
	{
		pPlayer->BerserkerAttack( pAI->GetHOBJECT() );

		// Get the number of kicks required to knock the AI off.

		int iKicksToKnockOffAI = GetKickCount( g_pAIDB, pAI );

		// Remember who is being held in place, so that we can insure release on
		// deactivation (in case the target changes).  Note that we do not need to 
		// check for an existing fact here, as this class and only this class 
		// creates and destroys instances on activate/deactivation.

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );

		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
		if ( NULL == pFact )
		{
			pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
			pFact->SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
		}
		if ( pFact )
		{
			pFact->SetTargetObject( pProp->hWSValue );
			pFact->SetIndex( iKicksToKnockOffAI ); 
		}

		AITRACE( AIShowBerserker, ( pAI->GetHOBJECT(), "(Mounted) Kick count: %d", iKicksToKnockOffAI ) );
	}
}

