// ----------------------------------------------------------------------- //
//
// MODULE  : TO2PlayerStats.h
//
// PURPOSE : Definition of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TO2PLAYERSTATS_H
#define __TO2PLAYERSTATS_H

#include "ltbasedefs.h"
#include "weaponmgr.h"
#include "overlays.h"
#include "IDList.h"
#include "IntelItemList.h"
#include "SkillsButeMgr.h"
#include "PlayerStats.h"
#include "SharedMission.h"


class CTO2PlayerStats : public CPlayerStats
{
public:

	CTO2PlayerStats();
	~CTO2PlayerStats();

	IDList*		GetObjectives()				{ return &m_Objectives; }
	IDList*		GetOptionalObjectives()		{ return &m_OptionalObjectives; }
	IDList*		GetCompletedObjectives()	{ return &m_CompletedObjectives; }
	IDList*		GetParameters()				{ return &m_Parameters; }
	IDList*		GetKeys()					{ return &m_Keys; }
	
	CIntelItemList* GetIntelList()			{ return &m_IntelList; }

    LTBOOL      Init();
	void		Term();
	void		Update();

    void        OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
	void		OnExitWorld();

    void        UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce=LTFALSE);
	void		ResetStats();

	void		Clear();
    void        UpdateHealth(uint32 nHealth);
    void        UpdateArmor(uint32 nArmor);
    void        UpdateMaxHealth(uint32 nHealth);
    void        UpdateMaxArmor(uint32 nArmor);
	void		UpdateEnergy(uint32 nEnergy) {}
	void		UpdateMaxEnergy(uint32 nEnergy) {}
    void		UpdateProgress(uint32 nProgress) { m_dwProgress = (nProgress >= m_dwMaxProgress ? m_dwMaxProgress : nProgress); }
	void		UpdateMaxProgress(uint32 nMaxProgress) { m_dwMaxProgress = nMaxProgress; }
	void        UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId, uint32 nAmmo, LTBOOL bPickedup=LTFALSE, LTBOOL bDisplayMsg=LTTRUE);
    void        UpdateGear(uint8 nGearId);
    void        UpdateMod(uint8 nModId);
    void        UpdateAir(LTFLOAT nPercent);
    void        UpdateObjectives(uint8 nThing, uint8 nType, uint32 dwId);
    void        UpdateKeys(uint8 nType, uint16 dwId);
	void        UpdateIntel(uint32 nTextId, uint8 nPopupId, LTBOOL bIsIntel);
    void        UpdateHiding(LTBOOL bIsHiding, LTBOOL bIsHidden, LTBOOL bCantHide, float fHideDuration);
	void		UpdateSkills(ILTMessage_Read *pMsg);
	void		GainSkills(uint8 nRewardId, uint8 nBonusId, uint16 nAmount);
	void		UpdateMissionStats(ILTMessage_Read *pMsg);

	void		ResetSkills();						 //clears all skill data
	void		ResetObjectives();					 //clears all objective data
	void		ResetMissionStats();

	void		ResetInventory();						 //clears all inventory data
	void		DropInventory(LTBOOL bResetGear=LTTRUE); //drops are currently carried weapons and ammo
														 // also drops gear and mods if bResetGear is set

	void		OnObjectivesDataMessage(ILTMessage_Read *pMsg);

	void		ClearMissionInfo();
	int			GetMissionDamage()		{return m_nDamage;}

	void		SetObjectivesSeen();

	const MissionStats* GetMissionStats() const {return &m_MissionStats;}

	void		Save(ILTMessage_Write *pMsg);
	void		Load(ILTMessage_Read *pMsg);

    uint8       GetCurWeapon() const { return m_nCurrentWeapon; }

	// [KLS - 02/13/02] Updated to support checking for mods on weapons other than
	// the current weapon...
    uint8           GetMod(ModType eType, const WEAPON* pW=LTNULL);
    inline uint8    GetSilencer(const WEAPON* pW=LTNULL)   { return GetMod(SILENCER, pW); }
    inline uint8    GetScope(const WEAPON* pW=LTNULL)      { return GetMod(SCOPE, pW); }

    inline LTBOOL    CanBeSilenced() { return (GetSilencer() != WMGR_INVALID_ID); }
    inline LTBOOL    CanHaveScope()  { return (GetScope() != WMGR_INVALID_ID); }



    uint32       GetAmmoCount(uint8 nAmmoId) const;
    LTBOOL       HaveWeapon(uint8 nWeaponId) const;
    LTBOOL       HaveMod(uint8 nModId) const;
    LTBOOL       HaveGear(uint8 nGearId) const;

    LTBOOL       CanUseWeapon(uint8 nWeaponId) const;
    LTBOOL       CanUseAmmo(uint8 nAmmoId) const;
    LTBOOL       CanUseMod(uint8 nModId) const;
    LTBOOL       CanUseGear(uint8 nGearId) const;


	void		Setup( );

    LTBOOL      HaveAirSupply();


    uint32      GetHealth()		{ return m_nHealth;}	// current health
    uint32      GetMaxHealth()	{ return m_nMaxHealth;}	// current maximum health
    uint32      GetArmor()		{ return m_nArmor;}		// current armor
    uint32      GetMaxArmor()	{ return m_nMaxArmor;}	// current maximum armor
	uint32		GetEnergy()		{ return 0; }
	uint32		GetMaxEnergy()	{ return 0; }
    LTFLOAT		GetAirPercent()	{ return m_fAirPercent;}
	uint32		GetProgress()	{ return m_dwProgress; }
	uint32		GetMaxProgress(){ return m_dwMaxProgress;	}

    uint8       GetCurrentWeapon()	{ return m_nCurrentWeapon;}	// current weapon
    uint8       GetCurrentAmmo()	{ return m_nCurrentAmmo;}	// current ammo

	uint32		GetCurrentAmmoCount();

	LTBOOL		IsHiding()	{return m_bHiding;}
	LTBOOL		IsHidden()	{return m_bHidden;}
	LTBOOL		CanHide()	{return !m_bCantHide; }
	float		GetHideDuration() { return m_fHideDuration;	}

	uint32		GetTotalSkillPoints() {return m_nTotalSkillPoints;}
	uint32		GetAvailSkillPoints() {return m_nAvailSkillPoints;}
	uint8		GetSkillLevel(eSkill skill) {return m_nSkills[skill]; }

	const RANK*	GetRank();
	uint32		GetCostToUpgrade(eSkill skill);
	float		GetSkillModifier(eSkill skl, uint8 nMod); 

protected:

    void        AddCanUseWeapon(uint8 nWeaponId);
    void        AddCanUseAmmo(uint8 nAmmoId);
    void        AddCanUseMod(uint8 nModId);
    void        AddCanUseGear(uint8 nGearId);


protected:

	uint32		m_nHealth;			// current health
	uint32		m_nArmor;			// current armor
	uint32		m_nMaxHealth;		// current maximum health
	uint32		m_nMaxArmor;		// current maximum armor
	LTFLOAT		m_fAirPercent;
	uint32*		m_pnAmmo;			// All ammo values
	LTBOOL*		m_pbHaveAmmo;		// ammos that player had during mission
	LTBOOL*		m_pbHaveWeapon;		// Weapon status
	uint8		m_nCurrentWeapon;	// current weapon
	uint8		m_nCurrentAmmo;		// current ammo
	LTBOOL*		m_pbHaveMod;		// Mod status
	LTBOOL*		m_pbHaveGear;		// Gear status

	uint32		m_nTotalSkillPoints;
	uint32		m_nAvailSkillPoints;
	uint8		m_nSkills[kNumSkills];

	LTBOOL*		m_pbCanUseAmmo;		// Can we use this ammo
	LTBOOL*		m_pbCanUseWeapon;	// Can we carry this Weapon
	LTBOOL*		m_pbCanUseMod;		// Can we carry this Mod
	LTBOOL*		m_pbCanUseGear;		// Can we carry this Gear

	int			m_nDamage;			// Damage taken during this mission

	LTBOOL		m_bHidden;			//player is hidden
	LTBOOL		m_bHiding;			//player is trying to hide
	LTBOOL		m_bCantHide;		//player is unable to hide
	float		m_fHideDuration;	// how long it takes player to become hidden

	IDList	m_Objectives;
	IDList	m_OptionalObjectives;
	IDList	m_CompletedObjectives;
	IDList	m_Parameters;
	IDList	m_Keys;
	
	CIntelItemList	m_IntelList;


	LTBOOL			m_bObjAdded;
	LTBOOL			m_bObjRemoved;
	LTBOOL			m_bObjCompleted;

	MissionStats	m_MissionStats;

	uint32		m_dwProgress;
	uint32		m_dwMaxProgress;

};

#endif // __TO2PLAYERSTATS_H