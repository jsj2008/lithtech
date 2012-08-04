// **************************************************************************** //
//
//	Project:	The Dark
//	File:		sonicsdb.h
//
//	Purpose:	This is a class which accesses Sonic information from the database.
//
//	Author:		Andy Mattingly
//	Date:		07/19/04
//
//	Copyright (C) 2004-2005 (Monolith Productions)
//
// **************************************************************************** //

#ifndef __SONICSDB_H__
#define __SONICSDB_H__

// **************************************************************************** //

#include "GameDatabaseMgr.h"

// **************************************************************************** //

#define SDB_CATEGORY							"Sonics/"

#define SDB_BOOK_CATEGORY						SDB_CATEGORY "Book"
#define SDB_BOOK_BREATHMINRECOVERYRATE			"BreathMinRecoveryRate"
#define SDB_BOOK_BREATHMAXRECOVERYRATE			"BreathMaxRecoveryRate"
#define SDB_BOOK_BREATHINTONERECOVERYDELAY		"BreathIntoneRecoveryDelay"
#define SDB_BOOK_GROUPS							"Groups"
#define SDB_BOOK_DEFAULTSKILLLEVEL				"DefaultSkillLevel"
#define SDB_BOOK_SKILLS							"Skills"

#define SDB_SKILLS_CATEGORY						SDB_CATEGORY "Skills"
#define SDB_SKILLS_LEVELS						"Levels"
#define SDB_SKILLS_BREATHAMOUNT					"BreathAmount"
#define SDB_SKILLS_INTONECOMMAND				"IntoneCommand"
#define SDB_SKILLS_ANIMATIONPROPERTY			"AnimationProperty"

// **************************************************************************** //

typedef HRECORD	HSONICBOOK;
typedef HRECORD HSONICSKILLS;

// **************************************************************************** //

class SonicsDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( SonicsDB );

	public:

		// Initialization and termination
		bool	Init( const char *sDatabase = DB_Default_File );
		void	Term() {};

		// Access interfaces
		HCATEGORY		GetBookCategory() const;
		HSONICBOOK		GetBookRecord( const char* sName ) const;

		uint32			GetNumGroups( HSONICBOOK hRecord ) const;
		uint32			GetGroupIndex( HSONICBOOK hRecord, const char* sGroup );
		int32			GetDefaultSkillLevel( HSONICBOOK hRecord, const char* sGroup );
		int32			GetDefaultSkillLevel( HSONICBOOK hRecord, uint32 nIndex );
		HSONICSKILLS	GetSkillsRecord( HSONICBOOK hRecord, const char* sGroup );
		HSONICSKILLS	GetSkillsRecord( HSONICBOOK hRecord, uint32 nIndex );

		uint32			GetNumSkillLevels( HSONICSKILLS hRecord );
		float			GetBreathAmount( HSONICSKILLS hRecord, uint32 nSkill );
		const char*		GetIntoneCommand( HSONICSKILLS hRecord, uint32 nSkill );
		const char*		GetAnimationProperty( HSONICSKILLS hRecord, uint32 nSkill );


	private:

		// Categories that we'll need to access records from...
		HCATEGORY		m_hBookCategory;
		HCATEGORY		m_hSkillsCategory;
};

// **************************************************************************** //

extern SonicsDB* g_pSonicsDB;

// **************************************************************************** //

#endif//__SONICSDB_H__

