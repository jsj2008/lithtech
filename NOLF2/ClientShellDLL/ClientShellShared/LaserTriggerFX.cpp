// ----------------------------------------------------------------------- //
//
// MODULE  : LaserTriggerFX.cpp
//
// PURPOSE : LaserTrigger FX - Implementation
//
// CREATED : 2/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LaserTriggerFX.h"
#include "VarTrack.h"
#include "GameClientShell.h"
#include "VisionModeMgr.h"

VarTrack	g_vtLaserTriggerAlpha;

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserTriggerFX::Init
//
//	PURPOSE:	Init the laser trigger
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserTriggerFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	m_cs.hServerObj = hServObj;
    m_cs.Read(pMsg);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserTriggerFX::Init
//
//	PURPOSE:	Init the laser trigger fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserTriggerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((LTCREATESTRUCT*)psfxCreateStruct);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserTriggerFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserTriggerFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	if (!g_vtLaserTriggerAlpha.IsInitted())
	{
        g_vtLaserTriggerAlpha.Init(g_pLTClient, "LTAlpha", NULL, 0.01f);
	}

    if (!CalcBeamCoords()) return LTFALSE;

	m_pls.vInnerColorStart	= m_cs.vColor;
	m_pls.vInnerColorEnd	= m_cs.vColor;
    m_pls.vOuterColorStart  = LTVector(0, 0, 0);
    m_pls.vOuterColorEnd    = LTVector(0, 0, 0);
	m_pls.fAlphaStart		= m_cs.fAlpha;
	m_pls.fAlphaEnd			= m_cs.fAlpha;
	m_pls.fMinWidth			= 0;
	m_pls.fMaxWidth			= 50;
	m_pls.fMinDistMult		= 1.0f;
	m_pls.fMaxDistMult		= 1.0f;
	m_pls.fLifeTime			= 10000000.0f;
	m_pls.fAlphaLifeTime	= 10000000.0f;
	m_pls.fPerturb			= 0.0f;
    m_pls.bAdditive         = LTFALSE;
	m_pls.nWidthStyle		= PLWS_CONSTANT;
    m_pls.bAlignUp          = LTTRUE;
	m_pls.nNumSegments		= 1;
    m_pls.bNoZ              = LTTRUE;

	m_Beam.Init(&m_pls);
	m_Beam.CreateObject(m_pClientDE);


	// Create the sprites if requested...

	if (m_cs.bCreateSprite && m_cs.hstrSpriteFilename)
	{
		BSCREATESTRUCT bs;

		bs.fLifeTime	= 1000000.0f;
		bs.fInitialAlpha= 1.0f;
		bs.fFinalAlpha	= 1.0f;
        bs.pFilename    = g_pLTClient->GetStringData(m_cs.hstrSpriteFilename);
		bs.dwFlags		= FLAG_VISIBLE | FLAG_GLOWSPRITE | FLAG_SPRITEBIAS;
        bs.bAdditive    = LTTRUE;
		bs.nType		= OT_SPRITE;
		bs.vInitialScale.Init(m_cs.fSpriteScale, m_cs.fSpriteScale, 1.0f);
		bs.vFinalScale.Init(m_cs.fSpriteScale, m_cs.fSpriteScale, 1.0f);

		bs.vPos = m_pls.vStartPos;
		m_StartSprite.Init(&bs);
		m_StartSprite.CreateObject(m_pClientDE);

		bs.vPos = m_pls.vEndPos;
		m_EndSprite.Init(&bs);
		m_EndSprite.CreateObject(m_pClientDE);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserTriggerFX::Update
//
//	PURPOSE:	Update the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserTriggerFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	// Hide/show the beam if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			m_Beam.SetFlags(m_Beam.GetFlags() & ~FLAG_VISIBLE);

			// Update the sprites...

			if (m_cs.bCreateSprite)
			{
				HOBJECT hObj = m_StartSprite.GetObject();
				if (hObj)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, 0, FLAG_VISIBLE);
				}

				hObj = m_EndSprite.GetObject();
				if (hObj)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, 0, FLAG_VISIBLE);
				}
			}

            return LTTRUE;
		}
		else
		{
			m_Beam.SetFlags(m_Beam.GetFlags() | FLAG_VISIBLE);

			// Update the sprites...

			if (m_cs.bCreateSprite)
			{
				HOBJECT hObj = m_StartSprite.GetObject();
				if (hObj)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
				}

				hObj = m_EndSprite.GetObject();
				if (hObj)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
				}
			}
		}
	}


	// Move the beam to the correct position...

	if (CalcBeamCoords())
	{
        LTBOOL bShowSprites = LTFALSE;
		m_pls.fAlphaStart  = g_vtLaserTriggerAlpha.GetFloat();
        m_pls.bAdditive    = LTFALSE;

		// See if this client can clearly see laser triggers...

		if (g_pPlayerMgr->GetVisionModeMgr()->GetMode() == eVM_SPY)
		{
			m_pls.fAlphaStart = m_cs.fAlpha;
            m_pls.bAdditive   = LTTRUE;
            bShowSprites      = LTTRUE;
		}

		m_pls.fAlphaEnd	= m_pls.fAlphaStart;

		m_Beam.ReInit(&m_pls);
		m_Beam.Update();

		// Update the sprites...

		if (m_cs.bCreateSprite)
		{
			HOBJECT hObj = m_StartSprite.GetObject();
			if (hObj)
			{
				m_StartSprite.Update();

				g_pLTClient->SetObjectPos(hObj, &(m_pls.vStartPos));

				if (bShowSprites)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
				}
				else
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, 0, FLAG_VISIBLE);
				}
			}

			hObj = m_EndSprite.GetObject();
			if (hObj)
			{
				m_EndSprite.Update();

				g_pLTClient->SetObjectPos(hObj, &(m_pls.vEndPos));

				if (bShowSprites)
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
				}
				else
				{
					g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, 0, FLAG_VISIBLE);
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserTriggerFX::CalcBeamCoords
//
//	PURPOSE:	Calculate the beam start/end coords
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserTriggerFX::CalcBeamCoords()
{
    if (!m_hServerObject) return LTFALSE;

	// The beam is relative to the server object's dims...

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

    LTVector vU, vR, vF;
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();


	// Okay, the beam is always along the object's longest dim...

    LTVector vDir = vF;
    LTFLOAT fLongestDim = m_cs.vDims.z; // Assume forward first...

	m_pls.fMaxWidth = (m_cs.vDims.x + m_cs.vDims.y); // Assume x/y are same...

	if (m_cs.vDims.y > fLongestDim)
	{
		vDir = vU;
		fLongestDim =  m_cs.vDims.y;
		m_pls.fMaxWidth = (m_cs.vDims.x + m_cs.vDims.z);
	}
	if (m_cs.vDims.x > fLongestDim)
	{
		vDir = vR;
		fLongestDim =  m_cs.vDims.x;
		m_pls.fMaxWidth = (m_cs.vDims.y + m_cs.vDims.z);
	}

	m_pls.vStartPos = vPos + (vDir * fLongestDim);
	m_pls.vEndPos	= vPos - (vDir * fLongestDim);

    return LTTRUE;
}
