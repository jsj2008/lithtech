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
#include "DamageTypes.h"
#include "SurfaceTypes.h"
#include "WeaponDefs.h"
#include "Destructable.h"
#include "ModelFuncs.h"

class CWeapon;
class CBaseCharacter;

class CProjectile : public BaseClass
{
	public :

		CProjectile();
		
		void Setup(CWeapon* pWeapon, HOBJECT hFiredFrom);

		HOBJECT	GetFiredFrom() const { return m_hFiredFrom; }

		void SetLifeTime(DFLOAT fTime)		{ m_fLifeTime = fTime; }

		// Explosion accessors...

		DFLOAT	GetExplosionRadius()		const { return m_fRadius; }
		DFLOAT	GetExplosionDamage()		const { return m_fDamage; }
		DFLOAT	GetExplosionDuration()		const { return m_fExplosionDuration; }
		DamageType GetDamageType()			const { return m_eDamageType; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		virtual void HandleTouch(HOBJECT hObj);

		virtual void AddImpact(HOBJECT hObj, DVector vPos, DVector vNormal, SurfaceType eSurfaceType=ST_UNKNOWN);
		virtual void AddExplosion(DVector vPos, DVector vNormal);

		virtual void AddSpecialFX();
		virtual void RemoveObject();

		virtual void HandleImpact(HOBJECT hObj);


		DVector			m_vFirePos;				// Where were we fired from
		DVector			m_vDir;					// What direction our we moving

		DBOOL			m_bSilenced;			// This weapon silenced?
		DBOOL			m_bRemoveFromWorld;		// Should we be removed?
		DBOOL			m_bObjectRemoved;		// Have we been removed?
		DBOOL			m_bSnakingOn;			// Are we snaking?
		DBOOL			m_bFirstSnake;			// First snake calculation
		DBOOL			m_bCanLockOnTarget;		// Can we lock on targets
		DBOOL			m_bDetonated;			// Have we detonated yet?

		ModelSize		m_eModelSize;		    // Relative model size...
		RiotWeaponId	m_nId;					// Type of gun...
		ProjectileType	m_eType;				// Vector or projectile...
		DamageType		m_eDamageType;			// DT_XXX

		DFLOAT			m_fRadius;				// What is our damage radius
		DFLOAT			m_fStartTime;			// When did we start
		DFLOAT			m_fDamage;				// How much damage do we do
		DFLOAT			m_fLifeTime;			// How long do we stay around
		DFLOAT			m_fExplosionDuration;	// How long explosion lasts
		DFLOAT			m_fVelocity;			// What is our velocity
		DFLOAT			m_fMass;				// What is our mass
		DFLOAT			m_fRange;				// Vector weapon range
		DFLOAT			m_fSnakeUpVel;			// Speed of snake up
		DFLOAT			m_fSnakeDir;			// Direction to rotate
		DFLOAT			m_fLockWaitTime;		// How long to wait to lock on target

		HOBJECT			m_hFiredFrom;			// Who fired us
		HOBJECT			m_hLockOnTarget;		// Who we gonna hit?


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables

		CDestructable	m_damage;				// Handle damage

		char*			m_pProjectileFilename;	// Model filename
		char*			m_pProjectileSkin;		// Model skin file
	
		DVector			m_vDims;				// Model dimensions
		DVector			m_vScale;				// Model size

		DDWORD			m_dwFlags;				// Model flags
		DDWORD			m_dwCantDamageTypes;	// What type of damage can't damage us...
		DRotation		m_SnakingRot;			// Used for snaking.

	private :

		void InitialUpdate(int nInfo);
		void Update();
		void Detonate(HOBJECT hObj);
		void LockOnTarget();

		void DoProjectile();
		void DoVector();

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		
		void HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo);
		DBOOL HandlePotentialAIImpact(IntersectInfo & iInfo, DVector & vFrom);
		DBOOL HandlePotentialBodyImpact(IntersectInfo & iInfo, DVector & vFrom);
		DBOOL DoLocationBasedImpact(IntersectInfo & iInfo, DVector & vFrom,
									DDWORD nModelId, ModelSize eModelSize,
									DDWORD & nNode);
		void AdjustDamage(CBaseCharacter* pChar, DDWORD nNode);
};


#endif //  __PROJECTILE_H__