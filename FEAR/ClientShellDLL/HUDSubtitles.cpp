// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSubtitles.cpp
//
// PURPOSE : Implementation of CHUDSubtitles to display subtitles
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDSubtitles.h"
#include "PlayerMgr.h"
#include "InterfaceMgr.h"
#include "VarTrack.h"
#include "PlayerCamera.h"

VarTrack g_vtAdjustedRadius;

void TestSubtitleFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc <= 0) return;

	LTVector vPos;

	vPos.Init();

	g_pSubtitles->Show(argv[0],vPos);
	
}


CHUDSubtitles::CHUDSubtitles()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
	m_bVisible = false;
	m_fElapsedTime = 0.0f;

	m_vSpeakerPos.Init();
	m_fRadius = 0.0;
	m_fDuration = -1.0f;
	m_bSubtitlePriority = false;
}
	

bool CHUDSubtitles::Init()
{
    g_pLTClient->RegisterConsoleProgram("TestSubtitle", TestSubtitleFn);

	if (!g_vtAdjustedRadius.IsInitted())
	{
		g_vtAdjustedRadius.Init(g_pLTClient, "SubtitleSoundRadiusPercent", NULL, 0.77f);
	}


	UpdateLayout();

	return true;

}
void CHUDSubtitles::Term()
{
}

void CHUDSubtitles::Render()
{
	// Sanity checks...
	if (!m_bVisible) return;
	if (GetConsoleInt("Subtitles",0) == 0) return;

	// Only show subtitles if conversations in range...
	LTVector vListenerPos;
	bool bListenerInClient;
	LTRotation rRot;
	g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

	bool bForceDraw = (bool)(m_vSpeakerPos == LTVector(0, 0, 0));
	bForceDraw = ((g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic) ? true : bForceDraw);

	LTVector vPos = m_vSpeakerPos - vListenerPos;
	float fAdjustedRadius = m_fRadius * g_vtAdjustedRadius.GetFloat();

	if (!bForceDraw && vPos.Mag() > fAdjustedRadius)
		return;



	//render normal

	if (m_bOverflow)
		m_Text.RenderClipped(m_Rect);
	else
		m_Text.Render();


}

void CHUDSubtitles::Update()
{
	// Sanity checks...
	if (!m_bVisible) 
		return;

	if (GetConsoleInt("Subtitles",0) == 0) 
		return;


	//update the amount of time that has elapsed
	m_fElapsedTime += RealTimeTimer::Instance( ).GetTimerElapsedS( );

	if (m_fEndTime < m_fElapsedTime)
	{
		Clear();
		return;
	}

	//scroll it
	if (!m_bOverflow) 
		return;

	if (m_fScrollStartTime < m_fElapsedTime)
	{
		float fElapsedTime = m_fElapsedTime - m_fScrollStartTime;
		m_fOffset = (fElapsedTime * m_fScrollSpeed);
		if (m_fOffset > m_fMaxOffset) m_fOffset = m_fMaxOffset;
		m_Text.SetPos(LTVector2((float)m_Rect.Left(),(float)m_Rect.Top() - m_fOffset)); 

	}


}

bool CHUDSubtitles::Show(const char* szStringId, LTVector vSpeakerPos, float fRadius, float fDuration, bool bSubtitlePriority)
{

	// Only show subtitles if conversations in range...
	LTVector vListenerPos;
	bool bListenerInClient;
	LTRotation rRot;
	g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

	LTVector vPos = vSpeakerPos - vListenerPos;
	float fAdjustedRadius = fRadius * g_vtAdjustedRadius.GetFloat();
	float fDist = vPos.Mag();
	if (vSpeakerPos == LTVector(0, 0, 0))
		fDist = 0.0f;

	//should we override what ever is already playing?
	if (m_bVisible)
	{

		//if the old one has priority, and the new doesn't, don't play the new
		if (m_bSubtitlePriority && !bSubtitlePriority)
		{
			return false;
		}

		//if they have the same priority, check distances
		if (m_bSubtitlePriority == bSubtitlePriority)
		{
			LTVector vOldPos = m_vSpeakerPos - vListenerPos;
			float fOldDist = vOldPos.Mag();
			if (m_vSpeakerPos == LTVector(0, 0, 0))
				fOldDist = 0.0f;

			
			if (fOldDist < fDist)
			{
				return false;
			}
		}

	}
	

	const wchar_t *pStr = LoadString(szStringId);
	
	
	if (LTStrEmpty(pStr))
	{
		DebugCPrint(2,"CHUDSubtitles::Show(%s) : No Text",szStringId);
		return false;
	}
		
	m_Text.SetText(pStr);

	m_bVisible = true;
	m_bSubtitlePriority = bSubtitlePriority;

	m_vSpeakerPos = vSpeakerPos;
	m_fRadius = fRadius;
	m_fDuration = fDuration;

	if (m_fDuration < 0.0f)
		m_fDuration = 0.04f * (float)LTStrLen(m_Text.GetText());


	LTVector2n pos = m_vBasePos;
	uint32	width = m_nWidth;

/*
	if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic )
	{
		pos = m_CinematicPos;
		width = m_nCinematicWidth;
	}
*/

	LTVector2 vfScale = g_pInterfaceResMgr->GetScreenScale();
	uint32 x = (uint32)((float)pos.x * vfScale.x);
	uint32 y = (uint32)((float)pos.y * vfScale.y);
	width = (uint32 )((float)width * vfScale.x);
	uint32 height = (2 + m_nMaxLines * m_sTextFont.m_nHeight) ;

	float fFontHeight =  (float)m_sTextFont.m_nHeight;

	m_Rect.Init(x,y,x+width,y+height);

	m_Text.WordWrap(m_Rect.GetWidth());
	float textX = (float)m_Rect.Left();
	float textY = (float)m_Rect.Top();

	LTRect2n rExt;
	m_Text.CreateTexture();
	m_Text.GetExtents(rExt);

	uint32 numLines = (uint32)(rExt.GetHeight() / fFontHeight);

	if (numLines > m_nMaxLines)
	{
		m_bOverflow = true;
		float fTimePerLine = m_fDuration / ((float)numLines + 1.0f);
		float fDelay = (float)m_nMaxLines * fTimePerLine;
		m_fScrollStartTime = fDelay;

		m_fScrollSpeed = fFontHeight / fTimePerLine;
		
		m_fMaxOffset = (float)(rExt.GetHeight() - m_Rect.GetHeight());
	}
	else
	{
		m_bOverflow = false;
		textX += (float)(m_Rect.GetWidth() - width) / 2.0f;
		m_fOffset = (float)(m_Rect.GetHeight() - height);
		
		textY += m_fOffset;
		m_fScrollSpeed = 0.0f;

	}

	m_fEndTime = m_fDuration;

	//reset our time to the beginning
	m_fElapsedTime = 0.0f;


	m_Text.SetPos(LTVector2(textX,textY));

	return true;

}

void CHUDSubtitles::Clear()
{
	m_bVisible = false;
	m_Text.SetText(NULL);

	//reset our elapsed time as well, just to be safe
	m_fElapsedTime = 0.0f;

	m_bSubtitlePriority = false;

}



void CHUDSubtitles::ScaleChanged()
{
	LTVector2 vfScale = g_pInterfaceResMgr->GetScreenScale();

	float x = (float)m_vBasePos.x * vfScale.x;
	float y = (float)m_vBasePos.y * vfScale.y;

/*
	if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic )
	{
		pos = m_CinematicPos;
		width = m_nCinematicWidth;

		x = (float)m_CinematicPos.x * vfScale.x;
		y = (float)m_CinematicPos.y * vfScale.y;

	}
*/
	float fw = float(m_nWidth) * vfScale.x;
	float fh = float(2 + m_nMaxLines * m_sTextFont.m_nHeight);

	m_Rect.Init((uint32)x,(uint32)y,(uint32)(x+fw),(uint32)(y+fh));

		
	if (!m_bOverflow)
	{

		LTRect2n rExt;
		m_Text.GetExtents(rExt);

		x += (float)(m_Rect.GetWidth() - rExt.GetWidth()) / 2.0f;
		m_fOffset = (float)(m_Rect.GetHeight() - rExt.GetHeight());
		y += m_fOffset;
	}

	m_Text.SetPos(LTVector2(x,y));


}

void CHUDSubtitles::UpdateLayout()
{

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDSubtitle");
	}

	CHUDItem::UpdateLayout();

	m_nWidth = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_nMaxLines = ( uint8 )g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);

	m_Text.SetDropShadow(1);

}



