// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "ObjectUtilities.h"

class CImpact;

class CProjectile : public BaseClass
{
	public :

		CProjectile(DBYTE nType = OT_MODEL);
		virtual ~CProjectile();
		
		virtual void Setup(DVector *vDir, DBYTE nType, DFLOAT fDamage, DFLOAT fVelocity,
							int nRadius, HOBJECT hFiredFrom);
		void SetFlags(DDWORD dwFlags);

		virtual void Explode();
		virtual void BreakLink(HOBJECT hObj);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		virtual void  PostPropRead(ObjectCreateStruct *pStruct);
		virtual DBOOL InitialUpdate(DVector* pMovement);
		virtual DBOOL Update(DVector* pMovement);
		virtual void  HandleTouch(HOBJECT hObj);

		virtual void DamageObjectsWithinRadius();

		virtual void AddImpact(DVector vPos, DVector vNormal, HOBJECT hObj);
		virtual void AddBloodImpact(DVector *vPos, DVector *vNormal, DFLOAT fDamage);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		virtual void AddLight(DFLOAT fRadius, DFLOAT fRed, DFLOAT fGreen, DFLOAT fBlue);
		virtual void AddSmokeTrail(DVector vVel);
		virtual void AddSparks(DVector vPos, DVector vNormal, DFLOAT fDamage, HOBJECT hObject);

	private:

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

	protected:
		// Member Variables

		CDestructable	m_damage;

		char*			m_pProjectileFilename;
		char*			m_pProjectileSkin;

		DBYTE			m_nWeaponType;			// Type of the weapon that fired this, for SFX
		DVector			m_vDir;					// What direction our we moving
		DVector			m_vInitScale;			// Initial scale for the object
		DFLOAT			m_fVelocity;			// What is our velocity
		DDWORD			m_dwFlags;				// Model flags

		DBYTE			m_nDamageType;
		HOBJECT			m_hFiredFrom;			// Who fired us
		HOBJECT			m_hSmokeTrail;
		int				m_nRadius;				// What is our damage radius
		DFLOAT			m_fStartTime;			// When did we start
		DBOOL			m_bRemoveFromWorld;		// Should we be removed?
		DFLOAT			m_fDamage;				// How much damage do we do
		DFLOAT			m_fLifeTime;			// How long do we stay around
		DDWORD			m_nDamageMsgID;			// Message to send for damage, MID_DAMAGE default

		DBOOL			m_bSmokeTrail;			// Do we generate a smoke trail

		DBOOL			m_bShockwave;			// Do we generate a shockwave
		DBOOL			m_bExplosion;			// Do we generate an explosion?
		DBOOL			m_bSparks;				// Do we generate sparks on impact?
		DBOOL			m_bClientFX;			// Do we generate client fx?
		DVector			m_vShockwaveScaleMin;	// Minimum shockwave size
		DVector			m_vShockwaveScaleMax;	// Maximum shockwave size
		DVector			m_vLightColor;			// Color for dynamic light and explosion
		DFLOAT			m_fShockwaveDuration;	// Length of shockwave
		HSTRING			m_hstrShockwaveFilename;	// Name of shockwave  file

		DFLOAT			m_fImpactScaleMin;		// Minimum impact sprite size
		DFLOAT			m_fImpactScaleMax;		// Maximum impact sprite size
		DFLOAT			m_fImpactDuration;		// Duration of impact
		DVector			m_vDims;				// Dimensions of the projectile
		HOBJECT			m_hLight;				// To light the way..
		DBOOL			m_bExplode;				// Explode on the next update
		HOBJECT			m_hHitObject;			// Object we hit

		HSOUNDDE		m_hSound;				// Sound to follow the projectile

		DDWORD			m_dwClientID;			// Client ID of owner (zero if not a player)

		// Various object specific members
		// I put them here so I didn't have to write Save/Load functions for each one.
		// GK
		DVector m_LastPos;
		DBOOL	m_bArmed;
		DFLOAT	m_fPitchVel;
		DFLOAT	m_fYawVel;
		DFLOAT	m_fPitch;
		DFLOAT	m_fYaw;

		// Particle trail variables
		DVector	m_vTrailOffset;
		DFLOAT	m_fTrailScale;
		DDWORD	m_dwTrailScaleFlags;
		DDWORD	m_dwTrailFXID;
		DDWORD	m_dwTrailFXFlags;
};


#endif //  __PROJECTILE_H__