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

#ifndef __PREFETCH_UTILITIES_H__
#define __PREFETCH_UTILITIES_H__

#include "platform.h"
#include "iobjectresourcegatherer.h"

//-----------------------------------------------------------------------------
// definitions

typedef std::vector<std::string> ResourceList;

//-----------------------------------------------------------------------------
// helper macros

#define ADD_PREFETCH_RESOURCE_PROPS() \
	ADD_BOOLPROP_FLAG( GlobalResources, false, 0, "Indicates whether the object's resources should be considered as global in the resource streaming system.")

//-----------------------------------------------------------------------------
// Prefetching functions - responsible for gathering the resources an object
// needs, determining where they will be needed, and passing that information
// to the asset streaming system

//-----------------------------------------------------------------------------
// This is the default prefetching function that passes all of the resources
// this object will need to the streaming system. Game object classes can define
// their own custom functions, but this should work for the majority of the cases
// It calls back into the game object class function GetPrefetchResourceList for the
// list of specific resources and then calls the default resource placement function.

template <class T>
void DefaultPrefetch(const char* pszObjectName, IObjectResourceGatherer* pInterface)
{
	if(pInterface->DoesObjectExist(pszObjectName))
	{
		ResourceList Resources;
		T::GetPrefetchResourceList(pszObjectName, pInterface, Resources);
		PlaceResourcesDefault(pszObjectName,  pInterface, Resources);
	}
}

//-----------------------------------------------------------------------------
// helper for adding a string property value directly to the resource list
void AddPropStringToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName);

// helper for adding a sound property to the resource list - this determines
// if the property is a database reference or a file reference and processes it
// accordingly
void AddSoundResourceToObjectGatherer(IObjectResourceGatherer* pInterface, ResourceList& cResourceList, const char* pszObjectName, const char* pszPropName);

//-----------------------------------------------------------------------------
// custom player prefetching for the player

void PrefetchPlayer(const char* pszObjectName, IObjectResourceGatherer* pInterface);
void PrefetchWorldModel(const char* pszObjectname, IObjectResourceGatherer* pInterface);

//-----------------------------------------------------------------------------
// Resources Placement functions - 
//   decide which streaming regions to place the resources

void PlaceResourcesDefault(const char* pszObjectName, IObjectResourceGatherer* pInterface, const ResourceList& Resources );
void PlaceResourcesPlayer(const ResourceList& Resources, IObjectResourceGatherer* pInterface);

//-----------------------------------------------------------------------------
// Resource list functions - determine the list of resources an object needs

void GetModelResources(ResourceList& Resources, HRECORD hModel, const char* pszModelAtt="ModelFile");
void GetMaterialListResources(ResourceList& Resources, HRECORD hModel, const char* pszMaterialListAtt="Material");
void GetSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt="SoundFile");
void GetLocalizedSoundListResources(ResourceList& Resources, HRECORD hParent, const char* pszSoundListAtt="SoundFile");
void GetClientFXResources(ResourceList& Resources, const char* pszClientFX);
void GetClientFXResources(ResourceList& Resources, HRECORD hParent, const char* pszClientFXAtt);
void GetWeaponListResources(ResourceList& Resources, HRECORD hParent, const char* pszWeaponListAtt);
void GetWeaponResources(ResourceList& Resources, HRECORD hWeapon, HAMMO hAmmo=NULL);
void GetCollisionPropertyResources(ResourceList& Resources, HRECORD hCollisionProp);
void GetAmmoResources(ResourceList& Resources, HRECORD hAmmo);
void GetCharacterModelResources(ResourceList& Resources, HRECORD hModel);
void GetCharacterInventoryResources(ResourceList& Resources, HRECORD hModel, bool bUseDefaultWeapons, bool bUseDefaultAttachments);
void GetPlayerResources(ResourceList& Resources);
void GetPlayerInventoryResources(ResourceList& Resources);
void GetRecordResources(ResourceList& Resources, HRECORD hRecord, bool bRecurseRecordLinks);

#endif // __PREFETCH_UTILITIES_H__

