// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDoor.cpp
//
// PURPOSE : AI NavMesh Link Door class implementation.
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshLinkDoor.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AIAssert.h"
#include "Door.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AIPathKnowledgeMgr.h"
#include "AnimationContext.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "NodeTrackerContext.h"

// WorldEdit

LINKFROM_MODULE( AINavMeshLinkDoor );

BEGIN_CLASS( AINavMeshLinkDoor )
	ADD_STRINGPROP_FLAG(SmartObject,	"Door",			0|PF_STATICLIST, "SmartObject used to specify animations for traversing the link")
	ADD_STRINGPROP_FLAG(Door,			"",				0|PF_OBJECTLINK, "The name of the door corresponding to this link.")
END_CLASS_FLAGS_PLUGIN( AINavMeshLinkDoor, AINavMeshLinkAbstract, 0, AINavMeshLinkDoorPlugin, "This link is used to specify that the brush contains a door object that the AI must open while pathfinding, if closed" )

CMDMGR_BEGIN_REGISTER_CLASS( AINavMeshLinkDoor )
CMDMGR_END_REGISTER_CLASS( AINavMeshLinkDoor, AINavMeshLinkAbstract )

namespace 
{
	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	GetDoor()
	//              
	//	PURPOSE:	Simple utility function to handle converting a door object to
	//				a door pointer safely.  Asserts and returns NULL if the passed
	//				in object is not a door.  A NULL handle is allowed, and NULL
	//				is safely returned in response.
	//              
	//----------------------------------------------------------------------------

	Door* GetDoor(HOBJECT hDoor)
	{
		if (!hDoor)
		{
			return NULL;
		}

		if (!IsKindOf(hDoor, "Door"))
		{
			AIASSERT(0, hDoor, "GetDoor : Object is expected to be a door.");
			return NULL;
		}

		return (Door*)g_pLTServer->HandleToObject(hDoor);
	}

	enum ENUM_AIDoorStatus
	{
		AIDoorStatus_RequiresAction,	// Door is closed or closing, and allows the AI to use it.
		AIDoorStatus_NotTraversable,	// Door is closed or closing, and does not allow the AI to use it.
		AIDoorStatus_Blocked,			// Door is not open, can be opened by the AI, but is obstructed.
		AIDoorStatus_Open,				// AI can now move through the door.
		AIDoorStatus_Opening,			// Door isn't open, but will be soon if not blocked
	};

	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	GetTraverseDoorStatus()
	//              
	//	PURPOSE:	Returns an enumerated value describing the state of the door 
	//				to the AI.
	//              
	//----------------------------------------------------------------------------
	ENUM_AIDoorStatus GetTraverseDoorStatus( CAI* pAI, HOBJECT hDoor)
	{
		Door* pDoor = GetDoor(hDoor);

		// No door (likely removed or destroyed).

		if (!pDoor)
		{
			return AIDoorStatus_Open;
		}

		// See if this door is known to be blocked

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Knowledge);
		factQuery.SetKnowledgeType(kKnowledge_DoorJammed);
		factQuery.SetSourceObject(hDoor);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);

		if (pFact)
		{
			if (pFact->GetTime() < g_pLTServer->GetTime() || 
				pDoor->GetState() == DOORSTATE_OPEN)
			{
				pAI->GetAIWorkingMemory()->ClearWMFact(pFact);
				pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
			}
			else
			{
				return AIDoorStatus_NotTraversable;
			}
		}

		// See if this door is known to be blocked

		CAIWMFact factBlockedQuery;
		factBlockedQuery.SetFactType(kFact_Knowledge);
		factBlockedQuery.SetKnowledgeType(kKnowledge_DoorBlocked);
		factBlockedQuery.SetSourceObject(hDoor);
		pFact = pAI->GetAIWorkingMemory()->FindWMFact(factBlockedQuery);

		if (pFact)
		{
			if (pFact->GetTime() < g_pLTServer->GetTime() || 
				pDoor->GetState() == DOORSTATE_OPEN)
			{
				pAI->GetAIWorkingMemory()->ClearWMFact(pFact);
				pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
			}
			else
			{
				return AIDoorStatus_Blocked;
			}
		}

		// Door is open.

		if (pDoor->GetState() == DOORSTATE_OPEN)
		{
			return AIDoorStatus_Open;
		}	
		
		// Door will be open soon, but AI cannot enter yet.

		if (pDoor->GetState() == DOORSTATE_OPENING)
		{
			return AIDoorStatus_Opening;
		}

		// Door is closed or closing, and AI cannot open it.

		if (!pDoor->IsAITriggerable() 
			|| pDoor->IsLockedForCharacter(pAI->GetHOBJECT()))
		{
			return AIDoorStatus_NotTraversable;
		}

		// Door needs to be opened.

		return AIDoorStatus_RequiresAction;
	}
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::Channel
//              
//	PURPOSE:	Constructs the channel into an inert state.
//              
//----------------------------------------------------------------------------

AINavMeshLinkDoor::Channel::Channel()
{
	m_iDoorCount = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::Save/Load
//              
//	PURPOSE:	Handles serializaing the channel.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::Channel::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_iDoorCount);
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		SAVE_HOBJECT(m_hDoor[i]);
	}

	SAVE_VECTOR(m_v0);
	SAVE_VECTOR(m_v1);
}

void AINavMeshLinkDoor::Channel::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT(m_iDoorCount);
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		LOAD_HOBJECT(m_hDoor[i]);
	}

	LOAD_VECTOR(m_v0);
	LOAD_VECTOR(m_v1);
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::Center
//              
//	PURPOSE:	Returns the center point of the channel.
//              
//----------------------------------------------------------------------------

LTVector AINavMeshLinkDoor::Channel::GetCenter() const
{
	return m_v0 + 0.5f*(m_v1 - m_v0);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::Open
//              
//	PURPOSE:	Handles opening the doors the channel uses.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::Channel::Open(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		g_pCmdMgr->QueueMessage( pAI, GetDoor(m_hDoor[i]), "ON" );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetRequiresAction
//              
//	PURPOSE:	Returns true if the Channel requires an action (opening a door, 
//				etc) to be traversed.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetRequiresAction(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		// A door requires an action.  Other doors may be blocked, locked, etc!  
		// This does NOT mean that a channel is traversable.

		ENUM_AIDoorStatus eStatus = GetTraverseDoorStatus(pAI,  m_hDoor[i]);
		if (eStatus == AIDoorStatus_RequiresAction)
		{
			return true;
		}
	}

	// No doors require actions.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetIsOpen
//              
//	PURPOSE:	Returns true if the Channel can be walked through at the 
//				current time.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetIsOpen(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		// A door is not open, therefore the channel is not open.

		ENUM_AIDoorStatus eStatus = GetTraverseDoorStatus(pAI,  m_hDoor[i]);
		if (eStatus != AIDoorStatus_Open)
		{
			return false;
		}
	}

	// All doors must be open.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetIsRotating
//              
//	PURPOSE:	Returns true if the all doors in the channel are rotating, 
//				false if they are not.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetIsRotating() const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		// A door is rotating, therefore the channel is not rotating.

		Door* pDoor = GetDoor(m_hDoor[i]);
		if (pDoor)
		{
			if (!pDoor->IsRotatingWorldModel())
			{
				return false;
			}
		}
	}

	// All doors are rotating.

	return true;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetIsOpen
//              
//	PURPOSE:	Returns true if the Channel is in the process of opening and
//				will likely be traversable soon.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetIsOpening(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		// A door is not open, therefore the channel is not open.

		ENUM_AIDoorStatus eStatus = GetTraverseDoorStatus(pAI,  m_hDoor[i]);
		if (eStatus != AIDoorStatus_Open 
			&& eStatus != AIDoorStatus_Opening)
		{
			return false;
		}
	}

	// All doors must be open.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetCanTraverse
//              
//	PURPOSE:	Returns true if the AI can pass through this channel at all.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetCanTraverse(CAI* pAI) const
{
	ENUM_AIDoorStatus eDoorStatus[kMaxDoors];
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		// A non-traverable door was found, this channel cannot be traversed.

		eDoorStatus[i] = GetTraverseDoorStatus(pAI,  m_hDoor[i]);
		if (AIDoorStatus_NotTraversable == eDoorStatus[i] )
		{
			return false;
		}

		// Double doors are not all in the same state

		if (i > 0 
			&& eDoorStatus[i] != eDoorStatus[i-1])
		{
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::GetCanTraverse
//              
//	PURPOSE:	Returns true if this channel is blocked, false if it is not. 
//				A channel is blocked if any of the doors involved in opening
//				it are blocked.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::Channel::GetBlocked(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		Door* pDoor = GetDoor(m_hDoor[i]);

		// Doesn't exist

		if (!pDoor)
		{
			return false;
		}

		// Door is blocked

		if (pDoor->GetBlocked())
		{
			return true;
		}
	}

	// No doors are blocked.

	return false;
}

bool AINavMeshLinkDoor::Channel::GetBlockedToAI(CAI* pAI) const
{
	for (int i = 0; i < m_iDoorCount; ++i)
	{
		if (AIDoorStatus_Blocked == GetTraverseDoorStatus(pAI,  m_hDoor[i]))
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	Channel::SetBlockedToAI
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::Channel::SetBlockedToAI(CAI* pAI, bool bJammed ) const
{
	// Tell the AI which doors are blocked.  
	//
	// We don't want to block the channel as this would allow for 'dumb' cases,
	// like the AI trying single door channel then trying the double door 
	// (obviously blocked if the single channel was).
	//
	// We also do not want to block both doors, as one may be open or otherwise
	// manipulatable.  As this is the case, block the doors individually as an
	// implementation deal that may change in the future.

	for (int i = 0; i < m_iDoorCount; ++i)
	{
		Door* pDoor = GetDoor(m_hDoor[i]);

		if (pDoor && pDoor->GetBlocked())
		{
			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Knowledge);
			factQuery.SetKnowledgeType(bJammed ? kKnowledge_DoorJammed : kKnowledge_DoorBlocked );
			factQuery.SetSourceObject(pDoor->GetHOBJECT());
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
			
			if (!pFact)
			{
				pFact = pAI->GetAIWorkingMemory()->CreateWMFact(kFact_Knowledge);
				pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
			}

			if (pFact)
			{
				pFact->SetKnowledgeType(bJammed ? kKnowledge_DoorJammed : kKnowledge_DoorBlocked);
				pFact->SetSourceObject(pDoor->GetHOBJECT());
				pFact->SetTime(g_pLTServer->GetTime() + g_pAIDB->GetAIConstantsRecord()->fBlockedDoorAvoidanceTime);
			}
		}
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::Constructor
//              
//	PURPOSE:	Constructor
//              
//----------------------------------------------------------------------------

AINavMeshLinkDoor::AINavMeshLinkDoor()
{
	for (int i = 0; i < kMaxDoors; ++i)
	{
		m_hDoor[i] = NULL;
	}

	m_iValidDoors = 0;
	m_iValidChannels = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp( pProps );

	// Read door name.

	const char* pszPropString = pProps->GetString( "Door", "" );
	if( pszPropString[0] )
	{
		m_strDoor = pszPropString;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINavMeshLinkDoor
//              
//----------------------------------------------------------------------------
void AINavMeshLinkDoor::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_STDSTRING(m_strDoor);
		
	{for (int i = 0; i < kMaxDoors; ++i)
	{
		SAVE_HOBJECT(m_hDoor[i]);
	}}

	{for (int i = 0; i < kChannel_Count; ++i)
	{
		m_Channel[i].Save(pMsg);
	}}
	
	SAVE_INT(m_iValidDoors);
	SAVE_INT(m_iValidChannels);
}

void AINavMeshLinkDoor::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_STDSTRING(m_strDoor);

	{for (int i = 0; i < kMaxDoors; ++i)
	{
		LOAD_HOBJECT(m_hDoor[i]);
	}}

	{for (int i = 0; i < kChannel_Count; ++i)
	{
		m_Channel[i].Load(pMsg);
	}}

	LOAD_INT(m_iValidDoors);
	LOAD_INT(m_iValidChannels);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::InitialUpdate
//              
//	PURPOSE:	Convert the Door's name to a handle.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::InitialUpdate()
{
	super::InitialUpdate();

	// No door specified.

	if( m_strDoor.empty() )
	{
		return;
	}

	m_iValidDoors = 0;

	// Lookup door by name.

	const char* pszName = m_strDoor.c_str();

	for (int i = 0; i < kMaxDoors; ++i)
	{
		// Link to the named door

		HOBJECT hDoor;
		LTRESULT res = FindNamedObject( pszName, hDoor );
		if( res != LT_OK || !hDoor || !IsKindOf(hDoor, "Door"))
		{
			AIASSERT2( 0, NULL, "AINavMeshLinkDoor::InitialUpdate: %s: Object '%s' is not a Door.", GetName(), m_strDoor.c_str() );
			break;
		}
		else
		{
			++m_iValidDoors;

			m_hDoor[i] = hDoor;

			Door* pDoor = GetDoor(hDoor);
			if( !pDoor )
			{
				AIASSERT1( 0, NULL, "AINavMeshLinkDoor::InitialUpdate: Door '%s' is NULL.", m_strDoor.c_str() );
				continue;
			}

			// Tell the door about the associated link.
			// The door needs to know to invalidate AI path caches 
			// if it has an associated NavMesh link.

			pDoor->SetNMLinkID( m_eNMLinkID );

			// Get the next linked door 
			pszName = pDoor->GetDoorLinkName().c_str();

			// No more new doors
			if (0==LTStrCmp(pszName, ""))
			{
				break;
			}
		}
	}

	SetupDoorChannels();

	// Free the string.

	m_strDoor.clear();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::SetupDoorChannels
//              
//	PURPOSE:	Handle associating space in the doorway with doors.  This 
//				association is refered to as a channel.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::SetupDoorChannels()
{
	// Connect the doors to ranges.

	switch (m_iValidDoors)
	{
	case 0:
		m_Channel[kChannel_Door1].m_v0 = m_vLinkEdgeA0;
		m_Channel[kChannel_Door1].m_v1 = m_vLinkEdgeA1;
		m_Channel[kChannel_Door1].m_hDoor[0] = NULL;
		m_Channel[kChannel_Door1].m_hDoor[1] = NULL;
		m_Channel[kChannel_Door1].m_iDoorCount = 0;
		m_iValidChannels = 1;
		break;

	case 1:
		{
			m_Channel[kChannel_Door1].m_v0 = m_vLinkEdgeA0;
			m_Channel[kChannel_Door1].m_v1 = m_vLinkEdgeA1;
			m_Channel[kChannel_Door1].m_hDoor[0] = m_hDoor[0];
			m_Channel[kChannel_Door1].m_hDoor[1] = NULL;
			m_Channel[kChannel_Door1].m_iDoorCount = 1;
			m_iValidChannels = 1;
		}
		break;

	case 2:
		{
			LTVector vDoor0;
			LTRESULT Result = g_pLTServer->GetObjectPos(m_hDoor[0], &vDoor0);
			AIASSERT(Result == LT_OK, NULL, "AINavMeshLinkDoor::SetupDoorChannels : Failed to get door0 position.");

			LTVector vDoor1;
			Result = g_pLTServer->GetObjectPos(m_hDoor[1], &vDoor1);
			AIASSERT(Result == LT_OK, NULL, "AINavMeshLinkDoor::SetupDoorChannels : Failed to get door1 position.");

			LTVector	vDir = m_vLinkEdgeA1 - m_vLinkEdgeA0;
			float		flDist = vDir.Mag();
			LTVector	vDirNorm = vDir.GetUnit();

			bool bDoor0ClosestToLinkEdgeA0 = false;
			if (vDoor0.DistSqr(m_vLinkEdgeA0) < vDoor1.DistSqr(m_vLinkEdgeA0))
			{
				bDoor0ClosestToLinkEdgeA0 = true;
			}

			m_Channel[kChannel_Door1].m_v0 = m_vLinkEdgeA0;
			m_Channel[kChannel_Door1].m_v1 = m_vLinkEdgeA0 + (vDirNorm * (0.4f*flDist));
			m_Channel[kChannel_Door1].m_hDoor[0] = bDoor0ClosestToLinkEdgeA0 ? m_hDoor[0] : m_hDoor[1];
			m_Channel[kChannel_Door1].m_hDoor[1] = NULL;
			m_Channel[kChannel_Door1].m_iDoorCount = 1;

			m_Channel[kChannel_Door2].m_v0 = m_vLinkEdgeA0 + (vDirNorm * (0.6f*flDist));
			m_Channel[kChannel_Door2].m_v1 = m_vLinkEdgeA0 + (vDirNorm * (1.0f*flDist));
			m_Channel[kChannel_Door2].m_hDoor[0] = bDoor0ClosestToLinkEdgeA0 ? m_hDoor[1] : m_hDoor[0];
			m_Channel[kChannel_Door2].m_hDoor[1] = NULL;
			m_Channel[kChannel_Door2].m_iDoorCount = 1;

			m_Channel[kChannel_DoorDouble].m_v0 = m_vLinkEdgeA0 + (vDirNorm * (0.4f*flDist));
			m_Channel[kChannel_DoorDouble].m_v1 = m_vLinkEdgeA0 + (vDirNorm * (0.6f*flDist));
			m_Channel[kChannel_DoorDouble].m_hDoor[0] = m_hDoor[0];
			m_Channel[kChannel_DoorDouble].m_hDoor[1] = m_hDoor[1];
			m_Channel[kChannel_DoorDouble].m_iDoorCount = 2;

			m_iValidChannels = 3;
		}
		break;

	default:
		AIASSERT(0, NULL, "AINavMeshLinkDoor::SetupDoorChannels : Unexpected Door Count");
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::GetNMLinkPathingWeight
//              
//	PURPOSE:	Returns the nav mesh links pathing cost multiplier
//              
//----------------------------------------------------------------------------

float AINavMeshLinkDoor::GetNMLinkPathingWeight(CAI* pAI)
{
	// If the link is blocked, it has a higher traversal cost.

	for (int i = 0; i < m_iValidChannels; ++i)
	{
		// Channel AI knows is unblocked.

		if (!m_Channel[i].GetBlockedToAI(pAI))
		{
			return super::GetNMLinkPathingWeight( pAI );;
		}
	}

	return g_pAIDB->GetAIConstantsRecord()->fBlockedDoorWeightMultiplier;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsLinkPassable
//              
//	PURPOSE:	Return true if link is currently passable.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsLinkPassable( CAI* pAI, ENUM_NMPolyID ePolyTo )
{
	// Intentionally do NOT call super::IsLinkPassable.
	// Limping AI should still be able to traverse doors.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Can the AI unblock doors?

	bool bAICanUnblockDoors = g_pAIActionMgr->ActionSetSupportsAbility(
		pAI->GetAIBlackBoard()->GetBBAIActionSet(), 
		kActionAbility_UnblockDoor);

	// Can at least one channel be traversed?

	for (int i = 0; i < m_iValidChannels; ++i)
	{
		// AI cannot traverse this link.

		if (!m_Channel[i].GetCanTraverse(pAI))
		{
			continue;
		}
		
		// Channel is blocked, and AI cannot pass it.

		if (m_Channel[i].GetBlockedToAI(pAI) && !bAICanUnblockDoors)
		{
			continue;
		}

		// AI must be able to pass this channel

		return true;
	}

	// If there are no doors, link is always passable.

	if( !DoDoorsExist() )
	{
		return true;
	}

	// Link is not passable.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsLinkRelevant
//              
//	PURPOSE:	Return true if link is relevant to a goal.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsLinkRelevant( CAI* pAI )
{
	// AI is not currently standing in this link.

	if( !IsInLinkOrOffsetEntry(pAI) )
	{
		return false;
	}

	// No channel selected.

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::IsLinkRelevant : Fact Pointer is NULL");
		return false;
	}
	
	// AI can no longer traverse this channel.

	if (!pChannel->GetCanTraverse(pAI))
	{
		return false;
	}

	// Channel being traversed does not require an action

	if (!pChannel->GetRequiresAction(pAI) && 
		!pChannel->GetBlocked(pAI))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsLinkValid
//              
//	PURPOSE:	Return true if link is currently valid for this AI.
//
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsLinkValid( CAI* pAI, EnumAIActionType eActionType, bool bTraversalInProgress ) 
{
	if( !super::IsLinkValid( pAI, eActionType, bTraversalInProgress ) )
	{
		return false;
	}

	// No AI.

	if (!pAI)
	{
		return false;
	}

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);

	// No channel.  This means that a path has not been generated through
	// this link yet.  As this shouldn't happen, assert.

	if (!pChannel)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::IsLinkValid : No channel during validity check");
		return false;
	}

	// Channel cannot be crossed in any way.

	if (!pChannel->GetCanTraverse(pAI))
	{
		return false;
	}

	// Channel is blocked, and action cannot unblock doors.

	if (pChannel->GetBlocked(pAI))
	{
		if( IsInLinkOrOffsetEntry(pAI) )
		{
			if (!g_pAIActionMgr->ActionSupportsAbility( eActionType, kActionAbility_UnblockDoor))
			{
				pChannel->SetBlockedToAI(pAI, false);
				return false;
			}
		}
		else 
		{
			if (! g_pAIActionMgr->ActionSetSupportsAbility(
				pAI->GetAIBlackBoard()->GetBBAIActionSet(), 
				kActionAbility_UnblockDoor))
			{
				pChannel->SetBlockedToAI(pAI, false);
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::ActivateTraversal
//              
//	PURPOSE:	Setup AI to traverse the link.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject )
{
	super::ActivateTraversal( pAI, pStateUseSmartObject );
	
	// Sanity check.
	
	if( !pAI )
	{
		return;
	}

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);

	// No channel.

	if (!pChannel)
	{
		return;
	}

	// Face the center of the channel.
	
	pAI->GetAIBlackBoard()->SetBBFacePos( pChannel->GetCenter() );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsTraversalComplete
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsTraversalComplete( CAI* pAI )
{
	// Sanity check.

	if (!pAI)
	{
		return false;
	}

	// NavMesh Poly does not exist.
	
	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eNMPolyID );
	if( !pPoly )
	{
		return true;
	}

	// The traversal just started.
	
	if( pAI->GetAIBlackBoard()->GetBBStateChangeTime() == g_pLTServer->GetTime() )
   	{
   		return false;
   	}

	// The traversal is complete when the animation completes.
	
	if( !pAI->GetAnimationContext()->IsLocked() )
	{
		return true;
	}

	// Traversal is not complete.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::GetNMLinkOffsetEntryDist
//              
//	PURPOSE:	Return the offset entry into edge.
//              
//----------------------------------------------------------------------------

float AINavMeshLinkDoor::GetNMLinkOffsetEntryDist( float fDefaultOffset ) const
{
	// Only return an offset entry if a door exists that 
	// is not currently open.

	Door* pDoor;
	for( uint32 iDoor=0; iDoor < kMaxDoors; ++iDoor )
	{
		if( m_hDoor[iDoor] )
		{
			pDoor = GetDoor( m_hDoor[iDoor] );
			if( !pDoor )
			{
				continue;
			}

			if( pDoor->GetState() != DOORSTATE_OPEN 
				&& pDoor->GetState() != DOORSTATE_OPENING )
			{
				return fDefaultOffset;
			}
		}
	}

	// All doors are open. Ignore the offset.

	return 0.f;
}

float AINavMeshLinkDoor::GetNMLinkOffsetEntryDistA() const
{
	return GetNMLinkOffsetEntryDist( super::GetNMLinkOffsetEntryDistA() );
}

float AINavMeshLinkDoor::GetNMLinkOffsetEntryDistB() const
{
	return GetNMLinkOffsetEntryDist( super::GetNMLinkOffsetEntryDistB() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsPullStringsModifying
//              
//	PURPOSE:	Function returns true when the pull string may modify the 
//				path, false when it promises not to.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsPullStringsModifying( CAI* pAI )
{
	// If there are no doors, do not modify the path.

	if( !DoDoorsExist() )
	{
		return false;
	}

	// Do not modify the path if we are already in the link.
	// This can result in the AI turning around to walk the wrong direction.

	if( pAI && IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::PullStrings
//              
//	PURPOSE:	Return true if traversal of the link is complete.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos )
{
	if (!pvNewPos)
	{
		return false;
	}

	// Handle the string pulling based on the number of channels the string 
	// may be pulled though.  If there is only one channel, it must be pulled 
	// though it.  If there are more than one, then a channel must be selected.
	switch(m_iValidChannels)
	{
	case 0:
	case 1:
		{
			return ChannelPullString(kChannel_Door1, vPtPrev, vPtNext, pAI, pvNewPos);
		}
		break;

	case 3:
		{
			ENUM_Channels eChannel = SelectChannelForStringPull(vPtPrev, vPtNext, pAI, pvNewPos);
			return ChannelPullString(eChannel, vPtPrev, vPtNext, pAI, pvNewPos);
		}
		break;

	default:
		AIASSERT(0, NULL, "AINavMeshLinkDoor::DoubleDoorPullString : Failed to find a valid door to traverse doors")
		break;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::DoubleDoorPullString
//              
//	PURPOSE:	Handles pulling a string through 
//              
//----------------------------------------------------------------------------

AINavMeshLinkDoor::ENUM_Channels AINavMeshLinkDoor::SelectChannelForStringPull(const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos) const
{
	ENUM_Channels eChannel = kChannel_Invalid;

	// Both doors are closed, and both doors are openable.  Open both, as 
	// they are linked and will both open together anyway.

	if (m_Channel[kChannel_DoorDouble].GetCanTraverse(pAI) 
		&& m_Channel[kChannel_DoorDouble].GetRequiresAction(pAI)
		&& !m_Channel[kChannel_DoorDouble].GetBlocked(pAI))
	{
		eChannel = kChannel_DoorDouble;
	}
	else
	{
		// Look for the closest channel the AI can traverse to the intersect 
		// point.

		LTVector vIntersect;
		if( kRayIntersect_Failure == RayIntersectLineSegment( vPtPrev, vPtNext, m_vLinkEdgeA0, m_vLinkEdgeA1, true, &vIntersect ) )
		{
			AIASSERT(0, NULL, "Line does not cross LinkEdge, verify that this system works as expected.");
		}
		else
		{
			ENUM_Channels eBestChannel = kChannel_Invalid;
			float flBestDistance = FLT_MAX;
			for (int i = 0; i < m_iValidChannels; ++i)
			{
				if (!m_Channel[i].GetCanTraverse(pAI) 
					|| m_Channel[i].GetBlocked(pAI))
				{
					continue;
				}

				float flTestDistance = vIntersect.DistSqr(m_Channel[i].GetCenter());
				if ( flTestDistance < flBestDistance)
				{
					eBestChannel = (ENUM_Channels)i;
					flBestDistance = flTestDistance;
				}
			}

			eChannel = eBestChannel;

			// No travsable unblocked channels?  Try the best one which is blocked..

			if ( kChannel_Invalid == eChannel )
			{
				bool bAICanUnblockDoors = g_pAIActionMgr->ActionSetSupportsAbility(
					pAI->GetAIBlackBoard()->GetBBAIActionSet(), 
					kActionAbility_UnblockDoor);
				if (bAICanUnblockDoors)
				{
					float flBestDistance = FLT_MAX;
					for (int i = 0; i < m_iValidChannels; ++i)
					{
						if (!m_Channel[i].GetCanTraverse(pAI))
						{
							continue;
						}

						float flTestDistance = vIntersect.DistSqr(m_Channel[i].GetCenter());
						if ( flTestDistance < flBestDistance)
						{
							eBestChannel = (ENUM_Channels)i;
							flBestDistance = flTestDistance;
						}
					}

					eChannel = eBestChannel;
				}
			}
		}
	}

	// Failed to find a channel for some reason.  Report the error and use 
	// the initial.

	if (eChannel == kChannel_Invalid)
	{
		AIASSERT(0,NULL, "Failed to find door to open.");
		eChannel = kChannel_Door1;
	}

	return eChannel;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::ChannelPullString
//              
//	PURPOSE:	Handles pulling the string though the selected channel.  The
//				string must be pulled though the center point of the selected
//				channel.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::ChannelPullString(ENUM_Channels eChannel, const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos)
{
	AIASSERT(m_Channel[eChannel].GetCanTraverse(pAI), NULL, "AINavMeshLinkDoor::PullStrings : Pulling strings through untraversable channel." );
	
	SetChannel(pAI, eChannel);
	*pvNewPos = m_Channel[eChannel].GetCenter();

	// If there are no doors, do not modify the path.

	if( !DoDoorsExist() )
	{
		return false;
	}

	// Do not modify the path if we are already in the link.
	// This can result in the AI turning around to walk the wrong direction.

	if( IsInLinkOrOffsetEntry( pAI ) )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsTraversalDoorBlocked
//              
//	PURPOSE:	Determines if the link is traversable to a door kick.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsTraversalDoorBlockedToAI( CAI* pAI )
{
	// Sanity check.

	if (!pAI)
	{
		return false;
	}

	// No selected Channel

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		return false;
	}

	// Channel is not blocked

	if (!pChannel->GetBlockedToAI(pAI))
	{
		return false;
	}

	return true;
}

void AINavMeshLinkDoor::SetDoorBlockedIsJammed( CAI* pAI )
{
	// Sanity check.

	if (!pAI)
	{
		return;
	}

	// No selected Channel

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		return;
	}

	// Channel is not blocked

	pChannel->SetBlockedToAI(pAI, true);
}

bool AINavMeshLinkDoor::IsDoorRotating(CAI* pAI)
{
	// Sanity check.

	if (!pAI)
	{
		return false;
	}

	// No selected Channel

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		return false;
	}

	return pChannel->GetIsRotating();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::IsTraversalDoorBlocked
//              
//	PURPOSE:	Determines if the link is traversable to a door kick.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::IsTraversalDoorBlocked( CAI* pAI )
{
	// Sanity check.

	if (!pAI)
	{
		return false;
	}

	// No selected Channel

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		return false;
	}

	// Channel is not blocked

	if (!pChannel->GetBlocked(pAI))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::DoDoorsExist
//              
//	PURPOSE:	Return true if doors exist.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::DoDoorsExist() const
{
	// Return true if at least one door exists.

	for( uint32 iDoor=0; iDoor < kMaxDoors; ++iDoor )
	{
		if( m_hDoor[iDoor] )
		{
			return true;
		}
	}

	// No doors exist.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::OpenDoor
//              
//	PURPOSE:	Handle opening the doors an AI depends on.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::OpenDoor(CAI* pAI)
{
	// Open the door.

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (pChannel)
	{
		pChannel->Open(pAI);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::HandleDoorClosed
//              
//	PURPOSE:	Handle a door closing.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::HandleDoorClosed( Door* pDoor )
{
	// Sanity check.

	if( !pDoor )
	{
		return;
	}

	g_pAIPathMgrNavMesh->InvalidatePathKnowledge( pDoor->m_hObject );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::HandleDoorOpened
//              
//	PURPOSE:	Handle a door opening.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::HandleDoorOpened( Door* pDoor )
{
	// Sanity check.

	if( !pDoor )
	{
		return;
	}

	g_pAIPathMgrNavMesh->InvalidatePathKnowledge( pDoor->m_hObject );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::ApplyMovementAnimation
//              
//	PURPOSE:	Handle applying the movement animation.  If the AI is waiting 
//				for a door to open, stomp the movement group to none.  This
//				will cause the AI to idle in place.  This is ONLY done for 
//				doors which are in the process of opening.  A closed door must
//				be opened first via traveral.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::ApplyMovementAnimation( CAI* pAI )
{
	if (!pAI)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::ApplyMovementAnimation : No AI pointer.")
		return;
	}

 	// Bail if no path set.
 
 	if( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
 	{
 		return;
 	}

	// Not yet in the link

	if( !IsInLinkOrOffsetEntry(pAI) )
	{
		return;
	}

	// No channel selected.

	const Channel* pChannel = NULL;
	GetChannel(pAI, &pChannel);
	if (!pChannel)
	{
		return;
	}

	// If the animation context is locked.

	if( pAI->GetAnimationContext()->IsLocked() )
	{
		pAI->GetAIBlackBoard()->SetBBFacePos( pChannel->GetCenter() );
		pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(true);
		return;
	}

	// Channel requires an action to initiate opening.  Don't tweak the anim.

	if (pChannel->GetRequiresAction(pAI))
	{
		pAI->GetAIBlackBoard()->SetBBFacePos( pChannel->GetCenter() );
		pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately(true);
		return;
	}

	// Channel is already open, don't wait.

	if (pChannel->GetIsOpen(pAI))
	{
		return;
	}

	pAI->GetAnimationContext()->SetProp( kAPG_Movement, kAP_None );	
	pAI->GetAnimationContext()->SetProp( kAPG_MovementDir, kAP_None );	
	pAI->GetAnimationContext()->SetProp( kAPG_Activity, kAP_None );	
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::SetChannel
//              
//	PURPOSE:	Stores in AI working memory the channel the AI is traversing
//				for this link.  This allows later lookup to determine 
//				dependant doors.
//              
//----------------------------------------------------------------------------

void AINavMeshLinkDoor::SetChannel(CAI* pAI, ENUM_Channels eChannel) const
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_PathInfo);
	factQuery.SetTargetObject(m_hObject);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
	
	if (!pFact)
	{
		pFact = pAI->GetAIWorkingMemory()->CreateWMFact(kFact_PathInfo);
		pFact->SetTargetObject(m_hObject);
	}

	if (pFact)
	{
		pFact->SetIndex(eChannel);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINavMeshLinkDoor::GetChannel
//              
//	PURPOSE:	Returns true if the AI had selected a channel, else returns 
//				false.  Returns by parameter the selected channel if one has 
//				been stored off for this door.
//              
//----------------------------------------------------------------------------

bool AINavMeshLinkDoor::GetChannel(CAI* pAI, const Channel** outpChannel) const
{
	if (!pAI)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::GetChannel : CAI Pointer is NULL");
		return false;
	}

	if (!outpChannel)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::GetChannel : AINavMeshLinkDoor Pointer is NULL");
		return false;
	}

	// Find a fact describing traversal of this link

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_PathInfo);
	factQuery.SetTargetObject(m_hObject);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(factQuery);
	if (!pFact)
	{
		*outpChannel = NULL;
		return false;
	}

	// Find out which channel they are traversing

	if (pFact->GetIndex() < 0 
		|| pFact->GetIndex() >= kChannel_Count)
	{
		AIASSERT(0, NULL, "AINavMeshLinkDoor::GetChannel : Invalid channel index.");
		*outpChannel = NULL;
		return false;
	}

	*outpChannel = &m_Channel[pFact->GetIndex()];

	return true;
}
