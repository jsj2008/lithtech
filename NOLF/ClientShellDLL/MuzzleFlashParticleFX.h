// ----------------------------------------------------------------------- //
//
// MODULE  : MuzzleFlashParticleFX.h
//
// PURPOSE : MuzzleFlash particle special fx class - Definition
//
// CREATED : 1/17/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MUZZLEFLASH_PARTICLE_FX_H__
#define __MUZZLEFLASH_PARTICLE_FX_H__

#include "BaseParticleSystemFX.h"
#include "FXButeMgr.h"

struct MFPCREATESTRUCT : public BPSCREATESTRUCT
{
    MFPCREATESTRUCT();

	HOBJECT				hFiredFrom;
    LTVector             vPos;
    LTRotation           rRot;
	CParticleMuzzleFX*	pPMuzzleFX;
    LTBOOL               bPlayerView;
};

inline MFPCREATESTRUCT::MFPCREATESTRUCT()
{
	rRot.Init();
	vPos.Init();
    hFiredFrom  = LTNULL;
    pPMuzzleFX  = LTNULL;
    bPlayerView = LTFALSE;
}


class CMuzzleFlashParticleFX : public CBaseParticleSystemFX
{
	public :

		CMuzzleFlashParticleFX() : CBaseParticleSystemFX()
		{
			m_fStartTime = 0.0f;
		}

        LTBOOL Reset(MFPCREATESTRUCT & mfcs);

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

	private :

        LTBOOL   AddMuzzleFlash();

		MFPCREATESTRUCT m_cs;			// Our data
        LTFLOAT          m_fStartTime;   // When did we start
};

#endif // __MUZZLEFLASH_PARTICLE_FX_H__