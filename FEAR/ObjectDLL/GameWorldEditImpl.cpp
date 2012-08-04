// ----------------------------------------------------------------------- //
//
// MODULE  : GameWorldEditImpl.cpp
//
// PURPOSE : Defines the CGameWorldEditImpl class.  This class
//           implements the IGameWorldEdit interface.  This interface
//           is used to communicate with WorldEdit.
//
// CREATED : 01/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GameWorldEditImpl.h"
#include "SurfaceDB.h"

//----------------------
// GetIGameWorldEdit
//
// Handles acquiring an IGameWorldEdit object for the calling binaries

extern "C"
{
	MODULE_EXPORT IGameWorldEdit* GetIGameWorldEdit()
	{
		//we only have a singleton of this object so return that
		static CGameWorldEditImpl s_GameWorldEdit;
		return &s_GameWorldEdit;
	}
}

// constructor
CGameWorldEditImpl::CGameWorldEditImpl()
{
}

// destructor
CGameWorldEditImpl::~CGameWorldEditImpl()
{
}

// returns the number of surface records in the database
uint32 CGameWorldEditImpl::GetSurfaceCount()
{
	LTASSERT( g_pSurfaceDB, "Surface database not initialized!" );
	if( !g_pSurfaceDB )
		return 0;

	return g_pSurfaceDB->GetNumSurfaces();
}

// returns the name of the specified surface
bool CGameWorldEditImpl::GetSurfaceName( uint32 nSurface, char* szSurface, uint32 nLength )
{
	LTASSERT( g_pSurfaceDB, "Surface database not initialized!" );
	if( !g_pSurfaceDB )
		return false;

	if( nSurface >= g_pSurfaceDB->GetNumSurfaces() )
	{
		LTERROR( "Invalid surface index!" );
		return false;
	}

	HRECORD hSurface = g_pSurfaceDB->GetSurfaceByIndex( nSurface );
	if( !hSurface )
	{
		LTERROR( "Surface not found!" );
		return false;
	}

	const char* pszName = g_pSurfaceDB->GetRecordName(hSurface);
	if( !pszName )
	{
		LTERROR( "Surface has no name!" );
		return false;
	}

	LTStrCpy( szSurface, pszName, nLength ); 

	return true;
}

// returns the Id of the specified surface
bool CGameWorldEditImpl::GetSurfaceId( uint32 nSurface, uint32& nId )
{
	LTASSERT( g_pSurfaceDB, "Surface database not initialized!" );
	if( !g_pSurfaceDB )
		return false;

	if( nSurface >= g_pSurfaceDB->GetNumSurfaces() )
	{
		LTERROR( "Invalid surface index!" );
		return false;
	}

	HRECORD hSurface = g_pSurfaceDB->GetSurfaceByIndex( nSurface );
	if( !hSurface )
	{
		LTERROR( "Surface not found!" );
		return false;
	}

	nId = (uint32)g_pSurfaceDB->GetInt32( hSurface, SrfDB_Srf_nId );

	return true;
}

// validates the surface records and returns the first error encountered in the passed in string
bool CGameWorldEditImpl::ValidateSurfaceRecords( char* szError, uint32 nLength )
{
	HRECORD pRecordArray[SrfDB_MaxSurfaces];
	memset( pRecordArray, 0, sizeof(pRecordArray) );

	for(uint32 nSurface=0;nSurface<g_pSurfaceDB->GetNumSurfaces();++nSurface)
	{
		HRECORD hSurface = g_pSurfaceDB->GetSurfaceByIndex( nSurface );
		if( !hSurface )
			continue;

		int32 nId = g_pSurfaceDB->GetInt32( hSurface, SrfDB_Srf_nId );

		if( nId >= SrfDB_MaxSurfaces ) 
		{
			const char* pszName1 = g_pSurfaceDB->GetRecordName(hSurface);
			LTSNPrintF( szError, nLength, "The surface record for '%s' contains an invalid ID of '%d'.  The surface ID must range is [0, %d[.", pszName1, nId, SrfDB_MaxSurfaces );
			return false;
		}

		if( pRecordArray[nId] )
		{
			const char* pszName1 = g_pSurfaceDB->GetRecordName(hSurface);
			const char* pszName2 = g_pSurfaceDB->GetRecordName(pRecordArray[nId]);
			LTSNPrintF( szError, nLength, "The surface records for '%s' and '%s' use the same surface ID of '%d'", pszName1, pszName2, nId );
			return false;
		}

		pRecordArray[nId] = hSurface;
	}

	return true;
}
