// ----------------------------------------------------------------------- //
//
// MODULE  : HostOptionsMapMgr.h
//
// PURPOSE : Declares the CHostOptionsMapMgr class.  This class reads and
//           writes a binary mapping file that maps a server options
//           file to a friendly unicode name.
//
// CREATED : 06/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#pragma once
#ifndef __HOSTOPTIONSMAPMGR_H__
#define __HOSTOPTIONSMAPMGR_H__

#include <vector>
#include <algorithm>
#include "memblockallocator.h"
#include "DebugNew.h"
#include "CommonUtilities.h"

#define HOSTOPTIONSMAP_FILE "HostOptions.map"

class CHostOptionsMapMgr
{
	PREVENT_OBJECT_COPYING( CHostOptionsMapMgr );

public:
	struct SHostOptionsMapBinaryPred
	{
		CHostOptionsMapMgr&		m_MapMgr;

		SHostOptionsMapBinaryPred( CHostOptionsMapMgr& MapMgr ) : 
			m_MapMgr(MapMgr) 
		{}

		bool operator()(const wchar_t* wsz, uint8* p) const {
			return (LTStrICmp(wsz, m_MapMgr.GetFriendlyName(p)) < 0);
		}
		bool operator()(uint8* p, const wchar_t* wsz) const {
			return (LTStrICmp(m_MapMgr.GetFriendlyName(p), wsz) < 0);
		}
	};

	struct SHostOptionsMapFindPred
	{
		CHostOptionsMapMgr&		m_MapMgr;
		const char*				m_szFileName;

		SHostOptionsMapFindPred( CHostOptionsMapMgr& MapMgr, const char* szFileName ) : 
			m_MapMgr(MapMgr),
			m_szFileName(szFileName)
		{}

		bool operator()(uint8* p) const {
			return LTStrIEquals(m_szFileName, m_MapMgr.GetFileName(p));
		}
	};

private:
	CHostOptionsMapMgr();
	~CHostOptionsMapMgr();

protected:
	// list of mappings
	// pointer to memory block that contains the mapping
	std::vector<uint8*>				m_Map;

	inline const char*		GetFileName( uint8* pBuffer );
	inline const wchar_t*	GetFriendlyName( uint8* pBuffer );

	// determines if a buffer contains valid data with memory bounds
	inline bool				IsBufferValid( uint8* pBuffer, uint32 nBufferSize );

public:

	// singleton instance
	static CHostOptionsMapMgr& Instance();

	// returns the path to the host options map file
	inline void				GetHostOptionsMapFilePath( char* szFilePath, uint32 nFilePathSize );

	// load the binary map file from disk given a file name
	bool					Load( const char* pszFilename = NULL );
	// save the binary map to file given a file name
	bool					Save( const char* pszFilename = NULL );

	// add a new map to the list
	// if the filename already exists in the map this function will fail
	// if the friendly name already exists in the map this function will fail
	inline bool				Add( const char* szFileName, const wchar_t* wszFriendlyName );
	// deletes a map given a filename
	inline bool				Delete( const char* szFileName );
	// deletes a map given a friendly name
	inline bool				Delete( const wchar_t* wszFriendlyName );
	// deletes a map given an index
	inline bool				Delete( uint32 nIndex );
	// renames the map
	inline bool				Rename( const wchar_t* wszFriendlyNameOld, const wchar_t* wszFriendlyNameNew );

	// returns the number of entries in the map
	uint32					GetEntryCount() { return (uint32)(size_t)m_Map.size();	}
	// returns the file name of an entry
	inline const char*		GetFileName( uint32 nIndex );
	// returns the friendly name of an entry
	inline const wchar_t*	GetFriendlyName( uint32 nIndex );

	// given a friendly name this function will return a file name 
	// or NULL if the friendly name is not found
	inline const char*		GetFileNameFromFriendlyName( const wchar_t* wszFriendlyName );
	// given a file name this function will return a friendly name 
	// or NULL if the file name is not found
	inline const wchar_t*	GetFriendlyNameFromFileName( const char* szFileName );

	// returns true if the file name is already mapped
	inline bool				IsFileNameMapped( const char* szFileName );
	// returns true if the friendly name is already mapped
	inline bool				IsFriendlyNameMapped( const wchar_t* wszFriendlyName );
};

/*
 *	Inlines
 */

const char* CHostOptionsMapMgr::GetFileName( uint8* pBuffer )
{
	LTASSERT( pBuffer, "Invalid buffer pointer" );
	if( !pBuffer )
		return NULL;

	return (const char*)pBuffer;
}

const char* CHostOptionsMapMgr::GetFileName( uint32 nIndex )
{
	LTASSERT( nIndex < GetEntryCount(), "Invalid map index" );
	if( nIndex >= GetEntryCount() )
		return NULL;

	return GetFileName( m_Map[nIndex] );
}

const wchar_t* CHostOptionsMapMgr::GetFriendlyName( uint8* pBuffer )
{
	LTASSERT( pBuffer, "Invalid buffer pointer" );
	if( !pBuffer )
		return NULL;

	uint32 nFileNameLength = LTStrLen( GetFileName(pBuffer) );
	return (const wchar_t*)(pBuffer + AlignAllocSize(nFileNameLength + 1));
}

const wchar_t* CHostOptionsMapMgr::GetFriendlyName( uint32 nIndex )
{
	LTASSERT( nIndex < GetEntryCount(), "Invalid map index" );
	if( nIndex >= GetEntryCount() )
		return NULL;

	return GetFriendlyName( m_Map[nIndex] );
}

const char* CHostOptionsMapMgr::GetFileNameFromFriendlyName( const wchar_t* wszFriendlyName )
{
	std::vector<uint8*>::iterator itrMap;
	itrMap = std::lower_bound( m_Map.begin(), m_Map.end(), wszFriendlyName, SHostOptionsMapBinaryPred(*this) );
	if( itrMap != m_Map.end() && LTStrIEquals(GetFriendlyName(*itrMap), wszFriendlyName) )
		return GetFileName(*itrMap);

	return NULL;
}

const wchar_t* CHostOptionsMapMgr::GetFriendlyNameFromFileName( const char* szFileName )
{
	std::vector<uint8*>::iterator itrMap;
	itrMap = std::find_if( m_Map.begin(), m_Map.end(), SHostOptionsMapFindPred(*this, szFileName) );
	if( itrMap != m_Map.end() )
		return GetFriendlyName(*itrMap);

	return NULL;
}

bool CHostOptionsMapMgr::IsFileNameMapped( const char* szFileName )
{
	LTASSERT(szFileName, "Invalid file name");
	if( !szFileName )
		return false;
	return (GetFriendlyNameFromFileName(szFileName) != NULL);
}

bool CHostOptionsMapMgr::IsFriendlyNameMapped( const wchar_t* wszFriendlyName )
{
	LTASSERT(wszFriendlyName, "Invalid friendly name");
	if( !wszFriendlyName )
		return false;
	return (GetFileNameFromFriendlyName(wszFriendlyName) != NULL);
}

bool CHostOptionsMapMgr::Add( const char* szFileName, const wchar_t* wszFriendlyName )
{
	LTASSERT(szFileName, "Invalid file name");
	LTASSERT(wszFriendlyName, "Invalid friendly name");
	if( !szFileName || !wszFriendlyName )
		return false;

	if( IsFileNameMapped(szFileName) )
		return false;
	if( IsFriendlyNameMapped(wszFriendlyName) )
		return false;

	uint32 nFileNameLength = LTStrLen( szFileName );
	uint32 nFriendlyNameLength = LTStrLen( wszFriendlyName );

	uint32 nBufferSize = AlignAllocSize(nFileNameLength + 1) + ((nFriendlyNameLength + 1) * sizeof(wchar_t));
	uint8* pBuffer = debug_newa(uint8, nBufferSize);
	LTASSERT( pBuffer, "Out of memory!" );
	if( !pBuffer )
		return false;

	// copy the strings to the buffer
	LTStrCpy( (char*)pBuffer, szFileName, nFileNameLength + 1 );
	LTStrCpy( (wchar_t*)(pBuffer + AlignAllocSize(nFileNameLength + 1)), wszFriendlyName, nFriendlyNameLength + 1 );

	std::vector<uint8*>::iterator itrMap;
	itrMap = std::lower_bound( m_Map.begin(), m_Map.end(), wszFriendlyName, SHostOptionsMapBinaryPred(*this) );
	m_Map.insert( itrMap, pBuffer );

	return true;
}

bool CHostOptionsMapMgr::Delete( const char* szFileName )
{
	std::vector<uint8*>::iterator itrMap;
	itrMap = std::find_if( m_Map.begin(), m_Map.end(), SHostOptionsMapFindPred(*this, szFileName) );
	if( itrMap != m_Map.end() )
		return Delete( (uint32)(itrMap - m_Map.begin()) );

	return false;
}

bool CHostOptionsMapMgr::Delete( const wchar_t* wszFriendlyName )
{
	std::vector<uint8*>::iterator itrMap;
	itrMap = std::lower_bound( m_Map.begin(), m_Map.end(), wszFriendlyName, SHostOptionsMapBinaryPred(*this) );
	if( itrMap != m_Map.end() && LTStrIEquals(GetFriendlyName(*itrMap), wszFriendlyName) )
		return Delete( (uint32)(itrMap - m_Map.begin()) );

	return false;
}

bool CHostOptionsMapMgr::Delete( uint32 nIndex )
{
	LTASSERT( nIndex < GetEntryCount(), "Invalid map index" );
	if( nIndex >= GetEntryCount() )
		return false;

	debug_deletea(m_Map[nIndex]);
	m_Map.erase( m_Map.begin() + nIndex );

	return true;
}

bool CHostOptionsMapMgr::IsBufferValid( uint8* pBuffer, uint32 nBufferSize )
{
	// it is safe to get the file name because it is at the beginning of the file
	const char* szFileName = GetFileName( pBuffer );
	
	uint32 nChar;
	for(nChar=0;nChar<nBufferSize;++nChar)
	{
		if( szFileName[nChar] == '\0' )
			break;
	}
	if( nChar == nBufferSize )
		return false;

	// it is now safe to get the friendly name
	const wchar_t* wszFriendlyName = GetFriendlyName( pBuffer );
	uint32 nBytesLeft = nBufferSize - ((uint8*)wszFriendlyName - pBuffer);
	if( nBytesLeft == 0 )
		return false;

	uint32 nCharLeft = nBytesLeft / 2;

	for(nChar=0;nChar<=nCharLeft;++nChar)
	{
		if( wszFriendlyName[nChar] == '\0' )
			break;
	}
	if( nChar > nCharLeft )
		return false;

	return true;
}

void CHostOptionsMapMgr::GetHostOptionsMapFilePath( char* szFilePath, uint32 nFilePathSize )
{
	LTASSERT( szFilePath, "Invalid string buffer" );
	if( !szFilePath )
		return;

	LTStrCpy( szFilePath, HOSTOPTIONSMAP_FILE, nFilePathSize );
}

bool CHostOptionsMapMgr::Rename( const wchar_t* wszFriendlyNameOld, const wchar_t* wszFriendlyNameNew )
{
	LTASSERT( wszFriendlyNameOld, "Invalid old string pointer" );
	LTASSERT( wszFriendlyNameNew, "Invalid new string pointer" );
	if( !wszFriendlyNameOld || !wszFriendlyNameNew )
		return false;

	const char* szFileName = GetFileNameFromFriendlyName( wszFriendlyNameOld );
	if( !szFileName )
		return false;

	if( !Add(szFileName, wszFriendlyNameNew) )
		return false;

	Delete( wszFriendlyNameOld );

	return true;
}


#endif  // __HOSTOPTIONSMAPMGR_H__
