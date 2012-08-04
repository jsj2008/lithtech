// ----------------------------------------------------------------------- //
//
// MODULE  : LeashFX.h
//
// PURPOSE : Leash special fx class - Definition
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __Leash_FX_H__
#define __Leash_FX_H__

#include "BasePolyDrawFX.h"
#include "SoundMgr.h"
#include "SFXMsgIds.h"

#define NUM_Leash_SEGMENTS	15

struct LEASHFXCREATESTRUCT : public SFXCREATESTRUCT
{
    LEASHFXCREATESTRUCT();

    LTVector vStartPos;
    LTVector vEndPos;
    LTVector vLeashColor;
    LTFLOAT  fLeashSize;
    uint8   cSegments;
};

inline LEASHFXCREATESTRUCT::LEASHFXCREATESTRUCT()
{
	vStartPos.Init();
	vEndPos.Init();
	vLeashColor.Init();
	fLeashSize	= 0.0f;
	cSegments	= 0;
}

struct LeashVerts
{
    LeashVerts()
	{
		vPos.Init();
	}

    LTVector vPos;
};

class CLeashFX : public CBasePolyDrawFX
{
	public :

		CLeashFX() : CBasePolyDrawFX()
		{
            m_pVerts            = LTNULL;
            m_bFirstTime        = LTTRUE;
		}

		~CLeashFX()
		{
			if (m_pVerts)
			{
				debug_deletea(m_pVerts);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

        virtual LTBOOL Draw(ILTCustomDraw *pDraw);

		LEASHFXCREATESTRUCT	m_cs;
		LeashVerts*			m_pVerts;
        LTBOOL               m_bFirstTime;

        LTBOOL   SetupLeash();

		void	HandleFirstTime();
};

#endif // __Leash_FX_H__