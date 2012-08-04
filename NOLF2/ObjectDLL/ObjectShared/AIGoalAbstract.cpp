// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstract.cpp
//
// PURPOSE : AIGoalAbstract abctract class implementation
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAbstract.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AINodeMgr.h"
#include "AI.h"
#include "AIPathMgr.h"
#include "ParsedMsg.h"
#include "AnimationMgr.h"
#include "AIUtils.h"
#include "AIMovement.h"

#include <algorithm>


// Globals/statics.
static LTFLOAT s_fGoalUpdateBasis = 0.0f;
extern class CAIPathMgr* g_pAIPathMgr;

//
// BlockNode
//
// Locks a node represented by an INVALID_NODE structure
//
struct BlockNode :
std::unary_function<INVALID_NODE*, int>
{
	BlockNode( HOBJECT hBlocker ) { m_hBlocker = hBlocker; }
	HOBJECT m_hBlocker;

	int operator()(INVALID_NODE* pInvalidNode) const
	{
		AIASSERT(pInvalidNode!=NULL, NULL, "BlockNode: NULL Node ptr" );
		if ( pInvalidNode->m_hNode != NULL )
		{
			AINode* pBlockedAttractorNode = (AINode*)g_pLTServer->HandleToObject(pInvalidNode->m_hNode);
			pBlockedAttractorNode->Lock( m_hBlocker );
			pInvalidNode->m_bBlocked = true;
		}
		return 0;
	}
};

//
// UnblockNode
//
// Unlocks a locked node represented by an INVALID_NODE structure
//
struct UnblockNode :
std::unary_function<INVALID_NODE*, int>
{
	UnblockNode( HOBJECT hBlocker ) { m_hBlocker = hBlocker; }
	HOBJECT m_hBlocker;

	int operator()( INVALID_NODE* pInvalidNode ) const
	{
		AIASSERT(pInvalidNode!=NULL, NULL, "BlockNode: NULL Node ptr" );
		if ( pInvalidNode->m_bBlocked )
		{
			// Test to be sure it was blocked -- if a node was added during the
			// existance of this object, then we don't want to unlock it an extra
			// time.
			pInvalidNode->m_bBlocked = false;
			AINode* pBlockedAttractorNode = (AINode*)g_pLTServer->HandleToObject(pInvalidNode->m_hNode);
			pBlockedAttractorNode->Unlock( m_hBlocker );
		}
		return 0;
	}
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract::CAIGoalAbstract()
{
	m_fBaseImportance	= 0.f;
	m_fCurImportance	= 0.f;
	m_fLastCurToBaseTime= 0.f;
	m_fLastUpdateTime	= 0.f;
	m_fNextUpdateTime	= 0.f;
	m_fDecayRate		= 0.f;
	m_bFreezeDecay		= LTFALSE;
	m_bLockedAnimIsInterruptable = LTTRUE;
	m_bForceAnimInterrupt = LTFALSE;
	m_bRequiresImmediateResponse = LTFALSE;
	m_bRequiresUpdates = LTFALSE;
	m_bPermanentGoal = LTFALSE;

	m_pGoalMgr	= LTNULL;
	m_pAI		= LTNULL;

	m_hBlockedAttractorNode = LTNULL;

	m_bScripted = LTFALSE;
	m_bDeleteGoalNextUpdate = LTFALSE;

	// Distribute updating times of goals.
	m_fUpdateTimer = s_fGoalUpdateBasis;
	s_fGoalUpdateBasis += .02f;
	if ( s_fGoalUpdateBasis > 0.5f )
	{
		s_fGoalUpdateBasis = 0.0f;
	}
}

CAIGoalAbstract::~CAIGoalAbstract()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_FLOAT(m_fBaseImportance);
	SAVE_FLOAT(m_fCurImportance);
	SAVE_TIME(m_fLastCurToBaseTime);
	SAVE_TIME(m_fLastUpdateTime);
	SAVE_TIME(m_fNextUpdateTime);
	SAVE_FLOAT(m_fUpdateTimer);
	SAVE_FLOAT(m_fDecayRate);
	SAVE_BOOL(m_bFreezeDecay);
	SAVE_BOOL(m_bLockedAnimIsInterruptable);
	SAVE_BOOL(m_bForceAnimInterrupt);
	SAVE_BOOL(m_bScripted);
	SAVE_BOOL(m_bDeleteGoalNextUpdate);
	SAVE_BOOL(m_bRequiresImmediateResponse);
	SAVE_BOOL(m_bRequiresUpdates);
	SAVE_BOOL(m_bPermanentGoal);
	SAVE_HOBJECT(m_hBlockedAttractorNode);
}

void CAIGoalAbstract::Load(ILTMessage_Read *pMsg)
{
	LOAD_FLOAT(m_fBaseImportance);
	LOAD_FLOAT(m_fCurImportance);
	LOAD_TIME(m_fLastCurToBaseTime);
	LOAD_TIME(m_fLastUpdateTime);
	LOAD_TIME(m_fNextUpdateTime);
	LOAD_FLOAT(m_fUpdateTimer);
	LOAD_FLOAT(m_fDecayRate);
	LOAD_BOOL(m_bFreezeDecay);
	LOAD_BOOL(m_bLockedAnimIsInterruptable);
	LOAD_BOOL(m_bForceAnimInterrupt);
	LOAD_BOOL(m_bScripted);
	LOAD_BOOL(m_bDeleteGoalNextUpdate);
	LOAD_BOOL(m_bRequiresImmediateResponse);
	LOAD_BOOL(m_bRequiresUpdates);
	LOAD_BOOL(m_bPermanentGoal);
	LOAD_HOBJECT(m_hBlockedAttractorNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::ActivateGoal/DeactivateGoal
//
//	PURPOSE:	Activate and deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::ActivateGoal()
{
	AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Activating Goal %s (%.2f)\n", s_aszGoalTypes[GetGoalType()], m_fCurImportance ) );
}

void CAIGoalAbstract::DeactivateGoal()
{
	AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Deactivating Goal %s (%.2f)\n", s_aszGoalTypes[GetGoalType()], m_fCurImportance ) );

	if( m_pGoalMgr->IsGoalLocked( this ) )
	{
		m_pGoalMgr->UnlockGoal( this );
	}

	// Reset NextUpdateTime.

	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	if( pTemplate->fFrequencyMax > 0.f )
	{
		m_fNextUpdateTime = g_pLTServer->GetTime() + GetRandom(pTemplate->fFrequencyMin, pTemplate->fFrequencyMax);
	}

	// Reset AIs default senses, from aibutes.txt.

	m_pAI->ResetBaseSenseFlags();

	// Goals with SenseTriggers and/or Attractors deactivate back to zero importance.

	if( ( pTemplate->aAttractors || pTemplate->flagSenseTriggers ) &&
		( m_fDecayRate > 0.f ) )
	{
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::InitGoal(CAI* pAI)
{
	AIASSERT(pAI != LTNULL, LTNULL, "CAIGoalAbstract::InitGoal: AI is NULL");
	AIASSERT(pAI->GetGoalMgr() != LTNULL, pAI->m_hObject, "CAIGoalAbstract::InitGoal: GoalMgr is NULL");

	m_pAI = pAI;
	m_pGoalMgr = m_pAI->GetGoalMgr();
}

void CAIGoalAbstract::InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime)
{
	InitGoal(pAI);

	SetBaseImportance(fImportance);

	m_fLastUpdateTime = fTime;

	// Goals with SenseTriggers and/or Attractors and/or DamageHandlers start inactive.
	// Goals that are not triggered start with their base importance.

	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	if( pTemplate->aAttractors ||
		pTemplate->flagSenseTriggers ||
		pTemplate->nDamagePriority)
	{
		m_fCurImportance = 0.f;
	}

	// Recalc CurImportance.

	RecalcImportance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::SetBaseImportance
//
//	PURPOSE:	Set base importance of goal.
//              When base changes, update curImportance as well.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::SetBaseImportance(LTFLOAT fImportance)
{
	m_fBaseImportance	= fImportance;
	m_fCurImportance	= fImportance;

	// Set decay rate based on DecayTime and Importance.

	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	if( pTemplate->fDecayTime > 0.f )
	{
		m_fDecayRate = m_fBaseImportance / pTemplate->fDecayTime;
	}
	else {
		m_fDecayRate = 0.f;
	}

	// Does this goal freeze the decay of other goals when active?

	m_bFreezeDecay = pTemplate->bFreezeDecay;

	// Can this goal's locked animations be cut off?

	m_bLockedAnimIsInterruptable = pTemplate->bLockedAnimIsInterruptable;

	// Can this goal cut off locked animations to activate?

	m_bForceAnimInterrupt = pTemplate->bForceAnimInterrupt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::UpdateGoalTimer
//
//	PURPOSE:	Decay Importance and check if it's time to Recalc Importance.
//
// ----------------------------------------------------------------------- //

LTFLOAT CAIGoalAbstract::UpdateGoalTimer(LTFLOAT fTime)
{
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

	LTFLOAT fDeltaTime = fTime - m_fLastUpdateTime;

	// Freeze goal updates while movement is locked.

	if( m_pAI->GetAIMovement()->IsMovementLocked() )
	{
		m_fLastCurToBaseTime = fTime;
	}

	// Locked goals do not decay.
	// Do not decay during the same sense update that raised the importance.
	// Do not decay if current goal freezes decays.
	// Do not decay during transition animations.
	else if( ( !m_pGoalMgr->IsGoalLocked(this) )
		&& ( m_pGoalMgr->GetCurrentGoal() )
		&& ( m_pGoalMgr->GetCurrentGoal()->GetCurImportance() > 0.f )
		&& ( m_fCurImportance > 0.f )
		&& ( m_fDecayRate > 0.f )
		&& ( ( m_fLastCurToBaseTime + m_pAI->GetSenseUpdateRate() ) < fTime )
		&& ( !m_pGoalMgr->FreezeDecay() )
		&& ( !m_pAI->GetAnimationContext()->IsTransitioning() ) )
	{
		// Triggered goals should go completely inactive
		// when another goal is active.

		if( !m_bRequiresImmediateResponse )
		{
			if( ( pTemplate->flagSenseTriggers != kSense_None ) &&
				( !m_pGoalMgr->IsCurGoal( this ) ) )
			{
				AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "Setting stimulated Goal %s to 0.00\n", s_aszGoalTypes[GetGoalType()] ) );
				m_fCurImportance = 0.f;
			}
			else {
				// Decay CurImportance by DecayRate.
				m_fCurImportance -= m_fDecayRate * fDeltaTime;
			}
		}
	}

	if( m_bRequiresUpdates ||
		( !m_pGoalMgr->HasLockedGoal() ) )
	{
		// Check if it's time to Recalc.
		if(pTemplate->fUpdateRate > 0.f)
		{
			m_fUpdateTimer += fDeltaTime;
			if(m_fUpdateTimer > pTemplate->fUpdateRate)
			{
				// Check for attractors.
				if(pTemplate->aAttractors != LTNULL)
				{
					HandleGoalAttractors();
				}

				// Recalc CurImportance.
				RecalcImportance();

				// Reset UpdateTimer.
				m_fUpdateTimer = 0.f;
			}
		}
	}

	m_fLastUpdateTime = fTime;

	return m_fCurImportance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::UpdateGoal()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::GetAlternateDeathAnimation
//
//	PURPOSE:	Give goal a chance to choose an appropriate death animation.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIGoalAbstract::GetAlternateDeathAnimation()
{
	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::SetCurToBaseImportance
//
//	PURPOSE:	Set Importance to Base, to give opportunity for activation.
//              Randomized chance of activation.
//              Check for time delay, and reset.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::SetCurToBaseImportance()
{
	LTFLOAT fCurTime = g_pLTServer->GetTime();
	if( ( fCurTime > m_fNextUpdateTime ) || m_bRequiresImmediateResponse )
	{
		AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

		// Randomize activation if ChanceToActivate is not 100%.

		LTFLOAT fChance = (pTemplate->fChanceToActivate < 1.f) ? GetRandom(0.0, 1.0f) : 1.f;
		if( pTemplate->fChanceToActivate >= fChance )
		{
			m_fCurImportance = m_fBaseImportance;
			m_fLastCurToBaseTime = fCurTime;
		}
		else
		{
			// Reset NextUpdateTime.

			SetRandomNextUpdate();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::SetRandomNextUpdate
//
//	PURPOSE:	Randomize the next time this goal can activate.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::SetRandomNextUpdate()
{
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

	if( pTemplate->fFrequencyMax > 0.f )
	{
		LTFLOAT fCurTime = g_pLTServer->GetTime();
		m_fNextUpdateTime = fCurTime + GetRandom(pTemplate->fFrequencyMin, pTemplate->fFrequencyMax);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::RecalcImportance
//
//	PURPOSE:	Recalculate Importance.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::RecalcImportance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstract::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	ASSERT(pSenseRecord && (pSenseRecord->eSenseType != kSense_InvalidType));

	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

	// This goal cares about this sense.
	if(pSenseRecord->eSenseType & pTemplate->flagSenseTriggers)
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::HandleGoalAttractors
//
//	PURPOSE:	Return Attractor AINode that triggered change in importance, if any.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAbstract::HandleGoalAttractors()
{
	AINode* pNode = FindGoalAttractors( LTFALSE, LTNULL );

	if( pNode )
	{
		SetCurToBaseImportance();
		return pNode;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::FindGoalAttractors
//
//	PURPOSE:	Return Attractor AINode that triggered change in importance, if any.
//				This overload only finds nodes matching a specified node owner.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAbstract::FindGoalAttractors(LTBOOL bRequiresOwner, HOBJECT hOwner)
{
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalAbstract::HandleGoalAttractors: Goal has no attractors.");

	// Check if attractors are triggering activateability.
	AINode* pNode;
	for(uint32 iAttractor=0; iAttractor < pTemplate->cAttractors; ++iAttractor)
	{
		if( bRequiresOwner )
		{
			// Find nearest owned node.
			pNode = g_pAINodeMgr->FindRandomOwnedNode(m_pAI, pTemplate->aAttractors[iAttractor], hOwner);
		}
		else {
			// Find nearest UNowned node.
			pNode = g_pAINodeMgr->FindNearestNodeInRadius(m_pAI, pTemplate->aAttractors[iAttractor], m_pAI->GetPosition(), pTemplate->fAttractorDistSqr * m_fBaseImportance, LTTRUE);
		}

		if(pNode != LTNULL)
		{
			return pNode;
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::BlockAttractorNodeFromSearch
//
//	PURPOSE:	Lock a node so that it will not be included in a search.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::BlockAttractorNodeFromSearch(HOBJECT hNode)
{
	// Lock the node, so that we don't find it in a search.
	if(hNode != LTNULL)
	{
		m_hBlockedAttractorNode = hNode;
		AINode* pBlockedAttractorNode = (AINode*)g_pLTServer->HandleToObject(hNode);
		pBlockedAttractorNode->Lock( m_pAI->m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::UnblockAttractorNodeFromSearch
//
//	PURPOSE:	Unlock a node that was locked to block a search.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstract::UnblockAttractorNodeFromSearch(HOBJECT hNode)
{
	AIASSERT( (hNode == LTNULL) || (m_hBlockedAttractorNode == hNode), m_pAI->m_hObject, "CAIGoalAbstract::UnblockAttractorNodeFromSearch: hNode does not match.");
	if( m_hBlockedAttractorNode != LTNULL )
	{
		AINode* pBlockedAttractorNode = (AINode*)g_pLTServer->HandleToObject(hNode);
		pBlockedAttractorNode->Unlock( m_pAI->m_hObject );
		m_hBlockedAttractorNode = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::SelectTriggeredAISound
//
//	PURPOSE:	Select an appropriate AISound.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstract::SelectTriggeredAISound(EnumAISoundType* peAISoundType)
{
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::HandleCommand
//
//	PURPOSE:	Handles a command.
//
// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::HandleCommand(const CParsedMsg &cMsg)
{
	// First token specifies the goal.  Skip it.

	char szTempBuff[256];
	char* szName;
	char* szValue;
	for(uint8 iToken=1; iToken < cMsg.GetArgCount(); ++iToken)
	{
		SAFE_STRCPY(szTempBuff, cMsg.GetArg(iToken));
		szName  = strtok(szTempBuff, "=");
		szValue = strtok(LTNULL, "");

		// Check for legal NAME=VALUE pair.
		ASSERT(szName && szValue);
		if ( !(szName && szValue) )
		{
            g_pLTServer->CPrint("Garbage name/value pair = %s", cMsg.GetArg(iToken).c_str());
			continue;
		}

		HandleNameValuePair(szName, szValue);
	}

	return true;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAIGoalAbstract::AddInvalidNode()
//
//	PURPOSE:	Adds an invalid node to the list of nodes which are filtered
//				out of searches.  This will minimize the number of unneeded
//				search hits.
//
//----------------------------------------------------------------------------
void CAIGoalAbstract::AddInvalidNode( HOBJECT hNode )
{
	using std::find_if;
	using std::bind2nd;
	using std::equal_to;

	std::vector<INVALID_NODE*>* pList = m_pAI->GetInvalidNodeList();

	// If the node is already in the list, then return.  This shouldn't
	// really happen, but it is worth checking for.
	INVALID_NODE *pNull = NULL;
	std::vector<INVALID_NODE*>::iterator it;
	it = find_if(
		pList->begin(),
		pList->end(),
		bind2nd( equal_to<INVALID_NODE*>(), pNull ));

	if ( it !=  pList->end() )
	{
		return;
	}

	INVALID_NODE* pNode = m_pAI->AddNewInvalidNode();
	pNode->m_hNode = hNode;
	pNode->m_flTime = g_pLTServer->GetTime() + 10.0f;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CAIGoalAbstract::IsPathToNodeValid()
//
//	PURPOSE:	Checks to see if there is a valid path to a node.  If there
//				is not, then add the node to the invalid node list.
//
//	NOTE:		Ugly early return to allow enabling/disabling of feature
//
//----------------------------------------------------------------------------
bool CAIGoalAbstract::IsPathToNodeValid( AINode* pNode )
{
	if ( !g_pAIButeMgr->GetRules()->bKeepInvalidNodeHistory )
	{
		return true;
	}

	if ( LTFALSE == g_pAIPathMgr->HasPath(m_pAI, pNode) )
	{
		AddInvalidNode( pNode->m_hObject );
		return false;
	}
	return true;
}


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
//	ROUTINE:	AutoNodeBlocker::AutoNodeBlocker()
//
//	PURPOSE:	Blocks the list of nodes.
//
//	NOTE:		Ugly early return to allow enabling/disabling of feature
//
//----------------------------------------------------------------------------
AutoNodeBlocker::AutoNodeBlocker( HOBJECT hBlocker, std::vector<INVALID_NODE*>* pList )
{
	if ( !g_pAIButeMgr->GetRules()->bKeepInvalidNodeHistory )
	{
		return;
	}

	// Check inputs.
	if( !pList )
	{
		AIASSERT( NULL, NULL,
			"AutoNodeBlocker: No invalid node list!" );
		return;
	}

	// Keep our blocking object around for the Lock/Unlocks.
	m_hBlocker = hBlocker;

	m_plistInvalidNodes = pList;

	using std::for_each;
	for_each(
		m_plistInvalidNodes->begin(),
		m_plistInvalidNodes->end(),
		BlockNode( m_hBlocker )
		);
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	AutoNodeBlocker::AutoNodeBlocker()
//
//	PURPOSE:	Unlocks all blocked nodes in the list.
//
//	NOTE:		Ugly early return to allow enabling/disabling of feature
//
//----------------------------------------------------------------------------
AutoNodeBlocker::~AutoNodeBlocker()
{
	if ( !g_pAIButeMgr->GetRules()->bKeepInvalidNodeHistory )
	{
		return;
	}


	AIASSERT( m_plistInvalidNodes, NULL,
		"~AutoNodeBlocker: No invalid node list!" );

	using std::for_each;
	for_each(
		m_plistInvalidNodes->begin(),
		m_plistInvalidNodes->end(),
		UnblockNode( m_hBlocker )
		);
}


//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
//	ROUTINE:	INVALID_NODE::INVALID_NODE()
//
//	PURPOSE:	Initializes the node
//
//----------------------------------------------------------------------------
INVALID_NODE::INVALID_NODE()
{
	m_bBlocked = false;
	m_hNode = NULL;
	m_flTime = 0.0f;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	INVALID_NODE::operator==()
//
//	PURPOSE:	Handles equality testing of the INVALID_NODE
//
//----------------------------------------------------------------------------
bool INVALID_NODE::operator==(const LTObjRef& rhs )
{
	if (rhs == m_hNode)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	INVALID_NODE::INVALID_NODE()
//
//	PURPOSE:	Copies a INVALID_NODE.
//
//----------------------------------------------------------------------------
INVALID_NODE::INVALID_NODE(const INVALID_NODE& rhs)
{
	m_bBlocked = rhs.m_bBlocked;
	m_hNode = rhs.m_hNode;
	m_flTime = rhs.m_flTime;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	INVALID_NODE::operator=()
//
//	PURPOSE:	Does assignment of a INVALID_NODE to another INVALID_NODE
//
//----------------------------------------------------------------------------
INVALID_NODE& INVALID_NODE::operator=(const INVALID_NODE& rhs )
{
	m_bBlocked = rhs.m_bBlocked;
	m_hNode = rhs.m_hNode;
	m_flTime = rhs.m_flTime;
	return *this;
}
