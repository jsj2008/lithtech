// ----------------------------------------------------------------------- //
//
// MODULE  : TriggerTypeDB.h
//
// PURPOSE : Defines an interface for accessing TriggerTypes
//
// CREATED : 3/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __TRIGGERTYPEDB_H_
#define __TRIGGERTYPEDB_H_

//
// Includes...
//

#include "CategoryDB.h"


// ----------------------------------------------------------------------- //
//
//	CLASS:		TriggerTypeDB
//
//	PURPOSE:	Database for accessing TriggerTypes
//
// ----------------------------------------------------------------------- //

class TriggerTypeDB : public CategoryDB
{
	DECLARE_SINGLETON( TriggerTypeDB );

public:

	virtual bool		Init( const char *szDatabaseFile = DB_Default_File );

	// Queries

	int					GetTriggerTypeRecordCount() const;
	HRECORD				GetTriggerTypeRecord(int nIndex) const;
	HRECORD				GetTriggerRecord(const char* const pszRecordName) const;
	const char* const	GetTriggerTypeRecordName(HRECORD) const;

	const char* const	GetIconTexture(HRECORD) const;

private:

	using CategoryDB::Init;
};

class TriggerTypeDBPlugin
{
public:
	bool		PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

#endif // __TRIGGERTYPEDB_H_

