// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFilterDB.h
//
// PURPOSE : Declares the SoundFilter 
//
// CREATED : 3/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __SOUNDFILTERDB_H_
#define __SOUNDFILTERDB_H_


//
// Includes...
//

#include "GameDatabaseMgr.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		SoundFilterDB
//
//	PURPOSE:	Supports querying for sound filter information.
//
// ----------------------------------------------------------------------- //

class SoundFilterDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( SoundFilterDB );

public:
	// Constructors
	
	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term();

	// Queries
	HRECORD		GetUnfilteredFilterRecord() const;
	HRECORD		GetDynamicFilterRecord() const;

	HCATEGORY	GetSoundFilterCategory() const { return m_hSoundFilterCategory; }

	HRECORD		GetFilterRecord(int nIndex) const;
	HRECORD		GetFilterRecord(const char* const pszFilterName) const;
	const char*	GetFilterRecordName(HRECORD hSoundFilterRecord) const;
	int			GetFilterRecordCount() const;

	const char*	GetFilterName(HRECORD hSoundFilter) const;
	bool		IsFilterDynamic(HRECORD hSoundFilter) const;
	bool		IsFilterUnFiltered(HRECORD hSoundFilter) const;
	const char* GetFilterParmeterName(HRECORD hSoundFilter, int iFilterIndex) const;
	float		GetFilterParmeterValue(HRECORD hSoundFilter, int iFilterIndex) const;
	int			GetFilterParmeterCount() const;

private:
	HCATEGORY	m_hSoundFilterCategory;
	HRECORD		m_hUnfilteredFilterRecord;
	HRECORD		m_hDynamicFilterRecord;
};

class SoundFilterDBPlugin
{
public:
	bool		PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

#endif // __SOUNDFILTERDB_H_
