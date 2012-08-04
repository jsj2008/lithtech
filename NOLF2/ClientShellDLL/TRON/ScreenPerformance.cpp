// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPerformance.cpp
//
// PURPOSE : Interface screen for setting performance options
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPerformance.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kGap = 200;
	int kWidth = 0;
	int kNumCfg;

	int nInitVal[6];
	int nInitTB;
	int nInitEBM;
	int nInitEM;
	int nInitDT;


	LTBOOL g_bSettingOverall = LTFALSE;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPerformance::CScreenPerformance()
{
	m_pDisplay = LTNULL;
	m_pSFX = LTNULL;
	m_pTexture = LTNULL;
	m_pDisplayLabel = LTNULL;
	m_pSFXLabel = LTNULL;
	m_pTextureLabel = LTNULL;
	m_pDisplayFrame = LTNULL;
	m_pSFXFrame = LTNULL;
	m_pTextureFrame = LTNULL;

	memset(&m_sCfg,0,sizeof(m_sCfg));


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

	m_pPerformance = AddCycle(IDS_PERFORMANCE,IDS_HELP_PERFORMANCE,kGap,&m_nOverall);
	kNumCfg = g_pPerformanceMgr->m_ConfigList.size();

	for (int i = 0; i < kNumCfg; i++)
	{
		m_pPerformance->AddString(g_pPerformanceMgr->m_ConfigList[i]->szNiceName);
	}

	char szTmp[64];
	FormatString(IDS_CUSTOM,szTmp,sizeof(szTmp));
	m_pPerformance->AddString(szTmp);


	CLTGUITextCtrl *pCtrl = AddTextItem(IDS_DISPLAY, CMD_DISPLAY, IDS_HELP_ADVDISPLAY);

	pCtrl = AddTextItem(IDS_SFX, CMD_SFX, IDS_HELP_SFX);

	pCtrl = AddTextItem(IDS_TEXTURE_RES, CMD_TEXTURE, IDS_HELP_TEXTURE_RES);

	char szOn[32] = "";
	char szOff[32] = "";
	LoadString(IDS_ON,szOn,sizeof(szOn));
	LoadString(IDS_OFF,szOff,sizeof(szOff));
	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_PERFORMANCE,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);


	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"DisplayRect");
	int nHeight = (rect.bottom - rect.top) + 16;
	int nWidth = (rect.right - rect.left);
	LTIntPt pos = LTIntPt(rect.left-8,rect.top-8);

	m_pDisplayFrame = debug_new(CLTGUIFrame);
	m_pDisplayFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pDisplayFrame->SetBasePos(pos);
	m_pDisplayFrame->Show(LTFALSE);
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
		m_pDisplay->SetFrameWidth(2);
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

		pCtrl = CreateCycle(IDS_TRIPLE_BUFF, IDS_HELP_TRIPLE_BUFF,listGap, &m_sCfg.nSettings[kPerform_TripleBuffering]);
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
		m_pSFX->SetFrameWidth(2);
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

		pCtrl = CreateCycle(IDS_MUZZLELIGHT, IDS_HELP_MUZZLELIGHT, listGap, &m_sCfg.nSettings[kPerform_MuzzleLight]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_WEATHER, IDS_HELP_WEATHER, listGap, &m_sCfg.nSettings[kPerform_WeatherFX]);
		pCtrl->AddString(szOff);
		pCtrl->AddString(szOn);
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_IMPACT,IDS_HELP_IMPACTS, listGap, &m_sCfg.nSettings[kPerform_ImpactFX]);
		pCtrl->AddString(LoadTempString(IDS_LOW));
		pCtrl->AddString(LoadTempString(IDS_MEDIUM));
		pCtrl->AddString(LoadTempString(IDS_HIGH));
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);

		pCtrl = CreateCycle(IDS_DEBRIS,IDS_HELP_DEBRIS, listGap, &m_sCfg.nSettings[kPerform_DebrisFX]);
		pCtrl->AddString(LoadTempString(IDS_LOW));
		pCtrl->AddString(LoadTempString(IDS_MEDIUM));
		pCtrl->AddString(LoadTempString(IDS_HIGH));
		pCtrl->SetFont(NULL,nListFontSize);
		m_pSFX->AddControl(pCtrl);
	}

	rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"TextureRect");

	nHeight = (rect.bottom - rect.top) + 16;
	nWidth = (rect.right - rect.left);
	pos = LTIntPt(rect.left-8,rect.top-8);

	m_pTextureFrame = debug_new(CLTGUIFrame);
	m_pTextureFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pTextureFrame->SetBasePos(pos);
	m_pTextureFrame->Show(LTFALSE);
	AddControl(m_pTextureFrame);

	pos.x += 8;
	pos.y += 8;

	m_pTextureLabel = AddTextItem(IDS_TEXTURE_RES, LTNULL, LTNULL, LTIntPt(rect.left,rect.top), LTTRUE);
	m_pTextureLabel->Show(LTFALSE);
	rect.top += (nFontSize+8);	

	listGap = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"TextureColumn");
	nWidth = (rect.right - rect.left) - 32;
	int sliderWidth = g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"TextureSlider");
	m_pTexture = AddList(LTIntPt(rect.left,rect.top),rect.bottom - rect.top, LTTRUE, nWidth);
	if (m_pTexture)
	{
		m_pTexture->SetIndent(LTIntPt(8,8));
		m_pTexture->SetFrameWidth(2);
		m_pTexture->Show(LTFALSE);
		m_pTexture->SetItemSpacing(nListSpacing);

		m_pDetailLevel = CreateCycle(IDS_DETAILLEVEL,IDS_HELP_DETAILLEVEL,listGap,&m_sCfg.nSettings[kPerform_DetailLevel]);
		m_pDetailLevel->AddString(LoadTempString(IDS_LOW));
		m_pDetailLevel->AddString(LoadTempString(IDS_MEDIUM));
		m_pDetailLevel->AddString(LoadTempString(IDS_HIGH));
		m_pDetailLevel->AddString(LoadTempString(IDS_CUSTOMIZED));
		m_pDetailLevel->SetFont(NULL,nListFontSize);
		m_pTexture->AddControl(m_pDetailLevel);

		m_pTexGroup[0]=CreateSlider(IDS_TEXTURE_WORLD, IDS_HELP_TEXTURE_WORLD, listGap, sliderWidth, -1, &m_nSliderVal[0]);
		m_pTexGroup[1]=CreateSlider(IDS_TEXTURE_WEAPONS, IDS_HELP_TEXTURE_WEAPONS, listGap, sliderWidth, -1, &m_nSliderVal[1]);
		m_pTexGroup[2]=CreateSlider(IDS_TEXTURE_CHARS, IDS_HELP_TEXTURE_CHARS, listGap, sliderWidth, -1, &m_nSliderVal[2]);
		m_pTexGroup[3]=CreateSlider(IDS_TEXTURE_PROPS, IDS_HELP_TEXTURE_PROPS, listGap, sliderWidth, -1, &m_nSliderVal[3]);
		m_pTexGroup[4]=CreateSlider(IDS_TEXTURE_SFX, IDS_HELP_TEXTURE_SFX, listGap, sliderWidth, -1, &m_nSliderVal[4]);
		m_pTexGroup[5]=CreateSlider(IDS_TEXTURE_SKY, IDS_HELP_TEXTURE_SKY, listGap, sliderWidth, -1, &m_nSliderVal[5]);

		for (int i = 0; i < 6; i++)
		{
			m_pTexGroup[i]->SetSliderRange(0,5);
			m_pTexGroup[i]->SetSliderIncrement(1);
			m_pTexGroup[i]->SetFont(NULL,nListFontSize);
			m_pTexture->AddControl(m_pTexGroup[i]);
		}

	}
	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenPerformance::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		m_pDisplay->Show(LTFALSE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pDisplayFrame->Show(LTFALSE);

		m_pSFX->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);

		m_pTexture->Show(LTFALSE);
		m_pTextureLabel->Show(LTFALSE);
		m_pTextureFrame->Show(LTFALSE);


		m_pDisplay->SetSelection(kNoSelection);
		m_pSFX->SetSelection(kNoSelection);
		m_pScreenMgr->EscapeCurrentScreen();
		break;
	case CMD_DISPLAY:
	{
		m_pSFX->Show(LTFALSE);
		m_pDisplay->Show(LTTRUE);
		m_pTexture->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pDisplayLabel->Show(LTTRUE);
		m_pTextureLabel->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);
		m_pDisplayFrame->Show(LTTRUE);
		m_pTextureFrame->Show(LTFALSE);

		SetSelection(GetIndex(m_pDisplay));
		m_pDisplay->SetSelection(1);
	}	break;
	case CMD_SFX:
	{
		m_pDisplay->Show(LTFALSE);
		m_pSFX->Show(LTTRUE);
		m_pTexture->Show(LTFALSE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pSFXLabel->Show(LTTRUE);
		m_pTextureLabel->Show(LTFALSE);
		m_pDisplayFrame->Show(LTFALSE);
		m_pSFXFrame->Show(LTTRUE);
		m_pTextureFrame->Show(LTFALSE);

		SetSelection(GetIndex(m_pSFX));
		m_pSFX->SetSelection(1);
	}	break;
	case CMD_TEXTURE:
	{
		m_pDisplay->Show(LTFALSE);
		m_pSFX->Show(LTFALSE);
		m_pTexture->Show(LTTRUE);
		m_pDisplayLabel->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pTextureLabel->Show(LTTRUE);
		m_pDisplayFrame->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);
		m_pTextureFrame->Show(LTTRUE);

		SetSelection(GetIndex(m_pTexture));
		m_pTexture->SetSelection(1);
	}	break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenPerformance::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (bFocus)
	{
		pProfile->SetPerformance();

		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
		g_bSettingOverall = LTTRUE;
		m_sCfg = pProfile->m_sPerformance;

		
		for (int i = 0; i < kNumDetailLevels; i++)
		{
			nInitVal[i] = pProfile->m_sPerformance.nDetails[i];
		}
		nInitEBM = pProfile->m_sPerformance.nSettings[kPerform_EnvironmentBumpMapping];
		nInitEM = pProfile->m_sPerformance.nSettings[kPerform_EnvironmentMapping];
		nInitDT = pProfile->m_sPerformance.nSettings[kPerform_DetailTextures];
		nInitTB = pProfile->m_sPerformance.nSettings[kPerform_TripleBuffering];

        UpdateData(LTFALSE);
		g_bSettingOverall = LTFALSE;
	}
	else
	{
		LTBOOL bRebind = LTFALSE;
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
		

		for (int i = 0; i < kNumDetailLevels; i++)
		{
			if (nInitVal[i] != pProfile->m_sPerformance.nDetails[i])
			{
				bRebind = LTTRUE;
			}
		}
		if ( (nInitTB != pProfile->m_sPerformance.nSettings[kPerform_TripleBuffering]) ||
			 (nInitEBM != pProfile->m_sPerformance.nSettings[kPerform_EnvironmentBumpMapping]) ||
			 (nInitEM != pProfile->m_sPerformance.nSettings[kPerform_EnvironmentMapping]) ||
			 (nInitDT != pProfile->m_sPerformance.nSettings[kPerform_DetailTextures])
			 )

		{
			// Set the renderer mode
			g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
			g_pLTClient->RunConsoleString("RestartRender");
			g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);

			//no need to rebind if we restarted renderer
			bRebind = LTFALSE;
		}

		if (bRebind)
		{
			g_pInterfaceResMgr->DrawMessage(IDS_REBINDING_TEXTURES);
			g_pLTClient->RunConsoleString("RebindTextures");
		}


		pProfile->ApplyPerformance();
		pProfile->Save();

		m_pDisplay->Show(LTFALSE);
		m_pSFX->Show(LTFALSE);
		m_pTexture->Show(LTFALSE);

		m_pDisplayLabel->Show(LTFALSE);
		m_pSFXLabel->Show(LTFALSE);
		m_pTextureLabel->Show(LTFALSE);

		m_pDisplayFrame->Show(LTFALSE);
		m_pSFXFrame->Show(LTFALSE);
		m_pTextureFrame->Show(LTFALSE);

		m_pDisplay->SetSelection(kNoSelection);
		m_pSFX->SetSelection(kNoSelection);
		m_pTexture->SetSelection(kNoSelection);

	}
	CBaseScreen::OnFocus(bFocus);
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
			UpdateData(LTFALSE);
		}
        return LTTRUE;
	}
	if (GetSelectedControl() == m_pTexture)
	{
		if (m_pTexture->GetSelectedControl() == m_pDetailLevel)
		{
			int nDetail = m_pDetailLevel->GetSelIndex();
			--nDetail;
			if (nDetail < 0)
				nDetail = 2;
			m_pPerformance->SetSelIndex(nDetail);
			m_sCfg.nSettings[kPerform_DetailLevel] = nDetail;

			int nDetails[6];
			g_pPerformanceMgr->SetDetailLevels(nDetail,nDetails);
			for (int i = 0; i < 6; i++)
			{
				m_sCfg.nDetails[i] = nDetails[i];
			}

		}
		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
		UpdateData(LTFALSE);
        return LTTRUE;
	}

	LTBOOL bHandled = CBaseScreen::OnLeft();
	if (bHandled)
	{
		UpdateData(LTTRUE);
		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
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
			UpdateData(LTFALSE);
		}

		return LTTRUE;
	}
	if (GetSelectedControl() == m_pTexture)
	{
		if (m_pTexture->GetSelectedControl() == m_pDetailLevel)
		{
			int nDetail = m_pDetailLevel->GetSelIndex();
			++nDetail;
			if (nDetail > 2)
				nDetail = 0;
			m_pPerformance->SetSelIndex(nDetail);
			m_sCfg.nSettings[kPerform_DetailLevel] = nDetail;

			int nDetails[6];
			g_pPerformanceMgr->SetDetailLevels(nDetail,nDetails);
			for (int i = 0; i < 6; i++)
			{
				m_sCfg.nDetails[i] = nDetails[i];
			}

		}
		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
		UpdateData(LTFALSE);
        return LTTRUE;
	}
	LTBOOL bHandled = CBaseScreen::OnRight();
	if (bHandled)
	{
		UpdateData(LTTRUE);
		g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
		m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
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
		if (pCtrl == m_pPerformance )
		{

			m_pDisplay->Show(LTFALSE);
			m_pDisplay->SetSelection(kNoSelection);
			m_pSFX->Show(LTFALSE);
			m_pSFX->SetSelection(kNoSelection);
			m_pTexture->Show(LTFALSE);
			m_pTexture->SetSelection(kNoSelection);

			m_pDisplayFrame->Show(LTFALSE);
			m_pSFXFrame->Show(LTFALSE);
			m_pTextureFrame->Show(LTFALSE);


			return OnRight();
		}
		if (pCtrl == m_pTexture && m_pTexture->GetSelectedControl() == m_pDetailLevel)
		{
			return OnRight();
		}
	}
	LTBOOL bHandled = CBaseScreen::OnLButtonUp(x, y);
	if (bHandled)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pDisplay ||
			pCtrl == m_pSFX ||
			pCtrl == m_pTexture)
		{
			UpdateData(LTTRUE);
			g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
			m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
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
		if (pCtrl == m_pPerformance )
		{

			m_pDisplay->Show(LTFALSE);
			m_pDisplay->SetSelection(kNoSelection);
			m_pSFX->Show(LTFALSE);
			m_pSFX->SetSelection(kNoSelection);
			m_pTexture->Show(LTFALSE);
			m_pTexture->SetSelection(kNoSelection);

			m_pDisplayLabel->Show(LTFALSE);
			m_pSFXLabel->Show(LTFALSE);
			m_pTextureLabel->Show(LTFALSE);

			m_pDisplayFrame->Show(LTFALSE);
			m_pSFXFrame->Show(LTFALSE);
			m_pTextureFrame->Show(LTFALSE);

			return OnLeft();
		}
		if (pCtrl == m_pTexture && m_pTexture->GetSelectedControl() == m_pDetailLevel)
		{
			return OnLeft();
		}
	}
	LTBOOL bHandled = CBaseScreen::OnRButtonUp(x, y);
	if (bHandled)
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pDisplay ||
			pCtrl == m_pSFX ||
			pCtrl == m_pTexture)
		{
			UpdateData(LTTRUE);
			g_pPerformanceMgr->SetPerformanceOptions(&m_sCfg);
			m_nOverall = g_pPerformanceMgr->GetPerformanceCfg();
			UpdateData(LTFALSE);
		}

	}
	return bHandled;
}


void CScreenPerformance::Escape()
{
	if (m_pDisplay->IsVisible())
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
	else if (m_pTexture->IsVisible())
	{
		m_pTexture->Show(LTFALSE);
		m_pTextureLabel->Show(LTFALSE);
		m_pTextureFrame->Show(LTFALSE);
		SetSelection(3);
	}
	else
	{
		CBaseScreen::Escape();
	}
}

// Calls UpdateData on each control in the screen
void CScreenPerformance::UpdateData(LTBOOL bSaveAndValidate)
{
	

	if (bSaveAndValidate)
	{
		CBaseScreen::UpdateData(bSaveAndValidate);
		int nDetails[6];
		for (int i = 0; i < 6; i++)
		{
			nDetails[i] = 3 - m_nSliderVal[i];
			m_sCfg.nDetails[i] = nDetails[i];
		}
		m_sCfg.nSettings[kPerform_DetailLevel] = g_pPerformanceMgr->GetDetailLevel(nDetails);
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			m_nSliderVal[i] = 3 - m_sCfg.nDetails[i];
		}
		CBaseScreen::UpdateData(bSaveAndValidate);
	}


}
