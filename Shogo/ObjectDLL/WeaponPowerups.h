// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPowerups.h
//
// PURPOSE : Riot weapon powerups implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_POWERUPS_H__
#define __WEAPON_POWERUPS_H__

#include "Powerup.h"
#include "Bouncer.h"
#include "ModelFuncs.h"

class WeaponPowerup : public Powerup
{
	public :

		WeaponPowerup();

	protected :

		DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void			ObjectTouch(HOBJECT hObject);

		void			ReadProp( ObjectCreateStruct *pStruct );
		void			PostPropRead( ObjectCreateStruct *pStruct );
		void			InitialUpdate( CServerDE *pServer );

		DBYTE			m_iWeaponType;
		DDWORD			m_dwAmmo;
		ModelSize		m_eModelSize;

		CBouncer		m_bounce;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

class PulseRiflePowerup : public WeaponPowerup
{
	public :
		PulseRiflePowerup();
};

class SpiderPowerup : public WeaponPowerup
{
	public :
		SpiderPowerup();
};

class BullgutPowerup : public WeaponPowerup
{
	public :
		BullgutPowerup();
};

class SniperRiflePowerup : public WeaponPowerup
{
	public :
		SniperRiflePowerup();
};

class JuggernautPowerup : public WeaponPowerup
{
	public :
		JuggernautPowerup();
};

class ShredderPowerup : public WeaponPowerup
{
	public :
		ShredderPowerup();
};

class RedRiotPowerup : public WeaponPowerup
{
	public :
		RedRiotPowerup();
};

class PistolsPowerup : public WeaponPowerup
{
	public :
		PistolsPowerup();
};

class ShotgunPowerup : public WeaponPowerup
{
	public :
		ShotgunPowerup();
};

class AssaultRiflePowerup : public WeaponPowerup
{
	public :
		AssaultRiflePowerup();
};

class EnergyGrenadePowerup : public WeaponPowerup
{
	public :
		EnergyGrenadePowerup();
};

class KatoGrenadePowerup : public WeaponPowerup
{
	public :
		KatoGrenadePowerup();
};

class MachineGunPowerup : public WeaponPowerup
{
	public :
		MachineGunPowerup();
};

class TOWPowerup : public WeaponPowerup
{
	public :
		TOWPowerup();
};

class LaserCannonPowerup : public WeaponPowerup
{
	public :
		LaserCannonPowerup();
};

class SqueakyToyPowerup : public WeaponPowerup
{
	public :
		SqueakyToyPowerup();
};

#endif //  __WEAPON_POWERUPS_H__