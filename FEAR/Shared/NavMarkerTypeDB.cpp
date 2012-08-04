// ----------------------------------------------------------------------- //
//
// MODULE  : NavMarkerTypeDB.cpp
//
// PURPOSE : NavMarker database implementation
//
// CREATED : 11/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "NavMarkerTypeDB.h"

//
// Defines...
//

const char * const NMTDB_Category = "Interface/NavMarkerTypes";

const char * const NMTDB_WorldOffset = "World Offset";
const char * const NMTDB_IconTexture  = "Icon Texture";
const char * const NMTDB_IconSize = "Icon Size";
const char * const NMTDB_IconOffset = "Icon Offset";
const char * const NMTDB_IconColor = "Icon Color";
const char * const NMTDB_UseRange = "Use Range";
const char * const NMTDB_RangeOffset = "Range Offset";
const char * const NMTDB_Layout = "Layout";
const char * const NMTDB_TextColor = "Text Color";
const char * const NMTDB_ArrowTexture = "Arrow Texture";
const char * const NMTDB_ArrowSize = "Arrow Size";
const char * const NMTDB_ArrowColor = "Arrow Color";
const char * const NMTDB_Priority = "Priority";
const char * const NMTDB_sFXName = "FXName";
const char * const NMTDB_Lifetime = "Lifetime";
const char * const NMTDB_MultiplayerFadeAngle = "MultiplayerFadeAngle";
const char * const NMTDB_vFadeRange = "FadeRange";
const char * const NMTDB_fMinimumFadeAlpha = "MinimumFadeAlpha";

const char * const NLDB_Category = "Interface/NavMarkerLayouts";

const char * const NLDB_TextFont = "Text Font";
const char * const NLDB_TextOffset = "Text Offset";
const char * const NLDB_TextSize = "Text Size";

//
// Globals...
//

CNavMarkerTypeDB* g_pNavMarkerTypeDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDB::CNavMarkerTypeDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerTypeDB::CNavMarkerTypeDB()
{
	m_hCatTypes = NULL;
	m_nNumTypes = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDB::~CNavMarkerTypeDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerTypeDB::~CNavMarkerTypeDB()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //

bool CNavMarkerTypeDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pNavMarkerTypeDB = this;

	// Get handles to all of the categories in the database...

	m_hCatTypes = g_pLTDatabase->GetCategory( m_hDatabase, NMTDB_Category );
	if( !m_hCatTypes )
		return false;

	ASSERT( g_pLTDatabase->GetNumRecords( m_hCatTypes ) == ( uint16 )g_pLTDatabase->GetNumRecords( m_hCatTypes ));
	m_nNumTypes = ( uint16 )g_pLTDatabase->GetNumRecords( m_hCatTypes );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDB::GetRecord()
//
//	PURPOSE:	Get a handle to a specified record...
//
// ----------------------------------------------------------------------- //

HNAVMARKERTYPE CNavMarkerTypeDB::GetRecord(const char *pszType ) const
{
	if( !pszType )
		return NULL;

	return g_pLTDatabase->GetRecord( m_hCatTypes, pszType );
}

LTVector CNavMarkerTypeDB::GetWorldOffset(HNAVMARKERTYPE hRecord) const
{
	return GetVector3(hRecord,NMTDB_WorldOffset);
}

const char * CNavMarkerTypeDB::GetIconTexture(HNAVMARKERTYPE hRecord) const
{
	static const char * const szDef = "Interface\\HUD\\Nav_marker.dds";
	return GetString(hRecord,NMTDB_IconTexture,0,szDef);
}

LTVector2 CNavMarkerTypeDB::GetIconSize(HNAVMARKERTYPE hRecord) const
{
	static LTVector2 vDef(32.0f,32.0f);
	return GetVector2(hRecord,NMTDB_IconSize,0,vDef);
}

LTVector2 CNavMarkerTypeDB::GetIconOffset(HNAVMARKERTYPE hRecord) const
{
	return GetVector2(hRecord,NMTDB_IconOffset);
}

uint32 CNavMarkerTypeDB::GetIconColor(HNAVMARKERTYPE hRecord) const
{
	static uint32 argbDef = 0xFFFFFFFF;
	return (uint32)GetInt32(hRecord,NMTDB_IconColor,0,argbDef);
}

 //global, equals text size of "default" type
uint8 CNavMarkerTypeDB::GetRangeSize() const
{
	HNAVMARKERTYPE hRecord = GetRecord("Default");
	return GetTextSize( hRecord );
}

LTVector2 CNavMarkerTypeDB::GetRangeOffset(HNAVMARKERTYPE hRecord) const
{
	static LTVector2 vDef(16.0f,0.0f);
	return GetVector2(hRecord,NMTDB_RangeOffset,0,vDef);
}

bool CNavMarkerTypeDB::UseRange(HNAVMARKERTYPE hRecord) const
{
	return GetBool(hRecord,NMTDB_UseRange,0,true);
}

uint8 CNavMarkerTypeDB::GetTextSize(HNAVMARKERTYPE hRecord) const
{
	HRECORD hLayout = GetLayoutRecord( hRecord );
	return CNavMarkerLayoutDB::Instance( ).GetTextSize( hLayout );
}

const char* CNavMarkerTypeDB::GetTextFont(HNAVMARKERTYPE hRecord) const
{
	HRECORD hLayout = GetLayoutRecord( hRecord );
	return CNavMarkerLayoutDB::Instance( ).GetTextFont( hLayout );
}


LTVector2 CNavMarkerTypeDB::GetTextOffset(HNAVMARKERTYPE hRecord) const
{
	HRECORD hLayout = GetLayoutRecord( hRecord );
	return CNavMarkerLayoutDB::Instance( ).GetTextOffset( hLayout );
}

uint32 CNavMarkerTypeDB::GetTextColor(HNAVMARKERTYPE hRecord) const
{
	static uint32 argbDef = 0xFFFFFFFF;
	return (uint32)GetInt32(hRecord,NMTDB_TextColor,0,argbDef);
}

const char * CNavMarkerTypeDB::GetArrowTexture(HNAVMARKERTYPE hRecord) const
{
	static const char * const szDef = "Interface\\HUD\\nav_arrow.dds";
	return GetString(hRecord,NMTDB_ArrowTexture,0,szDef);
}

LTVector2 CNavMarkerTypeDB::GetArrowSize(HNAVMARKERTYPE hRecord) const
{
	static LTVector2 vDef(32.0f,32.0f);
	return GetVector2(hRecord,NMTDB_ArrowSize,0,vDef);
}

uint32 CNavMarkerTypeDB::GetArrowColor(HNAVMARKERTYPE hRecord) const
{
	static uint32 argbDef = 0xFFFFFFFF;
	return (uint32)GetInt32(hRecord,NMTDB_ArrowColor,0,argbDef);
}

uint8 CNavMarkerTypeDB::GetPriority(HNAVMARKERTYPE hRecord) const
{
	return (uint8)GetInt32(hRecord,NMTDB_Priority,0,1);
}

const char * CNavMarkerTypeDB::GetClientFX(HNAVMARKERTYPE hRecord) const
{
	return GetString(hRecord,NMTDB_sFXName,0,"");
}

float CNavMarkerTypeDB::GetLifetime(HNAVMARKERTYPE hRecord) const
{
	return GetFloat(hRecord,NMTDB_Lifetime,0,0.0f);
}

float CNavMarkerTypeDB::GetMultiplayerFadeAngle(HNAVMARKERTYPE hRecord) const
{
	return GetFloat(hRecord,NMTDB_MultiplayerFadeAngle,0,0.0f);
}

LTVector2 CNavMarkerTypeDB::GetFadeRange(HNAVMARKERTYPE hRecord) const
{
	static LTVector2 vDef(0.0f,0.0f);
	return GetVector2(hRecord,NMTDB_vFadeRange,0,vDef);
}

float CNavMarkerTypeDB::GetMinimumFadeAlpha(HNAVMARKERTYPE hRecord) const
{
	return GetFloat(hRecord,NMTDB_fMinimumFadeAlpha,0,0.0f);
}




HRECORD CNavMarkerTypeDB::GetLayoutRecord(HNAVMARKERTYPE hRecord) const
{
	return CNavMarkerLayoutDB::Instance( ).GetRecordLinkToLocalizedDB( g_pLTDatabase->GetAttribute( hRecord, NMTDB_Layout ),
		0, CNavMarkerLayoutDB::Instance( ).GetCategory( ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerLayoutDB::CNavMarkerLayoutDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerLayoutDB::CNavMarkerLayoutDB()
{
	m_hLayoutCat = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerLayoutDB::~CNavMarkerLayoutDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerLayoutDB::~CNavMarkerLayoutDB()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerLayoutDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //

bool CNavMarkerLayoutDB::Init( const char *szDatabaseFile /* = DB_Default_Localized_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Get handles to all of the categories in the database...
	m_hLayoutCat = g_pLTDatabase->GetCategory( m_hDatabase, NLDB_Category );
	if( !m_hLayoutCat )
		return false;

	return true;
}

uint8 CNavMarkerLayoutDB::GetTextSize(HRECORD hRecord) const
{
	return (uint8)GetInt32(hRecord,NLDB_TextSize,0,12);
}

const char* CNavMarkerLayoutDB::GetTextFont(HRECORD hRecord) const
{
	HRECORD hFontRecord = GetRecordLink(hRecord,NLDB_TextFont);
	return GetString(hFontRecord,"Face");
}


LTVector2 CNavMarkerLayoutDB::GetTextOffset(HRECORD hRecord) const
{
	static LTVector2 vDef(16.0f,-16.0f);
	return GetVector2(hRecord,NLDB_TextOffset,0,vDef);
}



#ifdef _SERVERBUILD // Server-side only

/****************************************************************************
*
* CNavMarkerTypeDBPlugin is used to help facilitate populating the WorldEdit object
* properties for NavMarkers
*
*****************************************************************************/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDBPlugin::CNavMarkerTypeDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerTypeDBPlugin::CNavMarkerTypeDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDBPlugin::~CNavMarkerTypeDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CNavMarkerTypeDBPlugin::~CNavMarkerTypeDBPlugin()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNavMarkerTypeDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CNavMarkerTypeDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
										  const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( !aszStrings || !pcStrings || !g_pNavMarkerTypeDB )
	{
		LTERROR( "CNavMarkerTypeDBPlugin::PopulateStringList: Invalid input parameters" );
		return;
	}

	// Add an entry for each type combination...
	uint16 nNumTypes = g_pNavMarkerTypeDB->GetNumTypes();

	for( uint16 nType = 0; nType < nNumTypes; ++nType )
	{
		LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many types to fit in the list.  Enlarge list size?" );

		HNAVMARKERTYPE hType = g_pNavMarkerTypeDB->GetRecord( nType );
		if( !hType )
			continue;

		const char *pszTypeName = g_pNavMarkerTypeDB->GetRecordName( hType );
		if( !pszTypeName )
			continue;

		LTStrCpy( aszStrings[(*pcStrings)++], pszTypeName, cMaxStringLength );
	}
}


#endif // _SERVERBUILD
