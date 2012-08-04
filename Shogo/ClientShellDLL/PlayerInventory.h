// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerInventory.h
//
// PURPOSE : Definition of PlayerInventory class
//
// CREATED : 3/3/98
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERINVENTORY_H
#define __PLAYERINVENTORY_H

#include "WeaponDefs.h"

class ILTClient;
class CRiotClientShell;

class CPlayerInventory
{
public:

	CPlayerInventory();
	~CPlayerInventory();

	LTBOOL		Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
	void		Term();

	void		Reset();
	void		Update();

	void		GunPickup (uint8 nType, LTBOOL bDisplayMessage=LTTRUE);
	void		UpdateAmmo (uint32 nType, uint32 nAmount);

	void		ShogoPowerupPickup (PickupItemType ePowerup);
	void		ShogoPowerupClear();

	void		Draw (LTBOOL bDrawOrdinance);

	void		Save(ILTMessage_Write* hWrite);
	void		Load(ILTMessage_Read* hRead);

	LTBOOL		CanDrawGun(uint8 nWeaponId);

protected:

	LTBOOL		InitSurfaces();

protected:
	
	ILTClient*			m_pClientDE;
	CRiotClientShell*	m_pClientShell;

	LTBOOL				m_bHaveGun[GUN_MAX_NUMBER];
	LTBOOL				m_bHaveShogoLetter[5];
	HSURFACE			m_hShogoLetter[5];
	
	LTBOOL				m_bAmmoChanged;
	uint32				m_nNewAmmoType;
	uint32				m_nNewAmmoAmount;

	HLTFONT				m_hAmmoCountFont;

	HSURFACE			m_hCurrentMessage;
	uint32				m_cxCurrentMessage;
	uint32				m_cyCurrentMessage;

	LTFLOAT				m_fDisplayTimeLeft;

	HSURFACE			m_hOrdinance;

	HSURFACE			m_hGunIcon[GUN_MAX_NUMBER];
	uint32				m_cxGunIcon[GUN_MAX_NUMBER];
	uint32				m_cyGunIcon[GUN_MAX_NUMBER];

	HSURFACE			m_hGunName[GUN_MAX_NUMBER];
	HSURFACE			m_hAmmoCount[GUN_MAX_NUMBER];

};

#endif