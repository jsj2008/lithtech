// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.h
//
// PURPOSE : Definition of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYERSTATS_H
#define __PLAYERSTATS_H

#include "ltbasedefs.h"
#include "IDList.h"
#include "PlayerStats.h"
#include "SharedMission.h"
#include "MsgIDs.h"

class CPlayerStats
{
public:

	CPlayerStats();
	virtual ~CPlayerStats();

	bool		Init();
	void		Term();
	void		Update();

	void		OnEnterWorld(bool bRestoringGame=false);
	void		OnExitWorld();

	void		UpdatePlayerWeapon( HWEAPON hWeapon, HAMMO hAmmo, bool bForce = false );
	void		UpdatePlayerGrenade( HWEAPON hGrenade, bool bForceDeselect );
	void		ResetStats();
	
	void		Clear();
	void		UpdateHealth(uint32 nHealth);
	void		UpdateArmor(uint32 nArmor);
	void		UpdateMaxHealth(uint32 nHealth);
	void		UpdateMaxArmor(uint32 nArmor);
	void		UpdateProgress(uint32 nProgress) { m_dwProgress = (nProgress >= m_dwMaxProgress ? m_dwMaxProgress : nProgress); }
	void		UpdateMaxProgress(uint32 nMaxProgress) { m_dwMaxProgress = nMaxProgress; }
	void		UpdateAmmo( HWEAPON hWeapon, HAMMO hAmmo, uint32 nAmmo, bool bPickedup, bool bDisplayMsg, uint8 nSlot, bool bInstantChange=false);
	void		UpdateAmmoInClip(HWEAPON hWeapon,uint32 nAmmo);
	void		RemoveWeapon( HWEAPON hWeapon );
	void		UpdateGear( HGEAR hGear, GearMsgType eMsgType, uint8 nCount );
	void		UpdateMod( HMOD hMod );
	void		UpdateAir(float nPercent);
	void		UpdateMissionStats(ILTMessage_Read *pMsg);

	void		ResetMissionStats();

	void		ResetInventory();						 //clears all inventory data
	void		DropInventory(bool bResetGear=true); //drops are currently carried weapons and ammo
														 // also drops gear and mods if bResetGear is set


	void		ClearMissionInfo();
	int			GetMissionDamage()		{return m_nDamage;}


	const MissionStats* GetMissionStats() const {return &m_MissionStats;}

	void		Save(ILTMessage_Write *pMsg);
	void		Load(ILTMessage_Read *pMsg);

	HWEAPON		GetCurWeaponRecord() const { return m_hCurrentWeapon; }
	uint8		GetWeaponSlot( HWEAPON hWeapon );
	HWEAPON		GetWeaponInSlot(uint8 slot);
 
	// [KLS - 02/13/02] Updated to support checking for mods on weapons other than
	// the current weapon...
	HMOD			GetMod( ModType eType, HWEAPON hW = NULL );
	inline HMOD		GetSilencer( HWEAPON hW=NULL)	{ return GetMod(SILENCER, hW); }
	inline HMOD		GetScope( HWEAPON hW=NULL)		{ return GetMod(SCOPE, hW); }

	inline bool	CanBeSilenced() { return (GetSilencer() != NULL); }
	inline bool	CanHaveScope()  { return (GetScope() != NULL); }



	uint32		GetAmmoCount( HAMMO hAmmo ) const;
	uint32		GetAmmoInClip( HWEAPON hWeapon ) const;
	bool		HaveWeapon( HWEAPON hWeapon ) const;
	uint32		GetNumHaveWeapons( ) const { return m_nNumHaveWeapons; }
	bool		HaveMod( HMOD hMod ) const;
	uint8		GetGearCount( HGEAR hGear ) const;
	bool		HaveHadGear(HGEAR hGear) const;

	void		SetWeaponCapacity(uint8 nCap);
	uint8		GetWeaponCapacity() const {return m_nWeaponCapacity; }
	bool		HasEmptySlot() const { return (FindFirstEmptySlot() != WDB_INVALID_WEAPON_INDEX); }
	uint8		FindFirstEmptySlot( ) const;
	bool		UnlimitedCapacity() const;

	void		NextGrenade();
	void		LastGrenade();

	bool	  HaveAirSupply();


	uint32		GetHealth()		{ return m_nHealth;}	// current health
	uint32		GetMaxHealth()	{ return m_nMaxHealth;}	// current maximum health
	uint32		GetArmor()		{ return m_nArmor;}		// current armor
	uint32		GetMaxArmor()	{ return m_nMaxArmor;}	// current maximum armor
	float		GetAirPercent()	{ return m_fAirPercent;}
	uint32		GetProgress()	{ return m_dwProgress; }
	uint32		GetMaxProgress(){ return m_dwMaxProgress;	}

	HWEAPON		GetCurrentWeaponRecord()	const { return m_hCurrentWeapon;}	// current weapon
	HAMMO		GetCurrentAmmoRecord()		const { return m_hCurrentAmmo;}		// current ammo
	HWEAPON		GetCurrentGrenadeRecord()	const { return m_hCurrentGrenade;}	// current grenade
	HWEAPON		GetLastGrenadeRecord()		const { return m_hLastGrenade;}	// last grenade

	uint32		GetCurrentAmmoCount();
	uint32		GetCurrentGrenadeCount();

	void		SetObjective(HRECORD hObjective);
	void		SetMission(const char* pszMission);
	HRECORD 	GetObjective() const {return m_hObjective;}
	const char* GetMission() const {return m_sMission.c_str();}
 

protected:
 
	uint32		m_nHealth;			// current health
	uint32		m_nArmor;			// current armor
	uint32		m_nMaxHealth;		// current maximum health
	uint32		m_nMaxArmor;		// current maximum armor
	float		m_fAirPercent;
	uint32*		m_pnAmmo;			// All ammo values
	bool*		m_pbHaveWeapon;		// Weapon status
	uint32*		m_pnAmmoInClip;		// weapon ammo
	uint32		m_nNumHaveWeapons;	// Count of weapons we have.

	HWEAPON		m_hCurrentWeapon;	// current weapon
	HAMMO		m_hCurrentAmmo;		// current ammo
	HWEAPON		m_hCurrentGrenade;	// current grenade
	HWEAPON		m_hLastGrenade;		// last grenade we had selected

	uint8*		m_pbHaveMod;		// Mod status

	int			m_nDamage;			// Damage taken during this mission

	uint8		m_nWeaponCapacity;

	MissionStats	m_MissionStats;

	uint32		m_dwProgress;
	uint32		m_dwMaxProgress;

	//vector tracking gear items that have been picked up
	typedef std::vector<uint8, LTAllocator<uint8, LT_MEM_TYPE_GAMECODE> > GearArray;
	GearArray m_vecGearCount;
	GearArray m_vecHadGear;

	//set of weapons we are carrying
	typedef std::vector<HWEAPON, LTAllocator<HWEAPON, LT_MEM_TYPE_GAMECODE> > WeaponArray;
	WeaponArray	m_vecWeaponSlots;


	std::string			m_sMission;
	HRECORD				m_hObjective;

};

extern CPlayerStats* g_pPlayerStats;

#endif // __PLAYERSTATS_H
