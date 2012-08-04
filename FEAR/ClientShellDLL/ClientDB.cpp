
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientDB.cpp
//
// PURPOSE : 
//
// CREATED : 3/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientDB.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientDB::Constructor/Destructor()
//
//	PURPOSE:	Handles constructing and destructing the Client Database	
//
// ----------------------------------------------------------------------- //

ClientDB::ClientDB()
	: m_hDebugKeyCat(NULL)
	, m_hClientSharedRecord(NULL)
	, m_hHeadBobCat(NULL)
	, m_hFlashlightCat(NULL)
	, m_hMovementCat(NULL)
	, m_hHUDFlashCat(NULL)
	, m_hPlayerMovementRecord(NULL)
	, m_hClientFXSequenceCat( NULL )
{
}

ClientDB::~ClientDB() 
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientDB::Init()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool ClientDB::Init( const char *szDatabaseFile )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Get handles to all of the categories in the database...
	m_hDebugKeyCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_DebugKeyCat);
	LTASSERT_PARAM1(NULL != m_hDebugKeyCat, "ClientDB::Init : Missing category %s", CDB_DebugKeyCat);

	HCATEGORY hClientSharedCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_ClientSharedCat);
	LTASSERT_PARAM1(NULL != hClientSharedCat, "ClientDB::Init : Missing category %s", CDB_ClientSharedCat);

	m_hClientSharedRecord = CGameDatabaseReader::GetRecord(hClientSharedCat, CDB_ClientSharedRecord);
	LTASSERT_PARAM1(NULL != m_hClientSharedRecord, "ClientDB::Init : Missing record %s", CDB_ClientSharedRecord);

	m_hHeadBobCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_HeadBobCat);
	LTASSERT_PARAM1(NULL != m_hHeadBobCat, "ClientDB::Init : Missing category %s", CDB_HeadBobCat);

	m_hFlashlightCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_FlashlightCat);
	LTASSERT_PARAM1(NULL != m_hFlashlightCat, "ClientDB::Init : Missing category %s", CDB_FlashlightCat);

	m_hMovementCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_MovementCat);
	LTASSERT_PARAM1(NULL != m_hMovementCat, "ClientDB::Init : Missing category %s", CDB_MovementCat);

	m_hOverlayCat = g_pLTDatabase->GetCategory(m_hDatabase, CDB_OverlayCat);
	LTASSERT_PARAM1(NULL != m_hOverlayCat, "ClientDB::Init : Missing category %s", CDB_OverlayCat);

	m_hHUDFlashCat = g_pLTDatabase->GetCategory(m_hDatabase,CDB_HUDFlashCat);
	LTASSERT_PARAM1(NULL != m_hHUDFlashCat, "ClientDB::Init : Missing category %s", CDB_HUDFlashCat);

	HCATEGORY hPlayerMovementCat = g_pLTDatabase->GetCategory( m_hDatabase, CDB_PlayerMovementCat );
	LTASSERT_PARAM1( NULL != hPlayerMovementCat, "ClientDB::Init : Missing category %s", CDB_PlayerMovementCat );

	m_hPlayerMovementRecord = CGameDatabaseReader::GetRecord( hPlayerMovementCat, CDB_PlayerMovementRec );
	LTASSERT_PARAM1( NULL != m_hPlayerMovementRecord, "ClientDB::Init : Missing record %s", CDB_PlayerMovementRec);

	m_hClientFXSequenceCat = g_pLTDatabase->GetCategory( m_hDatabase, CDB_FXSEQ_Category );
	LTASSERT_PARAM1(NULL != m_hHUDFlashCat, "ClientDB::Init : Missing category %s", CDB_FXSEQ_Category );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientDB::Term()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void ClientDB::Term()
{
}

// ----------------------------------------------------------------------- //
//	ROUTINE:	ClientDB::GetFlashRecord()
//	PURPOSE:	Get the record associated with a particular HUD Flash type
// ----------------------------------------------------------------------- //
HRECORD ClientDB::GetHUDFlashRecord(const char* pszRecordName)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(m_hHUDFlashCat,pszRecordName);
	LTASSERT(hRec,"HUD Flash record does not exist.");
	return hRec;
}



