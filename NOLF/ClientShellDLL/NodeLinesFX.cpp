// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLinesFX.cpp
//
// PURPOSE : NodeLines special FX - Implementation
//
// CREATED : 1/21/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "NodeLinesFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::Init
//
//	PURPOSE:	Init the node lines fx
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeLinesFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	NLCREATESTRUCT* pNL = (NLCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vSource, pNL->vSource);
	VEC_COPY(m_vDestination, pNL->vDestination);

    m_bFirstUpdate = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CNodeLinesFX::RemoveFX()
{
	if (m_pFX)
	{
		debug_delete(m_pFX);
        m_pFX = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::Update
//
//	PURPOSE:	Update the node lines
//
// ----------------------------------------------------------------------- //

LTBOOL CNodeLinesFX::Update()
{
    if(!m_pClientDE) return LTFALSE;

	if (m_bFirstUpdate)
	{
		BSCREATESTRUCT ex;

        LTVector vDirection = m_vDestination - m_vSource;
        LTFLOAT fDistance = VEC_MAG(vDirection);

		vDirection.Norm();

		ex.vPos = m_vSource + vDirection*fDistance/2.0f;

		m_pClientDE->AlignRotation(&ex.rRot, &vDirection, NULL);

		VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
		VEC_SET(ex.vInitialScale, 1.0f, 1.0f, fDistance);
		VEC_SET(ex.vFinalScale, 1.0f, 1.0f, fDistance);
		VEC_SET(ex.vInitialColor, 1.0f, 0.0f, 0.0f);
		VEC_SET(ex.vFinalColor, 1.0f, 0.0f, 0.0f);
        ex.bUseUserColors = LTTRUE;

		ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
		ex.fLifeTime		= 500000.0f;
		ex.fInitialAlpha	= 1.0f;
		ex.fFinalAlpha		= 1.0f;
		ex.pFilename		= "Models\\1x1_square.abc";
		ex.nType			= OT_MODEL;
		//ex.pSkin			= "SpecialFX\\Explosions\\Juggernaut.dtx";

		m_pFX = debug_new(CBaseScaleFX);
		if (!m_pFX)
		{
			RemoveFX();
            return LTFALSE;
		}

		m_pFX->Init(&ex);
		m_pFX->CreateObject(m_pClientDE);

        m_bFirstUpdate = LTFALSE;
	}

	if (m_bWantRemove)
	{
		RemoveFX();
        return LTFALSE;
	}

	if (m_pFX)
	{
		m_pFX->Update();
	}

    return LTTRUE;
}