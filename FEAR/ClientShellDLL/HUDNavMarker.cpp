// ----------------------------------------------------------------------- //
//
// MODULE  : HUDNavMarker.cpp
//
// PURPOSE : HUDItem to display a navigation marker
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDNavMarker.h"
#include "HUDMgr.h"
#include "NavMarkerTypeDB.h"
#include "InterfaceMgr.h"
#include "HUDNavMarkerMgr.h"
#include "PlayerCamera.h"
#include "GameModeMgr.h"
#include "CharacterFX.h"
#include "ClientDB.h"

LTVector2 CHUDNavMarker::s_vCenterPt;
float CHUDNavMarker::s_fArrowRadius = 0.0f;

static const float s_fMinimumRange = 0.3f;



//******************************************************************************************
//**
//** HUD Navigation marker display
//**
//******************************************************************************************

CHUDNavMarker::CHUDNavMarker()
{
	ClearData();
	m_Range.SetDropShadow(2);
	m_Text.SetDropShadow(2);
	m_FlashTimer.SetEngineTimer(RealTimeTimer::Instance());
	m_bScale = false;
}

void CHUDNavMarker::ClearData()
{
	m_hIcon = NULL;
	m_hArrow = NULL;
	m_bOnScreen = false;
	m_bUseRange = false;
	m_fRange = 0.0f;
	m_hType = NULL;
	m_vTargetPos.Init();
	m_vWorldOffset.Init();
	m_Range.FlushTexture();
	m_Text.FlushTexture();

}


void CHUDNavMarker::UpdateData(const HUDNavMarker_create* pNMCS)
{
	if (!pNMCS)
	{
		ClearData();
		return;
	}

	m_bIsActive = pNMCS->m_bIsActive;
	m_hTarget = pNMCS->m_hTarget;

	SetType(pNMCS->m_hType);
	ScaleChanged();
	
	m_nTeamId = pNMCS->m_nTeamId;

	m_pCharFx = pNMCS->m_pCharFx;

	wchar_t const* pwszText = pNMCS->m_pText ? pNMCS->m_pText : L"";
	if (LTStrCmp(pwszText,m_Text.GetText()) != 0)
		m_Text.SetText(pwszText);

	m_fRange = 0.0f;
	m_vTargetPos.Init();
	m_bOnScreen = false;
	
	if (pNMCS->m_pOverrideTex && pNMCS->m_pOverrideTex[0])
	{
		m_hIcon.Load(pNMCS->m_pOverrideTex);
		m_cIconColor = argbWhite;
		DrawPrimSetRGBA(m_Icon,m_cIconColor);
	}

}

void CHUDNavMarker::Update()
{
}

// this function will fail if the marker has a valid target object
bool  CHUDNavMarker::SetPos(const LTVector& vPos) 
{
	if (m_hTarget) return false;
	m_vTargetPos = vPos + m_vWorldOffset;
	return true;
}

void CHUDNavMarker::Render()
{
	if (!IsActive()) return;


	LTVector const& vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	// Get the vector to the target
	if (m_hTarget)
	{
		if (m_hTarget == g_pLTClient->GetClientObject())
			return;
		g_pLTClient->GetObjectPos(m_hTarget,&m_vTargetPos);
		m_vTargetPos += m_vWorldOffset;
	}

	//Calculate on screen position
	LTVector vScreenPos3 = g_pInterfaceMgr->GetScreenFromWorldPos(m_vTargetPos,g_pPlayerMgr->GetPlayerCamera()->GetCamera(),m_bOnScreen);
	LTVector2 vScreenPos(vScreenPos3.x,vScreenPos3.y);

	// Calculate range
	m_fRange = vCameraPos.Dist(m_vTargetPos) / 100.0f;

	//if we're not onscreen or we are too close, don't bother rendering
	if (!m_bOnScreen || m_fRange < s_fMinimumRange) return;

	UpdateFlash();

	float fAlpha = 1.0f;

	if (IsMultiplayerGameClient() && g_pNavMarkerMgr->MultiplayerFilter() && m_fFadeAngle > 0.0f)
	{

		LTRotation rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
		LTVector vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();


		// Get the vector to the target
		LTVector vTarget = m_vTargetPos - vCameraPos;
		vTarget.Normalize();

		float fF = vTarget.Dot(rRot.Forward());
		float a = acosf(fF);

		if (a <= m_fFadeAngle)
		{
			fAlpha = 1.0f;
		}
		else 
		{
			fAlpha = 1.0f - (a - m_fFadeAngle) / m_fFadeAngle;
			if (fAlpha < 0.0f)
			{
				return;
			}

		}
	}

	if (m_fMinAlpha < 1.0f)
	{
		
		float fZoom = g_pPlayerMgr->GetPlayerCamera()->GetZoomMag();
		float fRange = m_fRange;
		if (fZoom > 0.0f)
		{
			fRange = m_fRange / fZoom;
		}
		
		if (fRange < m_fMinFade)
		{
			fAlpha *= 1.0f;
		}
		else if (fRange >= m_fMaxFade)
		{
			fAlpha *= m_fMinAlpha;
		}
		else
		{
			float x = (fRange - m_fMinFade) / m_fFadeRange;
			fAlpha *= 1.0f - (x * m_fAlphaRange);
		}

	}


	SetAlpha(fAlpha);

	//		Update range text
	wchar_t szTmp[16];
	FormatString("HUD_Range_NavMarker_Format",szTmp,LTARRAYSIZE(szTmp),m_fRange);
	if (!LTStrIEquals(szTmp,m_Range.GetText()) )
		m_Range.SetText(szTmp);

	LTVector2 vSize = m_vIconSize;
	LTVector2 vOffset = m_vIconOffset;
	if (m_bScale)
	{
		vOffset.x *= g_pInterfaceResMgr->GetXRatio();
		vOffset.y *= g_pInterfaceResMgr->GetYRatio();
		vSize.x *= g_pInterfaceResMgr->GetXRatio();
		vSize.y *= g_pInterfaceResMgr->GetYRatio();
	}
	LTVector2 vPos = vScreenPos + vOffset;

	DrawPrimSetXYWH(m_Icon,vPos.x,vPos.y,vSize.x,vSize.y);

	vOffset = m_vRangeOffset;
	if (m_bScale)
	{
		vOffset.x *= g_pInterfaceResMgr->GetXRatio();
		vOffset.y *= g_pInterfaceResMgr->GetYRatio();
	}
	vPos = vScreenPos + vOffset;
	m_Range.SetPos( vPos );

	vOffset = m_vTextOffset;
	if (m_bScale)
	{
		vOffset.x *= g_pInterfaceResMgr->GetXRatio();
		vOffset.y *= g_pInterfaceResMgr->GetYRatio();
	}
	vPos = vScreenPos + vOffset;
	m_Text.SetPos( vPos );

	if (m_hIcon)
	{
		g_pDrawPrim->SetTexture(m_hIcon);
		g_pDrawPrim->DrawPrim(&m_Icon,1);
	}

	if (m_bUseRange)
		m_Range.Render();

	if (!m_Text.IsEmpty())
		m_Text.Render();

}

void CHUDNavMarker::RenderArrow()
{
	if (!IsActive() || !m_hArrow) return;
	if (m_fRange < s_fMinimumRange) return;

	UpdateFlash();

	//	Calculate arrow position
	LTRotation rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
	LTVector vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();

	// Get the relevant axes
//	LTVector vForward = rRot.Forward(); - not needed for the calculations
	LTVector vRight = rRot.Right();
	LTVector vUp = rRot.Up();

	// Get the vector to the target
	LTVector vTarget = m_vTargetPos - vCameraPos;
	vTarget.Normalize();

	//we are currently aimed at vForward, we want to aim at vTarget
	// calculate the angle we would need to turn
//	float fF = vTarget.Dot(vForward);
	float fR = vTarget.Dot(vRight);
	float fU = vTarget.Dot(vUp);
	float a = atan2f(fU,fR);
	float xc = cosf(a);
	float yc = sinf(a);

	//calculate the base position on the arrow
	float x = s_vCenterPt.x + s_fArrowRadius * xc;
	float y = s_vCenterPt.y - s_fArrowRadius * yc;

	// Now, we need to rotate the arrow texture so it's pointing at the correct angle

	// first generate a 2-D vector along the angle
	LTVector vA(xc,yc,0);

	//now find the perpindicular 2-D vector
	LTVector vP = vA.Cross(LTVector(0,0,1));

	// now scale these vectors
	vA *= m_vArrowSize.x / 2.0f;
	vP *= m_vArrowSize.y / 2.0f;

	m_Arrow.verts[0].pos.x = x - vP.x - vA.x;
	m_Arrow.verts[0].pos.y = y + vP.y + vA.y;
	m_Arrow.verts[1].pos.x = x - vP.x + vA.x;
	m_Arrow.verts[1].pos.y = y + vP.y - vA.y;
	m_Arrow.verts[2].pos.x = x + vP.x + vA.x;
	m_Arrow.verts[2].pos.y = y - vP.y - vA.y;
	m_Arrow.verts[3].pos.x = x + vP.x - vA.x;
	m_Arrow.verts[3].pos.y = y - vP.y + vA.y;

	float fAlpha = 1.0f;
	if (m_fMinAlpha < 1.0f)
	{
		if (m_fRange < m_fMinFade)
		{
			fAlpha *= 1.0f;
		}
		else if (m_fRange >= m_fMaxFade)
		{
			fAlpha *= m_fMinAlpha;
		}
		else
		{
			float x = (m_fRange - m_fMinFade) / m_fFadeRange;
			fAlpha *= 1.0f - (x * m_fAlphaRange);
		}

	}
	SetAlpha(fAlpha);

	if (m_hArrow)
	{
		g_pDrawPrim->SetTexture(m_hArrow);
		g_pDrawPrim->DrawPrim(&m_Arrow,1);
	}
}


void CHUDNavMarker::ScaleChanged()
{

	LTVector2 vC( (float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f, (float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f);
	if (!vC.NearlyEquals(s_vCenterPt) )
	{
		s_vCenterPt = vC;
		s_fArrowRadius = (float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f - 32.0f;
	}


	if (m_bScale)
	{
		uint32 nHeight = (uint32)(g_pInterfaceResMgr->GetYRatio() * g_pNavMarkerTypeDB->GetTextSize(m_hType));
		m_Text.SetFontHeight(nHeight);
	}

}

bool CHUDNavMarker::IsActive() const
{
	if ( GameModeMgr::Instance( ).m_grbUseTeams && m_nTeamId != INVALID_TEAM && !g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(m_nTeamId))
		return false;

	// Check if the character is alive.
	if( m_hTarget && m_pCharFx )
	{
		// Don't bother with dead character markers.
		if( m_pCharFx->m_cs.bIsDead || !m_pCharFx->m_cs.hModel )
			return false;

		// Skip invisible guys, like other spectators.
		uint32 nFlags = 0;
		g_pCommonLT->GetObjectFlags( m_pCharFx->GetServerObj(), OFT_Flags, nFlags );
		if( !( nFlags & FLAG_VISIBLE ))
			return false;
	}

	return (m_bIsActive);
}
uint8 CHUDNavMarker::GetPriority() const
{
	if (!m_hType) return 0;
	//get priority from type
	return g_pNavMarkerTypeDB->GetPriority(m_hType);
}


void CHUDNavMarker::SetType(HRECORD hType)
{
	m_hType = hType;

	if (!m_hType) return;
	
	m_vWorldOffset = g_pNavMarkerTypeDB->GetWorldOffset(m_hType);

	m_hIcon.Load( g_pNavMarkerTypeDB->GetIconTexture(m_hType));
	m_cIconColor = g_pNavMarkerTypeDB->GetIconColor(m_hType);
	DrawPrimSetRGBA(m_Icon,m_cIconColor);
	SetupQuadUVs(m_Icon,m_hIcon,0.0f,0.0f,1.0f,1.0f);

	m_hArrow.Load(g_pNavMarkerTypeDB->GetArrowTexture(m_hType));
	m_cArrowColor = g_pNavMarkerTypeDB->GetArrowColor(m_hType);
	DrawPrimSetRGBA(m_Arrow,m_cArrowColor);
	SetupQuadUVs(m_Arrow,m_hArrow,0.0f,0.0f,1.0f,1.0f);

	m_fFadeAngle = DEG2RAD(g_pNavMarkerTypeDB->GetMultiplayerFadeAngle(m_hType));
	m_bUseRange = g_pNavMarkerTypeDB->UseRange(m_hType);
	m_cTextColor = g_pNavMarkerTypeDB->GetTextColor(m_hType);
	m_Range.SetColor(m_cTextColor);
	m_Text.SetColor(m_cTextColor);

	m_vIconSize = g_pNavMarkerTypeDB->GetIconSize(m_hType);
	m_vIconOffset = g_pNavMarkerTypeDB->GetIconOffset(m_hType);
	//adjust offset so icon is centered
	m_vIconOffset -= (m_vIconSize / 2.0f);
	m_vArrowSize = g_pNavMarkerTypeDB->GetArrowSize(m_hType);
	m_vRangeOffset = g_pNavMarkerTypeDB->GetRangeOffset(m_hType);
	m_vTextOffset = g_pNavMarkerTypeDB->GetTextOffset(m_hType);

	m_Text.SetFont( CFontInfo(g_pNavMarkerTypeDB->GetTextFont(m_hType),g_pNavMarkerTypeDB->GetTextSize(m_hType)) );
	m_Range.SetSourceString(g_pNavMarkerMgr->GetRangeTextureString());

	m_fMinAlpha = g_pNavMarkerTypeDB->GetMinimumFadeAlpha(m_hType);

	LTVector2 vFadeRange = g_pNavMarkerTypeDB->GetFadeRange(m_hType);
	m_fMinFade = vFadeRange.x;
	m_fMaxFade = vFadeRange.y;

	m_fFadeRange = m_fMaxFade - m_fMinFade;
	m_fAlphaRange = 1.0f - m_fMinAlpha;


}


void CHUDNavMarker::SetAlpha(float fAlpha)
{
	if (fAlpha < 1.0f)
	{
		DrawPrimSetRGBA(m_Icon,FadeARGB(m_cIconColor,fAlpha));
		DrawPrimSetRGBA(m_Arrow,FadeARGB(m_cArrowColor,fAlpha));
		m_Range.SetColor(FadeARGB(m_cTextColor,fAlpha));
		m_Text.SetColor(FadeARGB(m_cTextColor,fAlpha));
	}
	else
	{
		DrawPrimSetRGBA(m_Icon,m_cIconColor);
		DrawPrimSetRGBA(m_Arrow,m_cArrowColor);
		m_Range.SetColor(m_cTextColor);
		m_Text.SetColor(m_cTextColor);
	}
}


void CHUDNavMarker::Flash(const char* pszFlash)
{
	ClientDB& clientDB = ClientDB::Instance();
	HRECORD hFlash = clientDB.GetHUDFlashRecord(pszFlash);
	if (hFlash)
	{
		m_cFlashColor = ( uint32 )clientDB.GetInt32(hFlash,CDB_cFlashColor, 0, 0xFFFFFFFF);

		float fDuration = clientDB.GetFloat(hFlash,CDB_fFlashDuration);
		float fRate = LTMAX(1.0f,clientDB.GetFloat(hFlash,CDB_fFlashRate));
		m_nFlashCount = uint32(fDuration * fRate);
		m_FlashTimer.Start(fDuration / fRate);
	}
}

void CHUDNavMarker::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.IsStarted())
		return;

	//current flash timed out
	if (m_FlashTimer.IsTimedOut())
	{
		
		if (m_nFlashCount > 0)
		{
			//still flashing, restart timer
			m_FlashTimer.Start(m_FlashTimer.GetDuration());
			--m_nFlashCount;
		}
		else
		{
			//done flashing
			m_FlashTimer.Stop();
		}

	}

	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		DrawPrimSetRGBA(m_Icon,m_cFlashColor);
		DrawPrimSetRGBA(m_Arrow,m_cFlashColor);
		m_Range.SetColor(m_cFlashColor);
		m_Text.SetColor(m_cFlashColor);
	}
	else
	{
		DrawPrimSetRGBA(m_Icon,m_cIconColor);
		DrawPrimSetRGBA(m_Arrow,m_cArrowColor);
		m_Range.SetColor(m_cTextColor);
		m_Text.SetColor(m_cTextColor);
	}
}
