// ----------------------------------------------------------------------- //
//
// MODULE  : AIPathKnowledgeMgr.cpp
//
// PURPOSE : AIPathKnowledgeMgr implementation
//
// CREATED : 5/28/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIPathKnowledgeMgr.h"
#include "AINavMesh.h"
#include "AIUtils.h"
#include "AI.h"


DEFINE_AI_FACTORY_CLASS( CAIPathKnowledgeMgr );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::CAIPathKnowledgeMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAIPathKnowledgeMgr::CAIPathKnowledgeMgr()
{
	m_pAI = NULL;
	m_nPathKnowledgeIndex = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::~CAIPathKnowledgeMgr()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

CAIPathKnowledgeMgr::~CAIPathKnowledgeMgr()
{
	m_listPathKnowledge.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void CAIPathKnowledgeMgr::Init(CAI* pAI)
{
	m_pAI = pAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::Save/Load()
//
//	PURPOSE:	Save and Load
//
// ----------------------------------------------------------------------- //

void CAIPathKnowledgeMgr::Save(ILTMessage_Write *pMsg)
{
	AIPATH_KNOWLEDGE_LIST::iterator it;
	SAVE_DWORD( m_listPathKnowledge.size() );
	for( it = m_listPathKnowledge.begin(); it != m_listPathKnowledge.end(); ++it )
	{
		SAVE_DWORD( it->first );
		SAVE_DWORD( it->second );
	}
	
	SAVE_DWORD( m_nPathKnowledgeIndex );
}

void CAIPathKnowledgeMgr::Load(ILTMessage_Read *pMsg)
{
	uint32 cKnowledge;
	LOAD_DWORD(cKnowledge);

	CAIPathMgrNavMesh::EnumPathBuildStatus eStatus;
	ENUM_NMComponentID eComponent;

	for( uint32 iKnowledge=0; iKnowledge < cKnowledge; ++iKnowledge )
	{
		LOAD_DWORD_CAST( eComponent, ENUM_NMComponentID );
		LOAD_DWORD_CAST( eStatus, CAIPathMgrNavMesh::EnumPathBuildStatus );

		m_listPathKnowledge.push_back( AIPATH_KNOWLEDGE_LIST::value_type( eComponent, eStatus ) );
	}

	LOAD_DWORD( m_nPathKnowledgeIndex );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::RegisterPathKnowledge()
//
//	PURPOSE:	Record volumes that AI can or cannot find paths to. 
//
// ----------------------------------------------------------------------- //

bool CAIPathKnowledgeMgr::RegisterPathKnowledge(ENUM_NMPolyID eSourcePoly, ENUM_NMPolyID eDestPoly, CAIPathMgrNavMesh::EnumPathBuildStatus eStatus)
{
	// Existing path knowledge is only valid while our index matches
	// the global index.

	if( m_nPathKnowledgeIndex != g_pAIPathMgrNavMesh->GetPathKnowledgeIndex() )
	{
		ClearPathKnowledge();
		m_nPathKnowledgeIndex = g_pAIPathMgrNavMesh->GetPathKnowledgeIndex();
	}

	// Determine dest component.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( eDestPoly );
	if( !pPoly )
	{
		return false;
	}

	// Search for existing knowledge of paths to the dest poly.

	ENUM_NMComponentID eDestComponent = pPoly->GetNMComponentID();
	AIPATH_KNOWLEDGE_LIST::iterator it = m_listPathKnowledge.begin();
	for ( ; it != m_listPathKnowledge.end(); ++it )
	{
		if ( eDestComponent == it->first )
		{
			break;
		}
	}

	// Register a new piece of path knowledge.

	if( it == m_listPathKnowledge.end() )
	{
		if( eStatus == CAIPathMgrNavMesh::kPath_NoPathFound )
		{
			AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Registering NoPathFound from NavMesh Poly %d to NavMesh Poly %d", eSourcePoly, eDestPoly ) );
		}
		else if( eStatus == CAIPathMgrNavMesh::kPath_PathFound )
		{
			AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Registering PathFound from NavMesh Poly %d to NavMesh Poly %d", eSourcePoly, eDestPoly ) );
		}
		m_listPathKnowledge.push_back( AIPATH_KNOWLEDGE_LIST::value_type( eDestComponent, eStatus ) );
		return true;
	}

	// There is a problem if the status has changed!
	// The status of a dest volume should only change if something has happened to
	// the connectivity: e.g. a door was locked/unlocked, a volume was enabled/disabled,
	// an AI used a volume flagged as OnlyJumpDown, etc.
	// In these cases, Path Knowledge should be cleared.

	CAIPathMgrNavMesh::EnumPathBuildStatus eStatusExisting = it->second;
	if( eStatusExisting != eStatus )
	{
		AIASSERT1( 0, m_pAI->m_hObject, "CAIPathKnowledgeMgr::RegisterPathKnowledge: NavMesh Poly %d status has changed!", eDestPoly );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::GetPathKnowledge()
//
//	PURPOSE:	Get known status of path to dest volume, if any. 
//
// ----------------------------------------------------------------------- //

CAIPathMgrNavMesh::EnumPathBuildStatus CAIPathKnowledgeMgr::GetPathKnowledge(ENUM_NMPolyID eDestPoly)
{
	// Existing path knowledge is only valid while our index matches
	// the global index.

	if( m_nPathKnowledgeIndex != g_pAIPathMgrNavMesh->GetPathKnowledgeIndex() )
	{
		ClearPathKnowledge();
		m_nPathKnowledgeIndex = g_pAIPathMgrNavMesh->GetPathKnowledgeIndex();
		return CAIPathMgrNavMesh::kPath_Unknown;
	}

	// Determine dest component.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( eDestPoly );
	if( !pPoly )
	{
		return CAIPathMgrNavMesh::kPath_Unknown;
	}

	// Search for existing knowledge of paths to the dest volume.

	ENUM_NMComponentID eDestComponent = pPoly->GetNMComponentID();
	AIPATH_KNOWLEDGE_LIST::iterator it = m_listPathKnowledge.begin();
	for ( ; it != m_listPathKnowledge.end(); ++it )
	{
		if ( eDestComponent == it->first )
		{
			break;
		}
	}

	// Return existing knowledge.

	if( it != m_listPathKnowledge.end() )
	{
		if( it->second == CAIPathMgrNavMesh::kPath_NoPathFound )
		{
			AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Returning cached NoPathFound for NavMesh Poly %d", eDestPoly ) );
		}

		return it->second;
	}

	// No knowledge exists.

	return CAIPathMgrNavMesh::kPath_Unknown;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::ClearKnowledge()
//
//	PURPOSE:	Clear existing path knowledge. 
//
// ----------------------------------------------------------------------- //

void CAIPathKnowledgeMgr::ClearPathKnowledge()
{
	m_listPathKnowledge.erase( m_listPathKnowledge.begin(), m_listPathKnowledge.end() );
	AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Clearing cached PathKnowledge" ) );
}
