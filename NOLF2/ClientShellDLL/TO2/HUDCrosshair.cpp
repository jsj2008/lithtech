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
#include "TO2HUDMgr.h"
#include "HUDCrosshair.h"
#include "PlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "TO2GameClientShell.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "TargetMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"

namespace
{
	VarTrack g_vtScopeLRGap;
	VarTrack g_vtScopeUDGap;
	VarTrack g_vtScopeLRRadius;
	VarTrack g_vtScopeUDRadius;

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
	m_style = 0;
	m_bCanActivateTarget = true;
	m_nTargetTeam = INVALID_TEAM;
	m_eLevel = kHUDRenderDead;
}
	

LTBOOL CHUDCrosshair::Init()
{

	g_pDrawPrim->SetRGBA(&m_Poly[0],argbWhite);
	g_pDrawPrim->SetRGBA(&m_Poly[1],argbWhite);
	g_pDrawPrim->SetRGBA(&m_Poly[2],argbWhite);


    g_vtScopeLRGap.Init(g_pLTClient, "ScopeLRGap", NULL, 32.0f);
    g_vtScopeUDGap.Init(g_pLTClient, "ScopeUPGap", NULL, 32.0f);
    g_vtScopeLRRadius.Init(g_pLTClient, "ScopeLRRadius", NULL, 0.377f);
    g_vtScopeUDRadius.Init(g_pLTClient, "ScopeUDRadius", NULL, 0.34f);

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_StrPos = LTIntPt(320,250);
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


	uint8 style = (uint8)GetConsoleInt("CrosshairStyle",0);
	SetStyle(style);

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

	// See if we can show a crosshair or not...
	if (g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() &&
		!g_pMoveMgr->GetVehicleMgr()->CanShowCrosshair())
	{
		return;
	}

	if (!m_bEnabled)
		return;



	if (g_pPlayerMgr->IsPlayerDead())
	{
		g_pDrawPrim->SetTexture(LTNULL);
		g_pDrawPrim->SetRGBA(&m_Poly[2],argbWhite);

		float x = 319.0f * g_pInterfaceResMgr->GetXRatio();
		float y = 239.0f * g_pInterfaceResMgr->GetYRatio();
		float w = 2.0f * g_pInterfaceResMgr->GetXRatio();

		g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,w,w);
		g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	}
	else
	{
		g_pDrawPrim->SetTexture(m_hAccurate);
		g_pDrawPrim->DrawPrim(&m_Poly[0],1);

		if (GetConsoleInt("CrosshairDynamic",1))
		{
			g_pDrawPrim->SetTexture(m_hInaccurate);
			g_pDrawPrim->DrawPrim(&m_Poly[1],1);
		}
	}


	if( m_pStr )
	{
		m_pStr->SetPosition(m_x+2.0f,m_y+2.0f);
		m_pStr->SetColor(argbBlack);
		m_pStr->Render();

		m_pStr->SetPosition(m_x,m_y);
		if (m_bCanActivateTarget)
		{
			if (m_nTargetTeam == INVALID_TEAM)
			{
				m_pStr->SetColor(m_StrColor);
			}
			else
			{
				m_pStr->SetColor(m_TeamColor[m_nTargetTeam]);
			}
		}
		else
		{
			m_pStr->SetColor(m_StrDisColor);
		}

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
}


void CHUDCrosshair::Update()
{
	if (!m_bEnabled)
		return;


	
	IClientWeaponBase* pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();

    float fPerturb = 1.0f;

	// Just use the weapon's dynamic perturb to determine the amount the
	// crosshair should scale...
	if (pClientWeapon)
	{
		fPerturb = pClientWeapon->GetDynamicPerturb();
	}

	uint8 style = (uint8)GetConsoleInt("CrosshairStyle",0);
	if (style != m_style)
		SetStyle(style);

	bool bDynamic = (!!GetConsoleInt("CrosshairDynamic",1));


	uint32 cr = (uint32)(GetConsoleInt("CrosshairRed",0x00));
	uint32 cg = (uint32)(GetConsoleInt("CrosshairGreen",0xFF));
	uint32 cb = (uint32)(GetConsoleInt("CrosshairBlue",0xFF));
	uint32 ca = (uint32)( (1.0f - fPerturb) * 255.0f);

	if (!bDynamic)
		ca = 0xFF;

	uint32 crosscolor = SET_ARGB(ca,cr,cg,cb);
	g_pDrawPrim->SetRGBA(&m_Poly[0],crosscolor);

	if (bDynamic)
	{
		ca = (uint32)( fPerturb * 255.0f);
		crosscolor = SET_ARGB(ca,cr,cg,cb);
		g_pDrawPrim->SetRGBA(&m_Poly[1],crosscolor);
	}


	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		m_fScale = g_pInterfaceResMgr->GetXRatio();

		ScalePolies();

		m_x = (float)m_StrPos.x * m_fScale;
		m_y = (float)m_StrPos.y * m_fScale;
		uint8 nTextSize = (uint8)((float)m_nStrSz * m_fScale);
		m_pStr->SetCharScreenHeight(nTextSize);

		m_dbgx = (float)m_DbgPos.x * m_fScale;
		m_dbgy = (float)m_DbgPos.y * m_fScale;
		nTextSize = (uint8)((float)m_nDbgSz * m_fScale);
		m_pDbgStr->SetCharScreenHeight(nTextSize);
	}

	m_bCanActivateTarget = g_pPlayerMgr->GetTargetMgr()->CanActivateTarget();
	m_nTargetTeam = g_pPlayerMgr->GetTargetMgr()->GetTargetTeam();

	if (g_pPlayerMgr->IsSearching())
	{
		m_pStr->SetText(LoadTempString(IDS_TARGET_SEARCHING));
		return;
	}

	HOBJECT hObj = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
	uint16  nID = g_pPlayerMgr->GetTargetMgr()->GetTargetStringID();

	if (strcmp(g_pPlayerMgr->GetTargetMgr()->GetDebugString(),m_pDbgStr->GetText()))
		m_pDbgStr->SetText(g_pPlayerMgr->GetTargetMgr()->GetDebugString());

	//track whether or not we've zoomed since our last update
	static bool bZoomed = false;
	if( g_pPlayerMgr->IsZooming() )
	{
		if (!bZoomed)
		{
			bZoomed = true;
			m_pStr->SetText("");
		}
		
		return;
	}

	//if we have zoomed since our last full update, force an update
	if (!bZoomed && hObj == m_hObj && nID == m_nString)
		return;

	bZoomed = false;

	m_hObj = hObj;
	m_nString = nID;
	m_pStr->SetText(g_pPlayerMgr->GetTargetMgr()->GetTargetString());


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

		if ( (dwUserFlags & USRFLG_GADGET_CAMERA) && g_pPlayerMgr->InCameraGadgetRange( hObj ) )
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
//	ROUTINE:	CHUDCrosshair::RenderScope()
//
//	PURPOSE:	Draw the scope crosshair
//
// ----------------------------------------------------------------------- //
void CHUDCrosshair::RenderScope()
{
	g_pDrawPrim->SetTexture(LTNULL);

	g_pDrawPrim->SetRGBA(&m_Poly[2],argbBlack);

	float cx = 320.0f * g_pInterfaceResMgr->GetXRatio();
	float cy = 240.0f * g_pInterfaceResMgr->GetYRatio();

	float hR = g_vtScopeLRRadius.GetFloat() * cx * 2.0f;
	float hGap = g_vtScopeLRGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();
	float vR = g_vtScopeUDRadius.GetFloat() * cx * 2.0f;
	float vGap = g_vtScopeUDGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();


	//left post
	float x = cx - hR;
	float y = cy - 2.0f;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//right post
	x = cx + hGap;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//horizontal hair
	x = cx - hGap;
	y = cy - 1.0f;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,(hGap * 2.0f),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);



	//top post
	x = cx - 2.0f;
	y = cy - vR;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//bottom post
	y = cy + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//vertical hair
	x = cx - 1.0f;
	y = cy - vGap;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,2.0f,(vGap * 2.0f));
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);


	uint32 gold = SET_ARGB(255,140,128,20);
	g_pDrawPrim->SetRGBA(&m_Poly[2],gold);

	//left highlight
	x = cx - hR;
	y = cy - 1.0f;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//right highlight
	x = cx + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//top highlight
	x = cx - 1.0f;
	y = cy - vR;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);

	//bottom highlight
	y = cy + vGap;
	g_pDrawPrim->SetXYWH(&m_Poly[2],x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&m_Poly[2],1);


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

	LTVector vCol = g_pLayoutMgr->GetVector("Miscellaneous","Team1Color");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_TeamColor[0] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector("Miscellaneous","Team2Color");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_TeamColor[1] = SET_ARGB(0xFF,nR,nG,nB);

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
}




void CHUDCrosshair::SetStyle(uint8 style)
{
	char *szTag = "HUDCrosshair";
	char szAtt[32];
	sprintf(szAtt,"Crosshair%d",style);

	if (!g_pLayoutMgr->HasValue(szTag,szAtt))
	{
		style = 0;
		sprintf(szAtt,"Crosshair%d",0);
	}


	char szTexA[256];
	char szTexI[256];
	g_pLayoutMgr->GetString(szTag,szAtt,szTexA,sizeof(szTexA));
	SAFE_STRCPY(szTexI,szTexA);
	strcat(szTexA,"_A.dtx");
	strcat(szTexI,"_I.dtx");


	uint32 tw,th;
	m_hAccurate = g_pInterfaceResMgr->GetTexture(szTexA);
	g_pTexInterface->GetTextureDims(m_hAccurate,tw,th);
	m_fAccurateSz = (float)tw;

	//setup the poly UV coords
	SetupQuadUVs(m_Poly[0], m_hAccurate, 0.0f, 0.0f, 1.0f, 1.0f);

	m_hInaccurate = g_pInterfaceResMgr->GetTexture(szTexI);
	g_pTexInterface->GetTextureDims(m_hInaccurate,tw,th);
	m_fInaccurateSz = (float)tw;

	//setup the poly UV coords
	SetupQuadUVs(m_Poly[1], m_hInaccurate, 0.0f, 0.0f, 1.0f, 1.0f);

	ScalePolies();

	m_style = style;
}

void CHUDCrosshair::ScalePolies()
{
	float x = (float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f;
	float y = (float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f;
	float sz = m_fAccurateSz;
	
	x -= sz/2.0f;
	y -= sz/2.0f;
	g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,sz,sz);

	x = (float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f;
	y = (float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f;
	sz = m_fInaccurateSz;
	
	x -= sz/2.0f;
	y -= sz/2.0f;
	g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,sz,sz);
};

