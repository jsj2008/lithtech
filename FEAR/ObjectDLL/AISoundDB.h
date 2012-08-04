// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundDB.h
//
// PURPOSE : Definitition of AI Sound database
//
// CREATED : 03/19/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AISOUNDDB_H__
#define __AISOUNDDB_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "SoundDB.h"

//
// Defines...
//
const char* const AISoundDB_nMixChannel =			"MixChannel";

//
// Forward declarations...
//

class	CAISoundDB;

extern CAISoundDB* g_pAISoundDB;


//
// Structs... 
//



//
// AISoundDB... 
//

class CAISoundDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CAISoundDB );

	public :	// Methods...

		bool	Init( const char *szDatabaseFile = DB_Default_File );
		void	Term();

		void	GetRandomSoundFilename(HRECORD hSoundTemplate, const char* pAttributeBase, char *pBuf, uint16 nBufLen);
		void	GetRandomSoundFilename(HRECORD hAISoundSet, char *pBuf, uint16 nBufLen);

		int32	GetMixChannel(HRECORD hSoundTemplate);

		HCATEGORY GetSoundTemplateCategory() const { return m_hSoundTemplateCat; }
		HCATEGORY GetSoundSetCategory() const { return m_hSoundSetCat; }

	private :
		bool	InitSoundSets();
		
		HCATEGORY m_hSoundTemplateCat;
		HCATEGORY m_hSoundSetCat;

		SoundSetArray	m_vecSoundSets;

};


#endif  // __AISOUNDDB_H__
