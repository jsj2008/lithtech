// ----------------------------------------------------------------------- //
//
// MODULE  : BaseParticleSystemFX.h
//
// PURPOSE : BaseParticleSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_PARTICLE_SYSTEM_FX_H__
#define __BASE_PARTICLE_SYSTEM_FX_H__

#include "SpecialFX.h"

#define PSFX_DEFAULT_GRAVITY	-500
#define PSFX_DEFAULT_RADIUS		500

class CBaseParticleSystemFX : public CSpecialFX
{
	public :

		CBaseParticleSystemFX() : CSpecialFX() 
		{
			m_pTextureName	= DNULL;
			m_fGravity		= PSFX_DEFAULT_GRAVITY; 
			m_fRadius		= PSFX_DEFAULT_RADIUS;
			m_dwFlags		= 0;
			m_fColorScale	= 1.0f;
			m_bSetSoftwareColor = DTRUE;
			VEC_INIT(m_vRotAmount);
			VEC_INIT(m_vRotVel);
			VEC_INIT(m_vPos);
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);
			ROT_INIT(m_rRot);
			VEC_INIT(m_vColorRange);
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		DBOOL	m_bSetSoftwareColor;

	protected :

		char*	m_pTextureName;			// Name of the particle texture
		DFLOAT	m_fGravity;				// Gravity of particle system 
		DFLOAT	m_fRadius;				// Radius of particle system
		DDWORD	m_dwFlags;				// Particle system setup flags
		DFLOAT	m_fColorScale;			// System starting color scale
		DVector m_vPos;					// Particle system initial pos
		DRotation m_rRot;				// Particle system ininial rotation
		DVector	m_vRotAmount;			// Amount to rotate
		DVector	m_vRotVel;				// Rotation velocity

		DVector	m_vColor1;				// Low color value
		DVector	m_vColor2;				// Low high color value
		DVector	m_vColorRange;			// Range of particle colors

		virtual void GetRandomColorInRange(DVector & vColor);
};

#endif // __BASE_PARTICLE_SYSTEM_FX_H__