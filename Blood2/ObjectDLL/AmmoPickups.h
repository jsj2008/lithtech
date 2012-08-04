// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoPickups.h
//
// PURPOSE : Ammunition powerup items
//
// CREATED : 11/24/97
//
// ----------------------------------------------------------------------- //

#ifndef __AMMOPICKUPS_H__
#define __AMMOPICKUPS_H__


#include "PickupObject.h"
#include "InventoryMgr.h"
#include "BloodServerShell.h"


class AmmoPickup : public PickupObject
{
	public:

		AmmoPickup() : PickupObject()
		{
			if( m_dwNumPU == 0 )
			{
				dl_TieOff( &m_PUHead );
				m_PUHead.m_pData = DNULL;
			}

			m_szPickupSound	= "sounds\\powerups\\ammo1.wav";
		}

		~AmmoPickup()
		{
			if( m_Link.m_pData && m_dwNumPU > 0 )
			{
				dl_Remove( &m_Link );
				m_dwNumPU--;
			}
		}

		static DLink	m_PUHead;
		static DDWORD	m_dwNumPU;
		DLink			m_Link;

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
		void		ObjectTouch (HOBJECT hObject);
};


class BulletAmmoPU : public AmmoPickup
{
	public :
		BulletAmmoPU();
};


class ShellAmmoPU : public AmmoPickup
{
	public :
		ShellAmmoPU();
};


class BMGAmmoPU : public AmmoPickup
{
	public :
		BMGAmmoPU();
};


class FlareAmmoPU : public AmmoPickup
{
	public :
		FlareAmmoPU();
};


class DieBugDieAmmoPU : public AmmoPickup
{
	public :
		DieBugDieAmmoPU();
};


class HowitzerAmmoPU : public AmmoPickup
{
	public :
		HowitzerAmmoPU();
};


class FuelAmmoPU : public AmmoPickup
{
	public :
		FuelAmmoPU();
};


class BatteryAmmoPU : public AmmoPickup
{
	public :
		BatteryAmmoPU();
};

/*		Don't need this one, since Mana regens
class ManaAmmoPU : public AmmoPickup
{
	public :
		ManaAmmoPU();
};

*/
#endif //  __AMMOPICKUPS_H__