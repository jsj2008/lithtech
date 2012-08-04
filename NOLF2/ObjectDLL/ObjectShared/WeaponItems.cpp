// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.cpp
//
// PURPOSE : Weapon items - Implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//			 07/12/02 - kls
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponItems.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "Character.h"
#include "ObjectMsgs.h"
#include "CVarTrack.h"
#include "GameServerShell.h"

LINKFROM_MODULE( WeaponItems );

extern CGameServerShell* g_pGameServerShell;

#define UPDATE_DELTA				0.1f
#define WEAPONITEM_PICKUP_SOUND		"WeaponItemPickup"
#define WEAPONITEM_RESPAWN_SOUND	"WeaponItemRespawn"

static const char s_szDefaultRS[] = "RS\\Default.ltb";

#pragma force_active on
BEGIN_CLASS(WeaponItem)
	// Hidden property overrides...
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_HIDDEN | PF_MODEL )
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(RespawnSound, WEAPONITEM_RESPAWN_SOUND, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, WEAPONITEM_PICKUP_SOUND, PF_HIDDEN)

	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(WeaponType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS )
	ADD_LONGINTPROP_FLAG(WeaponTypeId, WMGR_INVALID_ID, PF_HIDDEN) // Only used by SpawnObject
	ADD_LONGINTPROP(AmmoAmount, -1)
END_CLASS_DEFAULT_FLAGS_PLUGIN(WeaponItem, PickupItem, NULL, NULL, 0, CWeaponItemPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( WeaponItem )
CMDMGR_END_REGISTER_CLASS( WeaponItem, PickupItem )


extern CVarTrack g_RespawnScaleTrack;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::WeaponItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WeaponItem::WeaponItem() : PickupItem()
{
	m_nAmmo		= -1;
	m_nWeaponId = WMGR_INVALID_ID;
	m_nAmmoId	= WMGR_INVALID_ID;

    m_bBounce = LTFALSE;

	m_dwFlags |= FLAG_NOSLIDING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 WeaponItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
		    uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);
            SetNextUpdate(UPDATE_NEVER);

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp((ObjectCreateStruct*)pData))
					return 0;
			}

			if (fData != PRECREATE_SAVEGAME)
			{
				if( !PostPropRead((ObjectCreateStruct*)pData))
					return 0;
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}

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
//	ROUTINE:	WeaponItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

bool WeaponItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

 	g_pWeaponMgr->ReadWeaponProp("WeaponType", m_nWeaponId, m_nAmmoId);

	// If the WeaponType is invalid, see if the WeaponTypeId property
	// was set correctly...

	WEAPON const *pWeapon = NULL;

	if (m_nWeaponId == WMGR_INVALID_ID)
	{
		if (g_pLTServer->GetPropGeneric("WeaponTypeId", &genProp) == LT_OK)
		{
			m_nWeaponId = (uint8) genProp.m_Long;

			// Set ammo type to default (could be changed by AmmoType
			// property...
			pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
			if (pWeapon)
			{
				m_nAmmoId = pWeapon->nDefaultAmmoId;
			}
		}
	}

	pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if( !pWeapon )
		return false;

	if( pWeapon->bInfiniteAmmo )
	{
		m_nAmmo = 1000;
	}
	else
	{
		if (g_pLTServer->GetPropGeneric("AmmoAmount", &genProp) == LT_OK)
		{
			m_nAmmo = genProp.m_Long;
		}
	}

	// See if this weapon has been restricted.  If it was,
	// don't allow the weapon.
	if( pWeapon->bServerRestricted )
		return false;

	// This allows ammo type to be specified without appending it
	// to the WeaponType attribute (e.g., a spawned weapon)...

    if (g_pLTServer->GetPropGeneric("AmmoType", &genProp) == LT_OK)
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(genProp.m_String);
		if (pAmmo)
		{
			m_nAmmoId = pAmmo->nId;
		}
	}

	AMMO const* pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
	if( !pAmmo )
		return false;
	
	// If the ammo is restricted, we need to check if
	// the default ammo is restricted too.  If so, we don'
	// allow this weapon.  If the default ammo is ok, 
	// we'll just switch to that.
	if( pAmmo->bServerRestricted )
	{
		// Check if this is the default ammo, which means the only ammo.
		if( m_nAmmoId == pWeapon->nDefaultAmmoId )
			return false;

		// Make sure the default ammo id is not restricted too.  If it is, we can't
		// use this weapon.
		pAmmo = g_pWeaponMgr->GetAmmo( pWeapon->nDefaultAmmoId );
		if( !pAmmo || pAmmo->bServerRestricted )
			return false;

		// Default isn't restricted.  We'll just use that.
		m_nAmmoId = pWeapon->nDefaultAmmoId;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

bool WeaponItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if( !pWeapon )
		return false;

	if( !pStruct )
		return false;

	SAFE_STRCPY(pStruct->m_Filename, pWeapon->szHHModel);

	pWeapon->blrHHSkins.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	pWeapon->blrHHRenderStyles.CopyList(0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	// See if our default model was changed...

	CheckForOverrideModel(pStruct);

	// Set up the appropriate pick up and respawn sounds...

	FREE_HSTRING(m_hstrSoundFile);
    m_hstrSoundFile = g_pLTServer->CreateString( WEAPONITEM_PICKUP_SOUND );

	FREE_HSTRING( m_hstrRespawnSoundFile );
	m_hstrRespawnSoundFile = g_pLTServer->CreateString( WEAPONITEM_RESPAWN_SOUND );

	m_vScale = pWeapon->vHHScale;
	m_sClientFX = pWeapon->szPowerupFX;

    m_bBounce = LTFALSE;
    m_bRotate = LTFALSE;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::InitialUpdate
//
//	PURPOSE:	Handle inital update engine messages
//
// ----------------------------------------------------------------------- //

void WeaponItem::InitialUpdate()
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo) return;
    
	// Set our default ammo if necessary...

	if (m_nAmmo < 0)
	{
		m_nAmmo = pAmmo->nSpawnedAmount;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::ObjectTouch
//
//	PURPOSE:	Add weapon powerup to object
//
// ----------------------------------------------------------------------- //

void WeaponItem::ObjectTouch(HOBJECT hObject, bool bForcePickup/*=false*/)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_ADDWEAPON);
			cMsg.Writeuint8(m_nWeaponId);
            cMsg.Writeuint8(m_nAmmoId);
            cMsg.Writeint32(m_nAmmo);
			cMsg.Writebool(bForcePickup);
			g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PickedUp
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up...
//
// ----------------------------------------------------------------------- //

void WeaponItem::PickedUp( ILTMessage_Read *pMsg )
{
	PickupItem::PickedUp( pMsg );

	if( m_bRespawn )
	{
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( m_nWeaponId );
		if( !pWeapon )
			return;

		// Change the skins and renderstyles to the waiting to respawn files...

		ObjectCreateStruct ocs;

		pWeapon->blrRespawnWaitSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
		pWeapon->blrRespawnWaitRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

		if( pWeapon->blrRespawnWaitRenderStyles.GetNumItems() < 1 )
			LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
		
		g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

		// Stop playing PowerupFX and play RespawnWaitFX...
	
		SetClientFX( pWeapon->szRespawnWaitFX );

		// Set our visibility and translucency...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, pWeapon->bRespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, pWeapon->bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Respawn
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void WeaponItem::Respawn( )
{
	PickupItem::Respawn();

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( m_nWeaponId );
	if( !pWeapon )
		return;
	
	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	pWeapon->blrHHSkins.CopyList( 0, ocs.m_SkinNames[0], MAX_CS_FILENAME_LEN + 1 );
	pWeapon->blrHHRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN + 1 );

	if( pWeapon->blrHHRenderStyles.GetNumItems() < 1 )
		LTStrCpy( ocs.m_RenderStyleNames[0], s_szDefaultRS, ARRAY_LEN( s_szDefaultRS ));
	
	// See if we are using a different model...

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	
	SetClientFX( pWeapon->szPowerupFX );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

    SAVE_BYTE(m_nWeaponId);
    SAVE_BYTE(m_nAmmoId);
    SAVE_INT(m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

    LOAD_BYTE(m_nWeaponId);
    LOAD_BYTE(m_nAmmoId);
    LOAD_INT(m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2
LTRESULT CWeaponItemPlugin::PreHook_EditStringList(const char* szRezPath, 
												   const char* szPropName, 
												   char** aszStrings, 
												   uint32* pcStrings, 
												   const uint32 cMaxStrings, 
												   const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi("WeaponType", szPropName) == 0)
	{
		m_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		m_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CWeaponItemPlugin::PreHook_Dims(const char* szRezPath,
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims )
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponMgr ) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Remove the , that is put into some weapon names.
	char szModifiedPropValue[256];
	SAFE_STRCPY( szModifiedPropValue, szPropValue );
	strtok( szModifiedPropValue, "," );

	WEAPON const* pWeapon = g_pWeaponMgr->GetWeapon(( char* )szModifiedPropValue);
	if( !pWeapon || !pWeapon->szHHModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	strcpy( szModelFilenameBuf, pWeapon->szHHModel );

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}



#endif