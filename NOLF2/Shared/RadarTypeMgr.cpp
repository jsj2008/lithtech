// ----------------------------------------------------------------------- //
//
// MODULE  : RadarTypeMgr.cpp
//
// PURPOSE : RadarTypeMgr implementation
//
// CREATED : 6/06/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "WeaponMgr.h"
	#include "RadarTypeMgr.h"

//
// Defines...
//

	#define	RTMGR_TAG			"RadarType"

	#define	RTMGR_NAME			"Name"
	#define RTMGR_ICON			"Icon"
	#define RTMGR_DRAWORDER		"DrawOrder"

//
// Globals...
//

	CRadarTypeMgr	*g_pRadarTypeMgr = LTNULL;

	static char s_aTagName[30];
	static char s_aAttName[100];
	static char s_aBuffer[100];

#ifndef _CLIENTBUILD
	
	bool CRadarTypeMgrPlugin::sm_bInitted = false;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRadarTypeMgr::CRadarTypeMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRadarTypeMgr::CRadarTypeMgr()
: CGameButeMgr()
{
	m_RadarTypeList.Init( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarTypeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CRadarTypeMgr& CRadarTypeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CRadarTypeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRadarTypeMgr::~CRadarTypeMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CRadarTypeMgr::~CRadarTypeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRadarTypeMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CRadarTypeMgr::Term( )
{
	g_pRadarTypeMgr = LTNULL;

	m_RadarTypeList.Clear();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRadarTypeMgr::Init
//
//  PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CRadarTypeMgr::Init( const char *szAttributeFile )
{
	if( g_pRadarTypeMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the singelton ptr

	g_pRadarTypeMgr = this;


	// Read in the properties for each Radar Type record...

	int nNum = 0;
	sprintf( s_aTagName, "%s%d", RTMGR_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		RADARTYPE	*pRadarType = debug_new( RADARTYPE );

		if( pRadarType && pRadarType->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it to the list...

			pRadarType->nId = nNum;
			m_RadarTypeList.AddTail( pRadarType );
		}
		else
		{
			debug_delete( pRadarType );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", RTMGR_TAG, nNum );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CRadarTypeMgr::GetRadarType
//
//  PURPOSE:	Get the specified Radar Type record
//
// ----------------------------------------------------------------------- //

RADARTYPE *CRadarTypeMgr::GetRadarType( uint32 nId )
{
	RADARTYPE **pCur = LTNULL;

	pCur = m_RadarTypeList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->nId == nId )
		{
			return *pCur;
		}

		pCur = m_RadarTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CRadarTypeMgr::GetRadarType
//
//  PURPOSE:	Get the specified Radar Type record
//
// ----------------------------------------------------------------------- //

RADARTYPE *CRadarTypeMgr::GetRadarType(const char *pName )
{
	if( !pName ) return LTNULL;

	RADARTYPE **pCur = LTNULL;

	pCur = m_RadarTypeList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->szName[0] && (!_stricmp( (*pCur)->szName , pName )) )
		{
			return *pCur;
		}

		pCur = m_RadarTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RADARTYPE::RADARTYPE
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

RADARTYPE::RADARTYPE()
:	nId			( RTMGR_INVALID_ID ),
	szName		( LTNULL ),
	szIcon		( LTNULL )
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RADARTYPE::~RADARTYPE
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

RADARTYPE::~RADARTYPE()
{	
	debug_deletea( szName );
	debug_deletea( szIcon );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RADARTYPE::Init
//
//  PURPOSE:	Build the Radar Type struct
//
// ----------------------------------------------------------------------- //

bool RADARTYPE::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return false;

	szName		= GetString( ButeMgr, aTagName, RTMGR_NAME, RTMGR_MAX_NAME_LENGTH );
	szIcon		= GetString( ButeMgr, aTagName, RTMGR_ICON, RTMGR_MAX_FILE_PATH );
	nDrawOrder	= ButeMgr.GetInt(aTagName, RTMGR_DRAWORDER,0);

	return true;
}


#ifndef _CLIENTBUILD
////////////////////////////////////////////////////////////////////////////
//
// CRadarTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use RadarTypeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarTypeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CRadarTypeMgrPlugin::PreHook_EditStringList( const char* szRezPath,
													  const char* szPropName,
													  char** aszStrings,
													  uint32* pcStrings,
													  const uint32 cMaxStrings,
													  const uint32 cMaxStringLength )
{
	if( !sm_bInitted )
	{
		char szFile[256] = {0};
		sprintf( szFile, "%s\\%s", szRezPath, RTMGR_DEFAULT_FILE );
		
		CRadarTypeMgr& RadarType = CRadarTypeMgr::Instance( );
        
		RadarType.SetInRezFile( LTFALSE );
        RadarType.Init( szFile );
        
		sm_bInitted = true;

		if( !g_pRadarTypeMgr )
		{
			strcpy( aszStrings[(*pcStrings)++], "<Error - Radar Type Mgr not Inited!>" );
			return LT_UNSUPPORTED;
		}
	}

	if( !PopulateStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
	{
		strcpy( aszStrings[(*pcStrings)++], "<Error - Populating String List!>" );
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRadarTypeMgrPlugin::CRadarTypeMgrPlugin
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CRadarTypeMgrPlugin::PopulateStringList( char** aszStrings,
											  uint32* pcStrings,
											  const uint32 cMaxStrings,
											  const uint32 cMaxStringLength )
{
	ASSERT( aszStrings && pcStrings && g_pRadarTypeMgr ); 
	if( !aszStrings || !pcStrings || !g_pRadarTypeMgr ) return false;
	 
	// Add an entry for each Radar Type

	int nNumRadarTypes = g_pRadarTypeMgr->GetNumRadarTypes();

	RADARTYPE *pRadarType = LTNULL;

	for( int i = 0; i < nNumRadarTypes; ++i )
	{
		ASSERT(cMaxStrings > (*pcStrings) + 1);

		pRadarType = g_pRadarTypeMgr->GetRadarType( i );
		if( pRadarType && pRadarType->szName[0] )
		{
			uint32 dwRTNameLen = strlen( pRadarType->szName );
			if( dwRTNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings )
			{
				strcpy( aszStrings[(*pcStrings)++], pRadarType->szName );
			}
		}
	}
	
	return true;
}

#endif // _CLIENTBUILD