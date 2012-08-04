// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPostload.h
//
// PURPOSE : Interface screen to be displayed after loading a level but before
//				starting to play it.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenPostload.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"


namespace
{
	bool s_bFlash;
	float s_fFlashTime;
	uint32 s_nFlashColor;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPostload::CScreenPostload()
{
	m_bReadLayout = false;
	
	m_pMissionNameStr = LTNULL;
	m_pLevelNameStr = LTNULL;
	m_pBriefingStr = LTNULL;
	m_pHelpStr = LTNULL;
	m_pContinueStr = LTNULL;

	m_nDefaultTitleFont = 0;
	m_nDefaultTitleFontSize = 0;
	m_nDefaultTitleColor = 0;
	m_nDefaultBriefingWidth = 0;
	m_nDefaultBriefingFont = 0;
	m_nDefaultBriefingFontSize = 0;
	m_nDefaultHelpColor = 0;
	m_nDefaultHelpWidth = 0;
	m_nDefaultHelpFont = 0;
	m_nDefaultHelpFontSize = 0;
	m_nDefaultHelpColor = 0;
	m_nDefaultContinueFont = 0;
	m_nDefaultContinueFontSize = 0;
	m_nDefaultContinueColor = 0;
	m_bPressAnyKey = false;
}

CScreenPostload::~CScreenPostload()
{
	if (m_pMissionNameStr)
	{
		g_pFontManager->DestroyPolyString(m_pMissionNameStr);
		m_pMissionNameStr = LTNULL;
	}
	if (m_pLevelNameStr)
	{
		g_pFontManager->DestroyPolyString(m_pLevelNameStr);
		m_pLevelNameStr = LTNULL;
	}
	if (m_pBriefingStr)
	{
		g_pFontManager->DestroyPolyString(m_pBriefingStr);
		m_pBriefingStr = LTNULL;
	}
	if (m_pHelpStr)
	{
		g_pFontManager->DestroyPolyString(m_pHelpStr);
		m_pHelpStr = LTNULL;
	}
}


// Build the screen
LTBOOL CScreenPostload::Build()
{
	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenPostload::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// [KLS 6/23/02] - Turn the cursor off until everything is loaded...
		g_pInterfaceMgr->UseCursor(LTFALSE, LTFALSE);

		// Assume we won't be showing the press any key text.
		m_bPressAnyKey = false;

		// Check if we have a mission entry.
		bool bGotMission = false;
		if( !g_pMissionMgr->IsCustomLevel( ))
		{
			int nCurMission = g_pMissionMgr->GetCurrentMission( );
			MISSION* pMission = g_pMissionButeMgr->GetMission( nCurMission );

			if( pMission )
			{
				m_missionname = LoadTempString(pMission->nNameId);
				int nCurLevel = g_pMissionMgr->GetCurrentLevel( );
				m_levelname = LoadTempString( pMission->aLevels[nCurLevel].nNameId );
				m_layout = pMission->szLayout;
				
				// Show a briefing for this level if it exists.  Also, only
				// show the briefing if we haven't been to this level before.
				int nBriefingId = pMission->aLevels[nCurLevel].nBriefingId;
				if( nBriefingId >= 0 && !g_pMissionMgr->IsRestoringLevel( ))
				{
					m_briefing = LoadTempString( nBriefingId );
					m_layout = pMission->szBriefLayout;
				}
				else
					m_briefing = "";

				// Show help text for this level if it exists.
				int nHelpId = pMission->aLevels[nCurLevel].nHelpId;
				if( nHelpId >= 0)
					m_help = LoadTempString( nHelpId );
				else
					m_help = "";
				bGotMission = true;
			}
		}


		// If we were unsuccessful in getting info from the mission, then just
		// use defaults.
		if( !bGotMission )
		{
			// If connecting to a remote server, set our mission descriptor to 
			// the ip we're connecting to.
			if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			{
				// Make a loading string using the IP to be joined.
				char szLoadingString[256];
				if (strlen(g_pClientMultiplayerMgr->GetStartGameRequest( ).m_HostInfo.m_sName))
				{
					sprintf( szLoadingString, "%s:  %s", LoadTempString(IDS_CONNECTING_TO_SERVER), 
						g_pClientMultiplayerMgr->GetStartGameRequest( ).m_HostInfo.m_sName );
				}
				else
				{
					sprintf( szLoadingString, "%s", LoadTempString(IDS_CONNECTING_TO_SERVER));
				}
				m_missionname = szLoadingString;

				sprintf( szLoadingString, "    (%s)", g_pClientMultiplayerMgr->GetStartGameRequest( ).m_TCPAddress );

				m_levelname = szLoadingString;
			}
			// Local game, set the mission descriptor to the level name.
			else
			{

				if (g_pGameClientShell->IsRunningPerformanceTest())
				{
					m_missionname = LoadTempString( IDS_TITLE_PERFORMANCE_TEST );
					m_levelname = "";
				}
				else
				{
					m_missionname = LoadTempString( IDS_CUSTOM_LEVEL );
					// Split the worldname up into parts so we can get the load string.
					char const* pszWorldName = g_pMissionMgr->GetCurrentWorldName( );
					char szWorldTitle[MAX_PATH] = "";
					_splitpath( pszWorldName, NULL, NULL, szWorldTitle, NULL );
					m_levelname = szWorldTitle;

				}

			}

			m_layout = "LoadScreenDefault";
			m_briefing = "";
		}

		char szTagName[30];
		if (!m_bReadLayout)
		{
			m_bReadLayout = true;

			SAFE_STRCPY(szTagName,"LoadScreenDefault");

			//default layout info title string
			m_DefaultTitlePos = g_pLayoutMgr->GetPoint(szTagName,"TitlePos");
			m_nDefaultTitleFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"TitleFont");
			m_nDefaultTitleFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"TitleSize");

			LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "TitleColor");
			uint8 nR = (uint8)vColor.x;
			uint8 nG = (uint8)vColor.y;
			uint8 nB = (uint8)vColor.z;

			m_nDefaultTitleColor = SET_ARGB(0xFF,nR,nG,nB);

			//default layout info level string
			m_DefaultLevelPos = g_pLayoutMgr->GetPoint(szTagName,"LevelPos");
			m_nDefaultLevelFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"LevelFont");
			m_nDefaultLevelFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"LevelSize");

			vColor = g_pLayoutMgr->GetVector(szTagName, "LevelColor");
			nR = (uint8)vColor.x;
			nG = (uint8)vColor.y;
			nB = (uint8)vColor.z;

			m_nDefaultLevelColor = SET_ARGB(0xFF,nR,nG,nB);

			//default layout info mission briefing string
			LTRect rect = g_pLayoutMgr->GetRect(szTagName,"BriefingRect");
			m_DefaultBriefingPos = LTIntPt(rect.left,rect.top);
			m_nDefaultBriefingWidth = (rect.right - rect.left);

			m_nDefaultBriefingFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"BriefingFont");
			m_nDefaultBriefingFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"BriefingSize");

			vColor = g_pLayoutMgr->GetVector(szTagName, "BriefingColor");
			nR = (uint8)vColor.x;
			nG = (uint8)vColor.y;
			nB = (uint8)vColor.z;

			m_nDefaultBriefingColor = SET_ARGB(0xFF,nR,nG,nB);

			//default layout info mission help string
			rect = g_pLayoutMgr->GetRect(szTagName,"HelpRect");
			m_DefaultHelpPos = LTIntPt(rect.left,rect.top);
			m_nDefaultHelpWidth = (rect.right - rect.left);

			m_nDefaultHelpFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"HelpFont");
			m_nDefaultHelpFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"HelpSize");

			vColor = g_pLayoutMgr->GetVector(szTagName, "HelpColor");
			nR = (uint8)vColor.x;
			nG = (uint8)vColor.y;
			nB = (uint8)vColor.z;

			m_nDefaultHelpColor = SET_ARGB(0xFF,nR,nG,nB);

			//default layout info Continue string
			m_DefaultContinuePos = g_pLayoutMgr->GetPoint(szTagName,"ContinuePos");
			m_nDefaultContinueFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"ContinueFont");
			m_nDefaultContinueFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"ContinueSize");

			vColor = g_pLayoutMgr->GetVector(szTagName, "ContinueColor");
			nR = (uint8)vColor.x;
			nG = (uint8)vColor.y;
			nB = (uint8)vColor.z;

			m_nDefaultContinueColor = SET_ARGB(0xFF,nR,nG,nB);

		}

		//Setup title string
		LTIntPt TitlePos			= m_DefaultTitlePos;
		uint8	TitleFont			= m_nDefaultTitleFont;
		uint8	TitleFontSize		= m_nDefaultTitleFontSize;
		uint32	TitleColor			= m_nDefaultTitleColor;
		LTIntPt LevelPos			= m_DefaultLevelPos;
		uint8	LevelFont			= m_nDefaultLevelFont;
		uint8	LevelFontSize		= m_nDefaultLevelFontSize;
		uint32	LevelColor			= m_nDefaultLevelColor;

		//look for override values
		SAFE_STRCPY(szTagName,m_layout.c_str());
		if (g_pLayoutMgr->Exist(szTagName))
		{
			//override layout info title string
			if (g_pLayoutMgr->HasValue(szTagName,"TitlePos"))
				TitlePos = g_pLayoutMgr->GetPoint(szTagName,"TitlePos");

			if (g_pLayoutMgr->HasValue(szTagName,"TitleFont"))
				TitleFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"TitleFont");

			if (g_pLayoutMgr->HasValue(szTagName,"TitleSize"))
				TitleFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"TitleSize");

			if (g_pLayoutMgr->HasValue(szTagName,"TitleColor"))
			{
				LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "TitleColor");
				uint8 nR = (uint8)vColor.x;
				uint8 nG = (uint8)vColor.y;
				uint8 nB = (uint8)vColor.z;

				TitleColor = SET_ARGB(0xFF,nR,nG,nB);
			}

			//override layout info Level string
			if (g_pLayoutMgr->HasValue(szTagName,"LevelPos"))
				LevelPos = g_pLayoutMgr->GetPoint(szTagName,"LevelPos");

			if (g_pLayoutMgr->HasValue(szTagName,"LevelFont"))
				LevelFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"LevelFont");

			if (g_pLayoutMgr->HasValue(szTagName,"LevelSize"))
				LevelFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"LevelSize");

			if (g_pLayoutMgr->HasValue(szTagName,"LevelColor"))
			{
				LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "LevelColor");
				uint8 nR = (uint8)vColor.x;
				uint8 nG = (uint8)vColor.y;
				uint8 nB = (uint8)vColor.z;

				LevelColor = SET_ARGB(0xFF,nR,nG,nB);
			}
		}

		uint8 nFontSize = (uint8)((float)TitleFontSize * g_pInterfaceResMgr->GetXRatio());
		CUIFont *pFont = g_pInterfaceResMgr->GetFont(TitleFont);

		if (!m_pMissionNameStr)
		{
			m_pMissionNameStr = g_pFontManager->CreateFormattedPolyString(pFont,"");
		}
		m_pMissionNameStr->SetColor(TitleColor);
		m_pMissionNameStr->SetText(m_missionname.c_str());
		m_pMissionNameStr->SetCharScreenHeight(nFontSize);
		float x = (float)TitlePos.x * g_pInterfaceResMgr->GetXRatio();
		float y = (float)TitlePos.y * g_pInterfaceResMgr->GetYRatio();
		m_pMissionNameStr->SetPosition(x,y);

		nFontSize = (uint8)((float)LevelFontSize * g_pInterfaceResMgr->GetXRatio());
		pFont = g_pInterfaceResMgr->GetFont(LevelFont);

		if (!m_pLevelNameStr)
		{
			m_pLevelNameStr = g_pFontManager->CreateFormattedPolyString(pFont,"");
		}
		m_pLevelNameStr->SetColor(LevelColor);
		m_pLevelNameStr->SetText(m_levelname.c_str());
		m_pLevelNameStr->SetCharScreenHeight(nFontSize);
		x = (float)LevelPos.x * g_pInterfaceResMgr->GetXRatio();
		y = (float)LevelPos.y * g_pInterfaceResMgr->GetYRatio();
		m_pLevelNameStr->SetPosition(x,y);

		//Setup briefing string
		LTIntPt BriefingPos			= m_DefaultBriefingPos;
		uint16	BriefingWidth		= m_nDefaultBriefingWidth;
		uint8	BriefingFont		= m_nDefaultBriefingFont;
		uint8	BriefingFontSize	= m_nDefaultBriefingFontSize;
		uint32	BriefingColor		= m_nDefaultBriefingColor;

		//look for override values
		if (g_pLayoutMgr->Exist(szTagName))
		{
			//default layout info mission briefing string
			if (g_pLayoutMgr->HasValue(szTagName,"BriefingRect"))
			{
				LTRect rect = g_pLayoutMgr->GetRect(szTagName,"BriefingRect");
				BriefingPos = LTIntPt(rect.left,rect.top);
				BriefingWidth = (rect.right - rect.left);
			}

			if (g_pLayoutMgr->HasValue(szTagName,"BriefingFont"))
				BriefingFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"BriefingFont");

			if (g_pLayoutMgr->HasValue(szTagName,"BriefingSize"))
				BriefingFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"BriefingSize");

			if (g_pLayoutMgr->HasValue(szTagName,"BriefingColor"))
			{
				LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "BriefingColor");
				uint8 nR = (uint8)vColor.x;
				uint8 nG = (uint8)vColor.y;
				uint8 nB = (uint8)vColor.z;

				BriefingColor = SET_ARGB(0xFF,nR,nG,nB);
			}

		}

		nFontSize = (uint8)((float)BriefingFontSize * g_pInterfaceResMgr->GetXRatio());
		pFont = g_pInterfaceResMgr->GetFont(BriefingFont);

		if (!m_pBriefingStr)
		{
			m_pBriefingStr = g_pFontManager->CreateFormattedPolyString(pFont,"");
		}
		
		m_pBriefingStr->SetText(m_briefing.c_str());
		m_pBriefingStr->SetColor(BriefingColor);
		m_pBriefingStr->SetCharScreenHeight(nFontSize);
		x = (float)BriefingPos.x * g_pInterfaceResMgr->GetXRatio();
		y = (float)BriefingPos.y * g_pInterfaceResMgr->GetYRatio();
		m_pBriefingStr->SetPosition(x,y);
		m_pBriefingStr->SetWrapWidth((uint16)(g_pInterfaceResMgr->GetXRatio() * (float)BriefingWidth));


		//Setup briefing string
		LTIntPt HelpPos			= m_DefaultHelpPos;
		uint16	HelpWidth		= m_nDefaultHelpWidth;
		uint8	HelpFont		= m_nDefaultHelpFont;
		uint8	HelpFontSize	= m_nDefaultHelpFontSize;
		uint32	HelpColor		= m_nDefaultHelpColor;

		//look for override values
		if (g_pLayoutMgr->Exist(szTagName))
		{
			//default layout info mission briefing string
			if (g_pLayoutMgr->HasValue(szTagName,"HelpRect"))
			{
				LTRect rect = g_pLayoutMgr->GetRect(szTagName,"HelpRect");
				HelpPos = LTIntPt(rect.left,rect.top);
				HelpWidth = (rect.right - rect.left);
			}

			if (g_pLayoutMgr->HasValue(szTagName,"HelpFont"))
				HelpFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"HelpFont");

			if (g_pLayoutMgr->HasValue(szTagName,"HelpSize"))
				HelpFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"HelpSize");

			if (g_pLayoutMgr->HasValue(szTagName,"HelpColor"))
			{
				LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "HelpColor");
				uint8 nR = (uint8)vColor.x;
				uint8 nG = (uint8)vColor.y;
				uint8 nB = (uint8)vColor.z;

				HelpColor = SET_ARGB(0xFF,nR,nG,nB);
			}

		}

		nFontSize = (uint8)((float)HelpFontSize * g_pInterfaceResMgr->GetXRatio());
		pFont = g_pInterfaceResMgr->GetFont(HelpFont);

		if (!m_pHelpStr)
		{
			m_pHelpStr = g_pFontManager->CreateFormattedPolyString(pFont,"");
		}
	
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		if (pProfile->m_bLoadScreenTips)
		{

			m_pHelpStr->SetText(m_help.c_str());
			m_pHelpStr->SetColor(HelpColor);
			m_pHelpStr->SetCharScreenHeight(nFontSize);
			x = (float)HelpPos.x * g_pInterfaceResMgr->GetXRatio();
			y = (float)HelpPos.y * g_pInterfaceResMgr->GetYRatio();
			m_pHelpStr->SetPosition(x,y);
			m_pHelpStr->SetWrapWidth((uint16)(g_pInterfaceResMgr->GetXRatio() * (float)HelpWidth));
		}
		else
		{
			m_pHelpStr->SetText("");
		}

		
		//Setup Continue string
		LTIntPt ContinuePos				= m_DefaultContinuePos;
		uint8	ContinueFont			= m_nDefaultContinueFont;
		uint8	ContinueFontSize		= m_nDefaultContinueFontSize;
		m_nContinueColor				= m_nDefaultContinueColor;

		//look for override values
		if (g_pLayoutMgr->Exist(szTagName))
		{
			//override layout info Continue string
			if (g_pLayoutMgr->HasValue(szTagName,"ContinuePos"))
				ContinuePos = g_pLayoutMgr->GetPoint(szTagName,"ContinuePos");

			if (g_pLayoutMgr->HasValue(szTagName,"ContinueFont"))
				ContinueFont = (uint8)g_pLayoutMgr->GetInt(szTagName,"ContinueFont");

			if (g_pLayoutMgr->HasValue(szTagName,"ContinueSize"))
				ContinueFontSize = (uint8)g_pLayoutMgr->GetInt(szTagName,"ContinueSize");

			if (g_pLayoutMgr->HasValue(szTagName,"ContinueColor"))
			{
				LTVector vColor = g_pLayoutMgr->GetVector(szTagName, "ContinueColor");
				uint8 nR = (uint8)vColor.x;
				uint8 nG = (uint8)vColor.y;
				uint8 nB = (uint8)vColor.z;

				m_nContinueColor = SET_ARGB(0xFF,nR,nG,nB);
			}

		}

		// [KLS 6/21/02] - We restore music here to make sure everything is loaded
		// before the "press any key" text is shown...

		g_pGameClientShell->RestoreMusic();

		// [KLS 6/23/02] - Turn the cursor on once everything is loaded...
		g_pInterfaceMgr->UseCursor(LTTRUE);


		nFontSize = (uint8)((float)ContinueFontSize * g_pInterfaceResMgr->GetXRatio());
		pFont = g_pInterfaceResMgr->GetFont(ContinueFont);

		if (!m_pContinueStr)
		{
			m_pContinueStr = g_pFontManager->CreateFormattedPolyString(pFont,"");
		}

		// If we have a briefing, then post the press any key.
		if( m_briefing.length( ) > 0 )
		{
			m_bPressAnyKey = true;
			m_pContinueStr->SetText( LoadTempString( IDS_PRESS_ANY_KEY ));
		}
		// Otherwise show the "waiting for other players".
		else
		{
			m_bPressAnyKey = false;
			m_pContinueStr->SetText( LoadTempString( IDS_WAITINGFOROTHERPLAYERS ));

			g_pGameClientShell->SendClientLoadedMessage( );
		}

		m_pContinueStr->SetColor(m_nContinueColor);
		m_pContinueStr->SetCharScreenHeight(nFontSize);
		x = (float)ContinuePos.x * g_pInterfaceResMgr->GetXRatio();
		y = (float)ContinuePos.y * g_pInterfaceResMgr->GetYRatio();
		m_pContinueStr->SetPosition(x,y);

		s_bFlash = false;
		s_fFlashTime = g_pLTClient->GetTime() + 0.333f;

		s_nFlashColor = (m_nContinueColor ^ 0x00FFFFFF);

		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\pressanykey.wav");
	}

	CBaseScreen::OnFocus(bFocus);
	m_bVisited = LTFALSE;

}


void CScreenPostload::Escape()
{
	// Check if they were prompted to press any key.
	if( m_bPressAnyKey )
	{
		// Acknowledge keypress.
		m_bPressAnyKey = false;

		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
		{
			// Tell the client we're ready to play.
			g_pGameClientShell->SendClientLoadedMessage( );

			// Change to waiting for other players.
			m_pContinueStr->SetText( LoadTempString(IDS_WAITINGFOROTHERPLAYERS));
			m_pContinueStr->SetColor(m_nContinueColor);
		}
		else
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
		}
	}
}

bool CScreenPostload::UpdateInterfaceSFX()
{
	if( m_bPressAnyKey )
	{
		float fTime = g_pLTClient->GetTime();
		if (fTime > s_fFlashTime)
		{
			s_bFlash = !s_bFlash;
			s_fFlashTime = fTime + 0.333f;
			if (s_bFlash)
			{
				m_pContinueStr->SetColor(s_nFlashColor);
			}
			else
			{
				m_pContinueStr->SetColor(m_nContinueColor);
			}

		}
	}


	return CBaseScreen::UpdateInterfaceSFX();
}

void CScreenPostload::CreateInterfaceSFX()
{

	int n = 0;
	char szTagName[30];
	char szAttName[30];
	char szFXName[128];

	m_bHaveLights = LTFALSE;


	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

    g_pLTClient->GetObjectPos(hCamera, &s_vPos);
    g_pLTClient->GetObjectRotation(hCamera, &s_rRot);
	s_vU = s_rRot.Up();
	s_vR = s_rRot.Right();
	s_vF = s_rRot.Forward();


	SAFE_STRCPY(szTagName,m_layout.c_str());
	if (!g_pLayoutMgr->Exist(szTagName))
		SAFE_STRCPY(szTagName,"LoadScreenDefault");

	sprintf(szAttName,"Light%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateLightFX(szFXName);
		}

		n++;
		sprintf(szAttName,"Light%d",n);

	}


	n = 0;
	sprintf(szAttName,"PostScale%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CBaseScaleFX *pSFX = CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"PostScale%d",n);

	}

	

	n = 0;
	sprintf(szAttName,"PostFX%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			INT_FX* pFX = g_pLayoutMgr->GetFX(szFXName);
			if (pFX)
			{
				g_pInterfaceMgr->AddInterfaceFX(LTNULL, pFX->szFXName,pFX->vPos,pFX->bLoop);
			}
		}

		
		n++;
		sprintf(szAttName,"PostFX%d",n);

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPostload::Render
//
//	PURPOSE:	Renders the screen to a surface
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenPostload::Render(HSURFACE hDestSurf)
{
	// Check if we're finished waiting for a key.
	if( !m_bPressAnyKey )
	{
		// Check if the server's ready for us to get in and play.
		if( g_pGameClientShell->GetSwitchingWorldsState( ) == eSwitchingWorldsStateFinished )
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
			return TRUE;
		}
	}

	if (m_pMissionNameStr)
		m_pMissionNameStr->Render();
	if (m_pLevelNameStr)
		m_pLevelNameStr->Render();
	if (m_pBriefingStr)
		m_pBriefingStr->Render();
	if (m_pHelpStr)
		m_pHelpStr->Render();
	if (m_pContinueStr)
		m_pContinueStr->Render();

	return LTTRUE;

}