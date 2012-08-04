// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSparksFX.h
//
// PURPOSE : Sparks special fx class - Definition
//
// CREATED : 1/21/99
//
// ----------------------------------------------------------------------- //

#ifndef __RANDOM_SPARKS_FX_H__
#define __RANDOM_SPARKS_FX_H__

#include "BaseParticleSystemFX.h"

struct RANDOMSPARKSCREATESTRUCT : public BPSCREATESTRUCT
{
    RANDOMSPARKSCREATESTRUCT();

    LTVector vPos;
    LTVector vDir;
    uint8   nSparks;
    LTFLOAT  fDuration;
    LTVector vMinVelAdjust;
    LTVector vMaxVelAdjust;
    LTFLOAT  fRadius;
};

inline RANDOMSPARKSCREATESTRUCT::RANDOMSPARKSCREATESTRUCT()
{
	vPos.Init();
	vDir.Init();
	vMinVelAdjust.Init(1, 0.025f, 1);
	vMaxVelAdjust.Init(1, 1, 1);
	nSparks		= 0;
	fDuration	= 0.0f;
	fRadius		= 300.0f;
}


class CRandomSparksFX : public CBaseParticleSystemFX
{
	public :

		CRandomSparksFX() : CBaseParticleSystemFX()
		{
			VEC_INIT(m_vDir);
			m_nSparks	 = 5;
			m_fDuration	 = 1.0f;
			m_fStartTime = 0.0f;
			m_vMinVelAdjust.Init(1, 0.025f, 1);
			m_vMaxVelAdjust.Init(1, 1, 1);
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_RANDOMSPARKS_ID; }

	private :

        LTBOOL AddSparks();

        LTVector m_vDir;             // Direction sparks shoot
        uint8   m_nSparks;          // Number of sparks
        LTFLOAT  m_fDuration;        // Life time of sparks
        LTFLOAT  m_fStartTime;       // When did we start
        LTVector m_vMinVelAdjust;    // How much to adjust the min velocity
        LTVector m_vMaxVelAdjust;    // How much to adjust the max velocity.
};

#endif // __RANDOM_SPARKS_FX_H__