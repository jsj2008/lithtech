//------------------------------------------------------------------
//
//   MODULE    : SERVERSPECIALFX.CPP
//
//   PURPOSE   : Implements class CClientFX
//
//   CREATED   : On 8/23/00 At 6:45:30 PM
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------

//
// Includes...
//

#include "Stdafx.h"
#include "ServerSpecialFX.h"
#include "FxFlags.h"
#include "FxDefs.h"
#include "iltmessage.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "resourceextensions.h"
#include "ltfileoperations.h"
#include "ltfileread.h"

LINKFROM_MODULE( ServerSpecialFX );

//Note: Since this object can also be created at runtime, any updates to these properties must also
//be updated in SpecialFX::HandleEffectMsg
BEGIN_CLASS( SpecialFX )
	ADD_STRINGPROP_FLAG( FxName, FX_NONE, PF_STATICLIST, "A dropdown list of names of FX created in FXEdit." )
	ADD_BOOLPROP( Loop, true, "If true the FX will continue to loop.  If false the FX will only play once." )
	ADD_BOOLPROP( SmoothShutdown, true, "If this is set to true the FX will smoothly shut down when the object is turned off or the FX ends.  (i.e. ParticleSystems will continue to update visible particles untill they expire when it is turned off." ) 
	ADD_BOOLPROP( StartOn, true, "If true the FX will start playing immediately." )
	ADD_BOOLPROP( OneTime, false, "If this is true and the FX is non-looping the object will be removed as soon as the FX is played." )
	ADD_STRINGPROP_FLAG( TargetObject, "", PF_OBJECTLINK, "The name of an object to use as a target.  Some FX can use a target for position information such as lightning which can use a model targets sockets and nodes to atach its lightning to." )
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH(SpecialFX, GameBase, 0, SpecialFXPlugin, DefaultPrefetch<SpecialFX>, "This object will play an FXEdit created FX at the position of the object." )

CMDMGR_BEGIN_REGISTER_CLASS( SpecialFX )

	ADD_MESSAGE( ON, 1, NULL, MSG_HANDLER( SpecialFX, HandleOnMsg ), "ON", "Tells the SpecialFX object to play its effect", "msg SpecialFX ON" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( SpecialFX, HandleOffMsg ), "OFF", "Tells the SpecialFX object to stop playing its looping effect", "msg SpecialFX OFF" )
	ADD_MESSAGE( TOGGLE, 1, NULL, MSG_HANDLER( SpecialFX, HandleToggleMsg ), "TOGGLE", "Tells the SpecialFX object to toggle its on/off state.", "msg SpecialFX TOGGLE" )
	ADD_MESSAGE( EFFECT, 2, NULL, MSG_HANDLER( SpecialFX, HandleEffectMsg ), "EFFECT", "Tells the SpecialFX object to begin playing the specified effect instead of the current effect", "msg SpecialFX EFFECT <effect name>" )

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
	m_sTargetName		( ),
	m_hTargetObj		( NULL ),
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

			if ((uint32)fData != PRECREATE_SAVEGAME)
			{
				// Read in object properties

				ReadProps(&pOcs->m_cProperties);
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

				if( !m_sTargetName.empty() )
				{
					ObjArray<HOBJECT, 1> objArray;
					g_pLTServer->FindNamedObjects( m_sTargetName.c_str(), objArray );

					if( objArray.NumObjects() > 0 )
					{
						m_hTargetObj = objArray.GetObject( 0 );
					}
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

		case MID_SAVESPECIALEFFECTMESSAGE :
		{
			OnSaveSFXMessage( static_cast<ILTMessage_Write*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_LOADSPECIALEFFECTMESSAGE :
		{
			OnLoadSFXMessage( static_cast<ILTMessage_Read*>( pData ), static_cast<uint32>( fData ) );
		}
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void SpecialFX::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	TurnON();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void SpecialFX::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	TurnOFF();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::HandleToggleMsg
//
//  PURPOSE:	Handle a TOGGLE message...
//
// ----------------------------------------------------------------------- //

void SpecialFX::HandleToggleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bIsOn ? TurnOFF() : TurnON() ;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFX::HandleEffectMsg
//
//  PURPOSE:	Handle a EFFECT message which changes the currently playing effect
//
// ----------------------------------------------------------------------- //

void SpecialFX::HandleEffectMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	//verify the argument count
	if(crParsedMsg.GetArgCount() < 2)
		return;

	//since the client uses this object's existence as a key to playing the effect, we need
	//to destroy this object and create a new one that will play this effect

	ObjectCreateStruct ocs;

	//get our information from this object
	ocs.m_hClass = g_pLTServer->GetObjectClass(m_hObject);

	//get the name of this object
	g_pLTServer->GetObjectName(m_hObject, ocs.m_Name, LTARRAYSIZE(ocs.m_Name));

	//and its location in the world
	LTTransform tTransform;
	g_pLTServer->GetObjectTransform(m_hObject, &tTransform);
	ocs.m_Pos = tTransform.m_vPos;
	ocs.m_Rotation = tTransform.m_rRot;
	ocs.m_Scale = tTransform.m_fScale;

	//and the object flags
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, ocs.m_Flags);
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, ocs.m_Flags2);

	//now we need to setup our properties
	ocs.m_cProperties.AddProp("FxName", GenericProp(crParsedMsg.GetArg(1), LT_PT_STRING));
	ocs.m_cProperties.AddProp("Loop", GenericProp(!!(m_dwFxFlags & FXFLAG_LOOP), LT_PT_BOOL));
	ocs.m_cProperties.AddProp("SmoothShutdown", GenericProp(!(m_dwFxFlags & FXFLAG_NOSMOOTHSHUTDOWN), LT_PT_BOOL));
	ocs.m_cProperties.AddProp("StartOn", GenericProp(m_bIsOn, LT_PT_BOOL));
	ocs.m_cProperties.AddProp("OneTime", GenericProp(m_bOneTime, LT_PT_BOOL));
	ocs.m_cProperties.AddProp("TargetObject", GenericProp(m_sTargetName.c_str(), LT_PT_STRING));

	//we can now create this new object
	g_pLTServer->CreateObject(ocs.m_hClass, &ocs);

	//and now destroy our object
	g_pLTServer->RemoveObject(m_hObject);
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
	if(m_bIsOn)
		return;

	if (!m_bLoop)
	{
		// We can turn on (create) non-looping fx as often as we want...
		LTRigidTransform tTransform;
		g_pLTServer->GetObjectTransform(m_hObject, &tTransform);
		
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
		cMsg.WriteString(m_sFxName.c_str());
		cMsg.Writebool( !!( m_dwFxFlags & FXFLAG_LOOP ));
		cMsg.Writebool( !!( m_dwFxFlags & FXFLAG_NOSMOOTHSHUTDOWN ));

		// No special parent.
		cMsg.Writebool( false );
		cMsg.WriteLTVector(tTransform.m_vPos);
		cMsg.WriteCompLTRotation(tTransform.m_rRot);


		// Write the target information if we have any...
		if( m_hTargetObj )
		{
			cMsg.Writebool( true );
			cMsg.WriteObject( m_hTargetObj );
		}
		else
		{
			cMsg.Writebool( false );
		}

		g_pLTServer->SendSFXMessage(cMsg.Read(), 0);

		if (m_bOneTime)
		{
			g_pLTServer->RemoveObject(m_hObject);
		}
	}
	else
	{
		CLIENTFXGROUPCREATESTRUCT ClientFXGroupCS;
		ClientFXGroupCS.m_nMsgID = SFX_CLIENTFXGROUP;
		ClientFXGroupCS.m_sFXName = m_sFxName;
		ClientFXGroupCS.m_dwFxFlags = m_dwFxFlags;
		ClientFXGroupCS.m_hTargetObj = m_hTargetObj;
		
		CAutoMessage cMsg;
		ClientFXGroupCS.Write( cMsg );
		g_pLTServer->SetObjectSFXMessage( m_hObject, cMsg.Read() );
	}

	// Set flags so the client knows we are on...
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_SFX_ON, USRFLG_SFX_ON );

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
	if( !m_bIsOn )
		return;

	// Set flags so the client knows we are off...
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_FORCECLIENTUPDATE );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, USRFLG_SFX_ON );
	
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
	SAVE_STDSTRING(m_sFxName);
	SAVE_DWORD(m_dwFxFlags);
	SAVE_bool(m_bStartOn);
	SAVE_bool(m_bLoop);
	SAVE_bool(m_bIsOn);
	SAVE_bool(m_bOneTime);
	SAVE_STDSTRING(m_sTargetName);
	SAVE_HOBJECT(m_hTargetObj);
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
	LOAD_STDSTRING(m_sFxName);
	LOAD_DWORD(m_dwFxFlags);
	LOAD_bool(m_bStartOn);
	LOAD_bool(m_bLoop);
	LOAD_bool(m_bIsOn);
	LOAD_bool(m_bOneTime);
	LOAD_STDSTRING(m_sTargetName);
	LOAD_HOBJECT(m_hTargetObj);

	if (!m_bLoop)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAGMASK_ALL);
		SetNextUpdate(UPDATE_NEVER);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSaveSFXMessage()
//
//   PURPOSE  : Handles saving the object special effect message.
//
//------------------------------------------------------------------

void SpecialFX::OnSaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cGetMsg;
	g_pLTServer->GetObjectSFXMessage( m_hObject, cGetMsg );
	CLTMsgRef_Read pSFXMsg = cGetMsg.Read( );

	if( pSFXMsg->Size( ) == 0 )
		return;
	
	CLIENTFXGROUPCREATESTRUCT ClientFXGroupCS;
	ClientFXGroupCS.Read( pSFXMsg );
	ClientFXGroupCS.Write( pMsg );
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLoadSFXMessage()
//
//   PURPOSE  : Handles loading the object special effect message.
//
//------------------------------------------------------------------

void SpecialFX::OnLoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags )
{
	if( !pMsg || pMsg->Size( ) == 0 )
		return;

	CAutoMessage cSFXMsg;
	
	CLIENTFXGROUPCREATESTRUCT ClientFXGroupCS;
	ClientFXGroupCS.Read( pMsg );
	ClientFXGroupCS.Write( cSFXMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cSFXMsg.Read( ) );
}

//------------------------------------------------------------------
//
//   FUNCTION : ReadProps()
//
//   PURPOSE  : Reads in object properties
//
//------------------------------------------------------------------

void SpecialFX::ReadProps(const GenericPropList *pProps)
{
	m_bStartOn		= pProps->GetBool( "StartOn", m_bStartOn );
	m_bOneTime		= pProps->GetBool( "OneTime", m_bOneTime );
	m_sFxName		= pProps->GetString( "FxName", "" );

	// Get the loop property and set the FX flag...
	m_bLoop			= pProps->GetBool( "Loop", m_bLoop );
	m_dwFxFlags		= m_bLoop ? FXFLAG_LOOP : 0;

	m_dwFxFlags		|= (pProps->GetBool( "SmoothShutdown", true ) ? 0 : FXFLAG_NOSMOOTHSHUTDOWN);
	m_sTargetName	= pProps->GetString( "TargetObject", "" );
}


//-----------------
// Plugin Implementation
//-----------------


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFXPlugin::PreHook_EditStringList
//
//  PURPOSE:	Fill the FxName Prop in WorldEdit with valid fxGroup Names created with FXEdit
//
// ----------------------------------------------------------------------- //

LTRESULT SpecialFXPlugin::PreHook_EditStringList( const char *szRezPath, const char *szPropName, char **aszStrings,
													 uint32 *pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Make sure this is the prop we want
	if( !LTStrICmp( "FxName", szPropName ) )
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
	LTStrCpy(aszStrings[(*pcStrings)++], FX_NONE, cMaxStringLength );

	char	szFile[_MAX_FNAME];
	
	LTFINDFILEINFO fcfFile;

	// Build the search spec path
	LTSNPrintF( szFile, LTARRAYSIZE(szFile), "%s\\ClientFX\\*."RESEXT_EFFECT_PACKED, szRezPath );

	LTFINDFILEHANDLE handle;
	if(!LTFileOperations::FindFirst( szFile, handle, &fcfFile ))
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "<error> - Couldn't locate an effect file!", cMaxStringLength );
		return LT_OK;
	}

	do
	{
		// Build the full filename
		LTSNPrintF( szFile, LTARRAYSIZE(szFile), "%s\\ClientFX\\%s", szRezPath, fcfFile.name );

		// Read the file and fill in the list
		ParseEffectFile( szFile, aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
	}
	while(LTFileOperations::FindNext( handle, &fcfFile ));
	
	LTFileOperations::FindClose(handle);

	// Alphabetize the strings, skipping the FX_NONE entry which is always first.
	if (*pcStrings > 1)
	{
		qsort( aszStrings+1, (*pcStrings)-1, sizeof( char * ), CaseInsensitiveCompare );		
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SpecialFXPlugin::ParseFCF
//
//  PURPOSE:	Parse an effect file and populate the string list
//
// ----------------------------------------------------------------------- //

//The format of the header as it resides on disk
struct SEffectHeader
{
	uint32	m_nFourCC;
	uint32	m_nFileVersion;
	uint32	m_nEffectVersion;
	uint32	m_nStringTableSize;
	uint32	m_nFunctionCurveDataSize;
	uint32	m_nTotalKeys;
	uint32	m_nTotalEffects;
	uint32	m_nNumEffectTypes;
	uint32	m_nStringTableOffset;
	uint32	m_nEffectListOffset;
};

//the version of the file that we support for parsing
#define FILE_VERSION	2
				
bool SpecialFXPlugin::ParseEffectFile(const char *pszFilename, char **aszStrings, uint32 *pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength )
{
	CLTFileRead cFileRead;
	if (!cFileRead.Open(pszFilename))
	{
		return false;
	}

	//now read in the header from the file
	SEffectHeader Header;
	if (!cFileRead.Read(&Header, sizeof(Header)))
	{
		return false;
	}
		
	//perform a version check
	if(	(Header.m_nFourCC != LTMakeFourCC('L', 'T', 'F', 'X')) ||
		(Header.m_nFileVersion != FILE_VERSION))
	{
		return false;
	}

	//now allocate and load the whole table
	char* pszStringTable = debug_newa(char, Header.m_nStringTableSize);
	if(!pszStringTable)
	{
		return false;
	}

	//read in the string table
	if (!cFileRead.Seek(Header.m_nStringTableOffset))
	{
		return false;
	}

	if (!cFileRead.Read(pszStringTable, Header.m_nStringTableSize))
	{
		return false;
	}
	
	//now read in the effect list
	if (!cFileRead.Seek(Header.m_nEffectListOffset))
	{
		return false;
	}

	for(uint32 nCurrEffect = 0; nCurrEffect < Header.m_nTotalEffects; nCurrEffect++)
	{
		//read in the index into the string table
		uint32 nIndex = 0;
		if (!cFileRead.Read(&nIndex, sizeof(nIndex)))
		{
			return false;
		}

		//make sure we have room
		if(*pcStrings >= cMaxStrings)
			break;

		//and add this string to our list of strings
		LTStrCpy( aszStrings[(*pcStrings)++], &pszStringTable[nIndex], cMaxStringLength );
	}

	//free up everything
	delete [] pszStringTable;
	cFileRead.Close();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialFX::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void SpecialFX::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// Get the weapon and ammo database records...
	char szFxName[256] = {'\0'};
	pInterface->GetPropString(pszObjectName, "FxName", szFxName, LTARRAYSIZE(szFxName), "");
	if (!LTStrEmpty(szFxName))
	{
		GetClientFXResources(Resources, szFxName);
	}
}
