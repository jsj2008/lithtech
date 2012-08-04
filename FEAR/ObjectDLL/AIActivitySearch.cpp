// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivitySearch.cpp
//
// PURPOSE : AIActivitySearch class implementation
//
// CREATED : 8/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivitySearch.h"
#include "AI.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "AINodeMgr.h"
#include "AIStimulusMgr.h"
#include "AIBlackBoard.h"
#include "AISoundMgr.h"
#include "AISquad.h"
#include "CharacterDB.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivitySearch, kActivity_Search );

#define MIN_LOST_TIME	12.f


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivitySearch::CAIActivitySearch()
{
	m_nActivityPriority = 3;
	m_fActivityUpdateRate = 0.5f;

	m_eTaskType = kTask_LeadSearch;

	m_fActivityTimeOut = 120.f;

	m_bSearchFromLastKnownPos = false;

	m_bScriptedSearch = false;
	m_eDynamicSearchOriginPoly = kNMPoly_Invalid;

	m_eOriginComponent = kNMComponent_Invalid;
	m_eLastComponent = kNMComponent_Invalid;

	m_hOriginGuard = NULL;

	for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
	{
		m_aSearchPartys[iSearch].hSearchers[0] = NULL;
		m_aSearchPartys[iSearch].hSearchers[1] = NULL;
		m_aSearchPartys[iSearch].eCurComponent = kNMComponent_Invalid;
		m_aSearchPartys[iSearch].eLastComponent = kNMComponent_Invalid;
		m_aSearchPartys[iSearch].cNodesSearchedInComponent = 0;
		m_aSearchPartys[iSearch].cComponentsSearched = 0;
	}
}

CAIActivitySearch::~CAIActivitySearch()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	ENUM_NMComponentID eComponent;
	NMCOMPONENT_LIST::iterator itList;

	// Save Open list.

	SAVE_INT( m_lstOpen.size() );
	for( itList = m_lstOpen.begin(); itList != m_lstOpen.end(); ++itList )
	{
		eComponent = *itList;
		SAVE_DWORD( eComponent );
	}

	// Save Closed list.

	SAVE_INT( m_lstClosed.size() );
	for( itList = m_lstClosed.begin(); itList != m_lstClosed.end(); ++itList )
	{
		eComponent = *itList;
		SAVE_DWORD( eComponent );
	}

	// Save scripted search flag.

	SAVE_bool( m_bScriptedSearch );

	// Save dynamic search origin poly.

	SAVE_DWORD( m_eDynamicSearchOriginPoly );

	// Save search from last known position flag.

	SAVE_bool( m_bSearchFromLastKnownPos );

	// Save origin component.

	SAVE_DWORD( m_eOriginComponent );

	// Save last component occupied.

	SAVE_DWORD( m_eLastComponent );

	// Save origin guard.

	SAVE_HOBJECT( m_hOriginGuard );

	// Save search partys.

	for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
	{
		SAVE_HOBJECT( m_aSearchPartys[iSearch].hSearchers[0] );
		SAVE_HOBJECT( m_aSearchPartys[iSearch].hSearchers[1] );
		SAVE_DWORD( m_aSearchPartys[iSearch].eCurComponent );
		SAVE_DWORD( m_aSearchPartys[iSearch].eLastComponent );
		SAVE_DWORD( m_aSearchPartys[iSearch].cNodesSearchedInComponent );
		SAVE_DWORD( m_aSearchPartys[iSearch].cComponentsSearched );
	}
}

// ----------------------------------------------------------------------- //

void CAIActivitySearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	uint32 cComponents;
	uint32 iComponent;
	ENUM_NMComponentID eComponent;

	// Load Open list.

	LOAD_INT( cComponents );
	m_lstOpen.reserve( cComponents );
	for( iComponent=0; iComponent < cComponents; ++iComponent )
	{
		LOAD_DWORD_CAST( eComponent, ENUM_NMComponentID );
		m_lstOpen.push_back( eComponent );
	}

	// Load Closed list.

	LOAD_INT( cComponents );
	m_lstClosed.reserve( cComponents );
	for( iComponent=0; iComponent < cComponents; ++iComponent )
	{
		LOAD_DWORD_CAST( eComponent, ENUM_NMComponentID );
		m_lstClosed.push_back( eComponent );
	}

	// Load scripted search flag.

	LOAD_bool( m_bScriptedSearch );

	// Load dynamic search origin poly.

	LOAD_DWORD_CAST( m_eDynamicSearchOriginPoly, ENUM_NMPolyID );

	// Load search from target's last known position flag.

	LOAD_bool( m_bSearchFromLastKnownPos );

	// Load origin component.

	LOAD_DWORD_CAST( m_eOriginComponent, ENUM_NMComponentID );

	// Load last component occupied.

	LOAD_DWORD_CAST( m_eLastComponent, ENUM_NMComponentID );

	// Save origin guard.

	LOAD_HOBJECT( m_hOriginGuard );

	// Load search partys.

	for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
	{
		LOAD_HOBJECT( m_aSearchPartys[iSearch].hSearchers[0] );
		LOAD_HOBJECT( m_aSearchPartys[iSearch].hSearchers[1] );
		LOAD_DWORD_CAST( m_aSearchPartys[iSearch].eCurComponent, ENUM_NMComponentID );
		LOAD_DWORD_CAST( m_aSearchPartys[iSearch].eLastComponent, ENUM_NMComponentID );
		LOAD_DWORD( m_aSearchPartys[iSearch].cNodesSearchedInComponent );
		LOAD_DWORD( m_aSearchPartys[iSearch].cComponentsSearched );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::FindLeader
//
//	PURPOSE:	Return true if successfully found leader.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::FindLeader()
{
	// Don't try to search if no search nodes exist.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( kNode_Search );
	if( !( pNodeList && pNodeList->size() > 0 ) )
	{
		return false;
	}

	// Found someone ordered to search.

	m_bScriptedSearch = false;
	if( super::FindLeader() )
	{
		m_bScriptedSearch = true;
		return true;
	}

	//
	// Find someone with the desire to search.
	//

	CAI* pCurAI;
	HOBJECT hLeader = NULL;
	ENUM_NMPolyID ePolyOrigin = kNMPoly_Invalid;
	double fCurTime = g_pLTServer->GetTime();

	CAIWMFact* pFact;
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Search);
	uint32 iPotential;
	for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		// Bail if no AI.

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		if( pCurAI->HasTarget( kTarget_Character ) )
		{
			// Squad is not lost if someone still knows of the target's position.

			if( pCurAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & ( kTargetTrack_Normal | kTargetTrack_SeekEnemy ) )
			{
				return false;
			}

			// Squad is not lost if someone knew of the target's position recently.

			if( pCurAI->GetAIBlackBoard()->GetBBTargetLostTime() > fCurTime - MIN_LOST_TIME )
			{
				return false;
			}
		}

		// Found someone with the desire to search.

		pFact = pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			hLeader = pCurAI->m_hObject;
			ePolyOrigin = g_pAIQuadTree->GetContainingNMPoly( pFact->GetPos(), pCurAI->GetCharTypeMask(), kNMPoly_Invalid, pCurAI );
		}
	}


	//
	// Find someone eligible for a dynamic search.
	//

	// Iterate over participants.
	// Search for an AI who has lost his target.

	if( !hLeader )
	{
		for( iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
		{
			// Bail if no AI.

			pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
			if( !pCurAI )
			{
				continue;
			}

			// Skip AI who are not targeting anyone.

			if( !pCurAI->HasTarget( kTarget_Character ) )
			{
				continue;
			}

			// Squad is not lost if someone still knows of the target's position.

			if( pCurAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & ( kTargetTrack_Normal | kTargetTrack_SeekEnemy ) )
			{
				return false;
			}

			// Squad is not lost if someone knew of the target's position recently.

			if( pCurAI->GetAIBlackBoard()->GetBBTargetLostTime() > fCurTime - MIN_LOST_TIME )
			{
				return false;
			}

			// Found a lost AI.

			hLeader = pCurAI->m_hObject;
			CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( hLeader );
			if( pLeader )
			{
				m_bSearchFromLastKnownPos = true;
				ePolyOrigin = pLeader->GetAIBlackBoard()->GetBBTargetLostNavMeshPoly();
			}
		}
	}

	// Bail if the leader cannot find a path to the search origin.

	if( IsAI( hLeader ) )
	{
		CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( hLeader );
		if( !g_pAIPathMgrNavMesh->HasPath( pLeader, pLeader->GetCharTypeMask(), ePolyOrigin ) )
		{
			return false;
		}
	}

	// Always cover the leader in dynamic situations.

	if( hLeader )
	{
		m_bAdvanceCovered = true;
	}

	// Return true if a leader was found.

	m_eDynamicSearchOriginPoly = ePolyOrigin;
	m_hAdvanceAI = hLeader;
	return !!m_hAdvanceAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ActivateActivity
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::ActivateActivity()
{
	if( !super::ActivateActivity() )
	{
		return false;
	}

	// Bail if no leader.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( !pLeader )
	{
		return false;
	}

	// Scripted search origin is at a node.

	ENUM_NMPolyID ePolyOrigin = kNMPoly_Invalid;
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hAdvanceNode );
	if( pNode )
	{
		ePolyOrigin = pNode->GetNodeContainingNMPoly();
	}

	// Dynamic searches start from where the leader lost the target.

	else
	{
		ePolyOrigin = m_eDynamicSearchOriginPoly;
	}

	// Bail if no containing poly.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePolyOrigin );
	if( !pPoly )
	{
		return false;
	}

	// Record the component of search origin.

	m_eOriginComponent = pPoly->GetNMComponentID();

	// Start by exploring the component of search origin.

	AddComponentToList( m_eOriginComponent, m_lstOpen );

	// Clear all partipant's knowledge of threats.
	// This ensures that they obey squad orders.

	ClearAIThreatKnowledge( pLeader );

	CAI* pFollower;
	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		if( !m_aFollowAI[iFollow].hFollowAI )
		{
			break;
		}

		pFollower = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		ClearAIThreatKnowledge( pFollower );

		// Last AI waits at the border to the search origin component.

		m_hOriginGuard = m_aFollowAI[iFollow].hFollowAI;
	}

	// Ensure everyone is suspicious.

	ForceAISuspicious();

	// "Regroup!"
	// "Roger"

	LTRect3f AABB;
	CalcActivityAABB( &AABB );
	float fMaxWidth = 3.f * ( m_cPotentialParticipants * ( pLeader->GetRadius() * 2.f ) );
	if( ( AABB.GetDepth() > fMaxWidth ) ||
		( AABB.GetWidth() > fMaxWidth ) )
	{
		g_pAISoundMgr->RequestAISound( pLeader->m_hObject, kAIS_Regroup, kAISndCat_Event, NULL, 0.f );

		// Find a follower far enough to respond.

		for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
		{
			pFollower = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
			if( pFollower && pFollower->GetPosition().DistSqr( pLeader->GetPosition() ) > fMaxWidth * fMaxWidth )
			{
				g_pAISoundMgr->RequestAISoundSequence( pFollower->m_hObject, kAIS_Affirmative, pLeader->m_hObject, kAIS_Regroup, kAIS_Regroup, kAISndCat_Event, NULL, 0.3f );
				break;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::DeactivateActivity
//
//	PURPOSE:	Deactivate an active activity.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::DeactivateActivity()
{
	// AI are suspicious after a failed search.

	if( !m_bEnemySpotted )
	{
		ForceAISuspicious();

		// Clear knowledge of bodies they may have come across while searching.

		CAI* pCurAI;
		for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
		{
			// Bail if no AI.

			pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
			if( pCurAI )
			{
				ClearAIThreatKnowledge( pCurAI );

				// Start with a clean slate, in terms of reacting to a surprise threat.

				CAIWMFact factQuery;
				factQuery.SetFactType( kFact_Knowledge );
				factQuery.SetKnowledgeType( kKnowledge_FirstDangerTime );
				pCurAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
				pCurAI->GetAIBlackBoard()->SetBBTargetLastVisibleTime( 0.f );
			}
		}
	}

	m_lstOpen.resize( 0 );
	m_lstClosed.resize( 0 );

	m_bSearchFromLastKnownPos = false;

	m_hOriginGuard = NULL;

	super::DeactivateActivity();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ForceAISuspicious
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::ForceAISuspicious()
{
	// Leader is suspicious.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( pLeader )
	{
		g_pCmdMgr->QueueMessage( pLeader, pLeader, "AWARENESS SUSPICIOUS" );
	}

	// Followers are suspicious.

	CAI* pFollower;
	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		if( !m_aFollowAI[iFollow].hFollowAI )
		{
			continue;
		}

		pFollower = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		g_pCmdMgr->QueueMessage( pFollower, pFollower, "AWARENESS SUSPICIOUS" );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ClearAIThreatKnowledge
//
//	PURPOSE:	Clear previous knowledge of threats.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::ClearAIThreatKnowledge( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Clear everything that may cause AI to ignore the search.

	// Clear previous target.

	g_pCmdMgr->QueueMessage( pAI, pAI, "TARGET NONE" );

	// Clear memories of threats.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Character );
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	factQuery.SetFactType( kFact_Disturbance );
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Do not seek enemy.

	pAI->GetAISensorMgr()->RemoveAISensor( kSensor_SeekEnemy );

	// Re-evaluate goals.

	pAI->GetAIBlackBoard()->SetBBInvalidatePlan( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::UpdateActivity
//
//	PURPOSE:	Return true if Activity should continue running.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::UpdateActivity()
{
	// Bail if a threat has been detected.

	if( ( g_pLTServer->GetTime() > m_fActivityActivateTime ) &&
		( ThreatDetectedBySquad() ) )
	{
		return false;
	}

	// Update orderly advance.

	if( !super::UpdateActivity() )
	{
		return false;
	}

	// Advancing.

	if( m_eActStatus == kActStatus_Advancing )
	{
		CloseComponentOnExit();
		if( WaitAtComponentBorder( m_hOriginGuard, m_eOriginComponent ) )
		{
			HandleAdvanceNodeArrival();
		}
	}

	// Searching.

	if( m_eActStatus == kActStatus_Searching )
	{
		bool bContinueSearching = false;
		for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
		{
			if( m_aSearchPartys[iSearch].hSearchers[0] && 
				UpdateSearchParty( m_aSearchPartys[iSearch] ) )
			{
				bContinueSearching = true;
			}

			// "He's not here"

			else {
				g_pAISoundMgr->RequestAISound( m_aSearchPartys[iSearch].hSearchers[0], kAIS_SearchFailed, kAISndCat_Event, NULL, 0.f );
			}
		}

		return bContinueSearching;
	}

	// Continue updating.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ThreatDetectedBySquad
//
//	PURPOSE:	Return true if the squad has detected a threat.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::ThreatDetectedBySquad()
{
	CAI* pCurAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		// Bail if no AI.

		pCurAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iPotential] );
		if( !pCurAI )
		{
			continue;
		}

		// AI is targeting a threat of some kind.

		if( pCurAI->HasTarget( kTarget_Character | kTarget_Disturbance | kTarget_Object ) )
		{
			HOBJECT hThreat = pCurAI->GetAIBlackBoard()->GetBBTargetObject();
			if( IsCharacter( hThreat ) )
			{
				CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hThreat );
				EnumCharacterStance eStance = g_pCharacterDB->GetStance( pCurAI->GetAlignment(), pChar->GetAlignment() );
				if( eStance == kCharStance_Hate )
				{
					AITRACE( AIShowActivities, ( pCurAI->m_hObject, "Detected a threat!" ) );
					AITRACE( AIShowActivities, ( pCurAI->GetAIBlackBoard()->GetBBTargetObject(), "<-- THIS IS THE THREAT" ) );
					m_bEnemySpotted = true;
					return true;
				}
			}
		}
	}

	// No threat detected.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::BeginAdvance
//
//	PURPOSE:	Kick-off the advance.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::BeginAdvance()
{
	// Intentionally do NOT call super::BeginAdvance, 
	// We need to first find a search node to advance to.

	// Bail if no leader.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( !pLeader )
	{
		return true;
	}

	// Leader waits for followers to begin moving before advancing.

	if( !SquadRegroup() )
	{
		// Leader stands alert.

		g_pCmdMgr->QueueMessage( pLeader, pLeader, "ALERT" );
		return true;
	}

	// Dynamic search from the last known position.

	if( m_bSearchFromLastKnownPos )
	{
		// Order AI to search the target's last known position.

		g_pCmdMgr->QueueMessage( pLeader, pLeader, "PRIVATE_SEARCH_LOST_TARGET" );
		AITRACE( AIShowActivities, ( pLeader->m_hObject, "Ordered to Search." ) );
	}

	// Search from some starting search node.

	else 
	{
		// Select a new search origin component to explore.

		m_eOriginComponent = m_lstOpen.front();

		// Expand neighbors of next component to explore.

		ExploreComponent( m_eOriginComponent );

		// Truly begin the advance when a node has been found.

		AINode* pNode = g_pAINodeMgr->FindNodeInComponent( pLeader, kNode_Search, m_eOriginComponent, m_fActivityActivateTime, m_bScriptedSearch );
		if( !pNode )
		{
			// No more components to explore.

			if( m_lstOpen.empty() )
			{
				AITRACE( AIShowActivities, ( m_hAdvanceAI, "Cannot find any search nodes! Activity bailing." ) );
				m_eActStatus = kActStatus_Failed;
				return false;
			}

			return true;
		}

		// We are ready to advance to the node.

		m_hAdvanceNode = pNode->m_hObject;

		// Order AI to search the node.

		char szMsg[128];
		LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "PRIVATE_SEARCH %s", pNode->GetNodeName() );		
		g_pCmdMgr->QueueMessage( pLeader, pLeader, szMsg );
		AITRACE( AIShowActivities, ( pLeader->m_hObject, "Ordered to Search." ) );
	}

	// Leader stops standing alert.

	g_pCmdMgr->QueueMessage( pLeader, pLeader, "ALERT 0" );

	// "Find him!"

	if( m_cPotentialParticipants > 1 )
	{
		g_pAISoundMgr->RequestAISound( pLeader->m_hObject, kAIS_OrderSearch, kAISndCat_Event, NULL, 0.f );
	}

	m_eActStatus = kActStatus_Advancing;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::CloseComponentOnExit
//
//	PURPOSE:	Add components to closed list as leader exits them.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::CloseComponentOnExit()
{
	// Sanity check.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( !pAI )
	{
		return;
	}

	// Determine the leader's current component.

	ENUM_NMComponentID eComponent = kNMComponent_Invalid;
	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pAI->GetCurrentNavMeshPoly() );
	if( pPoly )
	{
		eComponent = pPoly->GetNMComponentID();
	}

	// Leader exited a component, so close it out.

	if( m_eLastComponent != eComponent )
	{
		AddComponentToList( m_eLastComponent, m_lstClosed );
		RemoveComponentFromList( m_eLastComponent, m_lstOpen );
		m_eLastComponent = eComponent;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::HandleAdvanceNodeArrival
//
//	PURPOSE:	Handle arriving at the advance destination node.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::HandleAdvanceNodeArrival()
{
	// Complete the advance.

	super::ClearOrders();

	// Divide searchers into partys of two.

	CAI* pAI;
	uint32 iFollow = 0;
	uint32 iParty = 0;
	uint32 iSearcher = 1;
	m_aSearchPartys[0].hSearchers[0] = m_hAdvanceAI;
	m_aSearchPartys[0].eCurComponent = m_eOriginComponent;
	while( iFollow < MAX_AI_FOLLOW )
	{
		// Origin guard does not join a party, and just waits 
		// guarding the search origin.

		if( m_hOriginGuard == m_aFollowAI[iFollow].hFollowAI )
		{
			break;
		}

		// Followers wait alert.

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT" );

		m_aSearchPartys[iParty].hSearchers[iSearcher] = m_aFollowAI[iFollow].hFollowAI;
		++iFollow;
		++iSearcher;

		if( iSearcher > 1 )
		{
			iSearcher = 0;
			++iParty;
		}
	}

	// Origin guard goes alert.

	if( !IsDeadAI( m_hOriginGuard ) )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hOriginGuard );
		g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT" );

		// "I'll wait here"

		g_pAISoundMgr->RequestAISound( m_hOriginGuard, kAIS_SearchGuard, kAISndCat_Event, NULL, 0.f );
	}

	// Start searching.

	AITRACE( AIShowActivities, ( m_hAdvanceAI, "Done advancing. Commencing search." ) );
	m_eActStatus = kActStatus_Searching;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::UpdateSearchParty
//
//	PURPOSE:	Return true if the search party is still searching.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::UpdateSearchParty( SAI_SEARCH_PARTY& SearchParty )
{
	// Sanity check.

	if( !SearchParty.hSearchers[0] )
	{
		return false;
	}

	// Searcher does not exist.

	CAI* pSearcher = (CAI*)g_pLTServer->HandleToObject( SearchParty.hSearchers[0] );
	if( !pSearcher )
	{
		return false;
	}

	// Follower waits at the border of the component.
	// (e.g. guarding the door).

	CAI* pFollower = (CAI*)g_pLTServer->HandleToObject( SearchParty.hSearchers[1] );
	if( pFollower )
	{
		if( WaitAtComponentBorder( pFollower->m_hObject, SearchParty.eCurComponent ) )
		{
			g_pCmdMgr->QueueMessage( pFollower, pFollower, "ALERT" );
		}
	}

	// AI is busy searching at node, or is en route to the node.

	if( IsSearchInProgress( pSearcher ) )
	{
		return true;
	}

	// Select a node to search in the current component.
	// If no node is found, continue exploring components on subsequent
	// updates until a node is found, or all components are exhausted.

	if( SearchParty.cNodesSearchedInComponent < 2 )
	{
		AINode* pNode = g_pAINodeMgr->FindNodeInComponent( pSearcher, kNode_Search, SearchParty.eCurComponent, m_fActivityActivateTime, m_bScriptedSearch );
		if( pNode )
		{
			SearchParty.cNodesSearchedInComponent++;
			if( SearchParty.eCurComponent != SearchParty.eLastComponent )
			{
				// Swap leader and follower, so AI closest to door exits first.
				// Don't swap in the first component of the search.

				if( SearchParty.hSearchers[1] &&
					( SearchParty.eLastComponent != kNMComponent_Invalid ) &&
					( SearchParty.eLastComponent != m_eOriginComponent ) )
				{
					SearchParty.hSearchers[0] = SearchParty.hSearchers[1];
					SearchParty.hSearchers[1] = pSearcher->m_hObject;

					pSearcher = (CAI*)g_pLTServer->HandleToObject( SearchParty.hSearchers[0] );
					pFollower = (CAI*)g_pLTServer->HandleToObject( SearchParty.hSearchers[1] );
				}

				SearchParty.eLastComponent = SearchParty.eCurComponent;
			}

			// Order AI to search the node.

			char szMsg[128];
			LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "PRIVATE_SEARCH %s", pNode->GetNodeName() );		
			g_pCmdMgr->QueueMessage( pSearcher, pSearcher, szMsg );
			g_pCmdMgr->QueueMessage( pSearcher, pSearcher, "FOLLOW NONE" );
			g_pCmdMgr->QueueMessage( pSearcher, pSearcher, "ALERT 0" );
			AITRACE( AIShowActivities, ( pSearcher->m_hObject, "Ordered to Search." ) );

			// Order other AI to follow.

			if( pFollower )
			{
				// Don't follow if already in the component.

				CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pFollower->GetCurrentNavMeshPoly() );
				if( !( pPoly && pPoly->GetNMComponentID() == SearchParty.eCurComponent ) )
				{
					LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "FOLLOW %s", pSearcher->GetName() );		
					g_pCmdMgr->QueueMessage( pFollower, pFollower, szMsg );
					g_pCmdMgr->QueueMessage( pFollower, pFollower, "ALERT 0" );
				}
				else {
					g_pCmdMgr->QueueMessage( pFollower, pFollower, "ALERT" );
				}

				g_pCmdMgr->QueueMessage( pFollower, pFollower, "PRIVATE_SEARCH NONE" );
				AITRACE( AIShowActivities, ( pFollower->m_hObject, "Ordered to Follow." ) );
			}

			// Announce searching a new component.

			if( SearchParty.cNodesSearchedInComponent == 1 )
			{
				PlaySearchLocationSound( SearchParty, pNode );
			}

			return true;
		}
	}

	// No more components to explore.

	if( m_lstOpen.empty() )
	{
		return false;
	}

	// Assign search party a new component to explore.

	SearchParty.eCurComponent = m_lstOpen.front();
	if( SearchParty.cNodesSearchedInComponent > 0 )
	{
		SearchParty.cComponentsSearched++;
		SearchParty.cNodesSearchedInComponent = 0;
	}

	// Expand neighbors of next component to explore.

	ExploreComponent( SearchParty.eCurComponent );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::PlaySearchLocationSound
//
//	PURPOSE:	Play sound annoucning searching a new component.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::PlaySearchLocationSound( SAI_SEARCH_PARTY& SearchParty, AINode* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return;
	}

	HOBJECT hSearcher = SearchParty.hSearchers[0];
	HOBJECT hFollower = SearchParty.hSearchers[1];

	// Follower announces location.
	// e.g. "Check the office"

	if( hFollower && ( pNode->GetType() == kNode_Search ) )
	{
		AINodeSearch* pNodeSearch = (AINodeSearch*)pNode;
		EnumAISoundType eLocationSound = pNodeSearch->GetLocationAISoundType();
		if( ValidateSearchLocationSound( hFollower, eLocationSound ) )
		{
			g_pAISoundMgr->RequestAISound( hFollower, eLocationSound, kAISndCat_Event, NULL, 0.f );
			return;
		}
	}

	// Searcher says "Clear!"

	if( SearchParty.cComponentsSearched > 1 )
	{
		g_pAISoundMgr->RequestAISound( hSearcher, kAIS_SearchClear, kAISndCat_Event, NULL, 0.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ValidateSearchLocationSound
//
//	PURPOSE:	Return true if search location sound is appropriate to play.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::ValidateSearchLocationSound( HOBJECT hAI, EnumAISoundType eLocationSound )
{
	// Sanity check.

	if( ( hAI == NULL ) ||
		( eLocationSound == kAIS_InvalidType ) )
	{
		return false;
	}

	// Some locations have complementary locations.
	// e.g. Inside/Outside, Upstairs/Downstairs.
	// If there is no complementary location, sound is Ok to play.

	EnumAISoundType eComplementarySound = kAIS_InvalidType;
	switch( eLocationSound )
	{
		// Inside / Outside.

		case kAIS_SearchCheckInside:
			eComplementarySound = kAIS_SearchCheckOutside;
			break;

		case kAIS_SearchCheckOutside:
			eComplementarySound = kAIS_SearchCheckInside;
			break;

		// Upstairs / Downstairs.

		case kAIS_SearchCheckUpstairs:
			eComplementarySound = kAIS_SearchCheckDownstairs;
			break;

		case kAIS_SearchCheckDownstairs:
			eComplementarySound = kAIS_SearchCheckUpstairs;
			break;

		// No complementary sound exists, so no further checks 
		// are required.  Just play the sound.

		default:  return true;
	}

	// Bail if AI is not in the NavMesh.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hAI );
	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pAI->GetCurrentNavMeshPoly() );
	if( !pPoly )
	{
		return false;
	}

	// Bail if poly is not in a component.

	ENUM_NMComponentID eComponent = pPoly->GetNMComponentID();
	if( eComponent == kNMComponent_Invalid )
	{
		return false;
	}

	// No nodes exist.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( kNode_Search );
	if( !pNodeList )
	{
		return false;
	}

	// Find a node in the AI's current component that has
	// the complementary sound.

	CAINavMeshPoly* pNodePoly;
	AINodeSearch* pNodeSearch;
	AINODE_LIST::iterator itNode;
	for( itNode = pNodeList->begin(); itNode != pNodeList->end(); ++itNode )
	{
		// Skip nodes that do not have the complementary sound.

		pNodeSearch = (AINodeSearch*)(*itNode);
		if( pNodeSearch->GetLocationAISoundType() != eComplementarySound )
		{
			continue;
		}

		// Found a node in the AI's current component that has
		// the complementary sound.

		pNodePoly = g_pAINavMesh->GetNMPoly( pNodeSearch->GetNodeContainingNMPoly() );
		if( pNodePoly && ( pNodePoly->GetNMComponentID() == eComponent ) )
		{
			return true;
		}
	}

	// AI could not find a node with the complementary sound
	// in his current component.  Do not play the location sound.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ClearOrders
//
//	PURPOSE:	Clears orders issued during activity.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::ClearOrders()
{
	super::ClearOrders();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Search);

	// Clear orders.

	// Leader.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hAdvanceAI );
	if( pAI )
	{
		g_pCmdMgr->QueueMessage( pAI, pAI, "SEARCH NONE" );
		g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
		g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SEARCH NONE" );
		g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT 0" );

		pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	}

	// Followers.

	for( uint32 iFollow=0; iFollow < m_cFollowAI; ++iFollow )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_aFollowAI[iFollow].hFollowAI );
		if( pAI )
		{
			g_pCmdMgr->QueueMessage( pAI, pAI, "SEARCH NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SEARCH NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT 0" );

			pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	
			pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
		}
	}

	// Searcher parties.

	for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
	{
		if( m_aSearchPartys[iSearch].hSearchers[0] )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( m_aSearchPartys[iSearch].hSearchers[0] );
			if( pAI )
			{
				g_pCmdMgr->QueueMessage( pAI, pAI, "SEARCH NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SEARCH NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT 0" );

				pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

				pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
			}
		}

		if( m_aSearchPartys[iSearch].hSearchers[1] )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( m_aSearchPartys[iSearch].hSearchers[1] );
			if( pAI )
			{
				g_pCmdMgr->QueueMessage( pAI, pAI, "SEARCH NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SEARCH NONE" );
				g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT 0" );

				pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

				pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
			}
		}
	}

	// Origin guard.

	if( !IsDeadAI( m_hOriginGuard ) )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_hOriginGuard );
		if( pAI )
		{
			g_pCmdMgr->QueueMessage( pAI, pAI, "SEARCH NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "PRIVATE_SEARCH NONE" );
			g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT 0" );

			pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

			pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::IsSearchInProgress
//
//	PURPOSE:	Return true if AI is busy searching.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::IsSearchInProgress( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI is at a search node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		HOBJECT hAtNode = pProp->hWSValue;
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hAtNode );
		if( pNode && ( pNode->GetType() == kNode_Search ) )
		{
			// We are done searching.

			SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_UsingObject, pAI->m_hObject );
			if( pProp && ( pProp->hWSValue == hAtNode ) )
			{
				return false;
			}

			// We can no longer search this node.

			if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), NULL, kThreatPos_TargetPos, kNodeStatus_All ) )
			{
				return false;
			}

			// We are still searching.

			return true;
		}
	}

	// AI is en route to a search node.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Search );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		return true;
	}

	// AI is not searching.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::WaitAtComponentBorder
//
//	PURPOSE:	Return true if AI is waiting at the border of the 
//              component being searched.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::WaitAtComponentBorder( HOBJECT hAI, ENUM_NMComponentID eComponent )
{
	// Sanity check.

	CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hAI );
	if( !pAI )
	{
		return false;
	}

	// AI is not moving.

	if( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
	{
		return false;
	}

	// AI is not in the specified component.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pAI->GetCurrentNavMeshPoly() );
	if( !( pPoly && pPoly->GetNMComponentID() == eComponent ) )
	{
		return false;
	}

	// Stop following the searcher.

	g_pCmdMgr->QueueMessage( pAI, pAI, "FOLLOW NONE" );

	// Stand alert.

	g_pCmdMgr->QueueMessage( pAI, pAI, "ALERT" );

	// AI is waiting at the border.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::IsComponentInList
//
//	PURPOSE:	Return true if component exists in a list.
//
// ----------------------------------------------------------------------- //

bool CAIActivitySearch::IsComponentInList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return false;
	}

	// Find component in list.

	NMCOMPONENT_LIST::iterator itComponent;
	for( itComponent = lstComponents.begin(); itComponent != lstComponents.end(); ++itComponent )
	{
		if( eComponent == *itComponent )
		{
			return true;
		}
	}

	// Component is not in list.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::AddComponentToList
//
//	PURPOSE:	Add a component to a list.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::AddComponentToList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return;
	}

	// Ensure ID is not already in the list.

	if( IsComponentInList( eComponent, lstComponents ) )
	{
		return;
	}

	// Add the component to the list.

	lstComponents.push_back( eComponent );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::RemoveComponentFromList
//
//	PURPOSE:	Remove component from a list.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::RemoveComponentFromList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents )
{
	// Sanity check.

	if( eComponent == kNMComponent_Invalid )
	{
		return;
	}

	// Erase component from list.

	NMCOMPONENT_LIST::iterator itComponent;
	for( itComponent = lstComponents.begin(); itComponent != lstComponents.end(); ++itComponent )
	{
		if( eComponent == *itComponent )
		{
			lstComponents.erase( itComponent );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ExploreComponent
//
//	PURPOSE:	Add component's neighbors to Open, and component to Closed.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::ExploreComponent( ENUM_NMComponentID eComponent )
{
	// Sanity check.

	CAINavMeshComponent* pComponent = g_pAINavMesh->GetNMComponent( eComponent );
	if( !pComponent )
	{
		return;
	}

	// Add neighbors to Open list.

	CAINavMeshComponent* pNeighbor;
	int cNeighbors = pComponent->GetNumNMComponentNeighbors();
	for( int iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
	{
		pNeighbor = pComponent->GetNMComponentNeighbor( iNeighbor );
		if( pNeighbor )
		{
			// Don't re-add components that are already explored.

			if( !IsComponentInList( pNeighbor->GetNMComponentID(), m_lstClosed ) )
			{
				AddComponentToList( pNeighbor->GetNMComponentID(), m_lstOpen );
			}
		}
	}

	// Add component to Closed list.

	AddComponentToList( eComponent, m_lstClosed );

	// Remove component from Open list.

	RemoveComponentFromList( eComponent, m_lstOpen );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivitySearch::ClearDeadAI
//
//	PURPOSE:	Clear handles to dead AI.
//
// ----------------------------------------------------------------------- //

void CAIActivitySearch::ClearDeadAI()
{
	super::ClearDeadAI();

	for( uint32 iSearch=0; iSearch < MAX_AI_SEARCH; ++iSearch )
	{
		if( m_aSearchPartys[iSearch].hSearchers[0] && IsDeadAI( m_aSearchPartys[iSearch].hSearchers[0] ) )
		{
			m_aSearchPartys[iSearch].hSearchers[0] = NULL;
		}

		if( m_aSearchPartys[iSearch].hSearchers[1] && IsDeadAI( m_aSearchPartys[iSearch].hSearchers[1] ) )
		{
			m_aSearchPartys[iSearch].hSearchers[1] = NULL;
		}

		// Always occupy the first slot of possible.

		if( m_aSearchPartys[iSearch].hSearchers[1] && !( m_aSearchPartys[iSearch].hSearchers[0] ) )
		{
			m_aSearchPartys[iSearch].hSearchers[0] = m_aSearchPartys[iSearch].hSearchers[1];
			m_aSearchPartys[iSearch].hSearchers[1] = NULL;
		}
	}

	if( m_hOriginGuard && IsDeadAI( m_hOriginGuard ) )
	{
		m_hOriginGuard = NULL;
	}
}
