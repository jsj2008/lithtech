// ----------------------------------------------------------------------- //
//
// MODULE  : LensFlareFX.h
//
// PURPOSE : LensFlare special fx class - Definition
//
// CREATED : 5/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LENS_FLARE_FX_H__
#define __LENS_FLARE_FX_H__

#include "SpecialFX.h"

struct LENSFLARECREATESTRUCT : public SFXCREATESTRUCT
{
	LENSFLARECREATESTRUCT();

    LTBOOL InitFromMessage(LENSFLARECREATESTRUCT & lens, ILTMessage_Read *pMsg);

    LTBOOL   bInSkyBox;
    LTBOOL   bCreateSprite;
    LTBOOL   bSpriteOnly;
    LTBOOL   bUseObjectAngle;
    LTBOOL   bSpriteAdditive;
    LTFLOAT  fSpriteOffset;
    LTFLOAT  fMinAngle;
    LTFLOAT  fMinSpriteAlpha;
    LTFLOAT  fMaxSpriteAlpha;
    LTFLOAT  fMinSpriteScale;
    LTFLOAT  fMaxSpriteScale;
	HSTRING	hstrSpriteFile;
    LTBOOL   bBlindingFlare;
    LTFLOAT  fBlindObjectAngle;
    LTFLOAT  fBlindCameraAngle;
    LTFLOAT  fMinBlindScale;
    LTFLOAT  fMaxBlindScale;
};

inline LENSFLARECREATESTRUCT::LENSFLARECREATESTRUCT()
{
    bInSkyBox           = LTFALSE;
    bCreateSprite       = LTFALSE;
    bSpriteOnly         = LTFALSE;
    bUseObjectAngle     = LTFALSE;
    bBlindingFlare      = LTFALSE;
    bSpriteAdditive     = LTTRUE;
	fSpriteOffset		= 0.0f;
	fMinAngle			= 0.0f;
	fMinSpriteAlpha		= 0.0f;
	fMaxSpriteAlpha		= 0.0f;
	fMinSpriteScale		= 0.0f;
	fMaxSpriteScale		= 0.0f;
	fBlindObjectAngle	= 0.0f;
	fBlindCameraAngle	= 0.0f;
	fMinBlindScale		= 0.0f;
	fMaxBlindScale		= 0.0f;
    hstrSpriteFile      = LTNULL;
}

class CLensFlareFX : public CSpecialFX
{
	public :

		CLensFlareFX() : CSpecialFX()
		{
            m_hFlare = LTNULL;
		}

		~CLensFlareFX()
		{
			if (m_hFlare)
			{
				m_pClientDE->RemoveObject(m_hFlare);
			}

			if (m_cs.hstrSpriteFile)
			{
				m_pClientDE->FreeString(m_cs.hstrSpriteFile);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_LENSFLARE_ID; }

	protected :

		LENSFLARECREATESTRUCT		m_cs;
		HOBJECT						m_hFlare;
};

#endif // __LENS_FLARE_FX_H__