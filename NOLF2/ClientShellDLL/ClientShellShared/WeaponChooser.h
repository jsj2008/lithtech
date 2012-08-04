// WeaponChooser.h: interface for the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WEAPONCHOOSER_H_)
#define _WEAPONCHOOSER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGUIMgr.h"
#include "Timer.h"

class CWeaponChooser
{
public:
	CWeaponChooser();
	virtual ~CWeaponChooser();

	void	Init();
	void	Term();

    bool	Open(uint8 nClass);
	void	Close();
    bool	IsOpen()    {return m_bIsOpen;}

	void	NextWeapon(uint8 nClass);
	void	PrevWeapon();
	void	EndAutoSwitch(bool bNextWeaponKey = true);

	void	Update();

    uint8   GetCurrentSelection() {return m_nWeapon;}
    uint8   GetCurrentClass() {return m_nClass;}

private:

	uint8			m_nWeapon;
	uint8			m_nClass;
    bool			m_bIsOpen;

	CTimer		m_NextWeaponKeyDownTimer;
	CTimer		m_PrevWeaponKeyDownTimer;
	CTimer		m_AutoSwitchTimer;
	CTimer		m_AutoCloseTimer;
};

class CAmmoChooser
{
public:
	CAmmoChooser();
	virtual ~CAmmoChooser();

	void	Init();
	void	Term();

    bool	Open();
	void	Close();
    bool	IsOpen()    {return m_bIsOpen;}

	void	NextAmmo();
	void	EndAutoSwitch();

	void	Update();

    uint8   GetCurrentSelection() {return m_nAmmo;}

private:
	uint8			m_nAmmo;
    bool			m_bIsOpen;

	CTimer		m_NextAmmoKeyDownTimer;
	CTimer		m_AutoSwitchTimer;
	CTimer		m_AutoCloseTimer;

};

#endif // !defined(_WEAPONCHOOSER_H_)