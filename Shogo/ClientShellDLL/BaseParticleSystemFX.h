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
			m_pTextureName	= LTNULL;
			m_fGravity		= PSFX_DEFAULT_GRAVITY; 
			m_fRadius		= PSFX_DEFAULT_RADIUS;
			m_dwFlags		= 0;
			m_fColorScale	= 1.0f;
			m_bSetSoftwareColor = LTTRUE;
			VEC_INIT(m_vRotAmount);
			VEC_INIT(m_vRotVel);
			VEC_INIT(m_vPos);
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);
			m_rRot.Init();
			VEC_INIT(m_vColorRange);
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();
		virtual LTBOOL CreateObject(ILTClient* pClientDE);

		LTBOOL	m_bSetSoftwareColor;

	protected :

		char*	m_pTextureName;			// Name of the particle texture
		LTFLOAT	m_fGravity;				// Gravity of particle system 
		LTFLOAT	m_fRadius;				// Radius of particle system
		uint32	m_dwFlags;				// Particle system setup flags
		LTFLOAT	m_fColorScale;			// System starting color scale
		LTVector m_vPos;					// Particle system initial pos
		LTRotation m_rRot;				// Particle system ininial rotation
		LTVector	m_vRotAmount;			// Amount to rotate
		LTVector	m_vRotVel;				// Rotation velocity

		LTVector	m_vColor1;				// Low color value
		LTVector	m_vColor2;				// Low high color value
		LTVector	m_vColorRange;			// Range of particle colors

		virtual void GetRandomColorInRange(LTVector & vColor);
};

#endif // __BASE_PARTICLE_SYSTEM_FX_H__