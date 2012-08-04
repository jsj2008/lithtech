// ----------------------------------------------------------------------- //
//
// MODULE  : VideoFX.cpp
//
// PURPOSE : VideoFX special FX - Definitions
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VIDEO_FX_H__
#define __VIDEO_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "LightFX.h"

struct VIDEOCREATESTRUCT : public SFXCREATESTRUCT
{
    VIDEOCREATESTRUCT();

	char		szVideo[128];
};

inline VIDEOCREATESTRUCT::VIDEOCREATESTRUCT()
{
	szVideo[0] = 0;
}

class CVideoFX : public CSpecialFX
{
	public :

		CVideoFX() : CSpecialFX()
		{
		}

		~CVideoFX()
		{
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_VIDEO_ID; }

	private :

		VIDEOCREATESTRUCT	m_cs;
};

#endif // __VIDEO_FX_H__