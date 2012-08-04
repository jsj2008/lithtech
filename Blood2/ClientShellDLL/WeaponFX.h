// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.h
//
// PURPOSE : Weapon special effects
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONFX_H__
#define __WEAPONFX_H__

//*******************************************************************************//

#include "SpecialFX.h"
#include "TracerFX.h"
#include "SmokeFX.h"
#include "ContainerCodes.h"
#include "SharedDefs.h"

//*******************************************************************************//

struct WFXCREATESTRUCT : public SFXCREATESTRUCT
{
	WFXCREATESTRUCT::WFXCREATESTRUCT();

	// The location information for the effects
	DVector		vSourcePos;
	DVector		vDestPos;
	DVector		vForward;
	DVector		vNormal;

	// Information about the type of effects to create
	DDWORD		nFXFlags;
	DDWORD		nExtraData;

	// Extra data (not used by all effects)
	DBYTE		nAmmoType;
	DBYTE		nSurfaceType;
	DBYTE		nExplosionType;
	DFLOAT		fDamage;
	DFLOAT		fDensity;
	DVector		vColor1;
	DVector		vColor2;
	DVector		vLightColor1;
	DVector		vLightColor2;
};

//*******************************************************************************//

inline WFXCREATESTRUCT::WFXCREATESTRUCT()
{
	memset(this, 0, sizeof(WFXCREATESTRUCT));
}

//*******************************************************************************//

class CWeaponFX : public CSpecialFX
{
	public :
		// Initialize the rest of the data
		CWeaponFX() : CSpecialFX() 
		{
			m_nFXFlags			= 0;
			m_nExtraData		= 0;

			m_nAmmoType			= 0;
			m_eSurfaceType		= SURFTYPE_UNKNOWN;
			m_nExplosionType	= 0;

			m_fDamage			= 1.0f;
			m_fDensity			= 10.0f;

			m_eSourceCode		= CC_NOTHING;
			m_eDestCode			= CC_NOTHING;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	protected :
	
		DDWORD			m_nFXFlags;			// FX to create
		DDWORD			m_nExtraData;		// Extra data for effects (not used by all)

		DBYTE			m_nAmmoType;		// Type of ammo
		SurfaceType		m_eSurfaceType;		// Surface hit by bullet
		DBYTE			m_nExplosionType;	// Explosion to create at impact point

		DFLOAT			m_fDamage;			// Damage done by bullet
		DFLOAT			m_fDensity;			// Denisty of a particle trail

		DVector			m_vColor1;			// General color 1
		DVector			m_vColor2;			// General color 2
		DVector			m_vLightColor1;		// Light color 1
		DVector			m_vLightColor2;		// Light color 2

		ContainerCode	m_eSourceCode;		// Container fired from within
		ContainerCode	m_eDestCode;		// Container where shot hit

		DVector			m_vSourcePos;		// Position bullet was fired from
		DVector			m_vDestPos;			// Position the effect is created at
		DVector			m_vForward;			// A straight vector from the gun rotation
		DVector			m_vNormal;			// Normal of surface (as a vector)

		DVector			m_vDir;				// Direction the bullet is traveling
		DRotation		m_rRotation;		// Rotation based off the normal of the surface

		void CreateMuzzleSmoke();
		void CreateMuzzleLight();
		void CreateShellCasing();
		void CreateTracer();
		void CreateParticleTrail();

		void CreateMark();
		void CreateFlash();
		void CreateSparks();
		void CreateSplash();
		void CreateSmoke();
		void CreateBloodSplat();
		void CreateBloodSpurt();
		void CreateExplosion();
		void CreateFragments();
		void CreateImpactLight();
		void CreateShakeNFlash();

		void CreateBubbles(DVector* pvStartPos, DBYTE numBubbles);

		void PlayImpactSound();

		char* GetMarkSprite(SurfaceType eSurfType);
		char* GetSparkTexture(SurfaceType eSurfType, DVector* pvColor1, DVector* pvColor2);

		void SetupSmoke(DBYTE nWeaponType, SurfaceType eSurfType, SMCREATESTRUCT* pSM);
		void CreateLightFX(DVector* pvPos, DVector* pvColor);

		char* CWeaponFX::GetImpactSound(DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eSurfType);
};

//*******************************************************************************//

#endif // __WEAPONFX_H__