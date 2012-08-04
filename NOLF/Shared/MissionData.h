// MissionData.h: interface for the CMissionData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MISSIONDATA_H__C0F11D60_3F7F_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_MISSIONDATA_H__C0F11D60_3F7F_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdlith.h"
#include "WeaponMgr.h"

class CAmmoData
{
  public:

	 CAmmoData() {m_nID = WMGR_INVALID_ID; m_nCount = 0;}
	virtual ~CAmmoData() {}

    inline void WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
	{
        pInterface->WriteToMessageFloat(hWrite, (LTFLOAT)m_nID);
        pInterface->WriteToMessageFloat(hWrite, (LTFLOAT)m_nCount);
	}

    inline void ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hRead)
	{
        m_nID    = (int) pInterface->ReadFromMessageFloat(hRead);
        m_nCount = (int) pInterface->ReadFromMessageFloat(hRead);
	}

	int		m_nID;
	int		m_nCount;
};

class CWeaponData
{
  public:

	CWeaponData() {m_nID = WMGR_INVALID_ID;}
	virtual ~CWeaponData() {}

    inline void WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
	{
        pInterface->WriteToMessageFloat(hWrite, (LTFLOAT)m_nID);
	}

    inline void ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hRead)
	{
        m_nID = (int) pInterface->ReadFromMessageFloat(hRead);
	}

	int		m_nID;
};

class CModData
{
  public:

	CModData() {m_nID = WMGR_INVALID_ID;}
	virtual ~CModData() {}

    inline void WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
	{
        pInterface->WriteToMessageFloat(hWrite, (LTFLOAT)m_nID);
	}

    inline void ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hRead)
	{
        m_nID = (int) pInterface->ReadFromMessageFloat(hRead);
	}

	int		m_nID;
};

class CGearData
{
  public:

	CGearData() {m_nID = WMGR_INVALID_ID;}
	virtual ~CGearData() {}

    inline void WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
	{
        pInterface->WriteToMessageFloat(hWrite, (LTFLOAT)m_nID);
	}

    inline void ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hRead)
	{
        m_nID       = (int) pInterface->ReadFromMessageFloat(hRead);
	}

	int		m_nID;
};

class CMissionData
{
public:
	CMissionData();
	virtual ~CMissionData();

    void        WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hMessage);
    void        ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hMessage);

	void		NewMission(int mission);
	int			GetMissionNum()	const {return m_nMission;}
	int			GetLevelNum()	const {return m_nLevel;}
	void		SetLevelNum(int nLevel)	{m_nLevel = nLevel;}

	void		ClearWeapons();
	void		ClearGadgets();
    LTBOOL       AddWeapon(int weaponID);
    LTBOOL       RemoveWeapon(int weaponID);
	int			GetWeapons(CWeaponData **weapons, int nArraySize);
	int			GetNumWeapons();
	int			GetNumGadgets();
	CWeaponData* GetWeaponData(int nID);

	void		ClearAmmo();
	void		ClearSupplies();
    LTBOOL       AddAmmo(int ammoID, int count);
    LTBOOL       RemoveAmmo(int ammoID);
	int			GetAmmo(CAmmoData **ammo, int nArraySize);
	int			GetNumAmmoTypes() {return (int)m_Ammo.GetSize();}
	CAmmoData*	GetAmmoData(int nID);

	void		ClearMods();
    LTBOOL       AddMod(int modID);
    LTBOOL       RemoveMod(int modID);
	int			GetMods(CModData **mod, int nArraySize);
	int			GetNumModTypes() {return (int)m_Mods.GetSize();}
	CModData*	GetModData(int nID);

	void		ClearGear();
    LTBOOL       AddGear(int modID);
    LTBOOL       RemoveGear(int modID);
	int			GetGear(CGearData **mod, int nArraySize);
	int			GetNumGearTypes() {return (int)m_Gear.GetSize();}
	CGearData*	GetGearData(int nID);

	void		Clear();
	void		ClearWeaponsAndGadgets();
	void		ClearAllAmmo();

private:
	int m_nMission;
	int m_nLevel;

	CMoArray<CWeaponData*> m_Weapons;
	CMoArray<CAmmoData*> m_Ammo;
	CMoArray<CModData*> m_Mods;
	CMoArray<CGearData*> m_Gear;

	DWORD	GetAmmoIndex(int nID);
	DWORD	GetWeaponIndex(int nID);
	DWORD	GetModIndex(int nID);
	DWORD	GetGearIndex(int nID);


};

#endif // !defined(AFX_MISSIONDATA_H__C0F11D60_3F7F_11D3_B2DB_006097097C7B__INCLUDED_)