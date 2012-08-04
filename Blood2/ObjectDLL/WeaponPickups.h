// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPickups.h
//
// PURPOSE : Weapon powerup items
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONPICKUPS_H__
#define __WEAPONPICKUPS_H__


#include "PickupObject.h"
#include "InventoryMgr.h"
#include "SharedDefs.h"


class WeaponPickup : public PickupObject
{
	public:

		WeaponPickup() : PickupObject() 
		{
			m_bBounce = DTRUE;
			m_fRespawnTime = 0.0f;		// Set to 0 so dropped weapons won't respawn
			m_szPickupSound = "sounds\\powerups\\weapon1.wav";
		}

	protected :

		void		ObjectTouch (HOBJECT hObject);
		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
};


class BerettaPU : public WeaponPickup
{
	public :
		BerettaPU();
};


class ShotgunPU : public WeaponPickup
{
	public :
		ShotgunPU();
};


#ifdef _ADD_ON

class CombatShotgunPU : public WeaponPickup
{
	public :
		CombatShotgunPU();
};

#endif

class SniperRiflePU : public WeaponPickup
{
	public :
		SniperRiflePU();
};


class AssaultRiflePU : public WeaponPickup
{
	public :
		AssaultRiflePU();
};


class SubMachineGunPU : public WeaponPickup
{
	public :
		SubMachineGunPU();
};


class FlareGunPU : public WeaponPickup
{
	public :
		FlareGunPU();
};


class HowitzerPU : public WeaponPickup
{
	public :
		HowitzerPU();
};


class BugSprayPU : public WeaponPickup
{
	public :
		BugSprayPU();
};


class NapalmCannonPU : public WeaponPickup
{
	public :
		NapalmCannonPU();
};


class MiniGunPU : public WeaponPickup
{
	public :
		MiniGunPU();
};


class VoodooDollPU : public WeaponPickup
{
	public :
		VoodooDollPU();
};


class OrbPU : public WeaponPickup
{
	public :
		OrbPU();
};


class DeathRayPU : public WeaponPickup
{
	public :
		DeathRayPU();
};


class LifeLeechPU : public WeaponPickup
{
	public :
		LifeLeechPU();
};

#ifdef _ADD_ON

class FlayerPU : public WeaponPickup
{
	public :
		FlayerPU();
};

#endif

class TeslaCannonPU : public WeaponPickup
{
	public :
		TeslaCannonPU();
};


class SingularityPU : public WeaponPickup
{
	public :
		SingularityPU();
};


// Function which spawns a new weapon pickup item 
HOBJECT SpawnWeaponPickup(DDWORD dwWeapType, HOBJECT hOwner, DVector *pvPos, DRotation *prRot, DBOOL bIncludeAmmo);


#endif //  __WEAPONPICKUPS_H__