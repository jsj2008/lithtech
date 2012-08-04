//*****************************************************************************//

#ifndef __GAMEWEAPONS_H__
#define __GAMEWEAPONS_H__

//*****************************************************************************//

#include "ViewWeapon.h"

//*****************************************************************************//

class CBeretta : public CViewWeapon
{
	public:
		CBeretta() : CViewWeapon()		{}
};

//*****************************************************************************//

class CShotgun : public CViewWeapon
{
	public:
		CShotgun() : CViewWeapon() 
		{
			m_nOrigFireAnim		= (DDWORD)-1;
			m_bSwitchBarrels	= DFALSE;
			m_bSecondBarrel		= DFALSE;
		}

		void		UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);

	private:
		char*		m_szShotgunShellFilename;
		DDWORD		m_nOrigFireAnim;
		DBOOL		m_bSwitchBarrels;	
		DBOOL		m_bSecondBarrel;
};

//*****************************************************************************//

#ifdef _ADD_ON

class CCombatShotgun : public CViewWeapon
{
	public:
		CCombatShotgun() : CViewWeapon() 	{}

		DBYTE GetAmmoType(DBOOL bAltFire = DFALSE) 
		{ 
			if (!bAltFire)		return m_nAmmoType;
				else			return AMMO_DIEBUGDIE;
		}

	private:
};

#endif

//*****************************************************************************//

class CSniperRifle : public CViewWeapon
{
	public:
		CSniperRifle() : CViewWeapon()		{}
};

//*****************************************************************************//

class CAssaultRifle : public CViewWeapon
{
	public:
		CAssaultRifle() : CViewWeapon()		{}

		DBYTE GetAmmoType(DBOOL bAltFire = DFALSE) 
		{ 
			if (!bAltFire)		return m_nAmmoType;
				else			return AMMO_DIEBUGDIE;
		}
};

//*****************************************************************************//

class CSubMachineGun : public CViewWeapon
{
	public:
		CSubMachineGun() : CViewWeapon()	{}
};

//*****************************************************************************//

class CFlareGun : public CViewWeapon
{
	public:
		CFlareGun() : CViewWeapon()		{}
};

//*****************************************************************************//

class CHowitzer : public CViewWeapon
{
	public:
		CHowitzer() : CViewWeapon()		{}
};

//*****************************************************************************//

class CBugSpray : public CViewWeapon
{
	public:
		CBugSpray() : CViewWeapon()		{}
};

//*****************************************************************************//

class CNapalmCannon : public CViewWeapon
{
	public:
		CNapalmCannon() : CViewWeapon()		{}
};

//*****************************************************************************//

class CMiniGun : public CViewWeapon
{
	public:
		CMiniGun() : CViewWeapon() 
			{ m_bEjectShell = DFALSE; }

	protected:
		// Stub function so minigun doesn't make impact sprites.
		void AddImpact(DVector vPos, DVector vFire, DVector vNormal) {}
};

//*****************************************************************************//

#define		VOODOO_WONKY_DIST		3000.0f
#define		VOODOO_WONKY_FOV		0.65f
#define		VOODOO_PRIMARY_FOV		0.9f

//*****************************************************************************//

class CVoodooDoll : public CViewWeapon
{
	public:
		CVoodooDoll() : CViewWeapon()
		{
			m_nFlags = 0;
			m_nHitType = 0;
			m_nHitFX = 0;
		}

		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags);

	private:
		int		m_nHitType;
		int		m_nHitFX;
};

//*****************************************************************************//

class COrb : public CViewWeapon
{
	public:
		COrb() : CViewWeapon()	{}

		void UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
};

//*****************************************************************************//

#define		DEATHRAY_BEAM_DIST		250.0f
#define		DEATHRAY_DAMAGE_DELAY	0.5f
#define		DEATHRAY_BEAM_FOV		0.9f

//*****************************************************************************//

class CDeathRay : public CViewWeapon
{
	public:
		CDeathRay() : CViewWeapon()
			{ m_fLastDamageTime = 0.0f; }

	private:
		DFLOAT	m_fLastDamageTime;
};

//*****************************************************************************//

class CLifeLeech : public CViewWeapon
{
	public:
		CLifeLeech() : CViewWeapon()
			{ m_nFlags = 0; }

		void UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
};

//*****************************************************************************//

#ifdef _ADD_ON

class CFlayer : public CViewWeapon
{
	public:
		CFlayer() : CViewWeapon()	{}
};

#endif

//*****************************************************************************//

class CTeslaCannon : public CViewWeapon
{
	public:
		CTeslaCannon() : CViewWeapon()
			{ bSprite = DNULL; numAltFires = 0; }

		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags);

	protected:
		DBOOL		bSprite;
		DDWORD		numAltFires;
};

//*****************************************************************************//

class CSingularity : public CViewWeapon
{
	public:
		CSingularity() : CViewWeapon()	{}

		void		UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);

	protected:
		DBOOL		bSprite;
};

//*****************************************************************************//

class CMelee : public CViewWeapon
{
	public:
		CMelee() : CViewWeapon()
			{ m_nDamageType = DAMAGE_TYPE_MELEE; }

		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
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

class CWeapProximityBomb : public CViewWeapon
{
	public:
		CWeapProximityBomb() : CViewWeapon()
			{ m_nFlags = 0; }
		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags);

	private:
		DFLOAT		m_fStartTime;
		DFLOAT		m_fTimeHeld;
};

//*****************************************************************************//

class CWeapRemoteBomb : public CViewWeapon
{
	public:
		CWeapRemoteBomb() : CViewWeapon()
		{
			m_bTriggerBomb = DFALSE;
			m_nFlags = 0;
		}
		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags);

	private:
		DBOOL		m_bTriggerBomb;
		DFLOAT		m_fStartTime;
		DFLOAT		m_fTimeHeld;
};

//*****************************************************************************//

class CWeapTimeBomb : public CViewWeapon
{
	public:
		CWeapTimeBomb() : CViewWeapon()
			{ m_nFlags = 0; }
		void	UpdateFiringState(DVector *firedPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring);
		DBOOL	FireMsgSpecialData(HMESSAGEWRITE &hWrite, DBYTE &byFlags);

	private:
		DFLOAT		m_fStartTime;
		DFLOAT		m_fTimeHeld;
};

//*****************************************************************************//

#endif		// Greg 10/28 - disable bombs for demo


#endif // __GAMEWEAPONS_H__