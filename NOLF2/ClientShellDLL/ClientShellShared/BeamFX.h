// ----------------------------------------------------------------------- //
//
// MODULE  : BeamFX.h
//
// PURPOSE : Tracer special fx class - Definition
//
// CREATED : 5/15/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BEAM_FX_H__
#define __BEAM_FX_H__

#include "SpecialFX.h"
#include "PolyLineFX.h"
#include "FXButeMgr.h"

struct BEAMCREATESTRUCT : public SFXCREATESTRUCT
{
	BEAMCREATESTRUCT();

    LTVector     vStartPos;
    LTVector     vEndPos;
	BEAMFX*		pBeamFX;
};

inline BEAMCREATESTRUCT::BEAMCREATESTRUCT()
{
	vStartPos.Init();
	vEndPos.Init();
    pBeamFX = LTNULL;
}


class CBeamFX : public CSpecialFX
{
	public :

		CBeamFX() : CSpecialFX()
		{
            m_bFirstUpdate  = LTTRUE;
			m_fStartTime	= 0.0f;
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_BEAM_ID; }

	protected :

		BEAMCREATESTRUCT	m_cs;

        LTBOOL           m_bFirstUpdate;
        LTFLOAT          m_fStartTime;

		CPolyLineFX		m_Beam;
};

#endif // __BEAM_FX_H__