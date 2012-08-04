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
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "WeaponItems.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "Character.h"
#include "ObjectMsgs.h"
#include "VarTrack.h"
#include "GameServerShell.h"
#include "resourceextensions.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "AIStimulusMgr.h"
#include "CharacterDB.h"
#include "AIDB.h"
#include "ServerUtilities.h"
#include "CollisionsDB.h"
#include "FXDB.h"

LINKFROM_MODULE( WeaponItems );

extern CGameServerShell* g_pGameServerShell;

#define UPDATE_DELTA				0.1f

static const char s_szDefaultRS[] = "RS\\Default." RESEXT_MODEL_COMPRESSED;

BEGIN_CLASS(WeaponItem)
	// Hidden property overrides...
	ADD_STRINGPROP_FLAG(Model, "", PF_HIDDEN | PF_STATICLIST, "Contains a list of prop objects that can be used for the model.")
	ADD_REALPROP_FLAG(RespawnTime, 15.0f, PF_HIDDEN, "The amount of time in seconds before a powerup reappears after being picked up by a player.")
	ADD_STRINGPROP_FLAG(WeaponType, "", PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "A pull down menu used to choose the type of WeaponItem the power up will be.")
	ADD_STRINGPROP_FLAG(AmmoType, "", PF_HIDDEN, "This allows ammo type to be specified without appending it to the WeaponType attribute (e.g., a spawned weapon)") 
	ADD_LONGINTPROP(AmmoAmount, -1, "This value determines the amount of ammunition that the weapon will have. A value of -1 will use the default amounts set in the attribute file.")
	ADD_LONGINTPROP(Health, PF_HIDDEN, "This value determines the number of impacts that the weapon can sustain. A value of -1 will use the maximum health for the weapon type.")
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH(WeaponItem, PickupItem, 0, CWeaponItemPlugin, DefaultPrefetch<WeaponItem>, "A Weapon pickup item that a player can acquire.")

CMDMGR_BEGIN_REGISTER_CLASS( WeaponItem )
CMDMGR_END_REGISTER_CLASS( WeaponItem, PickupItem )


extern VarTrack g_RespawnScaleTrack;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::WeaponItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WeaponItem::WeaponItem() 
:	PickupItem		( ),
	m_nAmmo			( -1 ),
	m_hAmmo			( NULL ),
	m_eStimID		( kStimID_Invalid ),
	m_nHealth		( 0 )
{
	m_dwFlags |= FLAG_NOSLIDING;

	// KLS 5/2/04 - Projectiles shouldn't impact on weapon items...
	m_dwUserFlags |= USRFLG_IGNORE_PROJECTILES;

	m_Shared.m_ePickupItemType = kPickupItemType_Weapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::WeaponItem
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

WeaponItem::~WeaponItem()
{
	// Insure the stimulus is unregistered.

	UnregisterStimulus();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 WeaponItem::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
		    uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);
            //SetNextUpdate(UPDATE_NEVER);

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				if( !ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties))
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

bool WeaponItem::ReadProp(const GenericPropList *pProps)
{
	const char *pszWeaponType = pProps->GetString( "WeaponType", "" );
	if( pszWeaponType[0] )
	{
		g_pWeaponDB->ReadWeapon( pszWeaponType, m_Shared.m_hRecord, m_hAmmo, !USE_AI_DATA );
	}

	if( !m_Shared.m_hRecord )
		return false;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hRecord, !USE_AI_DATA);
	if	(WasPlaced() && !g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bCanBePlaced))
	{
		LTERROR_PARAM1("Weapon %s is not a legal weapon to be placed in a level.",pszWeaponType);
		g_pLTServer->CPrint("Weapon %s is not a legal weapon to be placed in a level.",pszWeaponType);
		return false;
	}




	if( g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ))
	{
		m_nAmmo = 1000;
	}
	else
	{
		m_nAmmo = pProps->GetLongInt( "AmmoAmount", m_nAmmo );
	}

	m_nHealth = pProps->GetLongInt( "Health", g_pWeaponDB->GetInt32(hWpnData, WDB_WEAPON_nMaxHealth) );

	// See if this weapon has been restricted.  If it was,
	// don't allow the weapon.
	if( g_pWeaponDB->IsRestricted( hWpnData ) )
		return false;

	// This allows ammo type to be specified without appending it
	// to the WeaponType attribute (e.g., a spawned weapon)...

	const char *pszAmmoType = pProps->GetString( "AmmoType", "" );
	if( pszAmmoType[0] )
	{
		m_hAmmo = g_pWeaponDB->GetAmmoRecord(pszAmmoType );
	}

	if( !m_hAmmo )
		return false;
	
	// If the ammo is restricted, we need to check if
	// the default ammo is restricted too.  If so, we don'
	// allow this weapon.  If the default ammo is ok, 
	// we'll just switch to that.
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,!USE_AI_DATA); 
	if( g_pWeaponDB->IsRestricted( hAmmoData ))
	{
		// Check if this is the default ammo, which means the only ammo.

		HAMMO hDefaultAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		if( m_hAmmo == hDefaultAmmo )
			return false;

		// Make sure the default ammo id is not restricted too.  If it is, we can't
		// use this weapon.
		HAMMODATA hDefAmmoData = g_pWeaponDB->GetAmmoData(hDefaultAmmo,!USE_AI_DATA); 
		if( g_pWeaponDB->IsRestricted( hDefAmmoData ))
			return false;

		// Default isn't restricted.  We'll just use that.
		m_hAmmo = hDefaultAmmo;
	}

	m_sCountdownFX = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sCountdownFX);
	m_sEndFX = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sEndFX);

	if (IsMultiplayerGameServer())
	{
		m_bControlledRespawn = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bRespawnControlledMP);
	}
	else
	{
		m_bControlledRespawn = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bRespawnControlledSP);
	}

	m_fRespawnDelay	= g_pWeaponDB->GetFloat(hWpnData,WDB_WEAPON_fRespawnWait);

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
	if( !m_Shared.m_hRecord )
		return false;

	if( !pStruct )
		return false;

	HATTRIBUTE hAttrib = NULL;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_Shared.m_hRecord, !USE_AI_DATA );
	HATTRIBUTE hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_RightHandWeapon );
	if( !hWeaponModelStruct )
	{
		LTERROR( "Failed to retrieve the weapon model struct." );
		return false;
	}

	hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
	const char *pszFilename = g_pWeaponDB->GetString( hAttrib );
	if( !pszFilename || !pszFilename[0] )
	{
		// Use the left model...
		
		hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_LeftHandWeapon );

		hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
		pStruct->SetFileName( g_pWeaponDB->GetString( hAttrib ));

		// Get all specified materials...
		char szCompleteName[256];
		LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", WDB_WEAPON_LeftHandWeapon, WDB_WEAPON_sHHMaterial );
		g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, pStruct->m_Materials[0],
			LTARRAYSIZE(pStruct->m_Materials), LTARRAYSIZE(pStruct->m_Materials[0]) );

		hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_fHHScale );
		m_fScale = g_pWeaponDB->GetFloat( hAttrib );
		
	}
	else
	{
		pStruct->SetFileName( pszFilename );

		// Get all specified materials...
		char szCompleteName[256];
		LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", WDB_WEAPON_RightHandWeapon, WDB_WEAPON_sHHMaterial );
		g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName,pStruct->m_Materials[0],
			LTARRAYSIZE(pStruct->m_Materials), LTARRAYSIZE(pStruct->m_Materials[0]) );

		hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_fHHScale );
		m_fScale = g_pWeaponDB->GetFloat( hAttrib );
	}

	 // See if our default model was changed...

	CheckForOverrideModel(pStruct);

	m_Shared.m_sClientFX = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sPowerupFX );

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
	if( !m_hAmmo )
		return;
    
	// Set our default ammo if necessary...
	if (m_nAmmo <= 0)
	{
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,false);
		//looked up the placed amount
		if (WasPlaced())
		{
			m_nAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPlacedAmount );
		}
		//if we still don't have any, used the initial pickup amount
		if (m_nAmmo <= 0)
		{
			m_nAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPickupInitialAmount );
		}
		
	}

	// Register a stimulus with the AI, so that it knows about this weapon.

	RegisterStimulus();

	// for broken weapon pickups the use undamaged weapon model assets
	DisplayWeaponModelPieces(m_hObject, m_Shared.m_hRecord, WDB_WEAPON_sShowPieces, true,  !USE_AI_DATA);
	DisplayWeaponModelPieces(m_hObject, m_Shared.m_hRecord, WDB_WEAPON_sHidePieces, false, !USE_AI_DATA);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::ObjectTouch
//
//	PURPOSE:	Add weapon powerup to object
//
// ----------------------------------------------------------------------- //

void WeaponItem::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject) || IsAI(hObject))
	{
		CCharacter* pCharObj = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hObject ));

		if (pCharObj && pCharObj->IsAlive())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_ADDWEAPON);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hRecord );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );
            cMsg.Writeint32(m_nAmmo);
			cMsg.Writebool(m_bRespawn);
			cMsg.Writeuint32(m_nHealth);
			cMsg.Writebool(WasPlaced());
			cMsg.WriteObject(m_hDroppedBy);
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

void WeaponItem::PickedUp( bool bWasPickedUp, bool bWeaponsStay )
{
	PickupItem::PickedUp( bWasPickedUp, bWeaponsStay );

	// Read the message for ourselves.

	if( bWeaponsStay || !bWasPickedUp )
		return;

	// Always remove the stimulus.  If it respawns, reregister it.

	if ( bWasPickedUp && ( m_eStimID != kStimID_Invalid ) )
	{
		UnregisterStimulus();
	}

	if( m_bRespawn )
	{
		if( !m_Shared.m_hRecord )
			return;

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hRecord, !USE_AI_DATA);

		// Change the skins and renderstyles to the waiting to respawn files...

		ObjectCreateStruct ocs;

		g_pWeaponDB->CopyStringValues( hWpnData, WDB_WEAPON_sRespawnWaitMaterial,
										ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials),
										LTARRAYSIZE(ocs.m_Materials[0]));
		
		g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

		// Stop playing PowerupFX and play RespawnWaitFX...
	
		SetClientFX( g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sRespawnWaitFX ));

		// Set our visibility and translucency...

		bool bRespawnWaitVisible = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bRespawnWaitVisible );
		bool bRespawnWaitTranslucent = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bRespawnWaitTranslucent );

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, bRespawnWaitVisible ? FLAG_VISIBLE : 0, FLAG_VISIBLE );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, bRespawnWaitTranslucent ? FLAG2_FORCETRANSLUCENT : 0, FLAG2_FORCETRANSLUCENT );
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

	if( !m_Shared.m_hRecord )
		return;
	
	// Change the skins and renderstyles back to the normal powerup files...

	ObjectCreateStruct ocs;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hRecord, !USE_AI_DATA);
	g_pWeaponDB->CopyStringValues(	hWpnData, WDB_WEAPON_sHHMaterial,
									ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials), 
									LTARRAYSIZE(ocs.m_Materials[0]));

	// Get all specified materials...
	char szCompleteName[256];
	LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", WDB_WEAPON_RightHandWeapon, WDB_WEAPON_sHHMaterial );
	g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, ocs.m_Materials[0],
								   LTARRAYSIZE(ocs.m_Materials), LTARRAYSIZE(ocs.m_Materials[0]) );

	if( !ocs.m_Materials[0][0] )
	{
		LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", WDB_WEAPON_LeftHandWeapon, WDB_WEAPON_sHHMaterial );
		g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, ocs.m_Materials[0],
									   LTARRAYSIZE(ocs.m_Materials), LTARRAYSIZE(ocs.m_Materials[0]) );
	}

	
	// See if we are using a different model...

	CheckForOverrideModel( &ocs );

	g_pCommonLT->SetObjectFilenames( m_hObject, &ocs );

	// Stop playing RespawnWaitFX and play PowerupFX...
	SetClientFX( g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sPowerupFX ));

	// Register a stimulus with the AI, so that it knows about this weapon.

	RegisterStimulus();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::RegisterStimulus
//
//	PURPOSE:	Handle registering a stimulus with the AI system, 
//				to allow the AI to interact with this WeaponItem.
//				This logic currently assumes that WeaponItems do not
//				move.  If they do, this may need to be readdressed.
//
// ----------------------------------------------------------------------- //

void WeaponItem::RegisterStimulus()
{
	if (m_eStimID != kStimID_Invalid)
	{
		return;
	}

	if (g_pAIStimulusMgr)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		EnumCharacterAlignment eAlignment = g_pCharacterDB->String2Alignment(g_pAIDB->GetAIConstantsRecord()->strObjectAlignmentName.c_str());
		StimulusRecordCreateStruct scs(kStim_WeaponItem, eAlignment, vPos, m_hObject);
		m_eStimID = g_pAIStimulusMgr->RegisterStimulus(scs);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::UnregisterStimulus
//
//	PURPOSE:	Unregister the stimulus.
//
// ----------------------------------------------------------------------- //

void WeaponItem::UnregisterStimulus()
{
	if (g_pAIStimulusMgr)
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eStimID );
		m_eStimID = kStimID_Invalid;
	}
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

	SAVE_HRECORD( m_hAmmo );
	SAVE_INT(m_nAmmo);
	SAVE_INT(m_eStimID);
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

	LOAD_HRECORD( m_hAmmo , g_pWeaponDB->GetAmmoCategory() );
	LOAD_INT(m_nAmmo);
	LOAD_INT_CAST(m_eStimID, EnumAIStimulusID);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	WeaponItem::AddMuzzleSmoke
//
//  PURPOSE:	Adds a muzzle smoke clientfx.
//
// ----------------------------------------------------------------------- //

void WeaponItem::AddMuzzleSmoke( )
{
	// Get our muzzle smoke fx.
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hRecord, !USE_AI_DATA);
	char const* pszMuzzleSmokeFx = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sMuzzleSmokeFX );
	if( pszMuzzleSmokeFx && pszMuzzleSmokeFx[0] )
	{
		SetDroppedClientFX( pszMuzzleSmokeFx );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CWeaponItemPlugin::PreHook_EditStringList(const char* szRezPath, 
												   const char* szPropName, 
												   char** aszStrings, 
												   uint32* pcStrings, 
												   const uint32 cMaxStrings, 
												   const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if( LTStrIEquals( "WeaponType", szPropName ) )
	{
		g_pWeaponDBPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		g_pWeaponDBPlugin->PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength,true);

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
										 const char* szPropName, 
										 const char* szPropValue,
										 char* szModelFilenameBuf,
										 int nModelFilenameBufLen,
										 LTVector & vDims,
										 const char* pszObjName, 
										 ILTPreInterface *pInterface)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1 || !g_pWeaponDB )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	// Remove the , that is put into some weapon names.
	char szModifiedPropValue[256];
	LTStrCpy( szModifiedPropValue, szPropValue, LTARRAYSIZE(szModifiedPropValue) );
	strtok( szModifiedPropValue, "," );

	HATTRIBUTE hAttrib = NULL;
	HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( szModifiedPropValue );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	HATTRIBUTE hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_RightHandWeapon );

	hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
	const char *pszHHModel = g_pWeaponDB->GetString( hAttrib );
	if( !pszHHModel[0] )
	{
		// Check the left model...
		hWeaponModelStruct = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_LeftHandWeapon );
		
		hAttrib = g_pWeaponDB->GetStructAttribute( hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
		pszHHModel = g_pWeaponDB->GetString( hAttrib );
		if( !pszHHModel[0] )
		{
			return LT_UNSUPPORTED;
		}
	}

	LTStrCpy( szModelFilenameBuf, pszHHModel, nModelFilenameBufLen );

	// Need to convert the .ltb filename to one that WorldEdit understands...
	
	//ConvertLTBFilename( szModelFilenameBuf );

	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::GetCollisionProperty
//
//	PURPOSE:	get collision property for dropped items
//
// ----------------------------------------------------------------------- //

HRECORD WeaponItem::GetCollisionProperty()
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hRecord, !USE_AI_DATA);
	return g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rCollisionProperty );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void WeaponItem::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// Get the weapon and ammo database records...
	HWEAPON hWeapon = NULL;
	char pszWeapon[256] = {'\0'};
	pInterface->GetPropString(pszObjectName, "WeaponType", pszWeapon, LTARRAYSIZE(pszWeapon), "");
	if (!LTStrEmpty(pszWeapon))
	{
		hWeapon = g_pWeaponDB->GetWeaponRecord(pszWeapon);
	}

	HAMMO hAmmo = NULL;
	char pszAmmo[256]	= {'\0'};
	pInterface->GetPropString(pszObjectName, "AmmoName", pszAmmo, LTARRAYSIZE(pszAmmo), "");
	if(!LTStrEmpty(pszAmmo))
	{
		hAmmo = g_pWeaponDB->GetAmmoRecord(pszAmmo);
	}

	// prefetch the resources
	if (hWeapon)
	{
		GetWeaponResources(Resources, hWeapon, hAmmo);
	}
}
