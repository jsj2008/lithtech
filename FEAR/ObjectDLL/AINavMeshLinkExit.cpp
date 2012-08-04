// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkExit.cpp
//
// PURPOSE : 
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkExit.h"
#include "AINode.h"
#include "AI.h"
#include "AINavMesh.h"
#include "AIActionMgr.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_AINavMeshLinkExit 0

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_AINavMeshLinkExit CF_HIDDEN

#endif

BEGIN_CLASS( AINavMeshLinkExit )

	// Hide irrelevant AINavMeshLinkAbstract properties.

	ADD_STRINGPROP_FLAG(SmartObject,					"",				PF_HIDDEN|PF_OBJECTLINK, "Not used.")

	// Add new properties

	ADD_REALPROP_FLAG(ThreatRadius,				256.0f,			0|PF_RADIUS, "If the AIs threat is inside this radius, the exit link behavior is not executed. [WorldEdit units]")
	ADD_REALPROP_FLAG(BoundaryRadius,			1024.0f,		0|PF_RADIUS|PF_FOVFARZ, "If the AI's threat is outside of this radius, the exit link behavior is not relevant to perform. [WorldEdit units]")

	PROP_DEFINEGROUP(ActionNodeList, PF_GROUP(1), "")
		ADD_STRINGPROP_FLAG(ActionNode1, "", PF_GROUP(1)|PF_OBJECTLINK, "ActionNode associated with this AINavMeshLinkExit")
		ADD_STRINGPROP_FLAG(ActionNode2, "", PF_GROUP(1)|PF_OBJECTLINK, "ActionNode associated with this AINavMeshLinkExit")
		ADD_STRINGPROP_FLAG(ActionNode3, "", PF_GROUP(1)|PF_OBJECTLINK, "ActionNode associated with this AINavMeshLinkExit")
		ADD_STRINGPROP_FLAG(ActionNode4, "", PF_GROUP(1)|PF_OBJECTLINK, "ActionNode associated with this AINavMeshLinkExit")

END_CLASS_FLAGS(AINavMeshLinkExit, AINavMeshLinkAbstract, CF_HIDDEN_AINavMeshLinkExit, "TODO:CLASSDESC")

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkExit::Constructor/Destructor
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

AINavMeshLinkExit::AINavMeshLinkExit():
	m_flThreatRadiusSqr(0.f)
	, m_flBoundaryRadiusSqr(0.f)
{
}

AINavMeshLinkExit::~AINavMeshLinkExit()
{
}

void AINavMeshLinkExit::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_flThreatRadiusSqr);
	LOAD_FLOAT(m_flBoundaryRadiusSqr);
	for (int i = 0; i < kMaxActionNodes; ++i)
	{
		LOAD_HOBJECT(m_hActionSmartObjectNode[i]);
	}

	// Don't need to load m_aszActionSmartObjectName
}

void AINavMeshLinkExit::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_flThreatRadiusSqr);
	SAVE_FLOAT(m_flBoundaryRadiusSqr);
	for (int i = 0; i < kMaxActionNodes; ++i)
	{
		SAVE_HOBJECT(m_hActionSmartObjectNode[i]);
	}

	// Don't need to save m_aszActionSmartObjectName
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkAbstract::EngineMessageFn
//              
//	PURPOSE:	Handle engine messages.
//              
//----------------------------------------------------------------------------

uint32 AINavMeshLinkExit::EngineMessageFn(uint32 messageID, void *pvData, float fData)
{
	switch(messageID)
	{
		case MID_ALLOBJECTSCREATED:
		{
			// Convert the names to handles
			for (int i = 0; i < kMaxActionNodes; ++i)
			{
				HOBJECT hObj = NULL;
				m_hActionSmartObjectNode[i] = NULL;
				if (LT_OK != FindNamedObject(m_aszActionSmartObjectName[i].c_str(), hObj))
				{
					continue;
				}

				AIASSERT1(IsKindOf(hObj, "AINodeSmartObject"), m_hObject, "Linked node is not a smartobject: %d", i);
				if (IsKindOf(hObj, "AINodeSmartObject"))
				{
					m_hActionSmartObjectNode[i] = hObj;
				}
			}
		}
		break;
	}

	return super::EngineMessageFn(messageID, pvData, fData);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkExit::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkExit::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	// Read the action nodes.
	for (int i = 0; i < kMaxActionNodes; ++i)
	{
		char szBuffer[64];
		LTSNPrintF( szBuffer, ARRAY_LEN( szBuffer ), "%s%d", "ActionNode", i+1 );
		m_aszActionSmartObjectName[i] = pProps->GetString(szBuffer, "");
	}

	m_flThreatRadiusSqr = pProps->GetReal("ThreatRadius", 0.f);
	m_flThreatRadiusSqr *= m_flThreatRadiusSqr;

	m_flBoundaryRadiusSqr = pProps->GetReal("BoundaryRadius", 0.f);
	m_flBoundaryRadiusSqr *= m_flBoundaryRadiusSqr;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkExit::IsLinkExitRelevant
//              
//	PURPOSE:	Returns true if the exit link is relevant, false if it is 
//				not.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkExit::IsLinkExitRelevant( CAI* pAI )
{
	// Bail if no path set.

	if( pAI->GetAIMovement()->IsUnset() )
	{
		return false;
	}

	// AI is not in the link.

	if( !IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	// AI does not have a character enemy.

	if (!pAI->HasTarget(kTarget_Character))
	{
		return false;
	}
	CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(pAI->GetAIBlackBoard()->GetBBTargetObject());
	if (!pCharacter)
	{
		return false;
	}

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	LTVector vTargetPosition2D;
	g_pLTServer->GetObjectPos(hTarget, &vTargetPosition2D);
	vTargetPosition2D.y = 0.f;

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	LTVector vLinkCenter2D = pPoly->GetNMPolyCenter();
	vLinkCenter2D.y = 0.f;

	float flDistanceToTargetSqr = vLinkCenter2D.DistSqr(vTargetPosition2D);

	// Target is too close

	if (flDistanceToTargetSqr < m_flThreatRadiusSqr)
	{
		return false;
	}

	// Target is too far

	if (flDistanceToTargetSqr > m_flBoundaryRadiusSqr)	
	{
		return false;
	}

	LTRigidTransform tfView;
	pCharacter->GetViewTransform( tfView );

	LTVector vThreatForward2D = tfView.m_rRot.Forward();
	vThreatForward2D.y = 0.f;
	if( vThreatForward2D != LTVector::GetIdentity() )
	{
		vThreatForward2D.Normalize();
	}

	// Threat is not facing the link

	if ((vLinkCenter2D - vTargetPosition2D).Dot(vThreatForward2D) < c_fFOV60)
	{
		return false;
	}

	// Threat is not behind the direction the AI is heading.
	
	LTVector vAIPosition2D = pAI->GetPosition();
	vAIPosition2D.y = 0.f;

	LTVector vDirAIToEnemy2D = vTargetPosition2D - vAIPosition2D;
	if( vDirAIToEnemy2D != LTVector::GetIdentity() )
	{
		vDirAIToEnemy2D.Normalize();
	}

	LTVector vDestDir = pAI->GetAIMovement()->GetDest() - pAI->GetPosition();
	vDestDir.y = 0.f;
	if( vDestDir != LTVector::GetIdentity() )
	{
		vDestDir.Normalize();
	}

	float flDotProduct = vDestDir.Dot(vDirAIToEnemy2D);
	if (flDotProduct > -0.5)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkExit::IsLinkExitRelevant
//              
//	PURPOSE:	Returns a pointer to the smartobject to use based on distance
//				from the destination point out of the link to the nearest 
//				node.  This may return a node that is not valid for the AI to 
//				use; the caller is responsible for any further filtering.s
//              
//----------------------------------------------------------------------------

AINodeSmartObject* AINavMeshLinkExit::GetActionSmartObjectNode( CAI* pAI )
{
	// Find the closest node to the destination position, and use it as the 
	// action node.

	const LTVector& vDest = pAI->GetAIMovement()->GetDest();

	HOBJECT hBestNode = NULL;
	float flBestFromEndPositionSqr = FLT_MAX;

	for (int i = 0; i < kMaxActionNodes; ++i)
	{
		if (!m_hActionSmartObjectNode[i])
		{
			continue;
		}

		LTVector vPosition;
		g_pLTServer->GetObjectPos(m_hActionSmartObjectNode[i], &vPosition);
		float flDistToNode = vPosition.DistSqr(vDest);
		if (flDistToNode < flBestFromEndPositionSqr)
		{
			flBestFromEndPositionSqr = flDistToNode;
			hBestNode = m_hActionSmartObjectNode[i];
		}
	}	

	// Only smartobjects should be in this list, so just cast to it without 
	// a check.
	
	return (AINodeSmartObject*)g_pLTServer->HandleToObject(hBestNode);
}
