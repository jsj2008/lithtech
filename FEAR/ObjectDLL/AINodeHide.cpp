// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeHide.cpp
//
// PURPOSE : Implementation of the Hide node.  This node defines a location 
//			for an AI to hide from its target at.
//
// CREATED : 4/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeHide.h"
#include "DEditColors.h"
#include "AIWorldState.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIWorkingMemoryCentral.h"

LINKFROM_MODULE( AINodeHide );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINodeHide 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINodeHide CF_HIDDEN

#endif

BEGIN_CLASS(AINodeHide)

	ADD_DEDIT_COLOR( AINodeHide )

	// Hide many of the SmartObject properties.

	ADD_STRINGPROP_FLAG(Object,					"",				PF_HIDDEN|PF_OBJECTLINK, "The name of the object that the AI is to use.")
	ADD_COMMANDPROP_FLAG(PreActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_COMMANDPROP_FLAG(PostActivateCommand,	"",				PF_HIDDEN|PF_NOTIFYCHANGE, "TODO:PROPDESC")
	ADD_BOOLPROP_FLAG(OneWay,					false,			PF_HIDDEN, "TODO:PROPDESC")

	// Add Hide properties.
	
	THREATRADIUS_PROPS( 256.0f )
	BOUNDARYRADIUS_PROPS( 1024.0f )
	THREATFOV_PROPS()
	ADD_REALPROP_FLAG(MinExpiration,			10.0f,			0, "Minimum time AI stays at Hide node. [Seconds]")
	ADD_REALPROP_FLAG(MaxExpiration,			45.0f,			0, "Maximum time AI stays at Hide node. [Seconds]")

	// Override the base class version:

	ADD_STRINGPROP_FLAG(SmartObject,			"Hide", 		0|PF_STATICLIST|PF_DIMS, "TODO:PROPDESC")

END_CLASS_FLAGS_PLUGIN(AINodeHide, AINodeSmartObject, CF_HIDDEN_AINodeHide, AINodeHidePlugin, "The AI runs here, and hopes to remain unseen by the player until the player is closer and has their back to the AI")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeHide)
CMDMGR_END_REGISTER_CLASS(AINodeHide, AINodeSmartObject)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeHide::ReadProp()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

AINodeHide::AINodeHide() : 	
	  m_fMinExpiration(0.f)
	, m_fMaxExpiration(0.f)
	, m_fExpirationTime(0.f)
{
}

AINodeHide::~AINodeHide()
{
}

void AINodeHide::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_ThreatRadiusValidator.Load( pMsg );
	m_BoundaryRadiusValidator.Load( pMsg );
	m_ThreatFOV.Load( pMsg );

	LOAD_FLOAT(m_fMinExpiration);
	LOAD_FLOAT(m_fMaxExpiration);
	LOAD_TIME(m_fExpirationTime);
}

void AINodeHide::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_ThreatRadiusValidator.Save( pMsg );
	m_BoundaryRadiusValidator.Save( pMsg );
	m_ThreatFOV.Save( pMsg );

	SAVE_FLOAT(m_fMinExpiration);
	SAVE_FLOAT(m_fMaxExpiration);
	SAVE_TIME(m_fExpirationTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeHide::ReadProp()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void AINodeHide::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	m_ThreatRadiusValidator.ReadProps( pProps );
	m_BoundaryRadiusValidator.ReadProps( pProps );
	m_ThreatFOV.ReadProps( pProps );

	m_fMinExpiration = pProps->GetReal( "MinExpiration", m_fMinExpiration );
	m_fMaxExpiration = pProps->GetReal( "MaxExpiration", m_fMaxExpiration );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeHide::InitNode
//              
//	PURPOSE:	Initialize the node.
//              
//----------------------------------------------------------------------------

void AINodeHide::InitNode()
{
	super::InitNode();

	m_ThreatRadiusValidator.InitNodeValidator( this );
	m_BoundaryRadiusValidator.InitNodeValidator( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeHide::IsNodeValid()
//
//	PURPOSE:	Returns true if this node is valid given the current state
//				of the node and AI.
//
//	TODO:		Most of these are redundant/duplicates of the Cover/Ambush 
//				nodes.  Consider moving these queries into the AINode base 
//				class(?) and doing filtering in data.
//
// ----------------------------------------------------------------------- //

bool AINodeHide::IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags )
{
	uint32 dwFilteredStatusFlags = FilterStatusFlags(pAI, dwStatusFlags);

	// Skip many of the validation tests if AI has been scripted to go to 
	// this node but is not yet there.

	if( pAI )
	{
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( !pProp || pProp->hWSValue != m_hObject )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType( kFact_Task );
			factQuery.SetTargetObject(m_hObject);
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( pFact )
			{
				return true;
			}
		}
	}

	// AI is at the node and the node is expired.

	if( dwFilteredStatusFlags & kNodeStatus_Expired )
	{
		if( m_fExpirationTime > 0.f )
		{
			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
			if( pProp && pProp->hWSValue == m_hObject )
			{
				// Expiration time has passed.

				if( g_pLTServer->GetTime() > m_fExpirationTime )
				{
					// Do not invalidate by the expiration time if AI is scripted to hide.

					CAIWMFact factQuery;
					factQuery.SetFactType( kFact_Task );
					factQuery.SetTaskType( kTask_Hide );
					CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
					if( ! ( pFact && pFact->GetTargetObject() == m_hObject ) )
					{
						return false;
					}
				}
			}
		}
	}

	// An AI has been damaged at this node; it isn't safe to use.

	if( dwFilteredStatusFlags & kNodeStatus_Damaged )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Knowledge);
		factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
		factQuery.SetTargetObject(m_hObject);

		CAIWMFact* pFact = NULL;
		pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
		if( pFact )
		{
			if (pFact->GetTime() < g_pLTServer->GetTime())
			{
				// Expired fact, remove it.

				g_pAIWorkingMemoryCentral->ClearWMFact(pFact);
			}
			else
			{
				// Fact applies, return.
				if ( DidDamage( pAI, pFact ) )
				{
					return false;
				}
			}
		}
	}

	if( hThreat )
	{
		// Precalculate frequently used information so that it does not have 
		// to be done multiple times.

		LTVector vThreatPos;
		ENUM_NMPolyID eThreatNMPoly;
		GetThreatPosition( pAI, hThreat, eThreatPos, &vThreatPos, &eThreatNMPoly );
		float fNodeDistSqr = m_vPos.DistSqr(vThreatPos);

		LTVector vThreatDir = vThreatPos - m_vPos;
		vThreatDir.y = 0.f;
		vThreatDir.Normalize();

		float flToThreatDp = vThreatDir.Dot(m_rRot.Forward());

		// Threat inside radius.

		if ( !m_ThreatRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		// Threat outside boundary.

		if ( !m_BoundaryRadiusValidator.Evaluate( this, dwFilteredStatusFlags, vThreatPos, eThreatNMPoly ) )
		{
			return false;
		}

		// Threat fov

		const bool kbIsIgnoreDir = false;
		if ( !m_ThreatFOV.Evaluate( dwFilteredStatusFlags, vThreatPos, m_vPos, m_rRot.Forward(), kbIsIgnoreDir ) )
		{
			return false;
		}

		// Threat is engaged in a melee combat
		// TODO: Implement

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatEngagedInMelee )
		{
		}


		// Threat is outside of the nodes FOV AND inside the AIs FOV AND facing away from the AI

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatIsAtDisadvantage )
		{
			// Verify that the threat it inside the AIs FOV

			if( dwFilteredStatusFlags & kNodeStatus_ThreatInsideRadius )
			{
				// TODO: This overloads the threat radius and does not take 
				// into account the range of the AIs weapon.  Consider revising 
				// based on level design feedback.  This meets the initial 
				// request, which is 'no more than 3 radii'.

				if( fNodeDistSqr < m_ThreatRadiusValidator.GetThreatRadiusSqr() )
				{
					if (pAI->IsInsideFOV(vThreatDir.MagSqr(), vThreatDir))
					{
						// Verify that the threat is a character facing away from the AI.

						if (IsCharacter(hThreat))
						{
							CCharacter* pThreat = (CCharacter*)g_pLTServer->HandleToObject(hThreat);
							LTRigidTransform rThreatRotation;
							pThreat->GetViewTransform(rThreatRotation);
							LTVector vThreadForward = rThreatRotation.m_rRot.Forward();
							vThreadForward.y = 0.f;
							vThreadForward.Normalize();
							if ( 0 > vThreadForward.Dot(-vThreatDir))
							{
								return false;
							}
						}
					}
				}
			}
		}


		// Threat is aware of the AIs location

		if ( dwFilteredStatusFlags & kNodeStatus_ThreatAwareOfPosition )
		{
			// If the AI just saw the target, he hasn't had enough time to 
			// determine if the target has seen him yet.  Only perform this 
			// additional test if the target change time was at least 1 second 
			// ago.
			// The time here, 1.0, is an arbitrary duration that can be tweaked
			// as needed.  This is meant to be a bit of buffer time to allow the 
			// AI to sense target before performing confidence tests.

			if ( 1.0f > ( g_pLTServer->GetTime() - pAI->GetAIBlackBoard()->GetBBTargetChangeTime() ) )
			{
				return false;
			}
			else
			{
				CAIWMFact factQuery;
				factQuery.SetFactType( kFact_Knowledge );
				factQuery.SetKnowledgeType( kKnowledge_EnemyKnowsPosition );
				CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
				if (pFact
					&& (pFact->GetConfidence(CAIWMFact::kFactMask_KnowledgeType) > g_pAIDB->GetMiscFloat( "TargetIsAwareOfPositionMaxHide" ) ))
				{
					return false;
				}
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeHide::HandleAIArrival()
//
//	PURPOSE:	Handles the AI arriving at the node.  Resets the expiration
//				timeout when an AI arrives at the node.  If the AI is not
//				at the node, this time is irrelevant.
//
// ----------------------------------------------------------------------- //

void AINodeHide::HandleAIArrival( CAI* pAI )
{
	super::HandleAIArrival( pAI );

	// Calculate a random expiration time.

	if( ( m_fMinExpiration > 0.f ) &&
		( m_fMaxExpiration > 0.f ) )
	{
		m_fExpirationTime = g_pLTServer->GetTime() + GetRandom( m_fMinExpiration, m_fMaxExpiration );
	}
}
