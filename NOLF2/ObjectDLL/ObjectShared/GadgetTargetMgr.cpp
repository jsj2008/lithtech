// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetMgr.cpp
//
// PURPOSE : GadgetTargetMgr implementation
//
// CREATED : 9/04/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "GadgetTargetMgr.h"
	#include "WeaponMgr.h"
	#include "FXButeMgr.h"

//
// Define...
//

	#define	GTMGR_TAG					"GadgetTarget"
										
	#define GTMGR_NAME					"Name"
	#define	GTMGR_FILENAME				"FileName"
	#define GTMGR_DEBRISTYPE			"DebrisType"
	#define	GTMGR_DISABLINGSOUND		"DisablingSound"
	#define GTMGR_DISABLEDSOUND			"DisabledSound"
	#define	GTMGR_SKIN					"Skin"
	#define	GTMGR_RENDERSTYLE			"RenderStyle"
	#define GTMGR_SCALE					"Scale"
	#define	GTMGR_OBJECTCOLOR			"ObjectColor"
	#define	GTMGR_ALPHA					"Alpha"
	#define GTMGR_HITPOINTS				"HitPoints"
	#define GTMGR_SOLID					"Solid"
	#define GTMGR_VISIBLE				"Visible"
	#define GTMGR_SHADOW				"Shadow"
	#define GTMGR_MOVETOFLOOR			"MoveToFloor"
	#define GTMGR_ADDITIVE				"Additive"
	#define GTMGR_MULTIPLY				"Multiply"
	#define GTMGR_TYPE					"Type"
	#define GTMGR_MINTIME				"MinTime"
	#define GTMGR_MAXTIME				"MaxTime"
	#define GTMGR_SOUNDRADIUS			"SoundRadius"
	#define GTMGR_CODEID				"CodeID"
	#define GTMGR_REMOVEWHENDISABLED	"RemoveWhenDisabled"
	#define GTMGR_INFINITEDISABLES		"InfiniteDisables"
	#define GTMGR_INFINITEACTIVATES		"InfiniteActivates"

	#define	GTMGR_LIGHTTYPE				"LightType"
	#define	GTMGR_LIGHTSCALE			"LightScale"
	#define	GTMGR_LIGHTRENDERSTYLE		"LightRenderStyle"
	#define	GTMGR_LIGHT1FILENAME		"Light1FileName"
	#define	GTMGR_LIGHT1SKIN			"Light1Skin"
	#define	GTMGR_LIGHT1SOCKET			"Light1Socket"
	#define	GTMGR_LIGHT2FILENAME		"Light2FileName"
	#define	GTMGR_LIGHT2SKIN			"Light2Skin"
	#define	GTMGR_LIGHT2SOCKET			"Light2Socket"


//
// Globals...
//

	CGadgetTargetMgr	*g_pGadgetTargetMgr = LTNULL;

	static char s_aTagName[30];
	static char s_aAttName[100];
	static char s_aBuffer[100];

	// Plugin Static

	CGadgetTargetMgr	CGadgetTargetMgrPlugin::sm_GadgetTargetMgr;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetMgr::CGadgetTargetMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGadgetTargetMgr::CGadgetTargetMgr()
: CGameButeMgr()
{
	m_GadgetTargetList.Init( LTTRUE );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetMgr::~CGadgetTargetMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGadgetTargetMgr::~CGadgetTargetMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetMgr::Init
//
//  PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CGadgetTargetMgr::Init( const char *szAttributeFile )
{
	if( g_pGadgetTargetMgr || !szAttributeFile ) return LTFALSE;
	if( !Parse( szAttributeFile )) return LTFALSE;

	// Set the singelton ptr

	g_pGadgetTargetMgr = this;


	// Read in the properties for each Gadget Target record...

	int nNum = 0;
	sprintf( s_aTagName, "%s%d", GTMGR_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		GADGETTARGET	*pGadgetTarget = debug_new( GADGETTARGET );

		if( pGadgetTarget && pGadgetTarget->Init( m_buteMgr, s_aTagName ))
		{
			// Set the ID and add it to the list...

			pGadgetTarget->nId = nNum;
			m_GadgetTargetList.AddTail( pGadgetTarget );
		}
		else
		{
			debug_delete( pGadgetTarget );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", GTMGR_TAG, nNum );
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CGadgetTargetMgr::GetGadgetTarget
//
//  PURPOSE:	Get the specified Gadget Target record
//
// ----------------------------------------------------------------------- //

GADGETTARGET *CGadgetTargetMgr::GetGadgetTarget( uint32 nId )
{
	GADGETTARGET **pCur = LTNULL;

	pCur = m_GadgetTargetList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->nId == nId )
		{
			return *pCur;
		}

		pCur = m_GadgetTargetList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CGadgetTargetMgr::GetGadgetTarget
//
//  PURPOSE:	Get the specified Gadget Target record
//
// ----------------------------------------------------------------------- //

GADGETTARGET *CGadgetTargetMgr::GetGadgetTarget( char *pName )
{
	if( !pName ) return LTNULL;

	GADGETTARGET **pCur = LTNULL;

	pCur = m_GadgetTargetList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->szName[0] && (!_stricmp( (*pCur)->szName , pName )) )
		{
			return *pCur;
		}

		pCur = m_GadgetTargetList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CGadgetTargetMgr::Term( )
{
	g_pGadgetTargetMgr = LTNULL;

	m_GadgetTargetList.Clear();
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GADGETTARGET::GADGETTARGET
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GADGETTARGET::GADGETTARGET()
:	nId					( GTMGR_INVALID_ID ),
	szName				( LTNULL ),
	szFileName			( LTNULL ),
	szDebrisType		( LTNULL ),
	szDisablingSnd		( LTNULL ),
	szDisabledSnd		( LTNULL ),
	vScale				( 1.0f, 1.0f, 1.0f ),
	vObjectColor		( 255.0f, 255.0f, 255.0f ),
	fAlpha				( 1.0f ),
	nHitPts				( -1 ),
	bSolid				( LTTRUE ),
	bVisible			( LTTRUE ),
	bShadow				( LTFALSE ),
	bMoveToFloor		( LTFALSE ),
	bAdditive			( LTFALSE ),
	bMultiply			( LTFALSE ),
	nType				( -1 ),
	fMinTime			( 8.0f ),
	fMaxTime			( 0.0f ),
	fSoundRadius		( 400.0f ),
	dwCodeID			( 0 ),
	bRemoveWhenDisabled	( LTFALSE ),
	bInfiniteDisables	( LTFALSE ),
	bInfiniteActivates	( LTFALSE ),
	szLight1FileName	( LTNULL ),
	szLight2FileName	( LTNULL ),
	szLight1SocketName	( LTNULL ),
	szLight2SocketName	( LTNULL ),
	vLightScale			( 1.0f, 1.0f, 1.0f ),
	nLightType			( 0 )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GADGETTARGET::~GADGETTARGET
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

GADGETTARGET::~GADGETTARGET()
{	
	debug_deletea( szName );
	debug_deletea( szFileName );
	debug_deletea( szLight1FileName );
	debug_deletea( szLight2FileName );
	debug_deletea( szLight1SocketName );
	debug_deletea( szLight2SocketName );
	debug_deletea( szDebrisType );
	debug_deletea( szDisablingSnd );
	debug_deletea( szDisabledSnd );
}	


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GADGETTARGET::Init
//
//  PURPOSE:	Build the GadgetTarget struct
//
// ----------------------------------------------------------------------- //

LTBOOL	GADGETTARGET::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;

	szName			= GetString( ButeMgr, aTagName, GTMGR_NAME, GTMGR_MAX_NAME_LENGTH ); 
	szFileName		= GetString( ButeMgr, aTagName, GTMGR_FILENAME, GTMGR_MAX_FILE_PATH );
	szDebrisType	= GetString( ButeMgr, aTagName, GTMGR_DEBRISTYPE, GTMGR_MAX_NAME_LENGTH );
	szDisablingSnd	= GetString( ButeMgr, aTagName, GTMGR_DISABLINGSOUND, GTMGR_MAX_FILE_PATH );
	szDisabledSnd	= GetString( ButeMgr, aTagName, GTMGR_DISABLEDSOUND, GTMGR_MAX_FILE_PATH );

	blrGTSkinReader.Read( &ButeMgr, aTagName, GTMGR_SKIN, GTMGR_MAX_FILE_PATH );
	blrGTRenderStyleReader.Read( &ButeMgr, aTagName, GTMGR_RENDERSTYLE, GTMGR_MAX_FILE_PATH );

	vScale			= ButeMgr.GetVector( aTagName, GTMGR_SCALE );
	vObjectColor	= ButeMgr.GetVector( aTagName, GTMGR_OBJECTCOLOR );

	fAlpha			= (LTFLOAT)ButeMgr.GetDouble( aTagName, GTMGR_ALPHA, 1.0 );

	nHitPts			= ButeMgr.GetInt( aTagName, GTMGR_HITPOINTS );

	bSolid			= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_SOLID );
	bVisible		= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_VISIBLE );
	bShadow			= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_SHADOW );
	bMoveToFloor	= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_MOVETOFLOOR );
	bAdditive		= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_ADDITIVE );
	bMultiply		= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_MULTIPLY );

	nType			= ButeMgr.GetInt( aTagName, GTMGR_TYPE );
	fMinTime		= (LTFLOAT)ButeMgr.GetDouble( aTagName, GTMGR_MINTIME );
	fMaxTime		= (LTFLOAT)ButeMgr.GetDouble( aTagName, GTMGR_MAXTIME );
	fSoundRadius	= (LTFLOAT)ButeMgr.GetDouble( aTagName, GTMGR_SOUNDRADIUS );
	dwCodeID		= (uint32)ButeMgr.GetDouble( aTagName, GTMGR_CODEID );

	bRemoveWhenDisabled	= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_REMOVEWHENDISABLED );
	bInfiniteDisables	= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_INFINITEDISABLES );
	bInfiniteActivates	= (LTBOOL)ButeMgr.GetInt( aTagName, GTMGR_INFINITEACTIVATES );

	nLightType			= ButeMgr.GetInt( aTagName, GTMGR_LIGHTTYPE );
	vLightScale			= ButeMgr.GetVector( aTagName, GTMGR_LIGHTSCALE );
	blrGTLightRenderStyleReader.Read( &ButeMgr, aTagName, GTMGR_LIGHTRENDERSTYLE, GTMGR_MAX_FILE_PATH );

	szLight1FileName	= GetString( ButeMgr, aTagName, GTMGR_LIGHT1FILENAME, GTMGR_MAX_FILE_PATH );
	szLight2FileName	= GetString( ButeMgr, aTagName, GTMGR_LIGHT2FILENAME, GTMGR_MAX_FILE_PATH );
	szLight1SocketName	= GetString( ButeMgr, aTagName, GTMGR_LIGHT1SOCKET, GTMGR_MAX_FILE_PATH );
	szLight2SocketName	= GetString( ButeMgr, aTagName, GTMGR_LIGHT2SOCKET, GTMGR_MAX_FILE_PATH );

	blrGTLight1SkinReader.Read( &ButeMgr, aTagName, GTMGR_LIGHT1SKIN, GTMGR_MAX_FILE_PATH );
	blrGTLight2SkinReader.Read( &ButeMgr, aTagName, GTMGR_LIGHT2SKIN, GTMGR_MAX_FILE_PATH );

	return LTTRUE;
}



////////////////////////////////////////////////////////////////////////////
//
// CGadgetTargetMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CPropTypeMgr
//
////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGadgetTargetMgrPlugin::PreHook_EditStringList
//
//  PURPOSE:	Fill any string lists
//
// ----------------------------------------------------------------------- //

LTRESULT CGadgetTargetMgrPlugin::PreHook_EditStringList(
														const char* szRezPath,
														const char* szPropName,
														char** aszStrings,
														uint32* pcStrings,
														const uint32 cMaxStrings,
														const uint32 cMaxStringLength )
{
	if( !g_pGadgetTargetMgr )
	{
		// This will set the g_GadgetTargetMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf( szFile, "%s\\%s", szRezPath, GTMGR_DEFAULT_FILE );
        sm_GadgetTargetMgr.SetInRezFile( LTFALSE );
        sm_GadgetTargetMgr.Init( szFile );
		
		if( !g_pGadgetTargetMgr )
		{
			strcpy( aszStrings[(*pcStrings)++], "<Error - Gadget Target Mgr not Inited!>" );
			return LT_OK;
		}
	}

	if( !PopulateStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength ))
	{
		strcpy( aszStrings[(*pcStrings)++], "<Error - Populating String List!>" );
	}

	// Reset the filter out types
	m_dwFilterTypes = 0;

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGadgetTargetMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CGadgetTargetMgrPlugin::PopulateStringList( char** aszStrings, uint32* pcStrings,
												   const uint32 cMaxStrings, const uint32 cMaxStringLength )
{
    if( !aszStrings || !pcStrings || !g_pGadgetTargetMgr ) return LTFALSE;
	_ASSERT( aszStrings && pcStrings );

	// Add an entry for each GadgetTarget type

	int nNumGadgetTargets = g_pGadgetTargetMgr->GetNumGadgetTargets();
	_ASSERT(nNumGadgetTargets > 0);

    GADGETTARGET* pGadgetTarget = LTNULL;

	for( int i=0; i < nNumGadgetTargets; i++ ) 
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget( i );
		if( pGadgetTarget && pGadgetTarget->szName[0] )
		{
			// Do we want to filter out this type...

			if( GT_TYPE_TO_FLAG( pGadgetTarget->nType ) & m_dwFilterTypes ) continue;

            uint32 dwGTNameLen = strlen( pGadgetTarget->szName );

			if( dwGTNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings )
			{
				strcpy( aszStrings[(*pcStrings)++], pGadgetTarget->szName );
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGadgetTargetMgrPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this prop
//
// ----------------------------------------------------------------------- //

LTRESULT CGadgetTargetMgrPlugin::PreHook_Dims(
												const char* szRezPath,
												const char* szPropValue,
												char* szModelFilenameBuf,
												int	  nModelFilenameBufLen,
												LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	if( !g_pGadgetTargetMgr )
	{
		// This will set the g_pGadgetTargetMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf( szFile, "%s\\%s", szRezPath, GTMGR_DEFAULT_FILE );
        sm_GadgetTargetMgr.SetInRezFile( LTFALSE );
        sm_GadgetTargetMgr.Init( szFile );

		if( !g_pGadgetTargetMgr ) return LT_UNSUPPORTED;
	}

	GADGETTARGET* pGadgetTarget = g_pGadgetTargetMgr->GetGadgetTarget((char*)szPropValue);
	if( !pGadgetTarget || !pGadgetTarget->szFileName[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pGadgetTarget->szFileName );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}
