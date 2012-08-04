// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.cpp
//
// PURPOSE : Interface screen for setting performance options
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformance.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionMgr.h"
#include "PerformanceTest.h"
#include "ClientMultiplayerMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
extern VarTrack	g_vtPerformanceTestMode;

namespace
{
	int kGap = 200;
	int kWidth = 0;
	int kNumCfg;

	int nInitTex;
	int nInitEBM;
	int nInitEM;
	int nInitDT;

	int kDlgHt;
	int kDlgWd;

	LTBOOL g_bSettingOverall = LTFALSE;

}

	int nInitTB;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPerformance::CScreenPerformance()
{
	m_pResolution= NULL;
	m_pDisplay = LTNULL;
	m_pSFX = LTNULL;
	m_pDisplayLabel = LTNULL;
	m_pSFXLabel = LTNULL;
	m_pDisplayFrame = LTNULL;
	m_pSFXFrame = LTNULL;

	m_pPerformanceTest = LTNULL;

	memset(&m_sCfg,0,sizeof(m_sCfg));

	m_nTripleBuffer = 1;


}

CScreenPerformance::~CScreenPerformance()
{

}

// Build the screen
LTBOOL CScreenPerformance::Build()
{
	kGap = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PERFORMANCE,"ColumnWidth");
	kWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PERFORMANCE,"SliderWidth");
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);
	uint8 nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PERFORMANCE,"ListFontSize");
	int nListSpacing = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PERFORMANCE,"ListSpacing");

	CreateTitle(IDS_TITLE_PERFORMANCE);

	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_PERFORMANCE,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);


	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"MainRect");
	int nHeight = (rect.bottom - rect.top) + 16;
	int nWidth = (rect.right - rect.left);
	LTIntPt pos = LTIntPt(rect.left-8,rect.top-8);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	pFrame->SetBasePos(pos);
	pFrame->SetBorder(2,m_SelectedColor);
	AddControl(pFrame);

	m_nextPos.x = GetPageLeft();
	m_nextPos.y = GetPageTop();

	m_pPerformance = AddCycle(IDS_PERFORMANCE,IDS_HELP_PERFORMANCE,kGap,&m_nOverall);
	kNumCfg = g_pPerformanceMgr->m_ConfigList.size();

	for (int i = 0; i < kNumCfg; i++)
	{
		m_pPerformance->AddString(g_pPerformanceMgr->m_ConfigList[i]->szNiceName);
	}

	char szTmp[64];
	FormatString(IDS_CUSTOM,szTmp,sizeof(szTmp));
	m_pPerformance->AddString(szTmp);

	m_pDetailLevel = AddCycle(IDS_TEXTURE_RES,IDS_HELP_TEXTURE_RES,kGap,&m_sCfg.nSettings[kPerform_DetailLevel]);
	m_pDetailLevel->AddString(LoadTempString(IDS_LOW));
	m_pDetailLevel->AddString(LoadTempString(IDS_MEDIUM));
	m_pDetailLevel->AddString(LoadTempString(IDS_HIGH));

	m_pPrecache = AddCycle(IDS_PRECACHE,IDS_HELP_PRECACHE,kGap,&m_sCfg.nSettings[kPerform_PreCacheAssets]);
	m_pPrecache->AddString(LoadTempString(IDS_OFF));
	m_pPrecache->AddString(LoadTempString(IDS_ON));

	CLTGUITextCtrl *pCtrl = AddTextItem(IDS_DISPLAY, CMD_DISPLAY, IDS_HELP_ADVDISPLAY);

	pCtrl = AddTextItem(IDS_SFX, CMD_SFX, IDS_HELP_SFX);

	//add a gap before performance test item
	m_nextPos.y += g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);;
	m_pPerformanceTest = AddTextItem(IDS_PERFORMANCE_TEST,CMD_CUSTOM,IDS_HELP_PERFORMANCE_TEST);

	char szOn[32] = "";
	char szOff[32] = "";
	LoadString(IDS_ON,szOn,sizeof(szOn));
	LoadString(IDS_OFF,szOff,sizeof(szOff));

	rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"DisplayRect");
	nHeight = (rect.bottom - rect.top) + 16;
	nWidth = (rect.right - rect.left);
	pos = LTIntPt(rect.left-8,rect.top-8);

	m_pDisplayFrame = debug_new(CLTGUIFrame);
	m_pDisplayFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pDisplayFrame->SetBasePos(pos);
	m_pDisplayFrame->Show(LTFALSE);
	m_pDisplayFrame->SetBorder(2,m_SelectedColor);
	AddControl(m_pDisplayFrame);

	pos.x += 8;
	pos.y += 8;
	
	m_pDisplayLabel = AddTextItem(IDS_DISPLAY, LTNULL, LTNULL, pos, LTTRUE);
	m_pDisplayLabel->Show(LTFALSE);
	rect.top += (nFontSize+8);	

	int listGap = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"DisplayColumn");
	nWidth = (rect.right - rect.left) - 32;
	m_pDisplay = AddList(LTIntPt(rect.left,rect.top),rect.bottom - rect.top, LTTRUE, nWidth);
	if (m_pDisplay)
	{
		m_pDisplay->SetIndent(LTIntPt(8,8));
		m_pDisplay->Show(LTFALSE);
		m_pDisplay->SetItemSpacing(nListSpacing);

		CLTGUICycleCtrl *pCtrl = NULL;

		pCtrl = CreateCycle(IDS_DYNAMICLIGHTS, IDS_HELP_DYNAMICLIGHTS,listGap,&m_sCfg.nSettings[kPerform_DynamicLight]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	

		pCtrl = CreateCycle(IDS_SHADOWS, IDS_HELP_SHADOWS,listGap,&m_sCfg.nSettings[kPerform_ShadowDetail]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(LoadTempString(IDS_LOW));
		pCtrl->AddString(LoadTempString(IDS_MEDIUM));
		pCtrl->AddString(LoadTempString(IDS_HIGH));
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_POLYGRIDBUMP, IDS_HELP_POLYGRIDBUMP,listGap,&m_sCfg.nSettings[kPerform_PolyGridBumpmap]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	
		
		pCtrl = CreateCycle(IDS_POLYGRIDFRES, IDS_HELP_POLYGRIDFRES,listGap,&m_sCfg.nSettings[kPerform_PolyGridFresnel]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	
		
		pCtrl = CreateCycle(IDS_BUMPMAP, IDS_HELP_BUMPMAP,listGap,&m_sCfg.nSettings[kPerform_EnvironmentBumpMapping]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	

		pCtrl = CreateCycle(IDS_ANISOTROPIC, IDS_HELP_ANISOTROPIC,listGap,&m_sCfg.nSettings[kPerform_AnisotropicFiltering]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	
		
		pCtrl = CreateCycle(IDS_TRILINEAR, IDS_HELP_TRILINEAR ,listGap,&m_sCfg.nSettings[kPerform_TrilinearFiltering]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	
		
		pCtrl = CreateCycle(IDS_ENVIRONMENT_MAP, IDS_HELP_ENVIRONMENT_MAP,listGap,&m_sCfg.nSettings[kPerform_EnvironmentMapping]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	

		pCtrl = CreateCycle(IDS_DETAILTEXTURES, IDS_HELP_DETAILTEXTURES,listGap,&m_sCfg.nSettings[kPerform_DetailTextures]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	

		pCtrl = CreateCycle(IDS_TRIPLE_BUFF, IDS_HELP_TRIPLE_BUFF,listGap, &m_nTripleBuffer);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pDisplay->AddControl(pCtrl);	

		uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
		pCtrl->Enable( (dwAdvancedOptions & AO_TRIPLEBUFFER) );

	}

	
	rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SFXRect");

	nHeight = (rect.bottom - rect.top) + 16;
	nWidth = (rect.right - rect.left);
	pos = LTIntPt(rect.left-8,rect.top-8);

	m_pSFXFrame = debug_new(CLTGUIFrame);
	m_pSFXFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pSFXFrame->SetBasePos(pos);
	m_pSFXFrame->Show(LTFALSE);
	m_pSFXFrame->SetBorder(2,m_SelectedColor);
	AddControl(m_pSFXFrame);

	pos.x += 8;
	pos.y += 8;

	m_pSFXLabel = AddTextItem(IDS_SFX, LTNULL, LTNULL, LTIntPt(rect.left,rect.top), LTTRUE);
	m_pSFXLabel->Show(LTFALSE);
	rect.top += (nFontSize+8);	
	
	listGap = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"SFXColumn");
	nWidth = (rect.right - rect.left) - 32;
	m_pSFX = AddList(LTIntPt(rect.left,rect.top),rect.bottom - rect.top, LTTRUE, nWidth);
	if (m_pSFX)
	{
		m_pSFX->SetIndent(LTIntPt(8,8));
		m_pSFX->Show(LTFALSE);
		m_pSFX->SetItemSpacing(nListSpacing);

		CLTGUICycleCtrl *pCtrl = CreateCycle(IDS_TRACERS, IDS_HELP_TRACERS, listGap, &m_sCfg.nSettings[kPerform_Tracers]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_SHELLCASINGS, IDS_HELP_SHELLCASINGS, listGap, &m_sCfg.nSettings[kPerform_ShellCasings]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_ENV_DETAIL, IDS_HELP_ENV_DETAIL, listGap, &m_sCfg.nSettings[kPerform_EnvironmentalDetail]);
		pCtrl->AddString(LoadTempString(IDS_OFF));
		pCtrl->AddString(LoadTempString(IDS_LOW));
		pCtrl->AddString(LoadTempString(IDS_MEDIUM));
		pCtrl->AddString(LoadTempString(IDS_HIGH));
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_FX_DETAIL, IDS_HELP_FX_DETAIL, listGap, &m_sCfg.nSettings[kPerform_FXDetail]);
		pCtrl->AddString(LoadTempString(IDS_LOW));
		pCtrl->AddString(LoadTempString(IDS_MEDIUM));
		pCtrl->AddString(LoadTempString(IDS_HIGH));
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

	}


	pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ResolutionPos");
	m_pResolution = CreateTextItem(IDS_RES_DIFFERS,0,0,pos,LTTRUE);
	m_pResolution->SetFont(NULL,nListFontSize);
	m_pResolution->SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
	AddControl(m_pResolution);



	
	LTIntPt dlgPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"DialogPos");
	LTIntPt dlgSz = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"DialogSize");
	kDlgHt = dlgSz.y;
	kDlgWd = dlgSz.x;

	char szBack[128] = "";
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"DialogFrame",szBack,sizeof(szBack));

	m_pDlg = debug_new(CLTGUIWindow);
	m_pDlg->Create(g_pInterfaceResMgr->GetTexture(szBack), kDlgWd, kDlgHt);
	m_pDlg->SetBasePos(dlgPos);

	LTIntPt tmp(60,8);
	pCtrl = CreateTextItem(IDS_PERFORMANCE_RESULTS, LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(pCtrl, tmp);

#ifdef _SHOW_PERFORMACE_FRAMERATE_

	tmp.x = 10;
	tmp.y += (pCtrl->GetBaseHeight() + 18);
	m_pMaxFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pMaxFPS, tmp);

	tmp.y += (pCtrl->GetBaseHeight() + 2);
	m_pAveFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pAveFPS, tmp);

	tmp.y += (pCtrl->GetBaseHeight() + 2);
	m_pMinFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pMinFPS, tmp);

	tmp.y += (pCtrl->GetBaseHeight() + 16);
	m_pAboveMaxFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pAboveMaxFPS, tmp);
	
	tmp.y += (pCtrl->GetBaseHeight() + 2);
	m_pMintoMaxFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pMintoMaxFPS, tmp);

	tmp.y += (pCtrl->GetBaseHeight() + 2);
	m_pBelowMinFPS = CreateTextItem("<>", LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pDlg->AddControl(m_pBelowMinFPS, tmp);

#else

	const int nXOffset = 18;
	tmp.x = nXOffset;
	tmp.y += (pCtrl->GetBaseHeight() + 10);
	m_pRecommendation = CreateTextItem(FormatTempString(IDS_PERFORMANCE_RECOMMEND_LOWER), LTNULL, LTNULL, kDefaultPos, LTTRUE);
	m_pRecommendation->SetFixedWidth(kDlgWd - (2*nXOffset));
	m_pDlg->AddControl(m_pRecommendation, tmp);

#endif // _SHOW_PERFORMACE_FRAMERATE_

	pCtrl = CreateTextItem(IDS_OK, CMD_OK, LTNULL);
	tmp.y = kDlgHt - pCtrl->GetBaseHeight();
	tmp.x = (kDlgWd - pCtrl->GetBaseWidth()) / 2;
	m_pDlg->AddControl(pCtrl, tmp);

	m_pDlg->Show(LTFALSE);

	AddControl(m_pDlg);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenPerformance::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_OK:
		m_pDlg->Show(LTFALSE);
		SetCapture(NULL);
		break;

	case CMD_BACK:
		m_pDisplay->Show(LTFALSE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pDisplayFrame->Show(LTFALSE);

		m_pSFX->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);

		m_pDisplay->SetSelection(kNoSelection);
		m_pSFX->SetSelection(kNoSelection);
		m_pScreenMgr->EscapeCurrentScreen();
		break;
	case CMD_CUSTOM:
		m_pDisplay->Show(LTFALSE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pDisplayFrame->Show(LTFALSE);

		m_pSFX->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);

		m_pDisplay->SetSelection(kNoSelection);
		m_pSFX->SetSelection(kNoSelection);

		g_pGameClientShell->StartPerformanceTest();
		if (!g_pMissionMgr->StartPerformanceLevel())
		{
			_ASSERT(!"Performace Test Level failed!");
			g_pGameClientShell->StopPerformanceTest();
		}

		break;
	case CMD_DISPLAY:
	{
		m_pSFX->Show(LTFALSE);
		m_pDisplay->Show(LTTRUE);
		m_pSFXLabel->Show(LTFALSE);
		m_pDisplayLabel->Show(LTTRUE);
		m_pSFXFrame->Show(LTFALSE);
		m_pDisplayFrame->Show(LTTRUE);

		m_pDisplay->SetSelection(0);
		SetSelection(GetIndex(m_pDisplay));
		
	}	break;
	case CMD_SFX:
	{
		m_pDisplay->Show(LTFALSE);
		m_pSFX->Show(LTTRUE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pSFXLabel->Show(LTTRUE);
		m_pDisplayFrame->Show(LTFALSE);
		m_pSFXFrame->Show(LTTRUE);

		m_pSFX->SetSelection(0);
		SetSelection(GetIndex(m_pSFX));
		
	}	break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Screen specific rendering
LTBOOL   CScreenPerformance::Render(HSURFACE hDestSurf)
{
	// Dumb, but need to make sure this is enabled/disabled
	// correctly and there are a lot of timing issues with
	// when the client is actually connected/disconnected
	if (m_pPerformanceTest)
	{
#ifdef _TO2DEMO
		m_pPerformanceTest->Enable(LTFALSE);
#else
		m_pPerformanceTest->Enable(!g_pLTClient->IsConnected());
#endif // _TO2DEMO
	}

	return CBaseScreen::Render(hDestSurf);
}
// Change in focus
void    CScreenPerformance::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		if (g_pGameClientShell->IsRunningPerformanceTest())
		{
			// Make sure we're disconnected from server.
			if(g_pLTClient->IsConnected())
			{
				g_pInterfaceMgr->SetIntentionalDisconnect( true );
				g_pClientMultiplayerMgr->ForceDisconnect();
			}


			g_pGameClientShell->StopPerformanceTest();

			CPerformanceTest* pTest = g_pGameClientShell->GetLastPerformanceTest();
			if (pTest)
			{

#ifdef _SHOW_PERFORMACE_FRAMERATE_

				m_pMinFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_MIN, pTest->GetMinFPS()));
				m_pMaxFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_MAX, pTest->GetMaxFPS()));
				m_pAveFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_AVE, pTest->GetAveFPS()));

				m_pBelowMinFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_BELOW_MIN, 
					pTest->GetPercentFPSBelowMin(), pTest->GetMinTestFPS()));
				m_pMintoMaxFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_MINTOMAX, 
					pTest->GetPercentFPSMintoMax(), pTest->GetMinTestFPS(), pTest->GetMaxTestFPS()));
				m_pAboveMaxFPS->SetString(FormatTempString(IDS_PERFORMANCE_FPS_ABOVE_MAX, 
					pTest->GetPercentFPSAboveMax(), pTest->GetMaxTestFPS()));

#else

				int nChange = pTest->GetRecommendedDetailChange();
				if (nChange < 0)
				{
					m_pRecommendation->SetString(FormatTempString(IDS_PERFORMANCE_RECOMMEND_LOWER));
				}
				else if (nChange > 0)
				{
					m_pRecommendation->SetString(FormatTempString(IDS_PERFORMANCE_RECOMMEND_HIGHER));
				}
				else
				{
					m_pRecommendation->SetString(FormatTempString(IDS_PERFORMANCE_RECOMMEND_CURRENT));
				}

#endif // _SHOW_PERFORMACE_FRAMERATE_

				m_pDlg->Show(LTTRUE);
			}
		}

		pProfile->SetPerformance();

		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg(true);
		m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
		g_bSettingOverall = LTTRUE;
		m_sCfg = pProfile->m_sPerformance;

		nInitTex = pProfile->m_sPerformance.nSettings[kPerform_DetailLevel];
		nInitEBM = pProfile->m_sPerformance.nSettings[kPerform_EnvironmentBumpMapping];
		nInitEM = pProfile->m_sPerformance.nSettings[kPerform_EnvironmentMapping];
		nInitDT = pProfile->m_sPerformance.nSettings[kPerform_DetailTextures];
		nInitTB = pProfile->m_sPerformance.nSettings[kPerform_TripleBuffering];

        UpdateData(LTFALSE);
		g_bSettingOverall = LTFALSE;
	}
	else
	{
		
		UpdateData();

		g_pPerformanceMgr->SetPerformanceCfg(m_nOverall);
		if (m_nOverall < g_pPerformanceMgr->m_ConfigList.size())
		{
			pProfile->SetPerformance();
		}
		else
		{
			pProfile->m_sPerformance = m_sCfg;
		}
		
		bool bRebind = (nInitTex != pProfile->m_sPerformance.nSettings[kPerform_DetailLevel]);
		bool bRestart = (nInitTB != pProfile->m_sPerformance.nSettings[kPerform_TripleBuffering]) ||
			 (nInitEBM != pProfile->m_sPerformance.nSettings[kPerform_EnvironmentBumpMapping]) ||
			 (nInitEM != pProfile->m_sPerformance.nSettings[kPerform_EnvironmentMapping]) ||
			 (nInitDT != pProfile->m_sPerformance.nSettings[kPerform_DetailTextures]);

//		if (bRestart)
//			g_pLTClient->CPrint("CScreenPerformance::OnFocus() : restart because of setting change");


		//no need to restart in these cases because the call to ApplyPerformance() will restart the renderer
		switch (m_nOverall)
		{
		case 0:
			if (pProfile->m_nScreenWidth != 640)
			{
//				if (bRestart)
//					g_pLTClient->CPrint("CScreenPerformance::OnFocus() : cancel restart because of resolution change");
				bRestart = false;
			}
			break;
		case 1:

			if (pProfile->m_nScreenWidth != 800)
			{
//				if (bRestart)
//					g_pLTClient->CPrint("CScreenPerformance::OnFocus() : cancel restart because of resolution change");
				bRestart = false;
			}

			break;

		case 2:
			if (pProfile->m_nScreenWidth < 1024)
			{
//				if (bRestart)
//					g_pLTClient->CPrint("CScreenPerformance::OnFocus() : cancel restart because of resolution change");
				bRestart = false;
			}
			break;

		}


		if ( bRestart )

		{
			// Set the renderer mode
			g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
//			g_pLTClient->CPrint("CScreenPerformance::OnFocus() : restart");
			g_pLTClient->RunConsoleString("RestartRender");
			g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);
		}

		if (bRebind)
		{
			g_pInterfaceResMgr->DrawMessage(IDS_REBINDING_TEXTURES);
			g_pLTClient->RunConsoleString("RebindTextures");
		}

		pProfile->ApplyPerformance(true);


		pProfile->Save();


		//force this because restarting the renderer may show the hardware cursor
		g_pCursorMgr->UseHardwareCursor(pProfile->m_bHardwareCursor,true);


		m_pDisplay->Show(LTFALSE);
		m_pSFX->Show(LTFALSE);

		m_pDisplayLabel->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);

		m_pDisplayFrame->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);

		m_pDisplay->SetSelection(kNoSelection);
		m_pSFX->SetSelection(kNoSelection);

	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
	{
		if (m_pDlg->IsVisible())
		{
			SetCapture(m_pDlg);
			SetSelection( GetIndex(m_pDlg) );
		}
	}
}


LTBOOL CScreenPerformance::OnLeft()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int nOverall = m_pPerformance->GetSelIndex();
		--nOverall;
		if (nOverall < 0)
			nOverall = kNumCfg-1;
		m_pPerformance->SetSelIndex(nOverall);
		m_nOverall = nOverall;
		if (m_nOverall != kNumCfg)
		{
			g_pPerformanceMgr->SetPerformanceCfg(m_nOverall);
			g_pPerformanceMgr->GetPerformanceOptions(&m_sCfg);
			m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
			UpdateData(LTFALSE);
		}
        return LTTRUE;
	}

	LTBOOL bHandled = CBaseScreen::OnLeft();
	if (bHandled)
	{
		UpdateData(LTTRUE);

		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg(true);
		m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));

		UpdateData(LTFALSE);

	}
	return bHandled;
}

LTBOOL CScreenPerformance::OnRight()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int nOverall = m_pPerformance->GetSelIndex();
		++nOverall;
		if (nOverall > kNumCfg-1)
			nOverall = 0;
		m_pPerformance->SetSelIndex(nOverall);
		m_nOverall = nOverall;
		if (m_nOverall != kNumCfg)
		{
			g_pPerformanceMgr->SetPerformanceCfg(m_nOverall);
			g_pPerformanceMgr->GetPerformanceOptions(&m_sCfg);
			m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
			UpdateData(LTFALSE);
		}

		return LTTRUE;
	}
	LTBOOL bHandled = CBaseScreen::OnRight();
	if (bHandled)
	{
		UpdateData(LTTRUE);

		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg(true);
		m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
		UpdateData(LTFALSE);
	}
	return bHandled;
	
}


LTBOOL CScreenPerformance::OnLButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance || pCtrl == m_pDetailLevel || pCtrl == m_pPrecache)
		{

			m_pDisplay->Show(LTFALSE);
			m_pDisplay->SetSelection(kNoSelection);

			m_pSFX->Show(LTFALSE);
			m_pSFX->SetSelection(kNoSelection);

			m_pDisplayFrame->Show(LTFALSE);
			m_pSFXFrame->Show(LTFALSE);

			m_pDisplayLabel->Show(LTFALSE);
			m_pSFXLabel->Show(LTFALSE);


			return OnRight();
		}
	}
	LTBOOL bHandled = CBaseScreen::OnLButtonUp(x, y);
	if (bHandled)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pDisplay ||
			pCtrl == m_pSFX)
		{
			UpdateData(LTTRUE);

			g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
			m_nOverall = g_pPerformanceMgr->GetPerformanceCfg(true);
			m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
			UpdateData(LTFALSE);
		}
	}
	return bHandled;

}

LTBOOL CScreenPerformance::OnRButtonUp(int x, int y)
{
	uint16 nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance || pCtrl == m_pDetailLevel || pCtrl == m_pPrecache)
		{

			m_pDisplay->Show(LTFALSE);
			m_pDisplay->SetSelection(kNoSelection);
			m_pSFX->Show(LTFALSE);
			m_pSFX->SetSelection(kNoSelection);

			m_pDisplayLabel->Show(LTFALSE);
			m_pSFXLabel->Show(LTFALSE);

			m_pDisplayFrame->Show(LTFALSE);
			m_pSFXFrame->Show(LTFALSE);

			m_pDisplayLabel->Show(LTFALSE);
			m_pSFXLabel->Show(LTFALSE);

			return OnLeft();
		}
	}
	LTBOOL bHandled = CBaseScreen::OnRButtonUp(x, y);
	if (bHandled)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pDisplay ||
			pCtrl == m_pSFX )
		{
			UpdateData(LTTRUE);

			g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
			m_nOverall = g_pPerformanceMgr->GetPerformanceCfg(true);
			m_pResolution->Show(m_nOverall < 3 && m_nOverall != g_pPerformanceMgr->GetPerformanceCfg(false));
			UpdateData(LTFALSE);
		}

	}
	return bHandled;
}


void CScreenPerformance::Escape()
{

	if (m_pDlg->IsVisible())
	{
		m_pDlg->Show(LTFALSE);
		SetCapture(NULL);
	}
	else if (m_pDisplay->IsVisible())
	{
		m_pDisplay->Show(LTFALSE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pDisplayFrame->Show(LTFALSE);
		SetSelection(1);
	}
	else if (m_pSFX->IsVisible())
	{
		m_pSFX->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);
		SetSelection(2);
	}
	else
	{
		CBaseScreen::Escape();
	}
}

// Calls UpdateData on each control in the screen
void CScreenPerformance::UpdateData(LTBOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		//the TripleBuffering setting will be 1 or 2, whereas the control needs it to be 0 or 1
		m_nTripleBuffer = m_sCfg.nSettings[kPerform_TripleBuffering]-1;
	}

	CBaseScreen::UpdateData(bSaveAndValidate);

	if (bSaveAndValidate)
	{
		//the TripleBuffering setting will be 1 or 2, whereas the control needs it to be 0 or 1
		m_sCfg.nSettings[kPerform_TripleBuffering] = m_nTripleBuffer+1;
	}

}
