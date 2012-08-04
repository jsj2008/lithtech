// WeaponChooser.h: interface for the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WEAPONCHOOSER_H_)
#define _WEAPONCHOOSER_H_


#include "LTGUIMgr.h"

class CWeaponChooser
{
public:
	CWeaponChooser();
	virtual ~CWeaponChooser();

	void	Init();
	void	Term();

	bool	Open(uint8 nClass);
	void	Close();
	bool	IsOpen()	{return m_bIsOpen;}

	void	NextWeapon(uint8 nClass);
	void	PrevWeapon();
	void	EndAutoSwitch(bool bNextWeaponKey = true);

	void	Update();

	HWEAPON	GetCurrentSelection()	{return m_hWeapon;}
	uint8	GetCurrentClass()		{return m_nClass;}

private:

	HWEAPON			m_hWeapon;
	uint8			m_nClass;
	bool			m_bIsOpen;

	StopWatchTimer		m_NextWeaponKeyDownTimer;
	StopWatchTimer		m_PrevWeaponKeyDownTimer;
	StopWatchTimer		m_AutoSwitchTimer;
	StopWatchTimer		m_AutoCloseTimer;
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

    HAMMO   GetCurrentSelection() {return m_hAmmo;}

private:
	
	HAMMO		m_hAmmo;
	bool		m_bIsOpen;

	StopWatchTimer		m_NextAmmoKeyDownTimer;
	StopWatchTimer		m_AutoSwitchTimer;
	StopWatchTimer		m_AutoCloseTimer;

};

#endif // !defined(_WEAPONCHOOSER_H_)