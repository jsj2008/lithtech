// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponList.h
//
// PURPOSE : HUD Element displaying a list of weapon items
//
// CREATED : 12/17/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDWEAPONLIST_H__
#define __HUDWEAPONLIST_H__

#include "HUDWeapon.h"
#include "HUDInventoryList.h"

class CHUDWeaponList : public CHUDInventoryList
{
public:
	CHUDWeaponList();
	virtual ~CHUDWeaponList() {}

	virtual bool Init();
	virtual void Term();

	virtual void Update();
	virtual void UpdateLayout();

	void SetWeapon(HWEAPON hWeapon, uint8 nSlot );
	void RemoveWeapon(HWEAPON hWeapon);
	void RemoveWeaponFromSlot( uint8 nSlot );
	void Reset();

	void UpdateTriggerNames();
	void UpdatePos();


protected:
	uint8 FindWeapon(HWEAPON hWeapon);

	LTVector2n		m_vSelectedItemOffset;
	HWEAPON			m_hSelectedWeapon;
};

class CHUDGrenadeList : public CHUDInventoryList
{
public:
	CHUDGrenadeList();
	virtual ~CHUDGrenadeList() {}

	virtual bool Init();
	virtual void Term();

	virtual void Update();
	virtual void UpdateLayout();

	void UpdateTriggerNames();
	void UpdatePos();


protected:
//	uint8 FindWeapon(HWEAPON hWeapon);

};




#endif  // __HUDWEAPONLIST_H__
