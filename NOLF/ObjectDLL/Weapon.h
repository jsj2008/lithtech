// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.h
//
// PURPOSE : Weapon class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "ltbasedefs.h"
#include "WeaponMgr.h"
#include "ltserverobj.h"
#include "DamageTypes.h"
#include "ServerUtilities.h"
#include "MissionData.h"

class ILTServer;
class CPVWeaponModel;
class CWeapons;

#define W_SOUND_RADIUS	2000.0f
#define W_MAX_MODS		4

struct WFireInfo
{
	WFireInfo()
	{
        hFiredFrom = LTNULL;
        hTestObj = LTNULL;
		vPath.Init();
		vFirePos.Init();
		vFlashPos.Init();
		nSeed = 0;
        bAltFire = LTFALSE;
		fPerturbU = 1.0f;	// 0.0 - 1.0
		fPerturbR = 1.0f;	// 0.0 - 1.0
		bOverrideVelocity = LTFALSE;
		fOverrideVelocity = 0.0f;
		nFireTimestamp = 0;
	}

	HOBJECT		hFiredFrom;
	HOBJECT		hTestObj;
    LTVector	vPath;
    LTVector	vFirePos;
    LTVector	vFlashPos;
    uint8		nSeed;
    LTBOOL		bAltFire;
    LTFLOAT		fPerturbU;
    LTFLOAT		fPerturbR;
	LTBOOL		bOverrideVelocity;
	LTFLOAT		fOverrideVelocity;
	uint32		nFireTimestamp;
};

class CWeapon
{
	public :

		CWeapon();
		virtual ~CWeapon();

        LTBOOL Init(CWeapons* pParent, HOBJECT hObj, int nWeaponId, int nAmmoId);

		CWeapons* GetParent() const { return m_pParent; }
		void SetParent(CWeapons* pParent) { m_pParent = pParent; }

        void Aquire()   { m_bHave = LTTRUE; }
        void Drop()     { m_bHave = LTFALSE; }
        LTBOOL Have()    const { return m_bHave; }

        void ReloadClip(LTBOOL bPlayReload=LTTRUE, int nNewAmmo=-1);

		int		GetId()					const { return m_nWeaponId; }
		int		GetAmmoId()				const { return m_nAmmoId; }
		int		GetAmmoInClip()			const { return m_nAmmoInClip; }
		WEAPON*	GetWeaponData()			const { return m_pWeaponData; }
		AMMO*	GetAmmoData()			const { return m_pAmmoData; }

        LTFLOAT  GetInstDamage() const;
        LTFLOAT  GetProgDamage() const;
        inline LTFLOAT GetLifeTime();

		void    SetDamageFactor(LTFLOAT fVal)    { m_fDamageFactor = fVal; }
        void    SetLifetime(LTFLOAT fLife)       { m_fLifeTime = fLife; }
		void	SetAmmoId(int nId)				{ m_nAmmoId = nId; m_pAmmoData = g_pWeaponMgr->GetAmmo(nId); }

		HOBJECT GetModelObject();
		void	SetModelObject(HOBJECT hObj);

        virtual WeaponState UpdateWeapon(WFireInfo & fireInfo, LTBOOL bFire);

		void WriteClientInfo(HMESSAGEWRITE hMessage);

		virtual void Deselect();
		virtual void Select();

		virtual WeaponState Fire(WFireInfo & info);

        void Save(HMESSAGEWRITE hWrite, uint8 nType);
        void Load(HMESSAGEREAD hRead, uint8 nType);

		WeaponState GetState()	const { return m_eState; }
		HOBJECT GetObject()		const { return m_hObject; }

		void CacheFiles();
        LTBOOL AddMod(CModData* pMod);

		inline MOD*	GetSilencer()   { return GetModOfType(SILENCER); }
		inline MOD*	GetLaser()	    { return GetModOfType(LASER); }
		inline MOD*	GetModOfType(ModType eType);
		inline LTBOOL HaveMod(uint8 nModId);

	protected :

        virtual LTBOOL CreateProjectile(LTRotation & rRot,WFireInfo & info);

        LTFLOAT AIDifficultyAdjust() const;

		HOBJECT		m_hObject;			// Who owns us
		HOBJECT		m_hModelObject;		// Weapon Model

		WeaponState m_eState;			// What are we currently doing

        LTVector	m_vLastFirePath;    // Last direction fired

        LTBOOL		m_bHave;            // Do we have this weapon
		int			m_nAmmoInClip;		// How much ammo is in our clip

        LTFLOAT		m_fDamageFactor;    // How much damage is adjusted
        LTFLOAT		m_fRangeAdjust;     // How much the range is adjusted

        LTFLOAT		m_fLifeTime;        // New LifeTime of projectile.

		int			m_nAmmoId;			// What kind of ammo do we use
		int			m_nWeaponId;		// What kind of weapon are we

		CWeapons* m_pParent;			// Parent container...

		CModData m_Mods[W_MAX_MODS];	// Available mods

		WEAPON*	m_pWeaponData;			// Static weapon data
		AMMO*	m_pAmmoData;			// Static ammo data;

		int		m_nCurTracer;			// Current tracer count

		uint32	m_nLastTimestamp;		// The previous time of firing
};

inline LTFLOAT CWeapon::GetLifeTime()
{
	if (!g_pWeaponMgr) return 0.0f;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo || !pAmmo->pProjectileFX) return 0.0f;


	return m_fLifeTime < 0.0f ? pAmmo->pProjectileFX->fLifeTime : m_fLifeTime;
}

inline MOD* CWeapon::GetModOfType(ModType eType)
{
    MOD* pMod = LTNULL;
	for (int i=0; i < W_MAX_MODS; i++)
	{
		if (m_Mods[i].m_nID != WMGR_INVALID_ID)
		{
			pMod = g_pWeaponMgr->GetMod(m_Mods[i].m_nID);
			if (pMod && pMod->eType == eType)
			{
				return pMod;
			}
		}
	}

    return LTNULL;
}

inline LTBOOL CWeapon::HaveMod(uint8 nModId)
{
	for (int i=0; i < W_MAX_MODS; i++)
	{
		if (m_Mods[i].m_nID != WMGR_INVALID_ID)
		{
			if (m_Mods[i].m_nID == nModId)
			{
				return LTTRUE;
			}
		}
	}

    return LTFALSE;
}

#endif // __WEAPON_H__