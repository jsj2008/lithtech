// ----------------------------------------------------------------------- //
//
// MODULE  : AICentralKnowledgeMgr.cpp
//
// PURPOSE : AICentralKnowledgeMgr implementation
//
// CREATED : 4/6/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AICentralKnowledgeMgr.h"
#include "AIUtils.h"


DEFINE_AI_FACTORY_CLASS(CAICentralKnowledgeRecord);


// Globals / Statics

CAICentralKnowledgeMgr* g_pAICentralKnowledgeMgr = LTNULL;


//
// CAICentralKnowledgeRecord functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeRecord::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAICentralKnowledgeRecord::CAICentralKnowledgeRecord()
{
	if( g_pAICentralKnowledgeMgr )
	{
		m_hAI.SetReceiver( *g_pAICentralKnowledgeMgr );
		m_hKnowledgeTarget.SetReceiver( *g_pAICentralKnowledgeMgr );
	}
	else
	{
		AIASSERT( 0, NULL, "No g_pAICentralKnowledgeMgr." );
	}

	m_eKnowledgeType = kCK_InvalidType;

	m_hAI = LTNULL;
	m_pAI = LTNULL;
	m_hKnowledgeTarget = LTNULL;
	m_pKnowledgeTarget = LTNULL;
	m_bLinkKnowledge = LTTRUE;

	m_fKnowledgeData = 0.f;
	m_bKnowledgeDataIsTime = LTFALSE;
}

CAICentralKnowledgeRecord::~CAICentralKnowledgeRecord()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeRecord::Save/load
//
//	PURPOSE:	Save and load
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeRecord::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD( m_eKnowledgeType );
	SAVE_COBJECT( m_pAI );
	SAVE_COBJECT( m_pKnowledgeTarget );
	SAVE_BOOL( m_bLinkKnowledge );

	SAVE_BOOL( m_bKnowledgeDataIsTime );
	if( m_bKnowledgeDataIsTime )
	{
		SAVE_TIME( m_fKnowledgeData );
	}
	else {
		SAVE_FLOAT( m_fKnowledgeData );
	}
}

void CAICentralKnowledgeRecord::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eKnowledgeType, EnumAICentralKnowledgeType );
	LOAD_COBJECT( m_pAI, ILTBaseClass );
	m_hAI = (m_pAI) ? m_pAI->m_hObject : LTNULL;
	LOAD_COBJECT( m_pKnowledgeTarget, ILTBaseClass );
	m_hKnowledgeTarget= (m_pKnowledgeTarget) ? m_pKnowledgeTarget->m_hObject : LTNULL;
	LOAD_BOOL( m_bLinkKnowledge );

	LOAD_BOOL( m_bKnowledgeDataIsTime );
	if( m_bKnowledgeDataIsTime )
	{
		LOAD_TIME( m_fKnowledgeData );
	}
	else {
		LOAD_FLOAT( m_fKnowledgeData );
	}
}


//
// CAICentralKnowledgeMgr functions.
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::CAICentralKnowledgeMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAICentralKnowledgeMgr::CAICentralKnowledgeMgr()
{
	ASSERT(g_pAICentralKnowledgeMgr == NULL);
	g_pAICentralKnowledgeMgr	= this;

	Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::~CAICentralKnowledgeMgr()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

CAICentralKnowledgeMgr::~CAICentralKnowledgeMgr()
{
	ASSERT(g_pAICentralKnowledgeMgr != NULL);

	Term();

	g_pAICentralKnowledgeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::Init()
//
//	PURPOSE:	Init object
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::Init()
{
	ASSERT(g_pAICentralKnowledgeMgr != NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::Term()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::Term()
{
	ASSERT(g_pAICentralKnowledgeMgr != NULL);

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for(it = m_mapCentralKnowledge.begin(); it != m_mapCentralKnowledge.end(); ++it )
	{
		AI_FACTORY_DELETE(it->second);
	}

	// Remove all entries.
	m_mapCentralKnowledge.clear( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::Save/Load()
//
//	PURPOSE:	Save and Load
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::Save(ILTMessage_Write *pMsg)
{
	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	SAVE_DWORD( m_mapCentralKnowledge.size() );
	for( it = m_mapCentralKnowledge.begin(); it != m_mapCentralKnowledge.end(); ++it )
	{
		SAVE_DWORD( it->first );
		it->second->Save( pMsg );
	}	
}

void CAICentralKnowledgeMgr::Load(ILTMessage_Read *pMsg)
{
	EnumAICentralKnowledgeType eKnowledgeType;
	uint32 cKnowledge;
	LOAD_DWORD(cKnowledge);

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;
	for( uint32 iKnowledge=0; iKnowledge < cKnowledge; ++iKnowledge )
	{
		LOAD_DWORD_CAST( eKnowledgeType, EnumAICentralKnowledgeType );
		pAICentralKnowledgeRecord = AI_FACTORY_NEW( CAICentralKnowledgeRecord );
		pAICentralKnowledgeRecord->Load(pMsg);

		// Some Knowledge records may have handles to objects that have transitioned
		// to a new level.  Delete these records.

		if( ( !pAICentralKnowledgeRecord->m_pAI ) || 
			( !pAICentralKnowledgeRecord->m_pKnowledgeTarget ) )
		{
			AI_FACTORY_DELETE( pAICentralKnowledgeRecord );
		}
		else {
			m_mapCentralKnowledge.insert( AICENTRAL_KNOWLEDGE_MAP::value_type( eKnowledgeType, pAICentralKnowledgeRecord ) );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::OnLinkBroken
//
//	PURPOSE:	Handle broken links by deleting related Knowledge records.
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	AICENTRAL_KNOWLEDGE_MAP::iterator it = m_mapCentralKnowledge.begin();
	while( it != m_mapCentralKnowledge.end() )
	{
		if( ( it->second->m_bLinkKnowledge ) &&
			( ( &it->second->m_hAI == pRef ) || 
			  ( &it->second->m_hKnowledgeTarget == pRef ) ) )
		{
			AI_FACTORY_DELETE(it->second);
			AICENTRAL_KNOWLEDGE_MAP::iterator next = it;
			++next;
			m_mapCentralKnowledge.erase(it);
			it = next;
		}
		else {

			// If knowledge is not linked, the record will
			// exist after the objects it references are destroyed.
			// NULLify the pointers to objects that are being unlinked.

			if( &it->second->m_hAI == pRef )
			{
				it->second->m_pAI = LTNULL;
			}

			if( &it->second->m_hKnowledgeTarget )
			{
				it->second->m_pKnowledgeTarget = LTNULL;
			}

			++it;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::RegisterKnowledge
//
//	PURPOSE:	Add a new piece of knowledge.
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::RegisterKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::RegisterKnowledge: Knowledge without an associated AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CAICentralKnowledgeMgr::RegisterKnowledge: Knowledge needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord = AI_FACTORY_NEW( CAICentralKnowledgeRecord );
	pAICentralKnowledgeRecord->m_eKnowledgeType = eKnowledgeType;
	pAICentralKnowledgeRecord->m_pAI = pAI;
	pAICentralKnowledgeRecord->m_hAI = pAI ? pAI->m_hObject : LTNULL;
	pAICentralKnowledgeRecord->m_pKnowledgeTarget = pKnowledgeTarget;
	pAICentralKnowledgeRecord->m_hKnowledgeTarget = pKnowledgeTarget ? pKnowledgeTarget->m_hObject : LTNULL;
	pAICentralKnowledgeRecord->m_bLinkKnowledge = bLinkKnowledge;

	m_mapCentralKnowledge.insert( AICENTRAL_KNOWLEDGE_MAP::value_type( eKnowledgeType, pAICentralKnowledgeRecord ) );
}

// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::RegisterKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge, LTFLOAT fData, LTBOOL bIsTime)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::RegisterKnowledge: Knowledge without an associated AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CAICentralKnowledgeMgr::RegisterKnowledge: Knowledge needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord = AI_FACTORY_NEW( CAICentralKnowledgeRecord );
	pAICentralKnowledgeRecord->m_eKnowledgeType = eKnowledgeType;
	pAICentralKnowledgeRecord->m_pAI = pAI;
	pAICentralKnowledgeRecord->m_hAI = pAI ? pAI->m_hObject : LTNULL;
	pAICentralKnowledgeRecord->m_pKnowledgeTarget = pKnowledgeTarget;
	pAICentralKnowledgeRecord->m_hKnowledgeTarget = pKnowledgeTarget ? pKnowledgeTarget->m_hObject : LTNULL;
	pAICentralKnowledgeRecord->m_bLinkKnowledge = bLinkKnowledge;
	pAICentralKnowledgeRecord->m_fKnowledgeData = fData;
	pAICentralKnowledgeRecord->m_bKnowledgeDataIsTime = bIsTime;

	m_mapCentralKnowledge.insert( AICENTRAL_KNOWLEDGE_MAP::value_type( eKnowledgeType, pAICentralKnowledgeRecord ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::RemoveKnowledge
//
//	PURPOSE:	Removes a piece of knowledge.
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::RemoveKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::RemoveKnowledge: Knowledge removal request without an associated AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CAICentralKnowledgeMgr::RemoveKnowledge: Knowledge needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;

	// Note: It is assumed that there are not multiple identical knowledge records.

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;

		if( ( pAICentralKnowledgeRecord->m_pAI == pAI ) && 
			( pAICentralKnowledgeRecord->m_pKnowledgeTarget == pKnowledgeTarget ) )
		{
			AI_FACTORY_DELETE( pAICentralKnowledgeRecord );
			m_mapCentralKnowledge.erase( it );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::RemoveKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::RemoveKnowledge: Knowledge removal request without an associated AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CAICentralKnowledgeMgr::RemoveKnowledge: Knowledge needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;

	// Note: It is assumed that there are not multiple identical knowledge records.

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;

		if( pAICentralKnowledgeRecord->m_pAI == pAI )
		{
			AI_FACTORY_DELETE( pAICentralKnowledgeRecord );
			m_mapCentralKnowledge.erase( it );
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::ReplaceKnowledge
//
//	PURPOSE:	Replace knowledge with a new float.
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::ReplaceKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget, LTBOOL bLinkKnowledge, LTFLOAT fData, LTBOOL bIsTime)
{
	RemoveAllKnowledge( eKnowledgeType, pAI );
	RegisterKnowledge( eKnowledgeType, pAI, pKnowledgeTarget, bLinkKnowledge, fData, bIsTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::RemoveAllKnowledge
//
//	PURPOSE:	Removes all knowledge with matching key.
//
// ----------------------------------------------------------------------- //

void CAICentralKnowledgeMgr::RemoveAllKnowledge(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::RemoveAllKnowledge: Knowledge removal request without an associated AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CAICentralKnowledgeMgr::RemoveAllKnowledge: Knowledge needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;
		AI_FACTORY_DELETE( pAICentralKnowledgeRecord );
	}

	// Remove all matching keys.

	m_mapCentralKnowledge.erase( eKnowledgeType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::CountTargetMatches
//
//	PURPOSE:	Counts matching pieces of knowledge.
//
// ----------------------------------------------------------------------- //

uint32 CAICentralKnowledgeMgr::CountTargetMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::CountTargetMatches: Query with no AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CountTargetMatches::CountMatchingKnowledge: Query needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;
	uint32 cMatches = 0;

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;

		// Do targets match?

		if( pKnowledgeTarget == pAICentralKnowledgeRecord->m_pKnowledgeTarget )
		{
			++cMatches;
		}
	}

	return cMatches;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::CountMatches
//
//	PURPOSE:	Counts matching pieces of knowledge.
//
// ----------------------------------------------------------------------- //

uint32 CAICentralKnowledgeMgr::CountMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI, ILTBaseClass *pKnowledgeTarget)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::CountTargetMatches: Query with no AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CountTargetMatches::CountMatchingKnowledge: Query needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;
	uint32 cMatches = 0;

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;

		// Do targets match?

		if( ( pKnowledgeTarget == pAICentralKnowledgeRecord->m_pKnowledgeTarget ) &&
			( pAI == pAICentralKnowledgeRecord->m_pAI ) )
		{
			++cMatches;
		}
	}

	return cMatches;
}

// ----------------------------------------------------------------------- //

uint32 CAICentralKnowledgeMgr::CountMatches(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI)
{
	AIASSERT( pAI, LTNULL, "CAICentralKnowledgeMgr::CountTargetMatches: Query with no AI." );
	AIASSERT( eKnowledgeType != kCK_InvalidType, pAI->m_hObject, "CountTargetMatches::CountMatchingKnowledge: Query needs a valid type." );

	CAICentralKnowledgeRecord* pAICentralKnowledgeRecord;
	uint32 cMatches = 0;

	// Iterate over matching knowledge types.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		pAICentralKnowledgeRecord = it->second;

		// Do AIs match?

		if( pAI == pAICentralKnowledgeRecord->m_pAI )
		{
			++cMatches;
		}
	}

	return cMatches;
}

// ----------------------------------------------------------------------- //

uint32 CAICentralKnowledgeMgr::CountMatches(EnumAICentralKnowledgeType eKnowledgeType)
{
	AIASSERT( eKnowledgeType != kCK_InvalidType, LTNULL, "CountTargetMatches::CountMatchingKnowledge: Query needs a valid type." );
	return m_mapCentralKnowledge.count( eKnowledgeType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::GetKnowledgeFloat
//
//	PURPOSE:	Gets piece of knowledge.
//
// ----------------------------------------------------------------------- //

LTFLOAT	CAICentralKnowledgeMgr::GetKnowledgeFloat(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI)
{
	// Return the first match found.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		CAICentralKnowledgeRecord* pAICentralKnowledgeRecord = it->second;
		if( !pAI )
		{
			return pAICentralKnowledgeRecord->m_fKnowledgeData;
		}
		else if( pAI == pAICentralKnowledgeRecord->m_pAI )
		{
			return pAICentralKnowledgeRecord->m_fKnowledgeData;
		}
	}

	return 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICentralKnowledgeMgr::GetKnowledgeTarget
//
//	PURPOSE:	Gets piece of knowledge.
//
// ----------------------------------------------------------------------- //

ILTBaseClass *CAICentralKnowledgeMgr::GetKnowledgeTarget(EnumAICentralKnowledgeType eKnowledgeType, ILTBaseClass *pAI)
{
	// Return the first match found.

	AICENTRAL_KNOWLEDGE_MAP::iterator it;
	for( it = m_mapCentralKnowledge.lower_bound( eKnowledgeType ); it != m_mapCentralKnowledge.upper_bound( eKnowledgeType ); ++it )
	{
		CAICentralKnowledgeRecord* pAICentralKnowledgeRecord = it->second;
		if( !pAI )
		{
			return pAICentralKnowledgeRecord->m_pKnowledgeTarget;
		}
		else if( pAI == pAICentralKnowledgeRecord->m_pAI )
		{
			return pAICentralKnowledgeRecord->m_pKnowledgeTarget;
		}
	}

	return LTNULL;
}
