// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.h
//
// PURPOSE : HUD element to display a weapon item
//
// CREATED : 12/17/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDWEAPON_H__
#define __HUDWEAPON_H__

#include "HUDInventory.h"
#include "WeaponDB.h"

class CHUDWeapon : public CHUDInventory
{
public:
	CHUDWeapon();
	virtual ~CHUDWeapon() {}

	virtual bool	Init();
	virtual void	Render();
	virtual void	SetBasePos(const LTVector2n& pos);

	//position handled by CHUDWeaponList
	virtual void	ScaleChanged();

	void			SetWeaponRecord(HWEAPON hWeapon);
	HWEAPON			GetWeaponRecord() const {return m_hWeapon;}

	virtual void	Update();
	virtual void	UpdateLayout();
	virtual void	UpdateFade();
	virtual void	UpdateFlicker();
	virtual void	EndFlicker();
	virtual void	UpdateFlash();

	void			UpdateTriggerName(const wchar_t* szTrigger);

	void			Select(bool bSelected);
	bool			IsSelected() const {return m_bSelected;}

protected:
	void			UpdateBar();

	HWEAPON			m_hWeapon;

	CLTGUIString	m_Hotkey;
	uint32			m_cHotkeyColor;
	LTVector2n		m_vHotkeyOffset;

	uint32			m_cNormalColor;
	uint32			m_cDisabledColor;

	LTVector2n		m_vLargeSize;
	LTVector2n		m_vSmallSize;

	LTVector2n		m_vLargeOffset;
	LTVector2n		m_vSmallOffset;

	bool			m_bSelected;

	LTPoly_GT4			m_BarPoly;
	TextureReference	m_hBarTexture;
	float				m_fBarWidth;


};


#endif  // __HUDWEAPON_H__
