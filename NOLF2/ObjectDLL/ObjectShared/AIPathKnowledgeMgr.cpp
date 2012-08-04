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

#include "stdafx.h"
#include "AIPathKnowledgeMgr.h"
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
	m_pAI = LTNULL;
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
	m_mapPathKnowledge.clear();
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
	AIPATH_KNOWLEDGE_MAP::iterator it;
	SAVE_DWORD( m_mapPathKnowledge.size() );
	for( it = m_mapPathKnowledge.begin(); it != m_mapPathKnowledge.end(); ++it )
	{
		SAVE_COBJECT( it->first );
		SAVE_DWORD( it->second );
	}
	
	SAVE_DWORD( m_nPathKnowledgeIndex );
}

void CAIPathKnowledgeMgr::Load(ILTMessage_Read *pMsg)
{
	uint32 cKnowledge;
	LOAD_DWORD(cKnowledge);

	AIVolume *pVolume;
	CAIPathMgr::EnumPathBuildStatus eStatus;

	for( uint32 iKnowledge=0; iKnowledge < cKnowledge; ++iKnowledge )
	{
		LOAD_COBJECT( pVolume, AIVolume );
		LOAD_DWORD_CAST( eStatus, CAIPathMgr::EnumPathBuildStatus );

		m_mapPathKnowledge.insert( AIPATH_KNOWLEDGE_MAP::value_type( pVolume, eStatus ) );
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

LTBOOL CAIPathKnowledgeMgr::RegisterPathKnowledge(AIVolume *pDestVolume, CAIPathMgr::EnumPathBuildStatus eStatus)
{
	// Existing path knowledge is only valid while our index matches
	// the global index.

	if( m_nPathKnowledgeIndex != g_pAIPathMgr->GetPathKnowledgeIndex() )
	{
		ClearPathKnowledge();
		m_nPathKnowledgeIndex = g_pAIPathMgr->GetPathKnowledgeIndex();
	}

	// Search for existing knowledge of paths to the dest volume.

	AIPATH_KNOWLEDGE_MAP::iterator it = m_mapPathKnowledge.find( pDestVolume );

	// Register a new piece of path knowledge.

	if( it == m_mapPathKnowledge.end() )
	{
		if( eStatus == CAIPathMgr::kPath_NoPathFound )
		{
			AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Registering NoPathFound for %s", pDestVolume->GetName() ) );
		}
		m_mapPathKnowledge.insert( AIPATH_KNOWLEDGE_MAP::value_type( pDestVolume, eStatus ) );
		return LTTRUE;
	}

	// There is a problem if the status has changed!
	// The status of a dest volume should only change if something has happened to
	// the connectivity: e.g. a door was locked/unlocked, a volume was enabled/disabled,
	// an AI used a volume flagged as OnlyJumpDown, etc.
	// In these cases, Path Knowledge should be cleared.

	if( it->second != eStatus )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIPathKnowledgeMgr::RegisterPathKnowledge: Volume status has changed!" );
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPathKnowledgeMgr::GetPathKnowledge()
//
//	PURPOSE:	Get known status of path to dest volume, if any. 
//
// ----------------------------------------------------------------------- //

CAIPathMgr::EnumPathBuildStatus CAIPathKnowledgeMgr::GetPathKnowledge(AIVolume *pDestVolume)
{
	// Existing path knowledge is only valid while our index matches
	// the global index.

	if( m_nPathKnowledgeIndex != g_pAIPathMgr->GetPathKnowledgeIndex() )
	{
		ClearPathKnowledge();
		m_nPathKnowledgeIndex = g_pAIPathMgr->GetPathKnowledgeIndex();
		return CAIPathMgr::kPath_Unknown;
	}

	// Search for existing knowledge of paths to the dest volume.

	AIPATH_KNOWLEDGE_MAP::iterator it = m_mapPathKnowledge.find( pDestVolume );

	// Return existing knowledge.

	if( it != m_mapPathKnowledge.end() )
	{
		if( it->second == CAIPathMgr::kPath_NoPathFound )
		{
			AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Returning cached NoPathFound for %s", pDestVolume->GetName() ) );
		}

		return it->second;
	}

	// No knowledge exists.

	return CAIPathMgr::kPath_Unknown;
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
	m_mapPathKnowledge.clear();
	AITRACE( AIShowPaths, ( m_pAI->m_hObject, "Clearing cached PathKnowledge" ) );
}
