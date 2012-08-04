// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.h
//
// PURPOSE : Definition of PlayerStats class
//
// CREATED : 10/9/97
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERSTATS_H
#define __PLAYERSTATS_H

#include "WeaponDefs.h"
#include "ltbasedefs.h"
#include "iltmessage.h"

#define FLASH_HEALTH	0x01
#define FLASH_ARMOR		0x02
#define FLASH_AMMO		0x04

class CRiotClientShell;

class CPlayerStats
{
public:

	CPlayerStats();
	~CPlayerStats();

	LTBOOL		Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
	void		Term();

	void		OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
	void		OnExitWorld();

	void		Draw (LTBOOL bStatsSizedOff, LTBOOL bDrawHud);

	void		Clear();
	void		Update();
	void		UpdatePlayerMode (uint8 nNewMode, LTBOOL bForce=LTFALSE);
	void		UpdatePlayerWeapon (uint8 nWeapon, LTBOOL bForce=LTFALSE);
	void		UpdateHealth (uint32 nHealth);
	void		UpdateArmor (uint32 nArmor);
	void		UpdateAmmo (uint32 nType, uint32 nAmmo);
	void		UpdateAir (LTFLOAT nPercent);

	void		Save(ILTMessage_Write* hWrite);
	void		Load(ILTMessage_Read* hRead);

	void		ToggleCrosshair();
	void		EnableCrosshair(LTBOOL b=LTTRUE) { m_bCrosshairEnabled = b; }
	LTBOOL		CrosshairEnabled() const { return m_bCrosshairEnabled; }
	LTBOOL		CrosshairOn() const { return (LTBOOL)(m_nCrosshairLevel != 0); }

	void		SetDrawAmmo(LTBOOL bVal=LTTRUE) { m_bDrawAmmo = bVal; }
	
	uint8		GetCurWeapon() const { return m_nCurrentWeapon; }

	uint32		GetAmmoCount (uint8 nWeapon) const { if (nWeapon >= GUN_MAX_NUMBER) return 0; return m_nAmmo[nWeapon]; }

protected:

	void		InitOnFoot();
	void		InitEnforcer();
	void		InitAkuma();
	void		InitOrdog();
	void		InitPredator();

	void		DrawOnFoot (HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bStatsSizedOff);
	void		DrawEnforcer (HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bStatsSizedOff);
	void		DrawAkuma (HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bStatsSizedOff);
	void		DrawOrdog (HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bStatsSizedOff);
	void		DrawPredator (HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bStatsSizedOff);

	void		UpdateOnFootHealth();
	void		UpdatePredatorHealth();	
	void		UpdateEnforcerHealth();
	void		UpdateOrdogHealth();
	void		UpdateAkumaHealth();
	
	void		UpdateOnFootArmor();
	void		UpdatePredatorArmor();
	void		UpdateEnforcerArmor();
	void		UpdateOrdogArmor();
	void		UpdateAkumaArmor();
	
	void		UpdateOnFootAmmo();
	void		UpdatePredatorAmmo();
	void		UpdateEnforcerAmmo();
	void		UpdateOrdogAmmo();
	void		UpdateAkumaAmmo();

	void		UpdateHUDEffect();

		
protected:

	ILTClient*	m_pClientDE;
	CRiotClientShell* m_pClientShell;

	uint32		m_nHealth;						// current health
	uint32		m_nArmor;						// current armor
	uint32		m_nAmmo[GUN_MAX_NUMBER];		// current ammo
	uint8		m_nCurrentWeapon;				// current weapon

	LTBOOL		m_bHealthChanged;
	LTBOOL		m_bArmorChanged;
	LTBOOL		m_bAmmoChanged;

	HSURFACE	m_hAirMeter;
	LTRect		m_rcAirBar;
	uint32		m_cxAirMeter;
	uint32		m_cyAirMeter;
	LTFLOAT		m_fAirPercent;

	HLTFONT		m_hNumberFont;					// font for number display

	int			m_nCrosshairLevel;
	LTBOOL		m_bCrosshairEnabled;
	LTBOOL		m_bDrawHud;

	uint8		m_nPlayerMode;

	HSURFACE	m_hCrosshair1;
	HSURFACE	m_hCrosshair2;
	HSURFACE	m_hCrosshair3;
	HSURFACE	m_hZoomCrosshair;

	HSURFACE	m_hCleanHUDLeft;
	HSURFACE	m_hCleanHUDMiddle;
	HSURFACE	m_hCleanHUDRight;
	HSURFACE	m_hHUDDataLeft;
	HSURFACE	m_hHUDDataMiddle;
	HSURFACE	m_hHUDDataRight;
	HSURFACE	m_hHUDOverlayLeft;
	HSURFACE	m_hHUDOverlayMiddle;
	HSURFACE	m_hHUDOverlayRight;
	HSURFACE	m_hHUDLeft;
	HSURFACE	m_hHUDMiddle;
	HSURFACE	m_hHUDRight;
	HSURFACE	m_hHUDEffect;
	HSURFACE	m_hHUDNumbers;
	int			m_nNumWidths[10];
	int			m_nNumOffsets[10];

	LTFLOAT		m_nEffectPos;

	HSURFACE	m_hAmmoIcon;
	uint32		m_cxAmmoIcon;
	uint32		m_cyAmmoIcon;

	LTBOOL		m_bDrawAmmo;
};

#endif
