// ----------------------------------------------------------------------- //
//
// MODULE  : ItemPickups.cpp
//
// PURPOSE : Blood2 inventory item pickups - implementation
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#include "ItemPickups.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"
#include "generic_msg_de.h"
#include "ClientServerShared.h"


// *********************************************************************** //
//
//	CLASS:		ItemPickup
//
//	PURPOSE:	Base Item pickup item
//
// *********************************************************************** //


BEGIN_CLASS(ItemPickup)
END_CLASS_DEFAULT(ItemPickup, PickupObject, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ItemPickup::ObjectTouch
//
//	PURPOSE:	handles an object touch
//
// ----------------------------------------------------------------------- //

void ItemPickup::ObjectTouch(HOBJECT hObject)
{
	DBYTE nValue;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Always at least one object...
	nValue = ( DBYTE )DCLAMP( m_fValue * m_fValueMult, 0, 255 );
	if( nValue == 0 && m_fValueMult > 0.0f )
		nValue = 1;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_INVENTORYITEMTOUCH);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE)m_nType);
	pServerDE->WriteToMessageByte( hMessage, nValue );
	pServerDE->EndMessage(hMessage);
}



// *********************************************************************** //
//
//	CLASS:		ItemPickup
//
//	PURPOSE:	Base Item pickup item
//
// *********************************************************************** //


BEGIN_CLASS(ItemPickupCharged)
END_CLASS_DEFAULT_FLAGS(ItemPickupCharged, ItemPickup, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ItemPickupCharged::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD ItemPickupCharged::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nPowerupsLevel );
			}
			break;
		}
	}

	return ItemPickup::EngineMessageFn(messageID, pData, fData);
}

// *********************************************************************** //
//
//	CLASS:		BinocularsPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(BinocularsPU)
END_CLASS_DEFAULT(BinocularsPU, ItemPickupCharged, NULL, NULL)

// Constructor
BinocularsPU::BinocularsPU() : ItemPickupCharged()
{
	m_nType		= INV_BINOCULARS;
	m_szFile		= "Binoculars_pu";
	m_szObjectName	= "Binoculars";
	m_nNameID	= IDS_ITEM_BINOCULARS;
}


// *********************************************************************** //
//
//	CLASS:		TheEyePU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(TheEyePU)
END_CLASS_DEFAULT(TheEyePU, ItemPickupCharged, NULL, NULL)

// Constructor
TheEyePU::TheEyePU() : ItemPickupCharged()
{
	m_nType		= INV_THEEYE;
	m_szFile		= "TheEye_pu";
	m_szObjectName	= "The Eye";
	m_nNameID	= IDS_ITEM_EYE;
}

// *********************************************************************** //
//
//	CLASS:		FlashlightPU
//
//	PURPOSE:	Flashlight
//
// *********************************************************************** //

BEGIN_CLASS(FlashlightPU)
END_CLASS_DEFAULT(FlashlightPU, ItemPickupCharged, NULL, NULL)

// Constructor
FlashlightPU::FlashlightPU() : ItemPickupCharged()
{
	m_nType		= INV_FLASHLIGHT;
	m_szFile		= "Flashlight_pu";
	m_szObjectName	= "Flashlight";
	m_nNameID	= IDS_ITEM_FLASHLIGHT;
}


// *********************************************************************** //
//
//	CLASS:		MedKitPU
//
//	PURPOSE:	MedKit pickup
//
// *********************************************************************** //

BEGIN_CLASS(MedKitPU)
END_CLASS_DEFAULT(MedKitPU, ItemPickupCharged, NULL, NULL)

// Constructor
MedKitPU::MedKitPU() : ItemPickupCharged()
{
	m_nType		= INV_MEDKIT;
	m_szFile		= "Medkit_pu";
	m_szObjectName	= "Medkit";
	m_nNameID	= IDS_ITEM_MEDKIT;
}


// *********************************************************************** //
//
//	CLASS:		NightGogglesPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(NightGogglesPU)
END_CLASS_DEFAULT(NightGogglesPU, ItemPickupCharged, NULL, NULL)

// Constructor
NightGogglesPU::NightGogglesPU() : ItemPickupCharged()
{
	m_nType		= INV_NIGHTGOGGLES;
	m_szFile		= "NightGoggles_pu";
	m_szObjectName	= "Night Goggles";
	m_nNameID	= IDS_ITEM_GOGGLES;
}


// *********************************************************************** //
//
//	CLASS:		ItemPickupWeapon
//
//	PURPOSE:	Base weapon-like Item pickup item
//
// *********************************************************************** //


BEGIN_CLASS(ItemPickupWeapon)
END_CLASS_DEFAULT_FLAGS(ItemPickupWeapon, ItemPickup, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ItemPickupWeapon::EngineMessageFn
//
//	PURPOSE:	engine msg handler
//
// ----------------------------------------------------------------------- //

DDWORD ItemPickupWeapon::EngineMessageFn(DDWORD messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{

#ifdef _DEMO
			if (m_nType == INV_PROXIMITY || m_nType == INV_REMOTE || m_nType == INV_TIMEBOMB)
				g_pServerDE->RemoveObject(m_hObject);
			break;
#endif
			if( g_pBloodServerShell->IsMultiplayerGame( ))
			{
				m_fRespawnTime = ( DFLOAT )g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoRespawn;
				if( g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoLevel == LEVEL_NONE )
					g_pServerDE->RemoveObject( m_hObject );
				else
					m_fValueMult = ConvertNetItemMultiplier( g_pBloodServerShell->GetNetGameInfo( )->m_nAmmoLevel );
			}

			DDWORD dwFlags = g_pServerDE->GetObjectUserFlags( m_hObject );
			dwFlags |= USRFLG_GLOW;
			g_pServerDE->SetObjectUserFlags( m_hObject, dwFlags );

			break;
		}
	}

	return ItemPickup::EngineMessageFn(messageID, pData, fData);
}


// *********************************************************************** //
//
//	CLASS:		ProximitiesPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(ProximitiesPU)
END_CLASS_DEFAULT(ProximitiesPU, ItemPickupWeapon, NULL, NULL)

// Constructor
ProximitiesPU::ProximitiesPU() : ItemPickupWeapon()
{
	m_nType		= INV_PROXIMITY;
	m_szFile		= "Proximities_pu";
	m_szObjectName	= "Proximity Bomb";
	m_nNameID	= IDS_ITEM_PROX;
}


// *********************************************************************** //
//
//	CLASS:		RemotesPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(RemotesPU)
END_CLASS_DEFAULT(RemotesPU, ItemPickupWeapon, NULL, NULL)

// Constructor
RemotesPU::RemotesPU() : ItemPickupWeapon()
{
	m_nType		= INV_REMOTE;
	m_szFile		= "Remotes_pu";
	m_szObjectName	= "Remote Bomb";
	m_nNameID	= IDS_ITEM_REMOTE;
}


// *********************************************************************** //
//
//	CLASS:		TimeBombPU
//
//	PURPOSE:	
//
// *********************************************************************** //

BEGIN_CLASS(TimeBombPU)
END_CLASS_DEFAULT(TimeBombPU, ItemPickupWeapon, NULL, NULL)

// Constructor
TimeBombPU::TimeBombPU() : ItemPickupWeapon()
{
	m_nType		= INV_TIMEBOMB;
	m_szFile		= "TimeBomb_pu";
	m_szObjectName	= "Time Bomb";
	m_nNameID	= IDS_ITEM_TIME;
}



// *********************************************************************** //
//
//	CLASS:		KeyPickup
//
//	PURPOSE:	Special level-specific custom item pickups
//
// *********************************************************************** //


BEGIN_CLASS(KeyPickup)
	ADD_STRINGPROP(ObjectName, "SuperKey")		// item name
	ADD_LONGINTPROP(ResourceNum, 0)
	ADD_STRINGPROP(IconFile, "key.pcx")			// File to use for the status bar icon
	ADD_STRINGPROP(HiIconFile, "key_h.pcx")		// File to use for the status bar hilighted icon
	ADD_STRINGPROP(ModelFile, "models\\powerups\\key.abc")	// Model filename for this item
	ADD_STRINGPROP(ModelSkin, "skins\\powerups\\key.dtx")	// Model skin to use
	ADD_LONGINTPROP(UseCount, 1)				// number of times it can be used before removal
END_CLASS_DEFAULT(KeyPickup, PickupObject, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPickup::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD KeyPickup::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = PickupObject::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				GenericProp genProp;

				if (g_pServerDE->GetPropGeneric("ObjectName", &genProp) == DE_OK)
				{
					if (genProp.m_String[0]) m_hstrObjectName = g_pServerDE->CreateString(genProp.m_String);
				}

				if (g_pServerDE->GetPropGeneric("ResourceNum", &genProp) == DE_OK)
					m_nNameID = genProp.m_Long;

/*				if(m_nNameID)
				{
					g_pServerDE->FreeString(m_hstrObjectName);
					m_hstrObjectName = g_pServerDE->FormatString(m_nNameID);
				}
*/
				if (g_pServerDE->GetPropGeneric("IconFile", &genProp) == DE_OK)
				{
					if (genProp.m_String[0]) m_hstrIconFile = g_pServerDE->CreateString(genProp.m_String);
				}

				if (g_pServerDE->GetPropGeneric("HiIconFile", &genProp) == DE_OK)
				{
					if (genProp.m_String[0]) m_hstrIconFileH = g_pServerDE->CreateString(genProp.m_String);
				}

				if (g_pServerDE->GetPropGeneric("ModelFile", &genProp) == DE_OK)
				{
					if (genProp.m_String[0]) m_hstrFilename = g_pServerDE->CreateString(genProp.m_String);
				}

				if (g_pServerDE->GetPropGeneric("ModelSkin", &genProp) == DE_OK)
				{
					if (genProp.m_String[0]) m_hstrSkinName = g_pServerDE->CreateString(genProp.m_String);
				}

				if (g_pServerDE->GetPropGeneric("UseCount", &genProp) == DE_OK)
				{
					m_byUseCount = (DBYTE)DCLAMP(genProp.m_Long, 0, 255);
				}
			}

			if (m_hstrFilename && m_hstrSkinName)
			{
				char *pszTemp;
				pszTemp = g_pServerDE->GetStringData(m_hstrFilename);
				if( pszTemp )
					_mbscpy((unsigned char*)pStruct->m_Filename, (const unsigned char*)pszTemp);
				pszTemp = g_pServerDE->GetStringData(m_hstrSkinName);
				if( pszTemp )
					_mbscpy((unsigned char*)pStruct->m_SkinName, (const unsigned char*)pszTemp);
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
//				DVector vDims;
//				VEC_SET(vDims, 20.0f, 25.0f, 10.0f);

//				pServerDE->SetObjectDims(m_hObject, &vDims);
			}
			break;
		}

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		default : break;
	}

	return PickupObject::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPickup::ObjectTouch
//
//	PURPOSE:	Tell whoever touched us that we are picked up
//
// ----------------------------------------------------------------------- //

void KeyPickup::ObjectTouch(HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_KEYPICKUP);
	pServerDE->WriteToMessageHString(hMessage, m_hstrObjectName);
	pServerDE->WriteToMessageHString(hMessage, m_hstrDisplayName);
	pServerDE->WriteToMessageHString(hMessage, m_hstrIconFile);
	pServerDE->WriteToMessageHString(hMessage, m_hstrIconFileH);
	pServerDE->WriteToMessageByte(hMessage, m_byUseCount);
	pServerDE->EndMessage(hMessage);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPickup::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyPickup::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrFilename);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSkinName);
	pServerDE->WriteToMessageHString(hWrite, m_hstrIconFile);
	pServerDE->WriteToMessageHString(hWrite, m_hstrIconFileH);
	pServerDE->WriteToMessageByte(hWrite, m_byUseCount);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyPickup::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyPickup::Load(HMESSAGEREAD hRead, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrFilename  = pServerDE->ReadFromMessageHString(hRead);
	m_hstrSkinName  = pServerDE->ReadFromMessageHString(hRead);
	m_hstrIconFile  = pServerDE->ReadFromMessageHString(hRead);
	m_hstrIconFileH = pServerDE->ReadFromMessageHString(hRead);
	m_byUseCount	= pServerDE->ReadFromMessageByte(hRead);
}
