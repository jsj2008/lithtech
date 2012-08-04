// **************************************************************************** //
//
//	Project:	The Dark
//	File:		sonicsdb.cpp
//
//	Purpose:	This is a class which accesses Sonic information from the database.
//
//	Author:		Andy Mattingly
//	Date:		07/19/04
//
//	Copyright (C) 2004-2005 (Monolith Productions)
//
// **************************************************************************** //

#include "Stdafx.h"

#ifndef __SONICSDB_H__
#include "SonicsDB.h"
#endif//__SONICSDB_H__

// **************************************************************************** //

SonicsDB* g_pSonicsDB = NULL;

// **************************************************************************** //

SonicsDB::SonicsDB() : CGameDatabaseMgr()
{

}

// **************************************************************************** //

SonicsDB::~SonicsDB()
{

}

// **************************************************************************** //

bool SonicsDB::Init( const char *sDatabase )
{
	if( !OpenDatabase( sDatabase ) )
	{
		return false;
	}

	// Set the global database pointer...
	g_pSonicsDB = this;

	// Get handles to all of the categories in the database...
	m_hBookCategory = g_pLTDatabase->GetCategory( m_hDatabase, SDB_BOOK_CATEGORY );
	m_hSkillsCategory = g_pLTDatabase->GetCategory( m_hDatabase, SDB_SKILLS_CATEGORY );

	return true;
}

// **************************************************************************** //

HCATEGORY SonicsDB::GetBookCategory() const
{
	return m_hBookCategory;
}

// **************************************************************************** //

HSONICBOOK SonicsDB::GetBookRecord( const char* sName ) const
{
	return g_pLTDatabase->GetRecord( m_hBookCategory, sName );
}

// **************************************************************************** //

uint32 SonicsDB::GetNumGroups( HSONICBOOK hRecord ) const
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	return g_pLTDatabase->GetNumValues( hAttrib );
}

// **************************************************************************** //

uint32 SonicsDB::GetGroupIndex( HSONICBOOK hRecord, const char* sGroup )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	uint32 nGroups = g_pLTDatabase->GetNumValues( hAttrib );

	for( uint32 i = 0; i < nGroups; ++i )
	{
		HATTRIBUTE hSkillsAttrib = GetStructAttribute( hAttrib, i, SDB_BOOK_SKILLS );

		HRECORD hRecord = GetRecordLink( hSkillsAttrib );
		const char* sRecordName = g_pLTDatabase->GetRecordName( hRecord );

		if( !LTStrICmp( sRecordName, sGroup ) )
		{
			return i;
		}
	}

	return 0xFFFFFFFF;
}

// **************************************************************************** //

int32 SonicsDB::GetDefaultSkillLevel( HSONICBOOK hRecord, const char* sGroup )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	uint32 nGroups = g_pLTDatabase->GetNumValues( hAttrib );

	for( uint32 i = 0; i < nGroups; ++i )
	{
		HATTRIBUTE hDefaultSkillLevelAttrib = GetStructAttribute( hAttrib, i, SDB_BOOK_DEFAULTSKILLLEVEL );
		HATTRIBUTE hSkillsAttrib = GetStructAttribute( hAttrib, i, SDB_BOOK_SKILLS );

		HRECORD hRecord = GetRecordLink( hSkillsAttrib );
		const char* sRecordName = g_pLTDatabase->GetRecordName( hRecord );

		if( !LTStrICmp( sRecordName, sGroup ) )
		{
			return GetInt32( hDefaultSkillLevelAttrib );
		}
	}

	return 0;
}

// **************************************************************************** //

int32 SonicsDB::GetDefaultSkillLevel( HSONICBOOK hRecord, uint32 nIndex )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	HATTRIBUTE hDefaultSkillLevelAttrib = GetStructAttribute( hAttrib, nIndex, SDB_BOOK_DEFAULTSKILLLEVEL );

	return GetInt32( hDefaultSkillLevelAttrib );
}

// **************************************************************************** //

HSONICSKILLS SonicsDB::GetSkillsRecord( HSONICBOOK hRecord, const char* sGroup )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	uint32 nGroups = g_pLTDatabase->GetNumValues( hAttrib );

	for( uint32 i = 0; i < nGroups; ++i )
	{
		HATTRIBUTE hSkillsAttrib = GetStructAttribute( hAttrib, i, SDB_BOOK_SKILLS );

		HRECORD hRecord = GetRecordLink( hSkillsAttrib );
		const char* sRecordName = g_pLTDatabase->GetRecordName( hRecord );

		if( !LTStrICmp( sRecordName, sGroup ) )
		{
			return hRecord;
		}
	}

	return NULL;
}

// **************************************************************************** //

HSONICSKILLS SonicsDB::GetSkillsRecord( HSONICBOOK hRecord, uint32 nIndex )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_BOOK_GROUPS );
	HATTRIBUTE hSkillsAttrib = GetStructAttribute( hAttrib, nIndex, SDB_BOOK_SKILLS );

	return GetRecordLink( hSkillsAttrib );
}

// **************************************************************************** //

uint32 SonicsDB::GetNumSkillLevels( HSONICSKILLS hRecord )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_SKILLS_LEVELS );
	return g_pLTDatabase->GetNumValues( hAttrib );
}

// **************************************************************************** //

float SonicsDB::GetBreathAmount( HSONICSKILLS hRecord, uint32 nSkill )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_SKILLS_LEVELS );
	HATTRIBUTE hBreathAmountAttrib = GetStructAttribute( hAttrib, nSkill, SDB_SKILLS_BREATHAMOUNT );

	return GetFloat( hBreathAmountAttrib );
}

// **************************************************************************** //

const char* SonicsDB::GetIntoneCommand( HSONICSKILLS hRecord, uint32 nSkill )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_SKILLS_LEVELS );
	HATTRIBUTE hIntoneCommand = GetStructAttribute( hAttrib, nSkill, SDB_SKILLS_INTONECOMMAND );

	return GetString( hIntoneCommand );
}

// **************************************************************************** //

const char* SonicsDB::GetAnimationProperty( HSONICSKILLS hRecord, uint32 nSkill )
{
	HATTRIBUTE hAttrib = g_pLTDatabase->GetAttribute( hRecord, SDB_SKILLS_LEVELS );
	HATTRIBUTE hIntoneCommand = GetStructAttribute( hAttrib, nSkill, SDB_SKILLS_ANIMATIONPROPERTY );

	return GetString( hIntoneCommand );
}
