#ifndef __GAMEWEAPONS_H__
#define __GAMEWEAPONS_H__

#include "Weapon.h"
#include "GameProjectiles.h"
#include "ClientLightningSFX.h"
#include "ClientLaserBeamSFX.h"

//*****************************************************************************//

class CBeretta : public CWeapon
{
	public:
		CBeretta() : CWeapon(WEAP_BERETTA)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
			m_bMultiDamageBoost = DTRUE;
		}
};

//*****************************************************************************//

#define BARREL_1 1
#define BARREL_2 2

//*****************************************************************************//

class CShotgun : public CWeapon
{
	public:
		CShotgun() : CWeapon(WEAP_SHOTGUN) 
		{
			m_nOrigFireAnim = (DDWORD)-1;
			m_dwCurBarrel			= BARREL_1;
			m_bPlayImpactSound		= DTRUE;

			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

		DDWORD		Fire();

	private:
		char*		m_szShotgunShellFilename;
		DDWORD		m_nOrigFireAnim;
		DDWORD		m_dwCurBarrel;	
		DBOOL		m_bPlayImpactSound;
};

//*****************************************************************************//

#ifdef _ADD_ON

class CCombatShotgun : public CWeapon
{
	public:
		CCombatShotgun() : CWeapon(WEAP_COMBATSHOTGUN) 
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
			m_bMultiDamageBoost = DTRUE;
		}

		// Alt fire for Assault Rifle is a grenade launcher.
		DBYTE GetAmmoType(DBOOL bAltFire = DFALSE) 
		{ 
			if (!bAltFire) return m_nAmmoType;
			else return AMMO_DIEBUGDIE;
		}

	private:
};

#endif

//*****************************************************************************//

class CSniperRifle : public CWeapon
{
	public:
		CSniperRifle() : CWeapon(WEAP_SNIPERRIFLE)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}
};

//*****************************************************************************//

class CAssaultRifle : public CWeapon
{
	public:
		CAssaultRifle() : CWeapon(WEAP_ASSAULTRIFLE)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
			m_bMultiDamageBoost = DTRUE;
		}

		// Alt fire for Assault Rifle is a grenade launcher.
		DBYTE GetAmmoType(DBOOL bAltFire = DFALSE) 
		{ 
			if (!bAltFire) return m_nAmmoType;
			else return AMMO_DIEBUGDIE;
		}
};

//*****************************************************************************//

class CSubMachineGun : public CWeapon
{
	public:
		CSubMachineGun() : CWeapon(WEAP_SUBMACHINEGUN)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
			m_bMultiDamageBoost = DTRUE;
		}
};

//*****************************************************************************//

class CFlareGun : public CWeapon
{
	public:
		CFlareGun() : CWeapon(WEAP_FLAREGUN)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

		void FireSpread(Spread *spread, DDWORD shots, DFLOAT range, DBOOL bAltFire, DVector *rDir);
};

//*****************************************************************************//

class CHowitzer : public CWeapon
{
	public:
		CHowitzer() : CWeapon(WEAP_HOWITZER)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}
};

//*****************************************************************************//

class CBugSpray : public CWeapon
{
	public:
		CBugSpray() : CWeapon(WEAP_BUGSPRAY)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}
};

//*****************************************************************************//

class CNapalmCannon : public CWeapon
{
	public:
		CNapalmCannon() : CWeapon(WEAP_NAPALMCANNON)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

	protected:
		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CMiniGun : public CWeapon
{
	public:
		CMiniGun() : CWeapon(WEAP_MINIGUN) 
		{
			m_bEjectShell = DFALSE;

			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;

//			m_bMultiDamageBoost = DTRUE;
		}

	protected:
		// Stub function so minigun doesn't make impact sprites.
		void AddImpact(DVector vPos, DVector vFire, DVector vNormal) {}
};

//*****************************************************************************//

#define		VOODOO_WONKY_DIST		3000.0f
#define		VOODOO_WONKY_FOV		0.65f
#define		VOODOO_PRIMARY_FOV		0.9f

//*****************************************************************************//

class CVoodooDoll : public CWeapon
{
	public:
		CVoodooDoll() : CWeapon(WEAP_VOODOO)
		{
			m_nHitType = 0;
			m_nHitFX = 0;
		}

		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
		void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
			{ m_nHitFX = dwSpecial; }

	private:
        DBOOL	SendDamageMsg(HOBJECT firedTo, DVector vPoint, int nNode, DVector *vFire, DFLOAT m_nDamage);
		DBOOL	DamageClosestInFOV();
		DBOOL	WonkyThemAll(DFLOAT fWonkyTime);

		int		m_nHitType;
		int		m_nHitFX;
};

//*****************************************************************************//

class COrb : public CWeapon
{
	public:
		COrb() : CWeapon(WEAP_ORB) 
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		};

	private:
};

//*****************************************************************************//

#define		DEATHRAY_BEAM_DIST		250.0f
#define		DEATHRAY_DAMAGE_DELAY	0.5f
#define		DEATHRAY_BEAM_FOV		0.9f

//*****************************************************************************//

class CDeathRay : public CWeapon
{
	public:
		CDeathRay();
		~CDeathRay();

		DDWORD Fire();

	private:
		void	DamageObjectsInFOV(DFLOAT fRange, DFLOAT fFOV);

		DFLOAT		m_fLastDamageTime;
		BaseClass	*m_pRotModel;
};

//*****************************************************************************//

class CLifeLeech : public CWeapon
{
	public:
		CLifeLeech() : CWeapon(WEAP_LIFELEECH)	{}
};

//*****************************************************************************//

#ifdef _ADD_ON

class CFlayer : public CWeapon
{
	public:
		CFlayer() : CWeapon(WEAP_FLAYER)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

#endif

//*****************************************************************************//

class CTeslaCannon : public CWeapon
{
	public:
		CTeslaCannon() : CWeapon(WEAP_TESLACANNON)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

		void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
			{ numAltFires = dwSpecial; }

		DDWORD			Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);

	protected:
		DDWORD		numAltFires;
};

//*****************************************************************************//

class CSingularity : public CWeapon
{
	public:
		CSingularity() : CWeapon(WEAP_SINGULARITY)
		{
			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}

		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);

	private:
};

//*****************************************************************************//

class CMelee : public CWeapon
{
	public:
		CMelee() : CWeapon(WEAP_MELEE)
		{
			m_nDamageType = DAMAGE_TYPE_MELEE;

			m_nFlags = FLAG_ENVIRONMENTMAP;
			m_fChromeValue = 0.05f;
		}
};

//*****************************************************************************//
// CREATURE WEAPONS
//*****************************************************************************//

class CShikariClaw : public CWeapon
{
	public:
		CShikariClaw() : CWeapon(WEAP_SHIKARI_CLAW) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CShikariSpit : public CWeapon
{
	public:
		CShikariSpit() : CWeapon(WEAP_SHIKARI_SPIT) { m_nDamageType = DAMAGE_TYPE_ACID; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CSoulCrowbar : public CWeapon
{
	public:
		CSoulCrowbar() : CWeapon(WEAP_SOUL_CROWBAR) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CSoulAxe : public CWeapon
{
	public:
		CSoulAxe() : CWeapon(WEAP_SOUL_AXE) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CSoulPipe : public CWeapon
{
	public:
		CSoulPipe() : CWeapon(WEAP_SOUL_PIPE) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CSoulHook : public CWeapon
{
	public:
		CSoulHook() : CWeapon(WEAP_SOUL_HOOK) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CBehemothClaw : public CWeapon
{
	public:
		CBehemothClaw() : CWeapon(WEAP_BEHEMOTH_CLAW) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CEnergyBlast : public CWeapon
{
	public:
		CEnergyBlast() : CWeapon(WEAP_ZEALOT_ENERGYBLAST) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CShockwave : public CWeapon
{
	public:
		CShockwave() : CWeapon(WEAP_ZEALOT_SHOCKWAVE) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};


//*****************************************************************************//

class CFireball : public CWeapon
{
	public:
		CFireball() : CWeapon(WEAP_DRUDGE_FIREBALL) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CNightmareBite : public CWeapon
{
	public:
		CNightmareBite() : CWeapon(WEAP_NIGHTMARE_BITE) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CHandSqueeze : public CWeapon
{
	public:
		CHandSqueeze() : CWeapon(WEAP_HAND_SQUEEZE) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CThiefSuck : public CWeapon
{
	public:
		CThiefSuck() : CWeapon(WEAP_THIEF_SUCK) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CBoneleechSuck : public CWeapon
{
	public:
		CBoneleechSuck() : CWeapon(WEAP_BONELEECH_SUCK) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};

//*****************************************************************************//

class CBehemothShockwave : public CWeapon
{
	public:
		CBehemothShockwave() : CWeapon(WEAP_BEHEMOTH_SHOCKWAVE) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};


//*****************************************************************************//

class CDrudgeLightning : public CWeapon
{
	public:
		CDrudgeLightning() : CWeapon(WEAP_DRUDGE_LIGHTNING)	{ m_nDamageType = DAMAGE_TYPE_NORMAL; }

		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CGroundStrike : public CWeapon
{
	public:
		CGroundStrike() : CWeapon(WEAP_ZEALOT_GROUNDFIRE) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CDeathShroudZap : public CWeapon
{
	public:
		CDeathShroudZap();

		DDWORD	Fire();
		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CZealotHeal : public CWeapon
{
	public:
		CZealotHeal();

		DDWORD	Fire();
		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CZealotShield : public CWeapon
{
	public:
		CZealotShield();

		DDWORD	Fire();
		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CNagaEyes : public CWeapon
{
	public:
		CNagaEyes() : CWeapon(WEAP_NAGA_EYEBEAM)
		{
			m_nFlags = 0;
			m_fChromeValue = 0.0f;
		}

		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);

	private:
};

//*****************************************************************************//

class CNagaSpike : public CWeapon
{
	public:
		CNagaSpike() : CWeapon(WEAP_NAGA_SPIKE) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CNagaDebris : public CWeapon
{
	public:
		CNagaDebris() : CWeapon(WEAP_NAGA_DEBRIS) { m_nDamageType = DAMAGE_TYPE_EXPLODE; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CGideonShield : public CWeapon
{
	public:
		CGideonShield();

		DDWORD	Fire();
		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CGideonWind : public CWeapon
{
	public:
		CGideonWind();

		DDWORD	Fire();
		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CVomit : public CWeapon
{
	public:
		CVomit() : CWeapon(WEAP_GIDEON_VOMIT) { m_nDamageType = DAMAGE_TYPE_FIRE; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CGooSpit : public CWeapon
{
	public:
		CGooSpit() : CWeapon(WEAP_GIDEON_VOMIT) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DDWORD	Fire();
		CProjectile*	FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CGideonSpear : public CWeapon
{
	public:
		CGideonSpear() : CWeapon(WEAP_GIDEON_SPEAR) { m_nDamageType = DAMAGE_TYPE_NORMAL; };

		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CAncientOneBeam : public CWeapon
{
	public:
		CAncientOneBeam() : CWeapon(WEAP_ANCIENTONE_BEAM)
		{
			m_nFlags = 0;
			m_fChromeValue = 0.0f;
		}

		DBOOL	FireVector(DVector *vFire, DFLOAT dist, DBOOL bTracer);
};

//*****************************************************************************//

class CAncientOneTentacle : public CWeapon
{
	public:
		CAncientOneTentacle() : CWeapon(WEAP_ANCIENTONE_TENTACLE) { m_nDamageType = DAMAGE_TYPE_MELEE; }
        
		DDWORD	Fire();
};



#ifndef _DEMO	// Greg 10/28 - disable bombs for demo


//*****************************************************************************//
// INVENTORY WEAPONS
//*****************************************************************************//

#define		BOMBS_MAX_HOLD_TIME		1.25f
#define		BOMBS_MAX_DIST_TIME		0.75f
#define		BOMBS_MIN_VELOCITY		250.0f
#define		BOMBS_VEL_INCREASE		1500.0f


//*****************************************************************************//

class CWeapProximityBomb : public CWeapon
{
	public:
		CWeapProximityBomb() : CWeapon(WEAP_PROXIMITYBOMB) {}
		void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
			{ m_fProjVelocity = fSpecial; }

	protected:
		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CWeapRemoteBomb : public CWeapon
{
	public:
		CWeapRemoteBomb() : CWeapon(WEAP_REMOTEBOMB) {}
		void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
			{ m_fProjVelocity = fSpecial; }

	protected:
		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

class CWeapTimeBomb : public CWeapon
{
	public:
		CWeapTimeBomb() : CWeapon(WEAP_TIMEBOMB) {}
		void	SetSpecialData(DDWORD dwSpecial, DFLOAT fSpecial)
			{ m_fProjVelocity = fSpecial; }

	protected:
		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);
};

//*****************************************************************************//

#endif		// Greg 10/28 - disable bombs for demo

//*****************************************************************************//

class CSkull : public CWeapon
{
	public:
		CSkull() : CWeapon(WEAP_SKULL)	{}

		void UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);

	private:
};

//*****************************************************************************//

#ifdef _ADD_ON

class CGremlinRock : public CWeapon
{
	public:
		CGremlinRock() : CWeapon(WEAP_GREMLIN_ROCK) {}

	protected:
		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);

	private:
};

//*****************************************************************************//

class CNightmareFireballs : public CWeapon
{
	public:
		CNightmareFireballs() : CWeapon(WEAP_NIGHTMARE_FIREBALLS) {}

	protected:
//		CProjectile* FireProjectile(DVector *vFire, DFLOAT dist, DBOOL bAltFire);

	private:
};

#endif

#endif // __GAMEWEAPONS_H__