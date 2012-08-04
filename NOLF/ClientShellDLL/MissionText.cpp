// MissionText.cpp: implementation of the CMissionText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MissionText.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "SoundMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMissionText::CMissionText()
{
    m_hForeSurf         = LTNULL;
    m_hText             = LTNULL;
	m_fDelay			= 0.0f;
	m_fFadeTime			= 0.0f;
	m_fTimeRemaining	= 0.0f;
    m_hSound            = LTNULL;
	m_fAlpha			= 1.0f;
	m_bPause			= LTFALSE;
}



CMissionText::~CMissionText()
{
	Clear();
	if (m_hForeSurf)
	{
        g_pLTClient->DeleteSurface(m_hForeSurf);
        m_hForeSurf = LTNULL;
	}

}


void	CMissionText::Init()
{
	m_lfDrawData.dwFlags = LTF_DRAW_TIMED | LTF_DRAW_FORMATTED | LTF_TIMED_ALL | LTF_EXTRA_LOCKLAST;
	m_lfDrawData.byJustify = LTF_JUSTIFY_LEFT;
    m_lfDrawData.dwFormatWidth      = (uint32)g_pLayoutMgr->GetMissionTextWidth();
	m_lfDrawData.fLetterDelay		= g_pLayoutMgr->GetMissionTextLetterDelay();
	m_lfDrawData.fLineDelay			= g_pLayoutMgr->GetMissionTextLineDelay();
	m_lfDrawData.fLineScrollTime	= g_pLayoutMgr->GetMissionTextLineScrollTime();
	m_lfDrawData.hColor				= kWhite;

	m_fFadeTime			= g_pLayoutMgr->GetMissionTextFadeTime();
	m_fDelay			= m_fFadeTime + g_pLayoutMgr->GetMissionTextFadeDelay();

	m_pos				= g_pLayoutMgr->GetMissionTextPos();

	m_nLineHeight		= g_pInterfaceResMgr->GetMsgForeFont()->GetHeight();
	int numLines		= g_pLayoutMgr->GetMissionTextNumLines();

	m_dwWidth = m_lfDrawData.dwFormatWidth + 2;
    m_dwHeight = (uint32) (numLines * m_nLineHeight) + 16;

	m_nCursorPos = (int)m_dwHeight - m_nLineHeight;

    m_hForeSurf = g_pLTClient->CreateSurface(m_dwWidth,m_dwHeight);

	ClearSurface();
    g_pLTClient->OptimizeSurface(m_hForeSurf,LTNULL);

	g_pLayoutMgr->GetMissionTextTypeSound(m_TypeSound,sizeof(m_TypeSound));
	g_pLayoutMgr->GetMissionTextScrollSound(m_ScrollSound,sizeof(m_ScrollSound));

	
}

void	CMissionText::Start(int nStringId)
{
     HSTRING hText = g_pLTClient->FormatString(nStringId);
	 Start(hText);
     g_pLTClient->FreeString(hText);
}

void	CMissionText::Start(char *pszString)
{
     HSTRING hText = g_pLTClient->CreateString(pszString);
	 Start(hText);
     g_pLTClient->FreeString(hText);
}

// this makes a copy of hString, so be sure to free it
void	CMissionText::Start(HSTRING hString)
{

	if (m_hText)
	{
        g_pLTClient->FreeString(m_hText);
        m_hText = LTNULL;
	}

    m_lfSaveData.szPrevString = LTNULL;

    m_hText = g_pLTClient->CopyString(hString);

	g_pInterfaceResMgr->GetMsgForeFont()->Draw(m_hText, m_hForeSurf, &m_lfDrawData, 1, m_nCursorPos, &m_lfSaveData);
    g_pLTClient->OptimizeSurface(m_hForeSurf,LTNULL);

	m_fTimeRemaining = m_fDelay;

    m_bScrolling = LTFALSE;
	m_bPause = LTFALSE;
}

void	CMissionText::Clear()
{
	if (m_hText)
	{
        g_pLTClient->FreeString(m_hText);
        m_hText = LTNULL;
	}
	m_fTimeRemaining	= 0.0f;
	if(m_hSound)
	{
        g_pLTClient->KillSound(m_hSound);
        m_hSound = LTNULL;
	}
	m_bPause = LTFALSE;

}

void	CMissionText::Draw()
{
	if (!m_hText) return;

	int x = (int) ((float)m_pos.x * g_pInterfaceResMgr->GetXRatio());
	int y = (int) ((float)m_pos.y * g_pInterfaceResMgr->GetYRatio());

	uint8 nFade = (uint8)(255.0f * m_fAlpha);
	HLTCOLOR hColor = SETRGB(nFade,nFade,nFade);


	g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
	g_pLTClient->SetOptimized2DColor(hColor);
	g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, NULL, x+1, y+1);
	g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
	g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, NULL, x, y);
	g_pLTClient->SetOptimized2DColor(kWhite);
	g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);

    
}

void	CMissionText::Update()
{
	if (m_bPause) return;
	if (!m_hText)
	{
		if(m_hSound)
		{
            g_pLTClient->KillSound(m_hSound);
            m_hSound = LTNULL;
		}
		return;
	}

	ClearSurface();
	g_pInterfaceResMgr->GetMsgForeFont()->Draw(m_hText, m_hForeSurf, &m_lfDrawData, 1, m_nCursorPos, &m_lfSaveData);
    g_pLTClient->OptimizeSurface(m_hForeSurf,LTNULL);

	m_fAlpha = 1.0f;

	switch (m_lfSaveData.byLastState)
	{
	case LTF_STATE_DRAW_UPDATING:
		{
			if(!m_hSound)
			{
				m_hSound = 	g_pClientSoundMgr->PlayInterfaceSound(m_TypeSound, (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT));
			}
            m_bScrolling = LTFALSE;
		} break;

	case LTF_STATE_DRAW_LINE_DELAY:
		if(m_hSound)
		{
            g_pLTClient->KillSound(m_hSound);
            m_hSound = LTNULL;
		}
		break;
	case LTF_STATE_DRAW_SCROLLING:
		if(m_hSound)
		{
            g_pLTClient->KillSound(m_hSound);
            m_hSound = LTNULL;
		}
		if (!m_bScrolling)
		{
			g_pClientSoundMgr->PlayInterfaceSound(m_ScrollSound);
            m_bScrolling = LTTRUE;
		}
		break;

	case LTF_STATE_DRAW_FINISHED:
		{
            m_bScrolling = LTFALSE;
			if(m_hSound)
			{
                g_pLTClient->KillSound(m_hSound);
                m_hSound = LTNULL;
			}
            m_fTimeRemaining -= g_pGameClientShell->GetFrameTime();
			if (m_fTimeRemaining <= 0.0f)
			{
                g_pLTClient->FreeString(m_hText);
                m_hText = LTNULL;
			}
			else if (m_fTimeRemaining <= m_fFadeTime)
			{
				m_fAlpha = (m_fTimeRemaining / m_fFadeTime);
			}
		} break;
	}


}

void	CMissionText::ClearSurface()
{
    LTRect rcSrc(0,0, m_dwWidth, m_dwHeight);

//  g_pLTClient->FillRect(m_hSurf, &rcSrc, SETRGB(255,255,0));
//	rcSrc.left++;
//	rcSrc.top++;
//	rcSrc.right--;
//	rcSrc.bottom--;

    g_pLTClient->FillRect(m_hForeSurf, &rcSrc, LTNULL);
}


void	CMissionText::Pause(LTBOOL bPause)
{
	if (bPause)
	{
		if(m_hSound)
		{
			g_pLTClient->KillSound(m_hSound);
			m_hSound = LTNULL;
		}
	}
	m_bPause = bPause;
}