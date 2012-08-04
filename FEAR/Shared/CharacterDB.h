// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterDB.h
//
// PURPOSE : Character database definition
//
// CREATED : 03/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_DATABASE_H__
#define __CHARACTER_DATABASE_H__

//
// Includes...
//

	#include "GameDatabaseMgr.h"
	#include "CharacterAlignment.h"

//
// Defines...
//

	// The main system category were all Character related data goes...
	#define CHARACTERDB_CATEGORY					"Character/"

	#define CHARACTERDB_ALIGNMENT_CATEGORY			CHARACTERDB_CATEGORY "Alignments"

		#define CHARACTERDB_ALIGNMENT_sAlignment	"Alignment"
		#define CHARACTERDB_ALIGNMENT_sStance		"Stance"

	#define CHARACTERDB_CONSTANTS_CATEGORY				CHARACTERDB_CATEGORY "Constants"

		#define CHARACTERDB_CONSTANTS_sTeamAlignments		"TeamAlignments"

//
// Forward declarations...
//

class CCharacterDB;
extern CCharacterDB* g_pCharacterDB;



//
// Structs... 
//


//
// CharacterDB... 
//

class CCharacterDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CCharacterDB );

	public :	// Methods...

		bool	Init( const char *szDatabaseFile = DB_Default_File );
		void	Term() {};

		// Alignments.

		uint32					GetNumAlignments() const { return m_cAlignments; }
		EnumCharacterStance		GetStance( EnumCharacterAlignment eAlignment, EnumCharacterAlignment eFeelsAbout ) const;

		EnumCharacterAlignment	String2Alignment( const char* szName );
		const char*				Alignment2String( EnumCharacterAlignment eAlignment ) const;

		uint8					GetNumTeamAlignments() const { return m_cTeamAlignments; }
		EnumCharacterAlignment	GetTeamAlignment( uint8 nTeam ) const
		{
			return (nTeam < m_cTeamAlignments) ? m_aTeamAlignments[nTeam] : kCharAlignment_Invalid;
		}

	private :

		void	CreateAlignmentMatrix();
		void 	LoadConstants();

	private	:	// Members...

		uint32					m_cAlignments;
		std::string				m_aAlignmentNames[kCharAlignment_Max];

		EnumCharacterStance		m_aAlignments[kCharAlignment_Max][kCharAlignment_Max];

		uint8					m_cTeamAlignments;
		EnumCharacterAlignment*	m_aTeamAlignments;
};





#endif // __CHARACTER_DATABASE_H__
