// ----------------------------------------------------------------------- //
//
// MODULE  : GearItems.cpp
//
// PURPOSE : Gear items - Implementation
//
// CREATED : 10/22/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GearItems.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "resourceextensions.h"

LINKFROM_MODULE( GearItems );

#define GEARITEM_PICKUP_SOUND	"GearItemPickup"
#define GEARITEM_RESPAWN_SOUND	"GearItemRespawn"
 
static const char s_szDefaultRS[] = "RS\\Default." RESEXT_MODEL_PACKED;
 
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_GEARITEM CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_GEARITEM 0

#endif

BEGIN_CLASS(GearItem)
	ADD_STRINGPROP_FLAG(Model, "", PF_HIDDEN | PF_STATICLIST, "Contains a list of prop objects that can be used for the model.")
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN, "HIDDEN: Obsolete. To be removed." )

	ADD_REALPROP_FLAG(RespawnTime, 15.0f, PF_HIDDEN, "The amount of time in seconds before a powerup reappears after being picked up by a player.")

	ADD_STRINGPROP_FLAG(GearType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "A pull down menu used to choose the type of GearItem the power up will be." )

	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH(GearItem, PickupItem, CF_HIDDEN_GEARITEM, CGearPlugin, DefaultPrefetch<GearItem>, "GearItem objects are used to place Game Database defined Gear records.")


CMDMGR_BEGIN_REGISTER_CLASS( GearItem )
CMDMGR_END_REGISTER_CLASS( GearItem, PickupItem )

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::GearItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GearItem::GearItem()
:	PickupItem( )
{
	m_Shared.m_ePickupItemType = kPickupItemType_Gear;
}

GearItem::~GearItem()
{
	if( m_Shared.m_hNavMarker )
	{
		g_pLTServer->RemoveObject( m_Shared.m_hNavMarker );
		m_Shared.m_hNavMarker = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GearItem::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties))
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

		case MID_ALLOBJECTSCREATED:
		{
			SpawnMarker();
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

bool GearItem::ReadProp(const GenericPropList *pProps)
{
	const char *pszGearType = pProps->GetString( "GearType", "" );

	HGEAR hGear = NULL;
	if( pszGearType[0] )
	{
		hGear = g_pWeaponDB->GetGearRecord( pszGearType );

	}

	if( !hGear )
		return false;

	m_Shared.m_hRecord = hGear;

	if (IsMultiplayerGameServer())
	{
		m_bControlledRespawn = g_pWeaponDB->GetBool( m_Shared.m_hRecord, WDB_WEAPON_bRespawnControlledMP );
	}
	else
	{
		m_bControlledRespawn = g_pWeaponDB->GetBool( m_Shared.m_hRecord, WDB_WEAPON_bRespawnControlledSP );
	}

	m_fRespawnDelay	= g_pWeaponDB->GetFloat( m_Shared.m_hRecord, WDB_WEAPON_fRespawnWait );


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
	if( !m_Shared.m_hRecord || g_pWeaponDB->IsRestricted( m_Shared.m_hRecord ))
		return false;

	if( pStruct )
	{
		pStruct->SetFileName( g_pWeaponDB->GetString( m_Shared.m_hRecord, WDB_GEAR_sModel ));

		g_pWeaponDB->CopyStringValues( m_Shared.m_hRecord, WDB_GEAR_sMaterial,
										pStruct->m_Materials[0], LTARRAYSIZE(pStruct->m_Materials),
										LTARRAYSIZE( pStruct->m_Materials[0] ));

		// See if our default model was changed...

		CheckForOverrideModel(pStruct);

		// Set up the appropriate sounds...
		m_Shared.m_sClientFX = g_pWeaponDB->GetString( m_Shared.m_hRecord, WDB_GEAR_sPowerupFX );

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

void GearItem::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);

		if (pPlayer && pPlayer->IsAlive( ) )
		{
			SetPlayerObj(hObject);

			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_ADDGEAR);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hRecord );
			g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hObject, MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PickedUp
//
//	PURPOSE:	Picked up
//
// ----------------------------------------------------------------------- //

void GearItem::PickedUp( bool bWasPickedUp, bool bWeaponsStay )
{
	
	//if we weren't picked up, bail out early
	if (!bWasPickedUp)
	{
		//forward to base class for handling before we bail
		PickupItem::PickedUp( bWasPickedUp, bWeaponsStay );
		return;
	}

	// If we were touched by a player, our m_hPlayerObj data member will be
	// set.  Send a message to that player's client letting it know that an
	// item has been picked up...

	if (m_hPlayerObj)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hPlayerObj);
		if (pPlayer && pPlayer->IsAlive())
		{
			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_SFX_MESSAGE );
				cMsg.Writeuint8( SFX_CHARACTER_ID );
				cMsg.WriteObject( pPlayer->m_hObject );
				cMsg.WriteBits(CFX_USE_GEAR, FNumBitsExclusive<CFX_COUNT>::k_nValue );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hRecord );
				cMsg.Writeuint8(kGearAdd);
				cMsg.Writeuint8( (bWasPickedUp ? 1 : 0) );
				g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
			}
		}
	}

	if (bWasPickedUp)
	{
		PickupItem::PickedUp( bWasPickedUp, bWeaponsStay );
		
		if( m_Shared.m_hNavMarker )
		{
			g_pCmdMgr->QueueMessage( m_hObject, m_Shared.m_hNavMarker, "OFF" );
			// Force the client to keep track of this object, so the nave marker 
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE);
		}
		
		if( m_bRespawn )
		{
			if( !m_Shared.m_hRecord )
				return;



			// Change the skins and renderstyles to the waiting to respawn files...

			ObjectCreateStruct ocs;

			g_pWeaponDB->CopyStringValues( m_Shared.m_hRecord, WDB_GEAR_sRespawnWaitMaterial,
											ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials),
											LTARRAYSIZE( ocs.m_Materials[0] ));
			
			g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

			// Stop playing PowerupFX and play RespawnWaitFX...
		
			SetClientFX( g_pWeaponDB->GetString( m_Shared.m_hRecord, WDB_GEAR_sRespawnWaitFX ));

			// Set our visibility...

			bool RespawnWaitVisible = g_pWeaponDB->GetBool( m_Shared.m_hRecord, WDB_GEAR_bRespawnWaitVisible );
			bool bRespawnWaitTranslucent = g_pWeaponDB->GetBool( m_Shared.m_hRecord, WDB_GEAR_bRespawnWaitTranslucent );

			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, RespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
		}
	}
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::Respawn
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void GearItem::Respawn( )
{
	PickupItem::Respawn();

	if (m_Shared.m_hNavMarker)
	{
		g_pCmdMgr->QueueMessage( m_hObject, m_Shared.m_hNavMarker, "ON" );

		char szName[256];
		g_pLTServer->GetObjectName(m_hObject,szName,LTARRAYSIZE(szName));

		char szMsg[256] = "TARGET ";
		LTStrCat(szMsg,szName ,LTARRAYSIZE(szMsg));
		g_pCmdMgr->QueueMessage( m_hObject, m_Shared.m_hNavMarker, szMsg );

	}

	if( !m_Shared.m_hRecord )
		return;
	
	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	g_pWeaponDB->CopyStringValues( m_Shared.m_hRecord, WDB_GEAR_sMaterial,
									ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials),
									LTARRAYSIZE(ocs.m_Materials[0]));

	// See if we are using a different model...

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	
	SetClientFX( g_pWeaponDB->GetString( m_Shared.m_hRecord, WDB_GEAR_sPowerupFX ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::SpawnMarker
//
//	PURPOSE:	Handle spawning a nav marker
//
// ----------------------------------------------------------------------- //

void GearItem::SpawnMarker( )
{

	if (!m_Shared.m_hRecord)
		return;

	NavMarkerCreator nmc;
	//see what kind of marker we're supposed to use
	nmc.m_hType = g_pWeaponDB->GetRecordLink(m_Shared.m_hRecord,WDB_GEAR_rNavMarker);

	//create the marker, if a type was specified
	if (nmc.m_hType)
	{
		nmc.m_nTeamId = 255;
		nmc.m_hTarget = m_hObject;

		NavMarker* pNM = nmc.SpawnMarker();
		if (pNM)
		{
			//keep track in case we need to remove it later
			m_Shared.m_hNavMarker = pNM->m_hObject;
			CreateSpecialFX( true );
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGearPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CGearPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if( !g_pWeaponDB )
		return LT_UNSUPPORTED;

	// See if we can handle the property...

	if( LTStrIEquals( "GearType", szPropName ))
	{
		ASSERT( g_pWeaponDB->GetNumGear() == ( uint8 )g_pWeaponDB->GetNumGear());
		uint8 nNumGear = ( uint8 )LTMIN( g_pWeaponDB->GetNumGear(), 255 );

		ASSERT(cMaxStrings >= nNumGear);

		HGEAR hGear = NULL;

		for( uint32 i = 0; i < nNumGear; ++i )
		{
			hGear = g_pWeaponDB->GetGearRecord(i);

			const char *pszGear = g_pWeaponDB->GetRecordName( hGear );
			if( hGear && pszGear[0] && LTStrLen(pszGear) < cMaxStringLength )
			{
				LTStrCpy( aszStrings[i], pszGear, cMaxStringLength );
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
												const char* szPropName, 
												const char* szPropValue,
												char* szModelFilenameBuf,
												int	  nModelFilenameBufLen,
												LTVector & vDims,
												const char* pszObjName, 
												ILTPreInterface *pInterface)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponDB )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	HGEAR hGear = g_pWeaponDB->GetGearRecord( szPropValue );
	if (!hGear)
		return LT_UNSUPPORTED;

	const char *pszModel = g_pWeaponDB->GetString( hGear, WDB_GEAR_sModel );
	if( !pszModel[0] )
	{
		return LT_UNSUPPORTED;
	}

	LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );

	// Need to convert the RESEXT_MODEL_PACKED filename to one that WorldEdit understands...
	
	ConvertLTBFilename( szModelFilenameBuf, nModelFilenameBufLen );

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void GearItem::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// Get the gear item type
	char szGearItemType[MAX_PATH];
	pInterface->GetPropString(pszObjectName, "GearType", szGearItemType, LTARRAYSIZE(szGearItemType), "");

	if (!LTStrEmpty(szGearItemType))
	{
		// gather resources from the database record for this type
		HGEAR hGearItem = g_pWeaponDB->GetGearRecord(szGearItemType);
		GetRecordResources(Resources, hGearItem, true);
	}
}
