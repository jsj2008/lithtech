// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.h
//
// PURPOSE : Weapon items implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//			 07/12/02 - kls
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_ITEMS_H__
#define __WEAPON_ITEMS_H__

#include "PickupItem.h"
#include "AIEnumStimulusTypes.h"
#include "PrefetchUtilities.h"

LINKTO_MODULE( WeaponItems );

class CWeapon;

class WeaponItem : public PickupItem
{
	public :
		DEFINE_CAST(WeaponItem);

		WeaponItem();
		~WeaponItem();

		HWEAPON			GetWeaponRecord() const { return m_Shared.m_hRecord; }
		HAMMO			GetAmmoRecord()	const { return m_hAmmo; }
		int				GetAmmoAmount() const { return m_nAmmo; }
		void			SetAmmoAmount(int32 nCount) {m_nAmmo = nCount;}

		virtual void	ObjectTouch (HOBJECT hObject);

		// Adds a muzzle smoke clientfx.
		void			AddMuzzleSmoke( );

		virtual HRECORD GetCollisionProperty();

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

		uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);

		HAMMO			m_hAmmo;
		int32			m_nAmmo;
		EnumAIStimulusID	m_eStimID;
		uint32			m_nHealth;		// how many hits the weapon can take before it breaks

		virtual void	PickedUp( bool bWasPickedUp, bool bWeaponsStay );
		virtual void	Respawn( );


	private :

		bool			ReadProp(const GenericPropList *pProps);
		bool			PostPropRead(ObjectCreateStruct *pStruct);
		void			InitialUpdate();

		void			Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void			Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void			RegisterStimulus();
		void			UnregisterStimulus();
};

class CWeaponItemPlugin : public CPickupItemPlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList(
				const char* szRezPath,
				const char* szPropName,
				char** aszStrings,
				uint32* pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength);

		virtual LTRESULT PreHook_Dims(
				const char* szRezPath,
				const char* szPropName, 
				const char* szPropValue,
				char* szModelFilenameBuf,
				int	  nModelFilenameBufLen,
				LTVector & vDims,
				const char* pszObjName, 
				ILTPreInterface *pInterface);

};

#endif //  __WEAPON_ITEMS_H__
