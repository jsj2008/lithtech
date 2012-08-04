// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicLightFX.h
//
// PURPOSE : Dynamic Light special fx class - Definition
//
// CREATED : 2/25/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DYNAMIC_LIGHT_FX_H__
#define __DYNAMIC_LIGHT_FX_H__

#include "SpecialFX.h"


struct DLCREATESTRUCT : public SFXCREATESTRUCT
{
    DLCREATESTRUCT();

    LTVector vColor;
    LTVector vPos;
    LTFLOAT  fMinRadius;
    LTFLOAT  fMaxRadius;
    LTFLOAT  fRampUpTime;
    LTFLOAT  fMaxTime;
    LTFLOAT  fMinTime;
    LTFLOAT  fRampDownTime;
    uint32  dwFlags;
};

inline DLCREATESTRUCT::DLCREATESTRUCT()
{
	vColor.Init();
	vPos.Init();
	fMinRadius		= 0.0f;
	fMaxRadius		= 0.0f;
	fRampUpTime		= 0.0f;
	fMaxTime		= 0.0f;
	fMinTime		= 0.0f;
	fRampDownTime	= 0.0f;
	dwFlags			= 0;
}


class CDynamicLightFX : public CSpecialFX
{
	public :

		CDynamicLightFX() : CSpecialFX()
		{
			VEC_SET(m_vColor, 1.0f, 1.0f, 1.0f);
			m_vPos.Init();
			m_fMinRadius	= 100.0f;
			m_fMaxRadius	= 300.0f;
			m_fRampUpTime	= 1.0f;
			m_fMaxTime		= 1.0f;
			m_fMinTime		= 1.0f;
			m_fRampDownTime	= 1.0f;
			m_dwFlags		= 0;

			m_fStartTime	= 0.0f;
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_DYNAMICLIGHT_ID; }

	private :

        LTVector m_vColor;
        LTVector m_vPos;
        LTFLOAT  m_fMinRadius;
        LTFLOAT  m_fMaxRadius;
        LTFLOAT  m_fRampUpTime;
        LTFLOAT  m_fMaxTime;
        LTFLOAT  m_fMinTime;
        LTFLOAT  m_fRampDownTime;
        uint32  m_dwFlags;

        LTFLOAT  m_fStartTime;       // When did we start
};

#endif // __DYNAMIC_LIGHT_FX_H__