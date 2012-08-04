// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCombatOpportunity.cpp
//
// PURPOSE : This sensor handles sensing AICombatOpportunity objects, and 
//			 translating them into CAIWMFacts the AI can reason about.  
//
//			As the AI discovers AINodeCombatOpportunity and 
//			AINodeCombatOpportunityView independently, this sensor only
//			senses AICombatOpportunity objects which have target objects,
//			as these are the only type the AI shoots.
//
//			To the same end, the sensor purges any facts about 
//			AICombatOpportunity objects with NULL targets, as these
//			are obsolete.
//
// CREATED : 6/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorCombatOpportunity.h"
#include "AIStimulusMgr.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"
#include "AI.h"
#include "AICombatOpportunity.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"
#include "AISoundMgr.h"

LINKFROM_MODULE(AISensorCombatOpportunity);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCombatOpportunity, kSensor_CombatOpportunity );

struct BestCombatOpportunity
{
	BestCombatOpportunity(CAI* pAI, HOBJECT hTarget) 
		: m_pBestFact(NULL)
		, m_pAI(pAI)
		, m_hTarget(hTarget)
		, m_nIntersectSegmentCount(0)
		, m_bVisible(false)
	{
	}

	void operator()(CAIWMFact* pFact)
	{
		// Throttle the intersect count allowed per test.  Once this throttle 
		// is exceeded, no facts will be tested.
		//
		// TODO: Iteration can stop at this point.  Should a return statement
		// be added to the CollectFact iteration to allow an early out?  This
		// is the only case this is needed so far, so to avoid complicating
		// the system, it won't be added yet.

		if (1 == m_nIntersectSegmentCount)
		{
			return;
		}

		// Already found a visible combat opportunity.

		if (m_bVisible)
		{
			return;
		}

		// Fact is invalid.

		if (!pFact)
		{
			return;
		}
		
		// Fact is not a CombatOpportunity fact.

		if ( kFact_Knowledge != pFact->GetFactType()
			|| kKnowledge_CombatOpportunity != pFact->GetKnowledgeType())
		{
			return;
		}

		// No target object.  This combat opportunity is only accessible through nodes.

		if (!pFact->GetTargetObject())
		{
			return;
		}

		// Fact is timed out - it was evaluated recently, and failed.

		if ( g_pLTServer->GetTime() < pFact->GetTime() )
		{
			return;
		}

		// Prevent AI from turning around to fire in a different direction
		// than the target that is validating the CombatOp.

		LTVector vTargetOrigin;
		if ( LT_OK != g_pLTServer->GetObjectPos(pFact->GetTargetObject(), &vTargetOrigin))
		{
			return;
		}

		if ( !AIUtil_PositionShootable(m_pAI, vTargetOrigin) )
		{
			return;
		}

		// Failed to convert the handle into a CombatOpportunity object.

		if (!IsKindOf(pFact->GetSourceObject(), "AICombatOpportunity"))
		{
			return;
		}

		AICombatOpportunity* pCombatOpportunity = AICombatOpportunity::HandleToObject(pFact->GetSourceObject());
		if (!pCombatOpportunity)
		{
			return;
		}

		// Is this CombatOpportunity is worth considering?
		// Is the threat in the correct position?

		if ( !pCombatOpportunity->IsValid(m_pAI->GetHOBJECT(), m_hTarget, 
			AICombatOpportunity::kStatusFlag_ThreatPosition | AICombatOpportunity::kStatusFlag_QueryObjectInEnemyArea ) )
		{
			// TODO: Currently we are simply setting a delay time.  Look into 
			// doing something more intelligent for rotating the selection.
			pFact->SetTime(g_pLTServer->GetTime() + 1.0f);
			return;
		}

		// Check to see if the AI can shoot at the RangedTargetObject from 
		// its current position, or if there is an Action node the AI can use
		// to trigger this CombatOpportunity.
		//
		// Prefer checking visibility.  This is required to avoid a later 
		// intersect test.  If the target is visible from the AIs current 
		// position

		bool bCanUse = false;

		if (CombatOpportunityVisible(pCombatOpportunity, vTargetOrigin, pFact))
		{
			m_bVisible = true;
			bCanUse = true;
		}
		else
		{
			CAIWMFact* pCombatOpportunityViewFact = 
				m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, kNode_CombatOpportunityView, kNodeStatus_All ^ kNodeStatus_TargetisCombatOpportunity, NULL, m_hTarget );
			if (!bCanUse && pCombatOpportunityViewFact)
			{
				bCanUse = true;
			}
		}

		if ( !bCanUse )
		{
			// TODO: Currently we are simply setting a delay time.  Look into 
			// doing something more intelligent for rotating the selection.
			pFact->SetTime(g_pLTServer->GetTime() + 1.0f);
			return;
		}

		// This CombatOpportunity may be used.

		// TODO: Determine if there is a way to tell if there are 'better' combat 
		// opportunities.

		m_pBestFact = pFact;
	}


	bool CombatOpportunityVisible(AICombatOpportunity* pCombatOpportunity, const LTVector& vTargetOrigin, const CAIWMFact* const pFact)
	{
		// AI is not inside a valid space to use this combat opportunity. Or is the
		// AI at a view node?  If either of these are true, the AI may attack from 
		// this position.

		SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp(kWSK_AtNodeType, m_pAI->GetHOBJECT());

		bool bAIPositionValid = false;
		if (pProp && pProp->eAINodeTypeWSValue == kNode_CombatOpportunityView)
		{
			bAIPositionValid = true;
		}

		if (!bAIPositionValid)
		{
			if (pCombatOpportunity->IsValid(m_pAI->GetHOBJECT(), m_hTarget, AICombatOpportunity::kStatusFlag_AIPosition))
			{
				bAIPositionValid = true;
			}
		}

		if (!bAIPositionValid)
		{
			return false;
		}

		// Is this TargetObject in range?

		float flViewToTargetDistSqr = vTargetOrigin.DistSqr(m_pAI->GetPosition());
		bool bInRange = false;
		for (int iWeaponType = 0; iWeaponType != kAIWeaponType_Count; ++iWeaponType)
		{
			float flWeaponRangeSqr = AIWeaponUtils::GetWeaponRange(m_pAI, (ENUM_AIWeaponType)iWeaponType, true);
			flWeaponRangeSqr *= flWeaponRangeSqr;

			if (flViewToTargetDistSqr < flWeaponRangeSqr)
			{
				bInRange = true;
				break;
			}
		}
		if (!bInRange)
		{
			return false;
		}

		// Is the CombatOpportunity objects target object visible?

		LTVector vTargetDims;
		if (LT_OK != g_pPhysicsLT->GetObjectDims( pFact->GetTargetObject(), &vTargetDims ))
		{
			return false;
		}

		float fTargetRadius = vTargetDims.Mag();;
		float fSeeEnemyDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fTargetRadius;
		fSeeEnemyDistanceSqr *= fSeeEnemyDistanceSqr;

		++m_nIntersectSegmentCount;

		bool bIsAlert = ( m_pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert ) ||
						( m_pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Suspicious );
		if ( !m_pAI->IsObjectPositionVisible(
			CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), 
			pFact->GetTargetObject(), vTargetOrigin,
			fSeeEnemyDistanceSqr, !bIsAlert, false ) )
		{
			return false;
		}

		// AI is in range and can see the AICombatOpportunity.

		return true;
	}


	CAIWMFact*	m_pBestFact;
	CAI*		m_pAI;
	HOBJECT		m_hTarget;
	int 		m_nIntersectSegmentCount;
	bool		m_bVisible;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCombatOpportunity::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorCombatOpportunity::CAISensorCombatOpportunity()
{
}

CAISensorCombatOpportunity::~CAISensorCombatOpportunity()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorCombatOpportunity::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorCombatOpportunity
//              
//----------------------------------------------------------------------------

void CAISensorCombatOpportunity::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAISensorCombatOpportunity::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCombatOpportunity::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorCombatOpportunity::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Don't call down (base class does nothing, and returns false);

	// Sanity checks.

	if( !m_pAI )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: AI is NULL." );
		return false;
	}

	if( !m_pSensorRecord )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: SensorRecord is NULL." );
		return false;
	}

	if( !pStimulusRecord )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: StimulusRecord is NULL." );
		return false;
	}

	// Template specifies accepted stimuli.

	if( !( m_pSensorRecord->dwStimulusTypes & pStimulusRecord->m_eStimulusType ) )
	{
		return false;
	}

	// AICombatOpportunity does not have a target.  Without a target, there 
	// is no need to consider shooting at it, as this AICombatOpportunity can
	// only be handled through AINodes.

	if ( !pStimulusRecord->m_hStimulusTarget )
	{
		return false;
	}

	// Determine if this stimulus is already in the AIs working memory.  If it is, 
	// there is no need to add it.  If it is not, and if the AI is in the radius
	// of the 

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_CombatOpportunity );
	factQuery.SetSourceObject( pStimulusRecord->m_hStimulusSource );
	if (m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery ))
	{
		return false;
	}

	// The AI has discovered a CombatOpportunity!  Create fact about this 
	// opportunity, so that the AI may exploit it when possible.

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact(kFact_Knowledge);
	pFact->SetKnowledgeType( kKnowledge_CombatOpportunity );
	pFact->SetTime( g_pLTServer->GetTime() );
	pFact->SetSourceObject( pStimulusRecord->m_hStimulusSource );
	pFact->SetTargetObject( pStimulusRecord->m_hStimulusTarget );

	return true;
}

bool CAISensorCombatOpportunity::UpdateSensor()
{
	if (!super::UpdateSensor())
	{
		return false;
	}

	// Bail if we have targeted a combat opportunity too recently.

	if( g_pAIDB->GetAIConstantsRecord()->fCombatOpportunityFrequency > 0.f )
	{
		CAIWMFact factTimeoutQuery;
		factTimeoutQuery.SetFactType( kFact_Knowledge );
		factTimeoutQuery.SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime );
		CAIWMFact* pTimeoutFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
		if( pTimeoutFact && ( pTimeoutFact->GetTime() > g_pLTServer->GetTime() ) )
		{
			return false;
		}
	}

	// Clear out CAIWMFact instances with NULL target objects, as these facts
	// serve no purpose (kKnowledge_CombatOpportunity facts are used for 
	// shooting at an AICombatOpportunity objects' target.  If there is no 
	// target, there is nothing to shoot at, and the fact isn't needed).

	CAIWMFact factClearQuery;
	factClearQuery.SetFactType( kFact_Knowledge );
	factClearQuery.SetKnowledgeType( kKnowledge_CombatOpportunity );
	factClearQuery.SetTargetObject( NULL );
	m_pAI->GetAIWorkingMemory()->ClearWMFact( factClearQuery );

	//
	//-----------------------------------------------------------------------
	//

	// Currently using a combat opportunity node.  No need to search for one, 
	// as this process is intended for raising the target reevaluation flag.
	//
	// TODO: The above is only partly true.  For an AI to go from one 
	// CombatOpportunity to another, the AI needs to run this code 
	// continuously in case his current CombatOpportunity fails.  This 
	// greatly complicates the above code however, as it requires inserting
	// tests to insure that an AI doesn't invalidate his current 
	// CombatOpportunity.  As this may not be needed, the 'simple' approach
	// is applied for now.

	// Clear out any old UsableCombatOpportunity facts.
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_UsableCombatOpportunity );
	m_pAI->GetAIWorkingMemory()->ClearWMFact(factQuery);

	if ( kTarget_CombatOpportunity == m_pAI->GetAIBlackBoard()->GetBBTargetType() )
	{
		return false;
	}

	// Bail if we are targeting a character that we cannot see.

	if( ( kTarget_Character == m_pAI->GetAIBlackBoard()->GetBBTargetType() ) &&
		( ( !m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ) ||
		  ( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() == 0.f ) ||
		  ( !( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Normal ) ) ) )
	{
		return false;
	}


	// Look for a CombatOpportunity which may be used right now.  If one is
	// found, add a fact about it to working memory, and flag the AIs' target
	// for reevaluation.

	// Determine which type of object is currently being targeted.  Look for a 
	// combat opportunity that attacks this object.

	HOBJECT hCombatOpportunityTarget = NULL;
	switch(m_pAI->GetAIBlackBoard()->GetBBTargetType())
	{
	case kTarget_Character:
	case kTarget_Disturbance:
		{
			hCombatOpportunityTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		}
		break;

	case kTarget_CombatOpportunity:
		hCombatOpportunityTarget = m_pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget();
		break;
	}

	// No object to use the CombatOpportunity against.

	if (hCombatOpportunityTarget)
	{
		// HACK: Record who is being targeted as the object to use the combat 
		// opportunity against.  This is needed BEFORE calling the nodes 
		// IsNodeValid.

		m_pAI->GetAIBlackBoard()->SetBBCombatOpportunityTarget(hCombatOpportunityTarget);

		// Failed to find a CombatOpportunity relevant to fighting this character.

		BestCombatOpportunity CombatOpportunity(m_pAI, hCombatOpportunityTarget);
		m_pAI->GetAIWorkingMemory()->CollectFact(CombatOpportunity);
		if ( CombatOpportunity.m_pBestFact )
		{
			// If a combat opportunity was found, flag it for evaluation.
			// See if this fact is already registered.  If it is, update it.  If not,
			// create one.

			CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact(kFact_Knowledge);
			if (pFact)
			{
				pFact->SetKnowledgeType(kKnowledge_UsableCombatOpportunity);
				pFact->SetSourceObject(CombatOpportunity.m_pBestFact->GetSourceObject());
				pFact->SetTargetObject(hCombatOpportunityTarget);
			}

			// If the current target is not a CombatOpportunity, request a 
			// reevaluation of the target.  
			//
			// NOTE:  If there is a higher priority TargetType, this may happen 
			// every round.  If this is an issue, consider a smarter way of handling
			// this.  This is a workaround for the fact that the target typically 
			// isn't reevaluated unless the current target is lost.

			if ( kTarget_CombatOpportunity != m_pAI->GetAIBlackBoard()->GetBBTargetType() )
			{
				m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget(true);

				// Ally announced combat opportunity if possible.
				// e.g. "Shoot the power box!"

				HOBJECT hCombatOp = CombatOpportunity.m_pBestFact->GetSourceObject();
				if( IsAICombatOpportunity( hCombatOp ) )
				{
					AICombatOpportunity* pCombatOp = (AICombatOpportunity*)g_pLTServer->HandleToObject( hCombatOp );
					if( pCombatOp && ( pCombatOp->GetAllySound() != kAIS_InvalidType ) )
					{
						HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
						HOBJECT hAlly = g_pAICoordinator->FindAlly( m_pAI->m_hObject, hTarget );
						if( hAlly )
						{
							g_pAISoundMgr->RequestAISound( hAlly, pCombatOp->GetAllySound(), kAISndCat_Event, hTarget, 0.f );
							pCombatOp->SetAllySpeaker( hAlly );

							// Give the ally some time to start speaking.

							if( !pCombatOp->IsCombatOpportunityLocked() )
							{
								pCombatOp->LockCombatOpportunity( m_pAI->m_hObject );
								SetCombatOpportunityTimeout( 1.f );
							}
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCombatOpportunity::SetCombatOpportunityTimeout
//
//	PURPOSE:	Set timeout before AI can target combat opportunity.
//
// ----------------------------------------------------------------------- //

void CAISensorCombatOpportunity::SetCombatOpportunityTimeout( float fDelay )
{
	// Do not allow AI to target a combat opportunity for some time.

	CAIWMFact factTimeoutQuery;
	factTimeoutQuery.SetFactType( kFact_Knowledge );
	factTimeoutQuery.SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factTimeoutQuery );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
	}
	if( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_NextCombatOpportunityTargetTime, 1.f );
		pFact->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );
	}
}
