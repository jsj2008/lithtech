// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterDB.cpp
//
// PURPOSE : Character database implementation
//
// CREATED : 03/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "CharacterDB.h"

//
// Defines...
//


//
// Globals...
//

CCharacterDB* g_pCharacterDB = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::CCharacterDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CCharacterDB::CCharacterDB()
	: CGameDatabaseMgr()
{
	m_cAlignments = 0;
	m_cTeamAlignments = 0;
	m_aTeamAlignments = NULL;

	// Initialize all stances to undefined.

	int iAlignment, jAlignment;
	for( iAlignment=0; iAlignment < kCharAlignment_Max; ++iAlignment )
	{
		for( jAlignment=0; jAlignment < kCharAlignment_Max; ++jAlignment )
		{
			m_aAlignments[iAlignment][jAlignment] = kCharStance_Undetermined;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::~CCharacterDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CCharacterDB::~CCharacterDB()
{
	debug_deletea( m_aTeamAlignments );
	m_aTeamAlignments = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::Init()
//
//	PURPOSE:	Initialize the Character database...
//
// ----------------------------------------------------------------------- //

bool CCharacterDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ) )
	{
		return false;
	}

	// Set the global Character database pointer...

	g_pCharacterDB = this;

	CreateAlignmentMatrix();

	LoadConstants();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::CreateAlignmentMatrix()
//
//	PURPOSE:	Create Alignment matrix.
//
// ----------------------------------------------------------------------- //

void CCharacterDB::CreateAlignmentMatrix()
{
	HCATEGORY hCatAlignments = g_pLTDatabase->GetCategory( m_hDatabase, CHARACTERDB_ALIGNMENT_CATEGORY );
	if( !hCatAlignments )
	{
		return;
	}

	// The number of records defines the number of alignments.

	m_cAlignments = g_pLTDatabase->GetNumRecords( hCatAlignments );

	// Read the names of the alignments.

	HRECORD hRecord;
	const char* pszName;
	for( uint32 iAlignment=0; iAlignment < m_cAlignments; ++iAlignment )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAlignments, iAlignment );
		if( !hRecord )
		{
			continue;
		}

		pszName = g_pLTDatabase->GetRecordName( hRecord );
		if( pszName[0] )
		{
			m_aAlignmentNames[iAlignment] = pszName;
		}
	}

	EnumCharacterAlignment eAlignment;
	EnumCharacterAlignment eOther;
	EnumCharacterStance	eStance;
	const char* pszOther;
	const char* pszStance;
	uint32 iStance;
	uint32 cStances;

	// Fill the matrix.

	HRECORD hLink;
	HATTRIBUTE hAtt;
	for( uint32 iAlignment=0; iAlignment < m_cAlignments; ++iAlignment )
	{
		hRecord = g_pLTDatabase->GetRecordByIndex( hCatAlignments, iAlignment );
		if( !hRecord )
		{
			continue;
		}

		// The record defines the alignment being filled out.

		pszName = g_pLTDatabase->GetRecordName( hRecord );
		eAlignment = String2Alignment( pszName );

		hAtt = g_pLTDatabase->GetAttribute( hRecord, CHARACTERDB_ALIGNMENT_sAlignment );
		cStances = g_pLTDatabase->GetNumValues( hAtt );
		for( iStance=0; iStance < cStances; ++iStance )
		{
			hLink = GetRecordLink( hRecord, CHARACTERDB_ALIGNMENT_sAlignment, iStance );
			if( !hLink )
			{
				continue;
			}

			// The alignment the stance is towards.

			pszOther = g_pLTDatabase->GetRecordName( hLink );
			eOther = String2Alignment( pszOther );
			if( eOther == kCharAlignment_Invalid )
			{
				continue;
			}

			// Stance.

			pszStance = GetString( hRecord, CHARACTERDB_ALIGNMENT_sStance, iStance );
			eStance = CharacterAlignment::ConvertNameToStance( pszStance );

			// Fill in the cell.

			m_aAlignments[eAlignment][eOther] = eStance;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::LoadConstants
//
//	PURPOSE:	Load constants data
//
// ----------------------------------------------------------------------- //

void CCharacterDB::LoadConstants()
{
	HCATEGORY hCatConstants = g_pLTDatabase->GetCategory( m_hDatabase, CHARACTERDB_CONSTANTS_CATEGORY );
	if( !hCatConstants )
	{
		return;
	}

	HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCatConstants, 0 );
	if( !hRecord )
	{
		return;
	}

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, CHARACTERDB_CONSTANTS_sTeamAlignments );
	m_cTeamAlignments = g_pLTDatabase->GetNumValues( hAtt );

	if(m_cTeamAlignments)
	{
		m_aTeamAlignments = debug_newa( EnumCharacterAlignment, m_cTeamAlignments );
		
		for( uint8 iTeam=0; iTeam < m_cTeamAlignments; ++iTeam )
		{
			HRECORD hLink = GetRecordLink( hRecord, CHARACTERDB_CONSTANTS_sTeamAlignments, iTeam );
			if( !hLink )
			{
				continue;
			}

			m_aTeamAlignments[iTeam] = String2Alignment( g_pLTDatabase->GetRecordName( hLink ) );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::GetStance
//
//	PURPOSE:	Return the stance of how one alignment 
//              feels about another alignment.
//
// ----------------------------------------------------------------------- //

EnumCharacterStance CCharacterDB::GetStance( EnumCharacterAlignment eAlignment, EnumCharacterAlignment eFeelsAbout ) const
{
	// Alignment out of range.

	if( ( eAlignment <= kCharAlignment_Invalid ) || 
		( eFeelsAbout <= kCharAlignment_Invalid ) || 
		( (uint32)eAlignment >= m_cAlignments ) ||
		( (uint32)eFeelsAbout >= m_cAlignments ) )
	{
		return kCharStance_Undetermined;
	}

	// Return how an alignment feels about another alignment.

	return m_aAlignments[eAlignment][eFeelsAbout];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::String2Alignment()
//
//	PURPOSE:	Converts a string to an alignment enum.
//
// ----------------------------------------------------------------------- //

EnumCharacterAlignment CCharacterDB::String2Alignment( const char* szName )
{
	if( !szName[0] )
	{
		return kCharAlignment_Invalid;
	}

	for( uint32 iAlignment=0; iAlignment < m_cAlignments; ++iAlignment )
	{
		if( LTStrIEquals( szName, m_aAlignmentNames[iAlignment].c_str() ) )
		{
			return (EnumCharacterAlignment)iAlignment;
		}
	}

	return kCharAlignment_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterDB::Alignment2String()
//
//	PURPOSE:	Converts an alignment enum to a string.
//
// ----------------------------------------------------------------------- //

const char* CCharacterDB::Alignment2String( EnumCharacterAlignment eAlignment ) const
{
	if( ( eAlignment > kCharAlignment_Invalid ) && 
		( (uint32)eAlignment < m_cAlignments ) )
	{
		return m_aAlignmentNames[eAlignment].c_str();
	}

	return "";
}
