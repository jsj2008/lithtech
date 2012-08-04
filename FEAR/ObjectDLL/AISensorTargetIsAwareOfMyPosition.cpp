// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAwareOfMyPosition.cpp
//
// PURPOSE : 
//
// CREATED : 5/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTargetIsAwareOfMyPosition.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAwareOfMyPosition, kSensor_TargetIsAwareOfMyPosition );

static bool s_bInitializedConstants				= false;
static const float s_flStimulationMin			= 0.f;
static const float s_flStimulationMax			= 1.f;
static float s_flThreshhold						= c_fFOV60;
static float s_flStimulationIncreaseRate		= 1.0f;
static float s_flStimulationDecreaseRate		= 0.05f;

static void InitConstants()
{
	if ( !s_bInitializedConstants )
	{
		s_flThreshhold					= FOV2DP( g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionFOV" ) );
		s_flStimulationIncreaseRate		= g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionSimulationIncreaseRate" );
		s_flStimulationDecreaseRate		= g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionSimulationDecreaseRate" );

		s_bInitializedConstants = true;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAwareOfMyPosition::Cons/descructor()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAISensorTargetIsAwareOfMyPosition::CAISensorTargetIsAwareOfMyPosition() : 
	m_flLastUpdateTime(0.f)
{
	InitConstants();
}

CAISensorTargetIsAwareOfMyPosition::~CAISensorTargetIsAwareOfMyPosition()
{
}


void CAISensorTargetIsAwareOfMyPosition::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_TIME(m_flLastUpdateTime);
}

void CAISensorTargetIsAwareOfMyPosition::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_TIME(m_flLastUpdateTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAwareOfMyPosition::UpdateSensor()
//
//	PURPOSE:	Returns true if the AI has a target and if the target is
//				looking at the AI.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAwareOfMyPosition::UpdateSensor()
{
	// Find or create a fact about an enemy knowing the AIs position.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_EnemyKnowsPosition );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if (!pFact)
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact(kFact_Knowledge);
		if (pFact)
		{
			// The AI does not initially know the AIs position.

			pFact->SetKnowledgeType( kKnowledge_EnemyKnowsPosition, 0.f );
		}
	}

	if (!pFact)
	{
		return false;
	}

	// Always update the confidence in the knowledge that the enemy knows there 
	// the AI is.

	float fConfidence = pFact->GetConfidence(CAIWMFact::kFactMask_KnowledgeType);
	float fTimeDelta = (float)(g_pLTServer->GetTime() - m_flLastUpdateTime);

	if (m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye()
		&& TargetIsLookingAtMe())
	{
		// Increase the confidence in being seen

		fConfidence += s_flStimulationIncreaseRate*fTimeDelta;
	}
	else
	{
		// Decrease the confidence in being seen

		fConfidence -= s_flStimulationDecreaseRate*fTimeDelta;
	}

	// Update the confidence in the knowledge that the target knows your position.

	fConfidence = LTCLAMP(fConfidence, s_flStimulationMin, s_flStimulationMax);
	pFact->SetConfidence(CAIWMFact::kFactMask_KnowledgeType, fConfidence);

	m_flLastUpdateTime = g_pLTServer->GetTime();

	// Support debugging this info.

	AITRACE( AITargetIsAwareOfMyPosition, ( m_pAI->m_hObject, "TargetIsAwareOfMyPosition: %f", fConfidence ) );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::TargetIsLookingAtMe()
//
//	PURPOSE:	Returns true if the AI has a target and if the target is
//				looking at the AI.
//
//	TODO:		This is currently a near copy and pasted from 
//				AISensorIsTargetAimingAtMe.  This code will diverge 
//				significantly once the behavior for this sensor is further
//				defined.  For this reason, this functionality is not 
//				related to the AISensorIsTargetAimingAtMe.  If they 
//				converge, move the functionality to a utility function 
//				somewhere.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAwareOfMyPosition::TargetIsLookingAtMe()
{
	// No character target.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	// Target is not completely visible.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Character);
	factQuery.SetTargetObject(hTarget);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) >= 1.f ) ) )
	{
		return false;
	}

	// Target does not exist.

	CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
	if( !pCharacter )
	{
		return false;
	}

	LTRigidTransform tfView;
	pCharacter->GetViewTransform( tfView );

	LTVector vForward = tfView.m_rRot.Forward();

	LTVector vPos;
	g_pLTServer->GetObjectPos( pCharacter->m_hObject, &vPos);

	LTVector vDir;
	vDir = m_pAI->GetPosition() - vPos;
	vDir.y = 0.f;
	if( vDir != LTVector::GetIdentity() )
	{
		vDir.Normalize();
	}

	if ( vDir.Dot(vForward) > s_flThreshhold )
	{
		return true;
	}

	return false;
}
