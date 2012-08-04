// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCluster.cpp
//
// PURPOSE : AINodeCluster class implementation.
//           AI nodes may be clustered to prevent multiple AI from
//           selecting destination nodes that are too close together.
//
// CREATED : 4/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeCluster.h"
#include "AIAssert.h"
#include "AIUtils.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeCluster::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAINodeCluster::CAINodeCluster()
{
	m_eNodeClusterID = kNodeCluster_Invalid;
	m_nLockCount = 0;
	m_hLockingAI = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeCluster::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAINodeCluster::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(	m_eNodeClusterID );
	SAVE_HOBJECT( m_hLockingAI );
	SAVE_INT( m_nLockCount );
}

void CAINodeCluster::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eNodeClusterID, EnumAINodeClusterID );
	LOAD_HOBJECT( m_hLockingAI );
	LOAD_INT( m_nLockCount );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAINodeCluster::Lock / Unlock / IsLocked
//
//  PURPOSE:	Lock/Unlock cluster.
//
// ----------------------------------------------------------------------- //

void CAINodeCluster::LockCluster( HOBJECT hAI )
{
	// Cluster is locked by someone else.

	HOBJECT hLockingAI = GetLockingAI();
	if( hLockingAI && hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "CAINodeCluster::LockCluster: Cluster '%d' already locked by AI '%s'", m_eNodeClusterID, szName );
		return;
	}

	// Clear lock count from dead AI.

	if( !hLockingAI )
	{
		m_nLockCount = 0;
	}

	// Lock the cluster.

	AITRACE( AIShowNodes, ( hAI, "Locking cluster %d (lockcount = %d)\n", m_eNodeClusterID, m_nLockCount ) );
	m_hLockingAI = hAI; 
	++m_nLockCount;
}

// ----------------------------------------------------------------------- //

void CAINodeCluster::UnlockCluster( HOBJECT hAI )
{
	// Cluster is not locked.

	HOBJECT hLockingAI = GetLockingAI();
	if( !hLockingAI )
	{
		AIASSERT1( 0, hAI, "CAINodeCluster::Unlock: Cluster '%d' is not locked.", m_eNodeClusterID );
		return;
	}

	// Cluster is locked by someone else.

	if( hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "CAINodeCluster::Unlock: Cluster '%d' is locked by AI '%s'.", m_eNodeClusterID, szName );
		return;
	}

	// Unlock the cluster.

	AITRACE( AIShowNodes, ( hAI, "Unlocking cluster %d (lockcount= %d)\n", m_eNodeClusterID, m_nLockCount ) );
	--m_nLockCount;
	if( m_nLockCount == 0 )
	{
		m_hLockingAI = NULL;
	}
}

// ----------------------------------------------------------------------- //

bool CAINodeCluster::IsClusterLocked()
{
	return !!GetLockingAI();
}

// ----------------------------------------------------------------------- //

HOBJECT CAINodeCluster::GetLockingAI()
{
	if( m_hLockingAI && IsDeadAI( m_hLockingAI ) )
	{
		m_hLockingAI = NULL;
	}

	return m_hLockingAI; 
}

// ----------------------------------------------------------------------- //

