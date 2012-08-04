// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGear.h
//
// PURPOSE : HUD element to display a gear item
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDGEAR_H__
#define __HUDGEAR_H__

#include "HUDInventory.h"
#include "WeaponDB.h"

class CHUDGear : public CHUDInventory
{
public:
	CHUDGear();
	virtual ~CHUDGear() {}

	virtual bool Init();

	void	SetGearRecord(HGEAR hGear);
	HGEAR	GetGearRecord() const {return m_hGear;}

	virtual void Update();
	virtual void ScaleChanged() {};
	virtual void UpdateLayout();
	virtual void Render();

protected:
	HGEAR	m_hGear;
};

#endif  // __HUDGEAR_H__
