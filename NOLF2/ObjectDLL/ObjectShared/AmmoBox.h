// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.h
//
// PURPOSE : AmmoBox object definition
//
// CREATED : 10/28/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __AMMO_BOX_H__
#define __AMMO_BOX_H__

#include "PickupItem.h"
#include "WeaponMgr.h"

#define AB_MAX_TYPES	10

LINKTO_MODULE( AmmoBox );

class AmmoBox : public PickupItem
{
	public :

		AmmoBox();

		virtual void	PickedUp(ILTMessage_Read *pMsg);
		virtual void	Respawn( );

	protected :

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32          ObjectMessageFn (HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	ObjectTouch(HOBJECT hObject, bool bForcePickup=false);

        uint8           m_nAmmoId[AB_MAX_TYPES];
		int				m_nAmmoCount[AB_MAX_TYPES];

        uint8           m_nOriginalAmmoId[AB_MAX_TYPES];
		int				m_nOriginalAmmoCount[AB_MAX_TYPES];
		
		std::string		m_sPowerupFX;
		std::string		m_sRespawnWaitFX;

		CButeListReader	m_blrRespawnWaitSkins;
		CButeListReader	m_blrRespawnWaitRenderStyles;

		bool			m_bRespawnWaitVisible;
		bool			m_bRespawnWaitTranslucent;

	private :

		bool ReadProp(ObjectCreateStruct *pStruct);
        void InitialUpdate(ILTServer *pServer);
        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void Leftovers(ILTMessage_Read *pMsg);
};

#ifndef __PSX2
class CAmmoBoxPlugin : public CWeaponMgrPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

	protected:

		CPickupItemPlugin m_PickupItemPlugin;
};
#endif

#endif //  __AMMO_BOX_H__