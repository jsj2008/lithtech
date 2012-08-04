// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.cpp
//
// PURPOSE : FireFX special FX - Definitions
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FIRE_FX_H__
#define __FIRE_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "LightFX.h"

struct FIRECREATESTRUCT : public SFXCREATESTRUCT
{
    FIRECREATESTRUCT();

    LTVector vPos;
    LTVector vLightColor;
    LTVector vLightOffset;
    LTFLOAT  fLightRadius;
    LTFLOAT  fLightPhase;
    LTFLOAT  fLightFreq;
    LTFLOAT  fSoundRadius;
    LTFLOAT  fRadius;
    LTBOOL   bCreateSmoke;
    LTBOOL   bCreateLight;
    LTBOOL   bCreateSparks;
    LTBOOL   bCreateSound;
    LTBOOL   bBlackSmoke;
    LTBOOL   bSmokeOnly;
};

inline FIRECREATESTRUCT::FIRECREATESTRUCT()
{
	vPos.Init();
	vLightColor.Init();
	vLightOffset.Init();
	fLightRadius	= 0.0f;
	fLightPhase		= 0.0f;
	fLightFreq		= 0.0f;
	fSoundRadius	= 0.0f;
	fRadius			= 0.0f;
    bCreateSmoke    = LTFALSE;
    bCreateLight    = LTFALSE;
    bCreateSparks   = LTFALSE;
    bCreateSound    = LTFALSE;
    bBlackSmoke     = LTFALSE;
    bSmokeOnly      = LTFALSE;
}

class CFireFX : public CSpecialFX
{
	public :

		CFireFX() : CSpecialFX()
		{
            m_hSound = LTNULL;
			m_fSizeAdjust = 1.0f;
		}

		~CFireFX()
		{
			if (m_hSound)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hSound);
			}
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_FIRE_ID; }

	private :

		FIRECREATESTRUCT	m_cs;
        HLTSOUND            m_hSound;
        LTFLOAT              m_fSizeAdjust;

		CSmokeFX			m_Smoke1;
		CSmokeFX			m_Fire1;
		CSmokeFX			m_Fire2;
		CLightFX			m_Light;
};

#endif // __FIRE_FX_H__