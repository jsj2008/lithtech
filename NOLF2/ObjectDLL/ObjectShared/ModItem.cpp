// ----------------------------------------------------------------------- //
//
// MODULE  : ModItem.cpp
//
// PURPOSE : Mod items - Implementation
//
// CREATED : 7/21/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModItem.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

LINKFROM_MODULE( ModItem );

#define MODITEM_PICKUP_SOUND	"ModItemPickupSound"
#define MODITME_RESPAWN_SOUND	"ModItemRespawnSound"

static const char s_szDefaultRS[] = "RS\\Default.ltb";
 
#pragma force_active on
BEGIN_CLASS(ModItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_HIDDEN | PF_MODEL )
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(RespawnSound, MODITME_RESPAWN_SOUND, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, MODITEM_PICKUP_SOUND, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN | PF_DIMS)
	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(ModType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
END_CLASS_DEFAULT_FLAGS_PLUGIN(ModItem, PickupItem, NULL, NULL, 0, CModPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( ModItem )
CMDMGR_END_REGISTER_CLASS( ModItem, PickupItem )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::ModItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ModItem::ModItem() : PickupItem()
{
	m_nModId	= -1;
    m_bBounce	= LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ModItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp((ObjectCreateStruct*)pData))
					return 0;
			}

			if( fData != PRECREATE_SAVEGAME )
			{
				if( !PostPropRead((ObjectCreateStruct*)pData))
					return 0;
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return PickupItem::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

bool ModItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("ModType", &genProp) == LT_OK)
	{
		MOD const *pMod = g_pWeaponMgr->GetMod(genProp.m_String);
		if( !pMod )
			return false;

		m_nModId = pMod->nId;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

bool ModItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	MOD const *pMod = g_pWeaponMgr->GetMod(m_nModId);
	if( !pMod )
		return false;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pMod->szPowerupModel);

		pMod->blrPowerupSkins.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		pMod->blrPowerupRenderStyles.CopyList(0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

		// See if our default model was changed...

		CheckForOverrideModel(pStruct);

		// Set up the appropriate sounds...

		if (pMod->szPickUpSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pMod->szPickUpSound);
		}

		if (pMod->szRespawnSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pMod->szRespawnSound);
		}

		m_vScale.Init(pMod->fPowerupScale, pMod->fPowerupScale, pMod->fPowerupScale);
		
		m_sClientFX = pMod->szPowerupFX;

        m_bRotate = LTFALSE;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::ObjectTouch
//
//	PURPOSE:	Add mod to object
//
// ----------------------------------------------------------------------- //

void ModItem::ObjectTouch(HOBJECT hObject, bool bForcePickup/*=false*/)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);

		if (pPlayer && !pPlayer->IsDead())
		{
			SetPlayerObj(hObject);

			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_ADDMOD);
			cMsg.Writeuint8((uint8)m_nModId);
			g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ModItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE(m_nModId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ModItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_BYTE(m_nModId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::PickedUp
//
//	PURPOSE:	Picked up
//
// ----------------------------------------------------------------------- //

void ModItem::PickedUp(ILTMessage_Read *pMsg)
{
	// Did we really pick it up?

	bool bPickedUp  = (pMsg ? pMsg->Readbool() : true);
	bool bDisplaMsg = (pMsg ? pMsg->Readbool() : true);

	// If we were touched by a player, our m_hPlayerObj data member will be
	// set.  Send a message to that player's client letting it know that an
	// item has been picked up...

	if (m_hPlayerObj)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hPlayerObj);
		if (pPlayer && !pPlayer->IsDead())
		{
			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_MOD_PICKUP_ID);
				cMsg.Writeuint8((uint8)bPickedUp);
                cMsg.Writeuint8(m_nModId);
                cMsg.Writefloat(bDisplaMsg ? 1.0f : 0.0f);
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}
		}
	}

	if (bPickedUp)
	{
		PickupItem::PickedUp(pMsg);
		
		if( m_bRespawn )
		{
			MOD const *pMod = g_pWeaponMgr->GetMod( m_nModId );
			if( !pMod )
				return;

			// Change the skins and renderstyles to the waiting to respawn files...

			ObjectCreateStruct ocs;

			pMod->blrRespawnWaitSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
			pMod->blrRespawnWaitRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

			if( pMod->blrRespawnWaitRenderStyles.GetNumItems() < 1 )
				LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
			
			g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

			// Stop playing PowerupFX and play RespawnWaitFX...
		
			SetClientFX( pMod->szRespawnWaitFX );

			// Set our visibility...

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, pMod->bRespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, pMod->bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::PickedUp
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void ModItem::Respawn( )
{
	PickupItem::Respawn();

	MOD const *pMod = g_pWeaponMgr->GetMod( m_nModId );
	if( !pMod )
		return;
	
	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	pMod->blrPowerupSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
	pMod->blrPowerupRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	if( pMod->blrPowerupRenderStyles.GetNumItems() < 1 )
		LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
	
	// See if we are using a different model...

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	
	SetClientFX( pMod->szPowerupFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CModPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the property...

	if (_strcmpi("ModType", szPropName) == 0)
	{
        uint32 dwModIds = g_pWeaponMgr->GetNumModIds();

		_ASSERT(cMaxStrings >= dwModIds);

        MOD const *pMod = LTNULL;

        for (uint32 i=0; i < dwModIds; i++)
		{
			pMod = g_pWeaponMgr->GetMod(i);

			if (pMod && pMod->szName[0] && strlen(pMod->szName) < cMaxStringLength)
			{
				strcpy(aszStrings[i], pMod->szName);
				(*pcStrings)++;
			}
			else
			{
				return LT_UNSUPPORTED;
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModPlugin::PreHook_EditStringList
//
//	PURPOSE:	Check the changed prop value
//
// ----------------------------------------------------------------------- //

LTRESULT CModPlugin::PreHook_PropChanged( const char *szObjName,
										  const char *szPropName,
										  const int nPropType,
										  const GenericProp &gpPropValue,
										  ILTPreInterface *pInterface,
										  const char *szModifiers )
{
	// Just pass it to the PickUpPlugin...
	
	if( m_PickupItemPlugin.PreHook_PropChanged( szObjName,
												szPropName,
												nPropType,
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CModPlugin::PreHook_Dims(
												const char* szRezPath,
												const char* szPropValue,
												char* szModelFilenameBuf,
												int	  nModelFilenameBufLen,
												LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponMgr ) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	MOD const* pMod = g_pWeaponMgr->GetMod(( char* )szPropValue );
	if( !pMod || !pMod->szPowerupModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pMod->szPowerupModel );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}

