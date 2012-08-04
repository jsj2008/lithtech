// ----------------------------------------------------------------------- //
//
// MODULE  : BaseParticleSystemFX.h
//
// PURPOSE : BaseParticleSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_PARTICLE_SYSTEM_FX_H__
#define __BASE_PARTICLE_SYSTEM_FX_H__

#include "SpecialFX.h"

#define PSFX_DEFAULT_GRAVITY	-500
#define PSFX_DEFAULT_RADIUS		500

struct BPSCREATESTRUCT : public SFXCREATESTRUCT
{
    BPSCREATESTRUCT();

    LTBOOL   bRelToCameraPos;
    LTFLOAT  fInnerCamRadius;
    LTFLOAT  fOuterCamRadius;
    LTBOOL   bAdditive;
    LTBOOL   bMultiply;
    LTBOOL   bClientControlsPos;
    LTBOOL   bAdjustParticleAlpha;
    LTBOOL   bAdjustParticleScale;
    LTFLOAT  fStartParticleScale;
    LTFLOAT  fEndParticleScale;
    LTFLOAT  fStartParticleAlpha;
    LTFLOAT  fEndParticleAlpha;
};

inline BPSCREATESTRUCT::BPSCREATESTRUCT()
{
    bRelToCameraPos         = LTFALSE;
	fInnerCamRadius			= 0.0f;
	fOuterCamRadius			= 0.0f;
    bAdditive               = LTFALSE;
    bMultiply               = LTFALSE;
    bClientControlsPos      = LTFALSE;
    bAdjustParticleAlpha    = LTFALSE;
    bAdjustParticleScale    = LTFALSE;
	fStartParticleScale		= 0.0f;
	fEndParticleScale		= 0.0f;
	fStartParticleAlpha		= 0.0f;
	fEndParticleAlpha		= 0.0f;
}


class CBaseParticleSystemFX : public CSpecialFX
{
	public :

		CBaseParticleSystemFX() : CSpecialFX()
		{
            m_pTextureName      = LTNULL;
			m_fGravity			= PSFX_DEFAULT_GRAVITY;
			m_fRadius			= PSFX_DEFAULT_RADIUS;
			m_dwFlags			= 0;
			m_fColorScale		= 1.0f;

			m_vRotAmount.Init();
			m_vRotVel.Init();
			m_vPos.Init();
			m_vPosOffset.Init();
			m_vColorRange.Init();
			m_vColor1.Init(255.0f, 255.0f, 255.0f);
			m_vColor2.Init( 255.0f, 255.0f, 255.0f);
            m_rRot.Init();
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
		{
			return CSpecialFX::Init(hServObj, pMsg);
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		const char*	m_pTextureName;			// Name of the particle texture
        LTFLOAT     m_fGravity;             // Gravity of particle system
        LTFLOAT     m_fRadius;              // Radius of particle system
        uint32      m_dwFlags;              // Particle system setup flags
        LTFLOAT     m_fColorScale;          // System starting color scale
        LTVector    m_vPos;                 // Particle system initial pos
        LTRotation	m_rRot;                 // Particle system ininial rotation
        LTVector    m_vRotAmount;           // Amount to rotate
        LTVector    m_vRotVel;              // Rotation velocity
        LTVector    m_vPosOffset;           // Our position offset from server object

        LTVector    m_vColor1;              // Low color value
        LTVector    m_vColor2;              // Low high color value
        LTVector    m_vColorRange;          // Range of particle colors

		BPSCREATESTRUCT	m_basecs;			// Create struct

        virtual LTBOOL SetupSystem();
        virtual void GetRandomColorInRange(LTVector & vColor);
		virtual int	 GetNumParticles(int nNumParticles);
		virtual void RemoveAllParticles();
};

#endif // __BASE_PARTICLE_SYSTEM_FX_H__