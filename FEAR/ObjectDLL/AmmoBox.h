// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.h
//
// PURPOSE : AmmoBox object definition
//
// CREATED : 10/28/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AMMO_BOX_H__
#define __AMMO_BOX_H__

#include "PickupItem.h"

#define AB_MAX_TYPES	10

LINKTO_MODULE( AmmoBox );

class AmmoBox : public PickupItem
{
	public :

		AmmoBox();

		virtual void	PickedUp( bool bWasPickedUp, bool bWeaponsStay );
		virtual void	Respawn( );

		virtual PickupItemType GetPickupItemType( ) { return kPickupItemType_AmmoBox; }

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);
		uint32			ObjectMessageFn (HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	ObjectTouch(HOBJECT hObject);

		HAMMO			m_hAmmo[AB_MAX_TYPES];
		int				m_nAmmoCount[AB_MAX_TYPES];

		HAMMO			m_hOriginalAmmo[AB_MAX_TYPES];
		int				m_nOriginalAmmoCount[AB_MAX_TYPES];

		std::string		m_sPowerupFX;
		std::string		m_sRespawnWaitFX;

		std::string		m_sOriginalFilename;
		std::string		m_sOriginalMaterial;

		bool			m_bRespawnWaitVisible;
		bool			m_bRespawnWaitTranslucent;

	private :

		bool ReadProp(const GenericPropList *pProps);
		void InitialUpdate(ILTServer *pServer);
		void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void Leftovers(ILTMessage_Read *pMsg);
};

////////////////////////////////////////////////////////////////////////////
//
// CAmmoBoxPlugin is used to help facilitate populating the WorldEdit object
// properties that use WeaponDB
//
////////////////////////////////////////////////////////////////////////////

#include "iobjectplugin.h"

class CAmmoBoxPlugin : public IObjectPlugin
{
	public:

		LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

		LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

		virtual LTRESULT PreHook_Dims(const char* szRezPath,
			const char* szPropName, 
			const char* szPropValue,
			char* szModelFilenameBuf,
			int nModelFilenameBufLen,
			LTVector & vDims,
			const char* pszObjName, 
			ILTPreInterface *pInterface);

	protected:

		CPickupItemPlugin m_PickupItemPlugin;
};

#endif //  __AMMO_BOX_H__
