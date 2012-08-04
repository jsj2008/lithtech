// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDownload.cpp
//
// PURPOSE : Interface screen to set server download options
//
// CREATED : 11/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenHostDownload.h"
#include "ScreenCommands.h"
#include "GameModeMgr.h"
#include "CustomCtrls.h"

static const uint32 nMaxDL[] = 
{
	(1<<10),
	(1<<13),
	(1<<16),
	(1<<20),
	(1<<23),
	(1<<26),
	-1
};
static const uint32 nNumMaxDL = LTARRAYSIZE(nMaxDL);
static const int kMaxRateClientStrLen = 5;
static const int kMaxRateAllStrLen    = 6;

namespace
{
	enum eLocalCommands 
	{
		CMD_TOGGLE_DL = CMD_CUSTOM+1,
	};
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostDownload::CScreenHostDownload()
{
	m_pMaxRatePerClient			= NULL;
	m_pMaxRateOverall			= NULL;

	// default maximum download size to unlimited
	m_nMaxDownload				= nMaxDL[nNumMaxDL - 1];
}

CScreenHostDownload::~CScreenHostDownload()
{
}


// Build the screen
bool CScreenHostDownload::Build()
{

	CreateTitle("SCREENDL_TITLE");
	uint32 kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	uint32 kColumn1 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	uint32 kColumn2 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,2);

	CLTGUICtrl_create cs;
	cs.rnBaseRect  = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));
	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,cs);
	AddControl(pFrame);

	m_DefaultPos = m_ScreenRect.m_vMin;


	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	tcs.nCommandID = CMD_TOGGLE_DL;
	tcs.szHelpID = "SCREENDL_ALLOW_DL_HELP";
	tcs.pbValue = &m_bAllowContentDownload;
	tcs.nHeaderWidth = kColumn0;
	tcs.pCommandHandler = this;

	m_pAllowDownload = AddToggle("SCREENDL_ALLOW_DL",tcs );
	m_pAllowDownload->SetOnString(LoadString("IDS_YES"));
	m_pAllowDownload->SetOffString(LoadString("IDS_NO"));

	LabeledEditCtrl::CreateStruct labeledEditCtrlCS;
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMin.Init();
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMax = LTVector2n(kColumn0+(kColumn1/2),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	labeledEditCtrlCS.m_nLabelWidth = kColumn0;
	labeledEditCtrlCS.m_nEditWidth = kColumn1;

	labeledEditCtrlCS.m_cs.szHelpID = "SCREENDL_MAX_DL_CLIENT_HELP";
	labeledEditCtrlCS.m_MaxLength = kMaxRateClientStrLen + 1;
	labeledEditCtrlCS.m_pValueChangingCB = RatePerClientValueChangingCB;
	labeledEditCtrlCS.m_pUserData = this;
	labeledEditCtrlCS.m_eInput = kInputNumberOnly;
	labeledEditCtrlCS.m_pwsValue = &m_sMaxRatePerClient;
	m_pMaxRatePerClient = new LabeledEditCtrl;
	m_pMaxRatePerClient->Create( *this, labeledEditCtrlCS, LoadString("SCREENDL_MAX_DL_CLIENT"), true );

	labeledEditCtrlCS.m_cs.szHelpID = "SCREENDL_MAX_DL_ALL_HELP";
	labeledEditCtrlCS.m_MaxLength = kMaxRateAllStrLen + 1;
	labeledEditCtrlCS.m_pValueChangingCB = RateOverallValueChangingCB;
	labeledEditCtrlCS.m_pUserData = this;
	labeledEditCtrlCS.m_eInput = kInputNumberOnly;
	labeledEditCtrlCS.m_pwsValue = &m_sMaxRateOverall;
	m_pMaxRateOverall = new LabeledEditCtrl;
	m_pMaxRateOverall->Create( *this, labeledEditCtrlCS, LoadString("SCREENDL_MAX_DL_ALL"), true );

	CLTGUISlider_create scs;
	scs.rnBaseRect.m_vMin.Init();
	scs.rnBaseRect.m_vMax = LTVector2n(kColumn0+kColumn1,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	scs.nBarOffset = kColumn0;
	scs.szHelpID = "SCREENDL_MAX_NUM_DL_HELP";
	scs.pnValue = &m_nMaxNumClients;
	scs.nMin = 1;
	scs.nMax = GameModeMgr::Instance( ).m_ServerSettings.GetMaxPlayersForBandwidth();
	scs.nIncrement = 1;
	scs.bNumericDisplay = true;
	m_pMaxNumClients = AddSlider("SCREENDL_MAX_NUM_DL", scs );

	CLTGUICycleCtrl_create ccs;
	ccs.nHeaderWidth = kColumn0;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	ccs.szHelpID = "SCREENDL_MAX_DL_SIZE_HELP";
	ccs.pnValue = &m_nMaxDownload;
	m_pMaxDownload = AddCycle("SCREENDL_MAX_DL_SIZE",ccs,false);

	wchar_t	wsTmp[32];
	for	(uint32 n = 0; n < nNumMaxDL-1; ++n)
	{
		if (nMaxDL[n] < (1<<20))
		{
			FormatString("SCREENPLAYER_DL_KB",wsTmp,LTARRAYSIZE(wsTmp),(nMaxDL[n]/1024));
		}
		else
		{
			FormatString("SCREENPLAYER_DL_MB",wsTmp,LTARRAYSIZE(wsTmp),(nMaxDL[n]/(1<<20)));
		}
		m_pMaxDownload->AddString(wsTmp);
	}
	m_pMaxDownload->AddString(LoadString("SCREENPLAYER_DL_NOLIMIT"));

	LTVector2n vPos = m_pMaxRatePerClient->GetBasePos();
	vPos.x += (kColumn0 + kColumn2);

	CLTGUICtrl_create rateCs;
	rateCs.rnBaseRect.m_vMin = vPos;
	rateCs.rnBaseRect.m_vMax = vPos + LTVector2n(kColumn2,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pRatePerClient = AddTextItem(L"", rateCs, true );

	vPos = m_pMaxRateOverall->GetBasePos();
	vPos.x += (kColumn0 + kColumn2);

	rateCs.rnBaseRect.m_vMin = vPos;
	rateCs.rnBaseRect.m_vMax = vPos + LTVector2n(kColumn2,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pRateOverall = AddTextItem(L"", rateCs, true );


	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHostDownload::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_TOGGLE_DL:
		{
			m_pAllowDownload->UpdateData();
			m_pMaxDownload->Enable(m_bAllowContentDownload);
			m_pMaxRatePerClient->Enable(m_bAllowContentDownload);
			m_pMaxRateOverall->Enable(m_bAllowContentDownload);
			m_pMaxNumClients->Enable(m_bAllowContentDownload);
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Change in focus
void CScreenHostDownload::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_bAllowContentDownload = GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload;

		m_nMaxDownload = 0;
		while (m_nMaxDownload < (nNumMaxDL-1) && nMaxDL[m_nMaxDownload] < GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadSize)
		{
			m_nMaxDownload++;
		}

		m_nMaxNumClients = GameModeMgr::Instance().m_ServerSettings.m_nMaxSimultaneousDownloads;

		wchar_t szTemp[128] = L"";
		uint32 nRate = LTMIN(GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRatePerClient,GameModeMgr::Instance( ).m_ServerSettings.GetMaxPerClientDownload()) / 8;
		LTSNPrintF(szTemp,LTARRAYSIZE(szTemp),L"%d",nRate);
		m_sMaxRatePerClient = szTemp;
		RatePerClientValueChangingCB(m_sMaxRatePerClient,this);

		nRate = LTMIN(GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRateAllClients,GameModeMgr::Instance( ).m_ServerSettings.GetMaxOverallDownload()) / 8;
		LTSNPrintF(szTemp,LTARRAYSIZE(szTemp),L"%d",nRate);
		m_sMaxRateOverall = szTemp;
		RateOverallValueChangingCB(m_sMaxRateOverall,this);

		m_pMaxNumClients->SetSliderRange(1, GameModeMgr::Instance( ).m_ServerSettings.GetMaxPlayersForBandwidth());

		m_pMaxDownload->Enable(m_bAllowContentDownload);
		m_pMaxRatePerClient->Enable(m_bAllowContentDownload);
		m_pMaxRateOverall->Enable(m_bAllowContentDownload);
		m_pMaxNumClients->Enable(m_bAllowContentDownload);

		FormatString("SCREENDL_RANGE",szTemp,LTARRAYSIZE(szTemp),1024,GameModeMgr::Instance( ).m_ServerSettings.GetMaxPerClientDownload()/8);
		m_pRatePerClient->SetString( szTemp );

		FormatString("SCREENDL_RANGE",szTemp,LTARRAYSIZE(szTemp),1024,GameModeMgr::Instance( ).m_ServerSettings.GetMaxOverallDownload()/8);
		m_pRateOverall->SetString( szTemp );

		UpdateData(false);
	}
	else
	{
		UpdateData();

		GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload = m_bAllowContentDownload;
		GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadSize = nMaxDL[m_nMaxDownload];
		GameModeMgr::Instance().m_ServerSettings.m_nMaxSimultaneousDownloads = m_nMaxNumClients;

		GameModeMgr::Instance().WriteToOptionsFile( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ));

	}
	CBaseScreen::OnFocus(bFocus);

}

void CScreenHostDownload::RatePerClientValueChangingCB( std::wstring& wsValue, void* pUserData )
{
	CScreenHostDownload *pThisScreen = (CScreenHostDownload *)pUserData;

	// Make sure the input is in range.
	uint32 nRate = (uint32)LTCLAMP( LTStrToLong(wsValue.c_str( )), 1024, GameModeMgr::Instance( ).m_ServerSettings.GetMaxPerClientDownload()/8 );
	wchar_t wszClampedVal[10];
	LTSNPrintF( wszClampedVal, LTARRAYSIZE( wszClampedVal ), L"%d", nRate );
	wsValue = wszClampedVal;

	// Setup the host info.
	GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRatePerClient = nRate * 8;

	uint32 nOverall = GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRateAllClients;

	if (GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRatePerClient > nOverall)
	{
		pThisScreen->m_sMaxRateOverall = wsValue;
		RateOverallValueChangingCB( pThisScreen->m_sMaxRateOverall, pUserData );
		pThisScreen->m_pMaxRateOverall->UpdateData(false);
	}
};

void CScreenHostDownload::RateOverallValueChangingCB( std::wstring& wsValue, void* pUserData )
{
	CScreenHostDownload *pThisScreen = (CScreenHostDownload *)pUserData;

	// Make sure the input is in range.
	uint32 nVal = LTStrToLong(wsValue.c_str( ));
	uint32 nPerClient = GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRatePerClient / 8;
	uint32 nMax = GameModeMgr::Instance( ).m_ServerSettings.GetMaxOverallDownload() / 8;
	uint32 nRate = (uint32)LTCLAMP( nVal, nPerClient, nMax );
	wchar_t wszClampedVal[10];
	LTSNPrintF( wszClampedVal, LTARRAYSIZE( wszClampedVal ), L"%d", nRate );
	wsValue = wszClampedVal;

	// Setup the host info.
	GameModeMgr::Instance( ).m_ServerSettings.m_nMaxDownloadRateAllClients = nRate * 8;
};
