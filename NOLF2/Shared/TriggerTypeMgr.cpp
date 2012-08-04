// ----------------------------------------------------------------------- //
//
// MODULE  : TriggerTypeMgr.cpp
//
// PURPOSE : TriggerTypeMgr implementation
//
// CREATED : 7/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "WeaponMgr.h"
	#include "TriggerTypeMgr.h"


//
// Defines...
//

	#define	TTMGR_TAG		"TriggerType"
	
	#define TTMGR_NAME		"Name"
	#define TTMGR_ICON		"Icon"

//
// Globals...
//

	CTriggerTypeMgr	*g_pTriggerTypeMgr = LTNULL;
	
	static char s_aTagName[32];
	static char	s_aAttName[100];
	static char s_aBuffer[100];
	
#ifndef _CLIENTBUILD
	
	bool CTriggerTypeMgrPlugin::sm_bInitted = false;

#endif // _CLIENTBUILD


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerTypeMgr::CTriggerTypeMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTriggerTypeMgr::CTriggerTypeMgr()
:	CGameButeMgr()
{
	m_TriggerTypeList.Init( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerTypeMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CTriggerTypeMgr& CTriggerTypeMgr::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static CTriggerTypeMgr sSingleton;
	return sSingleton;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerTypeMgr::~CTriggerTypeMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTriggerTypeMgr::~CTriggerTypeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerTypeMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CTriggerTypeMgr::Term( )
{
	g_pTriggerTypeMgr = LTNULL;

	m_TriggerTypeList.Clear();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTriggerTypeMgr::Init
//
//  PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CTriggerTypeMgr::Init( const char *szAttributeFile /* = TTMGR_DEFAULT_FILE  */ )
{
	if( g_pTriggerTypeMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the singelton ptr

	g_pTriggerTypeMgr = this;

	
	// Read in the properies for each Trigger Type Record...

	int nNum = 0;
	sprintf( s_aTagName, "%s%i", TTMGR_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		TRIGGERTYPE	*pTriggerType = debug_new( TRIGGERTYPE );

		if( pTriggerType && pTriggerType->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it to the list...
			
			pTriggerType->nId = nNum;
			m_TriggerTypeList.AddTail( pTriggerType );
		}
		else
		{
			debug_delete( pTriggerType );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%i", TTMGR_TAG, nNum );
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CTriggerTypeMgr::GetTriggerType
//
//  PURPOSE:	Get the specified Trigger Type record
//
// ----------------------------------------------------------------------- //

TRIGGERTYPE *CTriggerTypeMgr::GetTriggerType( uint32 nId )
{
	TRIGGERTYPE **ppCur = LTNULL;

	ppCur = m_TriggerTypeList.GetItem( TLIT_FIRST );

	while( ppCur )
	{
		if( *ppCur && (*ppCur)->nId == nId )
		{
			return *ppCur;
		}

		ppCur = m_TriggerTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CTriggerTypeMgr::GetTriggerType
//
//  PURPOSE:	Get the specified Trigger Type record
//
// ----------------------------------------------------------------------- //

TRIGGERTYPE *CTriggerTypeMgr::GetTriggerType( const char *pName )
{
	if( !pName ) return LTNULL;
	
	TRIGGERTYPE **ppCur = LTNULL;

	ppCur = m_TriggerTypeList.GetItem( TLIT_FIRST );

	while( ppCur )
	{
		if( *ppCur && (*ppCur)->szName[0] && (!_stricmp( (*ppCur)->szName, pName )) )
		{
			return *ppCur;
		}

		ppCur = m_TriggerTypeList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TRIGGERTYPE::TRIGGERTYPE
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TRIGGERTYPE::TRIGGERTYPE( )
:	nId		( TTMGR_INVALID_ID ),
	szName	( LTNULL ),
	szIcon	( LTNULL )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TRIGGERTYPE::~TRIGGERTYPE
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

TRIGGERTYPE::~TRIGGERTYPE( )
{
	debug_delete( szName );
	debug_delete( szIcon );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	TRIGGERTYPE::Init
//
//  PURPOSE:	Build the Trigger Type struct
//
// ----------------------------------------------------------------------- //

bool TRIGGERTYPE::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return false;

	szName	= GetString( ButeMgr, aTagName, TTMGR_NAME, TTMGR_MAX_NAME_LENGTH );
	szIcon	= GetString( ButeMgr, aTagName, TTMGR_ICON, TTMGR_MAX_FILE_PATH );

	return true;
}


#ifndef _CLIENTBUILD
////////////////////////////////////////////////////////////////////////////
//
// CTriggerTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use TriggerTypeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerTypeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CTriggerTypeMgrPlugin::PreHook_EditStringList( const char* szRezPath,
														 const char* szPropName,
														 char** aszStrings,
														 uint32* pcStrings,
														 const uint32 cMaxStrings,
														 const uint32 cMaxStringLength )
{
	if( !sm_bInitted )
	{
		char szFile[256] = {0};
		sprintf( szFile, "%s\\%s", szRezPath, TTMGR_DEFAULT_FILE );
		
		CTriggerTypeMgr& TriggerType = CTriggerTypeMgr::Instance( );
        
		TriggerType.SetInRezFile( LTFALSE );
        TriggerType.Init( szFile );
        
		sm_bInitted = true;

		if( !g_pTriggerTypeMgr )
		{
			strcpy( aszStrings[(*pcStrings)++], "<Error - Trigger Type Mgr not Inited!>" );
		}
	}

	if( !PopulateStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
	{
		strcpy( aszStrings[(*pcStrings)++], "<Error - Populating String List!>" );
	}
	else
	{
		// Sort the list...
			
		qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTriggerTypeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CTriggerTypeMgrPlugin::PopulateStringList( char** aszStrings,
												 uint32* pcStrings,
												 const uint32 cMaxStrings,
												 const uint32 cMaxStringLength )
{
	ASSERT( aszStrings && pcStrings && g_pTriggerTypeMgr ); 
	if( !aszStrings || !pcStrings || !g_pTriggerTypeMgr ) return false;

	// always make the first option an invalid Trigger Type...

	strcpy( aszStrings[(*pcStrings)++], "<none>" );
	 
	// Add an entry for each Activate Type

	int nNumActivateTypes = g_pTriggerTypeMgr->GetNumTriggerTypes();

	TRIGGERTYPE *pTriggerType = LTNULL;

	for( int i = 0; i < nNumActivateTypes; ++i )
	{
		ASSERT(cMaxStrings > (*pcStrings) + 1);

		pTriggerType = g_pTriggerTypeMgr->GetTriggerType( i );
		if( pTriggerType && pTriggerType->szName[0] )
		{
			uint32 dwATNameLen = strlen( pTriggerType->szName );
			if( dwATNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings )
			{
				strcpy( aszStrings[(*pcStrings)++], pTriggerType->szName );
			}
		}
	}
	
	return true;
}

#endif // _CLIENTBUILD