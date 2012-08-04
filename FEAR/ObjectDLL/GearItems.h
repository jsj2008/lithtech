// ----------------------------------------------------------------------- //
//
// MODULE  : GearItems.h
//
// PURPOSE : Gear items definition
//
// CREATED : 10/22/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GEAR_ITEMS_H__
#define __GEAR_ITEMS_H__

#include "PickupItem.h"
#include "NavMarker.h"

LINKTO_MODULE( GearItems );

class GearItem : public PickupItem
{
	public :

		GearItem();
		virtual ~GearItem();


		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);

		virtual void	ObjectTouch(HOBJECT hObject);
		virtual void	PickedUp( bool bWasPickedUp, bool bWeaponsStay );
		virtual void	Respawn( );


	private :

		void			SpawnMarker();

		bool ReadProp(const GenericPropList *pProps);
		bool PostPropRead(ObjectCreateStruct *pStruct);
		void InitialUpdate(ILTServer *pServer);
};

////////////////////////////////////////////////////////////////////////////
//
// CGearPlugin is used to help facilitate populating the WorldEdit object
// properties that use WeaponDB
//
////////////////////////////////////////////////////////////////////////////

#include "iobjectplugin.h"

class CGearPlugin : public IObjectPlugin
{
	public:

		LTRESULT PreHook_EditStringList(
				const char* szRezPath,
				const char* szPropName,
				char** aszStrings,
				uint32* pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength );

		LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char	*szModifiers );

		LTRESULT PreHook_Dims(
				const char* szRezPath,
				const char* szPropName, 
				const char* szPropValue,
				char* szModelFilenameBuf,
				int	  nModelFilenameBufLen,
				LTVector & vDims,
				const char* pszObjName, 
				ILTPreInterface *pInterface);

	protected:

		CPickupItemPlugin m_PickupItemPlugin;
};

#endif //  __GEAR_ITEMS_H__
