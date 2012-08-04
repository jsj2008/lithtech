// ----------------------------------------------------------------------- //
//
// MODULE  : PropsDB.cpp
//
// PURPOSE : Implementation of the models database.
//
// CREATED : 03/25/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "PropsDB.h"

#if defined( _CLIENTBUILD )
#include "..\ClientShellDll\GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
#else
#endif


//
// Defines...
//

// PropsDB categories
const char* const PropsDB_PropsCat =		"Props";
	
//
// Globals...
//

PropsDB* g_pPropsDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::PropsDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

PropsDB::PropsDB()
{
	m_hPropsCat = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::~PropsDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

PropsDB::~PropsDB( )
{
	g_pPropsDB = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool PropsDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
	{
		LTERROR( "PropsDB::Init - Couldn't open database." );
		return false;
	}

	// Set the global database pointer...
	g_pPropsDB = this;

	// Get handles to all of the categories in the database...
	m_hPropsCat = g_pLTDatabase->GetCategory(m_hDatabase, PropsDB_PropsCat);
	if( !m_hPropsCat )
	{
		LTERROR( "PropsDB::Init - Couldn't initialize props category." );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetPropsByRecordName()
//
//	PURPOSE:	Gets the HPROP object by gdb record name.
//
// ----------------------------------------------------------------------- //
PropsDB::HPROP PropsDB::GetPropByRecordName( char const* pszRecordName ) const
{
	return ( HPROP )GetRecord( m_hPropsCat, pszRecordName );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetNumProps()
//
//	PURPOSE:	Gets the number of models are available.
//
// ----------------------------------------------------------------------- //
uint32 PropsDB::GetNumProps( ) const
{
	return g_pLTDatabase->GetNumRecords( m_hPropsCat );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetProp()
//
//	PURPOSE:	Gets the model by index.
//
// ----------------------------------------------------------------------- //
PropsDB::HPROP PropsDB::GetProp( uint32 nIndex ) const
{
	return ( HPROP )g_pLTDatabase->GetRecordByIndex( m_hPropsCat, nIndex );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetRecordNameOfHProp()
//
//	PURPOSE:	Gets the record name of an HPROP
//
// ----------------------------------------------------------------------- //
char const* PropsDB::GetRecordNameOfHProp( HPROP hProp ) const
{
	return g_pLTDatabase->GetRecordName( hProp );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetPropFilename() const
//
//	PURPOSE:	Gets the model filename.
//
// ----------------------------------------------------------------------- //
char const* PropsDB::GetPropFilename(HPROP hProp) const
{
	return GetString( hProp, "ModelFile" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetNumMaterials()
//
//	PURPOSE:	Gets the number of material filenames.
//
// ----------------------------------------------------------------------- //
uint32 PropsDB::GetNumMaterials( HPROP hProp ) const
{
	return GetNumValues( hProp, "Material" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::GetMaterialFilename()
//
//	PURPOSE:	Gets the material filename.
//
// ----------------------------------------------------------------------- //
char const* PropsDB::GetMaterialFilename(HPROP hProp, uint8 iMaterial) const
{
	return GetString( hProp, "Material", iMaterial );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PropsDB::CopyMaterialFilenames()
//
//	PURPOSE:	Gets all the material filenames.
//
// ----------------------------------------------------------------------- //
void PropsDB::CopyMaterialFilenames(HPROP hProp, char* paszDest, uint32 nNumValues, uint32 nStrLen) const
{
	CopyStringValues( hProp, "Material", paszDest, nNumValues, nStrLen );
}
