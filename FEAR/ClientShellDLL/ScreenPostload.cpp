// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPostload.h
//
// PURPOSE : Interface screen to be displayed after loading a level but before
//				starting to play it.
//
// (c) 2002-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenPostload.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientConnectionMgr.h"
#include "LoadingScreenLayout.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"

namespace
{
	bool s_bFlash;
	double s_fFlashTime;
	uint32 s_nFlashColor;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPostload::CScreenPostload()
{
	m_bPressAnyKey = false;
	m_nLoadDelay = 0;
}

CScreenPostload::~CScreenPostload()
{
}


// Build the screen
bool CScreenPostload::Build()
{
	// Make sure to call the base class

	CFontInfo Font(g_pLayoutDB->GetHelpFont(),12);

	CLTGUICtrl_create cs;
	m_MissionName.Create(L"",Font,cs);
	m_LevelName.Create(L"",Font,cs);
	m_Briefing.Create(L"",Font,cs);
	m_Help.Create(L"",Font,cs);
	m_Continue.Create(L"",Font,cs);


	if (! CBaseScreen::Build()) return false;

	UseBack(false);
	return true;
}


void CScreenPostload::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		// [KLS 6/23/02] - Turn the cursor off until everything is loaded...
		g_pInterfaceMgr->UseCursor(false, false);

		if (IsMultiplayerGameClient() && (GameModeMgr::Instance( ).m_grbUseTeams || GameModeMgr::Instance( ).m_grbUseLoadout ))
		{
			g_pClientConnectionMgr->SendMultiPlayerInfo();

			//delay sending the loaded message to guarantee the server has time to process the MP Info message
			m_nLoadDelay = 2; //delay two frames because the first update happens this same frame
		}
		else
		{
			m_nLoadDelay = 1; //send the loaded message on the next update (same frame)
		}


		// Check if we have a mission entry.
		bool bGotMission = false;
		if( !g_pMissionMgr->IsCustomLevel( ))
		{
			int nCurMission = g_pMissionMgr->GetCurrentMission( );
			HRECORD hMission = g_pMissionDB->GetMission( nCurMission );

			if( hMission )
			{
				int nCurLevel = g_pMissionMgr->GetCurrentLevel( );
				HRECORD hLevel = g_pMissionDB->GetLevel(hMission,nCurLevel);
				const char* szNameId = g_pMissionDB->GetString(hMission,MDB_Name);
				std::wstring sName;
				if( szNameId[0] != '\0' )
				{
					sName = LoadString(szNameId);
				}
				else
				{
					sName = g_pMissionDB->GetWString(hMission,MDB_NameStr);
					if (!sName.length())
					{
						sName = MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str();
					}
				}
				m_MissionName.SetString( sName.c_str() );
				m_LevelName.SetString(LoadString( g_pMissionDB->GetString(hLevel,MDB_Name)));
				
				// Show a briefing for this level if it exists.  Also, only
				// show the briefing if we haven't been to this level before.
				const char* szBriefingId = g_pMissionDB->GetString(hLevel,MDB_Briefing);
				if( (!LTStrIEquals(szBriefingId,"<none>")) )//&& !g_pMissionMgr->IsRestoringLevel( ))
				{
					m_Briefing.SetString(LoadString( szBriefingId ));
				}
				else
				{
					m_Briefing.SetString(L"");
				}

				// Show help text for this level if it exists.
				const char* szHelpId = g_pMissionDB->GetString(hLevel,MDB_Help);
				if( szHelpId[0] != '\0' )
				{
					m_Help.SetString(CreateHelpString( szHelpId ));
				}
				else
				{
					m_Help.SetString(L"");
				}
				bGotMission = true;

				if( IsMultiplayerGameClient( ))
				{
					m_photo = g_pMissionDB->GetString(hLevel,MDB_Photo);
				}
			}
		}


		// If we were unsuccessful in getting info from the mission, then just
		// use defaults.
		if( !bGotMission )
		{
			// If connecting to a remote server, set our mission descriptor to 
			// the ip we're connecting to.
			if( g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
			{
				// Make a loading string using the IP to be joined.
				wchar_t szLoadingString[256];

				if ( !GameModeMgr::Instance( ).m_grwsSessionName.GetValue().empty())
				{
					LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"%s:  %s", LoadString("IDS_CONNECTING_TO_SERVER"), 
						( wchar_t const* )GameModeMgr::Instance( ).m_grwsSessionName );
				}
				else
				{
					LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"%s", LoadString("IDS_CONNECTING_TO_SERVER"));
				}
				m_MissionName.SetString(szLoadingString);

				LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"    (%S)", g_pClientConnectionMgr->GetStartGameRequest( ).m_TCPAddress );

				m_LevelName.SetString(szLoadingString);
			}
			// Local game, set the mission descriptor to the level name.
			else
			{
				if (g_pGameClientShell->IsRunningPerformanceTest())
				{
					m_MissionName.SetString(LoadString( "IDS_TITLE_PERFORMANCE_TEST" ));
					m_LevelName.SetString(L"");
				}
				else
				{
					m_MissionName.SetString(LoadString( "IDS_CUSTOM_LEVEL" ));
					// Split the worldname up into parts so we can get the load string.
					wchar_t szWorldTitle[MAX_PATH] = L"";
					_wsplitpath( MPA2W(g_pMissionMgr->GetCurrentWorldName( )).c_str(), NULL, NULL, szWorldTitle, NULL );
					m_LevelName.SetString(szWorldTitle);
				}
			}

			m_Briefing.SetString(L"");
			m_Help.SetString(L"");
		}

		float w = float(g_pInterfaceResMgr->GetScreenWidth());
		float h = float(g_pInterfaceResMgr->GetScreenHeight());

		m_hBackTexture.Load(g_CurrentLayout.m_sBackTexture.c_str());

		LTVector2 vHalfTexel(0.5f / (float)w, 0.5f / (float)h);
		SetupQuadUVs(m_BackPoly, m_hBackTexture, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f );
		DrawPrimSetRGBA(m_BackPoly,argbWhite);
		DrawPrimSetXYWH(m_BackPoly,-0.5f,-0.5f,w,h);

		//*******************************************************************************
		// Build Mission Title String
		m_MissionName.SetFont(g_CurrentLayout.m_sTitleFont);
		m_MissionName.SetBasePos(g_CurrentLayout.m_TitlePos);
		m_MissionName.SetColor(g_CurrentLayout.m_cTitleColor);
		m_MissionName.SetScale(g_pInterfaceResMgr->GetScreenScale());
		m_MissionName.SetDropShadow(1);


		//*******************************************************************************
		// Build Mission Level String
		m_LevelName.SetFont(g_CurrentLayout.m_sLevelFont);
		m_LevelName.SetBasePos(g_CurrentLayout.m_LevelPos);
		m_LevelName.SetColor(g_CurrentLayout.m_cLevelColor);
		m_LevelName.SetScale(g_pInterfaceResMgr->GetScreenScale());
		m_LevelName.SetDropShadow(1);

		//*******************************************************************************
		// Build Mission Briefing String

		m_Briefing.SetFont(g_CurrentLayout.m_sBriefingFont);
		m_Briefing.SetBasePos(g_CurrentLayout.m_BriefingRect.m_vMin);
		m_Briefing.SetColor(g_CurrentLayout.m_cBriefingColor);
		m_Briefing.SetScale(g_pInterfaceResMgr->GetScreenScale());
		m_Briefing.SetDropShadow(1);

		if (g_pGameClientShell->IsRunningPerformanceTest())
		{
			m_Briefing.SetString(LoadString( "IDS_PERFORMANCE_TEST_BRIEF" ));
		}

		LTVector2n sz = g_CurrentLayout.m_BriefingRect.m_vMax - g_CurrentLayout.m_BriefingRect.m_vMin;
		m_Briefing.SetSize( sz );
		m_Briefing.SetWordWrap(true);



		//*******************************************************************************
		// Build Mission Help String
		m_Help.SetFont(g_CurrentLayout.m_sHelpFont);
		m_Help.SetBasePos(g_CurrentLayout.m_HelpRect.m_vMin);
		m_Help.SetColor(g_CurrentLayout.m_cHelpColor);
		m_Help.SetWordWrap(true);
		sz = g_CurrentLayout.m_HelpRect.m_vMax - g_CurrentLayout.m_HelpRect.m_vMin;
		m_Help.SetSize(sz);
		m_Help.SetScale(g_pInterfaceResMgr->GetScreenScale());
		m_Help.SetDropShadow(1);

//		m_Help.Show(pProfile->m_bLoadScreenTips);

		if( IsMultiplayerGameClient() && !m_photo.empty() )
		{
			m_hPhoto.Load(m_photo.c_str());
			if (m_hPhoto)
			{
				SetupQuadUVs(m_photoPoly, m_hPhoto, 0.0f, 0.0f, 1.0f, 1.0f);
				DrawPrimSetRGBA(m_photoPoly,argbWhite);

				float fScale = g_pInterfaceResMgr->GetXRatio();
				float fx = (float)g_CurrentLayout.m_PhotoRect.Left() * fScale;
				float fy = (float)g_CurrentLayout.m_PhotoRect.Top() * fScale;

				float fw = (float)(g_CurrentLayout.m_PhotoRect.GetWidth()) * fScale;
				float fh = (float)(g_CurrentLayout.m_PhotoRect.GetHeight()) * fScale;

				DrawPrimSetXYWH(m_photoPoly,fx,fy,fw,fh);
			}
		}


		m_Continue.SetFont(g_CurrentLayout.m_sContinueFont);
		m_Continue.SetBasePos(g_CurrentLayout.m_ContinuePos);
		m_Continue.SetColor(g_CurrentLayout.m_cContinueColor);
		m_Continue.SetScale(g_pInterfaceResMgr->GetScreenScale());
		m_Continue.SetAlignment(kCenter);

		// Assume we won't be showing the press any key text.
		m_bPressAnyKey = false;

		// If we have a briefing, then post the press any key.
		if( m_Briefing.GetString() && m_Briefing.GetString()[0] )
		{
			if (!g_pGameClientShell->IsRunningPerformanceTest() && !g_pMissionMgr->IsRestoringLevel( ))
			{
				m_bPressAnyKey = true;
				m_Continue.SetString( LoadString( "IDS_PRESS_ANY_KEY" ));
			}
		}
		// Otherwise show the "waiting for other players".
		else if( IsMultiplayerGameClient())
		{
			m_Continue.SetString( LoadString( "IDS_JOINING" ));
		}

		if (m_bPressAnyKey)
		{
			// KLS - Turn the cursor on once everything is loaded...If we actually
			// want to show it...
			g_pInterfaceMgr->UseCursor(true);

			s_bFlash = false;
			s_fFlashTime = RealTimeTimer::Instance().GetTimerAccumulatedS() + 0.333f;
			s_nFlashColor = (g_CurrentLayout.m_cContinueColor ^ 0x00FFFFFF);

			g_pClientSoundMgr->PlayInterfaceDBSound("PressAnyKey");
		}
	}
	else
	{
		m_hBackTexture.Free();
	}

	CBaseScreen::OnFocus(bFocus);
	m_bVisited = false;
}


void CScreenPostload::Escape()
{
	// Check if they were prompted to press any key.
	if( m_bPressAnyKey )
	{
		// Acknowledge keypress.
		m_bPressAnyKey = false;
	}
}

bool CScreenPostload::UpdateInterfaceSFX()
{
	if( m_bPressAnyKey )
	{
		double fTime = RealTimeTimer::Instance().GetTimerAccumulatedS();
		if (fTime > s_fFlashTime)
		{
			s_bFlash = !s_bFlash;
			s_fFlashTime = fTime + 0.333f;
			if (s_bFlash)
			{
				m_Continue.SetColor(s_nFlashColor);
			}
			else
			{
				m_Continue.SetColor(g_CurrentLayout.m_cContinueColor);
			}

		}
	}
	else if (m_nLoadDelay) 
	{
		m_nLoadDelay--;
	}

	return CBaseScreen::UpdateInterfaceSFX();
}

void CScreenPostload::CreateInterfaceSFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPostload::Render
//
//	PURPOSE:	Renders the screen to a surface
//
// ----------------------------------------------------------------------- //

bool CScreenPostload::Render()
{

	g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, 0);
	// Mmm..  Triple dimensional...
	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

		g_pDrawPrim->SetTexture(m_hBackTexture);
		g_pDrawPrim->DrawPrim(&m_BackPoly, 1);

		m_MissionName.Render();
		m_LevelName.Render();
		m_Briefing.Render();
		m_Help.Render();

		if (m_bPressAnyKey)
		{
			m_Continue.Render();
		}

		if (m_hPhoto)
		{
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

			g_pDrawPrim->SetTexture(m_hPhoto);
			g_pDrawPrim->DrawPrim(&m_photoPoly);
		}

		g_pLTClient->RenderConsoleToRenderTarget();

		g_pLTClient->GetRenderer()->End3D();
//		g_pLTClient->GetRenderer()->FlipScreen();
	}

	// Check if we're finished waiting for a key... do this after rendering to prevent problems with freeing
	//		resources in a change state call triggered below
	if( !m_bPressAnyKey && !m_nLoadDelay )
	{
		// If we're ready to go, but we're waiting for others, then post the waiting for others text.
		if( g_pClientConnectionMgr->GetSentClientConnectionState() == eClientConnectionState_InWorld )
		{
			if( IsMultiplayerGameClient( ))
				m_Continue.SetString( LoadString("IDS_WAITINGFOROTHERPLAYERS"));
		}
		else
		{
			// Send the inworld message to the server here.  We can get here
			// from several ways, so the postload is in charge of sending this
			// message so that it is localized to one place.
			g_pClientConnectionMgr->SendClientInWorldMessage();
		}


		// Don't do anything until the server has put the player in the world.  Once
		// our player state isn't none, we can start playing.
		if( g_pPlayerMgr->GetPlayerState() != ePlayerState_None )
		{
			g_pInterfaceMgr->ChangeState( GS_PLAYING );
		}
	}


	return true;

}