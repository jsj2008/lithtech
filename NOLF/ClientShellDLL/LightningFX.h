// ----------------------------------------------------------------------- //
//
// MODULE  : LightningFX.h
//
// PURPOSE : Lightning special fx class - Definition
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNING_FX_H__
#define __LIGHTNING_FX_H__

#include "SpecialFX.h"
#include "SoundMgr.h"
#include "SFXMsgIds.h"
#include "TemplateList.h"
#include "PolyLineFX.h"

struct LFXCREATESTRUCT : public SFXCREATESTRUCT
{
    LFXCREATESTRUCT();

	PLFXCREATESTRUCT	lfx;

    LTVector	vLightColor;
    LTBOOL		bOneTimeOnly;
    LTBOOL		bDynamicLight;
    LTBOOL		bPlaySound;
	LTFLOAT		fLightRadius;
	LTFLOAT		fSoundRadius;
    LTFLOAT		fMinDelayTime;
    LTFLOAT		fMaxDelayTime;
};

inline LFXCREATESTRUCT::LFXCREATESTRUCT()
{
	vLightColor.Init();

	fLightRadius	= 0.0f;
	fSoundRadius	= 0.0f;
    bDynamicLight	= LTFALSE;
    bPlaySound		= LTFALSE;
	bOneTimeOnly	= LTFALSE;
    fMinDelayTime	= 0.0f;
    fMaxDelayTime	= 0.0f;
}

struct PolyVert
{
    PolyVert()
	{
		vPos.Init();
		fOffset = 0.0f;
		fPosOffset = 0.0f;
	}

    LTVector vPos;
    LTFLOAT  fOffset;
    LTFLOAT  fPosOffset;
};

typedef CTList<PolyVert*> PolyVertList;


class CLightningFX : public CSpecialFX
{
	public :

		CLightningFX() : CSpecialFX()
		{
            m_hLight            = LTNULL;
			m_fStartTime		= 0.0f;
			m_fEndTime			= 0.0f;
            m_bFirstTime        = LTTRUE;
            m_bPlayedSound      = LTTRUE;
			m_fPlaySoundTime	= 0.0f;

			m_vMidPos.Init();

			m_hstrTexture		= LTNULL;
		}

		~CLightningFX()
		{
            if (g_pLTClient)
			{
				if (m_hLight)
				{
                    g_pLTClient->DeleteObject(m_hLight);
				}

				if (m_hstrTexture)
				{
					g_pLTClient->FreeString(m_hstrTexture);
				}
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_LIGHTNING_ID; }

	protected :

 		LFXCREATESTRUCT	m_cs;

        LTFLOAT      m_fStartTime;
        LTFLOAT      m_fEndTime;
        LTVector     m_vMidPos;

        LTFLOAT     m_fPlaySoundTime;
        LTBOOL      m_bPlayedSound;
        LTBOOL      m_bFirstTime;
 		CString		m_csThunderStr;

		HOBJECT		m_hLight;
		HSTRING		m_hstrTexture;

		CPolyLineFX	m_Line;		// Lightning

        LTBOOL      Setup();
		void		HandleFirstTime();
 		void		UpdateSound();
};

#endif // __LIGHTNING_FX_H__