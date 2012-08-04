// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectMgr.cpp
//
// PURPOSE : AITargetSelectMgr abstract class implementation
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectMgr.h"
#include "AIDB.h"


// Globals / Statics

CAITargetSelectMgr* g_pAITargetSelectMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectMgr::CAITargetSelectMgr()
{
	AIASSERT( !g_pAITargetSelectMgr, NULL, "CAITargetSelectMgr: Singleton already set." );
	g_pAITargetSelectMgr = this;

	for( int iTargetSelect=0; iTargetSelect < kTargetSelect_Count; ++iTargetSelect )
	{
		m_pAITargetSelects[iTargetSelect] = NULL;
	}
}

CAITargetSelectMgr::~CAITargetSelectMgr()
{
	AIASSERT( g_pAITargetSelectMgr, NULL, "CAITargetSelectMgr: No singleton." );
	g_pAITargetSelectMgr = NULL;

	TermAITargetSelectMgr();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::InitAITargetSelectMgr
//
//	PURPOSE:	Initialize AITargetSelectMgr.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectMgr::InitAITargetSelectMgr()
{
	// Create an instance of each target in the bute file.
	// TargetSelects are shared, rather than owned by each AI.

	CAITargetSelectAbstract* pTargetSelect;
	AIDB_TargetSelectRecord* pTargetRecord;
	unsigned int cTargetSelects = g_pAIDB->GetNumAITargetSelectRecords();
	for( unsigned int iTargetSelect=0; iTargetSelect < cTargetSelects; ++iTargetSelect )
	{
		pTargetRecord = g_pAIDB->GetAITargetSelectRecord( iTargetSelect );
		if( pTargetRecord && ( pTargetRecord->eTargetSelectClass != kTargetSelect_InvalidType ) )
		{
			pTargetSelect = AI_FACTORY_NEW_TargetSelect( pTargetRecord->eTargetSelectClass );
			if( pTargetSelect )
			{
				m_pAITargetSelects[pTargetRecord->eTargetSelectType] = pTargetSelect;
				pTargetSelect->InitTargetSelect( pTargetRecord );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::TermAITargetSelectMgr
//
//	PURPOSE:	Terminate AITargetSelectMgr.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectMgr::TermAITargetSelectMgr()
{
	// Delete instances of TargetSelects.

	for( int iTargetSelect=0; iTargetSelect < kTargetSelect_Count; ++iTargetSelect )
	{
		if( m_pAITargetSelects[iTargetSelect] )
		{
			AI_FACTORY_DELETE( m_pAITargetSelects[iTargetSelect] );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::AI_FACTORY_NEW_TargetSelect
//
//	PURPOSE:	Create an TargetSelect.
//
// ----------------------------------------------------------------------- //

CAITargetSelectAbstract* CAITargetSelectMgr::AI_FACTORY_NEW_TargetSelect( EnumAITargetSelectType eTargetSelectClass )
{
	// Call AI_FACTORY_NEW for the requested type of TargetSelect.

	switch( eTargetSelectClass )
	{
		#define TARGET_TYPE_AS_SWITCH 1
		#include "AIEnumTargetTypes.h"
		#undef TARGET_TYPE_AS_SWITCH

		default: AIASSERT( 0, NULL, "CAITargetSelectMgr::AI_FACTORY_NEW_TargetSelect: Unrecognized TargetSelect type." );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::GetAITargetSelect
//
//	PURPOSE:	Get a pointer to an TargetSelect.
//
// ----------------------------------------------------------------------- //

CAITargetSelectAbstract* CAITargetSelectMgr::GetAITargetSelect( EnumAITargetSelectType eTargetSelect )
{
	if( ( eTargetSelect > kTargetSelect_InvalidType ) &&
		( eTargetSelect < kTargetSelect_Count ) )
	{
		return m_pAITargetSelects[eTargetSelect];
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectMgr::IsInAITargetSelectSet
//
//	PURPOSE:	Returns true if the TargetSelect is in the specified set, else
//				returns false.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectMgr::IsTargetSelectInAITargetSelectSet(ENUM_AITargetSelectSet eSet, EnumAITargetSelectType eTargetSelect)
{
	// TargetSelect enum is invalid
	if( ( eTargetSelect <= kTargetSelect_InvalidType ) ||
		( eTargetSelect >= kTargetSelect_Count ) )
	{
		return false;
	}
	
	// TargetSelectSet ID is invalid.

	AIDB_TargetSelectSetRecord* pRecord = g_pAIDB->GetAITargetSelectSetRecord( eSet );
	if( !pRecord )
	{
		return false;
	}

	// Mask determines if TargetSelect is in TargetSelectsSet.

	return pRecord->TargetSelectMask.test( eTargetSelect );
}
