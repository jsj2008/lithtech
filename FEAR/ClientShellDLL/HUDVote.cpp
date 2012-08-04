// ----------------------------------------------------------------------- //
//
// MODULE  : HUDVote.cpp
//
// PURPOSE : HUD element to display the status of a vote
//
// CREATED : 12/02/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDVote.h"
#include "ClientVoteMgr.h"
#include "HUDDialogue.h"

CHUDVote::CHUDVote()
{
	m_UpdateFlags = kHUDVote;
	m_eLevel = kHUDRenderText;
	m_hTimerTexture = NULL;
}

bool CHUDVote::Init()
{
	CHUDItem::Init();

	UpdateLayout();
	return true;
}

void CHUDVote::Term()
{
	if (m_hTimerTexture)
	{
		g_pTextureString->ReleaseTextureString(m_hTimerTexture);
		m_hTimerTexture = NULL;
	}
}


void CHUDVote::Render()
{
	if (!m_bVisible)
	{
		return;
	}

	SetRenderState();

	// Draw the string to the surface...
	double fTimeLeft = ClientVoteMgr::Instance().GetVoteTimeLeft();

	uint8 nMinutes = uint8(fTimeLeft) / 60;
	uint8 nSeconds = fTimeLeft > 60.0 ? uint8(fTimeLeft) % 60 : uint8(fTimeLeft);

	wchar_t wBuffer[16] = L"";

	if (nMinutes != m_nLastMinutes || nSeconds != m_nLastSeconds)
	{
		m_nLastMinutes = nMinutes;
		m_nLastSeconds = nSeconds;

		FormatString("HUD_Timer_Format",wBuffer,LTARRAYSIZE(wBuffer),nMinutes, nSeconds);
		m_Timer.SetText(wBuffer);
	}


	m_Text.Render();
	m_Count.Render();

	if (!ClientVoteMgr::Instance().HasVoted())
	{
		m_Hotkeys.Render();
	}
	
	m_Timer.Render();

}

void CHUDVote::Update()
{
	bool bWasVisible = m_bVisible;
	m_bVisible = ClientVoteMgr::Instance().IsVoteInProgress();
	if (!m_bVisible)
	{
		return;
	}
	if (ClientVoteMgr::Instance().HasVoted())
	{
		m_cTextColor = m_cVotedTextColor;
	}
	else
	{
		m_cTextColor = m_cNormalTextColor;
	}
	m_Text.SetColor(m_cTextColor);
	m_Timer.SetColor(m_cTextColor);
	m_Count.SetColor(m_cTextColor);

	if (!bWasVisible)
	{
		g_pHUDDialogue->HideAll();
		Flash("NewVote");
	}

	m_Text.SetText(ClientVoteMgr::Instance().GetVoteString());

	wchar_t wszTxt[256] = L"";
	FormatString("Vote_CurrentCount",wszTxt,LTARRAYSIZE(wszTxt),ClientVoteMgr::Instance().GetVoteData().m_nYesVotes,ClientVoteMgr::Instance().GetVoteData().m_nVotesNeeded,ClientVoteMgr::Instance().GetVoteData().m_nNoVotes);
	m_Count.SetText(wszTxt);



	UpdateTextPos();

}
void CHUDVote::UpdateTextPos()
{
	LTVector2 vPos = m_Text.GetPos();

	vPos.y += float(m_sTextFont.m_nHeight);
	m_Count.SetPos(vPos);

	vPos.y += float(m_sTextFont.m_nHeight);
	m_Hotkeys.SetPos(vPos);

	m_Text.CreateTexture();

	LTRect2n rExt;
	m_Text.GetExtents(rExt);
	vPos = m_Text.GetPos();

	vPos.x += float(rExt.Right()) + 12.0f;
	m_Timer.SetPos(vPos);



}

void CHUDVote::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDVote");
	}

	CHUDItem::UpdateLayout();

	m_cNormalTextColor = m_cTextColor;
	m_cVotedTextColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);

	m_Count.SetFont(m_sTextFont);
	m_Count.SetColor(m_cTextColor);
	m_Count.SetAlignment(m_eTextAlignment);

	m_Timer.SetFont(m_sTextFont);
	m_Timer.SetColor(m_cTextColor);
	m_Timer.SetAlignment(m_eTextAlignment);

	static std::wstring srcStr = LoadString("HUD_Timer_Chars");
	if (m_hTimerTexture)
	{
		LTRESULT res = g_pTextureString->RecreateTextureString(m_hTimerTexture,srcStr.c_str(),m_sTextFont);
		if (res != LT_OK)
		{
			g_pTextureString->ReleaseTextureString(m_hTimerTexture);
			m_hTimerTexture = NULL;
		}
	}
	else
	{
		m_hTimerTexture = g_pTextureString->CreateTextureString(srcStr.c_str(),m_sTextFont);
	}

	m_Timer.SetSourceString( m_hTimerTexture);


	m_Hotkeys.SetFont(m_sTextFont);
	m_Hotkeys.SetColor(m_cTextColor);
	m_Hotkeys.SetAlignment(m_eTextAlignment);
	m_Hotkeys.SetText( LoadString("Vote_Keys"));

	UpdateTextPos();



}

void CHUDVote::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	UpdateTextPos();
}


void CHUDVote::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();

	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		m_Count.SetColor(m_cFlashColor);
		m_Timer.SetColor(m_cFlashColor);
		m_Hotkeys.SetColor(m_cFlashColor);
	}
	else
	{
		float fFade = GetFadeLevel();
		m_Count.SetColor(FadeARGB(m_cTextColor,fFade));
		m_Timer.SetColor(FadeARGB(m_cTextColor,fFade));
		m_Hotkeys.SetColor(FadeARGB(m_cTextColor,fFade));
	}
}
