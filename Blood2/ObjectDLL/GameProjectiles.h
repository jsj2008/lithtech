// ----------------------------------------------------------------------- //
//
// MODULE  : GameProjectiles.h
//
// PURPOSE : GameProjectiles class - definition
//
// CREATED : 10/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __GAMEPROJECTILES_H__
#define __GAMEPROJECTILES_H__

#include "Projectile.h"

// ----------------------------------------------------------------------- //
// Standard grenades
// ----------------------------------------------------------------------- //

class CGrenade : public CProjectile
{
	public :
		CGrenade();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  HandleTouch(HOBJECT hObj);
	
	private:
		int			m_nBounceCount;
};

#ifdef _ADD_ON

// ----------------------------------------------------------------------- //
// Gas grenades
// ----------------------------------------------------------------------- //

#define		GAS_GRENADE_SMOKE_TIME		10.0f
#define		GAS_GRENADE_DAMAGE_DIST		200.0f
#define		GAS_GRENADE_MAX_DAMAGE		3.0f
#define		GAS_GRENADE_DAMAGE_DELAY	0.5f

class CGasGrenade : public CProjectile
{
	public :
		CGasGrenade();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  HandleTouch(HOBJECT hObj);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);
	
	private:
		void	DamageAndWonky(DFLOAT fWonkyTime);

		DBOOL		m_bSmoking;
		DFLOAT		m_fSmokeTime;
		int			m_nBounceCount;
		DFLOAT		m_fLastWonkyTime;
};

#endif

#ifndef _DEMO // [gjk] 10/28 disable for demo

// ----------------------------------------------------------------------- //
// Time bomb
// ----------------------------------------------------------------------- //

class CTimeBomb : public CProjectile
{
	public :
		CTimeBomb();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);
	
	private:
		float	m_fTimeDelay;
};


// ----------------------------------------------------------------------- //
// Proximity bomb
// ----------------------------------------------------------------------- //

class CProximityBomb : public CProjectile
{
	public :
		CProximityBomb();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);
	
	private:
		float	m_fDetectRadius;
		float	m_fGetAwayTime;
		float	m_fExplodeDelay;
		DBOOL	m_bHitCharacter;
		DBOOL	m_bPlayArmSound;
		DBOOL	m_bPlayDetectSound;
};

// ----------------------------------------------------------------------- //
// Remote bomb
// ----------------------------------------------------------------------- //

class CRemoteBomb : public CProjectile
{
	public :
		CRemoteBomb();
		void	Detonate()		{ m_bExplode = DTRUE; }

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);
	
	private:
		DBOOL	m_bFalling;
};

#endif // _DEMO

// ----------------------------------------------------------------------- //
// Howitzer projectile
// ----------------------------------------------------------------------- //

class CHowitzerShell : public CProjectile
{
	public :
		CHowitzerShell();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);
};

// ----------------------------------------------------------------------- //
// Howitzer Alt projectile
// ----------------------------------------------------------------------- //

class CHowitzerAltShell : public CProjectile
{
	public :
		CHowitzerAltShell();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);
};

// ----------------------------------------------------------------------- //
// Howitzer Alt Fragment projectile
// ----------------------------------------------------------------------- //

class CHowitzerAltFrag : public CProjectile
{
	public :
		CHowitzerAltFrag();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DVector		tempNorm;
};

// ----------------------------------------------------------------------- //
// Napalm Primary Fire Projectile
// ----------------------------------------------------------------------- //

class CNapalmProjectile : public CProjectile
{
	public :
		CNapalmProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Napalm Alt Fire Projectile
// ----------------------------------------------------------------------- //

class CNapalmAltProjectile : public CProjectile
{
	public :
		CNapalmAltProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Napalm Alt Fireball Projectile
// ----------------------------------------------------------------------- //

class CNapalmFireball : public CProjectile
{
	public :
		CNapalmFireball();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DBYTE		m_nBounces;
};

// ----------------------------------------------------------------------- //
// Bug spray primary
// ----------------------------------------------------------------------- //

class CBugSprayProjectile : public CProjectile
{
	public :
		CBugSprayProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Bug spray alt
// ----------------------------------------------------------------------- //

class CBugSprayAltProjectile : public CProjectile
{
	public :
		CBugSprayAltProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Death ray primary
// ----------------------------------------------------------------------- //

#define		DEATHRAY_BOUNCE_DIST		1500.0f
#define		DEATHRAY_BOUNCE_DELAY		0.1f

// ----------------------------------------------------------------------- //

class CDeathRayProjectile : public CProjectile
{
	public :
		CDeathRayProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fBounceDist;
		DVector		m_vBeamDir;
		DBOOL		m_bFirstUpdate;
		DBOOL		m_bFirstShot;
};

// ----------------------------------------------------------------------- //
// Life Leech primary fire projectile
// ----------------------------------------------------------------------- //

class CLeechPrimeProjectile : public CProjectile
{
	public :
		CLeechPrimeProjectile();
	protected:
		virtual DBOOL InitialUpdate(DVector* vec);
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;

		DFLOAT		m_fCurve;
		DFLOAT		m_fAmp;
};

// ----------------------------------------------------------------------- //
// Life Leech alt fire explosion
// ----------------------------------------------------------------------- //

#define		LIFELEECH_PUSH_RADIUS		450.0f
#define		LIFELEECH_ROTATE_AMOUNT		0.2f

// ----------------------------------------------------------------------- //

class CLeechAltProjectile : public CProjectile
{
	public :
		CLeechAltProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
};

// ----------------------------------------------------------------------- //
// Flares
// ----------------------------------------------------------------------- //

class CFlareProjectile : public CProjectile
{
	public :
		CFlareProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DVector		m_vObjOffset;
		DBOOL		m_bTrackObj;

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;

		DFLOAT		m_fLastDamageTime;
		DFLOAT		m_fDamageTime;
		int			m_nColor;
};

// ----------------------------------------------------------------------- //
// Burst flares
// ----------------------------------------------------------------------- //

class CFlareAltProjectile : public CProjectile
{
	public :
		CFlareAltProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fBurstTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;
		DVector		m_vVel;
		DDWORD		m_nExpType;
		DBOOL		m_bGetVelocity;
};

// ----------------------------------------------------------------------- //
// Flare burst fragments
// ----------------------------------------------------------------------- //

class CFlareBurstProjectile : public CProjectile
{
	public :
		CFlareBurstProjectile();
		void	SetColor(DVector *vColor);

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DVector		m_vColor;
};

// ----------------------------------------------------------------------- //
// The Orb Primary Projectile
// ----------------------------------------------------------------------- //

#define		THEORB_DETECT_RADIUS		175.0f
#define		THEORB_ATTACH_RANGE			15.0f
#define		THEORB_ATTACH_RANGE_ALT		45.0f
#define		THEORB_DAMAGE_TIME			5.1f
#define		THEORB_DAMAGE_DELAY			1.0f
#define		THEORB_HEAD_OFFSET			7.5f

#define		THEORB_STATE_TRACKING		0
#define		THEORB_STATE_ATTACHED		1

// ----------------------------------------------------------------------- //

class COrbProjectile : public CProjectile
{
	public :
		COrbProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

		HOBJECT			FindTrackObj();
		void			TrackObjInRange();

		HOBJECT		m_hTrackObj;
		DFLOAT		m_fRotateAmount;
		DFLOAT		m_fHitTime;
		DFLOAT		m_fLastDamageTime;
		DBOOL		m_bState;
};

// ----------------------------------------------------------------------- //
// The Orb Alt Projectile
// ----------------------------------------------------------------------- //

class COrbAltProjectile : public CProjectile
{
	public :
		COrbAltProjectile();

	protected:
		virtual DBOOL	InitialUpdate(DVector* pMovement);
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fHitTime;
		DFLOAT		m_fLastDamageTime;
		DBOOL		m_bState;
		DBOOL		m_bSwitchView;
		DBOOL		m_bCreateCamera;
};

// ----------------------------------------------------------------------- //
// Tesla projectile
// ----------------------------------------------------------------------- //

class CTeslaProjectile : public CProjectile
{
	public :
		CTeslaProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Tesla Bolt projectile
// ----------------------------------------------------------------------- //

class CTeslaBoltProjectile : public CProjectile
{
	public :
		CTeslaBoltProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Tesla Ball projectile
// ----------------------------------------------------------------------- //

#define		TESLABALL_ATTRACT_MIN		100.0f
#define		TESLABALL_ATTRACT_MAX		400.0f
#define		TESLABALL_ATTRACT_RANGE		(TESLABALL_ATTRACT_MAX - TESLABALL_ATTRACT_MIN)
#define		TESLABALL_ATTRACT_TIME		2.5f
#define		TESLABALL_ATTRACT_DELAY		0.5f

#define		TESLABOLT_DAMAGE			100.0f
#define		TESLABOLT_DAMAGERADIUS		100.0f

// ----------------------------------------------------------------------- //

class CTeslaBallProjectile : public CProjectile
{
	public :
		CTeslaBallProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		void			FireBolt();
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fAttractRadius;
		DFLOAT		m_fStartTime;
		DFLOAT		m_fLastFireTime;
		HOBJECT		m_hLastObj;
};

// ----------------------------------------------------------------------- //
// Singularity projectile
// ----------------------------------------------------------------------- //

#define		SINGULARITY_ATTRACT_RADIUS		600.0f
#define		SINGULARITY_ATTRACT_ALT_RADIUS	350.0f
#define		SINGULARITY_KILL_RADIUS			50.0f
#define		SINGULARITY_ATTRACT_TIME		5.0f
#define		SINGULARITY_MAX_DAMAGE			1000.0f

// ----------------------------------------------------------------------- //

class CSingularityProjectile : public CProjectile
{
	public:
		CSingularityProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
};

// ----------------------------------------------------------------------- //

class CSingularityAltProjectile : public CProjectile
{
	public:
		CSingularityAltProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);

		DFLOAT		m_fStartTime;
};

// ----------------------------------------------------------------------- //
// Energy blast projectile
// ----------------------------------------------------------------------- //

class CEnergyBlastProjectile : public CProjectile
{
	public:
		CEnergyBlastProjectile();

		void Setup(DVector *vDir, DBYTE nType, DFLOAT fDamage, DFLOAT fVelocity,
					int nRadius, HOBJECT hFiredFrom);

	protected:
		virtual DBOOL InitialUpdate(DVector* vec);
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;

		DBOOL		m_bZealot;
		DBOOL		m_bDivine;
};

// ----------------------------------------------------------------------- //
// Fireball projectile
// ----------------------------------------------------------------------- //

class CFireballProjectile : public CProjectile
{
	public:
		CFireballProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;
};

// ----------------------------------------------------------------------- //
// Groundstrike projectile
// ----------------------------------------------------------------------- //

class CGroundStrikeProjectile : public CProjectile
{
	public:
		CGroundStrikeProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;
};


// ----------------------------------------------------------------------- //
// Shockwave projectile
// ----------------------------------------------------------------------- //

class CShockwaveProjectile : public CProjectile
{
	public :
		CShockwaveProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
};

// ----------------------------------------------------------------------- //
// Behemoth Shockwave projectile
// ----------------------------------------------------------------------- //

class CBehemothShockwaveProjectile : public CProjectile
{
	public :
		CBehemothShockwaveProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
};

// ----------------------------------------------------------------------- //
// Shikari acid loogie of death
// ----------------------------------------------------------------------- //

class CShikariLoogieProjectile : public CProjectile
{
	public:
		CShikariLoogieProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
		DVector		m_vColor;
};

// ----------------------------------------------------------------------- //
// Naga back spike
// ----------------------------------------------------------------------- //

class CNagaSpikeProjectile : public CProjectile
{
	public:
		CNagaSpikeProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);
};

// ----------------------------------------------------------------------- //
// Naga ceiling debris / shockwave
// ----------------------------------------------------------------------- //

class CNagaDebrisProjectile : public CProjectile
{
	public :
		CNagaDebrisProjectile();
	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);
};

// ----------------------------------------------------------------------- //
// Undead Gideon projectile vomit!
// ----------------------------------------------------------------------- //

class CVomitProjectile : public CProjectile
{
	public:
		CVomitProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// Undead Gideon goo spit
// ----------------------------------------------------------------------- //

class CGooProjectile : public CProjectile
{
	public:
		CGooProjectile();

	protected:
		virtual DBOOL Update(DVector *pMovement);
		virtual void  AddExplosion(DVector vPos, DVector vNormal);

		DFLOAT		m_fStartTime;
		DFLOAT		m_fDelay;
};

// ----------------------------------------------------------------------- //
// The Skull Primary Projectile
// ----------------------------------------------------------------------- //

#define		SKULL_DETECT_RADIUS			200.0f
#define		SKULL_ATTACH_RANGE			15.0f
#define		SKULL_HEAD_OFFSET			7.5f

#define		SKULL_STATE_TRACKING		0

// ----------------------------------------------------------------------- //

class CSkullProjectile : public CProjectile
{
	public :
		CSkullProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

		HOBJECT			FindTrackObj();
		void			TrackObjInRange();

		HOBJECT		m_hTrackObj;
		DFLOAT		m_fRotateAmount;
		DFLOAT		m_fHitTime;
		DFLOAT		m_fLastDamageTime;
		DBOOL		m_bState;
};

#ifdef _ADD_ON

// ----------------------------------------------------------------------- //
// The Flayer Primary Projectile
// ----------------------------------------------------------------------- //

#define		FLAYER_STATE_TRACKING		0
#define		FLAYER_STATE_ATTACHED		1
#define		FLAYER_STATE_RETRACT		2

#define		FLAYER_RETRACT_TIME			1.5f

#define		FLAYER_DAMAGE_INTERVAL		1.0f
#define		FLAYER_TRAP_TIME			8.0f

#define		FLAYER_WIGGLE_VALUES		32

// ----------------------------------------------------------------------- //

class CFlayerChain : public CProjectile
{
	public :
		CFlayerChain();

		virtual void BreakLink(HOBJECT hObj);

		void	SetTarget(HOBJECT hObj)
		{
			if(m_bState == FLAYER_STATE_TRACKING)
			{
				CServerDE* pServerDE = BaseClass::GetServerDE();
				if (!pServerDE) return;
				m_hTrackObj = hObj;
				pServerDE->CreateInterObjectLink(m_hObject, m_hTrackObj);
			}
		}

		void	ReleaseChain();


	protected:
		virtual DBOOL	InitialUpdate(DVector* vec);
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);
		virtual void	AddExplosion(DVector vPos, DVector vNormal);

	private:
		void			TrackObject();
		void			AdjustVelocity();
		DBOOL			DamageAttached();
		void			HandleAttachedEffects();
		void			CreateAttachedGIB();
		void			DeleteAttachedGIB();

	private:
		HOBJECT		m_hTrackObj;
		DFLOAT		m_fHitTime;
		DFLOAT		m_fLastDamageTime;
		DFLOAT		m_fRetractTime;
		DBOOL		m_bState;

		DVector		m_vHookPos1;
		DVector		m_vHookPos2;

		DBYTE		m_byNumLinks;
		DFLOAT		m_fScale;
		DFLOAT		m_fStretchedLength;

		HATTACHMENT	m_hGibAttachment;
		BaseClass	*m_hGibObj;

		DBOOL		m_bFirstUpdate;
};

// ----------------------------------------------------------------------- //
// The Flayer Alt Projectile
// ----------------------------------------------------------------------- //

#define		FLAYER_ALT_SEARCH_RANGE		800.0f
#define		FLAYER_ALT_SEARCH_TIME		20.0f

#define		FLAYER_ALT_CHAIN_INTERVAL	2.0f

// ----------------------------------------------------------------------- //

class CFlayerPortal : public CProjectile
{
	public :
		CFlayerPortal();

	protected:
		virtual DBOOL	InitialUpdate(DVector* vec);
		virtual DBOOL	Update(DVector *pMovement);

	private:
		DFLOAT		m_fSearchTime;
		DFLOAT		m_fLastChainTime;
};

// ----------------------------------------------------------------------- //
// The Gremlin Rock Projectile
// ----------------------------------------------------------------------- //

class CRockProjectile : public CProjectile
{
	public :
		CRockProjectile();

	protected:
		virtual DBOOL	Update(DVector *pMovement);
		virtual void	HandleTouch(HOBJECT hObj);

	private:
		DFLOAT		m_fFadeTime;
		int			m_nBounceCount;
};

// ----------------------------------------------------------------------- //
// The Nightmare Fireballs Projectile
// ----------------------------------------------------------------------- //

class CNightmareFireball : public CProjectile
{
	public :
		CNightmareFireball();

		virtual void BreakLink(HOBJECT hObj);

		void	SetTarget(HOBJECT hObj)
		{
			CServerDE* pServerDE = BaseClass::GetServerDE();
			if (!pServerDE) return;
			m_hTrackObj = hObj;
			pServerDE->CreateInterObjectLink(m_hObject, m_hTrackObj);
		}

	protected:
		virtual DBOOL	InitialUpdate(DVector* vec);
		virtual DBOOL	Update(DVector *pMovement);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

	private:
		void			TrackObject();
		void			TrackLocation();
		void			AdjustVelocity();

	private:
		HOBJECT		m_hTrackObj;
		DVector		m_vHitLocation;
};

#endif

#endif //  __GAMEPROJECTILES_H__