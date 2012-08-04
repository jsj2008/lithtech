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
#include "ltserverobj.h"
#include "DamageTypes.h"
#include "ServerUtilities.h"
#include "WeaponDB.h"
#include "EngineTimer.h"

struct WeaponFireInfo;
class ILTServer;
class CArsenal;
class CActiveWeapon;

#define W_MAX_MODS		4

class CWeapon
{
	public :

		CWeapon();
		virtual ~CWeapon();

		bool Init( CArsenal *pArsenal, HOBJECT hObj, HWEAPON hWeapon, HAMMO hAmmo );

		CArsenal*	GetArsenal()					const	{ return m_pArsenal; }
		void		SetArsenal( CArsenal *pArsenal )		{ m_pArsenal = pArsenal; }

		CActiveWeapon* GetActiveWeapon( ) const { return m_pActiveWeapon; }
		void		SetActiveWeapon( CActiveWeapon* pActiveWeapon ) { m_pActiveWeapon = pActiveWeapon; }

		void		Aquire() { m_bHave = true; }
		void		Drop();
		bool		Have()	const	{ return m_bHave; }

		void		ReloadClip( bool bPlayReload=true, int32 nNewAmmo = -1, HAMMO hAmmo = NULL );

		HWEAPON		GetWeaponRecord( )	const	{ return m_hWeapon; }
		HWEAPONDATA GetWeaponData() const;
		HAMMO		GetAmmoRecord( )	const	{ return m_hAmmo; }
		HAMMODATA	GetAmmoDataRecord()	const;

		int			GetAmmoInClip() 		const { return m_nAmmoInClip; }

		float		GetInstDamage() const;
		float		GetInstPenetration() const;
		float		GetProgDamage() const;
		float		GetLifeTime() const;

		void		SetDamageFactor(float fVal)	{ m_fDamageFactor = fVal; }
		void		SetLifetime(float fLife)	{ m_fLifeTime = fLife; }
		void		SetAmmo( HAMMO hAmmo )			{ m_hAmmo = hAmmo; }

		virtual WeaponState UpdateWeapon(WeaponFireInfo & fireInfo, bool bFire);

		void WriteClientInfo(ILTMessage_Write *pMsg);

		virtual void Deselect();
		virtual void Select();

		virtual WeaponState Fire(WeaponFireInfo & info);

		void Save(ILTMessage_Write *pMsg, uint8 nType);
		void Load(ILTMessage_Read *pMsg, uint8 nType);

		WeaponState GetState()	const { return m_eState; }
		HOBJECT GetObject() 	const { return m_hObject; }
		HOBJECT GetModelObject( ) const { return m_hModelObject; }
		void	SetModelObject(HOBJECT hObj);

		HOBJECT	GetDualWeaponModelObject( ) const { m_hDualWeaponModel;	}
		void	SetDualWeaponModelObject( HOBJECT hObject );

		bool AddMod( HMOD hMod );

		inline HMOD	GetSilencer() const   { return GetModOfType(SILENCER); }
		inline HMOD	GetModOfType(ModType eType) const;
		inline bool HaveMod( HMOD hMod );
		
		// Animation methods...
		
		inline HMODELANIM GetHandAni() const { return m_nHandAni; }
		inline HMODELANIM GetPreFireAni() const { return m_nPreFireAni; }
		inline HMODELANIM GetFireAni() const { return m_nFireAni; }
		inline HMODELANIM GetPostFireAni() const { return m_nPostFireAni; }
		inline HMODELANIM GetReloadAni() const { return m_nReloadAni; } 

		bool	PlayAnimation( HMODELANIM hAni, bool bForce, bool bLoop );
		void	SetAnimRate( float flRate );
		void	KillLoopSound( );

		StopWatchTimer const&	GetLastFireTime( ) const { return m_LastFireTime; }

		void	HideWeapon( bool bHidden );
		bool	IsHidden() const { return m_bHidden; }

		// breakable weapon functions
		int32	GetHealth() const		{ return m_nHealth;		}
		int32	GetWarnHealth() const	{ return m_nWarnHealth; }
		int32	GetMaxHealth() const	{ return m_nMaxHealth;	}
		void	SetHealth(int32 nNewHealth);
		void	DamageWeapon(int32 nAmount=1);
		void	CheckWeaponIsWeak();

		void	Reset();

	protected :

		virtual bool CreateProjectile(LTRotation & rRot,WeaponFireInfo & info);

		LTObjRef	m_hObject;			// Who owns us
		LTObjRef	m_hModelObject; 	// Weapon Model
		LTObjRef	m_hDualWeaponModel;

		WeaponState	m_eState;			// What are we currently doing

		bool		m_bHave;			// Do we have this weapon
		int			m_nAmmoInClip;		// How much ammo is in our clip

		float		m_fDamageFactor;	// How much damage is adjusted
		float		m_fRangeAdjust; 	// How much the range is adjusted

		float		m_fLifeTime;		// New LifeTime of projectile.

		HWEAPON		m_hWeapon;
		HAMMO		m_hAmmo;

		CArsenal	*m_pArsenal;		// Arsenal the weapon is a part of...

		CActiveWeapon* m_pActiveWeapon;		// Activeweapon we are set to.

		HMOD		m_Mods[W_MAX_MODS];	// Available mods

		int			m_nCurTracer;	// Current tracer count

		uint32		m_nLastTimestamp;	// The previous time of firing
		
		bool		m_bFirstSelection;	// Is this the first time we are selecting this weapon?

		HMODELANIM	m_nHandAni;			// Normal, idle, animation
		HMODELANIM	m_nPreFireAni;
		HMODELANIM	m_nFireAni;
		HMODELANIM	m_nPostFireAni;
		HMODELANIM	m_nReloadAni;
		HMODELANIM	m_nPlayerAni;
		
		StopWatchTimer m_LastFireTime;
		
		bool		m_bHidden;			// Is the weapon visible?

		// breakable weapons properties
		int32		m_nHealth;			// current number of impacts the weapon can still sustain before breaking
		int32		m_nMaxHealth;		// maximum number of impacts the weapon can sustain before breaking
		int32		m_nWarnHealth;		// number of impacts the weapon must sustain before showing HUD feedback
};

inline HMOD CWeapon::GetModOfType( ModType eType ) const
{
	for( int i = 0; i < W_MAX_MODS; ++i )
	{
		if( m_Mods[i] )
		{
			if( g_pWeaponDB->GetModType( m_Mods[i] ) == eType)
			{
				return m_Mods[i];
			}
		}
	}

	return NULL;
}

inline bool CWeapon::HaveMod( HMOD hMod )
{
	for( int i = 0; i < W_MAX_MODS; ++i )
	{
		if( m_Mods[i] )
		{
			if( m_Mods[i] == hMod )
			{
				return true;
			}
		}
	}

	return false;
}

#endif // __WEAPON_H__
