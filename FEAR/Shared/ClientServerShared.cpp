// ----------------------------------------------------------------------- //
//
// MODULE  : ClientServerShared.cpp
//
// PURPOSE : Utility functions used on client and server.
//
// CREATED : 10/21/02
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ClientServerShared.h"
#include "CommonUtilities.h"
#include "iltfilemgr.h"

#ifdef _CLIENTBUILD
	#include "../ClientShellDLL/ClientUtilities.h"
#endif //_CLIENTBUILD

#include "CollisionsDB.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetObjectMaterial
//
//	PURPOSE:	Sets a material of an object
//
// ----------------------------------------------------------------------- //
void SetObjectMaterial(HOBJECT hObj, uint8 nMaterialNum, const char* pszMaterial)
{
	if(nMaterialNum >= MAX_MATERIALS_PER_MODEL)
		return;

	if( LTStrEmpty( pszMaterial ))
		return;

	g_pModelLT->SetMaterialFilename( hObj, nMaterialNum, pszMaterial );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DebugCPrint
//
//  PURPOSE:	Prints an error message to the console.
//
// ----------------------------------------------------------------------- //

void DebugCPrint(int nLevel, const char* szFormat, ...)
{
	if (!g_pLTBase)
		return;

	if (nLevel > GetConsoleInt("DebugLevel",0))
		return;

	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	LTVSNPrintF(szBuffer, LTARRAYSIZE(szBuffer), szFormat, val);
	va_end(val);
	g_pLTBase->CPrintNoArgs(szBuffer);
	
}

// Initialize our search table.
struct HitLocationStringToEnum
{
	CParsedMsg::CToken	m_tokString;
	HitLocation			m_eHitLocation;
};
static HitLocationStringToEnum table[] = 
{
	{ "None", HL_UNKNOWN },
	{ "Head", HL_HEAD },
	{ "Torso", HL_TORSO },
	{ "Left Arm", HL_ARM_LEFT },
	{ "Right Arm", HL_ARM_RIGHT },
	{ "Left Leg", HL_LEG_LEFT },
	{ "Right Leg", HL_LEG_RIGHT },
};
static uint32 nTableCount = LTARRAYSIZE( table );

//retrieve the hit location enum from the text name of the location
HitLocation HitLocationFromString(const char* szLocation)
{
	// Find the string location and return the enum.
	CParsedMsg::CToken tokLocation = szLocation;
	for( uint32 i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_tokString == tokLocation )
			return table[i].m_eHitLocation;
	}

	return HL_UNKNOWN;
}

//retrieve the hit location enum from the text name of the location
const char* StringFromHitLocation(HitLocation eLoc)
{
	// Find the string location and return the enum.
	for( uint32 i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_eHitLocation == eLoc )
			return table[i].m_tokString.c_str();
	}

	return "Unknown";
}

HRECORD UserFlagToCollisionPropertyRecord( uint32 dwUserFlags )
{
	uint32 nIndex = (( dwUserFlags & USRFLG_COLLISIONPROPMASK ) >> 16 );
	if( !nIndex )
		return NULL;
	return g_pLTDatabase->GetRecordByIndex( DATABASE_CATEGORY( CollisionProperty ).GetCategory( ), nIndex - 1 );
}
uint32 CollisionPropertyRecordToUserFlag( HRECORD hCollisionProperty )
{
	// Get the index to the collisionproperty record.  The userflag
	// will contain 0 if invalid, otherwise it will contain 
	// the index+1.
	uint32 nIndex = g_pLTDatabase->GetRecordIndex( hCollisionProperty );
	if( nIndex == INVALID_GAME_DATABASE_INDEX )
		return 0;
	if( nIndex > 255 )
	{
		LTERROR( "CollisionProperty index is too large." );
		return 0;
	}
	return ( uint32 )(( nIndex + 1 ) << 16 );
}

HDATABASE OpenGameDatabase(const char* pszFilename)
{
	HDATABASE hDatabase = NULL;

	if ((hDatabase = g_pLTDatabase->OpenExistingDatabase(pszFilename)) == NULL)
	{
		// database has not been opened yet - open it now
		ILTInStream* pGameDatabaseStream = g_pLTBase->FileMgr()->OpenFile(pszFilename);

		if (!pGameDatabaseStream)
		{
			// failed to open stream on default database
			return NULL;
		}

		hDatabase = g_pLTDatabase->OpenNewDatabase(pszFilename, *pGameDatabaseStream);
		if (!hDatabase)
		{
			// failed to open a new instance of the database
			pGameDatabaseStream->Release();
			return NULL;
		}

		// release the stream since we successfully loaded the database
		pGameDatabaseStream->Release();
	}

	return hDatabase;
}

HDATABASE OpenOverridesDatabase(ILTInStream& OverridesStream, const char* pszConstraintsFilename)
{
	HDATABASE hDatabase = NULL;

	// open the constraints file
	ILTInStream* pConstraintsStream = g_pLTBase->FileMgr()->OpenFile(pszConstraintsFilename);

	if (!pConstraintsStream)
	{
		// must have a constraints file
		return NULL;
	}

	// create a database from the unpacked overrides stream
	hDatabase = g_pLTDatabase->OpenUnpackedDatabase(OverridesStream, *pConstraintsStream);
	if (!hDatabase)
	{
		// failed to open the database
		pConstraintsStream->Release();
		return NULL;
	}

	// release the constraints stream
	pConstraintsStream->Release();

	return hDatabase;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	UserFlagToObjectLOD
//
//  PURPOSE:	Extract the object detail level from the user flags...
//
// ----------------------------------------------------------------------- //

EEngineLOD UserFlagToObjectLOD( uint32 nUserFlags )
{
	return static_cast<EEngineLOD>( (nUserFlags & USRFLG_OBJECTLODMASK) >> 14 );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ObjectLODToUserFlag
//
//  PURPOSE:	Convert the object detail level for use in a user flag value... 
//
// ----------------------------------------------------------------------- //

uint32 ObjectLODToUserFlag( EEngineLOD eObjectLOD )
{
	return static_cast<uint32>(eObjectLOD << 14);
}

// EOF

