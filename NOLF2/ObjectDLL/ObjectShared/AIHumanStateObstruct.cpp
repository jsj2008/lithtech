//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateObstruct.cpp
//              
//	PURPOSE:	CAIHumanStateObstruct implementation
//              
//	CREATED:	12.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Handles moving to a points and blocking something.  The idea
//				is JUST getting between 2 things, NOT attacking or advancing.
//				
//				Flow:
//					- Go to Node
//					- Stay at node (time, event, or message based)
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIHUMANSTATEOBSTRUCT_H__
#include "AIHumanStateObstruct.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif 


// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateObstruct, kState_HumanObstruct);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::ConstructAIClass()
//              
//	PURPOSE:	Handle initialization of all variables whose value is known
//				at construction of the class (without the AI present) and 
//				which are potentially not initialized later.
//              
//----------------------------------------------------------------------------
CAIHumanStateObstruct::CAIHumanStateObstruct()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_fCloseEnoughDistSqr = 0.0;
}

CAIHumanStateObstruct::~CAIHumanStateObstruct()
{
	AI_FACTORY_DELETE( m_pStrategyFollowPath );
}

void CAIHumanStateObstruct::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIHumanStateObstruct::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::ClearObstructObject()
//              
//	PURPOSE:	Removes and unlinks the Obstruct Object
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::ClearObstructObject()
{
	if ( !m_hNodeToDoObstructAt )
		return;

	AINodeObstruct* pAINodeObstruct = dynamic_cast< AINodeObstruct* >( g_pLTServer->HandleToObject( m_hNodeToDoObstructAt ));
	if( !pAINodeObstruct )
		return;

	pAINodeObstruct->Unlock( m_pAI->m_hObject );
	m_hNodeToDoObstructAt = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::SetNode()
//              
//	PURPOSE:	Accessor function to allow setting a selected Obstruct node.
//				Clears the old node, validates the new, and then sets the
//				new path
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::SetNode(AINodeObstruct& ObstructNode)
{
	ClearObstructObject();

	if ( IsValidObstructNode( ObstructNode ) == false )
	{
		return;
	}
	if ( AttemptSetPathToNode( ObstructNode ) == false )
		return;

	SetPathNode( ObstructNode );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::SetAcceptableDistanceToNode()
//              
//	PURPOSE:	AI will move to the node till they are this close
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::SetAcceptableDistanceToNode(float fDist)
{
	m_fCloseEnoughDistSqr = fDist*fDist;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::IsValidObstructNode()
//              
//	PURPOSE:	Checks the validity of a passed node.
//              
//----------------------------------------------------------------------------
bool CAIHumanStateObstruct::IsValidObstructNode( AINodeObstruct& Node )
{
	if ( Node.IsLockedDisabledOrTimedOut() )
	{
		g_pLTServer->CPrint("OBSTRUCTNODE DEST=%s - Obstruct node is already locked!",
			g_pLTServer->GetStringData(Node.GetName()));

		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::SetPathNode()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::SetPathNode(AINodeObstruct& Node)
{
	m_hNodeToDoObstructAt = Node.m_hObject;

	Node.Lock( GetAI()->m_hObject );
	m_pStrategyFollowPath->SetMovement( Node.GetMovement() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::AttemptSetPathToNode()
//              
//	PURPOSE:	Attempts to set a path to a Node, returning true if
//				successful, cleaning itself up and returning false if it fails
//              
//----------------------------------------------------------------------------
bool CAIHumanStateObstruct::AttemptSetPathToNode(AINodeObstruct& Node)
{
	if ( m_pStrategyFollowPath->Set(&Node, LTFALSE) == LTTRUE )
	{
		return true;
	}

	g_pLTServer->CPrint("USEOBJECT OBJECT=%s -- unable to find path!", g_pLTServer->GetStringData(Node.GetName()));
	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::Init()
//              
//	PURPOSE:	Handles initialization of the CAIHumanStateObstruct class,
//				Retreive initialization information from the AI, and change
//				the AI state to reflect entry to this state
//              
//----------------------------------------------------------------------------
LTBOOL CAIHumanStateObstruct::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	InitPath( pAIHuman );

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::InitPath()
//              
//	PURPOSE:	Initialize the Path and all Path variables
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::InitPath(CAIHuman* pAIHuman) 
{
	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);
	m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::Update(void)
{
	super::Update();

	MaybePlayFirstUpdateSound();

	if ( IsNodeStillValid() == false )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	if ( CanUpdatePath() )
	{
		m_pStrategyFollowPath->Update();
	}
	else
	{
		GetAI()->FaceObject( GetObjectToObstruct() );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::IsNodeNoLongerValid()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
bool CAIHumanStateObstruct::IsNodeStillValid()
{
	// If the Node is no longer valid, then fail out of the state!
	if ( m_hNodeToDoObstructAt == NULL )
		return false;

	AINodeObstruct* pAINodeObstruct = dynamic_cast< AINodeObstruct* >( g_pLTServer->HandleToObject( m_hNodeToDoObstructAt ));
	if( !pAINodeObstruct )
		return false;

	LTVector vUnused;
	EnumNodeStatus stat = pAINodeObstruct->GetStatus( vUnused, m_hObjectToObstruct );

	if ( stat == kStatus_ThreatOutsideFOV )
		return false;

	if ( stat == kStatus_ThreatInsideRadius	)
		return false;

	if ( stat == kStatus_Invalid )
		return false;

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::CanUpdatePath()
//              
//	PURPOSE:	Returns true if we have a valid path that we ought to use,
//				false if we do not.
//              
//----------------------------------------------------------------------------
bool CAIHumanStateObstruct::CanUpdatePath(void)
{
	if ( IsAICloseEnoughToNode() == true )
		return false;
	if ( m_pStrategyFollowPath->IsSet() == LTFALSE )
		return false;
	if ( m_pStrategyFollowPath->IsDone() == LTTRUE )
		return false;

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::SetObjectToObstruct()
//              
//	PURPOSE:	Save a link to the object the AI is obstructing
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::SetObjectToObstruct(HOBJECT hObj)
{
	m_hObjectToObstruct = hObj;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::GetObjectToObstruct()
//              
//	PURPOSE:	Returns a link to the object the AI is obstructing
//              
//----------------------------------------------------------------------------
HOBJECT CAIHumanStateObstruct::GetObjectToObstruct(void)
{
	return m_hObjectToObstruct;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::IsAICloseEnoughToNode()
//              
//	PURPOSE:	Returns true if the AI is within the prespecified range, false
//				if the player is not.
//              
//----------------------------------------------------------------------------
bool CAIHumanStateObstruct::IsAICloseEnoughToNode(void)
{
	if ( !m_hNodeToDoObstructAt )
		return false;

	LTVector vNodePos, vAIPos;
    g_pLTServer->GetObjectPos(m_hNodeToDoObstructAt, &vNodePos);
    g_pLTServer->GetObjectPos(GetAI()->m_hObject, &vAIPos);

	float ActualDistance = VEC_DISTSQR(vAIPos, vNodePos);

	// If the distance we must attain is less than the current distance..
	if ( ActualDistance < m_fCloseEnoughDistSqr )
		return true;

	return false;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::MaybePlayFirstUpdateSound()
//              
//	PURPOSE:	Potentially play the first update sound.
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::MaybePlayFirstUpdateSound()
{
	// Play our first sound if we should -- Do we have a marking sound?
	if ( m_bPlayFirstSound || m_bFirstUpdate )
	{
		GetAI()->PlayCombatSound(kAIS_Search);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateObstruct::UpdateAnimation()
//              
//	PURPOSE:	Sets the Props for the given animation
//              
//----------------------------------------------------------------------------
void CAIHumanStateObstruct::UpdateAnimation(void)
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	if( IsNodeStillValid() && CanUpdatePath() )
	{
		m_pStrategyFollowPath->UpdateAnimation();
	}
}
