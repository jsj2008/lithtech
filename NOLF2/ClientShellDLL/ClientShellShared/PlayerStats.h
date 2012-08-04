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
#include "overlays.h"
#include "IDList.h"
#include "IntelItemList.h"
#include "SkillsButeMgr.h"
#include "weaponmgr.h"

class CGameClientShell;
struct MissionStats;

class CPlayerStats
{
public:

	virtual IDList*		GetObjectives() = 0;
	virtual IDList*		GetOptionalObjectives() = 0;
	virtual IDList*		GetCompletedObjectives() = 0;
	virtual IDList*		GetParameters() = 0;
	virtual IDList*		GetKeys() = 0;
	
	virtual CIntelItemList* GetIntelList() = 0;

    virtual LTBOOL      Init() = 0;
	virtual void		Term() = 0;

    virtual void        OnEnterWorld(LTBOOL bRestoringGame=LTFALSE) = 0;
	virtual void		OnExitWorld() = 0;
	virtual void		Update() {}

    virtual void        UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce=LTFALSE) = 0;
	virtual void		ResetStats() = 0;

	virtual void		Clear() = 0;
    virtual void        UpdateHealth(uint32 nHealth) = 0;
    virtual void        UpdateArmor(uint32 nArmor) = 0;
    virtual void        UpdateMaxHealth(uint32 nHealth) = 0;
    virtual void        UpdateMaxArmor(uint32 nArmor) = 0;
	virtual void        UpdateEnergy(uint32 nEnergy) = 0;
	virtual void        UpdateMaxEnergy(uint32 nEnergy) = 0;
	virtual void		UpdateProgress(uint32 nProgress) = 0;
	virtual	void		UpdateMaxProgress(uint32 nMaxProgress) = 0;
	
    virtual void        UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId, uint32 nAmmo, LTBOOL bPickedup=LTFALSE, LTBOOL bDisplayMsg=LTTRUE) = 0;
    virtual void        UpdateGear(uint8 nGearId) = 0;
    virtual void        UpdateMod(uint8 nModId) = 0;
    virtual void        UpdateAir(LTFLOAT nPercent) = 0;
    virtual void        UpdateObjectives(uint8 nThing, uint8 nType, uint32 dwId) = 0;
    virtual void        UpdateKeys(uint8 nType, uint16 dwId) = 0;
	virtual void        UpdateIntel(uint32 nTextId, uint8 nPopupId, LTBOOL bIsIntel) = 0;
    virtual void        UpdateHiding(LTBOOL bIsHiding,LTBOOL bIsHidden, LTBOOL bCantHide, float fHideDuration) = 0;
	virtual void		UpdateSkills(ILTMessage_Read *pMsg) = 0;
	virtual void		GainSkills(uint8 nRewardId, uint8 nBonusId, uint16 nAmount) = 0;
	virtual void		UpdateMissionStats(ILTMessage_Read *pMsg) = 0;

	virtual void		ResetSkills() = 0;						 //clears all skill data
	virtual void		ResetObjectives() = 0;					 //clears all objective data
	virtual void		ResetMissionStats() = 0;				 //clears all mission data

	virtual void		ResetInventory() = 0;						 //clears all inventory data
	virtual void		DropInventory(LTBOOL bResetGear=LTTRUE) = 0; //drops are currently carried weapons and ammo
														 // also drops gear and mods if bResetGear is set
	virtual void		OnObjectivesDataMessage(ILTMessage_Read *pMsg) = 0;

	virtual void		ClearMissionInfo() = 0;
	virtual int			GetMissionDamage() = 0;

	virtual void		SetObjectivesSeen() {};
	
	virtual const MissionStats* GetMissionStats() const = 0;

	virtual void		Save(ILTMessage_Write *pMsg) = 0;
	virtual void		Load(ILTMessage_Read *pMsg) = 0;

    virtual uint8       GetCurWeapon() const = 0;
 
	// [KLS - 02/13/02] Updated to support checking for mods on weapons other than
	// the current weapon...
     virtual uint8           GetMod(ModType eType, const WEAPON* pW=LTNULL) = 0;
    virtual inline uint8    GetSilencer(const WEAPON* pW=LTNULL) = 0;
    virtual inline uint8    GetScope(const WEAPON* pW=LTNULL) = 0;

    virtual inline LTBOOL    CanBeSilenced() = 0;
    virtual inline LTBOOL    CanHaveScope() = 0;


    virtual uint32       GetAmmoCount(uint8 nAmmoId) const = 0;
    virtual LTBOOL       HaveWeapon(uint8 nWeaponId) const = 0;
    virtual LTBOOL       HaveMod(uint8 nModId) const = 0;
    virtual LTBOOL       HaveGear(uint8 nGearId) const = 0;

    virtual LTBOOL       CanUseWeapon(uint8 nWeaponId) const = 0;
    virtual LTBOOL       CanUseAmmo(uint8 nAmmoId) const = 0;
    virtual LTBOOL       CanUseMod(uint8 nModId) const = 0;
    virtual LTBOOL       CanUseGear(uint8 nGearId) const = 0;


	virtual void		Setup( ) = 0;
 
    virtual LTBOOL      HaveAirSupply() = 0;
 


    virtual uint32      GetHealth() = 0;
    virtual uint32      GetMaxHealth() = 0;
    virtual uint32      GetArmor() = 0;
    virtual uint32      GetMaxArmor() = 0;
	virtual uint32		GetEnergy() = 0;
	virtual uint32		GetMaxEnergy() = 0;
    virtual LTFLOAT		GetAirPercent() = 0;
	virtual uint32		GetProgress() = 0;
	virtual uint32		GetMaxProgress() = 0;
 
    virtual uint8       GetCurrentWeapon() = 0;
    virtual uint8       GetCurrentAmmo() = 0;

	virtual uint32		GetCurrentAmmoCount() = 0;
 
	virtual LTBOOL		IsHiding() = 0;
	virtual LTBOOL		IsHidden() = 0;
	virtual LTBOOL		CanHide() = 0;
	virtual float		GetHideDuration() = 0;

	virtual uint32		GetTotalSkillPoints() = 0;
	virtual uint32		GetAvailSkillPoints() = 0;
	virtual uint8		GetSkillLevel(eSkill skill) = 0;
 
	virtual const RANK*	GetRank() = 0;
	virtual uint32		GetCostToUpgrade(eSkill skill) = 0;

	virtual float		GetSkillModifier(eSkill skl, uint8 nMod) = 0; 
};

extern CPlayerStats* g_pPlayerStats;


#endif
