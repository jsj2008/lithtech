// ----------------------------------------------------------------------- //
//
// MODULE  : PowerupPickups.h
//
// PURPOSE : Powerup pickup items
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __POWERUPPICKUPS_H__
#define __POWERUPPICKUPS_H__


#include "PickupObject.h"
#include "InventoryMgr.h"



class PowerupPickup : public PickupObject
{
	public:

		PowerupPickup() : PickupObject() 
		{
			m_bRotate = DTRUE;
		}

	protected :

		void		ObjectTouch (HOBJECT hObject);
};

class HealthBase : public PowerupPickup
{
	public:

		HealthBase();

	private:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);

};

// talisman pickups
class HealthPU : public HealthBase
{
	public :
		HealthPU();
};

class MegaHealthPU : public HealthBase
{
	public :
		MegaHealthPU();
};

class ArmorBase : public PowerupPickup
{
	public:

		ArmorBase();

	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
};

class WardPU : public ArmorBase
{
	public :
		WardPU();
};

class NecroWardPU : public ArmorBase
{
	public :
		NecroWardPU();
};

class EnhancementBase : public PowerupPickup
{
	public:

		EnhancementBase();

	protected:

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, float fData);
};

class InvulnerabilityPU : public EnhancementBase
{
	public :
		InvulnerabilityPU();
};

class StealthPU : public EnhancementBase
{
	public :
		StealthPU();
};

class AngerPU : public EnhancementBase
{
	public :
		AngerPU();
};

class RevenantPU : public EnhancementBase
{
	public :
		RevenantPU();
};

#endif //  __POWERUPPICKUPS_H__