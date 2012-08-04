// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.cpp
//
// PURPOSE : Background-thread loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LoadingScreen.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "BaseScreen.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "FXButeMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ScreenPostload.h"
#include "ClientResShared.h"

extern CGameClientShell* g_pGameClientShell;

CLoadingScreen::CLoadingScreen() :
	m_eCurState(STATE_NONE)
{
	// Create the event handles
	m_hEventEnd = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventThreadRunning = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nOldFarZ = 10000;
	m_bOldFogEnable = LTFALSE;
	m_pMissionNameStr = LTNULL;
	m_pLevelNameStr = LTNULL;
	m_pBriefingStr = LTNULL;
	m_pHelpStr = LTNULL;

	m_bReadLayout = false;
	
	m_nDefaultTitleFont = 0;
	m_nDefaultTitleFontSize = 0;
	m_nDefaultTitleColor = 0;

	m_nDefaultBriefingWidth = 0;
	m_nDefaultBriefingFont = 0;
	m_nDefaultBriefingFontSize = 0;
	m_nDefaultBriefingColor = 0;

	m_nDefaultHelpWidth = 0;
	m_nDefaultHelpFont = 0;
	m_nDefaultHelpFontSize = 0;
	m_nDefaultHelpColor = 0;

	InitializeCriticalSection(&m_MissionUpdate);

}

CLoadingScreen::~CLoadingScreen()
{
	// Terminate the object, just in case...
	Term();

	DeleteObject(m_hEventEnd);
	DeleteObject(m_hEventThreadRunning);

	DeleteCriticalSection(&m_MissionUpdate);

}

void CLoadingScreen::CreateScaleFX(char *szFXName)
{	
	if( m_pRenderScreen )
		return;

	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szFXName);
	if (pScaleFX)
	{
		CBaseScaleFX *pSFX = debug_new(CBaseScaleFX);
		if (!g_pFXButeMgr->CreateScaleFX(pScaleFX,m_vPos, m_vF, LTNULL, &m_rRot, pSFX))
		{
			debug_delete(pSFX);
			return;
		}
		m_SFXArray.Add(pSFX);
		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_NORMAL);				

		//adjust the object's position based on screen res
		HOBJECT hSFX = pSFX->GetObject();
		if (hSFX)
		{
			LTVector vNewPos;
			g_pLTClient->GetObjectPos(hSFX, &vNewPos);
			vNewPos.z;
			g_pLTClient->SetObjectPos(hSFX, &vNewPos);
		}
	}
}

void CLoadingScreen::CreateInterfaceSFX()
{
	if( m_pRenderScreen )
		return;
	
	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

	g_pLTClient->GetObjectPos(hCamera, &m_vPos);
	g_pLTClient->GetObjectRotation(hCamera, &m_rRot);
	m_vU = m_rRot.Up();
	m_vR = m_rRot.Right();
	m_vF = m_rRot.Forward();

	int n = 0;
	char szTagName[30];
	char szAttName[30];
	char szFXName[128];

	sprintf(szAttName,"Light%d",n);
	SAFE_STRCPY(szTagName,m_layout.c_str());
	if (!g_pLayoutMgr->Exist(szTagName))
		SAFE_STRCPY(szTagName,"LoadScreenDefault");
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
	sprintf(szAttName,"LoadScale%d",n);
	while (g_pLayoutMgr->HasValue(szTagName,szAttName))
	{
		
		g_pLayoutMgr->GetString(szTagName,szAttName,szFXName,128);
		if (strlen(szFXName))
		{
			CreateScaleFX(szFXName);
		}

		n++;
		sprintf(szAttName,"LoadScale%d",n);

	}

}

void CLoadingScreen::RemoveInterfaceSFX()
{
	g_pInterfaceMgr->RemoveInterfaceFX();

	while (m_SFXArray.GetSize() > 0)
	{
		CSpecialFX *pSFX = m_SFXArray[0];
		g_pInterfaceMgr->RemoveInterfaceSFX(pSFX);
		debug_delete(pSFX);
		m_SFXArray.Remove(0);
	}


}

void CLoadingScreen::UpdateInterfaceSFX()
{
	if( m_pRenderScreen )
		return;

	// Update the model's animtracker by hand...
	g_pInterfaceMgr->UpdateModelAnimations(m_fCurFrameDelta);

}

LTBOOL CLoadingScreen::Init()
{
	if (m_eCurState != STATE_NONE)
		return LTFALSE;

	if( m_pRenderScreen )
	{
		// Just let the render screen know it is about to be draw and set the state to init...

		m_pRenderScreen->OnFocus( true );
		m_eCurState = STATE_INIT;

		return LTTRUE;
	}
	
	m_photo = "";

	// Check if we have a mission entry.
	bool bGotMission = false;
	
	if( !g_pMissionMgr->IsCustomLevel( ))
	{
		int nCurMission = g_pMissionMgr->GetCurrentMission( );
		MISSION* pMission = g_pMissionButeMgr->GetMission( nCurMission );

		if( pMission )
		{
			if (pMission->nNameId > 0)
				m_missionname = LoadTempString(pMission->nNameId);
			else
				m_missionname = pMission->sName;

			int nCurLevel = g_pMissionMgr->GetCurrentLevel( );
			m_levelname = LoadTempString( pMission->aLevels[nCurLevel].nNameId );
			m_layout = pMission->szLayout;

			//set the post load layout here, so that it matches
			// even when our mission info is updated partway through the load
			CScreenPostload *pPostload = (CScreenPostload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_POSTLOAD));
			if (pPostload)
			{
				pPostload->SetLayout(pMission->szLayout);
			}


			// Show a briefing for this level if it exists.  Also, only
			// show the briefing if we haven't been to this level before.
			int nBriefingId = pMission->aLevels[nCurLevel].nBriefingId;
			if( nBriefingId >= 0 && !g_pMissionMgr->IsRestoringLevel( ))
			{
				m_briefing = LoadTempString( nBriefingId );
				m_layout = pMission->szBriefLayout;
				if (pPostload)
				{
					pPostload->SetLayout(pMission->szBriefLayout);
				}
			}
			else
				m_briefing = "";

			// Show help text for this level if it exists.
			int nHelpId = pMission->aLevels[nCurLevel].nHelpId;
			if( nHelpId >= 0)
			{
				m_help = LoadTempString( nHelpId );
				if (pPostload)
				{
					pPostload->SetLayout(pMission->szBriefLayout);
				}
			}
			else
				m_help = "";

			if( !IsCoopMultiplayerGameType( ))
			{
				m_photo = pMission->sPhoto;
			}


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
		m_help = "";
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

		//default layout info title string
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

		m_DefaultPhotoRect = g_pLayoutMgr->GetRect(szTagName,"PhotoRect");

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
	LTIntPt BriefingPos			= m_DefaultBriefingPos;
	uint16	BriefingWidth		= m_nDefaultBriefingWidth;
	uint8	BriefingFont		= m_nDefaultBriefingFont;
	uint8	BriefingFontSize	= m_nDefaultBriefingFontSize;
	uint32	BriefingColor		= m_nDefaultBriefingColor;
	LTIntPt HelpPos				= m_DefaultHelpPos;
	uint16	HelpWidth			= m_nDefaultHelpWidth;
	uint8	HelpFont			= m_nDefaultHelpFont;
	uint8	HelpFontSize		= m_nDefaultHelpFontSize;
	uint32	HelpColor			= m_nDefaultHelpColor;

	//*******************************************************************************
	// Build Mission Name String

	// look for override values
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


	//*******************************************************************************
	// Build Mission Briefing String

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

	
	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		m_pBriefingStr->SetText(LoadTempString( IDS_PERFORMANCE_TEST_BRIEF ));
	}
	else
	{
		m_pBriefingStr->SetText(m_briefing.c_str());
	}
	m_pBriefingStr->SetColor(BriefingColor);
	m_pBriefingStr->SetCharScreenHeight(nFontSize);
	x = (float)BriefingPos.x * g_pInterfaceResMgr->GetXRatio();
	y = (float)BriefingPos.y * g_pInterfaceResMgr->GetYRatio();
	m_pBriefingStr->SetPosition(x,y);
	m_pBriefingStr->SetWrapWidth((uint16)(g_pInterfaceResMgr->GetXRatio() * (float)BriefingWidth));

	//*******************************************************************************
	// Build Mission Help String

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


	if( !IsCoopMultiplayerGameType() && !m_photo.empty() )
	{
		m_hFrame = g_pInterfaceResMgr->GetTexture(m_photo.c_str());
		if (m_hFrame)
		{
			SetupQuadUVs(m_photoPoly, m_hFrame, 0.0f, 0.0f, 1.0f, 0.75f);
			g_pDrawPrim->SetRGBA(&m_photoPoly,argbWhite);

			float fScale = g_pInterfaceResMgr->GetXRatio();
			float fx = (float)m_DefaultPhotoRect.left * fScale;
			float fy = (float)m_DefaultPhotoRect.top * fScale;

			float fw = (float)(m_DefaultPhotoRect.right - m_DefaultPhotoRect.left) * fScale;
			float fh = (float)(m_DefaultPhotoRect.bottom - m_DefaultPhotoRect.top) * fScale;

			g_pDrawPrim->SetXYWH(&m_photoPoly,fx,fy,fw,fh);

		}
	}
	else
	{
		m_hFrame = LTNULL;
	}


	
	CreateInterfaceSFX();

	// Reset the frame counter
	m_nFrameCounter = 0;
	m_fLastFrameTime = CWinUtil::GetTime();
	m_fCurFrameDelta = 0.0f;
	
	m_eCurState = STATE_INIT;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Term()
{
	if (m_eCurState != STATE_INIT)
		return LTFALSE;

	// Clean up
	RemoveInterfaceSFX();

	m_eCurState = STATE_NONE;

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


	return LTTRUE;
}

unsigned long WINAPI CLoadingScreen::ThreadBootstrap(void *pData)
{
	return ((CLoadingScreen*)pData)->RunThread();
}

int CLoadingScreen::RunThread()
{
	// Change state
	m_eCurState = STATE_ACTIVE;

	// Tell the main thread we're now in our main loop
	SetEvent(m_hEventThreadRunning);

	// The main rendering loop...  (i.e. keep drawing until someone tells us to stop)
	while (WaitForSingleObject(m_hEventEnd, 0) == WAIT_TIMEOUT)
	{
		// Draw the frame..
		Update();
		
		// Make sure we're not running faster than 10fps so stuff can still happen in the background
		Sleep(100);
	}
	return 0;
}

LTBOOL CLoadingScreen::Update()
{
	// Make sure we're in a valid state...
	if ((m_eCurState != STATE_ACTIVE) && (m_eCurState != STATE_SHOW))
		return LTFALSE;

	// Only draw the render screen if we have one...

	if( m_pRenderScreen )
	{
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();

		g_pLTClient->Start3D();

		// Draw any FX First, such as the back ground...

		m_pRenderScreen->UpdateInterfaceSFX();
		g_pInterfaceMgr->DrawSFX();
		
		EnterCriticalSection(&m_MissionUpdate);

		g_pLTClient->StartOptimized2D();

		// Actually draw the screen...

		LTBOOL bRet = m_pRenderScreen->Render( hScreen );

		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
		g_pLTClient->FlipScreen(0);

		LeaveCriticalSection(&m_MissionUpdate);

		return bRet;
	}

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER,0);
	// Mmm..  Triple dimensional...
	g_pLTClient->Start3D();

	// Update our interface SFX since the interface mgr doesn't know about us
	UpdateInterfaceSFX();

	// Update the interface mgr
	g_pInterfaceMgr->DrawSFX();


	EnterCriticalSection(&m_MissionUpdate);

    HSURFACE hDestSurf = g_pLTClient->GetScreenSurface();

	// Go into optimized2d so the multiplayer info can draw
	g_pLTClient->StartOptimized2D();

	if (m_pMissionNameStr)
		m_pMissionNameStr->Render();
	if (m_pLevelNameStr)
		m_pLevelNameStr->Render();
	if (m_pBriefingStr)
		m_pBriefingStr->Render();
	if (m_pHelpStr)
		m_pHelpStr->Render();

	if (m_hFrame)
	{
		g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
		g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
		g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
		g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
		g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
		g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
		g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

		g_pDrawPrim->SetTexture(m_hFrame);
		g_pDrawPrim->DrawPrim(&m_photoPoly);
	}


    g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D(END3D_CANDRAWCONSOLE);


    g_pLTClient->FlipScreen(0);

	LeaveCriticalSection(&m_MissionUpdate);

	// Count it..
	++m_nFrameCounter;

	LTFLOAT fCurTime = CWinUtil::GetTime();
	m_fCurFrameDelta = fCurTime - m_fLastFrameTime;
	m_fLastFrameTime = fCurTime;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Show(LTBOOL bRun)
{
	if (bRun && !GetConsoleInt("DynamicLoadScreen",1))
		bRun = LTFALSE;
	// Make sure we're in the correct state
	if (m_eCurState == STATE_NONE)
	{
		if (!Init())
			return LTFALSE;
	}

	if (m_eCurState != STATE_INIT)
		return LTFALSE;

	// Turn off the cursor
	g_pCursorMgr->UseCursor(LTFALSE);
	
	// Set up the FarZ & turn off fog (farz of 0 is bogus)

	m_nOldFarZ = GetConsoleInt("FarZ", 10000);
	m_nOldFarZ = m_nOldFarZ == 0 ? 10000 : m_nOldFarZ;

	m_bOldFogEnable = (LTBOOL) GetConsoleInt("FogEnable", 0);

	g_pGameClientShell->SetFarZ(10000);
	WriteConsoleInt("FogEnable", 0);

	// Make sure we're not in optimized 2D mode (happens sometimes...)
	g_pLTClient->EndOptimized2D();

	
	// Go into the right state..
	m_eCurState = STATE_SHOW;

	// Update once so the screen's showing
	Update();

	// Start updating if they wanted it to..
	if (bRun)
		return Resume();

	// Ok, it's visible or active
	return LTTRUE;
}

LTBOOL CLoadingScreen::Pause()
{
	// Make sure we're in the right state
	if (m_eCurState != STATE_ACTIVE)
		return LTFALSE;

	// Shut down the loading screen thread
	SetEvent(m_hEventEnd);
	WaitForSingleObject(m_hThreadHandle, INFINITE);

	// Ok, it's just visible now..
	m_eCurState = STATE_SHOW;

	return LTTRUE;
}

LTBOOL CLoadingScreen::Resume()
{
	// Ensure our state
	if (m_eCurState != STATE_SHOW)
		return LTFALSE;

	// Reset the events
	ResetEvent(m_hEventEnd);
	ResetEvent(m_hEventThreadRunning);

	// Start up the loading screen thread
	uint32 uThreadID;
	m_hThreadHandle = CreateThread(NULL, 0, ThreadBootstrap, (void *)this, 0, (unsigned long *)&uThreadID);

	// Handle what shouldn't be possible..
	if (!m_hThreadHandle)
		return LTFALSE;

	// Wait for the loading thread to stop touching stuff..
	WaitForSingleObject(m_hEventThreadRunning, INFINITE);

	// Now we're actually active.  (Thank you Mr. Thread..)
	return LTTRUE;
}

LTBOOL CLoadingScreen::Hide()
{
	// Ensure our state
	if (m_eCurState == STATE_ACTIVE)
	{
		// Stop!!!
		if (!Pause())
			return LTFALSE;
	}

	if (m_eCurState != STATE_SHOW)
		return LTFALSE;

	// Clear the screen
//	g_pInterfaceMgr->ClearAllScreenBuffers();

	// Change state
	m_eCurState = STATE_INIT;

	// Clean up
	Term();

	// Re-set the console...
	g_pGameClientShell->SetFarZ(m_nOldFarZ);
	WriteConsoleInt("FogEnable", (int)m_bOldFogEnable);

	// Let the render screen know it's going away...

	if( m_pRenderScreen )
	{
		// Don't lose focus until after setting the state to init...
		
		m_pRenderScreen->OnFocus( LTFALSE );
		m_pRenderScreen = LTNULL;
	}

	// Done!
	return LTTRUE;
}


void CLoadingScreen::CreateLightFX(char *szFXName)
{
	if( m_pRenderScreen )
		return;

	INT_LIGHT* pLight = g_pLayoutMgr->GetLight(szFXName);
	if (pLight)
	{

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE | FLAG_ONLYLIGHTOBJECTS;

        createStruct.m_Pos = m_vPos;

		createStruct.m_Pos += pLight->vPos;;

        HOBJECT hLight = g_pLTClient->CreateObject(&createStruct);

		if (hLight)
		{
			g_pLTClient->SetLightColor(hLight, pLight->vColor.x, pLight->vColor.y, pLight->vColor.z);
			g_pLTClient->SetLightRadius(hLight, pLight->fRadius);

			g_pInterfaceMgr->AddInterfaceLight(hLight);
		}

	}
}

void CLoadingScreen::UpdateMissionInfo()
{
	// No need to update mission info if a different screen is being rendered...
	
	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	if( !g_pMissionMgr->IsCustomLevel( ))
	{
		int nCurMission = g_pMissionMgr->GetCurrentMission( );
		MISSION* pMission = g_pMissionButeMgr->GetMission( nCurMission );

		if( pMission )
		{
			if (pMission->nNameId > 0)
				m_missionname = LoadTempString(pMission->nNameId);
			else
				m_missionname = pMission->sName;

			int nCurLevel = g_pMissionMgr->GetCurrentLevel( );
			
			m_levelname = LoadTempString( pMission->aLevels[nCurLevel].nNameId );

			// Show a briefing for this level if it exists.  Also, only
			// show the briefing if we haven't been to this level before.
			int nBriefingId = pMission->aLevels[nCurLevel].nBriefingId;
			if( nBriefingId >= 0 && !g_pMissionMgr->IsRestoringLevel( ))
				m_briefing = LoadTempString( nBriefingId );
			else
				m_briefing = "";

			// Show help text for this level if it exists.  
			int nHelpId = pMission->aLevels[nCurLevel].nHelpId;
			if( nHelpId >= 0)
				m_help = LoadTempString( nHelpId );
			else
				m_help = "";

			if( !IsCoopMultiplayerGameType( ))
			{
				m_photo = pMission->sPhoto;
			}

		}

	}

	m_pMissionNameStr->SetText(m_missionname.c_str());
	m_pLevelNameStr->SetText(m_levelname.c_str());
	m_pHelpStr->SetText(m_help.c_str());

	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		m_pBriefingStr->SetText(LoadTempString( IDS_PERFORMANCE_TEST_BRIEF ));
	}
	else
	{
		m_pBriefingStr->SetText(m_briefing.c_str());
	}

	if( !IsCoopMultiplayerGameType() && !m_photo.empty())
	{
		m_hFrame = g_pInterfaceResMgr->GetTexture(m_photo.c_str());
		if (m_hFrame)
		{
			SetupQuadUVs(m_photoPoly, m_hFrame, 0.0f, 0.0f, 1.0f, 0.75f);
			g_pDrawPrim->SetRGBA(&m_photoPoly,argbWhite);

			float fScale = g_pInterfaceResMgr->GetXRatio();
			float fx = (float)m_DefaultPhotoRect.left * fScale;
			float fy = (float)m_DefaultPhotoRect.top * fScale;

			float fw = (float)(m_DefaultPhotoRect.right - m_DefaultPhotoRect.left) * fScale;
			float fh = (float)(m_DefaultPhotoRect.bottom - m_DefaultPhotoRect.top) * fScale;

			g_pDrawPrim->SetXYWH(&m_photoPoly,fx,fy,fw,fh);

		}
	}
	else
	{
		m_hFrame = LTNULL;
	}



	LeaveCriticalSection(&m_MissionUpdate);
}

bool CLoadingScreen::NeedsPostLoadScreen() const
{
	// Always go to postload screen if in mp.  We may need to wait there for other players.
	if( IsMultiplayerGame( ))
		return true;

	// Only go to postload if we had a briefing for sp.
	return ( !m_briefing.empty( ));
}