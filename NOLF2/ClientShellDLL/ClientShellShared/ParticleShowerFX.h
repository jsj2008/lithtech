// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleShowerFX.h
//
// PURPOSE : ParticleShower special fx class - Definition
//			(used to be CSparksFX)
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SHOWER_FX_H__
#define __PARTICLE_SHOWER_FX_H__

#include "BaseParticleSystemFX.h"


struct PARTICLESHOWERCREATESTRUCT : public BPSCREATESTRUCT
{
    PARTICLESHOWERCREATESTRUCT();

    LTVector vPos;
    LTVector vDir;
    LTVector vColor1;
    LTVector vColor2;
    uint8   nParticles;
    LTFLOAT  fDuration;
    LTFLOAT  fEmissionRadius;
    LTFLOAT  fRadius;
    LTFLOAT  fGravity;
	char*	pTexture;
};

inline PARTICLESHOWERCREATESTRUCT::PARTICLESHOWERCREATESTRUCT()
{
	vPos.Init();
	vDir.Init();
	vColor1.Init();
	vColor2.Init();
	nParticles		= 0;
	fDuration		= 0.0f;
	fEmissionRadius	= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;
    pTexture        = LTNULL;
}


class CParticleShowerFX : public CBaseParticleSystemFX
{
	public :

		CParticleShowerFX() : CBaseParticleSystemFX()
		{
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 0.0f);

			m_fStartTime = 0.0f;
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_PARTICLESHOWER_ID; }

	private :

        LTBOOL AddParticles();

		PARTICLESHOWERCREATESTRUCT m_cs;

        LTFLOAT  m_fStartTime;       // When did we start
};

#endif // __PARTICLE_SHOWER_FX_H__