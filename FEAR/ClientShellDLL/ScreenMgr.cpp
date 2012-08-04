// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMgr.cpp
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMgr.h"
#include "SoundMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
//screens
#include "BaseScreen.h"

#include "ScreenMain.h"

//under main
#include "ScreenSingle.h"
#include "ScreenOptions.h"
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
#include "ScreenMulti.h"
#endif
#include "ScreenProfile.h"

//under single
#include "ScreenLoad.h"
#include "ScreenSave.h"

//under multi
#include "ScreenPlayer.h"
#include "ScreenHost.h"
#include "ScreenPlayerTeam.h"
#include "ScreenPlayerLoadout.h"
#include "ScreenVote.h"
#include "ScreenMutePlayer.h"

//under options
#include "ScreenDisplay.h"
#include "ScreenAudio.h"
#include "ScreenControls.h"
#include "ScreenGame.h"
#include "ScreenWeapons.h"
#include "ScreenPerformance.h"
#include "ScreenPerformanceCPU.h"
#include "ScreenPerformanceGPU.h"

//under multi/host
#include "ScreenHostOptions.h"
#include "ScreenTeam.h"
#include "ScreenHostOptionFile.h"
#include "ScreenHostLevels.h"
#include "ScreenHostWeapons.h"
#include "ScreenHostDownload.h"
#include "ScreenHostVoting.h"

//under options/game
#include "ScreenCrosshair.h"

//under options/controls
#include "ScreenConfigure.h"
#include "ScreenMouse.h"
#include "ScreenJoystick.h"

#include "ScreenFailure.h"
#include "ScreenPreload.h"
#include "ScreenPostload.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


static char s_aScreenName[SCREEN_ID_UNASSIGNED+1][32] =
{
#define INCLUDE_AS_STRING
#include "ScreenEnum.h"
#undef INCLUDE_AS_STRING
};


void ReportScreenRecord(int argc, char **argv)
{
	if( !g_pInterfaceMgr || !g_pInterfaceMgr->GetScreenMgr() || g_pInterfaceMgr->GetGameState() != GS_SCREEN )
		return;

	char const* pszScreenName = g_pInterfaceMgr->GetScreenMgr()->GetScreenName(g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID());
	g_pLTBase->CPrint( "CurrentScreen: %s", pszScreenName );
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMgr::CScreenMgr()
{
    m_pCurrentScreen = NULL;
	m_eCurrentScreenID = SCREEN_ID_NONE;
	m_eLastScreenID = SCREEN_ID_NONE;
	m_nHistoryLen = 0;

	m_hMovie = NULL;
	m_hMusic = NULL;
}

CScreenMgr::~CScreenMgr()
{

}

//////////////////////////////////////////////////////////////////////
// Function name	: CScreenMgr::Init
// Description	    :
// Return type      : bool
//////////////////////////////////////////////////////////////////////

bool CScreenMgr::Init()
{
	m_screenArray.reserve(SCREEN_ID_UNASSIGNED);

	g_pLTClient->RegisterConsoleProgram("ReportScreenRecord", ReportScreenRecord);

	m_hMovie = NULL;
	m_hMusic = NULL;

	for (int nID = SCREEN_ID_MAIN; nID < SCREEN_ID_UNASSIGNED; ++nID)
	{
		AddScreen((eScreenID)nID);
	}

	m_HelpRect		= g_pLayoutDB->GetHelpRect();
	m_HelpSize		= g_pLayoutDB->GetHelpSize();
	m_HelpWidth		= (uint16)(m_HelpRect.GetWidth());
	m_BackRect = g_pLayoutDB->GetScreenBackRect();

	CLTGUICtrl_create cs;
	cs.rnBaseRect = m_BackRect;
	cs.nCommandID = CMD_BACK;
	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();
	uint32 nFontSize = g_pLayoutDB->GetScreenBackSize();
	m_Back.Create(LoadString("IDS_BACK"), CFontInfo(g_pLayoutDB->GetScreenBackFont(),nFontSize),cs);
	m_Back.SetScale(g_pInterfaceResMgr->GetScreenScale());

	// update the destination rect for the movie
	UpdateMovieDims();

//	m_pTransitionFXMgr = debug_new(CTransitionFXMgr);
//	m_pTransitionFXMgr->Init();
    return true;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CScreenMgr::Term
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CScreenMgr::Term()
{
	m_Back.Destroy();
	m_HelpStr.FlushTexture();

	// Term the screens
	for (uint16 i=0; i < m_screenArray.size(); ++i)
	{
		m_screenArray[i]->Term();
		debug_delete(m_screenArray[i]);
	}
	m_screenArray.clear();

	RemoveMedia();
}



// Renders the screen to a surface
bool CScreenMgr::Render()
{
	if (m_hMovie) 
	{
		//get the dimensions of the video
		LTVector2n vnVideoDims(0, 0);
		LTVector2n vnTextureDims(0, 0);

		ILTVideoTexture* pVideoTex = g_pLTClient->GetVideoTexture();
		if(pVideoTex->GetVideoTextureDims(m_hMovie, vnVideoDims, vnTextureDims) == LT_OK)
		{
			//determine the UV extents
			float fUOffset = 0.5f / (float)vnTextureDims.x;
			float fVOffset = 0.5f / (float)vnTextureDims.y;
			float fUWidth  = (float)vnVideoDims.x / (float)vnTextureDims.x;
			float fVHeight = (float)vnVideoDims.y / (float)vnTextureDims.y;

			//draw the video to the screen
			LT_POLYGT4 Quad;

			if( LTNearlyEquals(m_rfMovieDest.GetWidth(), 0.0f, 0.1f) || 
				LTNearlyEquals(m_rfMovieDest.GetHeight(), 0.0f, 0.1f) )
			{
				//render our movie to the screen
				uint32 nScreenWidth = 0;
				uint32 nScreenHeight = 0;
				g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nScreenWidth, nScreenHeight);

				//we can basically fit either the width or height to the screen width or height and maintain
				//aspect ratio, so we will simply try both
				uint32 nWidth = nScreenWidth;
				uint32 nHeight = nWidth * vnVideoDims.y / vnVideoDims.x;

				if(nHeight > nScreenHeight)
				{
					nHeight = nScreenHeight;
					nWidth  = nHeight * vnVideoDims.x / vnVideoDims.y;
				}

				DrawPrimSetXYWH(Quad, (nScreenWidth - nWidth) * 0.5f, (nScreenHeight - nHeight) * 0.5f, (float)nWidth, (float)nHeight);
			}
			else
			{
				DrawPrimSetXYWH(Quad, m_rfMovieDest.Left(), m_rfMovieDest.Top(), m_rfMovieDest.GetWidth(), m_rfMovieDest.GetHeight() );
			}

			DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
			DrawPrimSetUVWH(Quad, fUOffset, fVOffset, fUWidth - fUOffset, fVHeight - fVOffset);

			g_pDrawPrim->SetVideoTexture(m_hMovie);
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

			g_pDrawPrim->DrawPrim(&Quad, 1);
		}
	}

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	if (m_hStaticBackground != (HTEXTURE)NULL)
	{
		LTPoly_GT4 back;
		DrawPrimSetRGBA(back,argbWhite);
		LTVector2 vHalfTexel(0.5f / (float)g_pInterfaceResMgr->GetScreenWidth(), 0.5f / (float)g_pInterfaceResMgr->GetScreenHeight());
		SetupQuadUVs(back, m_hStaticBackground, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f );
		DrawPrimSetXYWH(back,-0.5f,-0.5f,float(g_pInterfaceResMgr->GetScreenWidth()),float(g_pInterfaceResMgr->GetScreenHeight()));

		g_pDrawPrim->SetTexture(m_hStaticBackground);
		g_pDrawPrim->DrawPrim(&back, 1);
	}

	if (m_pCurrentScreen)
	{
		return m_pCurrentScreen->Render();
	}

    return false;
}

bool CScreenMgr::UpdateInterfaceSFX()
{
	if (m_pCurrentScreen)
	{
		return m_pCurrentScreen->UpdateInterfaceSFX();
	}

	return true;
}

void CScreenMgr::HandleKeyDown (int vkey, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	if (m_pCurrentScreen)
	{
		if (vkey == VK_ESCAPE)
		{
			m_pCurrentScreen->Escape();
		}
		else
		{
			m_pCurrentScreen->HandleKeyDown(vkey,rep);
		}
	}
#endif // PLATFORM_XENON
}

void CScreenMgr::HandleKeyUp (int vkey)
{
	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->HandleKeyUp(vkey);
	}
}

void CScreenMgr::HandleChar (wchar_t c)
{
	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->HandleChar(c);
	}
}


eScreenID CScreenMgr::GetFromHistory( int nHistoryIndex )
{ 
	if( nHistoryIndex < MAX_SCREEN_HISTORY )
		return m_eScreenHistory[m_nHistoryLen - nHistoryIndex - 1];

	return SCREEN_ID_NONE;
}

bool CScreenMgr::PreviousScreen()
{
	if (m_nHistoryLen < 1) return false;

	CBaseScreen *pNewScreen=GetScreenFromID(m_eScreenHistory[m_nHistoryLen-1]);
	if (pNewScreen)
	{
		SwitchToScreen(pNewScreen);

        return true;
	}
    return false;
}

bool CScreenMgr::SetCurrentScreen(eScreenID screenID)
{

	CBaseScreen *pNewScreen=GetScreenFromID(screenID);
	if (pNewScreen)
	{
		SwitchToScreen(pNewScreen);

        return true;
	}
    return false;
}

void CScreenMgr::EscapeCurrentScreen()
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->Escape();
}

void CScreenMgr::ExitScreens()
{
	// Tell the old screen that it is losing focus
	if (m_pCurrentScreen)
	{
        m_pCurrentScreen->OnFocus(false);
	}

	//clear our screen history (no longer relevant)
	ClearHistory();

	RemoveMedia();
}

void CScreenMgr::RemoveMedia()
{
	if (m_hMovie)
	{
		g_pLTClient->GetVideoTexture()->ReleaseVideoTexture(m_hMovie);
		m_hMovie = NULL;
	}

	if (m_hMusic)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hMusic);
		m_hMusic = NULL;
	}

	if (m_hStaticBackground)
	{
		m_hStaticBackground.Free();
	}
}

void CScreenMgr::EnterScreens(bool bCreateMedia)
{
	if (!bCreateMedia)
	{
		RemoveMedia();
		return;
	}


	// KLS 1/28/05 - Cleaned up how this works as EnterScreens() and ExitScreens() are often called
	// a couple times in a row (e.g., loading a level)...

	// Since the movie, static background, and music are always being retrieved from the same
	// database record, there is no need to free them before loading them if they have already
	// been loaded...

	const char* pszMovie = g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ScreenMovie");
	if (pszMovie) // && (!g_pPlayerMgr->IsPlayerInWorld() || g_pInterfaceMgr->GetIntentionalDisconnect()))
	{
		if (NULL == m_hMovie)
		{
			ILTVideoTexture* pVideoTex = g_pLTClient->GetVideoTexture();
			if (pVideoTex)
			{
				m_hMovie = pVideoTex->CreateVideoTexture(pszMovie, 0);//, eVTO_PlaySound | eVTO_AllowStreaming);
				pVideoTex->SetVideoTextureTimer(m_hMovie, RealTimeTimer::Instance().GetHandle() );
			}
		}
	}
	else // No movie, so load a static background...
	{
		if ((HTEXTURE)NULL == m_hStaticBackground)
		{
			m_hStaticBackground.Load( g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ScreenStaticBackground") );
		}
	}

	if (NULL == m_hMusic)
	{
		const char* pszMusic = g_pLayoutDB->GetString( g_pLayoutDB->GetSharedRecord(), "ScreenMusic");
		if (pszMusic)
		{
			m_hMusic = g_pClientSoundMgr->PlaySoundLocal(pszMusic,SOUNDPRIORITY_MISC_LOW, (PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_FE_PERSISTENT_SOUND), SMGR_DEFAULT_VOLUME, 1.0f, DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_MUSIC);
		}
	}
}

void CScreenMgr::SwitchToScreen(CBaseScreen *pNewScreen)
{

	CBaseScreen *pOldScreen = m_pCurrentScreen;
	// Tell the old screen that it is losing focus
	if (m_pCurrentScreen && pNewScreen != m_pCurrentScreen)
	{
        m_pCurrentScreen->OnFocus(false);
		int insert = 0;

		eScreenID nextScreenID = (eScreenID)pNewScreen->GetScreenID();

		//if we hit the main screen, reset the history
		if (nextScreenID == SCREEN_ID_MAIN)
		{
			m_nHistoryLen = 0;
			m_eScreenHistory[0] = SCREEN_ID_NONE;
		}
		else
		{
			//look through the list of screens we've visited, if we find the
			// one we're going to cut the history back to that point.
			// otherwise add it at the end.
			
			eScreenID currentScreenID = (eScreenID)m_pCurrentScreen->GetScreenID();
			while (insert < m_nHistoryLen && m_eScreenHistory[insert] != nextScreenID)
				++insert;
			if (insert == m_nHistoryLen)
			{
				if (m_nHistoryLen < MAX_SCREEN_HISTORY)
				{
					++m_nHistoryLen;
					m_eScreenHistory[insert] = currentScreenID;
				}
			}
			else
			{
				m_nHistoryLen = insert;
			}

		}

	}

	m_pCurrentScreen=pNewScreen;
	m_eLastScreenID = m_eCurrentScreenID;

	m_eCurrentScreenID = (eScreenID)m_pCurrentScreen->GetScreenID();

	// If the new screen hasn't been built yet... better build it!
	if (!m_pCurrentScreen->IsBuilt())
	{

		m_pCurrentScreen->Build();
	}

	// Tell the new screen that it is gaining focus
	if (pNewScreen && pNewScreen != pOldScreen)
	{
        pNewScreen->OnFocus(true);
	}
}


// Returns a screen based on an ID
CBaseScreen *CScreenMgr::GetScreenFromID(eScreenID screenID)
{
	CBaseScreen *pScreen=NULL;

	ScreenArray::iterator iter = m_screenArray.begin();
	while (iter != m_screenArray.end() && (*iter)->GetScreenID() != (int)screenID)
		iter++;

	if (iter != m_screenArray.end())
		pScreen = (*iter);

	return pScreen;

}


// Mouse messages
void	CScreenMgr::OnLButtonDown(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonDown(x,y);
}

void	CScreenMgr::OnLButtonUp(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonUp(x,y);
}

void	CScreenMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnLButtonDblClick(x,y);
}

void	CScreenMgr::OnRButtonDown(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonDown(x,y);
}

void	CScreenMgr::OnRButtonUp(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonUp(x,y);
}

void	CScreenMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnRButtonDblClick(x,y);
}

void	CScreenMgr::OnMouseMove(int x, int y)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnMouseMove(x,y);
}

void	CScreenMgr::OnMouseWheel(int x, int y, int zDelta)
{
	if (m_pCurrentScreen)
		m_pCurrentScreen->OnMouseWheel(x,y,zDelta);
}


void CScreenMgr::AddScreen(eScreenID screenID)
{
	CBaseScreen* pScreen = NULL;

	switch (screenID)
	{
	case SCREEN_ID_MAIN:
		pScreen = debug_new(CScreenMain);
		break;
	case SCREEN_ID_SINGLE:
		pScreen = debug_new(CScreenSingle);
		break;
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_MULTI:
		pScreen = debug_new(CScreenMulti);
		break;
#endif // PLATFORM_XENON
	case SCREEN_ID_OPTIONS:
		pScreen = debug_new(CScreenOptions);
		break;
	case SCREEN_ID_DISPLAY	:
		pScreen = debug_new(CScreenDisplay);
		break;
	case SCREEN_ID_AUDIO:
		pScreen = debug_new(CScreenAudio);
		break;
	case SCREEN_ID_CONTROLS:
		pScreen = debug_new(CScreenControls);
		break;
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_CONFIGURE:
		pScreen = debug_new(CScreenConfigure);
		break;
#endif // PLATFORM_XENON
	case SCREEN_ID_MOUSE:
		pScreen = debug_new(CScreenMouse);
		break;
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_HOST:
		pScreen = debug_new(CScreenHost);
		break;
#endif // PLATFORM_XENON
	case SCREEN_ID_LOAD:
		pScreen = debug_new(CScreenLoad);
		break;
	case SCREEN_ID_PRELOAD:
		pScreen = debug_new(CScreenPreload);
		break;
	case SCREEN_ID_PROFILE:
		pScreen = debug_new(CScreenProfile);
		break;
	case SCREEN_ID_SAVE:
		pScreen = debug_new(CScreenSave);
		break;
	case SCREEN_ID_POSTLOAD:
		pScreen = debug_new(CScreenPostload);
		break;
	case SCREEN_ID_JOYSTICK:
		pScreen = debug_new(CScreenJoystick);
		break;
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_HOST_OPTIONS:
		pScreen = debug_new(CScreenHostOptions);
		break;
	case SCREEN_ID_VOTE:
		pScreen = debug_new(CScreenVote);
		break;
	case SCREEN_ID_MUTE:
		pScreen = debug_new(CScreenMutePlayer);
		break;
#endif // PLATFORM_XENON
	case SCREEN_ID_PLAYER:
		pScreen = debug_new(CScreenPlayer);
		break;
	case SCREEN_ID_TEAM:
		pScreen = debug_new(CScreenTeam);
		break;
	case SCREEN_ID_PLAYER_TEAM:
		pScreen = debug_new(CScreenPlayerTeam);
		break;
	case SCREEN_ID_PLAYER_LOADOUT:
		pScreen = debug_new(CScreenPlayerLoadout);
		break;
	case SCREEN_ID_GAME	:
		pScreen = debug_new(CScreenGame);
		break;
	case SCREEN_ID_WEAPONS	:
		pScreen = debug_new(CScreenWeapons);
		break;
	case SCREEN_ID_CROSSHAIR:
		pScreen = debug_new(CScreenCrosshair);
		break;
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_HOST_OPTIONS_FILE:
		pScreen = debug_new(CScreenHostOptionFile);
		break;
#endif // PLATFORM_XENON
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	case SCREEN_ID_HOST_LEVELS:
		pScreen = debug_new(CScreenHostLevels);
		break;
	case SCREEN_ID_HOST_WEAPONS:
		pScreen = debug_new(CScreenHostWeapons);
		break;
	case SCREEN_ID_HOST_DOWNLOAD:
		pScreen = debug_new(CScreenHostDownload);
		break;
	case SCREEN_ID_HOST_VOTING:
		pScreen = debug_new(CScreenHostVoting);
		break;
#endif // PLATFORM_XENON
	case SCREEN_ID_FAILURE:
		pScreen = debug_new(CScreenFailure);
		break;
	case SCREEN_ID_PERFORMANCE	:
		pScreen = debug_new(CScreenPerformance);
		break;
	case SCREEN_ID_PERFORMANCE_CPU	:
		pScreen = debug_new(CScreenPerformanceCPU);
		break;
	case SCREEN_ID_PERFORMANCE_GPU	:
		pScreen = debug_new(CScreenPerformanceGPU);
		break;
	}

	if (pScreen)
	{
		pScreen->Init(screenID);
		m_screenArray.push_back(pScreen);
	}

}

bool CScreenMgr::ForceScreenUpdate(eScreenID screenID)
{
    if (!m_pCurrentScreen || m_eCurrentScreenID != screenID) return false;

	return m_pCurrentScreen->HandleForceUpdate();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenMgr::ScreenDimsChanged
//
//	PURPOSE:	Handle the screen dims changing
//
// --------------------------------------------------------------------------- //

void CScreenMgr::ScreenDimsChanged()
{
	UpdateMovieDims();

	if (m_pCurrentScreen)
	{
		m_pCurrentScreen->ScreenDimsChanged();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenMgr::UpdateMovieDims
//
//	PURPOSE:	Updates the movie dims
//
// --------------------------------------------------------------------------- //
void CScreenMgr::UpdateMovieDims()
{
	LTVector4 vMovieDest = g_pLayoutDB->GetVector4( g_pLayoutDB->GetSharedRecord(), "ScreenMovieRect" );

	m_rfMovieDest.Left()	= vMovieDest.x * g_pInterfaceResMgr->GetXRatio();
	m_rfMovieDest.Right()	= vMovieDest.z * g_pInterfaceResMgr->GetXRatio();
	m_rfMovieDest.Top()		= vMovieDest.y * g_pInterfaceResMgr->GetYRatio();
	m_rfMovieDest.Bottom()	= vMovieDest.w * g_pInterfaceResMgr->GetYRatio();
}



void CScreenMgr::ClearHistory()
{
	m_nHistoryLen = 0;
	m_eScreenHistory[0] = (eScreenID)0;
	m_pCurrentScreen = NULL;
	m_eCurrentScreenID = (eScreenID)0;

}

void CScreenMgr::AddScreenToHistory(eScreenID screenID)
{
	int insert = 0;
	while (insert < m_nHistoryLen && m_eScreenHistory[insert] != screenID)
		++insert;
	if (insert == m_nHistoryLen)
	{
		if (m_nHistoryLen < MAX_SCREEN_HISTORY)
		{
			++m_nHistoryLen;
			m_eScreenHistory[insert] = screenID;
		}
	}
	else
	{
		m_nHistoryLen = insert+1;
	}

}

const char* CScreenMgr::GetScreenName(eScreenID id)
{
	return s_aScreenName[id];
}

// Returns a screen ID (uint16) based on a name
uint16 CScreenMgr::GetScreenIDFromName(char * pName)
{
	for (uint16 i=0; i < SCREEN_ID_UNASSIGNED; ++i)
	{
		if (!LTStrICmp(pName, s_aScreenName[i]))
			return (i);
	}
	return SCREEN_ID_UNASSIGNED;
}
