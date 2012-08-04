// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLightFX.h
//
// PURPOSE : SearchLight special fx class - Definition
//
// CREATED : 6/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_LIGHT_FX_H__
#define __SEARCH_LIGHT_FX_H__

#include "SpecialFX.h"

struct SEARCHLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	SEARCHLIGHTCREATESTRUCT();

    float  fBeamLength;
    float  fBeamRadius;
    float  fBeamAlpha;
    float  fBeamRotTime;
    float  fLightRadius;
    LTVector vLightColor;
    bool   bBeamAdditive;
};

inline SEARCHLIGHTCREATESTRUCT::SEARCHLIGHTCREATESTRUCT()
{
	fBeamLength		= 0.0f;
	fBeamRadius		= 0.0f;
	fBeamAlpha		= 0.0f;
	fBeamRotTime	= 0.0f;
	fLightRadius	= 0.0f;
    bBeamAdditive   = true;
	vLightColor.Init();
}


class CSearchLightFX : public CSpecialFX
{
	public :

		CSearchLightFX() : CSpecialFX()
		{
            m_hBeam     = NULL;
            m_hLight    = NULL;

			m_fBeamRotation = 0.0f;
		}

		~CSearchLightFX()
		{
			if (m_hBeam)
			{
				m_pClientDE->RemoveObject(m_hBeam);
			}

			if (m_hLight)
			{
				m_pClientDE->RemoveObject(m_hLight);
			}
		}

        virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual bool CreateObject(ILTClient* pClientDE);
        virtual bool Update();

		virtual uint32 GetSFXID() { return SFX_SEARCHLIGHT_ID; }

	protected :

		SEARCHLIGHTCREATESTRUCT		m_cs;
		LTObjRef					m_hBeam;
		LTObjRef					m_hLight;

        float                      m_fBeamRotation;
};

#endif // __SEARCH_LIGHT_FX_H__