// ----------------------------------------------------------------------- //
//
// MODULE  : Intelligence.cpp
//
// PURPOSE : Implementation of the Intelligence object
//
// CREATED : 9/14/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Intelligence.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "CommandMgr.h"
#include "ServerSoundMgr.h"
#include "GameServerShell.h"

LINKFROM_MODULE( Intelligence );

// Statics
 
static char *s_szActivate = "ACTIVATE";
static char *s_szGadget   = "GADGET";
static char *s_szRespawn   = "RESPAWN";

CVarTrack g_IntelRespawnScale;

#define INTEL_RESPAWN_SOUND	"IntelRespawn"
#define INTEL_PICKUP_SOUND	"IntelPickup"

// ----------------------------------------------------------------------- //
//
//	CLASS:		Intelligence
//
//	PURPOSE:	An Intelligence object
//
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(Intelligence)

	// Hide parent properties we don't care about...

	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
    ADD_LONGINTPROP(TextId, 0)
	ADD_BOOLPROP_FLAG(IsIntel, LTFALSE, 0)
	ADD_BOOLPROP_FLAG(ShowPopup, LTTRUE, 0)
	ADD_BOOLPROP_FLAG(AddToList, LTTRUE, 0)
    ADD_LONGINTPROP(PopupId, -1)
	ADD_BOOLPROP_FLAG(PhotoOnly, LTFALSE, 0)
	ADD_STRINGPROP_FLAG(PickedUpCommand, "", PF_NOTIFYCHANGE)

	ADD_BOOLPROP_FLAG(StartHidden, LTFALSE, 0)
	ADD_REALPROP(RespawnTime, 30.0f)


END_CLASS_DEFAULT_FLAGS_PLUGIN(Intelligence, Prop, NULL, NULL, 0, CIntelPlugin)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( Intelligence )

	CMDMGR_ADD_MSG( GADGET, 1, NULL, "GADGET" )
	CMDMGR_ADD_MSG( RESPAWN, 1, NULL, "RESPAWN" )

CMDMGR_END_REGISTER_CLASS( Intelligence, Prop )


#ifndef __PSX2
LTRESULT CIntelPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if ( LT_OK == CPropPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}
	else if (_strcmpi("Type", szPropName) == 0)
	{
		if (m_IntelMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

LTRESULT CIntelPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if (m_IntelMgrPlugin.PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

LTRESULT CIntelPlugin::PreHook_PropChanged( const char *szObjName,
											const char *szPropName,
											const int nPropType,
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{
	// Send to base class first to see if it can handle it...

	if( CPropPlugin::PreHook_PropChanged( szObjName,
										  szPropName,
										  nPropType,
										  gpPropValue,
										  pInterface,
										  szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	if( !_stricmp( szPropName, "PickedUpCommand" ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Intelligence()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Intelligence::Intelligence() : Prop ()
{
    m_bIsIntel			= LTFALSE;
    m_bShowPopup		= LTTRUE;
    m_bAddToList		= LTTRUE;
    m_bPhotoOnly        = LTFALSE;
    m_hstrPickedUpCmd   = LTNULL;
	m_nTextId			= 0;
	m_nIntelId			= INTELMGR_INVALID_ID;
	m_nPopupId			= INTELMGR_INVALID_ID;
	m_fRespawnDelay		= 30.0f;
	m_bStartHidden		= LTFALSE;
	m_hstrPickupSnd		= LTNULL;
	m_hstrRespawnSnd	= LTNULL;
	m_bSkipUpdate		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::~Intelligence()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Intelligence::~Intelligence()
{
	if (m_hstrPickedUpCmd)
	{
        g_pLTServer->FreeString(m_hstrPickedUpCmd);
        m_hstrPickedUpCmd = LTNULL;
	}

	FREE_HSTRING( m_hstrPickupSnd );
	FREE_HSTRING( m_hstrRespawnSnd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Intelligence::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				uint32 ret = Prop::EngineMessageFn(messageID, pData, fData);
				InitialUpdate();
				return ret;
			}
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Intelligence::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("PhotoOnly", &genProp) == LT_OK)
	{
		m_bPhotoOnly = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("StartHidden", &genProp) == LT_OK)
	{
		m_bStartHidden = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("ShowPopup", &genProp) == LT_OK)
	{
		m_bShowPopup = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("IsIntel", &genProp) == LT_OK)
	{
		m_bIsIntel = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("AddToList", &genProp) == LT_OK)
	{
		m_bAddToList = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PickedUpCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickedUpCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("TextId", &genProp) == LT_OK)
	{
		m_nTextId = genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("PopupId", &genProp) == LT_OK)
	{
		m_nPopupId = (uint8) genProp.m_Long;
	}

	if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnDelay = genProp.m_Float;
	}


   INTEL* pIntel = LTNULL;

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			pIntel = g_pIntelMgr->GetIntel(genProp.m_String);
		}
	}

	if (pIntel)
	{
		if (!pIntel || !pIntel->szFilename[0]) return;

		SAFE_STRCPY(pInfo->m_Filename, pIntel->szFilename);

		uint32 iSkin = 0;
		ConParse conParse;
		conParse.Init(pIntel->szSkin);
		while (g_pCommonLT->Parse(&conParse) == LT_OK)
		{
			if (conParse.m_nArgs > 0)
			{
				SAFE_STRCPY(pInfo->m_SkinNames[iSkin], conParse.m_Args[0]);
				iSkin++;
			}

			if (iSkin >= MAX_MODEL_TEXTURES)
				break;
		}
		pInfo->m_SkinName[MAX_CS_FILENAME_LEN] = '\0';

		m_nIntelId = pIntel->nId;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Intelligence::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Make sure we don't have gravity set...
	pStruct->m_Flags &= ~FLAG_GRAVITY;

	m_dwUsrFlgs |= (USRFLG_GLOW | USRFLG_IGNORE_PROJECTILES);

	if (m_bPhotoOnly)
	{
		m_dwUsrFlgs |= (USRFLG_GADGET_INTELLIGENCE | USRFLG_GADGET_CAMERA);
	}
	else
	{
		m_dwUsrFlgs |= USRFLG_CAN_ACTIVATE;
	}

    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetHitPoints(1.0f);
    m_damage.SetCanDamage(LTFALSE);
	
	// Use the default info id if necessary...

	INTEL* pIntel = g_pIntelMgr->GetIntel(m_nIntelId);
	if( !pIntel )
		return;

	if (!m_nTextId)
	{
		if (pIntel && m_bShowPopup)
		{
			m_nTextId = pIntel->nDefaultTextId;
		}
	}
	if (m_nPopupId == INTELMGR_INVALID_ID)
	{
		if (pIntel && m_bShowPopup)
		{
			m_nPopupId = pIntel->nPopupId;
		}
	}

	if( pIntel->szPickupSnd[0] )
	{
		m_hstrPickupSnd = g_pLTServer->CreateString( pIntel->szPickupSnd );
	}
	else
	{
		m_hstrPickupSnd = g_pLTServer->CreateString( INTEL_PICKUP_SOUND );
	}

	if( pIntel->szRespawnSnd[0] )
	{
		m_hstrRespawnSnd = g_pLTServer->CreateString( pIntel->szRespawnSnd );
	}
	else
	{
		m_hstrRespawnSnd = g_pLTServer->CreateString( INTEL_RESPAWN_SOUND );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Intelligence::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate(s_szActivate);
	static CParsedMsg::CToken s_cTok_Gadget(s_szActivate);
	static CParsedMsg::CToken s_cTok_Respawn(s_szRespawn);

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		if (!m_bPhotoOnly)
		{
			DoActivate(hSender);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Gadget)
	{
		if (m_bPhotoOnly)
		{
			HandleGadgetMsg(hSender);
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Respawn)
	{
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::HandleGadgetMsg
//
//	PURPOSE:	Handle the camera gadget
//
// ----------------------------------------------------------------------- //

void Intelligence::HandleGadgetMsg(HOBJECT hSender)
{
	DoActivate(hSender);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void Intelligence::DoActivate(HOBJECT hSender)
{
	// BL 10/30/00 - fix multiple photographs of items in multiplayer
	if ( IsMultiplayerGame( ))
	{
		// If it's not visible, then don't activate.
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		if ( !(dwFlags & FLAG_VISIBLE) )
		{
			return;
		}
	}

	// See if the sender was a player.
    CPlayerObj* pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject(hSender));

	// Wasn't a player.  Use the first player.
	if ( !pPlayer && !IsMultiplayerGame( ))
	{
		// Find the player if the sender isn't one...
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		if( iter == CPlayerObj::GetPlayerObjList( ).end( ))
			return;

		pPlayer = *iter;
	}

	// No player found.
	if( !pPlayer )
		return;

	// Increment the player's intelligence count...
	INTEL* pIntel = g_pIntelMgr->GetIntel(m_nIntelId);

	HCLIENT hClient = pPlayer->GetClient();
	if (hClient)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_INTEL_PICKEDUP);
        cMsg.Writeuint32(m_nTextId);
        cMsg.Writeuint8(m_nPopupId);
        cMsg.Writeuint8(m_bIsIntel);
        cMsg.Writeuint8(m_bShowPopup);
        cMsg.Writeuint8(m_bAddToList);
		g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
	}

	if (m_bAddToList)
		pPlayer->GetPlayerSkills()->GainIntelBonus();



	// If we have a command, process it...

	if (m_hstrPickedUpCmd)
	{
        const char* pCmd = g_pLTServer->GetStringData(m_hstrPickedUpCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}

	if (!IsMultiplayerGame( ))
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
	else
	{
		// In MP, make item invisible so it can't be activated again.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_RAYHIT);

		SetNextUpdate( m_fRespawnDelay / g_IntelRespawnScale.GetFloat(1.0f));
	}

	// Play pickup sound...
	
	if( m_hstrPickupSnd )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData( m_hstrPickupSnd ),
			600.0f, SOUNDPRIORITY_MISC_HIGH);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Intelligence::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bPhotoOnly);
    SAVE_BOOL(m_bIsIntel);
    SAVE_BOOL(m_bShowPopup);
    SAVE_BOOL(m_bAddToList);
    SAVE_HSTRING(m_hstrPickedUpCmd);
    SAVE_DWORD(m_nTextId);
    SAVE_BYTE(m_nPopupId);
    SAVE_BYTE(m_nIntelId);
    SAVE_BOOL(m_bStartHidden);
    SAVE_BOOL(m_bSkipUpdate);
    SAVE_FLOAT(m_fRespawnDelay);
	SAVE_HSTRING(m_hstrRespawnSnd);
	SAVE_HSTRING(m_hstrPickupSnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Intelligence::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_BOOL(m_bPhotoOnly);
    LOAD_BOOL(m_bIsIntel);
    LOAD_BOOL(m_bShowPopup);
    LOAD_BOOL(m_bAddToList);
    LOAD_HSTRING(m_hstrPickedUpCmd);
    LOAD_DWORD(m_nTextId);
    LOAD_BYTE(m_nPopupId);
    LOAD_BYTE(m_nIntelId);
    LOAD_BOOL(m_bStartHidden);
    LOAD_BOOL(m_bSkipUpdate);
    LOAD_FLOAT(m_fRespawnDelay);

	LOAD_HSTRING(m_hstrRespawnSnd);
	LOAD_HSTRING(m_hstrPickupSnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL Intelligence::Update()
{

	if (IsMultiplayerGame())
	{
		// If we aren't visible it must be time to respawn...

		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		if (!m_bSkipUpdate && !(dwFlags & FLAG_VISIBLE))
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_RAYHIT, FLAG_VISIBLE | FLAG_RAYHIT);

			// Let the world know what happened...

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData( m_hstrRespawnSnd ),
					600.0f, SOUNDPRIORITY_MISC_HIGH);
		}
		m_bSkipUpdate = LTFALSE;
	}
	

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Intelligence::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void Intelligence::InitialUpdate()
{

	if (!g_IntelRespawnScale.IsInitted())
	{
        g_IntelRespawnScale.Init(GetServerDE(), "IntelRespawnScale", LTNULL, 1.0f);
	}

 	if (m_bStartHidden)
	{
		SendTriggerMsgToObject(this, m_hObject, LTFALSE, "Hidden 1");
	}

	m_bSkipUpdate = m_bMoveToFloor;

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PICKUPITEM_ID);
    cMsg.Writebool(0);
    cMsg.Writebool(0);
	cMsg.Writeuint8(INVALID_TEAM);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

