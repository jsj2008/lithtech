// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCrosshair.cpp
//
// PURPOSE : HUDItem to display crosshair
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ClientRes.h"
#include "TronHUDMgr.h"
#include "HUDCrosshair.h"
#include "TronPlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "TronGameClientShell.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "TargetMgr.h"

#define CH_LEFT			(1 << 0)
#define CH_RIGHT		(1 << 1)
#define CH_TOP			(1 << 2)
#define CH_BOTTOM		(1 << 3)
#define CH_XLEFT		(1 << 4)
#define CH_XRIGHT		(1 << 5)
#define CH_XTOP			(1 << 6)
#define CH_XBOTTOM		(1 << 7)
#define CH_DOT			(1 << 8)
#define CH_FULLRIGHT	(1 << 9)
#define CH_FULLBOTTOM	(1 << 10)

namespace
{
	VarTrack g_vtScopeLRGap;
	VarTrack g_vtScopeUDGap;
	VarTrack g_vtScopeLRRadius;
	VarTrack g_vtScopeUDRadius;

	char * szReticleTex[] = {
		"Interface\\HUD\\reticle.dtx",
		"Interface\\HUD\\reticle_innocent.dtx",
		"Interface\\HUD\\reticle_locked.dtx",
		"Interface\\HUD\\reticle_activate.dtx",
	};
}



//******************************************************************************************
//** HUD crosshair
//******************************************************************************************
CHUDCrosshair::CHUDCrosshair()
{
	m_UpdateFlags = kHUDFrame;
	m_bEnabled = true;
	m_bArmed = false;
	m_pStr = LTNULL;
	m_fScale = 0.0f;
	m_x = 0.0f;
	m_y = 0.0f;
	m_dbgx = 0.0f;
	m_dbgy = 0.0f;
}
	

LTBOOL CHUDCrosshair::Init()
{

    m_fCrosshairGapMin = g_pLayoutMgr->GetCrosshairGapMin();
	if (m_fCrosshairGapMin <= 0.0f)
		m_fCrosshairGapMin = 5.0f;
    float fGapMax = g_pLayoutMgr->GetCrosshairGapMax();
	if (fGapMax <= 0.0f)
		fGapMax = 8.0f;
	m_fCrosshairGapRange = fGapMax - m_fCrosshairGapMin;

    m_fCrosshairBarMin = g_pLayoutMgr->GetCrosshairBarMin();
	if (m_fCrosshairBarMin <= 0.0f)
		m_fCrosshairBarMin = 5.0f;
	float fBarMax = g_pLayoutMgr->GetCrosshairBarMax();
	if (fBarMax <= 0.0f)
		fBarMax = 8.0f;
	m_fCrosshairBarRange = fBarMax - m_fCrosshairBarMin;

	g_pDrawPrim->SetUVWH(&m_Poly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);


    g_vtScopeLRGap.Init(g_pLTClient, "ScopeLRGap", NULL, 32.0f);
    g_vtScopeUDGap.Init(g_pLTClient, "ScopeUPGap", NULL, 32.0f);
    g_vtScopeLRRadius.Init(g_pLTClient, "ScopeLRRadius", NULL, 0.377f);
    g_vtScopeUDRadius.Init(g_pLTClient, "ScopeUDRadius", NULL, 0.34f);

	// ABM TRON reticles
	for (int i = 0; i < NUM_RETICLES; i++)
	{
		m_hReticleTex[i] = g_pInterfaceResMgr->GetTexture(szReticleTex[i]);
	}
	g_pDrawPrim->SetRGBA(&m_ReticlePrim, argbWhite);
	g_pDrawPrim->SetUVWH(&m_ReticlePrim, 0.0f, 0.0f, 1.0f, 1.0f);

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_StrPos = LTIntPt(320,220);
	m_nStrSz = 16;
	m_nStrJust = 1;
	m_StrColor = argbWhite;
	m_StrDisColor = 0x80808080;

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f,0.0f);
	m_pStr->SetColor(m_StrColor);

	m_DbgPos = LTIntPt(360,200);
	m_nDbgSz = 12;
	m_nDbgWidth = 220;
	m_nDbgJust = 0;
	m_DbgColor = argbWhite;

	m_pDbgStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f,0.0f);
	m_pDbgStr->SetColor(m_DbgColor);
	m_pDbgStr->SetWrapWidth(m_nDbgWidth);

	m_InfoPos = LTIntPt(320, 260);
	m_nInfoSz = 12;
	m_nInfoWidth = 200;
	m_nInfoJust = 1;
	m_InfoColor = 0x8000FF00;
	m_InfoBadColor = 0x80FF0000;

	m_pInfoStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f,0.0f);
	m_pInfoStr->SetColor(m_InfoColor);
	m_pInfoStr->SetWrapWidth(m_nInfoWidth);

	UpdateLayout();	


	return LTTRUE;
}

void CHUDCrosshair::Term()
{
	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
	if (m_pDbgStr)
	{
		g_pFontManager->DestroyPolyString(m_pDbgStr);
        m_pDbgStr=LTNULL;
	}
	if (m_pInfoStr)
	{
		g_pFontManager->DestroyPolyString(m_pInfoStr);
        m_pInfoStr=LTNULL;
	}
}

void CHUDCrosshair::Render()
{
	
	SetRenderState();
	if (g_pInterfaceMgr->IsOverlayActive(OVM_SCOPE))
	{
		if (!g_pPlayerMgr->UsingCamera())
			RenderScope();
		return;
	}


	if (!m_bEnabled)
		return;


	if (m_bArmed)
		RenderArmed();
	else
		RenderUnarmed();

	if( m_pStr )
	{
		m_pStr->SetPosition(m_x+2.0f,m_y+2.0f);
		m_pStr->SetColor(argbBlack);
		m_pStr->Render();

		m_pStr->SetPosition(m_x,m_y);
		if (g_pPlayerMgr->GetTargetMgr()->CanActivateTarget())
			m_pStr->SetColor(m_StrColor);
		else
			m_pStr->SetColor(m_StrDisColor);
		m_pStr->Render();
	}

	if( m_pDbgStr )
	{
		m_pDbgStr->SetPosition(m_dbgx+1.0f,m_dbgy+1.0f);
		m_pDbgStr->SetColor(argbBlack);
		m_pDbgStr->Render();

		m_pDbgStr->SetColor(m_DbgColor);
		m_pDbgStr->SetPosition(m_dbgx,m_dbgy);
		m_pDbgStr->Render();
	}

	if (m_pInfoStr)
	{
		m_pInfoStr->SetPosition(m_infox+1.0f,m_infoy+1.0f);
		m_pInfoStr->SetColor(argbBlack);
		m_pInfoStr->Render();

		m_pInfoStr->SetColor(m_InfoColor);
		m_pInfoStr->SetPosition(m_infox,m_infoy);
		m_pInfoStr->Render();
	}
}


void CHUDCrosshair::Update()
{
	if (!m_bEnabled)
		return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		m_fScale = g_pInterfaceResMgr->GetXRatio();
		m_x = (float)m_StrPos.x * m_fScale;
		m_y = (float)m_StrPos.y * m_fScale;

		uint8 nTextSize = (uint8)((float)m_nStrSz * m_fScale);
		m_pStr->SetCharScreenHeight(nTextSize);

		m_dbgx = (float)m_DbgPos.x * m_fScale;
		m_dbgy = (float)m_DbgPos.y * m_fScale;
	
		nTextSize = (uint8)((float)m_nDbgSz * m_fScale);
		m_pDbgStr->SetCharScreenHeight(nTextSize);

		m_infox = (float)m_InfoPos.x * m_fScale;
		m_infoy = (float)m_InfoPos.y * m_fScale;

		nTextSize = (uint8)((float)m_nInfoSz * m_fScale);
		m_pInfoStr->SetCharScreenHeight(nTextSize);
	}

	if (g_pPlayerMgr->IsCarryingBody())
	{
		m_nString = 0;
		if (g_pPlayerMgr->CanDropBody())
			m_pStr->SetText(LoadTempString(IDS_TARGET_DROP));
		else
			m_pStr->SetText("");
		return;
	}

	// Draw the appropriate cross-hair based on our current weapon,
	// and what is directly in front of us...

	bool bWeaponsEnabled = g_pPlayerMgr->GetClientWeaponMgr()->WeaponsEnabled();
	IClientWeaponBase* pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	m_bArmed = (pClientWeapon && bWeaponsEnabled);

	if (g_pPlayerMgr->IsSearching())
	{
		m_pStr->SetText(LoadTempString(IDS_TARGET_SEARCHING));
		return;
	}

	HOBJECT hObj = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
	uint16  nID = g_pPlayerMgr->GetTargetMgr()->GetTargetStringID();

	if (strcmp(g_pPlayerMgr->GetTargetMgr()->GetDebugString(),m_pDbgStr->GetText()))
		m_pDbgStr->SetText(g_pPlayerMgr->GetTargetMgr()->GetDebugString());

	if (hObj == m_hObj && nID == m_nString)
		return;

	m_hObj = hObj;
	m_nString = nID;
	m_pStr->SetText(g_pPlayerMgr->GetTargetMgr()->GetTargetString());

	if (g_pTronPlayerMgr->GetTronTargetMgr()->IsEnergyRequired())
	{
		int iEnergy = g_pTronPlayerMgr->GetTronTargetMgr()->GetEnergyRequired();
		m_pInfoStr->SetText(FormatTempString(IDS_REQUIRES_ENERGY,iEnergy));
		int iPlayerEnergy = g_pTronPlayerStats->GetEnergy();
		m_pInfoStr->SetColor((iPlayerEnergy > iEnergy) ? m_InfoColor : m_InfoBadColor);
	}
	else
	{
		m_pInfoStr->SetText("");
	}

	// Check for special case of camera-activate mode...

	bool bUsingCamera = false;
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(g_pPlayerStats->GetCurrentAmmo());
	if (pAmmo && pAmmo->eInstDamageType == DT_GADGET_CAMERA)
	{
		bUsingCamera = true;
	}
	if (!bUsingCamera || !g_pPlayerMgr->IsZoomed())
	{
		g_pInterfaceMgr->RemoveOverlay(OVM_CAMERA);
		g_pInterfaceMgr->RemoveOverlay(OVM_CAMERA_TARGET);
		return;
	}


	if (hObj)
	{
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);

		if ( dwUserFlags & USRFLG_GADGET_CAMERA )
		{
			g_pInterfaceMgr->CreateOverlay(OVM_CAMERA_TARGET);
			g_pInterfaceMgr->RemoveOverlay(OVM_CAMERA);
			return;

		}
	}

	g_pInterfaceMgr->CreateOverlay(OVM_CAMERA);
	g_pInterfaceMgr->RemoveOverlay(OVM_CAMERA_TARGET);


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDCrosshair::RenderArmed()
//
//	PURPOSE:	Draw the weapon crosshair
//
// ----------------------------------------------------------------------- //
void CHUDCrosshair::RenderArmed()
{
	LTIntPt pos;
	pos.x = g_pInterfaceResMgr->GetScreenWidth() / 2;
	pos.y = g_pInterfaceResMgr->GetScreenHeight() / 2;
	RenderArmed(LTFALSE, pos);
}

void CHUDCrosshair::RenderArmed(LTBOOL bMenu, LTIntPt pos)
{
	IClientWeaponBase* pClientWeapon = LTNULL;
	if (!bMenu)
	{
		pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	}

	float cx = (float)pos.x;
	float cy = (float)pos.y;

	g_pDrawPrim->SetXYWH(&m_ReticlePrim, cx-32.0f, cy-32.0f, 64.0f, 64.0f);

	// ABM TEST OF ISRAEL's CURSOR
	g_pDrawPrim->SetTexture(m_hReticleTex[0]);
	g_pDrawPrim->DrawPrim(&m_ReticlePrim);
/*
    LTFLOAT  fRad = 0.25;
	if (GetConsoleInt("CrosshairDynamic",0)  && pClientWeapon)
	{
		// Just use the weapon's dynamic perturb to determine the amount the
		// crosshair should scale...
		
		fRad = pClientWeapon->GetDynamicPerturb();
	}

	float fSize =  m_fCrosshairBarMin +  (fRad * m_fCrosshairBarRange);
	float fInside = m_fCrosshairGapMin +  (fRad * m_fCrosshairGapRange);
	float fOutside = fInside + fSize;
	float fSlide =  fInside + (fRad * fSize);


	uint32 cr = (uint32)(GetConsoleInt("CrosshairRed",0x00));
	uint32 cg = (uint32)(GetConsoleInt("CrosshairGreen",0xFF));
	uint32 cb = (uint32)(GetConsoleInt("CrosshairBlue",0xFF));
	uint32 ca = (uint32)(GetConsoleInt("CrosshairAlpha",0xFF));
	uint32 crosscolor = SET_ARGB(ca,cr,cg,cb);
	g_pDrawPrim->SetRGBA(&m_Poly,crosscolor);

    LTRect rect;

	int style = GetConsoleInt("CrosshairStyle",0);
    uint32 dwFlags = 0;
	switch (style)
	{
	case 0:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM;
		break;
	case 1:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM | CH_XLEFT | CH_XTOP | CH_XRIGHT | CH_XBOTTOM;
		break;
	case 2:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM | CH_DOT;
		break;
	case 3:
		dwFlags = CH_LEFT | CH_RIGHT | CH_FULLBOTTOM | CH_XLEFT | CH_XRIGHT;
		break;
	case 4:
		dwFlags = CH_DOT;
		break;
	case 5:
		dwFlags = CH_FULLRIGHT | CH_FULLBOTTOM;
		break;
	default:
		dwFlags = CH_LEFT | CH_TOP | CH_RIGHT | CH_BOTTOM;
		break;

	}

	g_pDrawPrim->SetTexture(LTNULL);


	//left
	if (dwFlags & CH_LEFT)
	{
		float x = cx - fOutside;
		float y = cy - 1.0f;
		float w = fSize;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
	if (dwFlags & CH_XLEFT)
	{
		float y = cy - fInside/2;
		float x = cx - fSlide - 1.0f;
		float w = 2.0f;
		float h = fInside;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	//right
	if (dwFlags & CH_RIGHT)
	{
		float y = cy - 1.0f;
		float x = cx + fInside;
		float w = fSize;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);

	}
	if (dwFlags & CH_XRIGHT)
	{
		float y = cy - fInside/2;
		float x = cx + fSlide - 1.0f;
		float w = 2.0f;
		float h = fInside;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
	if (dwFlags & CH_FULLRIGHT)
	{
		float y = cy - 1.0f;
		float x = cx;
		float w = fOutside;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	//top
	if (dwFlags & CH_TOP)
	{
		float y = cy - fOutside;
		float x = cx - 1.0f;
		float w = 2.0f;
		float h = fSize;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
	if (dwFlags & CH_XTOP)
	{
		float y = cy - fSlide - 1.0f;
		float x = cx - fInside/2.0f;
		float w = fInside;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	//bottom
	if (dwFlags & CH_BOTTOM)
	{
		float y = cy + fInside;
		float x = cx - 1.0f;
		float w = 2.0f;
		float h = fSize;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
	if (dwFlags & CH_XBOTTOM)
	{
		float y = cy + fSlide;
		float x = cx - fInside/2.0f;
		float w = fInside;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
	if (dwFlags & CH_FULLBOTTOM)
	{
		float y = cy;
		float x = cx - 1.0f;
		float w = 2.0f;
		float h = fOutside;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

	if (dwFlags & CH_DOT)
	{
		float y = cy - 1.0f;
		float x = cx - 1.0f;
		float w = 2.0f;
		float h = 2.0f;

		// draw our bars
		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}
*/
}

void CHUDCrosshair::RenderUnarmed()
{

	uint32 cr = (uint32)(GetConsoleInt("CrosshairRed",0x00));
	uint32 cg = (uint32)(GetConsoleInt("CrosshairGreen",0xFF));
	uint32 cb = (uint32)(GetConsoleInt("CrosshairBlue",0xFF));
	uint32 ca = (uint32)(GetConsoleInt("CrosshairAlpha",0xFF));
	uint32 crosscolor = SET_ARGB(ca,cr,cg,cb);
	g_pDrawPrim->SetRGBA(&m_Poly,crosscolor);

    LTRect rect;
	g_pDrawPrim->SetTexture(LTNULL);


	float y = (float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f - 1.0f;
	float x = (float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f - 1.0f;
	float w = 2.0f;
	float h = 2.0f;

	// draw our bars
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,h);
	g_pDrawPrim->DrawPrim(&m_Poly,1);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDCrosshair::RenderScope()
//
//	PURPOSE:	Draw the scope crosshair
//
// ----------------------------------------------------------------------- //
void CHUDCrosshair::RenderScope()
{
	g_pDrawPrim->SetTexture(LTNULL);

	g_pDrawPrim->SetRGBA(&m_Poly,argbBlack);

	float cx = 320.0f * g_pInterfaceResMgr->GetXRatio();
	float cy = 240.0f * g_pInterfaceResMgr->GetYRatio();

	float hR = g_vtScopeLRRadius.GetFloat() * cx * 2.0f;
	float hGap = g_vtScopeLRGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();
	float vR = g_vtScopeUDRadius.GetFloat() * cx * 2.0f;
	float vGap = g_vtScopeUDGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();


	//left post
	float x = cx - hR;
	float y = cy - 2.0f;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//right post
	x = cx + hGap;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//horizontal hair
	x = cx - hGap;
	y = cy - 1.0f;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,(hGap * 2.0f),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly,1);



	//top post
	x = cx - 2.0f;
	y = cy - vR;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//bottom post
	y = cy + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//vertical hair
	x = cx - 1.0f;
	y = cy - vGap;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,2.0f,(vGap * 2.0f));
	g_pDrawPrim->DrawPrim(&m_Poly,1);


	uint32 gold = SET_ARGB(255,140,128,20);
	g_pDrawPrim->SetRGBA(&m_Poly,gold);

	//left highlight
	x = cx - hR;
	y = cy - 1.0f;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//right highlight
	x = cx + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//top highlight
	x = cx - 1.0f;
	y = cy - vR;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly,1);

	//bottom highlight
	y = cy + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly,x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly,1);


}


void CHUDCrosshair::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	LTIntPt pos = g_pLayoutMgr->GetActivationTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_StrPos = pos;

	uint8 nTmp = g_pLayoutMgr->GetActivationTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nStrSz = nTmp;

	m_nStrJust = g_pLayoutMgr->GetActivationTextJustify(nCurrentLayout);

	uint32 color = g_pLayoutMgr->GetActivationTextColor(nCurrentLayout);
	if (color > 0)
		m_StrColor = color;

	color =	g_pLayoutMgr->GetActivationTextDisabledColor(nCurrentLayout);
	if (color > 0)
		m_StrDisColor = color;

	pos = g_pLayoutMgr->GetDebugTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_DbgPos = pos;

	nTmp = g_pLayoutMgr->GetDebugTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nDbgSz = nTmp;

	uint16 nWid = g_pLayoutMgr->GetDebugTextWidth(nCurrentLayout);
	if (nWid > 0)
		m_nDbgWidth = nWid;

	m_nDbgJust = g_pLayoutMgr->GetDebugTextJustify(nCurrentLayout);

	color = g_pLayoutMgr->GetDebugTextColor(nCurrentLayout);
	if (color > 0)
		m_DbgColor = color;

	pos = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextPos(nCurrentLayout);
	if (pos.x > 0)
		m_InfoPos = pos;

	nTmp = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextSize(nCurrentLayout);
	if (nTmp > 0)
		m_nInfoSz = nTmp;

	nWid = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextWidth(nCurrentLayout);
	if (nWid > 0)
		m_nInfoWidth = nWid;

	m_nInfoJust = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextJustify(nCurrentLayout);

	color = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextColor(nCurrentLayout);
	if (color > 0)
		m_InfoColor = color;

	color = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetInfoTextBadColor(nCurrentLayout);
	if (color > 0)
		m_InfoBadColor = color;

	m_fScale = g_pInterfaceResMgr->GetXRatio();

	m_x = (float)m_StrPos.x * m_fScale;
	m_y = (float)m_StrPos.y * m_fScale;
	uint8 nTextSize = (uint8)((float)m_nStrSz * m_fScale);

	m_pStr->SetPosition(m_x,m_y);
	m_pStr->SetCharScreenHeight(nTextSize);

	switch (m_nStrJust)
	{
	case 0:
		m_pStr->SetAlignmentH(CUI_HALIGN_LEFT);
		break;
	case 1:
		m_pStr->SetAlignmentH(CUI_HALIGN_CENTER);
		break;
	case 2:
		m_pStr->SetAlignmentH(CUI_HALIGN_RIGHT);
		break;
	}

	m_dbgx = (float)m_DbgPos.x * m_fScale;
	m_dbgy = (float)m_DbgPos.y * m_fScale;
	nTextSize = (uint8)((float)m_nDbgSz * m_fScale);

	m_pDbgStr->SetPosition(m_dbgx,m_dbgy);
	m_pDbgStr->SetCharScreenHeight(nTextSize);

	switch (m_nDbgJust)
	{
	case 0:
		m_pDbgStr->SetAlignmentH(CUI_HALIGN_LEFT);
		break;
	case 1:
		m_pDbgStr->SetAlignmentH(CUI_HALIGN_CENTER);
		break;
	case 2:
		m_pDbgStr->SetAlignmentH(CUI_HALIGN_RIGHT);
		break;
	}

	m_infox = (float)m_InfoPos.x * m_fScale;
	m_infoy = (float)m_InfoPos.y * m_fScale;
	nTextSize = (uint8)((float)m_nInfoSz * m_fScale);

	m_pInfoStr->SetPosition(m_infox,m_infoy);
	m_pInfoStr->SetCharScreenHeight(nTextSize);

	switch (m_nInfoJust)
	{
	case 0:
		m_pInfoStr->SetAlignmentH(CUI_HALIGN_LEFT);
		break;
	case 1:
		m_pInfoStr->SetAlignmentH(CUI_HALIGN_CENTER);
		break;
	case 2:
		m_pInfoStr->SetAlignmentH(CUI_HALIGN_RIGHT);
		break;
	}
}

