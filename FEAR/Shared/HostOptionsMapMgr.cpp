// ----------------------------------------------------------------------- //
//
// MODULE  : HostOptionsMapMgr.cpp
//
// PURPOSE : Defines the CHostOptionsMapMgr class.  This class reads and
//           writes a binary mapping file that maps a server options
//           file to a friendly unicode name.
//
// CREATED : 06/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HostOptionsMapMgr.h"
#include "iltfilemgr.h"
#include "ltfileoperations.h"

#define HOST_OPTION_FILE_GUARD		LTMakeFourCC('M', 'P', 'H', 'O')
#define HOST_OPTION_FILE_VERSION	1


// constructor
CHostOptionsMapMgr::CHostOptionsMapMgr()
{
	Load();
}

// destructor
CHostOptionsMapMgr::~CHostOptionsMapMgr()
{
	uint32 nMapCount = GetEntryCount();
	for(uint32 nMap=0;nMap<nMapCount;++nMap)
		debug_deletea(m_Map[nMap]);
	
	m_Map.clear();
}

// returns a singleton instance of this class
CHostOptionsMapMgr& CHostOptionsMapMgr::Instance()
{
	static CHostOptionsMapMgr s_HostOptionsMapMgr;
	return s_HostOptionsMapMgr;
}

// load the binary map file from disk given a file name
bool CHostOptionsMapMgr::Load( const char* pszFilename /*= NULL*/ )
{
	LTASSERT( g_pLTBase, "Base pointer not set.  Base pointer is needed for FileMgr access." );
	if( !g_pLTBase )
		return false;

	char szFilePath[MAX_PATH];
	if( pszFilename )
		LTStrCpy( szFilePath, pszFilename, LTARRAYSIZE(szFilePath) );
	else
		GetHostOptionsMapFilePath( szFilePath, LTARRAYSIZE(szFilePath) );

	// if the file does not exist then don't sweat it
	ILTInStream* pStream = g_pLTBase->FileMgr()->OpenUserFileForReading( szFilePath );
	if( !pStream )
		return true;

	// read the header
	uint32 nFileGuard = 0;
	*pStream >> nFileGuard;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	if( nFileGuard != HOST_OPTION_FILE_GUARD )
	{
		LTERROR( "Host options map file guard does not match" );
		LTSafeRelease( pStream );
		return false;
	}

	// read the version
	uint32 nVersion = 0;
	*pStream >> nVersion;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	if( nVersion != HOST_OPTION_FILE_VERSION )
	{
		LTERROR( "Host options map file version does not match" );
		LTSafeRelease( pStream );
		return false;
	}

	// read the number of entries
	uint32 nEntryCount = 0;
	*pStream >> nEntryCount;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	// reserve the amount of entries to avoid multiple allocations
	m_Map.reserve( nEntryCount );

	for(uint32 nEntry=0;nEntry<nEntryCount;++nEntry)
	{
		// read the size of the buffer
		uint32 nBufferSize = 0;
		*pStream >> nBufferSize;
		if( pStream->HasErrorOccurred() )
		{
			LTSafeRelease( pStream );
			return false;
		}

		uint8* pBuffer = debug_newa(uint8, nBufferSize);
		if( !pBuffer )
		{
			LTERROR_PARAM1( "Out of memory! Failed to allocated %d bytes of memory!", nBufferSize );
			LTSafeRelease( pStream );
			return false;
		}

		// read the buffer
		if( pStream->Read(pBuffer, nBufferSize) != LT_OK )
		{
			debug_deletea( pBuffer );
			LTSafeRelease( pStream );
			return false;
		}

		if( pStream->HasErrorOccurred() )
		{
			debug_deletea( pBuffer );
			LTSafeRelease( pStream );
			return false;
		}

		// check if this a valid buffer
		if( !IsBufferValid(pBuffer, nBufferSize) )
		{
			LTERROR( "Invalid buffer" );
			debug_deletea( pBuffer );
			continue;
		}

		const char* szFileName = GetFileName( pBuffer );
		const wchar_t* wszFriendlyName = GetFriendlyName( pBuffer );

		// make sure we don't add duplicates
		if( IsFileNameMapped(szFileName) || IsFriendlyNameMapped(wszFriendlyName) )
		{
			debug_deletea( pBuffer );
			continue;
		}

		std::vector<uint8*>::iterator itrMap;
		itrMap = std::lower_bound( m_Map.begin(), m_Map.end(), wszFriendlyName, SHostOptionsMapBinaryPred(*this) );
		m_Map.insert( itrMap, pBuffer );
	}

	// close the file
	LTSafeRelease( pStream );

	return true;
}

// save the binary map to file given a file name
bool CHostOptionsMapMgr::Save( const char* pszFilename /*= NULL*/ )
{
	LTASSERT( g_pLTBase, "Base pointer not set.  Base pointer is needed for FileMgr access." );
	if( !g_pLTBase )
		return false;

	char szFilePath[MAX_PATH];
	if( pszFilename )
		LTStrCpy( szFilePath, pszFilename, LTARRAYSIZE(szFilePath) );
	else
		GetHostOptionsMapFilePath( szFilePath, LTARRAYSIZE(szFilePath) );

	ILTOutStream* pStream = g_pLTBase->FileMgr()->OpenUserFileForWriting( szFilePath );
	LTASSERT( pStream, "Failed to open file for writing" );
	if( !pStream )
		return false;

	// save the header
	*pStream << (uint32)HOST_OPTION_FILE_GUARD;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	// save the version
	*pStream << (uint32)HOST_OPTION_FILE_VERSION;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	// save the number of entries
	uint32 nEntryCount = GetEntryCount();
	*pStream << nEntryCount;
	if( pStream->HasErrorOccurred() )
	{
		LTSafeRelease( pStream );
		return false;
	}

	// save each entry
	for(uint32 nEntry=0;nEntry<nEntryCount;++nEntry)
	{
		const char* szFileName = GetFileName( nEntry );
		const wchar_t* wszFriendlyName = GetFriendlyName( nEntry );

		uint32 nFileNameLength = LTStrLen( szFileName );
		uint32 nFriendlyNameLength = LTStrLen( wszFriendlyName );

		// save the size of the buffer
		uint32 nBufferSize = AlignAllocSize(nFileNameLength + 1) + ((nFriendlyNameLength + 1) * sizeof(wchar_t));
		*pStream << nBufferSize;
		if( pStream->HasErrorOccurred() )
		{
			LTSafeRelease( pStream );
			return false;
		}

		// save the buffer
		pStream->Write( m_Map[nEntry], nBufferSize );
		if( pStream->HasErrorOccurred() )
		{
			LTSafeRelease( pStream );
			return false;
		}
	}

	// close the file
	LTSafeRelease( pStream );

	return true;
}
