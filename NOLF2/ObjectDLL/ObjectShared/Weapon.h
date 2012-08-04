// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.h
//
// PURPOSE : Weapon class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "ltbasedefs.h"
#include "WeaponMgr.h"
#include "ltserverobj.h"
#include "DamageTypes.h"
#include "ServerUtilities.h"

struct WeaponFireInfo;
class ILTServer;
class CWeapons;

#define W_MAX_MODS		4

class CWeapon
{
	public :

		CWeapon();
		virtual ~CWeapon();

		LTBOOL Init(CWeapons* pParent, HOBJECT hObj, int nWeaponId, int nAmmoId);

		CWeapons* GetParent() const { return m_pParent; }
		void SetParent(CWeapons* pParent) { m_pParent = pParent; }

		void Aquire()	{ m_bHave = LTTRUE; }
		void Drop();
		LTBOOL Have()	 const { return m_bHave; }

		void ReloadClip(LTBOOL bPlayReload=LTTRUE, int nNewAmmo=-1, uint8 nAmmoId = WMGR_INVALID_ID);

		int 	GetId() 				const { return m_nWeaponId; }
		int 	GetAmmoId() 			const { return m_nAmmoId; }
		int 	GetAmmoInClip() 		const { return m_nAmmoInClip; }
		WEAPON const*GetWeaponData() 	const { return m_pWeaponData; }
		AMMO const *GetAmmoData()		const { return m_pAmmoData; }

		LTFLOAT  GetInstDamage() const;
		LTFLOAT  GetProgDamage() const;
		LTFLOAT GetLifeTime() const;

		void	SetDamageFactor(LTFLOAT fVal)	 { m_fDamageFactor = fVal; }
		void	SetLifetime(LTFLOAT fLife)		 { m_fLifeTime = fLife; }
		void	SetAmmoId(int nId)				{ m_nAmmoId = nId; m_pAmmoData = g_pWeaponMgr->GetAmmo(nId); }

		HOBJECT GetModelObject();
		void	SetModelObject(HOBJECT hObj);

        virtual WeaponState UpdateWeapon(WeaponFireInfo & fireInfo, LTBOOL bFire);

		void WriteClientInfo(ILTMessage_Write *pMsg);

		virtual void Deselect();
		virtual void Select();

		virtual WeaponState Fire(WeaponFireInfo & info);

		void Save(ILTMessage_Write *pMsg, uint8 nType);
		void Load(ILTMessage_Read *pMsg, uint8 nType);

		WeaponState GetState()	const { return m_eState; }
		HOBJECT GetObject() 	const { return m_hObject; }

		LTBOOL AddMod(int nMod);

		inline MOD const*	GetSilencer() const   { return GetModOfType(SILENCER); }
		inline MOD const*	GetModOfType(ModType eType) const;
		inline LTBOOL HaveMod(uint8 nModId);
		
		// Animation methods...
		
		inline HMODELANIM GetHandAni() const { return m_nHandAni; }
		inline HMODELANIM GetPreFireAni() const { return m_nPreFireAni; }
		inline HMODELANIM GetFireAni() const { return m_nFireAni; }
		inline HMODELANIM GetPostFireAni() const { return m_nPostFireAni; }

		bool	PlayAnimation( HMODELANIM hAni, bool bForce, bool bLoop, bool bReset );
		void	KillLoopSound( );

		float	GetLastFireTime( ) const { return m_fLastFireTime; }

		void	HideWeapon( bool bHidden );
		bool	IsHidden() const { return m_bHidden; }
	
	protected :

		virtual LTBOOL CreateProjectile(LTRotation & rRot,WeaponFireInfo & info);

		LTObjRef	m_hObject;			// Who owns us
		LTObjRef	m_hModelObject; 	// Weapon Model

		WeaponState  m_eState;			// What are we currently doing

		LTBOOL       m_bHave;			// Do we have this weapon
		int          m_nAmmoInClip;		// How much ammo is in our clip

		LTFLOAT      m_fDamageFactor;	// How much damage is adjusted
		LTFLOAT      m_fRangeAdjust; 	// How much the range is adjusted

		LTFLOAT      m_fLifeTime;		// New LifeTime of projectile.

		int          m_nAmmoId;			// What kind of ammo do we use
		int          m_nWeaponId;		// What kind of weapon are we

		CWeapons*    m_pParent;			// Parent container...

		int			m_Mods[W_MAX_MODS];	// Available mods

		WEAPON const *m_pWeaponData;		// Static weapon data
		AMMO const  *m_pAmmoData;			// Static ammo data;

		int          m_nCurTracer;			// Current tracer count

		uint32       m_nLastTimestamp;		// The previous time of firing
		
		LTBOOL		 m_bFirstSelection;	// Is this the first time we are selecting this weapon?

		HMODELANIM	 m_nHandAni;		// Normal, idle, animation
		HMODELANIM	 m_nPreFireAni;
		HMODELANIM	 m_nFireAni;
		HMODELANIM	 m_nPostFireAni;
		
		float		 m_fLastFireTime;
		
		bool		 m_bHidden;			// Is the weapon visible?
};

inline MOD const *CWeapon::GetModOfType(ModType eType) const
{
	MOD const *pMod = LTNULL;
	for (int i=0; i < W_MAX_MODS; i++)
	{
		if (m_Mods[i] != WMGR_INVALID_ID)
		{
			pMod = g_pWeaponMgr->GetMod(m_Mods[i]);
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
		if (m_Mods[i] != WMGR_INVALID_ID)
		{
			if (m_Mods[i] == nModId)
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

#endif // __WEAPON_H__
