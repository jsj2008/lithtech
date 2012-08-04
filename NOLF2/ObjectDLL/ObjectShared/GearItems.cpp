// ----------------------------------------------------------------------- //
//
// MODULE  : GearItems.cpp
//
// PURPOSE : Gear items - Implementation
//
// CREATED : 10/22/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GearItems.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

LINKFROM_MODULE( GearItems );

#define GEARITEM_PICKUP_SOUND	"GearItemPickup"
#define GEARITEM_RESPAWN_SOUND	"GearItemRespawn"

static const char s_szDefaultRS[] = "RS\\Default.ltb";
 
#pragma force_active on
BEGIN_CLASS(GearItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_HIDDEN | PF_MODEL )
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(RespawnSound, GEARITEM_RESPAWN_SOUND, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, GEARITEM_PICKUP_SOUND, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN )

	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(GearType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
END_CLASS_DEFAULT_FLAGS_PLUGIN(GearItem, PickupItem, NULL, NULL, 0, CGearPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( GearItem )
CMDMGR_END_REGISTER_CLASS( GearItem, PickupItem )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::GearItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GearItem::GearItem() : PickupItem()
{
	m_nGearId = -1;
    m_bBounce = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GearItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
				if( !PostPropRead((ObjectCreateStruct*)pData) )
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
//	ROUTINE:	GearItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

bool GearItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("GearType", &genProp) == LT_OK)
	{
		GEAR const *pGear = g_pWeaponMgr->GetGear(genProp.m_String);
		if (pGear)
		{
			if( pGear->bServerRestricted )
				return false;

			m_nGearId = pGear->nId;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

bool GearItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	GEAR const *pGear = g_pWeaponMgr->GetGear(m_nGearId);
	if( !pGear || pGear->bServerRestricted )
		return false;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pGear->szModel);

		pGear->blrSkins.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		pGear->blrRenderStyles.CopyList(0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

		// See if our default model was changed...

		CheckForOverrideModel(pStruct);

		// Set up the appropriate sounds...

		if (pGear->szPickUpSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pGear->szPickUpSound);
		}

		if (pGear->szRespawnSound[0])
		{
			FREE_HSTRING(m_hstrRespawnSoundFile);
			m_hstrRespawnSoundFile = g_pLTServer->CreateString(pGear->szRespawnSound);
		}

		m_sClientFX = pGear->szPowerupFX;

        m_bRotate = LTFALSE;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::ObjectTouch
//
//	PURPOSE:	Add weapon PickupItem to object
//
// ----------------------------------------------------------------------- //

void GearItem::ObjectTouch(HOBJECT hObject, bool bForcePickup/*=false*/)
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
			cMsg.Writeuint32(MID_ADDGEAR);
			cMsg.Writeuint8((uint8)m_nGearId);
			g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GearItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE(m_nGearId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GearItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_BYTE(m_nGearId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PickedUp
//
//	PURPOSE:	Picked up
//
// ----------------------------------------------------------------------- //

void GearItem::PickedUp(ILTMessage_Read *pMsg)
{
	// Did we really pick it up?

	bool bPickedUp = (pMsg ? pMsg->Readbool() : true);

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
				cMsg.Writeuint8(MID_GEAR_PICKEDUP);
				cMsg.Writeuint8((uint8)m_nGearId);
				cMsg.Writebool(bPickedUp);
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}
		}
	}

	if (bPickedUp)
	{
		PickupItem::PickedUp(pMsg);
		
		if( m_bRespawn )
		{
			GEAR const *pGear = g_pWeaponMgr->GetGear( m_nGearId );
			if( !pGear )
				return;

			// Change the skins and renderstyles to the waiting to respawn files...

			ObjectCreateStruct ocs;

			pGear->blrRespawnWaitSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
			pGear->blrRespawnWaitRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

			if( pGear->blrRespawnWaitRenderStyles.GetNumItems() < 1 )
				LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
			
			g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

			// Stop playing PowerupFX and play RespawnWaitFX...
		
			SetClientFX( pGear->szRespawnWaitFX );

			// Set our visibility...

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, pGear->bRespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, pGear->bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PickedUp
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void GearItem::Respawn( )
{
	PickupItem::Respawn();

	GEAR const *pGear = g_pWeaponMgr->GetGear( m_nGearId );
	if( !pGear )
		return;
	
	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	pGear->blrSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
	pGear->blrRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	if( pGear->blrRenderStyles.GetNumItems() < 1 )
		LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
	
	// See if we are using a different model...

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	
	SetClientFX( pGear->szPowerupFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGearPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2
LTRESULT CGearPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the property...

	if (_strcmpi("GearType", szPropName) == 0)
	{
        uint32 dwGearIds = g_pWeaponMgr->GetNumGearIds();

		_ASSERT(cMaxStrings >= dwGearIds);

        GEAR const *pGear = LTNULL;

        for (uint32 i=0; i < dwGearIds; i++)
		{
			pGear = g_pWeaponMgr->GetGear(i);

			if (pGear && pGear->szName[0] && strlen(pGear->szName) < cMaxStringLength)
			{
				strcpy(aszStrings[i], pGear->szName);
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
//	ROUTINE:	CGearPlugin::PreHook_EditStringList
//
//	PURPOSE:	Check the changed prop value
//
// ----------------------------------------------------------------------- //

LTRESULT CGearPlugin::PreHook_PropChanged( const char *szObjName,
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
//	ROUTINE:	CGearPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CGearPlugin::PreHook_Dims(
												const char* szRezPath,
												const char* szPropValue,
												char* szModelFilenameBuf,
												int	  nModelFilenameBufLen,
												LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponMgr ) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	GEAR const* pGear = g_pWeaponMgr->GetGear(( char* )szPropValue );
	if( !pGear || !pGear->szModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pGear->szModel );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}

#endif
