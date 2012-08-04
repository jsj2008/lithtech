// ----------------------------------------------------------------------- //
//
// MODULE  : ItemPickups.h
//
// PURPOSE : Item powerup items
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __ITEMPICKUPS_H__
#define __ITEMPICKUPS_H__


#include "PickupObject.h"
#include "InventoryMgr.h"
#include "BloodServerShell.h"


// Class for general item pickups
class ItemPickup : public PickupObject
{
	public:

		ItemPickup() : PickupObject() 
		{
			m_szPickupSound	= "sounds\\powerups\\inventory1.wav";
			m_fValue		= 1.0f;
			m_bBounce = DTRUE;
		}

	protected :

		void		ObjectTouch (HOBJECT hObject);
};


// This class is for the weapon like pickups...

class ItemPickupWeapon : public ItemPickup
{
	public:

		ItemPickupWeapon() : ItemPickup() 
		{
			m_szPickupSound	= "sounds\\powerups\\ammo2.wav";
			m_fValue		= 1.0f;
		}

	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
};

// This class is for the charging pickups...

class ItemPickupCharged : public ItemPickup
{
	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
};

class FlashlightPU : public ItemPickupCharged
{
	public :
		FlashlightPU();
};


class MedKitPU : public ItemPickupCharged
{
	public :
		MedKitPU();
};


class NightGogglesPU : public ItemPickupCharged
{
	public :
		NightGogglesPU();
};


class ProximitiesPU : public ItemPickupWeapon
{
	public :
		ProximitiesPU();
};


class RemotesPU : public ItemPickupWeapon
{
	public :
		RemotesPU();
};


class TimeBombPU : public ItemPickupWeapon
{
	public :
		TimeBombPU();
};


class BinocularsPU : public ItemPickupCharged
{
	public :
		BinocularsPU();
};


class TheEyePU : public ItemPickupCharged
{
	public :
		TheEyePU();
};

// Function which spawns a new item pickup 
//HOBJECT SpawnItemPickup(DDWORD dwItemType, HOBJECT hOwner);


// Class for special level-specific pickup items, keys, etc.
class KeyPickup : public PickupObject
{
	public:

		KeyPickup() : PickupObject() 
		{
			m_szPickupSound	= "sounds\\powerups\\inventory1.wav";
			m_hstrIconFile = DNULL;
			m_hstrIconFileH = DNULL;
			m_hstrFilename = DNULL;
			m_hstrSkinName = DNULL;
			m_byUseCount = 1;
			m_bBounce = DTRUE;
//			m_nNameID = 0;
		}

		~KeyPickup()
		{
			if (!g_pServerDE) return;

			if (m_hstrIconFile)
				g_pServerDE->FreeString(m_hstrIconFile);
			if (m_hstrIconFileH)
				g_pServerDE->FreeString(m_hstrIconFileH);
			if (m_hstrFilename)
				g_pServerDE->FreeString(m_hstrFilename);
			if (m_hstrSkinName)
				g_pServerDE->FreeString(m_hstrSkinName);
		}

	private:

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);
	
	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void		ObjectTouch (HOBJECT hObject);

		HSTRING		m_hstrIconFile;
		HSTRING		m_hstrIconFileH;
		HSTRING		m_hstrFilename;
		HSTRING		m_hstrSkinName;
		DBYTE		m_byUseCount;
//		int			m_nNameID;
};




#endif //  __ITEMPICKUPS_H__