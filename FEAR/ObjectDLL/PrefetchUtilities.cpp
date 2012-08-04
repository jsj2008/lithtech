// ----------------------------------------------------------------------- //
//
// MODULE  : PrefetchUtilities.h
//
// PURPOSE : Utilities functions and macros for the prefetch system
//
// CREATED : 03/04/05
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PrefetchUtilities.h"

// prefetching not required for Linux builds
#if !defined(PLATFORM_LINUX)

#include "sys/win/mpstrconv.h"
#include "StringEditMgr.h"
#include "FXDB.h"
#include "WeaponDB.h"
#include "ClientFXDB.h"
#include "AISoundDB.h"
#include "NetDefs.h"
#include "ServerDB.h"
#include "ShatterTypeDB.h"
#include "SlowMoDB.h"
#include "SoundDB.h"
#include "BroadcastDB.h"
#include "CollisionsDB.h"
#include "DialogueDB.h"

std::vector<std::string> g_SurfaceFXTypes;
std::vector<std::string> g_ClientFXHistory;

BEGIN_EXTERNC()

MODULE_EXPORT void GatherResourcesPreObjects(IObjectResourceGatherer* pInterface)
{
}


END_EXTERNC()

//-----------------------------------------------------------------------------
// Resource lists - get a list of all the resources an object needs
//-----------------------------------------------------------------------------

void GetSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt)
{
	if (hParent)
	{
		HATTRIBUTE hSoundListAtt = g_pLTDatabase->GetAttribute(hParent, pszSoundListAtt);
		if (hSoundListAtt)
		{
			// fetch all of the Sounds in the list, as they may be randomly played
			uint32 numSounds = g_pLTDatabase->GetNumValues(hSoundListAtt);
			for (uint32 iSound=0; iSound < numSounds; ++iSound)
			{
				const char* pszSound = g_pLTDatabase->GetString(hSoundListAtt, iSound, "");
				if(!LTStrEmpty(pszSound))
					Resources.push_back(pszSound);
			}
		}
	}
}

//-----------------------------------------------------------------------------

void GetLocalizedSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt)
{
	if (hParent)
	{
		HATTRIBUTE hSoundListAtt = g_pLTDatabase->GetAttribute(hParent, pszSoundListAtt);
		if (hSoundListAtt)
		{
			// fetch all of the Sounds in the list, as they may be randomly played
			uint32 numSounds = g_pLTDatabase->GetNumValues(hSoundListAtt);
			for (uint32 iSound=0; iSound < numSounds; ++iSound)
			{
				const char* pszSoundID = g_pLTDatabase->GetString(hSoundListAtt, iSound, "");
				if (!LTStrEmpty(pszSoundID))
				{
					const char* szSound = g_pLTIStringEdit->GetVoicePath( g_pLTDBStringEdit, pszSoundID );
					if (!LTStrEmpty(szSound))
					{
						Resources.push_back(szSound);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void GetModelResources(ResourceList& Resources, HRECORD hModel, const char* pszModelAtt)
{
	if (hModel)
	{
		const char* pFilename = g_pLTDatabase->GetString(g_pLTDatabase->GetAttribute(hModel, pszModelAtt), 0, "");
		if (!LTStrEmpty(pFilename))
			Resources.push_back(pFilename);
	}
}

//-----------------------------------------------------------------------------

void GetMaterialListResources(ResourceList& Resources, HRECORD hModel, const char* pszMaterialListAtt)
{
	if (hModel)
	{
		uint32 numMaterials = g_pLTDatabase->GetNumValues(g_pLTDatabase->GetAttribute(hModel, pszMaterialListAtt));
		for (uint32 iMaterial=0; iMaterial < numMaterials; ++iMaterial)
		{
			char const* pszMaterial = g_pLTDatabase->GetString(g_pLTDatabase->GetAttribute(hModel, pszMaterialListAtt), iMaterial, "");
			if (!LTStrEmpty(pszMaterial))
				Resources.push_back(pszMaterial);
		}
	}
}

//-----------------------------------------------------------------------------

void GetClientFXResources(ResourceList& Resources, const char* pszClientFX)
{
	if (!LTStrEmpty(pszClientFX))
	{
		if (find(g_ClientFXHistory.begin(), g_ClientFXHistory.end(), pszClientFX) == g_ClientFXHistory.end())
		{
			CClientFXDB::GetSingleton().GetFxResources(pszClientFX, (CClientFXDB::TFxResourceList&)Resources);
			g_ClientFXHistory.push_back(pszClientFX);
		}
	}
}

//-----------------------------------------------------------------------------

void GetClientFXResources(ResourceList& Resources, HRECORD hParent, const char* pszClientFXAtt)
{
	const char* pszClientFX = g_pLTDatabase->GetString(g_pLTDatabase->GetAttribute(hParent, pszClientFXAtt), 0, "");
	if (!LTStrEmpty(pszClientFX))
	{
		GetClientFXResources(Resources, pszClientFX);
	}
}

//-----------------------------------------------------------------------------

void GetCollisionPropertyResources(ResourceList& Resources, HRECORD hCollisionProp)
{
	if(hCollisionProp)
	{
		HATTRIBUTE hResponses = g_pWeaponDB->GetAttribute(hCollisionProp,"Responses");
		if(hResponses)
		{
			uint32 numResponses = g_pWeaponDB->GetNumValues(hResponses);
			for (uint32 nResponseIndex=0; nResponseIndex < numResponses; ++nResponseIndex)
			{
				DatabaseItem ResponseItem(hResponses,nResponseIndex);

				HATTRIBUTE hSoundAtt = ResponseItem.GetAttribute("SoundRecord");
				if (hSoundAtt)
				{
					uint32 numSounds = g_pLTDatabase->GetNumValues(hSoundAtt);
					for (uint32 nSoundIndex=0; nSoundIndex < numSounds; ++nSoundIndex)
					{
						DatabaseItem SoundItem(hSoundAtt,nSoundIndex);
						HATTRIBUTE hSoundType = SoundItem.GetAttribute("Sound");
						if(hSoundType)
						{
							GetMaterialListResources(Resources, g_pWeaponDB->GetRecordLink(hSoundType));
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void GetWeaponListResources(ResourceList& Resources, HRECORD hParent, const char* pszWeaponListAtt)
{
	HATTRIBUTE hWeaponListAtt = g_pLTDatabase->GetAttribute(hParent, pszWeaponListAtt);
	uint32 nWeaponCount = g_pLTDatabase->GetNumValues(hWeaponListAtt);
	for( uint32 iWeapon = 0; iWeapon < nWeaponCount; ++iWeapon )
	{
		GetWeaponResources(Resources, g_pLTDatabase->GetRecordLink(hWeaponListAtt, iWeapon, NULL));
	}
}

//-----------------------------------------------------------------------------

void GetWeaponResources(ResourceList& Resources, HWEAPON hWeapon, HAMMO hAmmo)
{
	// recursively gather the weapon and ammo records
	GetRecordResources(Resources, hWeapon, true);
	GetRecordResources(Resources, hAmmo, true);

	// get the pickup item respawn and pickup sounds
	HRECORD hRespawnSound = g_pSoundDB->GetSoundDBRecord("WeaponItemRespawn");
	GetRecordResources(Resources, hRespawnSound, true);
	HRECORD hPickupSound = g_pSoundDB->GetSoundDBRecord("WeaponItemPickup");
	GetRecordResources(Resources, hPickupSound, true);

	// store the SurfaceFX so we can use when we do the final surface processing 
	HATTRIBUTE hSurfaceFxTypeAtt = g_pLTDatabase->GetAttribute(hAmmo, WDB_AMMO_sSurfaceFXType);
	const char* pszSurfaceType = g_pLTDatabase->GetString(hSurfaceFxTypeAtt, 0, NULL);
	if (pszSurfaceType)
	{
		g_SurfaceFXTypes.push_back(pszSurfaceType);
	}
}

//-----------------------------------------------------------------------------

void GetAmmoResources(ResourceList& Resources, HRECORD hAmmo)
{
	// Ammo Effect resources

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo, !USE_AI_DATA);

	GetClientFXResources(Resources,hAmmoData,WDB_AMMO_sProjectileFX);

	HRECORD hFireFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sFireFX );
	Resources.push_back(g_pFXDB->GetString(hFireFX,FXDB_sShellModel));
	Resources.push_back(g_pFXDB->GetString(hFireFX,FXDB_sShellMaterial));

	HRECORD hTracerFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sTracerFX );
	GetClientFXResources(Resources, hTracerFX, FXDB_sFXName);

	//TODO: SurfaceFX - query all surfaces which are in the level, prefetch fx for each surface that is used in WeaponFX::CreateSurfaceSpecificFX().
	const char *pszSurfaceFXType = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sSurfaceFXType );
	if( pszSurfaceFXType[0] && !LTStrIEquals( pszSurfaceFXType, "None" ))
	{
		// keep track of this surface FX type for when we process the surfaces (in the post-process hook)
		g_SurfaceFXTypes.push_back(pszSurfaceFXType);
	}
}

//-----------------------------------------------------------------------------

void GetCharacterModelResources(ResourceList& Resources, HRECORD hModel)
{
	if(!hModel)
		return;

	// Model
	GetModelResources(Resources, hModel);

	// Materials
	GetMaterialListResources(Resources, hModel);

	// Sound template
	HRECORD hSoundTemplate = g_pAISoundDB->GetRecordLink(hModel,"SoundTemplate");
	if (hSoundTemplate)
	{
		// loop through all the AI sounds types in the sound template
		for (uint32 iSoundType=kAIS_InvalidType+1; iSoundType < kAIS_Count; ++iSoundType)
		{
			// for each sound type, there is a list of sound lists
			HATTRIBUTE hSoundTypeAtt = g_pAISoundDB->GetAttribute(hSoundTemplate, s_aszAISoundTypes[iSoundType]);
			if (hSoundTypeAtt)
			{
				// loop through each sound list
				uint32 nSoundSetCount = g_pAISoundDB->GetNumValues(hSoundTypeAtt);
				for(uint32 iSoundSet=0; iSoundSet < nSoundSetCount; ++iSoundSet)
				{
					// prefetch a single sound list
					GetLocalizedSoundListResources(Resources, g_pAISoundDB->GetRecordLink(hSoundTypeAtt,iSoundSet));
				}
			}
		}
	}

	// Dropped items
	HRECORD hDroppedItems = g_pLTDatabase->GetRecordLink(g_pLTDatabase->GetAttribute(hModel, AIDB_ATTRIBUTES_sDroppedItems),0,NULL);
	if (hDroppedItems)
	{
		HATTRIBUTE hItems = g_pLTDatabase->GetAttribute(hDroppedItems,AIDB_DROPPEDITEMS_Items);
		if (hItems)
		{
			uint32 nItemCount = g_pLTDatabase->GetNumValues(hItems);
			for (uint32 iItem=0; iItem < nItemCount; ++iItem)
			{
				HATTRIBUTE hGearAtt = g_pAIDB->GetStructAttribute(hItems,iItem,AIDB_DROPPEDITEMS_rGearItem);
				HGEAR hGear = g_pLTDatabase->GetRecordLink(hGearAtt,0,NULL);
				if (hGear)
				{
					GetModelResources(Resources,hGear, "Model");
					GetMaterialListResources(Resources, hGear, "Material");
				}

				HATTRIBUTE hWeaponAtt = g_pAIDB->GetStructAttribute(hItems,iItem,AIDB_DROPPEDITEMS_rWeaponItem);
				HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hWeaponAtt,0,NULL);
				if (hWeapon)
				{
					GetWeaponResources(Resources, hWeapon);
				}
			}
		}
	}

	// Loud / Quiet movement sounds
	GetMaterialListResources(Resources, g_pModelsDB->GetModelLoudMovementSnd(hModel));
	GetMaterialListResources(Resources, g_pModelsDB->GetModelQuietMovementSnd(hModel));
	GetMaterialListResources(Resources, g_pModelsDB->GetModelDeathGibSnd(hModel));
	GetMaterialListResources(Resources, g_pModelsDB->GetModelDeathDecapitateSnd(hModel));

	// Flesh and Armor surface type resources

	// TODO: cache 'relevant' weaponFX and impacts

	// Collision property
	GetCollisionPropertyResources(Resources, g_pModelsDB->GetCollisionProperty(hModel));

	// Hands Material
	GetMaterialListResources(Resources, hModel, "HandsMaterial");

	// Alt Head Material
	GetMaterialListResources(Resources, hModel, "AltHeadMaterial");

	// Alt Body Material
	GetMaterialListResources(Resources, hModel, "AltBodyMaterial");

	// Required Weapon
	GetWeaponListResources(Resources, hModel, "RequiredWeapon");

	// Player view attachment
	uint32 nPVAttachmentCount = g_pModelsDB->GetNumPlayerViewAttachments( hModel );
	for( uint32 iPVAttach = 0; iPVAttach < nPVAttachmentCount; ++iPVAttach )
	{
		HRECORD hPVAttachment = g_pLTDatabase->GetRecordLink(g_pLTDatabase->GetAttribute(hModel, "PlayerViewAttachment"), iPVAttach, NULL);
		if (hPVAttachment)
		{
			GetModelResources(Resources, hPVAttachment, "Model");
			GetMaterialListResources(Resources, hPVAttachment, "Materials");
		}
	}

	// Body severing
	HRECORD hSeverBody = g_pModelsDB->GetSeverBodyRecord(hModel);
	if (hSeverBody)
	{
		GetModelResources(Resources, hSeverBody);
		GetMaterialListResources(Resources, hSeverBody);

		// Sever piece list
		HATTRIBUTE hSeverPieces = g_pLTDatabase->GetAttribute(hSeverBody, "Piece");
		if (hSeverPieces)
		{
			uint32 nSeverPieceCount = g_pLTDatabase->GetNumValues(hSeverPieces);
			for (uint32 iSeverPiece=0; iSeverPiece < nSeverPieceCount; ++iSeverPiece)
			{
				HRECORD hPiece = g_pLTDatabase->GetRecordLink(hSeverPieces, iSeverPiece, NULL);
				if (hPiece)
				{
					// Sever piece
					GetModelResources(Resources, hPiece);
					GetMaterialListResources(Resources, hPiece);
					GetClientFXResources(Resources, hPiece, "BodyFX");
					GetClientFXResources(Resources, hPiece, "PartFX");
				}
			}
		}
	}

	// Gib FX
	GetClientFXResources(Resources, hModel, "GibFX");

	// Interface FX
	GetClientFXResources(Resources, hModel, "InterfaceFX");

	// Persistent Client FX
	uint32 nPersistenClientFXCount = g_pModelsDB->GetNumPersistentClientFX(hModel);
	for (uint32 iClientFX=0; iClientFX < nPersistenClientFXCount; ++iClientFX)
	{
		GetClientFXResources(Resources, g_pModelsDB->GetPersistentClientFXName(hModel, iClientFX));
	}

	// Initial Movement Sound
	GetMaterialListResources(Resources, g_pModelsDB->GetModelInitialMovementSoundRecord(hModel));
}

//-----------------------------------------------------------------------------

void GetCharacterInventoryResources(ResourceList& Resources, HRECORD hModel, bool bUseDefaultWeapons, bool bUseDefaultAttachments)
{
	if(!hModel)
		return;

	if( bUseDefaultAttachments )
	{
		uint32 nAttachmentCount = g_pLTDatabase->GetNumValues(g_pLTDatabase->GetAttribute(hModel, "DefaultAttachment"));
		for( uint32 iAttachment = 0; iAttachment < nAttachmentCount; ++iAttachment )
		{
			HRECORD hAttachment = g_pLTDatabase->GetRecordLink(g_pLTDatabase->GetAttribute(hModel, "DefaultAttachment"), iAttachment, NULL);
			if (hAttachment)
			{
				GetModelResources(Resources,hAttachment, "Model");
				GetMaterialListResources(Resources, hAttachment, "Materials");
			}
		}
	}

	if (bUseDefaultWeapons)
	{
		GetWeaponListResources(Resources, hModel, "DefaultWeapon");
	}
}

//-----------------------------------------------------------------------------

void GetPlayerResources(ResourceList& Resources)
{
	HRECORD hModel = g_pModelsDB->GetModelByRecordName(DEFAULT_PLAYERNAME);
	if (hModel)
	{
		//GetCharacterModelResources(Resources, hModel);
		GetRecordResources(Resources, hModel, true);
		GetPlayerInventoryResources(Resources);
	}
}

//-----------------------------------------------------------------------------

void GetPlayerInventoryResources(ResourceList& Resources)
{
	// handle default player inventory
	HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
	HATTRIBUTE hDefsAtt = g_pLTDatabase->GetAttribute(hGlobalRec,WDB_GLOBAL_rDefaultWeapons);
	uint32 nNumDefs = g_pLTDatabase->GetNumValues(hDefsAtt);

	//step through the list of weapons
	for (uint32 n = 0; n < nNumDefs; ++n)
	{
		//!! refactor all this into one bool function CPlayerObj::IsValidDefaultWeapon(hWeapon)

		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hDefsAtt,n,NULL);
		if( !hWeapon )
			continue;

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}

		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmoData || g_pWeaponDB->IsRestricted( hAmmoData ))
		{
			continue;
		}

		//GetWeaponResources(Resources, hWeapon, hAmmo);
		GetRecordResources(Resources, hWeapon, true);
		GetRecordResources(Resources, hAmmo, true);
	}
}

//-----------------------------------------------------------------------------

void GetRecordResources(ResourceList& Resources, HRECORD hStartRecord, bool bRecurseRecordLinks)
{
	//make sure we have a valid record
	if(!hStartRecord)
		return;

	//maintain a stack for traversal so that we can recurse without fear of stack overflow
	//and also detect and avoid infinite recursion
	static std::vector<HRECORD> RecordStack;
	RecordStack.push_back(hStartRecord);
	
	// history stack to keep track of where we've been
	static std::vector<HRECORD> HistoryStack;

	while(!RecordStack.empty())
	{
		HRECORD hRecord = RecordStack.back();	
		RecordStack.pop_back();

		// see if we've been here before
		if(std::find(HistoryStack.begin(), HistoryStack.end(), hRecord) != HistoryStack.end())
		{
			continue;
		}

		HistoryStack.push_back(hRecord);

		const char* pszRecordName = g_pLTDatabase->GetRecordName(hRecord);

		//now run through each attribute and determine which ones are resources, and which ones
		//are record links that we need to optionally traverse into
		uint32 nNumAttributes = g_pLTDatabase->GetNumAttributes(hRecord);
		for(uint32 nCurrAttrib = 0; nCurrAttrib < nNumAttributes; nCurrAttrib++)
		{
			//get this attribute
			HATTRIBUTE hCurrAttrib = g_pLTDatabase->GetAttributeByIndex(hRecord, nCurrAttrib);

			//determine if this is an attribute that we need to gather resources for
			EAttributeType eType	= g_pLTDatabase->GetAttributeType(hCurrAttrib);
			EAttributeUsage eUsage	= g_pLTDatabase->GetAttributeUsage(hCurrAttrib);
			const char* pszName		= g_pLTDatabase->GetAttributeName(hCurrAttrib);

			//get value information
			uint32 nNumValues = g_pLTDatabase->GetNumValues(hCurrAttrib);

			//handle adding filenames to our list
			if((eType == eAttributeType_String) && (eUsage == eAttributeUsage_Filename))
			{
				//this attribute is a resource, so add each value to the list
				for(uint32 nCurrValue = 0; nCurrValue < nNumValues; nCurrValue++)
				{
					const char* pszValue = g_pLTDatabase->GetString(hCurrAttrib, nCurrValue, ""); 
					Resources.push_back(pszValue);
				}			
			}

			//handle adding ClientFX to our list
			if((eType == eAttributeType_String) && (eUsage == eAttributeUsage_ClientFX))
			{
				//this attribute is a ClientFX, so for each fx we need to determine the used resources
				for(uint32 nCurrValue = 0; nCurrValue < nNumValues; nCurrValue++)
				{
					const char* pszValue = g_pLTDatabase->GetString(hCurrAttrib, nCurrValue, ""); 
					GetClientFXResources(Resources, pszValue);
				}		
			}

			//handle traversing into record links
			if(bRecurseRecordLinks && (eType == eAttributeType_RecordLink))
			{
				for(uint32 nCurrValue = 0; nCurrValue < nNumValues; nCurrValue++)
				{
					HRECORD hLinkedRecord = g_pLTDatabase->GetRecordLink(hCurrAttrib, nCurrValue, NULL);

					//handle invalid record links or self record links (self links cause infinite
					//recursion as this isn't on the stack and won't be detected)
					if(!hLinkedRecord || (hLinkedRecord == hRecord))
						continue;

					//see if this record is already on the stack
					if(std::find(RecordStack.begin(), RecordStack.end(), hLinkedRecord) == RecordStack.end())
					{
						//not in the list, so add it so that we will process it next
						RecordStack.push_back(hLinkedRecord);
					}
				}
			}
		}
	}

	HistoryStack.clear();
}

//-----------------------------------------------------------------------------
// Resource Placement - determines which streaming region to place the resources
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// helpers
void AddResourcesToObjectGathererRegion(IObjectResourceGatherer* pInterface, const char* pszRegion, const ResourceList& cResources)
{
	for (ResourceList::const_iterator itResource = cResources.begin(); itResource != cResources.end(); ++itResource)
	{
		pInterface->AddResourceToRegion((*itResource).c_str(), pszRegion);
	}
}

void AddResourcesToObjectGathererAABB(IObjectResourceGatherer* pInterface, const LTRect3f& rAABB, const ResourceList& cResources)
{
	for (ResourceList::const_iterator itResource = cResources.begin(); itResource != cResources.end(); ++itResource)
	{
		pInterface->AddResourceToAABB((*itResource).c_str(), rAABB);
	}
}

void AddPropStringToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName)
{
	char szPropString[MAX_PATH];
	pInterface->GetPropString(pszObjectName, pszPropName, szPropString, LTARRAYSIZE(szPropString), "");
	if (!LTStrEmpty(szPropString))
	{
		cResourceList.push_back(szPropString);
	}
}

void AddSoundResourceToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName)
{
	// get the value from the property
	char szPropString[MAX_PATH];
	pInterface->GetPropString(pszObjectName, pszPropName, szPropString, LTARRAYSIZE(szPropString), "");
	if (!LTStrEmpty(szPropString))
	{
		// try to locate a sound database record with this value
		HRECORD hSoundDBRecord = g_pSoundDB->GetSoundDBRecord(szPropString);
		if (hSoundDBRecord)
		{
			// we found a record, so gather all of the resources
			GetRecordResources(cResourceList, hSoundDBRecord, false);
		}
		else
		{
			// it's a filename
			cResourceList.push_back(szPropString);
		}
	}
}


//-----------------------------------------------------------------------------

void PlaceResourcesDefault(const char* pszObjectName, IObjectResourceGatherer* pInterface, const ResourceList& Resources )
{
	//prefetch based on the properties of an associated object resource area...

	if(pInterface->ContainedByResourceArea(pszObjectName))
	{
		if(pInterface->GetResourceAreaIsGlobal(pszObjectName))
		{
			AddResourcesToObjectGathererRegion(pInterface, NULL, Resources);
			return;
		}
		else if(pInterface->ResourceAreaHasDimensions(pszObjectName))
		{
			LTRect3f rAABB;
			pInterface->GetResourceAreaDimensions(pszObjectName, rAABB);
			AddResourcesToObjectGathererAABB(pInterface, rAABB, Resources);
			return;
		}
		else
		{
			uint32 nRegions = pInterface->GetNumResourceAreaRegions(pszObjectName);
			if (nRegions > 0)
			{
				for(uint32 RegionIndex=0; RegionIndex < nRegions; ++RegionIndex)
				{
					const char* pszRegion = pInterface->GetResourceAreaRegionName(pszObjectName, RegionIndex);
					if(!LTStrEmpty(pszRegion))
					{
						for (ResourceList::const_iterator itResource = Resources.begin(); itResource != Resources.end(); ++itResource)
						{
							pInterface->AddResourceToRegion(pszObjectName, pszRegion);
						}
					}
				}
			}
			else
			{
				pInterface->WarningMsg("Bad resource area - please reconfigure '%s'", pInterface->GetResourceAreaName(pszObjectName));
			}
			return;
		}
	}
	else
	{
		// prefetch based on the  model props.  NOTE: FEAR objects are always global, DARK looks at the
		// object props to determine if it is global or region based.
		bool bGlobal = true;

#if defined(PROJECT_DARK)
		bGlobal = pInterface->GetPropBool(pszObjectName, "GlobalResources", false);
#endif

		if(bGlobal)
		{
			// object set up to have global resources
			//pInterface->AddWorldModelToRegion(pszObjectName, NULL);
			AddResourcesToObjectGathererRegion(pInterface, NULL, Resources);
			return;
		}
		else
		{
			// set up resources based on object location
			// Possible improvement - use object extents as well
			LTRigidTransform tObjTrans = pInterface->GetTransform(pszObjectName);
			LTRect3f rAABB(tObjTrans.m_vPos,tObjTrans.m_vPos);
			AddResourcesToObjectGathererAABB(pInterface, rAABB, Resources);
			return;
		}
	}

	// we've fallen through - something idn't right.
	pInterface->ErrorMsg("Unknown prefetch error processing '%s'", pszObjectName);
}

//-----------------------------------------------------------------------------
// Prefetching
//-----------------------------------------------------------------------------

void PrefetchPlayer(const char* pszObjectName, IObjectResourceGatherer* pInterface)
{
	// ignore whatever game object name is prefetching the resources
	// on behalf of the player

	ResourceList Resources;
	GetPlayerResources(Resources);
	AddResourcesToObjectGathererRegion(pInterface, NULL, Resources);
}

//-----------------------------------------------------------------------------

void PrefetchWorldModel(const char* pszObjectName, IObjectResourceGatherer* pInterface)
{
	//prefetch based on the properties of an associated object resource area...

	if(pInterface->ContainedByResourceArea(pszObjectName))
	{
		// resource are needed globally
		if(pInterface->GetResourceAreaIsGlobal(pszObjectName))
		{
			pInterface->AddWorldModelToRegion(pszObjectName, NULL);
			return;
		}
		// resources need within the dimensions of the resource area object
		else if(pInterface->ResourceAreaHasDimensions(pszObjectName))
		{
			LTRect3f rAABB;
			pInterface->GetResourceAreaDimensions(pszObjectName, rAABB);
			pInterface->AddWorldModelToAABB(pszObjectName, rAABB);
			return;
		}
		// resources needed in named streaming regions
		else
		{
			uint32 nRegions = pInterface->GetNumResourceAreaRegions(pszObjectName);
			if(nRegions > 0 )
			{
				for(uint32 RegionIndex=0; RegionIndex < nRegions; ++RegionIndex)
				{
					const char* pszRegion = pInterface->GetResourceAreaRegionName(pszObjectName, RegionIndex);
					pInterface->AddWorldModelToRegion(pszObjectName, pszRegion);
				}
				return;
			}
			else
			{
				pInterface->WarningMsg("Bad resource area - please reconfigure '%s'", pInterface->GetResourceAreaName(pszObjectName));
				return;
			}
		}
	}

	// handle the collision property
	char szCollisionProperty[MAX_PATH];
	if (pInterface->GetPropString(pszObjectName, "CollisionProperty", szCollisionProperty, MAX_PATH, NULL))
	{
		HRECORD hCollisionProperty = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( CollisionProperty ).GetCategory(), szCollisionProperty );
		
		if (hCollisionProperty)
		{
			ResourceList cCollisionResources;
			GetRecordResources(cCollisionResources, hCollisionProperty, true);

			for (ResourceList::iterator itResource = cCollisionResources.begin(); itResource != cCollisionResources.end(); ++itResource)
			{
				pInterface->AddResourceToRegion((*itResource).c_str(), NULL);
			}
		}
	}


	// determine if we have any associated shatter resources
	char szShatterTypeBuffer[MAX_PATH];
	if (pInterface->GetPropString(pszObjectName, "ShatterType", szShatterTypeBuffer, MAX_PATH, NULL))
	{
		HSHATTERTYPE hShatterType = CShatterTypeDB::Instance().GetShatterType(szShatterTypeBuffer);
		if (hShatterType)
		{
			ResourceList cShatterResources;
			GetRecordResources(cShatterResources, hShatterType, true);

			for (ResourceList::iterator itResource = cShatterResources.begin(); itResource != cShatterResources.end(); ++itResource)
			{
				pInterface->AddResourceToRegion((*itResource).c_str(), NULL);
			}
		}
	}

	// determine if we have any associated destroyed FX resources
	char szDestroyedFXBuffer[MAX_PATH];
	if (pInterface->GetPropString(pszObjectName, "DestroyedFX", szDestroyedFXBuffer, MAX_PATH, NULL))
	{
		ResourceList cDestroyedFXResources;
		GetClientFXResources(cDestroyedFXResources, szDestroyedFXBuffer);

		for (ResourceList::iterator itResource = cDestroyedFXResources.begin(); itResource != cDestroyedFXResources.end(); ++itResource)
		{
			pInterface->AddResourceToRegion((*itResource).c_str(), NULL);
		}
	}
		
	// prefetch based on the world model props.  NOTE: FEAR objects are always global, DARK looks at the
	// object props to determine if it is global or region based.
	bool bGlobal = true;

#if defined(PROJECT_DARK)
	bGlobal = pInterface->GetPropBool(pszObjectName, "GlobalResources", false);
#endif


	if(bGlobal)
	{
		// object set up to have global resources
		pInterface->AddWorldModelToRegion(pszObjectName, NULL);
		return;
	}
	else
	{
		// set up resources based on object location and extents
		LTRect3f rAABB;
		if(pInterface->GetWorldModelAABB(pszObjectName, rAABB))
		{
			pInterface->AddWorldModelToAABB(pszObjectName, rAABB);
			return;
		}
		else
		{
			pInterface->WarningMsg("Prefetch unable to get WorldModel dimensions for object '%s'", pszObjectName);
			return;
		}
	}

	// error
	pInterface->ErrorMsg("Unknown prefetch error processing '%s'", pszObjectName);
}

//-----------------------------------------------------------------------------

// retrieves multiplayer-only resources
void GatherResourcesPostObjects_Multiplayer(IObjectResourceGatherer* pInterface)
{
	// get the multiplayer models
	uint32 nNumberOfMultiModels = g_pModelsDB->GetNumDMModels();
	for (uint32 nIndex = 0; nIndex < nNumberOfMultiModels; ++nIndex)
	{
		ModelsDB::HMODEL hModel = g_pModelsDB->GetDMModel(nIndex);
		ResourceList cModelResources;

		GetRecordResources(cModelResources, hModel, true);
		AddResourcesToObjectGathererRegion(pInterface, NULL, cModelResources);
	}

	// bring in the loadouts
	uint32 nNumberOfLoadouts = g_pWeaponDB->GetNumLoadouts();

	for (uint32 nLoadoutIndex = 0; nLoadoutIndex < nNumberOfLoadouts; ++nLoadoutIndex)
	{
		HRECORD hLoadout = g_pWeaponDB->GetLoadout(nLoadoutIndex);
		if (!hLoadout) 
		{
			continue;
		}

		HATTRIBUTE hWeaponAttribute = g_pLTDatabase->GetAttribute(hLoadout, WDB_LOADOUT_rWeapons);
		uint32 nNumberOfWeapons = g_pLTDatabase->GetNumValues(hWeaponAttribute);

		// step through the list of weapons
		for (uint32 nWeaponIndex = 0; nWeaponIndex < nNumberOfWeapons; ++nWeaponIndex)
		{
			HWEAPON hWeaponLink = g_pLTDatabase->GetRecordLink(hWeaponAttribute, nWeaponIndex, NULL);
			if (!hWeaponLink)
			{
				continue;
			}

			HATTRIBUTE hMultiplayerWeaponAtt = g_pLTDatabase->GetAttribute(hWeaponLink, "MultiPlayer");
			HWEAPONDATA hWeapon = g_pLTDatabase->GetRecordLink(hMultiplayerWeaponAtt, 0, NULL);
			if (!hWeapon)
			{
				HATTRIBUTE hDefaultWeaponAtt = g_pLTDatabase->GetAttribute(hWeaponLink, "Default");
				hWeapon = g_pWeaponDB->GetRecordLink(hDefaultWeaponAtt, 0, NULL);
			}

			if( g_pWeaponDB->IsRestricted( hWeapon ))
			{
				continue;
			}

			ResourceList cWeaponResources;
			GetRecordResources(cWeaponResources, hWeapon, true);
			AddResourcesToObjectGathererRegion(pInterface, NULL, cWeaponResources);

			HATTRIBUTE hAmmoAttribute = g_pLTDatabase->GetAttribute(hWeapon, WDB_WEAPON_rAmmoName);
			uint32 nNumberOfAmmos = g_pLTDatabase->GetNumValues(hAmmoAttribute);

			for (uint32 nAmmoIndex = 0; nAmmoIndex < nNumberOfAmmos; ++nAmmoIndex)
			{
				HRECORD hAmmoLink = g_pLTDatabase->GetRecordLink(hAmmoAttribute, nAmmoIndex, NULL);
				HATTRIBUTE hMultiplayerAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "MultiPlayer");

				HRECORD hAmmo = g_pLTDatabase->GetRecordLink(hMultiplayerAmmoAtt, 0, NULL);
				if (!hAmmo)
				{
					HATTRIBUTE hDefaultAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "Player");
					hAmmo = g_pWeaponDB->GetRecordLink(hDefaultAmmoAtt, 0, NULL);
				}

				// store the surfacefx for later use
				HATTRIBUTE hSurfaceFxTypeAtt = g_pLTDatabase->GetAttribute(hAmmo, WDB_AMMO_sSurfaceFXType);
				g_SurfaceFXTypes.push_back(g_pLTDatabase->GetString(hSurfaceFxTypeAtt, 0, NULL));

				ResourceList cAmmoResources;
				GetRecordResources(cAmmoResources, hAmmo, true);
				AddResourcesToObjectGathererRegion(pInterface, NULL, cAmmoResources);
			}
		}
	}

	// get the player default weapon
	HWEAPON hDefaultWeaponLink = g_pServerDB->GetPlayerDefaultWeapon();
	HATTRIBUTE hMultiplayerWeaponAtt = g_pLTDatabase->GetAttribute(hDefaultWeaponLink, "MultiPlayer");
	HWEAPONDATA hWeapon = g_pLTDatabase->GetRecordLink(hMultiplayerWeaponAtt, 0, NULL);
	if (!hWeapon)
	{
		HATTRIBUTE hDefaultWeaponAtt = g_pLTDatabase->GetAttribute(hDefaultWeaponLink, "Default");
		hWeapon = g_pWeaponDB->GetRecordLink(hDefaultWeaponAtt, 0, NULL);
	}

	ResourceList cWeaponResources;
	GetRecordResources(cWeaponResources, hWeapon, true);
	AddResourcesToObjectGathererRegion(pInterface, NULL, cWeaponResources);

	HATTRIBUTE hAmmoAtt = g_pLTDatabase->GetAttribute(hWeapon, WDB_WEAPON_rAmmoName);
	uint32 nNumAmmos = g_pLTDatabase->GetNumValues(hAmmoAtt);

	for (uint32 nAmmoIndex = 0; nAmmoIndex <nNumAmmos; ++nAmmoIndex)
	{
		HRECORD hAmmoLink = g_pLTDatabase->GetRecordLink(hAmmoAtt, nAmmoIndex, NULL);
		HATTRIBUTE hMultiplayerAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "MultiPlayer");

		HRECORD hAmmo = g_pLTDatabase->GetRecordLink(hMultiplayerAmmoAtt, 0, NULL);
		if (!hAmmo)
		{
			HATTRIBUTE hDefaultAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "Player");
			hAmmo = g_pWeaponDB->GetRecordLink(hDefaultAmmoAtt, 0, NULL);
		}

		// store the surfacefx for later use
		HATTRIBUTE hSurfaceFxTypeAtt = g_pLTDatabase->GetAttribute(hAmmo, WDB_AMMO_sSurfaceFXType);
		g_SurfaceFXTypes.push_back(g_pLTDatabase->GetString(hSurfaceFxTypeAtt, 0, NULL));

		//ResourceList cAmmoResources;
		//GetRecordResources(cAmmoResources, hAmmo, true);
		//AddResourcesToObjectGathererRegion(pInterface, NULL, cAmmoResources);
	}

	// get the multiplayer slowmo resources
	ResourceList cSlowmoResources;

	HRECORD hSlowMoMultiplayer = g_pLTDatabase->GetRecord(DATABASE_CATEGORY(SlowMo).GetCategory(), "Multiplayer");
	GetRecordResources(cSlowmoResources, hSlowMoMultiplayer, true);

	HATTRIBUTE hSlowMoRechargerFXAttribute = g_pLTDatabase->GetAttribute(g_pWeaponDB->GetGlobalRecord(), "SlowMoRechargerFXName");
	const char* pszSlowMoRechargerFXName = g_pLTDatabase->GetString(hSlowMoRechargerFXAttribute, 0, NULL);
	if (pszSlowMoRechargerFXName)
	{
		GetClientFXResources(cSlowmoResources, pszSlowMoRechargerFXName);
	}

	AddResourcesToObjectGathererRegion(pInterface, NULL, cSlowmoResources);

	// multiplayer voice files
	HDATABASE hDatabase = g_pLTDatabase->GetCategoryParent(g_pSoundDB->GetSoundCategory());
	HCATEGORY hBroadcastCategory = g_pLTDatabase->GetCategory(hDatabase, "Sound/Broadcast/Broadcasts");

	uint32 nNumberOfVoiceRecords = g_pLTDatabase->GetNumRecords(hBroadcastCategory);
	for (uint32 nVoiceIndex = 0; nVoiceIndex < nNumberOfVoiceRecords; ++nVoiceIndex)
	{
		HRECORD hVoiceRecord = g_pLTDatabase->GetRecordByIndex(hBroadcastCategory, nVoiceIndex);
		HATTRIBUTE hLineAttribute = g_pLTDatabase->GetAttribute(hVoiceRecord, "Line");
		uint32 nNumberOfLines = g_pLTDatabase->GetNumValues(hLineAttribute);

		for (uint32 nLineIndex = 0; nLineIndex < nNumberOfLines; ++nLineIndex)
		{
			const char* pszSoundID = g_pLTDatabase->GetString(hLineAttribute, nLineIndex, NULL);
			const char* pszVoiceFile = g_pLTIStringEdit->GetVoicePath(g_pLTDBStringEdit, pszSoundID);
			pInterface->AddResourceToRegion(pszVoiceFile, NULL);
		}
	}

	// multiplayer nav markers
	HCATEGORY hNavMarkerCategory = g_pLTDatabase->GetCategory(hDatabase, "Interface/NavMarkerTypes");
	uint32 nNumberOfNavMarkerRecords = g_pLTDatabase->GetNumRecords(hNavMarkerCategory);
	for (uint32 nNavMarkerIndex = 0; nNavMarkerIndex < nNumberOfNavMarkerRecords; ++nNavMarkerIndex)
	{
		ResourceList cNavMarkerResources;
		GetRecordResources(cNavMarkerResources, g_pLTDatabase->GetRecordByIndex(hNavMarkerCategory, nNavMarkerIndex), true);
		AddResourcesToObjectGathererRegion(pInterface, NULL, cNavMarkerResources);
	}

	// death fx
	HCATEGORY hClientSharedCategory = g_pLTDatabase->GetCategory(hDatabase, "Client/Shared");
	HRECORD hClientSharedRecord = g_pLTDatabase->GetRecord(hClientSharedCategory, "Shared");
	HATTRIBUTE hMultiplayerDeathClientFXAttribute = g_pLTDatabase->GetAttribute(hClientSharedRecord, "MultiplayerDeathClientFX");
	const char* pszMultiplayerDeathClientFXName = g_pLTDatabase->GetString(hMultiplayerDeathClientFXAttribute, 0, NULL);
	if (pszMultiplayerDeathClientFXName)
	{
		ResourceList cMultiplayerDeathFXResources;
		GetClientFXResources(cMultiplayerDeathFXResources, pszMultiplayerDeathClientFXName);
		AddResourcesToObjectGathererRegion(pInterface, NULL, cMultiplayerDeathFXResources);
	}

	// dropped gear items
	HATTRIBUTE hDroppedItemsAttribute = g_pLTDatabase->GetAttribute(g_pWeaponDB->GetGlobalRecord(), "DroppedGearItems");
	uint32 nNumberOfDroppedItems = g_pLTDatabase->GetNumValues(hDroppedItemsAttribute);
	for (uint32 nDropIndex = 0; nDropIndex < nNumberOfDroppedItems; ++nDropIndex)
	{
		HRECORD hDroppedItem = g_pLTDatabase->GetRecordLink(hDroppedItemsAttribute, nDropIndex, NULL);
		if (hDroppedItem)
		{
			ResourceList cDroppedItemResourceList;
			GetRecordResources(cDroppedItemResourceList, hDroppedItem, true);
			AddResourcesToObjectGathererRegion(pInterface, NULL, cDroppedItemResourceList);
		}
	}

	// other sounds
	pInterface->AddResourceToRegion("guns\\Snd\\Impacts\\MultiDing.wav", NULL);
	pInterface->AddResourceToRegion("Interface\\Snd\\chat.wav", NULL);
	pInterface->AddResourceToRegion("Interface\\Snd\\RadioOn.wav", NULL);
	pInterface->AddResourceToRegion("Interface\\Snd\\RadioOff.wav", NULL);
}

// retrieves singleplayer-only resources
void GatherResourcesPostObjects_Singleplayer(IObjectResourceGatherer* pInterface)
{
	HDATABASE hDatabase = g_pLTDatabase->GetCategoryParent(g_pSoundDB->GetSoundCategory());

	// get the mission default weapons - we need to remove the suffix
	// look it up in CMissionDB
	std::string strWorldName = pInterface->GetWorldName();
	size_t nExtensionPos = strWorldName.rfind(".");
	strWorldName = strWorldName.substr(0, nExtensionPos);

	uint32 nMissionID = 0;
	uint32 nLevelID = 0;
	if (g_pMissionDB->IsMissionLevel(strWorldName.c_str(), nMissionID, nLevelID))
	{
		HRECORD hMission = g_pMissionDB->GetMission(nMissionID);
		HATTRIBUTE hDefaultWeaponsAttribute = g_pLTDatabase->GetAttribute(hMission, MDB_DefaultWeapons);
		uint32 nNumberOfWeapons = g_pLTDatabase->GetNumValues(hDefaultWeaponsAttribute);
		for (uint32 nWeaponIndex = 0; nWeaponIndex < nNumberOfWeapons; ++nWeaponIndex)
		{
			HRECORD hWeaponLink = g_pLTDatabase->GetRecordLink(hDefaultWeaponsAttribute, nWeaponIndex, NULL);
			HATTRIBUTE hDefaultAttribute = g_pLTDatabase->GetAttribute(hWeaponLink, "Default");
			HRECORD hWeapon = g_pLTDatabase->GetRecordLink(hDefaultAttribute, 0, NULL);
			if (!hWeapon)
			{
				continue;
			}

			// get the resources for this weapon
			ResourceList cWeaponResources;
			GetRecordResources(cWeaponResources, hWeapon, true);
			AddResourcesToObjectGathererRegion(pInterface, NULL, cWeaponResources);

			// get the surface FX type from the ammo records
			HATTRIBUTE hAmmoAttribute = g_pLTDatabase->GetAttribute(hWeapon, WDB_WEAPON_rAmmoName);
			uint32 nNumberOfAmmos = g_pLTDatabase->GetNumValues(hAmmoAttribute);

			for (uint32 nAmmoIndex = 0; nAmmoIndex < nNumberOfAmmos; ++nAmmoIndex)
			{
				HRECORD hAmmoLink = g_pLTDatabase->GetRecordLink(hAmmoAttribute, nAmmoIndex, NULL);
				HATTRIBUTE hMultiplayerAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "MultiPlayer");

				HRECORD hAmmo = g_pLTDatabase->GetRecordLink(hMultiplayerAmmoAtt, 0, NULL);
				if (!hAmmo)
				{
					HATTRIBUTE hDefaultAmmoAtt = g_pLTDatabase->GetAttribute(hAmmoLink, "Player");
					hAmmo = g_pWeaponDB->GetRecordLink(hDefaultAmmoAtt, 0, NULL);
				}

				// store the surfacefx for later use
				HATTRIBUTE hSurfaceFxTypeAtt = g_pLTDatabase->GetAttribute(hAmmo, WDB_AMMO_sSurfaceFXType);
				g_SurfaceFXTypes.push_back(g_pLTDatabase->GetString(hSurfaceFxTypeAtt, 0, NULL));

				//ResourceList cAmmoResources;
				//GetRecordResources(cAmmoResources, hAmmo, true);
				//AddResourcesToObjectGathererRegion(pInterface, NULL, cAmmoResources);
			}
		}
	}

	// get the singleplayer slowmo resources
	ResourceList cSlowmoResources;

	HRECORD hSlowMoSingleplayer = g_pLTDatabase->GetRecord(DATABASE_CATEGORY(SlowMo).GetCategory(), "Default");
	GetRecordResources(cSlowmoResources, hSlowMoSingleplayer, true);
	AddResourcesToObjectGathererRegion(pInterface, NULL, cSlowmoResources);

	// death fx
	HCATEGORY hClientSharedCategory = g_pLTDatabase->GetCategory(hDatabase, "Client/Shared");
	HRECORD hClientSharedRecord = g_pLTDatabase->GetRecord(hClientSharedCategory, "Shared");
	HATTRIBUTE hMultiplayerDeathClientFXAttribute = g_pLTDatabase->GetAttribute(hClientSharedRecord, "MultiplayerDeathClientFX");
	const char* pszMultiplayerDeathClientFXName = g_pLTDatabase->GetString(hMultiplayerDeathClientFXAttribute, 0, NULL);
	if (pszMultiplayerDeathClientFXName)
	{
		ResourceList cMultiplayerDeathFXResources;
		GetClientFXResources(cMultiplayerDeathFXResources, pszMultiplayerDeathClientFXName);
		AddResourcesToObjectGathererRegion(pInterface, NULL, cMultiplayerDeathFXResources);
	}

	// HUD transmissions
	uint32 nNumberOfHUDTransmission = DATABASE_CATEGORY(Dialogue).GetNumRecords();
	for (uint32 nIndex = 0; nIndex < nNumberOfHUDTransmission; ++nIndex)
	{		
		HRECORD hTransmissionRecord = DATABASE_CATEGORY(Dialogue).GetRecordByIndex(nIndex);
		const char* pszIconTexture = DATABASE_CATEGORY(Dialogue).GETRECORDATTRIB(hTransmissionRecord, IconTexture);
		pInterface->AddResourceToRegion(pszIconTexture, NULL);
	}

	// non-surface related AI sounds
	ResourceList cAISoundResources;

	HRECORD hSoundRecord = g_pSoundDB->GetSoundDBRecord("AI_desk_slide");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("AI_divethru");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("AI_jump");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("AI_land");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("AI_slidecover");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Signal");
	GetRecordResources(cAISoundResources, hSoundRecord, false);

	AddResourcesToObjectGathererRegion(pInterface, NULL, cAISoundResources);
}

// retrieves common resources for both singleplayer and multiplayer
void GatherResourcesPostObjects_Common(IObjectResourceGatherer* pInterface)
{
	// pull in the default model
	pInterface->AddResourceToRegion("Models\\Default.Model00p", NULL);

	// other character sounds
	ResourceList cCharacterSoundResources;

	HRECORD hSoundRecord = g_pSoundDB->GetSoundDBRecord("punchimpact");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("feetland");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("feetjump");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Jump3D");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("JumpLocal");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("EarRingingBuzz");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("melee_01");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("punch_01");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("kick_01");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("slidekick");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Death");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("PlayerRespawn");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("bulletflyby");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("breath");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("HeadShot");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Heartbeat");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Kill");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Team Kill");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("flickerloop");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	hSoundRecord = g_pSoundDB->GetSoundDBRecord("Ladder_slide");
	GetRecordResources(cCharacterSoundResources, hSoundRecord, false);

	AddResourcesToObjectGathererRegion(pInterface, NULL, cCharacterSoundResources);

	// flashlight
	HDATABASE hDatabase = g_pLTDatabase->GetCategoryParent(g_pSoundDB->GetSoundCategory());
	HCATEGORY hFlashlightCategory = g_pLTDatabase->GetCategory(hDatabase, "Client/Flashlight");
	HRECORD hFlashlightRecord = g_pLTDatabase->GetRecord(hFlashlightCategory, "Player_Narrow");
	ResourceList cFlashlightResources;
	GetRecordResources(cFlashlightResources, hFlashlightRecord, true);
	AddResourcesToObjectGathererRegion(pInterface, NULL, cFlashlightResources);

	// proximity sounds - these are specified directly in animation frame strings so it's not possible
	// to automatically pull them in
	pInterface->AddResourceToRegion("guns\\Snd\\Proximity\\arm.wav", NULL);
	pInterface->AddResourceToRegion("guns\\Snd\\Proximity\\armed.wav", NULL);
	pInterface->AddResourceToRegion("guns\\Snd\\Proximity\\activate.wav", NULL);

	// other sounds
	pInterface->AddResourceToRegion("Snd\\Events\\beep01.wav", NULL);
}

BEGIN_EXTERNC()

MODULE_EXPORT void GatherResourcesPostObjects(IObjectResourceGatherer* pInterface)
{
	// get the world property for the world type
	const char* szWorldPropertiesObject;
	pInterface->GetObjectsOfClass("WorldProperties", &szWorldPropertiesObject, 1, NULL);

	char szWorldType[MAX_PATH];
	pInterface->GetPropString(szWorldPropertiesObject, "WorldType", szWorldType, LTARRAYSIZE(szWorldType), NULL);

	// handle singleplayer or multiplayer based on the world type
	if (LTStrEquals(szWorldType, "MultiPlayer"))
	{
		GatherResourcesPostObjects_Multiplayer(pInterface);
	}
	else
	{
		GatherResourcesPostObjects_Singleplayer(pInterface);
	}

	// retrieve common resources
	GatherResourcesPostObjects_Common(pInterface);

	// process the surfaces
	uint32 nNumberOfSurfaces = pInterface->GetNumRegionSurfaceTypes(NULL);

	for (uint32 nIndex = 0; nIndex < nNumberOfSurfaces; ++nIndex)
	{
		// retrieve the surface
		uint32 nSurfaceType = pInterface->GetRegionSurfaceType(NULL, nIndex);
		HSURFACE hSurface = g_pSurfaceDB->GetSurface((SurfaceType)nSurfaceType);

		if (hSurface)
		{
			// get all the resources referenced directly by this surface
			ResourceList cSurfaceResourceList;
			GetRecordResources(cSurfaceResourceList, hSurface, true);
			AddResourcesToObjectGathererRegion(pInterface, NULL, cSurfaceResourceList);
		
			// now iterate over all the surface fx types we found during our weapon processing
			for (std::vector<std::string>::iterator itSurfaceFX = g_SurfaceFXTypes.begin(); itSurfaceFX != g_SurfaceFXTypes.end(); ++itSurfaceFX)
			{
				ResourceList cImpactFXResourceList;
				const char* pszSurfaceFx = (*itSurfaceFX).c_str();
				HSRF_IMPACT hImpactFX = CSurfaceDB::Instance().GetSurfaceImpactFX(hSurface, pszSurfaceFx);

				GetRecordResources(cImpactFXResourceList, hImpactFX, true);
				AddResourcesToObjectGathererRegion(pInterface, NULL, cImpactFXResourceList);
			}
		}
	}
}

END_EXTERNC()

#else // PLATFORM_LINUX

// empty versions for the Linux build
void AddPropStringToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName) {}
void AddSoundResourceToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName) {}
void PrefetchPlayer(const char* pszObjectName, IObjectResourceGatherer* pInterface) {}
void PrefetchWorldModel(const char* pszObjectname, IObjectResourceGatherer* pInterface) {}
void PlaceResourcesDefault(const char* pszObjectName, IObjectResourceGatherer* pInterface, const ResourceList& Resources ) {}
void PlaceResourcesPlayer(const ResourceList& Resources, IObjectResourceGatherer* pInterface) {}
void GetModelResources(ResourceList& Resources, HRECORD hModel, const char* pszModelAtt) {}
void GetMaterialListResources(ResourceList& Resources, HRECORD hModel, const char* pszMaterialListAtt) {}
void GetSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt) {}
void GetLocalizedSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt) {}
void GetClientFXResources(ResourceList& Resources, const char* pszClientFX) {}
void GetClientFXResources(ResourceList& Resources, HRECORD hParent, const char* pszClientFXAtt) {}
void GetWeaponListResources(ResourceList& Resources, HRECORD hParent, const char* pszWeaponListAtt) {}
void GetWeaponResources(ResourceList& Resources, HRECORD hWeapon, HAMMO hAmmo) {}
void GetCollisionPropertyResources(ResourceList& Resources, HRECORD hCollisionProp) {}
void GetAmmoResources(ResourceList& Resources, HRECORD hAmmo) {}
void GetCharacterModelResources(ResourceList& Resources, HRECORD hModel) {}
void GetCharacterInventoryResources(ResourceList& Resources, HRECORD hModel, bool bUseDefaultWeapons, bool bUseDefaultAttachments) {}
void GetPlayerResources(ResourceList& Resources) {}
void GetPlayerInventoryResources(ResourceList& Resources) {}
void GetRecordResources(ResourceList& Resources, HRECORD hRecord, bool bRecurseRecordLinks) {}

#endif // PLATFORM_LINUX

