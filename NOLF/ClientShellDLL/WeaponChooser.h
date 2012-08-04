// WeaponChooser.h: interface for the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdlith.h"

#define NUM_WEAPON_ICONS	3
#define NUM_AMMO_ICONS		2

class CWeaponChooser
{
public:
	CWeaponChooser();
	virtual ~CWeaponChooser();

	void	Term();

    LTBOOL   Open();
	void	Close();
    LTBOOL   IsOpen()    {return m_bIsOpen;}

	void	NextWeapon();
	void	PrevWeapon();

	void	Draw();


    uint8   GetCurrentSelection() {return m_nWeapons[1];}

private:
	void		SetCommandStr(int nWeaponId);

	HSURFACE	m_hWeaponSurf[NUM_WEAPON_ICONS];
    uint8       m_nWeapons[NUM_WEAPON_ICONS];
    LTBOOL       m_bIsOpen;
	HSTRING		m_hWeaponStr;
	float		m_fStartTime;

	char		m_szWeaponCommand[8];
};

class CAmmoChooser
{
public:
	CAmmoChooser();
	virtual ~CAmmoChooser();

	void	Term();

    LTBOOL   Open();
	void	Close();
    LTBOOL   IsOpen()    {return m_bIsOpen;}

	void	NextAmmo();

	void	Draw();


    uint8   GetCurrentSelection() {return m_nAmmo[0];}

private:
	HSURFACE	m_hAmmoSurf[NUM_AMMO_ICONS];
    uint8       m_nAmmo[NUM_AMMO_ICONS];
    LTBOOL       m_bIsOpen;
	HSTRING		m_hAmmoStr;
	float		m_fStartTime;
};

#endif // !defined(AFX_WEAPONCHOOSER_H__1762B140_8553_11D3_B2DB_006097097C7B__INCLUDED_)