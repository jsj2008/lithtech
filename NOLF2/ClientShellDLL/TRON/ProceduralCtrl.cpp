//-------------------------------------------------------------------------
//
// MODULE  : ProceduralCtrl.cpp
//
// PURPOSE : GUI element for interacting with procedurals
//
// CREATED : 4/3/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

// write the renderFX function

#include "stdafx.h"
#include "ProceduralCtrl.h"
#include "InterfaceMgr.h"

#define DEBUG_DELETEA(x) if (x)	{debug_deletea(x);x = LTNULL;}

#define PROC_MODEL "ingame_procedurals_disc.ltb"
#define PROC_PROGRESS "ingame_procedurals_disc_prog.ltb"
#define PROC_CONDITION "ingame_procedurals_disc_cond.ltb"

#define PROC_RS_FULLBRIGHT "RS\\Alpha_fullbright.ltb"
#define PROC_RS_ADDITIVE "RS\\Alpha_Additive.ltb"

#define PROC_SKIN_EMPTY "Interface\\Subroutines\\skins\\proc_empty.dtx"
#define PROC_SKIN_HL "Interface\\Subroutines\\skins\\proc_hl.dtx"
//////////////////////////////////////////////////////////////////////
// CProceduralCtrl Construction/Destruction
//////////////////////////////////////////////////////////////////////
CProceduralCtrl::CProceduralCtrl()
{
	m_pProc			= LTNULL;

	m_pIdleFX		= LTNULL;		// waiting or working
	m_pWorkingFX	= LTNULL;
	m_pSubroutineFX	= LTNULL;		// model of the subroutine
	m_pSubBuildFX	= LTNULL;		// subroutine alpha, beta, gold model
	m_pConditionFX	= LTNULL;		// model of how much the subroutine is affected
	m_pProgressFX	= LTNULL;		// model of how much the subroutine has been cleared
	m_pHighlightFX	= LTNULL;		// highlight when the mouse cursor is over

	m_iSlot			= -1;
	m_CenterPos.x	= 100;
	m_CenterPos.y	= 100;
	m_iRadius		= 16;

	m_hObj			= LTNULL;
	m_szSocketName[0] = 0;
	m_bWorking		= false;
}

CProceduralCtrl::~CProceduralCtrl()
{
	Term();
}


CBaseScaleFX * CProceduralCtrl::CreateFX(char * szModel, char * szSkin, char *szRenderStyle, char *szAnim)
{
//	if (!m_pProc)
//		return LTNULL;

	if (!szModel || !szSkin)
		return LTNULL;

	if (!szRenderStyle)
		szRenderStyle = PROC_RS_FULLBRIGHT;

	// Make a few useful strings
	CButeListReader blrSkin;
	blrSkin.SetItem(0, szSkin, 128);

	CButeListReader blrRenderStyle;
	blrRenderStyle.SetItem(0, szRenderStyle, 128);

	char szModelName[1024];
	sprintf(szModelName, "Interface//Subroutines//Models//%s", szModel);


	// Get some vars
	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return LTNULL;

	LTVector vPos, vU, vR, vF;
	LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);

    g_pLTClient->GetObjectRotation(hCamera, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	BSCREATESTRUCT bcs;
	LTVector vTemp, vScale(1.0f,1.0f,1.0f);

	// If there is a model to attach it do, then set the transform to NOTHING
	float fZ = 320.0f;
	if (m_hObj != LTNULL)
	{
		fZ = 0.0f;
	}

	LTFLOAT fRot = 0.0f;
	fRot  = MATH_PI + DEG2RAD(fRot);
	rRot.Rotate(vU, fRot);

	VEC_MULSCALAR(vTemp, vF, fZ);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
	bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);
	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;

	bcs.pFilename = szModelName;
	bcs.pSkinReader = &blrSkin;
	bcs.pRenderStyleReader = &blrRenderStyle;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;

	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 0.99f;
	bcs.fFinalAlpha = 0.99f;
	bcs.fLifeTime = 1000000.0f;
	bcs.bLoop = LTFALSE;			// all models start out as non-looping

	bcs.fMinRotateVel = 1.0f;
	bcs.fMaxRotateVel = 1.0f;

	CBaseScaleFX * pPiece = debug_new(CBaseScaleFX);
	ASSERT(pPiece != LTNULL);

	if (pPiece->Init(&bcs))
	{
		pPiece->CreateObject(g_pLTClient);
		HOBJECT hObj = pPiece->GetObject();

		if (hObj)
		{
			g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, szAnim));
			g_pInterfaceMgr->AddInterfaceSFX(pPiece, IFX_MENU_ATTACH);
			return pPiece;
		}
	}
	// Fail case.  Should only be reached if no object was created
	debug_delete(pPiece);
	return LTNULL;
}

void CProceduralCtrl::KillFX(CBaseScaleFX * pFX)
{
// destroy pre-existing sub type fx
	if (pFX)
	{
//		g_pInterfaceMgr->RemoveInterfaceSFX(pFX);
		pFX->Reset();
		pFX->Term();
		debug_delete(pFX);
		pFX = LTNULL;
	}
}

void CProceduralCtrl::AssociateHObject(HOBJECT hObj, char * szSocketName)
{
	m_hObj = hObj;
	m_szSocketName[0] = 0;

	// sanity chex
	if (!szSocketName) return;
	if (!szSocketName[0]) return;

	if (strlen(szSocketName) < 32)
	{
		SAFE_STRCPY(m_szSocketName, szSocketName);
	}
}

// Initialization.  Called when Subroutines gain focus
bool CProceduralCtrl::Init(Procedural * pProc)
{
	m_bWorking = false;
	m_pProc = pProc;

	m_iSlot = pProc->iProcSlot;
	if (m_pProc->bPlayerHasThis)
	{
		m_CenterPos.x = (int)(pProc->CenterPos.x * g_pInterfaceResMgr->GetXRatio());
		m_CenterPos.y = (int)(pProc->CenterPos.y * g_pInterfaceResMgr->GetYRatio());
		m_iRadius = (int)(pProc->iRadius * g_pInterfaceResMgr->GetXRatio());

		// Create the idle model
		m_pIdleFX = CreateFX(PROC_MODEL, m_pProc->szIdleSkin, PROC_RS_FULLBRIGHT, "loop");
	}
	else
	{
		m_pProc = LTNULL;
		m_CenterPos.x = 0;
		m_CenterPos.y = 0;
		m_iRadius = 0;

		// Create the empty model
		m_pIdleFX = CreateFX(PROC_MODEL, PROC_SKIN_EMPTY, PROC_RS_FULLBRIGHT, "loop");
		return LTTRUE;
	}

	// Create the working model
	m_pWorkingFX = CreateFX(PROC_MODEL, m_pProc->szWorkSkin, PROC_RS_FULLBRIGHT, "work");

	char buf[16];
	int iDone = 0;
	if (m_pProc->pSub)
	{
		iDone = (int)(pProc->pSub->fPercentDone * 26.0f);
		if (iDone < 0)
			iDone = 0;
		else if (iDone > 26)
			iDone = 26;
	}
	sprintf(buf,"%d", iDone);

	// Create the Condition model
	m_pConditionFX = CreateFX(PROC_CONDITION, m_pProc->szConditionSkin, PROC_RS_FULLBRIGHT, buf);

	// Create the progress model
	m_pProgressFX = CreateFX(PROC_PROGRESS, m_pProc->szProgressSkin, PROC_RS_FULLBRIGHT, buf);


	// Create the highlight model
	m_pHighlightFX = CreateFX(PROC_MODEL, PROC_SKIN_HL, PROC_RS_ADDITIVE, "hl");

	// Hide 'em.
	if (m_pProc->pSub)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pIdleFX);

		if (m_pProc->pSub->pTronSubroutine->eFunction != FUNCTION_BADBLOCK)
		{
			m_pSubroutineFX = CreateFX(PROC_MODEL, m_pProc->pSub->pTronSubroutine->szSprite, PROC_RS_FULLBRIGHT, "sub");
			m_pSubBuildFX = CreateFX(PROC_MODEL, szRingTex[m_pProc->pSub->eVersion],	PROC_RS_FULLBRIGHT, "opt");
		}
		m_bWorking = true;
	}
	if (!m_pProc->pSub)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pWorkingFX);
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pConditionFX);
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pProgressFX);
	}
	g_pInterfaceMgr->RemoveInterfaceSFX(m_pHighlightFX);

	return LTTRUE;
}

void CProceduralCtrl::Term()
{
	m_pProc		= LTNULL;

	if (m_bWorking)
		RemoveSub();
	
	g_pInterfaceMgr->RemoveInterfaceSFX(m_pIdleFX);
	KillFX(m_pIdleFX);

	KillFX(m_pWorkingFX);
	KillFX(m_pSubBuildFX);
	KillFX(m_pProgressFX);
	KillFX(m_pHighlightFX);

	m_pIdleFX		= LTNULL;
	m_pWorkingFX	= LTNULL;
	m_pSubBuildFX	= LTNULL;
	m_pProgressFX	= LTNULL;
	m_pHighlightFX	= LTNULL;
}

bool CProceduralCtrl::Render()
{
//	if (!m_pProc)
//		return true;

	// if this procedural is working, then update the animation for the Condition and the Progress
	if (m_pProc && m_pProc->pSub)
	{
		float fProgress = m_pProc->pSub->fPercentDone;
		int iProgress = (int)(fProgress * 20.0f);
		char buf[80];
		sprintf(buf,"%d", iProgress);
//		sprintf(buf,"%d_%d", m_iSlot + 1, iProgress);
		if (m_pProgressFX)
		{
			g_pLTClient->SetModelAnimation(m_pProgressFX->GetObject(), g_pLTClient->GetAnimIndex(m_pProgressFX->GetObject(), buf));
		}
		if (m_pConditionFX)
		{
			g_pLTClient->SetModelAnimation(m_pConditionFX->GetObject(), g_pLTClient->GetAnimIndex(m_pConditionFX->GetObject(), buf));
		}
	}
	
	// If a model attachment is provided, then transform all available FX to move with the specified socket
	if (m_hObj && m_szSocketName[0])
	{
		LTransform transform;
		HMODELSOCKET hSocket;

		g_pModelLT->GetSocket(m_hObj, m_szSocketName, hSocket);
		// Iterate through all of the fx and transform them appropriately

		if (g_pModelLT->GetSocketTransform(m_hObj, hSocket, transform, LTTRUE) == LT_OK)
		{
			TransformFX(m_pIdleFX, transform);
			TransformFX(m_pWorkingFX, transform);
			TransformFX(m_pSubroutineFX, transform);
			TransformFX(m_pSubBuildFX, transform);
			TransformFX(m_pConditionFX, transform);
			TransformFX(m_pProgressFX, transform);
			TransformFX(m_pHighlightFX, transform);
		}
	}
	return true;
}

void CProceduralCtrl::TransformFX(CBaseScaleFX * pFX, LTransform transform)
{
	if (!pFX)
		return;

	HOBJECT hObj = pFX->GetObject();

	if (!hObj)
		return;

	g_pLTClient->SetObjectPosAndRotation(hObj, &transform.m_Pos, &transform.m_Rot);
}

LTBOOL CProceduralCtrl::OnMouseMove(int mx, int my, PlayerSubroutine * pCursorSub)
{
	if (!m_pProc)
		return LTTRUE;

	bool bWasHot = m_bHot;
/* pure circular hotness detection commented out
	// Determine if the mouse is over this control
	int dx2 = (mx - m_CenterPos.x) ^ 2;
	int dy2 = (my - m_CenterPos.y) ^ 2;

	m_bHot = ((dx2 + dy2) <= (m_iRadius ^2)) ? true : false;
*/
	m_bHot = true;

	if (pCursorSub)
	{
		if (pCursorSub->eState != m_pProc->eAffectState)
		{
			m_bHot = false;
		}
	}
	if (mx < m_CenterPos.x - m_iRadius || mx > m_CenterPos.x + m_iRadius)
		m_bHot = false;
	if (m_bHot)
	{
		if (my < m_CenterPos.y - m_iRadius || my > m_CenterPos.y + m_iRadius)
		{
			m_bHot = false;
		}
	}
	if (m_bHot && !bWasHot)
	{
		// Do not add highlighting if the cursor is holding an inappropriate subroutine?
		g_pInterfaceMgr->AddInterfaceSFX(m_pHighlightFX, IFX_MENU_ATTACH);
		return LTTRUE;
	}
	if (!m_bHot && bWasHot)
	{
		// remove highlighting
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pHighlightFX);
		return LTTRUE;
	}
	return LTTRUE;
}

void CProceduralCtrl::RemoveSub()
{
	// SanityChex
	if (!m_pProc) return;
	if (!m_pProc->pSub) return;

	if (!m_bWorking) return;

	// Hide the working, condition and progress models.
	if (m_pWorkingFX)
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pWorkingFX);

	if (m_pConditionFX)
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pConditionFX);

	if (m_pProgressFX)
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pProgressFX);

	// Remove the subroutine and build models
	if (m_pSubroutineFX)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pSubroutineFX);
		KillFX(m_pSubroutineFX);
		m_pSubroutineFX = LTNULL;
	}

	if (m_pSubBuildFX)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pSubBuildFX);
		KillFX(m_pSubBuildFX);
		m_pSubBuildFX = LTNULL;
	}

	// Show the "idle" model
	if (m_pIdleFX)
		g_pInterfaceMgr->AddInterfaceSFX(m_pIdleFX, IFX_MENU_ATTACH);

	m_bWorking = false;
	m_pProc->pSub = LTNULL;
}

void CProceduralCtrl::AddSub(PlayerSubroutine * pSub)
{
	// SanityChex
	if (!m_pProc) return;

	if (!pSub) return;

	m_pProc->pSub = pSub;

	if (m_bWorking)
	{
		RemoveSub();
	}

	// Hide the "idle" model
	if (m_pIdleFX)
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pIdleFX);


	// Show the working, condition and progress models.
	if (m_pWorkingFX)
		g_pInterfaceMgr->AddInterfaceSFX(m_pWorkingFX, IFX_MENU_ATTACH);

	if (m_pConditionFX)
		g_pInterfaceMgr->AddInterfaceSFX(m_pConditionFX, IFX_MENU_ATTACH);

	if (m_pProgressFX)
		g_pInterfaceMgr->AddInterfaceSFX(m_pProgressFX, IFX_MENU_ATTACH);

	// Add the subroutineFX and the subbuildFX

	if (pSub->pTronSubroutine->eFunction != FUNCTION_BADBLOCK)
	{
		m_pSubroutineFX = CreateFX(PROC_MODEL, pSub->pTronSubroutine->szSprite, PROC_RS_FULLBRIGHT, "sub");
		m_pSubBuildFX = CreateFX(PROC_MODEL, szRingTex[pSub->eVersion],	PROC_RS_FULLBRIGHT, "opt");
	}

	m_bWorking = true;
}
