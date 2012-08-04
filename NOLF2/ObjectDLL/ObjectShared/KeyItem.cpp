// ----------------------------------------------------------------------- //
//
// MODULE  : KeyItem.cpp
//
// PURPOSE : Implementation of the KeyItem object
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "KeyItem.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "PlayerObj.h"
#include "CommandMgr.h"
#include "ServerSoundMgr.h"
#include "GameServerShell.h"
#include "ServerSpecialFX.h"
#include "FxFlags.h"

LINKFROM_MODULE( KeyItem );

// Statics

static char *s_szActivate = "ACTIVATE";

#define KEY_CONTROL_OBJECT		"ControlObject"
#define KEY_DEFAULT_SND_RADIUS	600.0f

// ----------------------------------------------------------------------- //
//
//	CLASS:		KeyItem
//
//	PURPOSE:	An KeyItem object
//
// ----------------------------------------------------------------------- //


#define ADD_KEYITEM_CONTROL_OBJECTS( base_name, flags ) \
		PROP_DEFINEGROUP( ##base_name##s, flags ) \
			ADD_STRINGPROP_FLAG( ##base_name##0, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##1, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##2, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##3, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##4, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##5, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##6, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##7, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##8, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##9, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##10, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##11, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##12, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##13, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##14, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##15, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##16, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##17, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##18, "", flags | PF_OBJECTLINK ) \
			ADD_STRINGPROP_FLAG( ##base_name##19, "", flags | PF_OBJECTLINK )

#pragma force_active on
BEGIN_CLASS(KeyItem)

	// Hide parent properties we don't care about...

	ADD_STRINGPROP_FLAG(Filename, "", PF_HIDDEN | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, "", PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Type, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS)
	ADD_STRINGPROP_FLAG(PickedUpCommand, "", PF_NOTIFYCHANGE)

	ADD_BOOLPROP_FLAG(StartHidden, LTFALSE, 0)
	
	ADD_KEYITEM_CONTROL_OBJECTS( ControlObject, PF_GROUP(3) )

END_CLASS_DEFAULT_FLAGS_PLUGIN(KeyItem, Prop, NULL, NULL, 0, CKeyItemPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( KeyItem )

	CMDMGR_ADD_MSG( ACTIVATE,	1,	NULL,	"ACTIVATE" )	

CMDMGR_END_REGISTER_CLASS( KeyItem, Prop )

#ifndef __PSX2
LTRESULT CKeyItemPlugin::PreHook_EditStringList(
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
		if (m_KeyMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

LTRESULT CKeyItemPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{
	if (m_KeyMgrPlugin.PreHook_Dims(szRezPath, szPropValue,
		szModelFilenameBuf, nModelFilenameBufLen, vDims) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;

}

LTRESULT CKeyItemPlugin::PreHook_PropChanged( const char *szObjName,
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
//	ROUTINE:	KeyItem::KeyItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyItem::KeyItem() : Prop ()
{
    m_hstrPickedUpCmd   = LTNULL;
	m_bStartHidden		= LTFALSE;
	m_bFirstUpdate		= LTTRUE;
	m_bSkipUpdate		= LTFALSE;
	m_nKeyId			= -1;

	for( uint32 i =0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		m_hstrCtrlObjName[i] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::~KeyItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyItem::~KeyItem()
{
	if (m_hstrPickedUpCmd)
	{
        g_pLTServer->FreeString(m_hstrPickedUpCmd);
        m_hstrPickedUpCmd = LTNULL;
	}

	for( uint32 i =0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		FREE_HSTRING( m_hstrCtrlObjName[i] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 KeyItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
				InitialUpdate();
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void KeyItem::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartHidden", &genProp) == LT_OK)
	{
		m_bStartHidden = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("PickedUpCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickedUpCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}



   KEY* pKey = LTNULL;

    if (g_pLTServer->GetPropGeneric("Type", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			pKey = g_pKeyMgr->GetKey(genProp.m_String);
		}
	}

	if (pKey)
	{
		if (!pKey || !pKey->szFilename[0]) return;

		SAFE_STRCPY(pInfo->m_Filename, pKey->szFilename);
		pKey->blrSkins.CopyList(0, pInfo->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		pKey->blrRenderStyles.CopyList(0, pInfo->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

		m_nKeyId = pKey->nId;
	}

	// Save the name of any objects we are going to control...

	char szPropName[32] = {0};
	for( uint32 i = 0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		sprintf( szPropName, "%s%i", KEY_CONTROL_OBJECT, i );
		if( g_pLTServer->GetPropGeneric( szPropName, &genProp ) == LT_OK )
		{
			if( genProp.m_String[0] )
			{
				m_hstrCtrlObjName[i] = g_pLTServer->CreateString( genProp.m_String );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void KeyItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Make us activateable...

	pStruct->m_Flags |= FLAG_RAYHIT;

	// Make sure we don't have gravity set...

	pStruct->m_Flags &= ~FLAG_GRAVITY;

	m_dwUsrFlgs |= (USRFLG_GLOW | USRFLG_IGNORE_PROJECTILES);

	m_dwUsrFlgs |= USRFLG_CAN_ACTIVATE;

    m_damage.SetCanHeal(LTFALSE);
    m_damage.SetCanRepair(LTFALSE);
    m_damage.SetApplyDamagePhysics(LTFALSE);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetHitPoints(1.0f);
    m_damage.SetCanDamage(LTFALSE);
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool KeyItem::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Activate("ACTIVATE");

	if (cMsg.GetArg(0) == s_cTok_Activate)
	{
		DoActivate(hSender);
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::DoActivate()
//
//	PURPOSE:	Handle Activate...
//
// ----------------------------------------------------------------------- //

void KeyItem::DoActivate(HOBJECT hSender)
{

	HOBJECT hPlayer = hSender;

	if (!hSender || !IsPlayer(hSender))
	{
		// Find the player if the sender isn't one...

		ObjArray <HOBJECT, 1> objArray;
        g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);

		if (!objArray.NumObjects()) return;

		hPlayer = objArray.GetObject(0);
	}

	// Let the player know...
    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hPlayer);
	if (pPlayer)
	{
		pPlayer->UpdateKeys(ITEM_ADD_ID, m_nKeyId);

		HCLIENT hClient = pPlayer->GetClient();
		if (hClient)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_KEY_ID);
			cMsg.Writeuint8(ITEM_ADD_ID);
			cMsg.Writeuint8(0);
			cMsg.Writefloat((LTFLOAT)m_nKeyId);
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED );
		}


		// Play a sound of picking up the key item..

		KEY *pKey = g_pKeyMgr->GetKey( m_nKeyId );
		if( pKey && pKey->szSoundName[0])
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pKey->szSoundName, 
				KEY_DEFAULT_SND_RADIUS, SOUNDPRIORITY_MISC_HIGH);
		}


		// If we have a command, process it...

		if (m_hstrPickedUpCmd)
		{
            const char* pCmd = g_pLTServer->GetStringData(m_hstrPickedUpCmd);

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd, m_hObject, hSender);
			}
		}

		// And we're done.
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyItem::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    SAVE_HSTRING(m_hstrPickedUpCmd);
    SAVE_WORD(m_nKeyId);
    SAVE_BOOL(m_bStartHidden);
    SAVE_BOOL(m_bFirstUpdate);
    SAVE_BOOL(m_bSkipUpdate);

	for( uint32 i =0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		SAVE_HSTRING( m_hstrCtrlObjName[i] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyItem::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_HSTRING(m_hstrPickedUpCmd);
    LOAD_WORD(m_nKeyId);
	LOAD_BOOL(m_bStartHidden);
    LOAD_BOOL(m_bFirstUpdate);
    LOAD_BOOL(m_bSkipUpdate);

	for( uint32 i =0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		LOAD_HSTRING( m_hstrCtrlObjName[i] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL KeyItem::Update()
{
	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;
		if (m_bStartHidden)
		{
			SendTriggerMsgToObject(this, m_hObject, LTFALSE, "Hidden 1");
			return LTTRUE;
		}
	}

	// If we aren't visible it must be time to respawn...

    uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if (!m_bSkipUpdate && !(dwFlags & FLAG_VISIBLE))
	{
		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		dwFlags |= (FLAG_VISIBLE | FLAG_RAYHIT);
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	}
	m_bSkipUpdate = LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyItem::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void KeyItem::InitialUpdate()
{

 	if (m_bStartHidden)
	{
		SetNextUpdate(UPDATE_NEXT_FRAME);
	}

	m_bSkipUpdate = m_bMoveToFloor;
	
	KEY *pKey = g_pKeyMgr->GetKey( m_nKeyId );
	if( pKey )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_SFX_ON, USRFLG_SFX_ON );
		SetObjectClientFXMsg( m_hObject, pKey->szFXName, FXFLAG_LOOP );
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PICKUPITEM_ID);
    cMsg.Writebool(0);
    cMsg.Writebool(0);
	cMsg.Writeuint8(INVALID_TEAM);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KeyItem::AllObjectsCreated
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void KeyItem::AllObjectsCreated()
{
	for( uint32 i = 0; i < KEY_MAX_CONTROLOBJS; ++i )
	{
		if( m_hstrCtrlObjName[i] )
		{
			ObjArray<HOBJECT, 1> objArray;
			g_pLTServer->FindNamedObjects( g_pLTServer->GetStringData( m_hstrCtrlObjName[i] ), objArray );
			
			if( objArray.NumObjects() > 0 )
			{
				g_pKeyMgr->AddKeyControl( objArray.GetObject(0), m_nKeyId );
			}
		}
	}
}