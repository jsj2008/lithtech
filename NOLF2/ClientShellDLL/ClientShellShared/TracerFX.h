// ----------------------------------------------------------------------- //
//
// MODULE  : TracerFX.h
//
// PURPOSE : Tracer special fx class - Definition
//
// CREATED : 1/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __TRACER_FX_H__
#define __TRACER_FX_H__

#include "SpecialFX.h"
#include "PolyLineFX.h"
#include "FXButeMgr.h"

struct TRCREATESTRUCT : public SFXCREATESTRUCT
{
	TRCREATESTRUCT();

    LTVector    vStartPos;
    LTVector	vEndPos;
	TRACERFX*	pTracerFX;
};

inline TRCREATESTRUCT::TRCREATESTRUCT()
{
	vStartPos.Init();
	vEndPos.Init();
    pTracerFX = LTNULL;
}


class CTracerFX : public CSpecialFX
{
	public :

		CTracerFX() : CSpecialFX()
		{
            m_bFirstUpdate  = LTTRUE;
			m_fStartTime	= 0.0f;
			m_fDist			= 0.0f;
			m_fLifetime		= 0.0f;
			m_fVelocity		= 0.0f;
			m_vDir.Init(0, 0, 0);
			m_vTracerPos.Init();
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_TRACER_ID; }

	protected :

		TRCREATESTRUCT	m_cs;

        LTBOOL          m_bFirstUpdate;
        LTFLOAT         m_fStartTime;
        LTVector        m_vDir;
        LTFLOAT         m_fDist;
        LTFLOAT         m_fLifetime;
		LTFLOAT			m_fVelocity;
        LTVector        m_vTracerPos;

		CPolyLineFX		m_Tracer;
};

#endif // __TRACER_FX_H__