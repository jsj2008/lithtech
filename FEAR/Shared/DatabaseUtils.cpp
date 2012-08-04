// ----------------------------------------------------------------------- //
//
// MODULE  : DatabaseUtils.h
//
// PURPOSE : implementation of base classes for reading and writing GameDatabases
//
// CREATED : 07/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "DatabaseUtils.h"
	#include "ltlibraryloader.h"


//
// Statics...
//

	static uint32	 s_nGameDatabaseRef	= 0;
	static HLTMODULE s_hDatabaseInst		= NULL;

	IDatabaseMgr*			g_pLTDatabase = NULL;
	IDatabaseCreatorMgr*	g_pLTDatabaseCreator = NULL;

	static char szDBErrorString[256];


//
// External functions for building in SEM configurations...
//

	extern "C"
	{
		IDatabaseMgr* GetIDatabaseMgr();
		IDatabaseCreatorMgr* GetIDatabaseCreatorMgr();
	}

//
// Debug functions...
//
const char* GetAttributeTypeString(EAttributeType type)
{
	static char szStr[eAttributeType_RecordLink+1][64] = 
	{
		"eAttributeType_Invalid",
			"eAttributeType_Bool",
			"eAttributeType_Float",
			"eAttributeType_Int32",
			"eAttributeType_String",
			"eAttributeType_WString",
			"eAttributeType_Vector2",
			"eAttributeType_Vector3",
			"eAttributeType_Vector4",
			"eAttributeType_RecordLink"
	};
	return szStr[type];
}
const char* GetDBErrorString()
{
	return szDBErrorString;
}
bool VerifyAttribute( HRECORD hrecord, const char * pszName, EAttributeType type )
{
	szDBErrorString[0] = '\0';
	if (!hrecord)
	{
		LTSNPrintF(szDBErrorString,LTARRAYSIZE(szDBErrorString),"Attempted to read attribute '%s' from an invalid record.",pszName);
		return false;
	}


	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hrecord, pszName );
	if (hAtt)
	{
		EAttributeType eActualType = g_pLTDatabase->GetAttributeType( hAtt );
		if (eActualType != type)
		{
			LTSNPrintF(szDBErrorString,LTARRAYSIZE(szDBErrorString),"Attribute '%s' is of type %s. Attempted to access as type %s",pszName,GetAttributeTypeString(eActualType),GetAttributeTypeString(type));
			return false;
		}
		return true;

	}

	LTSNPrintF(szDBErrorString,LTARRAYSIZE(szDBErrorString),"Failed to read attribute '%s' from record '%s', category '%s'.",pszName, g_pLTDatabase->GetRecordName(hrecord), g_pLTDatabase->GetCategoryName(g_pLTDatabase->GetRecordParent(hrecord)));
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadDatabaseInterface()
//
//	PURPOSE:	Load the interfaces for the internal database mgrs...
//
// ----------------------------------------------------------------------- //

bool LoadDatabaseInterface( const char *pszDllFile /* = GDB_DLL_NAME  */ )
{
	// Increase the ref...

	++s_nGameDatabaseRef;

	if ( !g_pLTDatabase || !g_pLTDatabaseCreator )
	{
#if defined(PLATFORM_SEM)
		g_pLTDatabase = GetIDatabaseMgr();
		g_pLTDatabaseCreator = GetIDatabaseCreatorMgr();
#else // PLATFORM_SEM
		// The database was not setup yet.  Do so now...
		if( !s_hDatabaseInst && pszDllFile )
			s_hDatabaseInst = LTLibraryLoader::OpenLibrary( pszDllFile );

		if( s_hDatabaseInst )
		{
			// Get the databse interface function...

			HLTPROC hProc = LTLibraryLoader::GetProcAddress( s_hDatabaseInst, "GetIDatabaseMgr" );

			LTASSERT( hProc != NULL, "Unable to retrieve the DatabaseMgr function!" );
			if( hProc )
			{
				TGetIDatabaseMgrFn DBfn = (TGetIDatabaseMgrFn)hProc;
				g_pLTDatabase = DBfn();
			}
			else
			{
				FreeDatabaseInterface( );
			}

			// Get the database creator interface function...
			hProc = LTLibraryLoader::GetProcAddress( s_hDatabaseInst, "GetIDatabaseCreatorMgr" );

			LTASSERT( hProc != NULL, "Unable to retrieve the DatabaseCreatorMgr function!" );
			if( hProc )
			{
				TGetIDatabaseCreatorMgrFn DBfn = (TGetIDatabaseCreatorMgrFn)hProc;
				g_pLTDatabaseCreator = DBfn();
			}
			else
			{
				FreeDatabaseInterface( );
			}

		}
#endif // PLATFORM_SEM
	}

	return (g_pLTDatabase != NULL && g_pLTDatabaseCreator != NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FreeDatabaseInterface()
//
//	PURPOSE:	Free the database mgr...
//
// ----------------------------------------------------------------------- //

void FreeDatabaseInterface( )
{

	// Decrease the ref...
	--s_nGameDatabaseRef;
	LTASSERT( s_nGameDatabaseRef >= 0, "Freed more gamedatabases than created!" );

	if( s_nGameDatabaseRef == 0 )
	{
		// No more refs so free it...

		if( s_hDatabaseInst )
		{
			LTLibraryLoader::CloseLibrary( s_hDatabaseInst );
		}

		s_hDatabaseInst		= NULL;
		g_pLTDatabase		= NULL;
		g_pLTDatabaseCreator = NULL;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseReader::CGameDatabaseReader()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseReader::CGameDatabaseReader()
{
	LoadDatabaseInterface( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseReader::~CGameDatabaseReader()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseReader::~CGameDatabaseReader()
{
	// Clean up...

	FreeDatabaseInterface( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseReader::GetRecord()
//
//	PURPOSE:	Retrieve a record from the given category
//
// ----------------------------------------------------------------------- //
HRECORD CGameDatabaseReader::GetRecord( HCATEGORY hCat, const char *pszRecord ) const
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hCat,pszRecord);
	if (!hRec)
	{
		LTSNPrintF(szDBErrorString,LTARRAYSIZE(szDBErrorString),"Failed to read record '%s' from category '%s'.",pszRecord,g_pLTDatabase->GetCategoryName(hCat));
		LTASSERT(hRec,szDBErrorString);
	}
	return hRec;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseReader::GetRecord()
//
//	PURPOSE:	Retrieve a record from the given category and database
//
// ----------------------------------------------------------------------- //
HRECORD CGameDatabaseReader::GetRecord( HDATABASE hDatabase, const char *pszCategory, const char *pszRecord ) const
{
	
	HRECORD hRec = g_pLTDatabase->GetRecord(hDatabase,pszCategory,pszRecord);
	if (!hRec)
	{
		LTSNPrintF(szDBErrorString,LTARRAYSIZE(szDBErrorString),"Failed to read record '%s' from category '%s'.",pszRecord,pszCategory);
		LTASSERT(hRec,szDBErrorString);
	}
	return hRec;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseCreator::CGameDatabaseCreator()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseCreator::CGameDatabaseCreator()
{
	LoadDatabaseInterface( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseCreator::~CGameDatabaseCreator()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseCreator::~CGameDatabaseCreator()
{
	// Clean up...

	FreeDatabaseInterface( );
}
