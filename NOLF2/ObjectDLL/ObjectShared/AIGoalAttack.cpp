// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttack.cpp
//
// PURPOSE : AIGoalAttack implementation
//
// CREATED : 7/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttack.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AINode.h"
#include "AITarget.h"
#include "AnimatorPlayer.h"
#include "AIGoalButeMgr.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttack, kGoal_Attack);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttack::CAIGoalAttack()
{
	m_eWeaponType = kAIWeap_Ranged;

	m_bPanicCanActivate = LTFALSE;
	m_ePosture = kAP_Stand;
	m_fChaseDelay = 1.0f;

	m_bNodeBased	= LTFALSE;
	m_bForceSearch	= LTFALSE;
	m_hNode	= NULL;
	m_hFailedNode = NULL;

	m_bCheckForWeaponChange = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_eWeaponType );
	SAVE_FLOAT(	m_fChaseDelay );
	SAVE_DWORD( m_ePosture );
	SAVE_BOOL( m_bPanicCanActivate );
	SAVE_BOOL( m_bCheckForWeaponChange );

	SAVE_BOOL( m_bNodeBased );
	SAVE_BOOL( m_bForceSearch );
	SAVE_HOBJECT( m_hFailedNode );
	SAVE_HOBJECT( m_hNode );
}

void CAIGoalAttack::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_eWeaponType, EnumAIWeaponType );
	LOAD_FLOAT(	m_fChaseDelay );
	LOAD_DWORD_CAST( m_ePosture, EnumAnimProp );
	LOAD_BOOL( m_bPanicCanActivate );
	LOAD_BOOL( m_bCheckForWeaponChange );

	LOAD_BOOL( m_bNodeBased );
	LOAD_BOOL( m_bForceSearch );
	LOAD_HOBJECT( m_hFailedNode );
	LOAD_HOBJECT( m_hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	// Attack.

	SetStateAttack();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::DeactivateGoal()
{
	super::DeactivateGoal();
	m_hNode = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::SetStateAttack
//
//	PURPOSE:	Set Attack state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::SetStateAttack()
{
	m_pAI->SetCurrentWeapon( m_eWeaponType );

	if(m_pAI->GetState()->GetStateType() != kState_HumanAttack)
	{
		m_pAI->SetState( kState_HumanAttack );

		CAIHumanStateAttack* pAttackState = (CAIHumanStateAttack*)m_pAI->GetState();
		pAttackState->SetChaseDelay(m_fChaseDelay);
		pAttackState->SetPosture(m_ePosture);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::SetStatePanic
//
//	PURPOSE:	Setup Panic state.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::SetStatePanic()
{
	// Ignore most senses
	m_pAI->SetCurSenseFlags( kSense_SeeCatchableProjectile );

	m_pAI->SetState( kState_HumanPanic );

	CAIHumanStatePanic* pPanicState = (CAIHumanStatePanic*)m_pAI->GetState();
	pPanicState->SetCanActivate(m_bPanicCanActivate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAttack:
			HandleStateAttack();
			break;

		case kState_HumanFlee:
			HandleStateFlee();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		case kState_HumanHolster:
			HandleStateHolster();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttack::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleStateAttack
//
//	PURPOSE:	Determine what to do when in state Attack.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::HandleStateAttack()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Flee:
			{
				LTVector vScatterPos;
				CAIHumanStateAttack* pStateAttack = (CAIHumanStateAttack*)m_pAI->GetState();
				vScatterPos = pStateAttack->GetScatterPosition();

				CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

				HOBJECT hDanger;
				if( LT_OK == FindNamedObject( pAIHuman->GetBrain()->GetDodgeProjectileName(), hDanger ) )
				{
					m_pAI->SetState( kState_HumanFlee );
					CAIHumanStateFlee* pStateFlee = (CAIHumanStateFlee*)m_pAI->GetState();
					pStateFlee->SetDest(vScatterPos);
					pStateFlee->SetDanger(hDanger);
				}
			}
			break;

		case kSStat_FailedComplete:

			// Panic if no weapon.
			if(m_pAI->GetCurrentWeapon())
			{
				// Reset AIs default senses, from aibutes.txt.
				m_pAI->ResetBaseSenseFlags();

				m_pAI->SetState( kState_HumanAware );
			}
			else {
				SetStatePanic();
			}
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttack::HandleStateAttack: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleStateFlee
//
//	PURPOSE:	Determine what to do when in state Flee.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::HandleStateFlee()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			SetStateAttack();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttack::HandleStateFlee: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleStatePanic
//
//	PURPOSE:	Determine what to do when in state Panic.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttack::HandleStatePanic()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalAttack::HandleStatePanic: Unexpected State Status.");
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAttack::HandleStateHolster()
//              
//	PURPOSE:	Holstering is supported here as part of weapon changing.
//              
//----------------------------------------------------------------------------
void CAIGoalAttack::HandleStateHolster()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::HandleStateHolster: Failed to holster weapon." );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			{
				// Currently, the only want to be Holstering from within the 
				// attack goal is if we are doing a weapon change.  This may
				// not always be true -- as such, it is not really an error
				// condition at this point.
				if ( GetWeaponChangeDescription() )
				{
					CAIHuman* pAIHuman = dynamic_cast<CAIHuman*>(m_pAI);
					AIASSERT(pAIHuman, NULL, "Failed to retreive CAIHuman from CAI");
					if ( pAIHuman )
					{
						pAIHuman->SetHolsterString( GetWeaponChangeDescription() );
					}
				}

				m_pAI->SetState( kState_HumanDraw );
			}
			break;

		// Unexpected StateStatus.
		default:
			AIASSERT(0, m_pAI->m_hObject, "CAIGoalAbstractUseObject::HandleStateHolster: Unexpected State Status.");
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangeWeapons::HandleStateDraw()
//              
//	PURPOSE:	Drawing is supported here as part of weapon changing.
//              
//----------------------------------------------------------------------------
void CAIGoalAttack::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			AIASSERT(0, m_pAI->m_hObject, "AI Failed to complete HumanDraw" );
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal( this );
			SetStateAttack();
			break;

		// Unexpected StateStatus.
		default:
			AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::HandleStateDraw: Unexpected State Status.");
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangeWeapons::ActivateChangeWeapons()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalAttack::ActivateChangeWeapons()
{
	// Ignore senses other than SeeEnemy.
	if(m_pAI->GetCurrentWeapon())
	{
		m_pAI->SetState( kState_HumanHolster );
	}
	else
	{
		m_pAI->SetState( kState_HumanDraw );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //
AINode* CAIGoalAttack::HandleGoalAttractors()
{
	AINode* pNode = LTNULL;

	if( ( m_bNodeBased == LTTRUE ) && 
		( ( !m_pGoalMgr->IsCurGoal( this ) ) || m_bForceSearch ) )
	{
			AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
			AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalAttack::HandleGoalAttractors: Goal has no attractors.");

			// Block all of the invalid nodes in the list.  
			AutoNodeBlocker Blocker( m_pAI->m_hObject, m_pAI->GetInvalidNodeList() );

			// Bail if we do not have the right type of weapon.

			if( !m_pAI->HasWeapon( m_eWeaponType ) )
			{
				return LTNULL;
			}

			// If this goal has sense triggers, and nothing
			// has been sensed, exit the function.

			if( ( pTemplate->flagSenseTriggers != kSense_None ) &&
				( ( m_hStimulusSource == LTNULL ) ||
				  ( !m_pAI->GetSenseRecorder()->HasAnyStimulation( kSense_All ) ) ) )
			{
				return LTNULL;
			}

			// Lock the failed cover node, so that we don't try to use it again.
			BlockAttractorNodeFromSearch( m_hFailedNode );

			// Look for a cover node that covers from the threat.
			// If one is found, this goal activates.
			for(uint32 iAttractor=0; iAttractor < pTemplate->cAttractors; ++iAttractor)
			{
				pNode = FindNearestAttractorNode(pTemplate->aAttractors[iAttractor], m_pAI->GetPosition(), pTemplate->fAttractorDistSqr * m_fBaseImportance);
				if(pNode != LTNULL)	
				{
					if ( !IsPathToNodeValid(pNode) )
					{
						break;
					}
					
					if( m_hStimulusSource )
					{
						m_pAI->Target(m_hStimulusSource);
					}

					AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Setting node: %s", ::ToString( pNode->GetName() ) ) );

					m_hNode = pNode->m_hObject;
					SetCurToBaseImportance();
					m_hStimulusSource = LTNULL;
					break;
				}
				else if( !m_bForceSearch ) {
					m_hNode = LTNULL;
					m_fCurImportance = 0.f;
				}
			}

			// If we locked a node prior to the search, unlock it.
			UnblockAttractorNodeFromSearch( m_hFailedNode );
		}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //
AINode* CAIGoalAttack::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttack::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// This goal intentionally gets re-triggered.  When enemy is not
	// in sight, goal loses importance.

	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Check range.
		// Need a weapon to activate an attack goal.
		// Target must be visible.
		// AI is not already in this attack goal.
		m_pAI->Target(pSenseRecord->hLastStimulusSource);
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( pAIHuman->HasTarget() 
			&& pAIHuman->HasWeapon( m_eWeaponType )
			&& ( pAIHuman->GetBrain()->GetRangeStatus( m_eWeaponType ) == eRangeStatusOk )
			&& ( ( pAIHuman->GetTarget()->IsVisibleCompletely() ) ||
				 ( IsAI( pAIHuman->GetTarget()->GetVisionBlocker() ) && ( m_pAI->GetBrain()->GetAttackPoseCrouchChance() > 0.f ) ) ) )
		{
			if(m_bNodeBased == LTTRUE)
			{
				if(m_hNode)
				{
					return LTTRUE;
				}
			}
			else
			{
				return LTTRUE;
			}
		}
		else {
		
			// Clear whatever the superclass set, since requirements are not met.
			
			m_hStimulusSource = LTNULL;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttack::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "CHASEDELAY") )
	{
		m_fChaseDelay = (LTFLOAT)atof(szValue);
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "POSTURE") )
	{
		if ( !_stricmp(szValue, "CROUCH") )
		{
			m_ePosture = kAP_Crouch;
			return LTTRUE;
		}
		else if ( !_stricmp(szValue, "STAND") )
		{
			m_ePosture = kAP_Stand;
			return LTTRUE;
		}
	}
	else if ( !_stricmp(szName, "PANICCANACTIVATE") )
	{
		m_bPanicCanActivate = IsTrueChar(*szValue);
		return LTTRUE;
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAttack::HandlePotentialWeaponChange()
//              
//	PURPOSE:	Potentially does a state change to WeaponChange, if the 
//				AIs conditions match the nodes entry conditions.
//              
//----------------------------------------------------------------------------
void CAIGoalAttack::HandlePotentialWeaponChange()
{
	if ( m_bCheckForWeaponChange == LTTRUE )
	{
		m_bCheckForWeaponChange = LTFALSE;

		CAIHuman* pAIHuman = dynamic_cast<CAIHuman*>(m_pAI);
		AIASSERT( pAIHuman, m_pAI->GetHOBJECT(), "Unable to get AIHuman from AI" );
		if ( GetNodeAsChangeWeapons()->ShouldChangeWeapons(pAIHuman) )
		{
			ActivateChangeWeapons();
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAttack::GetNodeAsChangeWeapon()
//              
//	PURPOSE:	Returns the m_hNode as a ChangeWeapons node, or asserts if it fails	
//              
//----------------------------------------------------------------------------
AINodeChangeWeapons* CAIGoalAttack::GetNodeAsChangeWeapons() const
{
	AIASSERT( m_hNode, m_pAI->GetHOBJECT(), "AI in attack from cover without a valid m_hMode" );
	AINodeChangeWeapons* pNode = dynamic_cast<AINodeChangeWeapons*>(g_pLTServer->HandleToObject( m_hNode ));
	AIASSERT( pNode, m_pAI->m_hObject, "Unexpected node type" );

	return ( pNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAttack::GetWeaponChangeDescription()
//              
//	PURPOSE:	returns the weapon string specified by the node.
//              
//----------------------------------------------------------------------------
const char* const CAIGoalAttack::GetWeaponChangeDescription() const
{
	if ( !GetNodeAsChangeWeapons() )
	{
		return NULL;
	}

	CAIHuman* pAIHuman = dynamic_cast<CAIHuman*>(m_pAI);
	AIASSERT( pAIHuman, m_pAI->GetHOBJECT(), "Unable to get AIHuman from AI" );

	return ( GetNodeAsChangeWeapons()->GetWeaponString(pAIHuman) );
}
