//-------------------------------------------------------------------------
//
// MODULE  : ArcCtrl.cpp
//
// PURPOSE : GUI element for interacting with a segmented arc of buttons
//
// CREATED : 12/31/01 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#include "stdafx.h"
#include "ArcCtrl.h"
#include "InterfaceResMgr.h"
#include "InterfaceMgr.h"

#define DEBUG_DELETEA(x) if (x)	{debug_deletea(x);x = LTNULL;}

namespace
{
	const float EPSILON = 0.001f;
	const float RAD_TO_DEG = 180.0f / MATH_PI;
	const float DEG_TO_RAD = MATH_PI / 180.f;

    LTRotation g_rRot;
    LTVector g_vPos, g_vU, g_vR, g_vF;

// NOTE that this list must have 1-to-1 correspondence with the enumerations above
	char * segNames[] = {
		"none",

		// Library models
		"lib_hi",
		"lib_alpha",
		"lib_beta",
		"lib_gold",
		"lib_foreign",
		"lib_ghosted",
		"lib_corrupted",

		// System models
		"sys_hi",
		"sys_conflict",
		"sys_badblock",
		"sys_basecode",
		"sys_basecode_left",
		"sys_basecode_middle",
		"sys_basecode_right",
		"sys_alpha",
		"sys_beta",
		"sys_gold",

		// Creepin' corruption.
		"corrupt_alpha_0",
		"corrupt_alpha_1l",
		"corrupt_alpha_1r",
		"corrupt_alpha_2l",
		"corrupt_alpha_2r",
		"corrupt_alpha_3l",
		"corrupt_alpha_3r",
		"corrupt_alpha_4l",
		"corrupt_alpha_4r",

		"corrupt_beta_0l",
		"corrupt_beta_0r",
		"corrupt_beta_1l",
		"corrupt_beta_1r",
		"corrupt_beta_2l",
		"corrupt_beta_2r",

		"corrupt_gold_0",
		"corrupt_gold_1l",
		"corrupt_gold_1r",
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildPoly <global function>
//
//	PURPOSE:	Given a position, two angles and two radii, build a polygon.
//				The first radius is the inner one.
//
// ----------------------------------------------------------------------- //
void BuildPoly(LT_POLYF4 *pPoly, LTIntPt pos, float r1, float r2, float theta1, float theta2)
{
	if (!pPoly)
		return;

	float fCos1 = ltcosf(theta1);
	float fCos2 = ltcosf(theta2);
	float fSin1 = ltsinf(theta1);
	float fSin2 = ltsinf(theta2);

	pPoly->verts[0].x = pos.x + r1 * fCos1;
	pPoly->verts[0].y = pos.y - r1 * fSin1;
	pPoly->verts[0].z = SCREEN_NEAR_Z;

	pPoly->verts[1].x = pos.x + r1 * fCos2;
	pPoly->verts[1].y = pos.y - r1 * fSin2;
	pPoly->verts[1].z = SCREEN_NEAR_Z;

	pPoly->verts[2].x = pos.x + r2 * fCos2;
	pPoly->verts[2].y = pos.y - r2 * fSin2;
	pPoly->verts[2].z = SCREEN_NEAR_Z;

	pPoly->verts[3].x = pos.x + r2 * fCos1;
	pPoly->verts[3].y = pos.y - r2 * fSin1;
	pPoly->verts[3].z = SCREEN_NEAR_Z;
}

//////////////////////////////////////////////////////////////////////
// CArcSector Construction/Destruction
//////////////////////////////////////////////////////////////////////
CArcSector::CArcSector()
{
	m_eArcType			= ARC_TYPE_UNDEFINED;
	m_iSectorNumber		= -1;
	m_bOccupied			= false;
	m_bIgnore			= false;
	m_iColor			= 0x80606060;
	m_iFirst			= -1;
	m_iLast				= -1;

	// Sprites
	m_pSprite			= LTNULL;
	m_pRingSprite		= LTNULL;
	m_pConditionSprite	= LTNULL;

	m_pSubPtr			= LTNULL;
	m_pSubTypeFX		= LTNULL;
	m_Socket			= INVALID_MODEL_SOCKET;
	m_pHotFX			= LTNULL;
	m_iAnimNumber		= 0;

	// Visual appearance
	m_Position.x		= 0;
	m_Position.y		= 0;
	m_fInnerRadius		= 0.0f;
	m_fOuterRadius		= 0.0f;
	m_fTheta1			= 0.0f;
	m_fTheta2			= 0.0f;

	m_bShowSprites		= false;
	m_bShowIcon			= true;
}

CArcSector::~CArcSector()
{
	Term();
}

void CArcSector::Build(int iNum, LTIntPt pt, float r1, float r2, float fTheta1, float fTheta2, ArcType eType)
{
	// Safety call.
	ClearFX();

	m_iSectorNumber = iNum;
	m_Position.x = pt.x;
	m_Position.y = pt.y;
	m_fInnerRadius = r1;
	m_fOuterRadius = r2;
	m_fTheta1 = fTheta1;
	m_fTheta2 = fTheta2; 

	m_eArcType = eType;

	// Compute the anim number based on theta
	int iTheta = (int)((fTheta1+fTheta2) * 0.5f * RAD_TO_DEG);
	int iSector = iTheta / 15;
	
	m_iAnimNumber = ((29 - iSector) % 24) + 1;
}

void CArcSector::Term()
{
	ClearFX(); // remove all FX from this sector
}

void CArcSector::ShowSprites(bool bShow)
{
	m_bShowSprites = bShow;

	LTBOOL bShowSprite = bShow ? LTTRUE : LTFALSE;
	if (m_pSprite)
	{
		m_pSprite->Show(bShowSprite && m_bShowIcon);
	}
	if (m_pRingSprite)
	{
		m_pRingSprite->Show(bShowSprite);
	}
	if (m_pConditionSprite)
	{
		m_pConditionSprite->Show(bShowSprite);
	}
}

void CArcSector::ShowIcon(bool bShow)
{
	m_bShowIcon = bShow;
	if (m_pSprite)
	{
		m_pSprite->Show(bShow);
	}
}

void CArcSector::Render(HOBJECT hObj, char * szSocket)
{

	if (!hObj)
		return;

//	if (m_eArcType != ARC_TYPE_SYSTEM_MEMORY)
//		return;

	// No socket provided? Then no transform.
	if (!szSocket)
		return;

	if (!szSocket[0])
		return;

	// If we have a parent object and this arc type supports attachments to it,
	// then iterate through all of the objects and transform them appropriately.
	LTransform transform;;

	g_pModelLT->GetSocket(hObj, szSocket, m_Socket);

	if (g_pModelLT->GetSocketTransform(hObj, m_Socket, transform, LTTRUE) == LT_OK)
	{
		// Transform the subroutine
		if (m_pSubTypeFX && m_pSubTypeFX->GetObject())
		{
			g_pLTClient->SetObjectPosAndRotation(m_pSubTypeFX->GetObject(), &transform.m_Pos, &transform.m_Rot);
		}

		// Transform the highlight
		if (m_pHotFX && m_pHotFX->GetObject())
		{
			g_pLTClient->SetObjectPosAndRotation(m_pHotFX->GetObject(), &transform.m_Pos, &transform.m_Rot);
		}

		// Transform any condition FX
		FXArray::iterator iter = m_ConditionFXArray.begin();
		while (iter != m_ConditionFXArray.end())
		{
			if (*iter && (*iter)->GetObject())
			{
				g_pLTClient->SetObjectPosAndRotation((*iter)->GetObject(), &transform.m_Pos, &transform.m_Rot);
			}
			iter++;
		}
	}
}

void CArcSector::Occupy(int iFirst, int iLast, PlayerSubroutine * pSubPtr)
{
	ClearFX();

	m_bOccupied = true;
	m_pSubPtr = pSubPtr;
	m_iFirst = iFirst;
	m_iLast = iLast;

	// Only add the rest of the stuff to one of the possibly three sectors
	if (m_iSectorNumber != iFirst)
		return;

	int iSize;
	SegmentType eType = SEG_TYPE_NONE;

	if (m_eArcType == ARC_TYPE_SYSTEM_MEMORY)
	{
		iSize = 3 - (pSubPtr->eVersion);
		eType = (SegmentType)((int)SEG_SYSTEM_ALPHA + (int)pSubPtr->eVersion);
	}
	else if (m_eArcType == ARC_TYPE_LIBRARY)
	{
		iSize = 1;
		eType = (SegmentType)((int)SEG_LIBRARY_ALPHA + (int)pSubPtr->eVersion);
	}

	if (pSubPtr->pTronSubroutine->eFunction == FUNCTION_BADBLOCK)
	{
		iSize = 1;
		eType = SEG_SYSTEM_BADBLOCK;
	}

	// Compute the position for these sprites
	int sx, sy;
	{
		float fTheta = m_fTheta1 + ((m_fTheta2 - m_fTheta1) * 0.5f * (float)iSize);
		if (fTheta > 2.0f * MATH_PI)
			fTheta -= 2.0f * MATH_PI;
		float fRadius = 0.5f * (m_fInnerRadius + m_fOuterRadius);
		sx = m_Position.x + (int)(fRadius * ltcosf(fTheta) * 1.02f);
		sy = m_Position.y - (int)(fRadius * ltsinf(fTheta) * 0.98f);
	}

	// CREATION OF SPRITES

	// Create the condition sprite, if needed
	if (pSubPtr->eState != SUBSTATE_OKAY)
	{
		// Library can have Deleted, Foreign, or Corrupt
		// SysMem can have Corrupt

		switch(pSubPtr->eState)
		{
		case SUBSTATE_CORRUPT:
			m_pConditionSprite = g_pScreenSpriteMgr->CreateScreenSprite("Interface\\Menu\\Sptrex\\sub_corrupt.dtx",false, SPRITELAYER_SUBROUTINE_CONDITION);
			break;

		case SUBSTATE_DELETED:
			if (m_eArcType == ARC_TYPE_LIBRARY)
			{
				m_pConditionSprite = g_pScreenSpriteMgr->CreateScreenSprite("Interface\\Menu\\Sptrex\\sub_ghosted.dtx",false, SPRITELAYER_SUBROUTINE_CONDITION);
			}
			break;

		case SUBSTATE_UNUSABLE: // bad block or base code? No extra condition info
			// If this is base code, then select an appropriate eType
			if (pSubPtr->pTronSubroutine->eFunction == FUNCTION_BASECODE)
			{
				eType = SEG_SYSTEM_BASECODE;
				if (pSubPtr->bPrevious && pSubPtr->bNext)
				{
					eType = SEG_SYSTEM_BASECODE_MIDDLE;
				}
				else if (pSubPtr->bNext)
				{
					eType = SEG_SYSTEM_BASECODE_LEFT;
				}
				else if (pSubPtr->bPrevious)
				{
					eType = SEG_SYSTEM_BASECODE_RIGHT;
				}
			}
			break;
		case SUBSTATE_FOREIGN:
			break;
		}
	}

	if (m_pConditionSprite)
	{
		m_pConditionSprite->Show(m_bShowSprites);
		m_pConditionSprite->SetCenter(32,32);
		m_pConditionSprite->SetPosition(sx, sy);
	}

	// Create the appropriate sprites.  Hidden.
	if (pSubPtr->pTronSubroutine->szSprite[0])
	{
		m_pSprite = g_pScreenSpriteMgr->CreateScreenSprite(pSubPtr->pTronSubroutine->szSprite, false, SPRITELAYER_SUBROUTINE_SHAPE);
		if (m_pSprite)
		{
			m_pSprite->SetCenter(32,32);
			m_pSprite->SetPosition(sx, sy);
			m_pSprite->Show(m_bShowSprites && m_bShowIcon);
		}
	}

	if (pSubPtr->eState != SUBSTATE_UNUSABLE)
	{
		// Create the background sprite
		m_pRingSprite = g_pScreenSpriteMgr->CreateScreenSprite(szRingTex[pSubPtr->eVersion],false, SPRITELAYER_SUBROUTINE_BUILD);
		if (m_pRingSprite)
		{
			m_pRingSprite->SetCenter(32,32);
			m_pRingSprite->SetPosition(sx, sy);
			m_pRingSprite->Show(m_bShowSprites);
		}
	}

	if (m_eArcType == ARC_TYPE_SYSTEM_MEMORY)
	{
		m_pSubTypeFX = CreateModel(eType);
		if (m_pSubTypeFX)
		{
			HOBJECT hObj = m_pSubTypeFX->GetObject();
			if (hObj)
			{
				int iAdjustedAnimNumber = m_iAnimNumber - (iSize - 1);
				if (iAdjustedAnimNumber < 1) iAdjustedAnimNumber += 24;
				g_pLTClient->SetModelAnimation(hObj, iAdjustedAnimNumber);
			}
		}
	}
}

void CArcSector::ClearFX()
{
	// Clear the sprites
	if (m_pSprite)
	{
		g_pScreenSpriteMgr->DestroyScreenSprite(m_pSprite);
		m_pSprite = LTNULL;
	}

	if (m_pRingSprite)
	{
		g_pScreenSpriteMgr->DestroyScreenSprite(m_pRingSprite);
		m_pRingSprite = LTNULL;
	}

	if (m_pConditionSprite)
	{
		g_pScreenSpriteMgr->DestroyScreenSprite(m_pConditionSprite);
		m_pConditionSprite = LTNULL;
	}


	// destroy pre-existing sub type fx
	if (m_pSubTypeFX)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pSubTypeFX);
		m_pSubTypeFX->Reset();
		m_pSubTypeFX->Term();
		debug_delete(m_pSubTypeFX);
		m_pSubTypeFX = LTNULL;
	}

	// Remove any highlight FX
	RemoveHighlighting();

	// destroy any condition FX
	FXArray::iterator iter = m_ConditionFXArray.begin();
	while (iter != m_ConditionFXArray.end())
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(*iter);
		(*iter)->Reset();
		(*iter)->Term();
		debug_delete(*iter);
		iter++;
	}
	m_ConditionFXArray.clear();

	m_bOccupied = false;
	m_pSubPtr = LTNULL;
	m_iFirst = -1;
	m_iLast = -1;
}


void CArcSector::AddHighlighting(SegmentType eSegType)
{
	// destroy any highlight FX
	RemoveHighlighting();

	m_pHotFX = CreateModel(eSegType);
	if (m_pHotFX)
	{
		HOBJECT hObj = m_pHotFX->GetObject();
		if (hObj)
		{
			g_pLTClient->SetModelAnimation(hObj, m_iAnimNumber);
		}
	}
}


void CArcSector::RemoveHighlighting()
{
	// destroy any highlight FX
	if (m_pHotFX)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(m_pHotFX);
		m_pHotFX->Reset();
		m_pHotFX->Term();
		debug_delete(m_pHotFX);
		m_pHotFX = LTNULL;
	}
}

CBaseScaleFX * CArcSector::CreateModel(SegmentType eSegType)
{
	if (eSegType <= SEG_TYPE_NONE || eSegType >= SEG_TYPE_LAST)
		return LTNULL;

	INT_MENUPIECE * pInfo = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetMenuPiece(segNames[eSegType]);

	if (!pInfo)
		return LTNULL;

	// Get some vars
	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return LTNULL;

	LTVector vPos, vU, vR, vF;

    g_pLTClient->GetObjectPos(hCamera, &vPos);

	LTRotation rRot;
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	BSCREATESTRUCT bcs;
	LTVector vTemp, vScale(1.0f,1.0f,1.0f);

	char modName[128];

	SAFE_STRCPY(modName, pInfo->szModel);

	// if this model is an attachment (i.e. we're a system memory model) then
	// discard the position data
	LTVector vModPos(0.0f, 0.0f, 0.0f);
	if (m_eArcType != ARC_TYPE_SYSTEM_MEMORY)
	{
		vModPos = pInfo->vPos;
	}

	LTFLOAT fRot = 0.0f;
	fRot  = MATH_PI + DEG2RAD(fRot);
	rRot.Rotate(vU, fRot);

	VEC_MULSCALAR(vTemp, vF, vModPos.z);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vR, vModPos.x);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vU, vModPos.y);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
	bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);
	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;

	bcs.pFilename = modName;
	bcs.pSkinReader = &(pInfo->blrSkins);
	bcs.pRenderStyleReader = &(pInfo->blrRenderStyles);
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 0.99f;
	bcs.fFinalAlpha = 0.99f;
	bcs.fLifeTime = 1000000.0f;
	bcs.bLoop = LTFALSE;			// all models start out as non-looping

	bcs.fMinRotateVel = 1.0f;
	bcs.fMaxRotateVel = 1.0f;

	CBaseScaleFX * pPiece = debug_new(CBaseScaleFX);
	ASSERT(pPiece != LTNULL);

	// andy, we can't set the model anim here because we don't have enough information
	// so set it to anim 0 for now, and then let the calling code figure out which model to use
	if (pPiece->Init(&bcs))
	{
		pPiece->CreateObject(g_pLTClient);
		HOBJECT hObj = pPiece->GetObject();
		if (hObj)
		{
			g_pInterfaceMgr->AddInterfaceSFX(pPiece, IFX_WORLD);
			return pPiece;
		}
	}
	// Fail case.  Should only be reached if no object was created
	debug_delete(pPiece);
	return LTNULL;
}


//////////////////////////////////////////////////////////////////////
// CArcCtrl Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArcCtrl::CArcCtrl()
{
	m_pOutlinePrims		= LTNULL;
	m_pSeparatorPrims	= LTNULL;
	m_SectorArray		= LTNULL;
}

CArcCtrl::~CArcCtrl()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Render
//
//	PURPOSE:	Draw the arc control on the screen
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::Render()
{
	if (m_bHasChangedAppearance)
		Build();

	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_NOCOLOROP);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	// iterate through all ArcSectors and adjust their positions, draw programmer art
	for (int i = 0; i < m_nNumSegments; i++)
	{
		m_SectorArray[i].Render(m_hObj, m_szSocketName);
	}

	// Reduce animation time if needed
	if (m_fAnimTime > 0.0f)
	{
	}

	// Let the ScreenSpriteMgr handle its own drawing.

	if (m_bShowOutlines)
	{
		if (m_nSeparatorPrims)
			g_pDrawPrim->DrawPrim(m_pSeparatorPrims, m_nSeparatorPrims);
		if (m_nOutlinePrims)
			g_pDrawPrim->DrawPrim(m_pOutlinePrims, m_nOutlinePrims);
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Init()
//
//	PURPOSE:	These two functions set all initial values for a default controller
//
// ----------------------------------------------------------------------- //

LTBOOL CArcCtrl::Init()
{
	// Get the screen center for m_Position
	m_Position.x = (int)((float)g_pInterfaceResMgr->GetScreenWidth() * 0.5f);
	m_Position.y = (int)((float)g_pInterfaceResMgr->GetScreenHeight() * 0.5f);

	m_fArcStart				= 0.0f;
	m_fArcEnd				= 0.0f;

	m_fInnerRadius			= 100.0f;
	m_fOuterRadius			= 200.0f;
	m_fThreshold			= 0.0f;

	m_nNumSegments			= 24;
	m_fSegmentTheta			= 2.0f * MATH_PI / (float)m_nNumSegments;
	m_nNumHighlightSegments	= 0;	// start in "ignore cursor" mode

	m_HighlightColor		= 0xFFC0C0FF;
	m_EmptyColor			= 0x80606060;
	m_OutlineColor			= 0xFF8080FF;
	m_ConflictColor			= 0xFFFF0000;

	m_bShowInactive			= LTTRUE;		// default to displaying the entire control
	m_bShowOutlines			= LTTRUE;		// default to outlining it in a thick line
	m_CurrentSegment		= -1;			// no active segment
	m_fMouseAngle			= -1.0f;
	m_iTesselation			= 1;			// Arbitrary value.  Break a segment
	m_fAnimTime				= 0.0f;			// countdown timer for animation
	m_fTotalAnimTime		= 0.0f;			// Total time for animation

	m_SectorArray			= LTNULL;

	// Drawprim arrays
	m_nOutlinePrims			= 0;
	m_pOutlinePrims			= LTNULL;

	m_nSeparatorPrims		= 0;
	m_pSeparatorPrims		= LTNULL;

	m_bHasChangedAppearance = LTTRUE;

	m_bIsHot				= false;
	m_mouseX				= 0;
	m_mouseY				= 0;

	m_hObj					= LTNULL;
	m_szSocketName[0]		= 0;

	m_eArcType				= ARC_TYPE_UNDEFINED;
	m_iLibraryState			= 0;
	return LTTRUE;
}

LTBOOL CArcCtrl::Init(LTIntPt pos, float fInner, float fOuter, int nNumSegments)
{
	LTBOOL bResult = Init();

	float sx = g_pInterfaceResMgr->GetXRatio();

	m_Position.x = (int)((float)pos.x * sx);
	m_Position.y = (int)((float)pos.y * g_pInterfaceResMgr->GetYRatio());

	m_fInnerRadius = fInner * sx;
	m_fOuterRadius = fOuter * sx;
	m_nNumSegments = nNumSegments;

	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Term()
//
//	PURPOSE:	Clean up any garbage (dynamic memory, screen sprites)
//
// ----------------------------------------------------------------------- //


void CArcCtrl::Term()
{
	DEBUG_DELETEA(m_pOutlinePrims);
	DEBUG_DELETEA(m_pSeparatorPrims);
	DEBUG_DELETEA(m_SectorArray);
}

/////////////////////////////////////////////////////////////////////////////
// Parameters for controlling appearance and input
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetArc()
//
//	PURPOSE:	Allows the Arc's sweep to be something other than a circle
//				Note that the sweep should be defined counter-clockwise
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetArc(float fArcStart, float fArcEnd)
{
	m_fArcStart		= fArcStart * DEG_TO_RAD;
	m_fArcEnd		= fArcEnd * DEG_TO_RAD;

	// Crop angles to the the 0-2pi boundaries

	while (m_fArcStart > 2.0f * MATH_PI)
		m_fArcStart -= 2.0f * MATH_PI;
	while (m_fArcStart < 0.0f)
		m_fArcStart += 2.0f * MATH_PI;


	while (m_fArcEnd > 2.0f * MATH_PI)
		m_fArcEnd -= 2.0f * MATH_PI;
	while (m_fArcEnd < 0.0f)
		m_fArcEnd += 2.0f * MATH_PI;

	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetRadii
//
//	PURPOSE:	Define the dimensions of the circle to be used.  Note that
//				the Build() function will scale these values appropriately
//				and that the MouseMove() function will scale input as well
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetRadii(float fInnerRadius, float fOuterRadius, float fThreshold)
{
	m_fInnerRadius = fInnerRadius;
	m_fOuterRadius = fOuterRadius;
	m_fThreshold = fThreshold;

	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetPosition()
//
//	PURPOSE:	Position the center of the arc or circle.  Note that the
//				Build() function will rescale this from coordinates on a
//				640x480 screen to whatever the current resolution is
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetPosition(LTIntPt pos)
{
	m_Position = pos;
	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetNumSegments
//
//	PURPOSE:	Determine how many unique segments the arc should be
//				divided into.
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetNumSegments(int num)
{
	m_nNumSegments = num;
	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetNumHighlightSegments
//
//	PURPOSE:	Determine how many segments the cursor would take up if
//				Dragging and dropping.  If set to zero, then there is
//				no highlighting.  If set to -1, then we're in pickup mode
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetNumHighlightSegments(int num)
{
	if (IsOnMe(m_mouseX, m_mouseY))
	{
		RemoveHighlighting();
		m_nNumHighlightSegments = num;
		AddHighlighting();
	}
	else
	{
		m_nNumHighlightSegments = num;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ShowInactive
//
//	PURPOSE:	Sets flag for displaying drawprims versus using scaleFX
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ShowInactive(LTBOOL bShowInactive)
{
	m_bShowInactive = bShowInactive;
	m_bHasChangedAppearance = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ShowOutlines
//
//	PURPOSE:	Sets flag for displaying outlines for sectors
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ShowOutlines(LTBOOL bShowOutlines)
{
	m_bShowOutlines = bShowOutlines;
	m_bHasChangedAppearance = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::SetHighlightColor
//				CArcCtrl::SetEmptyColor
//				CArcCtrl::SetOutlineColor
//				CArcCtrl::SetConflictColor
//
//	PURPOSE:	Allow user overrides for the various colors
//
// ----------------------------------------------------------------------- //

void CArcCtrl::SetHighlightColor(uint32 color)
{
	m_HighlightColor		= color;
	m_bHasChangedAppearance = LTTRUE;
}

void CArcCtrl::SetEmptyColor(uint32 color)
{
	m_EmptyColor			= color;
	m_bHasChangedAppearance = LTTRUE;
}

void CArcCtrl::SetOutlineColor(uint32 color)
{
	m_OutlineColor			= color;
	m_bHasChangedAppearance = LTTRUE;
}

void CArcCtrl::SetConflictColor(uint32 color)
{
	m_ConflictColor			= color;
	m_bHasChangedAppearance = LTTRUE;
}

PlayerSubroutine * CArcCtrl::GetSegmentObject(int iSeg)
{
	if (iSeg >= 0 && iSeg < m_nNumSegments)
		return m_SectorArray[iSeg].GetSubObject();
	return LTNULL;
}

void CArcCtrl::AssociateHObject(HOBJECT hObj, char * szSocketName)
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

PlayerSubroutine * CArcCtrl::GetHotSub()
{
	ASSERT(m_CurrentSegment < m_nNumSegments);

	if (m_CurrentSegment != -1 && m_CurrentSegment < m_nNumSegments)
		return m_SectorArray[m_CurrentSegment].GetSubObject();
	return LTNULL;
}

void CArcCtrl::ShowSprites(bool bShow)
{
	if (m_SectorArray)
	{
		for (int i = 0; i < m_nNumSegments; i++)
		{
			m_SectorArray[i].ShowSprites(bShow);
		}
	}
}

void CArcCtrl::ShowIcon(int iSeg, bool bShow)
{
	if (m_SectorArray)
	{
		m_SectorArray[iSeg].ShowIcon(bShow);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ShiftClockwise
//				CArcCtrl::ShiftCounterClockwise
//
//	PURPOSE:	Spawn an animation of all the pieces on the arc moving
//
// ----------------------------------------------------------------------- //

// Menu interaction with the elements
void CArcCtrl::ShiftClockwise(PlayerSubroutine * pNewSub)
{
}

void CArcCtrl::ShiftCounterClockwise(PlayerSubroutine *pNewSub)
{
}

void CArcCtrl::OccupySegment(int segment, PlayerSubroutine * pSub)
{
	if (m_bHasChangedAppearance)
		Build();

	if (segment == -1 || segment >= m_nNumSegments)
		return;

	RemoveHighlighting();

	int iFirst = segment;
	ASSERT(iFirst < 24);
	int iLast = segment + m_nNumHighlightSegments;
 	if (iLast >= m_nNumSegments)
		iLast -= m_nNumSegments;

	// Add the highlighting to the new ones
	int i;
	for (i = 0; i < m_nNumHighlightSegments; i++)
	{
		int drawsegment = segment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		m_SectorArray[drawsegment].Occupy(iFirst,
										  iLast,
										  pSub);
	}
	if (pSub->bWorking)
	{
		m_SectorArray[segment].ShowIcon(false);
	}
	AddHighlighting();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ClearSegment
//
//	PURPOSE:	Set the color of a segment to the empty color, destroy
//				any attached sprite
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ClearSegment(int segment)
{
	if (m_bHasChangedAppearance)
		Build();

	if (segment == -1 || segment >= m_nNumSegments)
		return;

	RemoveHighlighting();

	int iFirst, iLast;
	m_SectorArray[segment].GetRange(iFirst, iLast);
	ASSERT(iFirst < 24);

	if (iFirst == -1 || iLast == -1)
		return;

	if (iLast < iFirst) iLast += m_nNumSegments;
	int iSize = iLast - iFirst;

	// Add the highlighting to the new ones
	int i;
	for (i = 0; i < iSize; i++)
	{
		int drawsegment = segment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		m_SectorArray[drawsegment].ClearFX();
	}
	AddHighlighting();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::IgnoreSegment
//
//	PURPOSE:	Flag whether a segment should be ignored by the cursor
//				when in "pickup"/"procedural" mode.
//
// ----------------------------------------------------------------------- //
void CArcCtrl::IgnoreSegment(int segment, bool bIgnore)
{
	if (segment < 0 || segment > m_nNumSegments)
		return;

	m_SectorArray[segment].Ignore(bIgnore);
}

void CArcCtrl::Update()
{
	if (m_bHasChangedAppearance)
		Build();

	RemoveHighlighting();
	AddHighlighting();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::OnMouseMove
//
//	PURPOSE:	Take the latest mouse coordinates and see if any changes
//				need to be made to the interface
//
// ----------------------------------------------------------------------- //
LTBOOL CArcCtrl::OnMouseMove(int mx, int my)
{
	m_mouseX = mx;
	m_mouseY = my;

	if (m_bHasChangedAppearance)
		Build();

	m_fMouseAngle = -1.0f;

	if (m_fTotalAnimTime > 0.0f)
		return LTTRUE;

	int OldSegment = m_CurrentSegment;
	int iNewSegment = -1;

	if (IsOnMe(mx, my))
	{
		// calculate which sector the mouse is on
		// based on the angle and the cursor size
		float fAngleOffset;
		if (m_nNumHighlightSegments == -1)
		{
			fAngleOffset = 0.0f;
		}
		else
		{
			fAngleOffset = m_fSegmentTheta * (0.5f * (m_nNumHighlightSegments-1));
		}
		float fSectorAngle = m_fMouseAngle - fAngleOffset - m_fArcStart;
		if (fSectorAngle <= 0.0f)
			fSectorAngle += 2.0f * MATH_PI;

		iNewSegment = (int)(fSectorAngle / m_fSegmentTheta) % 24;
		ASSERT(iNewSegment < 24);

		// if we're in pickup mode, tweak "old" and "current" to align with any
		// highlighted sector(s).  Also, if in pickup/procedural mode, check for ignore case
		int iFirst, iLast;
		m_SectorArray[iNewSegment].GetRange(iFirst, iLast);
		if (m_nNumHighlightSegments == -1)
		{
			if (m_SectorArray[iNewSegment].ShouldIgnore())
				iNewSegment = -1;
			else if (iFirst != -1)
			{
				iNewSegment = iFirst;
				ASSERT(iNewSegment < 24);
			}
		}
	}

	if (iNewSegment == m_CurrentSegment)
		return LTTRUE;

	// Aha! Must do a sector operation
	// remove highlighting from the old current segment(s), if applicable
	RemoveHighlighting();
	m_CurrentSegment = iNewSegment;
	ASSERT(m_CurrentSegment < m_nNumSegments);
	AddHighlighting();

	return LTTRUE;
}

void CArcCtrl::RemoveHighlighting()
{
	if (m_CurrentSegment == -1)
		return;

	int iNumDraw = m_nNumHighlightSegments;

	// the highlighting corresponds to exactly one occupied/unoccupied segment of 1-3 blocks.
	if (iNumDraw == -1)
	{
		int iFirst, iLast;
		m_SectorArray[m_CurrentSegment].GetRange(iFirst, iLast);
		if (iFirst != -1)
		{
			iNumDraw = iLast - iFirst;
			if (iNumDraw < 0)
				iNumDraw += m_nNumSegments;
		}
		else
		{
			iNumDraw = 1;
		}
	}

	for (int i = 0; i < iNumDraw; i++)
	{
		int drawsegment = m_CurrentSegment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		m_SectorArray[drawsegment].RemoveHighlighting();
	}
}

void CArcCtrl::AddHighlighting()
{
	if (m_CurrentSegment == -1)
		return;

	int iNumDraw = m_nNumHighlightSegments;

	// the highlighting corresponds to exactly one occupied/unoccupied segment of 1-3 blocks.
	if (iNumDraw == -1)
	{
		int iFirst, iLast;
		m_SectorArray[m_CurrentSegment].GetRange(iFirst, iLast);
		if (iFirst != -1)
		{
			iNumDraw = iLast - iFirst;
			if (iNumDraw < 0)
				iNumDraw += m_nNumSegments;
		}
		else
		{
			iNumDraw = 1;
		}
	}

	for (int i = 0; i < iNumDraw; i++)
	{
		int drawsegment = m_CurrentSegment + i;
		if (drawsegment >= m_nNumSegments)
			drawsegment -= m_nNumSegments;

		if (m_eArcType == ARC_TYPE_LIBRARY)
		{
			m_SectorArray[drawsegment].AddHighlighting(SEG_LIBRARY_HIGHLIGHT);
		}
		else
		{
			if (m_nNumHighlightSegments == -1)
			{
				m_SectorArray[drawsegment].AddHighlighting(SEG_SYSTEM_HIGHLIGHT);
			}
			else
			{
				if (m_SectorArray[drawsegment].IsOccupied())
				{
					m_SectorArray[drawsegment].AddHighlighting(SEG_SYSTEM_CONFLICT);
				}
				else
				{
					m_SectorArray[drawsegment].AddHighlighting(SEG_SYSTEM_HIGHLIGHT);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::IsOnMe
//
//	PURPOSE:	Quick test to see if the mouse is somewhere on the ArcCtrl
//
// ----------------------------------------------------------------------- //
LTBOOL CArcCtrl::IsOnMe(int mx, int my)
{
	m_bIsHot = false;

	if (m_fTotalAnimTime > 0.0f)
		return LTFALSE;

	if (m_nNumHighlightSegments == 0)
		return LTFALSE;

	float fInner = m_fInnerRadius - m_fThreshold;
	float fOuter = m_fOuterRadius + m_fThreshold;

	// 1. Fast cull against rectangle
	if (mx > m_Position.x + fOuter || mx < m_Position.x - fOuter)
		return LTFALSE;

	if (my > m_Position.y + fOuter || my < m_Position.y - fOuter)
		return LTFALSE;

	// 2.  Fast cull against radii
	int dx = mx - m_Position.x;
	int dy = my - m_Position.y;

	float fDistSquared = (float)((dx * dx) + (dy * dy));

	if (fDistSquared > fOuter * fOuter)
		return LTFALSE;

	if (fDistSquared < fInner * fInner)
		return LTFALSE;

	m_fMouseAngle = VectorToAngle((float)dx, (float)-dy);

	// 3.  Optional cull against arc
	if (m_fArcStart == m_fArcEnd)
	{
		m_bIsHot = true;
		return LTTRUE;
	}

	if (m_fArcEnd - m_fArcStart >= 2.0f * MATH_PI)
	{
		m_bIsHot = true;
		return LTTRUE;
	}

	if (m_fArcStart < m_fArcEnd)
	{
		if (m_fMouseAngle  < m_fArcStart || m_fMouseAngle > m_fArcEnd)
			return LTFALSE;
		else
		{
			m_bIsHot = true;
			return LTTRUE;
		}
	}
	else // check against an arc that spans the 360 degree end
	{
		if (m_fMouseAngle > m_fArcStart && m_fMouseAngle < 2.0f * MATH_PI)
		{
			m_bIsHot = true;
			return LTTRUE;
		}
		if (m_fMouseAngle < m_fArcEnd && m_fMouseAngle > 0.0f)
		{
			m_bIsHot = true;
			return LTTRUE;
		}
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::ComputeOptimalTesselation
//
//	PURPOSE:	Given the current parameters, set the most efficient
//				tesselation value for the rendering
//
// ----------------------------------------------------------------------- //

void CArcCtrl::ComputeOptimalTesselation()
{
	// Sanity check
	if (m_nNumSegments < 1)
		m_nNumSegments = 1;

	// tolerance is how close, in pixels, the approximation should be to a
	// true arc.
	float fTolerance = 1.0f;

	// Maximum tesselation is 24 segments
	const int iMaxTesselation = 24;

	// Start by testing with a tesselation of 1
	m_iTesselation = 1;

	// Here's my beautiful and elegant algorithm.

	// We want to determine how many straight line segments is the bare minimum
	// required in order to approximate the desired curve.  In this case, the
	// curve is an arc with a radius of 'm_fOuterRadius' and a sweep of 'fTheta'
	// radians.

	// The way we do this is to see where the line segment is farthest from the
	// true curve and minimize that distance.  The point where the line segment
	// is farthest is its midpoint.  So (stay with me here) if the line segment
	// is from zero degrees to fThetaPrime, where fThetaPrims = fTheta / iTess,
	// then we could take the midpoint of that line and compare its distance
	// from the origin to the radius.

	// Now here's the really clever bit.  If we took that line segment and
	// ROTATED it clockwise by HALF of fThetaPrime, then the line segment
	// would be perfectly vertical, and the midpoint would lie on the X axis,
	// making the comparison test trivial by eliminating magnitude tests.
	// To compute the x coordinate of the line segment is embarrassingly simple:
	// x = radius * cos(fThetaPrime / 2) !!!!!!

	// So to get the optimal value, we just increase iTess and compute x until
	// x is a tolerable distance from the true radius, or we hit our maximum
	// tesselation.  I rock.

	while (m_iTesselation < iMaxTesselation)
	{
		float fThetaPrime = m_fSegmentTheta / (float)m_iTesselation;
		float fLineX = m_fOuterRadius * ltcosf(fThetaPrime * 0.5);
		float fDelta = m_fOuterRadius - fLineX;

		// Exit when we have the tolerance we want!
		if (LTABS(fDelta) < fTolerance)
			return;

		m_iTesselation++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::Build()
//
//	PURPOSE:	Reconstruct the drawprims and other computed variables.
//
// ----------------------------------------------------------------------- //
void CArcCtrl::Build()
{
	bool bCircle = false;

	if (m_fArcStart == m_fArcEnd)
	{
		bCircle = true;
	}
	else if (m_fArcEnd - m_fArcStart >= 2.0f * MATH_PI)
	{
		bCircle = true;
	}

	Term();

	m_SectorArray = debug_newa(CArcSector, m_nNumSegments);

	float fSweep = m_fArcEnd - m_fArcStart;
	if (fSweep <= 0.0f)
		fSweep += 2.0f * MATH_PI;

	m_fSegmentTheta = fSweep / (float)m_nNumSegments;

	ComputeOptimalTesselation();

	float fMiniSweep = m_fSegmentTheta / m_iTesselation;	// could theoretically be the same as fSegment


	// Segments
	// number of segment prims is m_iTesselation * m_nNumberSegments;
	m_nSegmentPrims = m_iTesselation * m_nNumSegments;

	// Separators
	// if circle, number of separator prims is m_nNumberSegments;
	// if arc, number of separator prims is m_nNumberSegments -1;
	m_nSeparatorPrims = m_nNumSegments - (bCircle ? 0 : 1);
	m_pSeparatorPrims = debug_newa(LT_POLYF4, m_nSeparatorPrims);

	// Outlines
	// number of outline prims is number of segment prims
	// if inner radius > 1, then number of outline prims is DOUBLE number of segment prims
	// if arc, number of outline prims += 2;
	m_nOutlinePrims = m_nSegmentPrims;
	if (m_fInnerRadius > 1.0f)
		m_nOutlinePrims += m_nSegmentPrims;
	if (!bCircle)
		m_nOutlinePrims += 2;
	m_pOutlinePrims = debug_newa(LT_POLYF4, m_nOutlinePrims);


	for (int i = 0; i < m_nNumSegments; i++)
	{
		float fTheta1 = m_fArcStart + (float)i * m_fSegmentTheta;
		float fTheta2 = fTheta1 + m_fSegmentTheta;

		m_SectorArray[i].Build(i,					// sector number
							  m_Position,			// center of arc
							  m_fInnerRadius,		// inner arc
							  m_fOuterRadius,		// outer arc
							  fTheta1,				// beginning of sweep
							  fTheta2,				// end of sweep
							  m_eArcType);
//							  m_iTesselation);		// number of polies to use
		
		// Check to draw separators
		if (m_bShowOutlines)
		{
			LT_POLYF4 * pSepPoly = LTNULL;
			if (bCircle)
			{
				pSepPoly = &m_pSeparatorPrims[i];
			}
			else if (i > 0)
			{
				pSepPoly = &m_pSeparatorPrims[i-1];
			}
			if (pSepPoly)
			{
				BuildPoly(pSepPoly, m_Position, m_fInnerRadius, m_fOuterRadius, fTheta1, fTheta1 + DEG_TO_RAD * 0.5f);
				g_pDrawPrim->SetRGBA(pSepPoly, m_OutlineColor);
			}

		}

		for (int j = 0; j < m_iTesselation; j++)
		{
			float fThetaA = fTheta1 + (float)j * fMiniSweep;
			float fThetaB = fThetaA + fMiniSweep;
			
			// Check for outlines here.
			if (m_bShowOutlines)
			{
				LT_POLYF4 * pPoly = &m_pOutlinePrims[i * m_iTesselation + j];
				BuildPoly(pPoly, m_Position, m_fOuterRadius, m_fOuterRadius + 2.0f, fThetaA, fThetaB);
				g_pDrawPrim->SetRGBA(pPoly, m_OutlineColor);

				if (m_fInnerRadius > 1.0f)
				{
					pPoly = &m_pOutlinePrims[i * m_iTesselation + j + m_nSegmentPrims];
					BuildPoly(pPoly, m_Position, m_fInnerRadius - 2.0f, m_fInnerRadius, fThetaA, fThetaB);
					g_pDrawPrim->SetRGBA(pPoly, m_OutlineColor);
				}
			}
		}
	}

	// If it's an arc, cap it.
	if (m_bShowOutlines && !bCircle)
	{
		BuildPoly(&m_pOutlinePrims[m_nOutlinePrims-2], m_Position, m_fInnerRadius-2.0f, m_fOuterRadius+2.0f, m_fArcStart, m_fArcStart - DEG_TO_RAD*0.5f);
		BuildPoly(&m_pOutlinePrims[m_nOutlinePrims-1], m_Position, m_fInnerRadius-2.0f, m_fOuterRadius+2.0f, m_fArcEnd, m_fArcEnd + DEG_TO_RAD*0.5f);
		g_pDrawPrim->SetRGBA(&m_pOutlinePrims[m_nOutlinePrims-2], m_OutlineColor);
		g_pDrawPrim->SetRGBA(&m_pOutlinePrims[m_nOutlinePrims-1], m_OutlineColor);
	}

	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

    g_pLTClient->GetObjectPos(hCamera, &g_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &g_rRot);
	g_vU = g_rRot.Up();
	g_vR = g_rRot.Right();
	g_vF = g_rRot.Forward();

	m_bHasChangedAppearance = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArcCtrl::VectorToAngle
//
//	PURPOSE:	Take a vector from origin (in cartesian space, not screen space)
//				Return the corresponding angle in radians
//
// ----------------------------------------------------------------------- //

float CArcCtrl::VectorToAngle( float x, float y )
{
	// easy case, vector is too small
	if ( ( EPSILON > LTABS( x ) ) &&
	     ( EPSILON > LTABS( y ) ) )
	{
		return 0.0f;
	}

	// are we on the y-axis?
	if (LTABS(x) < EPSILON)
	{
		return (y > 0.0f ? MATH_HALFPI :  1.5f * MATH_PI);
	}
	// are we on the x-axis?
	if (LTABS(y) < EPSILON)
	{
		return (x > 0.0f ? 0 : MATH_PI);
	}

	float result;
	if ( ( 0.0f <= x ) && ( 0.0f <= y ) )
	{
		// quadrant 1
		result = ltatanf( static_cast< double >( y / x ) );
	}
	else if ( ( 0.0f >= x ) && ( 0.0f <= y ) )
	{
		// quadrant 2
		result = ( MATH_PI - ltatanf( static_cast< double >( - y / x ) ) );
	}
	else if ( ( 0.0f >= x ) && ( 0.0f >= y ) )
	{
		// quadrant 3
		result = ( MATH_PI - ltatanf( static_cast< double >( - y / x ) ) );
	}
	else if ( ( 0.0f <= x ) && ( 0.0f >= y ) )
	{
		// quadrant 4
		result = ( 2 * MATH_PI + ltatanf( static_cast< double >( y / x ) ) );
	}
	else
	{
		// shouldn't be able to get here
		ASSERT( 0 );
		result = 0.0f;
	}
	return result;
}

