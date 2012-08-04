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
#include "HUDMgr.h"
#include "HUDCrosshair.h"
#include "PlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"
#include "ClientWeaponMgr.h"
#include "TargetMgr.h"
#include "AutoTargetMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "PlayerCamera.h"
#include "AccuracyMgr.h"
#include "LadderMgr.h"
#include "GameModeMgr.h"
#include "MissionMgr.h"
#include "ClientDB.h"
#include "HUDActivateObject.h"
#include "ActivateObjectFX.h"

namespace
{
	VarTrack g_vtScopeLRGap;
	VarTrack g_vtScopeUDGap;
	VarTrack g_vtScopeLRRadius;
	VarTrack g_vtScopeUDRadius;
	VarTrack g_vtDisableCrosshair;
	VarTrack g_vtPerturbScale;
}



//******************************************************************************************
//** HUD crosshair
//******************************************************************************************
CHUDCrosshair::CHUDCrosshair()
{
	m_UpdateFlags = kHUDFrame;
	m_bEnabled = true;
	m_bArmed = false;
//	m_style = 0;
	m_bCanActivateTarget = true;
	m_nTargetTeam = INVALID_TEAM;
	m_fTargetTime = 0.0f;
	m_fHoldTime = 0.0f;


	m_eLevel = kHUDRenderDead;
	m_bRangeEnabled = false;
	m_hRangeTexture = NULL;

}
	

bool CHUDCrosshair::Init()
{
	g_pCrosshair = this;

	DrawPrimSetRGBA(m_Poly,argbWhite);

    g_vtScopeLRGap.Init(g_pLTClient, "ScopeLRGap", NULL, 32.0f);
    g_vtScopeUDGap.Init(g_pLTClient, "ScopeUPGap", NULL, 32.0f);
    g_vtScopeLRRadius.Init(g_pLTClient, "ScopeLRRadius", NULL, 0.377f);
    g_vtScopeUDRadius.Init(g_pLTClient, "ScopeUDRadius", NULL, 0.34f);
	g_vtDisableCrosshair.Init(g_pLTClient, "DisableCrosshair", NULL, 0.f);

	g_vtPerturbScale.Init(g_pLTClient, "PerturbScale", NULL, 0.5f);

	m_StrDisColor = 0x80808080;


	UpdateLayout();	

	ScaleChanged();


	return true;
}

void CHUDCrosshair::Term()
{
	if (m_hRangeTexture)
	{
		g_pTextureString->ReleaseTextureString(m_hRangeTexture);
		m_hRangeTexture = NULL;
	}
	g_pCrosshair = NULL;
}

bool CHUDCrosshair::CanShowCrosshair()
{
	if (!m_bEnabled)
		return false;

	if (g_vtDisableCrosshair.GetFloat())
	{
		return false;
	}

	// See if we can show a crosshair or not...
	if( (g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() && !g_pMoveMgr->GetVehicleMgr()->CanShowCrosshair()) || 
		!g_pPlayerMgr->GetPlayerCamera( )->CanShowCrosshair( ))
	{
		return false;
	}

	// See if the server gamestate prevents us from showing the crosshair.
	switch( g_pMissionMgr->GetServerGameState( ))
	{
	case EServerGameState_ShowScore:
	case EServerGameState_ExitingLevel:
	case EServerGameState_None:
	case EServerGameState_Loading:
		return false;
	}

	return true;
}

void CHUDCrosshair::Render()
{
	if (m_bRangeEnabled)
	{
		m_RangeDisplay.Render();
	}

	if( !CanShowCrosshair( ))
		return;

	SetRenderState();

	if (!g_pPlayerMgr->IsPlayerAlive() || LadderMgr::Instance().IsClimbing() )
	{
		g_pDrawPrim->SetTexture(NULL);
		LTPoly_G4 tmpPoly;

		DrawPrimSetRGBA(tmpPoly,argbWhite);

		float w = 1.0f;

		DrawPrimSetXYWH(tmpPoly,m_vCenter.x,m_vCenter.y,w,w);
		g_pDrawPrim->DrawPrim(&tmpPoly,1);

	}
	else if (CAutoTargetMgr::Instance().IsLockedOn())
	{

		LTPoly_G4 tmpPoly;
		DrawPrimSetRGBA(tmpPoly,argbWhite);

		float w = 1.0;

		DrawPrimSetXYWH(tmpPoly,m_vCenter.x,m_vCenter.y,w,w);
		g_pDrawPrim->DrawPrim(&tmpPoly,1);

		bool bOnScreen = false;
		LTVector vPos = CAutoTargetMgr::Instance().GetCrosshairPos();
		LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(vPos, g_pPlayerMgr->GetPlayerCamera()->GetCamera(),bOnScreen);

		RenderCrosshair(LTVector2(pos.x,pos.y));
	}
	else
	{
		RenderCrosshair(m_vCenter);
	}

	if (m_bCanActivateTarget)
	{
		if (GameModeMgr::Instance( ).m_grbUseTeams && m_nTargetTeam != INVALID_TEAM)
		{
			if (g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(m_nTargetTeam))
				m_Text.SetColor(m_TeamColor[0]);
			else
				m_Text.SetColor(m_TeamColor[1]);
		}
		else
		{
			m_Text.SetColor(m_cTextColor);
		}
	}
	else
	{
		uint32 dwDisabledColor = m_StrDisColor;

		// If there is an activate object use the disabled color for hte text...
		const CActivateObjectHandler *pActivateObj = g_pHUDActivateObject->GetActivateObjectHandler( );
		if( pActivateObj )
		{
			HRECORD hRecord = DATABASE_CATEGORY( Activate ).GetRecordByIndex( pActivateObj->m_nId );
			dwDisabledColor = DATABASE_CATEGORY( Activate ).GETRECORDATTRIB( hRecord, DisabledColor );	
		}

		m_Text.SetColor( dwDisabledColor );
	}

	bool bRenderText = true;
	//if we're in a MP game and looking at an opponent... do not display the name right away
	if (IsMultiplayerGameClient())
	{
		// Show names immediately if we're spectating
		if (g_pPlayerMgr->IsSpectating())
		{
			bRenderText = true;
		}
		//(names of friendlies don't need to show at all here)
		else if ( g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(m_nTargetTeam))
		{
			bRenderText = false;
		}
		// Otherwise, we need to aim at them for a while. 
		else 
		{
			//see if we've been aimed at him long enough
			double fHolding = RealTimeTimer::Instance().GetTimerAccumulatedS() - m_fTargetTime;
			if (fHolding < m_fHoldTime)
			{
				bRenderText = false;
			}
		}
	}

	if (bRenderText)
	{
		m_Text.Render();
	}
}


void CHUDCrosshair::Update()
{
	//allow the crosshair to be updated if you are in the screen state
	//	(i.e. in the options menu)
	if (g_pInterfaceMgr->GetGameState() != GS_SCREEN)
	{
		if (!m_bEnabled)
			return;

		if (g_vtDisableCrosshair.GetFloat())
		{
			return;
		}
	}

	if (m_bRangeEnabled)
	{
		wchar_t wsRange[16];
		float fRng = (g_pPlayerMgr->GetTargetMgr()->GetTargetRange())/100.0f;
		if (fRng < 1000.0f)
		{
			FormatString("HUD_Range_Scope_Format",wsRange,LTARRAYSIZE(wsRange),fRng);
		}
		else
		{
			FormatString("HUD_Range_Scope_Invalid",wsRange,LTARRAYSIZE(wsRange),fRng);
		}
		m_RangeDisplay.SetText(wsRange);
	}

	float fSize = GetConsoleFloat("CrosshairSize",12.0f);
	if (fSize != m_fBarSize)
	{
		m_fBarSize = fSize;
		m_fMaxSize = m_vCenter.y - m_fBarSize;

	}


	uint8 cr = (uint8)(GetConsoleInt("CrosshairRed",0x00));
	uint8 cg = (uint8)(GetConsoleInt("CrosshairGreen",0xFF));
	uint8 cb = (uint8)(GetConsoleInt("CrosshairBlue",0xFF));
	uint8 ca = (uint8)(GetConsoleInt("CrosshairAlpha",0xFF));
	if (g_pHUDMgr->GetFlickerLevel())
	{
		ca = uint8( m_fFlicker * float(ca) );
	}
	m_cCrosshairColor = SET_ARGB(ca,cr,cg,cb);

	m_bCanActivateTarget = g_pPlayerMgr->GetTargetMgr()->CanActivateTarget();
	m_nTargetTeam = g_pPlayerMgr->GetTargetMgr()->GetTargetTeam();

	HOBJECT hObj = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
	const char* szID = g_pPlayerMgr->GetTargetMgr()->GetTargetStringID();
	const wchar_t* szString = g_pPlayerMgr->GetTargetMgr()->GetTargetString();

	// If our target or string changed since the last update, force an update.
	if( hObj == m_hObj )
	{
		if( szID == m_szStringID || ( szID && m_szStringID && LTStrIEquals(szID, m_szStringID)) )
		{			
			// At this point, if the string id is not null then the strings have not changed.
			if( szID != NULL )
			{
				return;
			}

			// Because MultiPlayer mode will set the string with-out setting the string-id, we
			// need to check the string as well.
			const wchar_t* const szCurrentString = m_Text.GetText();
			if( szString == szCurrentString || ( szString && szCurrentString && LTStrIEquals(szString, szCurrentString)) )
				return;
		}
	}

	// Alright, something has changed, be sure to update our info.

	m_fTargetTime = RealTimeTimer::Instance().GetTimerAccumulatedS();

	m_hObj = hObj;
	m_szStringID = szID;
	m_Text.SetText(g_pPlayerMgr->GetTargetMgr()->GetTargetString());
}


void CHUDCrosshair::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	LTVector2 vPos;
	vPos.x = float(m_vRangePos.x) * g_pInterfaceResMgr->GetXRatio();
	vPos.y = float(m_vRangePos.y) * g_pInterfaceResMgr->GetYRatio();
	m_RangeDisplay.SetPos(vPos);

	m_vCenter.x = (float)(g_pInterfaceResMgr->GetScreenWidth()-1) / 2.0f;
	m_vCenter.y = (float)(g_pInterfaceResMgr->GetScreenHeight()-1) / 2.0f;

	uint32 nHeight = m_sTextFont.m_nHeight;
	if (GetConsoleBool("UseTextScaling",false))
	{
		nHeight = (uint32)(g_pInterfaceResMgr->GetYRatio() * m_sTextFont.m_nHeight);
	}
	m_Text.SetFontHeight(nHeight);

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
	g_pDrawPrim->SetTexture(NULL);

	LTPoly_G4 tmpPoly;
	DrawPrimSetRGBA(tmpPoly,argbBlack);

	float cx = 320.0f * g_pInterfaceResMgr->GetXRatio();
	float cy = 240.0f * g_pInterfaceResMgr->GetYRatio();

	float hR = g_vtScopeLRRadius.GetFloat() * cx * 2.0f;
	float hGap = g_vtScopeLRGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();
	float vR = g_vtScopeUDRadius.GetFloat() * cx * 2.0f;
	float vGap = g_vtScopeUDGap.GetFloat() * g_pInterfaceResMgr->GetXRatio();


	//left post
	float x = cx - hR;
	float y = cy - 2.0f;
	DrawPrimSetXYWH(tmpPoly,x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	//right post
	x = cx + hGap;
	DrawPrimSetXYWH(tmpPoly,x,y,(hR-hGap),4.0f);
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	//horizontal hair
	x = cx - hGap;
	y = cy - 1.0f;
	DrawPrimSetXYWH(tmpPoly,x,y,(hGap * 2.0f),2.0f);
	g_pDrawPrim->DrawPrim(&tmpPoly,1);



	//top post
	x = cx - 2.0f;
	y = cy - vR;
	DrawPrimSetXYWH(tmpPoly,x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	/*
	//bottom post
	y = cy + vGap;
	DrawPrimSetXYWH(tmpPoly,x,y,4.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&tmpPoly,1);
	*/

	//vertical hair
	x = cx - 1.0f;
	y = cy - vGap;
	DrawPrimSetXYWH(tmpPoly,x,y,2.0f,(vGap * 2.0f));
	g_pDrawPrim->DrawPrim(&tmpPoly,1);


	uint32 gold = SET_ARGB(255,140,128,20);
	DrawPrimSetRGBA(tmpPoly,gold);

	//left highlight
	x = cx - hR;
	y = cy - 1.0f;
	DrawPrimSetXYWH(tmpPoly,x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	//right highlight
	x = cx + vGap;
	DrawPrimSetXYWH(tmpPoly,x,y,(hR-hGap),2.0f);
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	/*
	//top highlight
	x = cx - 1.0f;
	y = cy - vR;
	DrawPrimSetXYWH(tmpPoly,x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	//bottom highlight
	y = cy + vGap;
	DrawPrimSetXYWH(tmpPoly,x,y,2.0f,(vR-vGap));
	g_pDrawPrim->DrawPrim(&tmpPoly,1);
	*/


}



void CHUDCrosshair::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDCrosshair");
	}

	CHUDItem::UpdateLayout();


	m_RangeDisplay.SetColor(m_cTextColor);
	m_RangeDisplay.SetAlignment(kLeft);

	static std::wstring srcStr = LoadString("HUD_Range_Chars");
	if (m_hRangeTexture)
	{
		LTRESULT res = g_pTextureString->RecreateTextureString(m_hRangeTexture,srcStr.c_str(),m_sTextFont);
		if (res != LT_OK)
		{
			g_pTextureString->ReleaseTextureString(m_hRangeTexture);
			m_hRangeTexture = NULL;
		}
	}
	else
	{
		m_hRangeTexture = g_pTextureString->CreateTextureString(srcStr.c_str(),m_sTextFont);
	}
	m_RangeDisplay.SetSourceString(m_hRangeTexture);

	

	m_StrDisColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_TeamColor[0] = g_pLayoutDB->GetTeamColor(0);
	m_TeamColor[1] = g_pLayoutDB->GetTeamColor(1);


	//how long to hold crosshair on enemy to see name
	m_fHoldTime = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);


}





void CHUDCrosshair::SetRangeDisplay(bool bDisplay, const LTVector2n* pvDisplayPos /* = NULL */, uint32 cDisplayColor /* = 0 */)
{
	m_bRangeEnabled = bDisplay;

	if (pvDisplayPos)
	{
		m_vRangePos = *pvDisplayPos;
		LTVector2 vPos;
		vPos.x = float(m_vRangePos.x) * g_pInterfaceResMgr->GetXRatio();
		vPos.y = float(m_vRangePos.y) * g_pInterfaceResMgr->GetYRatio();
		m_RangeDisplay.SetPos(vPos);
	}

	if (cDisplayColor)
		m_RangeDisplay.SetColor(cDisplayColor);
	else
		m_RangeDisplay.SetColor(m_cTextColor);

}


void CHUDCrosshair::RenderCrosshair(const LTVector2& vPos, bool bInterface /* = false */)
{

	g_pDrawPrim->SetTexture(NULL);

	float fInside = 8.0f;
	if (!bInterface)
		fInside = floorf( (g_vtPerturbScale.GetFloat() * CAccuracyMgr::Instance().GetCurrentWeaponPerturb()) );
		
	if (fInside > m_fMaxSize)
		fInside = m_fMaxSize;

	//center
	static float fThickness = 1.0f;
	float x = vPos.x;
	float y = vPos.y;
	float w = fThickness;
	float h = fThickness;

	LTPoly_G4 tmpPoly[4];
	DrawPrimSetRGBA(tmpPoly[0],m_cCrosshairColor);
	DrawPrimSetRGBA(tmpPoly[1],m_cCrosshairColor);
	DrawPrimSetRGBA(tmpPoly[2],m_cCrosshairColor);
	DrawPrimSetRGBA(tmpPoly[3],m_cCrosshairColor);


//don't draw 
//	DrawPrimSetXYWH(tmpPoly,x,y,w,h);
//	g_pDrawPrim->DrawPrim(&tmpPoly,1);

	//left
	x = (vPos.x - (fInside + m_fBarSize));
	y = vPos.y;
	w = m_fBarSize;
	h = fThickness;
	if (fInside < 1.0f)
	{
		DrawPrimSetXYWH(tmpPoly[0],x,y,w+fThickness,h);
	}
	else
	{
		DrawPrimSetXYWH(tmpPoly[0],x,y,w,h);
	}

	//right
	x = vPos.x + fInside + fThickness;
	y = vPos.y;
	DrawPrimSetXYWH(tmpPoly[1],x,y,w,h);

	//top
	x = vPos.x;
	y = (vPos.y - (fInside + m_fBarSize));
	w = fThickness;
	h = m_fBarSize;
	DrawPrimSetXYWH(tmpPoly[2],x,y,w,h);

	//bottom
	x = vPos.x;
	y = vPos.y + fInside + fThickness;
	DrawPrimSetXYWH(tmpPoly[3],x,y,w,h);


	g_pDrawPrim->DrawPrim(tmpPoly, 4);
}
