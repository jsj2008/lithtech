// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.cpp
//
// PURPOSE : AmmoBox object implementation
//
// CREATED : 10/28/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AmmoBox.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "resourceextensions.h"

#define UPDATE_DELTA			0.1f
#define UNUSED_STRING			"<none>"

#define SBMGR_AMMOBOX_POWERUPFX					"PowerupFX"
#define SBMGR_AMMOBOX_RESPAWNWAITFX				"RespawnWaitFX"
#define SBMGR_AMMOBOX_RESPAWNWAITMATERIALS		"RespawnWaitMaterials"
#define SBMGR_AMMOBOX_RESPAWNWAITVISIBLE		"RespawnWaitVisible"
#define SBMGR_AMMOBOX_RESPAWNWAITTRANSLUCENT	"RespawnWaitTranslucent"

static const char s_szDefaultRS[]		= "RS\\Default." RESEXT_MODEL_PACKED;
		
#define ModelNone	"<none>"

#define ADD_AMMO_PROP(num) \
		ADD_STRINGPROP_FLAG(AmmoType##num, UNUSED_STRING, PF_STATICLIST, "A pull down menu used to choose a type of ammunition to be contained within the powerup.")\
		ADD_LONGINTPROP(AmmoCount##num, 0, "The quantity of ammunition.")

LINKFROM_MODULE( AmmoBox );


BEGIN_CLASS(AmmoBox)
	ADD_REALPROP(RespawnTime, 30.0f, "The amount of time in seconds before a powerup reappears after being picked up by a player.")
	ADD_AMMO_PROP(1)
	ADD_AMMO_PROP(2)
	ADD_AMMO_PROP(3)
	ADD_AMMO_PROP(4)
	ADD_AMMO_PROP(5)
	ADD_AMMO_PROP(6)
	ADD_AMMO_PROP(7)
	ADD_AMMO_PROP(8)
	ADD_AMMO_PROP(9)
	ADD_AMMO_PROP(10)
END_CLASS_FLAGS_PLUGIN(AmmoBox, PickupItem, 0, CAmmoBoxPlugin, "AmmoBoxes are pick up items that can have multiple types of ammo for players to aquire when they walk over an AmmoBox." )


CMDMGR_BEGIN_REGISTER_CLASS( AmmoBox )
CMDMGR_END_REGISTER_CLASS( AmmoBox, PickupItem )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::AmmoBox
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AmmoBox::AmmoBox() : PickupItem()
{
	for (int i = 0; i < AB_MAX_TYPES; i++)
	{
		m_hAmmo[i] = m_hOriginalAmmo[i] = NULL;
		m_nAmmoCount[i]	= m_nOriginalAmmoCount[i] = 0;
	}

	m_bRespawnWaitVisible		= false;
	m_bRespawnWaitTranslucent	= false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties) )
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
//	ROUTINE:	AmmoBox::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

bool AmmoBox::ReadProp(const GenericPropList *pProps)
{
	// Counts the number of ammo types we put in our box.
	int nNumAmmoTypes = 0;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		char key[40];

		HAMMO hAmmo = NULL;

		LTSNPrintF( key, LTARRAYSIZE( key ), "AmmoType%d", i+1);
		const char *pszAmmoType = pProps->GetString( key, "" );

		if( pszAmmoType[0] )
		{
			if( !LTStrEquals( pszAmmoType, UNUSED_STRING ))
			{
				hAmmo = g_pWeaponDB->GetAmmoRecord( pszAmmoType );
				if( hAmmo )
				{
					// See if this ammo was server restricted.
					HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
					if( g_pWeaponDB->IsRestricted( hAmmoData ))
					{
						hAmmo = NULL;
					}
					else
					{
						m_hAmmo[nNumAmmoTypes] = m_hOriginalAmmo[nNumAmmoTypes] = hAmmo;
					}
				}
			}
		}

		if( hAmmo )
		{
			LTSNPrintF( key, LTARRAYSIZE( key ), "AmmoCount%d", i+1);
			int32 nAmmoCount = pProps->GetLongInt( key, 0 );

			m_nAmmoCount[nNumAmmoTypes] = m_nOriginalAmmoCount[nNumAmmoTypes] = nAmmoCount;
			if (m_nAmmoCount[nNumAmmoTypes] == 0)
			{
				HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,false); 
				int32 nSpawnedAmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPickupInitialAmount );
				m_nAmmoCount[nNumAmmoTypes] = m_nOriginalAmmoCount[nNumAmmoTypes] = nSpawnedAmount;
			}

			nNumAmmoTypes++;
		}
	}

	// See if there were no ammotypes that made it.  If so, then don't create this box.
	if( nNumAmmoTypes == 0 )
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectTouch
//
//	PURPOSE:	Add weapon PickupItem to object
//
// ----------------------------------------------------------------------- //

void AmmoBox::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && pCharObj->IsAlive())
		{
			uint8 nValidIds = 0;
			for (int i=0; i < AB_MAX_TYPES; i++)
			{
				if (m_hAmmo[i] != NULL && m_nAmmoCount[i] > 0)
				{
					nValidIds++;
				}
			}
			if (nValidIds)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint32(MID_AMMOBOX);
				cMsg.Writeuint8(nValidIds);
				for (int i=0; i < AB_MAX_TYPES; i++)
				{
					if (m_hAmmo[i] != NULL && m_nAmmoCount[i] > 0)
					{
						cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo[i] );
						cMsg.Writeint32(m_nAmmoCount[i]);
					}
				}
				g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Save(ILTMessage_Write *pMsg, uint32 /*dwSaveFlags*/)
{
	if (!pMsg) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		SAVE_HRECORD( m_hAmmo[i] );
		pMsg->Writeint32(m_nAmmoCount[i]);
		SAVE_HRECORD( m_hOriginalAmmo[i] );
		pMsg->Writeint32(m_nOriginalAmmoCount[i]);

	}

	SAVE_STDSTRING( m_sOriginalFilename );
	SAVE_STDSTRING( m_sOriginalMaterial);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Load(ILTMessage_Read *pMsg, uint32 /*dwLoadFlags*/)
{
	if (!pMsg) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
        LOAD_HRECORD( m_hAmmo[i], g_pWeaponDB->GetAmmoCategory() );
        m_nAmmoCount[i] = (int)pMsg->Readint32();
        LOAD_HRECORD( m_hOriginalAmmo[i], g_pWeaponDB->GetAmmoCategory() );
        m_nOriginalAmmoCount[i] = (int)pMsg->Readint32();
	}

	LOAD_STDSTRING( m_sOriginalFilename );
	LOAD_STDSTRING( m_sOriginalMaterial );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_AMMOBOX:
		{
			//figure out what's left
			Leftovers(pMsg);
		}
		break;

		default: break;
	}

	return PickupItem::ObjectMessageFn(hSender, pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Leftovers
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void AmmoBox::Leftovers(ILTMessage_Read *pMsg)
{
    uint8 numAmmoTypes = pMsg->Readuint8();

    int i;
    for (i = 0; i < numAmmoTypes; i++)
	{
        m_hAmmo[i]    = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
        m_nAmmoCount[i] = (int) pMsg->Readint32();

		if (m_nAmmoCount[i] <= 0)
			m_hAmmo[i] = NULL;

	}

	for (i=numAmmoTypes; i < AB_MAX_TYPES; i++)
	{
		m_hAmmo[i] = NULL;
		m_nAmmoCount[i] = 0;
	}

	SendPickedUp();
	
	// If in a deathmatch game we should act like the ammobox was empty and pick it up
	// if anything from the box was taken.
	
	if( IsMultiplayerGameServer() )
	{
		PickedUp( true, false );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::PickedUp
//
//	PURPOSE:	Restock in preparation for respawn...
//
// ----------------------------------------------------------------------- //

void AmmoBox::PickedUp( bool bWasPickedUp, bool bWeaponsStay )
{

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		m_hAmmo[i] = m_hOriginalAmmo[i];
		m_nAmmoCount[i] = m_nOriginalAmmoCount[i];
	}


	PickupItem::PickedUp( bWasPickedUp, bWeaponsStay );

	if (!bWasPickedUp)
		return;
	
	if( m_bRespawn )
	{
		// Change the skins and renderstyles to the waiting to respawn files...

		ObjectCreateStruct ocs;

		g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

		// Stop playing PowerupFX and play RespawnWaitFX...
	
		SetClientFX( m_sRespawnWaitFX.c_str() );

		// Set our visibility...

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_bRespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, m_bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Respawn
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void AmmoBox::Respawn( )
{
	PickupItem::Respawn();

	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	ocs.SetFileName( m_sOriginalFilename.c_str( ));
	ocs.SetMaterial( 0, m_sOriginalMaterial.c_str( ));

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	
	SetClientFX( m_sPowerupFX.c_str() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_EditStringList(const char* /*szRezPath*/, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( !g_pWeaponDB )
		return LT_UNSUPPORTED;

	// See if we can handle the property...

	char key[40];
	for (int index=1; index <= AB_MAX_TYPES; index++)
	{
		LTSNPrintF( key, ARRAY_LEN(key), "AmmoType%d", index );

		if( LTStrIEquals( key, szPropName ))
		{
			uint8 nAmmoTypes = g_pWeaponDB->GetNumAmmo();

			ASSERT(cMaxStrings >= nAmmoTypes);
			*pcStrings = 1;

			// Make sure the first string is the unused slot...

			LTStrCpy( aszStrings[0], UNUSED_STRING, cMaxStringLength );

			HAMMO hAmmo = NULL;

			for( uint8 i = 1; i <= nAmmoTypes; ++i )
			{
				hAmmo = g_pWeaponDB->GetAmmoRecord(i-1);

				const char *pszAmmo = g_pWeaponDB->GetRecordName( hAmmo );
				if( pszAmmo && LTStrLen(pszAmmo) < cMaxStringLength)
				{
					LTStrCpy( aszStrings[i], pszAmmo, cMaxStringLength );
					(*pcStrings)++;
				}
				else
				{
					return LT_UNSUPPORTED;
				}
			}

			return LT_OK;
		}
	}
	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList
//
//	PURPOSE:	Check the changed prop value
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_PropChanged( const char *szObjName,
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
//	ROUTINE:	CAmmoBoxPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this item
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_Dims(const char* szRezPath,
									  const char* szPropName, 
								   const char* szPropValue,
								   char* szModelFilenameBuf,
								   int nModelFilenameBufLen,
								   LTVector & vDims,
								   const char* pszObjName, 
								   ILTPreInterface *pInterface)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pPropsDB )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Get the prop and make sure it's not "<none>"
	PropsDB::HPROP hProp = NULL;
	static CParsedMsg::CToken s_cTok_None( ModelNone );
	CParsedMsg::CToken cTokModel( szPropValue );
	if( cTokModel != s_cTok_None )
		hProp = g_pPropsDB->GetPropByRecordName( cTokModel.c_str());
	if( !hProp )
		return LT_UNSUPPORTED;

	// Get the model and set it as our dims model
	const char *pszModel = g_pPropsDB->GetPropFilename( hProp );
	if( !pszModel[0] )
		return LT_UNSUPPORTED;
	LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );

	return LT_OK;
}

