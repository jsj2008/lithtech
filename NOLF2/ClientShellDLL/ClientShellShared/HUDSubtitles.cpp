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

VarTrack g_vtAdjustedRadius;

void TestSubtitleFn(int argc, char **argv)
{

	if (argc <= 0) return;

	int nID = atoi(argv[0]);

	LTVector vPos;

	vPos.Init();

	g_pSubtitles->Show(nID,vPos);
	
}


CHUDSubtitles::CHUDSubtitles()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
	m_bVisible = LTFALSE;
	m_fScale = 1.0f;
	m_fElapsedTime = 0.0f;

	m_vSpeakerPos.Init();
	m_fRadius = 0.0;
	m_fDuration = -1.0f;
	m_bSubtitlePriority = false;
}
	

LTBOOL CHUDSubtitles::Init()
{
    g_pLTClient->RegisterConsoleProgram("TestSubtitle", TestSubtitleFn);


	UpdateLayout();
	if (!m_pText)
		return LTFALSE;

	return LTTRUE;

}
void CHUDSubtitles::Term()
{
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}

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
	bForceDraw = g_pPlayerMgr->IsUsingExternalCamera() ? LTTRUE : bForceDraw;

	LTVector vPos = m_vSpeakerPos - vListenerPos;
	float fAdjustedRadius = m_fRadius * g_vtAdjustedRadius.GetFloat();

	if (!bForceDraw && vPos.Mag() > fAdjustedRadius)
		return;

	//render dropshadow
	float x,y;
	m_pText->GetPosition(&x,&y);
	m_pText->SetPosition(x+2.0f,y+2.0f);
	m_pText->SetColor(argbBlack);

	if (m_bOverflow)
		m_pText->RenderClipped(&m_DisplayRect);
	else
		m_pText->Render();

	//render normal
	m_pText->SetPosition(x,y);
	m_pText->SetColor(m_nTextColor);

	if (m_bOverflow)
		m_pText->RenderClipped(&m_DisplayRect);
	else
		m_pText->Render();


}

void CHUDSubtitles::Update()
{
	// Sanity checks...
	if (!m_bVisible) 
		return;

	if (GetConsoleInt("Subtitles",0) == 0) 
		return;


	//update the amount of time that has elapsed
	m_fElapsedTime += g_pLTClient->GetFrameTime();

	if (m_fEndTime < m_fElapsedTime)
	{
		Clear();
		return;
	}

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());


	//scroll it
	if (!m_bOverflow) 
		return;

	if (m_fScrollStartTime < m_fElapsedTime)
	{
		LTFLOAT fElapsedTime = m_fElapsedTime - m_fScrollStartTime;
		m_fOffset = (fElapsedTime * m_fScrollSpeed);
		if (m_fOffset > m_fMaxOffset) m_fOffset = m_fMaxOffset;

		
		float textY = m_DisplayRect.y - m_fOffset;
		m_pText->SetPosition(m_DisplayRect.x,textY);

	}


}

LTBOOL CHUDSubtitles::Show(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius, LTFLOAT fDuration, bool bSubtitlePriority)
{

	if (!g_vtAdjustedRadius.IsInitted())
	{
        g_vtAdjustedRadius.Init(g_pLTClient, "SubtitleSoundRadiusPercent", LTNULL, 0.77f);
	}

	//if it's too far away, don't play it...
	LTVector vListenerPos;
	bool bListenerInClient;
	LTRotation rRot;
	g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

	bool bForceDraw = (bool)(vSpeakerPos == LTVector(0, 0, 0));
	bForceDraw = g_pPlayerMgr->IsUsingExternalCamera() ? LTTRUE : bForceDraw;

	LTVector vPos = vSpeakerPos - vListenerPos;
	float fAdjustedRadius = fRadius * g_vtAdjustedRadius.GetFloat();
	float fDist = vPos.Mag();
	if (!bForceDraw && fDist > fAdjustedRadius)
	{
		return LTFALSE;
	}

	//should we override what ever is already playing?
	if (m_bVisible)
	{

		//if the old one has priority, and the new doesn't, don't play the new
		if (m_bSubtitlePriority && !bSubtitlePriority)
		{
			return LTFALSE;
		}

		//if they have the same priority, check distances
		if (m_bSubtitlePriority == bSubtitlePriority)
		{
			LTVector vOldPos = m_vSpeakerPos - vListenerPos;
			float fOldDist = vOldPos.Mag();

			
			if (fOldDist < fDist)
			{
				return LTFALSE;
			}
		}

	}
	

	ASSERT(m_pText);
	const char *pStr = LoadTempString(nStringId);
	
	
	if (!strlen(pStr))
	{
		g_pLTClient->CPrint("CHUDSubtitles::Show(%d) : No Text",nStringId);
		return LTFALSE;
	}
	
	
	
	
	m_pText->SetText(pStr);

	m_bVisible = LTTRUE;
	m_bSubtitlePriority = bSubtitlePriority;

	m_vSpeakerPos = vSpeakerPos;
	m_fRadius = fRadius;
	m_fDuration = fDuration;

	if (m_fDuration < 0.0f)
		m_fDuration = 0.04f * (float)m_pText->GetLength();


	SetScale(g_pInterfaceResMgr->GetXRatio());

	LTIntPt pos = m_FullScreenPos;
	uint16	width = m_nFullScreenWidth;
	if (g_pPlayerMgr->IsUsingExternalCamera())
	{
		pos = m_CinematicPos;
		width = m_nCinematicWidth;
	}

	m_DisplayRect.x = (float)pos.x * m_fScale;
	m_DisplayRect.y = (float)pos.y * m_fScale;
	m_DisplayRect.width = (float)width * m_fScale;
	m_DisplayRect.height = 2.0f + (float)m_nMaxLines * (float)m_nFontSize;

	m_pText->SetWrapWidth((uint16)m_DisplayRect.width);
	float textX = m_DisplayRect.x;
	float textY = m_DisplayRect.y;

	float fw,fh;
	m_pText->GetDims(&fw,&fh);

	uint8 numLines = (uint8)(fh / (float)m_nFontSize);
//	g_pLTClient->CPrint("Lines: %d",numLines);
//	g_pLTClient->CPrint("Duration: %0.2f",m_fDuration);

	if (numLines > m_nMaxLines)
	{
		m_bOverflow = LTTRUE;
		LTFLOAT fTimePerLine = m_fDuration / ((LTFLOAT)numLines + 1.0f);
		LTFLOAT fRemainingLines = (LTFLOAT)(numLines - m_nMaxLines);
		LTFLOAT fDelay = (LTFLOAT)m_nMaxLines * fTimePerLine;
		m_fScrollStartTime = fDelay;

		m_fScrollSpeed = (LTFLOAT)m_nFontSize / fTimePerLine;
		m_fMaxOffset = (fh + 2.0f) - m_DisplayRect.height;

	}
	else
	{
		m_bOverflow = LTFALSE;

		textX += (m_DisplayRect.width - fw) / 2.0f;
		m_fOffset = (m_DisplayRect.height - fh);
		textY += m_fOffset;
		m_fScrollSpeed = 0.0f;

	}

	m_fEndTime = m_fDuration;

	//reset our time to the beginning
	m_fElapsedTime = 0.0f;


	m_pText->SetPosition(textX,textY);

	return LTTRUE;

}

void CHUDSubtitles::Clear()
{
	m_bVisible = LTFALSE;
	if (m_pText)
		m_pText->SetText(" ");

	//reset our elapsed time as well, just to be safe
	m_fElapsedTime = 0.0f;

	m_bSubtitlePriority = false;

}



void CHUDSubtitles::SetScale(float fScale)
{
	m_fScale = fScale;
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	if (m_pText)
	{
		float x = (float)m_BasePos.x * m_fScale;
		float y = (float)m_BasePos.y * m_fScale;
		m_pText->SetCharScreenHeight(m_nFontSize);
		m_pText->SetPosition(x,y);

		LTIntPt pos = m_FullScreenPos;
		uint16	width = m_nFullScreenWidth;
		if (g_pPlayerMgr->IsUsingExternalCamera())
		{
			pos = m_CinematicPos;
			width = m_nCinematicWidth;
		}


		m_DisplayRect.x = (float)pos.x * m_fScale;
		m_DisplayRect.y = (float)pos.y * m_fScale;
		m_DisplayRect.width = (float)width * m_fScale;
		m_DisplayRect.height = (float)m_nMaxLines * (float)m_nFontSize;

		
		if (!m_bOverflow)
		{
			float textX = m_DisplayRect.x;
			float textY = m_DisplayRect.y;

			float fw,fh;
			m_pText->GetDims(&fw,&fh);

			textX += (m_DisplayRect.width - fw) / 2.0f;
			m_fOffset = (m_DisplayRect.height - fh);
			textY += m_fOffset;

		}

	}

}

void CHUDSubtitles::UpdateLayout()
{

	char *pTag = "Subtitle";

	m_CinematicPos = g_pLayoutMgr->GetPoint(pTag,"Pos");
	m_nCinematicWidth = (uint16)g_pLayoutMgr->GetInt(pTag,"Width");
	m_FullScreenPos = g_pLayoutMgr->GetPoint(pTag,"FullScreenPos");
	m_nFullScreenWidth = (uint16)g_pLayoutMgr->GetInt(pTag,"FullScreenWidth");
	m_nMaxLines  = (uint8)g_pLayoutMgr->GetInt(pTag,"NumLines");


	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	m_pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_nFontSize = m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont," ",(float)m_BasePos.x,(float)m_BasePos.y);
	if (!m_pText)
		return;

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_nTextColor = SET_ARGB(0xFF,nR,nG,nB);

	m_pText->SetColor(m_nTextColor);
	

}



