//------------------------------------------------------------------
//
//   MODULE    : SERVERSPECIALFX.CPP
//
//   PURPOSE   : Implements class CClientFX
//
//   CREATED   : On 8/23/00 At 6:45:30 PM
//
// (c) 2000 - 2002 LithTech Inc. and Monolith Productions, Inc.  
// All Rights Reserved
//
//------------------------------------------------------------------

//
// Includes...
//

#include "stdafx.h"
#include "ServerSpecialFX.h"
#include "FxFlags.h"
#include "FxDefs.h"
#include "iltmessage.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include <IO.h>
#include "CommandMgr.h"

//
// Defines...
//

#define SFX_MSG_TRIGGER	"TRIGGER"
#define SFX_MSG_ON		"ON"
#define	SFX_MSG_OFF		"OFF"


LINKFROM_MODULE( ServerSpecialFX );

BEGIN_CLASS( SpecialFX )
	ADD_STRINGPROP_FLAG( FxName, "<none>", PF_STATICLIST )
	ADD_BOOLPROP( Loop, LTTRUE )
	ADD_BOOLPROP( SmoothShutdown, LTTRUE ) 
	ADD_BOOLPROP( StartOn, LTTRUE )
	ADD_BOOLPROP( OneTime, LTFALSE )
	ADD_STRINGPROP_FLAG( TargetObject, "", PF_OBJECTLINK )
	ADD_BOOLPROP( RemoveTarget, LTFALSE )
	ADD_BOOLPROP_FLAG( SkyObject, LTFALSE, PF_HIDDEN )
END_CLASS_DEFAULT_FLAGS_PLUGIN( SpecialFX, GameBase, NULL, NULL, 0, SpecialFXPlugin )

CMDMGR_BEGIN_REGISTER_CLASS( SpecialFX )

	CMDMGR_ADD_MSG( ON, 1, NULL, SFX_MSG_ON )
	CMDMGR_ADD_MSG( OFF, 1, NULL, SFX_MSG_OFF )
	CMDMGR_ADD_MSG( TRIGGER, 1, NULL, SFX_MSG_TRIGGER )

CMDMGR_END_REGISTER_CLASS( SpecialFX, GameBase )

//------------------------------------------------------------------
//
//   FUNCTION : SpecialFX()
//
//   PURPOSE  : Standard constructor
//
//------------------------------------------------------------------

SpecialFX::SpecialFX() 
:	GameBase			( OT_NORMAL ),
	m_bStartOn			( true ),
	m_bIsOn				( false ),
	m_bOneTime			( false ),
	m_hstrTargetName	( LTNULL ),
	m_hTargetObj		( LTNULL ),
	m_bRemoveTarget		( false ),
	m_bFromSavedGame	( false )
{

}

//------------------------------------------------------------------
//
//   FUNCTION : ~SpecialFX()
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

SpecialFX::~SpecialFX()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : EngineMessageFn()
//
//   PURPOSE  : Handles engine messages
//
//------------------------------------------------------------------

uint32 SpecialFX::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_PRECREATE :
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct *pOcs = (ObjectCreateStruct *)pData;

			if ((uint32)fData == PRECREATE_WORLDFILE || (uint32)fData == PRECREATE_STRINGPROP)
			{
				// Read in object properties

				ReadProps(pOcs);
			}
			
			return dwRet;
		}
		break;
		
		case MID_INITIALUPDATE :
		{
			m_bFromSavedGame = ((uint32)fData == INITIALUPDATE_SAVEGAME);
			if ( !m_bFromSavedGame )
			{
				SetNextUpdate(UPDATE_NEVER);
			}
		}
		break;

		case MID_ALLOBJECTSCREATED :
		{
			if( !m_bFromSavedGame )
			{

				// See if we have a Target object...

				if( m_hstrTargetName )
				{
					ObjArray<HOBJECT, 1> objArray;
					g_pLTServer->FindNamedObjects( g_pLTServer->GetStringData( m_hstrTargetName ), objArray );

					if( objArray.NumObjects() > 0 )
					{
						m_hTargetObj = objArray.GetObject( 0 );
					}

					FREE_HSTRING( m_hstrTargetName );
				}

				if (m_bStartOn)
				{
					TurnON();
				}
			}
		}
		break;

		case MID_SAVEOBJECT :
		{			
			// Handle saving

			GameBase::EngineMessageFn(messageID, pData, fData);	
			OnSave((ILTMessage_Write*)pData, (uint32)fData);
			return LT_OK;
		}
		break;

		case MID_LOADOBJECT :
		{
			// Handle loading
			
			GameBase::EngineMessageFn(messageID, pData, fData);
			OnLoad((ILTMessage_Read*)pData, (uint32)fData);
			return LT_OK;
		}
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::OnTrigger
//
//  PURPOSE:	Deciphers a trigger msg
//
// ----------------------------------------------------------------------- //

bool SpecialFX::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Trigger(SFX_MSG_TRIGGER);
	static CParsedMsg::CToken s_cTok_On(SFX_MSG_ON);
	static CParsedMsg::CToken s_cTok_Off(SFX_MSG_OFF);

	if( cMsg.GetArg(0) == s_cTok_Trigger )
	{
		m_bIsOn ? TurnOFF() : TurnON() ;
	}
	else if( cMsg.GetArg(0) == s_cTok_On )
	{
		TurnON();
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		TurnOFF();
	}
	else
	{
		return GameBase::OnTrigger(hSender, cMsg);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::TurnON
//
//  PURPOSE:	Turn on the special fx
//
// ----------------------------------------------------------------------- //

void SpecialFX::TurnON()
{
	if (!m_bLoop)
	{
		// We can turn on (create) non-looping fx as often as we want...
		::PlayClientFX(m_sFxName, m_hObject, m_hTargetObj, LTNULL, LTNULL, LTNULL, m_dwFxFlags);

		if (m_bOneTime)
		{
			g_pLTServer->RemoveObject(m_hObject);
		}

		if( m_bRemoveTarget )
		{
			g_pLTServer->RemoveObject( m_hTargetObj );
		}
	}
	else if (!m_bIsOn)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SFX_CLIENTFXGROUP );
		cMsg.WriteString( m_sFxName );
		cMsg.Writeuint32( m_dwFxFlags );

		if( m_hTargetObj )
		{
			cMsg.Writeuint8( true );
			cMsg.WriteObject( m_hTargetObj );

			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hTargetObj, &vPos );

			cMsg.WriteCompPos( vPos );
		}
		else
		{
			cMsg.Writeuint8( false );
		}
	
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );

		// Set flags so the client knows we are on...
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_SFX_ON, USRFLG_SFX_ON );
	}

	m_bIsOn = true;
}	

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::TurnOFF
//
//  PURPOSE:	Turn off the special fx
//
// ----------------------------------------------------------------------- //

void SpecialFX::TurnOFF()
{
	if (!m_bIsOn) return;

	if (m_bLoop)
	{
		// Set flags so the client knows we are off...
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_FORCECLIENTUPDATE );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, USRFLG_SFX_ON );
	}

	m_bIsOn = false;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSave()
//
//   PURPOSE  : Handles saving
//
//------------------------------------------------------------------

void SpecialFX::OnSave(ILTMessage_Write *pMsg, uint32 dwFlags)
{
	SAVE_CHARSTRING(m_sFxName);
	SAVE_DWORD(m_dwFxFlags);
	SAVE_bool(m_bStartOn);
	SAVE_bool(m_bLoop);
	SAVE_bool(m_bIsOn);
	SAVE_bool(m_bOneTime);
	SAVE_HSTRING(m_hstrTargetName);
	SAVE_HOBJECT(m_hTargetObj);
	SAVE_bool(m_bRemoveTarget);
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLoad()
//
//   PURPOSE  : Handles loading
//
//------------------------------------------------------------------

void SpecialFX::OnLoad(ILTMessage_Read *pMsg, uint32 dwFlags)
{
	LOAD_CHARSTRING(m_sFxName, 128);
	LOAD_DWORD(m_dwFxFlags);
	LOAD_bool(m_bStartOn);
	LOAD_bool(m_bLoop);
	LOAD_bool(m_bIsOn);
	LOAD_bool(m_bOneTime);
	LOAD_HSTRING(m_hstrTargetName);
	LOAD_HOBJECT(m_hTargetObj);
	LOAD_bool(m_bRemoveTarget);

	if (!m_bLoop)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);
		SetNextUpdate(UPDATE_NEVER);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ReadProps()
//
//   PURPOSE  : Reads in object properties
//
//------------------------------------------------------------------

void SpecialFX::ReadProps(ObjectCreateStruct *pOcs)
{
	GenericProp GenProp;

	if(g_pLTServer->GetPropGeneric( "StartOn", &GenProp ) == LT_OK)
	{
		m_bStartOn = GenProp.m_Bool ? true : false;
	}

	if(g_pLTServer->GetPropGeneric( "OneTime", &GenProp ) == LT_OK)
	{
		m_bOneTime = GenProp.m_Bool ? true : false;
	}

	// Read name prop

	if(g_pLTServer->GetPropGeneric( "FxName", &GenProp ) == LT_OK)
	{
		LTStrCpy(m_sFxName, GenProp.m_String, sizeof(m_sFxName));
	}

	// Read loop prop
	if(g_pLTServer->GetPropGeneric( "Loop", &GenProp ) == LT_OK)
	{
		m_bLoop = GenProp.m_Bool ? true : false;
		m_dwFxFlags = m_bLoop ? FXFLAG_LOOP : 0;
	}
	else
	{
		m_bLoop = false;
		m_dwFxFlags = 0;
	}

	// Read smooth shutdown prop
	
	if(g_pLTServer->GetPropGeneric( "SmoothShutdown", &GenProp ) == LT_OK)
	{
		m_dwFxFlags |= GenProp.m_Bool ? 0 : FXFLAG_NOSMOOTHSHUTDOWN;
	}

	if(g_pLTServer->GetPropGeneric( "SkyObject", &GenProp ) == LT_OK)
	{
		m_dwFxFlags |= GenProp.m_Bool ? FXFLAG_SKYOBJECT : 0;
	}

	if( g_pLTServer->GetPropGeneric( "TargetObject", &GenProp ) == LT_OK )
	{
		if( GenProp.m_String[0] )
		{
			m_hstrTargetName = g_pLTServer->CreateString( GenProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "RemoveTarget", &GenProp ) == LT_OK )
	{
		m_bRemoveTarget = GenProp.m_Bool;
	}
}


//-----------------
// Plugin Implementation
//-----------------


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFXPlugin::PreHook_EditStringList
//
//  PURPOSE:	Fill the FxName Prop in DEdit with valid fxGroup Names created with FxED
//
// ----------------------------------------------------------------------- //

LTRESULT SpecialFXPlugin::PreHook_EditStringList( const char *szRezPath, const char *szPropName, char **aszStrings,
													 uint32 *pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Make sure this is the prop we want
	if( !stricmp( "FxName", szPropName ) )
	{
		return PopulateStringList( szRezPath, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFXPlugin::PopulateStringList
//
//  PURPOSE:	Fill the string list with the names of the FX...
//
// ----------------------------------------------------------------------- //

LTRESULT SpecialFXPlugin::PopulateStringList( const char *szRezPath,
											  char **aszStrings,
											  uint32 *pcStrings, 
											  const uint32 cMaxStrings, 
											  const uint32 cMaxStringLength )
{
	// always make the first option an invalid FxName
	strcpy(aszStrings[(*pcStrings)++], "<none>" );

	char	szFile[_MAX_FNAME];
	
	_finddata_t fcfFile;

	// Build the search spec path
	sprintf( szFile, "%s\\ClientFX\\*.fcf", szRezPath );

	long handle = _findfirst( szFile, &fcfFile );
	if( handle == -1 )
	{
		strcpy( aszStrings[(*pcStrings)++], "<error> - Couldn't locate a .fcf!" );
	}

	// Build the full filename
	sprintf( szFile, "%s\\ClientFX\\%s", szRezPath, fcfFile.name );

	// Read the first file and fill in the list
	if( !ParseFCF( szFile, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) )
		return LT_UNSUPPORTED;

	while( !_findnext( handle, &fcfFile ) )
	{
		// Build the full filename
		sprintf( szFile, "%s\\ClientFX\\%s", szRezPath, fcfFile.name );

		// Read the file and fill in the list
		if( !ParseFCF( szFile, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) )
			return LT_UNSUPPORTED;
	}
	
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFXPlugin::ParseFCF
//
//  PURPOSE:	Parse a .fcf file and populate the string list
//
// ----------------------------------------------------------------------- //

LTBOOL SpecialFXPlugin::ParseFCF( const char *szFcfPath, char **aszStrings, uint32 *pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength )
{
	// Attempt to open the file
	FILE	*pFxFile = fopen( szFcfPath, "rb" );
	if( !pFxFile ) return FALSE;

	char szBinTest[64] = {0};
	
	fread( szBinTest, 9, 1, pFxFile );
	
	// We got what we need now close it... 

	fclose( pFxFile );

	// Are we looking at a text or a binary file...

	if( !_stricmp( szBinTest, "Siblings:" ) )
	{
		pFxFile = fopen( szFcfPath, "rt" );
		if (!pFxFile) return LTFALSE;

		char szTag[64] = {0};
		char sTmp[256] = {0};

		// Read in Number of Tree Heads
		
		int nNumHeads;
		fscanf( pFxFile, "%s %i", szTag, &nNumHeads );
		
		if( nNumHeads == 0 )
		{
			fclose( pFxFile );
			return LTFALSE;
		}

		// Read the name...

		fscanf( pFxFile, "%s", szTag );
		fseek( pFxFile, 1, SEEK_CUR );
		fgets(sTmp, ARRAY_LEN(sTmp), pFxFile );
		int iStrSize = strlen( sTmp );
		if( (iStrSize > 0) && (sTmp[iStrSize - 1] == '\n') )
			sTmp[iStrSize - 1] = NULL;

		// Seek past the image data
		int	iDummy;
		fscanf( pFxFile, "%s %i", szTag, &iDummy );
		fscanf( pFxFile, "%s %i", szTag, &iDummy );
		
		// Check to see if there are any children

		BOOL bChildren;
		fscanf( pFxFile, "%s %i", szTag, &bChildren );
		 

		// Seek past the state
		uint32 dwDummy;
		fscanf( pFxFile, "%s %lu", szTag, &dwDummy );
		
		if( bChildren )
		{
			// We have some Group FX in this file...
			
			// Read in how many
		
			int nNumGroups;
			fscanf( pFxFile, "%s %lu", szTag, &nNumGroups );
			
			if( nNumGroups == 0 )
			{
				fclose( pFxFile );
				return LTFALSE;
			}

			char	szName[_MAX_FNAME];
			
			for( int i = 0; i < nNumGroups; i++ )
			{
				// Read in the GroupFX Name
				
				fscanf( pFxFile, "%s", szTag );
				fseek( pFxFile, 1, SEEK_CUR );
				fgets(szName, ARRAY_LEN(szName), pFxFile );
				int iStrSize = strlen( szName );
				if( (iStrSize > 0) && (szName[iStrSize - 1] == '\n') )
					szName[iStrSize - 1] = NULL;
				
				if( (*pcStrings) == cMaxStrings )
				{
					fclose( pFxFile );
					return LTFALSE;
				}

				// Set this as a valid FxName selection
				strcpy( aszStrings[(*pcStrings)++], szName );

				// Skip everything else
				fscanf( pFxFile, "%s %i", szTag, &iDummy );
				fscanf( pFxFile, "%s %i", szTag, &iDummy );
				fscanf( pFxFile, "%s %i", szTag, &bChildren );
				fscanf( pFxFile, "%s %lu", szTag, &dwDummy );
			}
		}

		fclose( pFxFile );
	}
	else
	{
		pFxFile = fopen( szFcfPath, "rb" );
		if( !pFxFile ) return LTFALSE;
		
		// Read in Number of Tree Heads
		
		int nNumHeads;
		fread( &nNumHeads, sizeof( int ), 1, pFxFile );

		if( nNumHeads == 0 )
		{
			fclose( pFxFile );
			return LTFALSE;
		}

		// Read in the Tree Head name length...
		
		int		nLen;
		fread( &nLen, sizeof( int ), 1, pFxFile );

		// Seek past the name
		fseek( pFxFile, sizeof( char) * nLen, SEEK_CUR );

		// Seek past the image data
		fseek( pFxFile, sizeof( int ) * 2, SEEK_CUR );

		// Check to see if there are any children

		BOOL bChildren;
		fread( &bChildren, sizeof( BOOL ), 1, pFxFile ); 

		// Seek past the state
		fseek( pFxFile, sizeof( UINT ), SEEK_CUR );

		if( bChildren )
		{
			// We have some Group FX in this file...
			
			// Read in how many
		
			int nNumGroups;
			fread( &nNumGroups, sizeof( int ), 1, pFxFile );

			if( nNumGroups == 0 )
			{
				fclose( pFxFile );
				return LTFALSE;
			}

			uint	nLen;
			char	szName[_MAX_FNAME];
			
			for( int i = 0; i < nNumGroups; i++ )
			{
				// Read in the GroupFX Name
		
				fread( &nLen, sizeof( uint ), 1, pFxFile );

				if( nLen > cMaxStringLength )
				{
					LTStrCpy( szName, "<error> - FxName too long!", sizeof(szName) );
					fseek( pFxFile, sizeof( char) * nLen, SEEK_CUR );
				}
				else
					fread( &szName, sizeof( char ), nLen, pFxFile );

				if( (*pcStrings) == cMaxStrings )
				{
					fclose( pFxFile );
					return LTFALSE;
				}

				// Set this as a valid FxName selection
				strcpy( aszStrings[(*pcStrings)++], szName );

				// Skip everything else
				fseek( pFxFile, (sizeof( int ) * 2) + (sizeof( BOOL )) + (sizeof( UINT )), SEEK_CUR ); 
			}
		}

		fclose( pFxFile );
	}

	return LTTRUE;
}

//-----------------
// Global functions
//-----------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SetObjectClientFXMsg
//
//  PURPOSE:	Sets up the special effect message for a particular
//				object
//
// ----------------------------------------------------------------------- //

void SetObjectClientFXMsg( HOBJECT hObj, char *sName, uint32 dwFlags )
{
// [KLS 5/19/02] This really should be depricated, the only place that is 
// currently using it is the KeyItem and that object really shouldn't be
// turning itself into a half-prop/half-special fx mutant.

	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_CLIENTFXGROUP );
	cMsg.WriteString( sName );
	cMsg.Writeuint32( dwFlags );

	// Do not use any target information...
	cMsg.Writeuint8( false );
	
	g_pLTServer->SetObjectSFXMessage( hObj, cMsg.Read() );
}