// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayer.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientConnectionMgr.h"
#include "ltfileoperations.h"
#include "iltfilemgr.h"
#include "sys/win/mpstrconv.h"

#if !defined(PLATFORM_XENON)
#include "ILTGameUtil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

static const int kMaxBandwidthStrLen = 8;
static const int kMaxCDKeyLength = 24;

static CClientFXLink g_Link;
static LTVector g_vPos;



namespace
{
	enum eLocalCommands 
	{
		CMD_PATCH = CMD_CUSTOM+1,
		CMD_PATCH_ICON,
		CMD_EDIT_CDKEY,
		CMD_TOGGLE_DL,
		CMD_TOGGLE_PUNKBUSTER,
		CMD_DMMODEL,
		CMD_TEAMMODEL,
	};

	void EditNameCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};
	void EditCDKeyCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_CDKEY);
	};

	void EditBandwidthCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	};
	#define INVALID_ANI			((HMODELANIM)-1)

	wchar_t CDKeyFilter(wchar_t c, uint32 nPos)
	{
		return toupper(c);
	}

}

static const uint32 nDL[] = 
{
	(1<<10),
	(1<<13),
	(1<<16),
	(1<<20),
	(1<<23),
	(1<<26),
	-1
};
static const uint32 nNumDL = LTARRAYSIZE(nDL);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayer::CScreenPlayer()
{
	m_pName			= NULL;
	m_pCDKeyCtrl	= NULL;
	m_pModel		= NULL;
	m_pDMModel		= NULL;
	m_pTeamModel	= NULL;
	m_pPatchPreSaleWarning = NULL;
	m_pModelPreSaleWarning = NULL;
	m_pLeft			= NULL;
	m_pRight		= NULL;
	m_nCurrentDMModel = 0;
	m_nCurrentTeamModel = 0;
	m_pUsePunkbuster = NULL;
	m_bUsePunkbuster = false;

	m_pBandwidthCycle	= NULL;
	m_pBandwidth		= NULL;
	m_nBandwidth		= 0;

}

CScreenPlayer::~CScreenPlayer()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayer::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayer::Term()
{
	CBaseScreen::Term();
}

// Build the screen
bool CScreenPlayer::Build()
{
	// Get list of insignias.
	SetupInsigniaPatches();

	CreateTitle("IDS_TITLE_PLAYER");
	int kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	int kColumn1 = m_ScreenRect.GetWidth() - kColumn0;
	int nScreenFontSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);
//	int kArrow = g_pLayoutDB->GetInt32(m_hLayout(SCREEN_ID_PLAYER,"ArrowWidth");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nScreenFontSize);
	cs.nCommandID = CMD_EDIT_NAME;
	cs.pCommandHandler = this;
	cs.szHelpID = "IDS_HELP_PLAYER_NAME";

	m_pName =  AddColumnCtrl( cs );
	m_pName->AddColumn(LoadString("IDS_PLAYER_NAME"), kColumn0);
	m_pName->AddColumn(L"<player name>", kColumn1, true);


#ifdef ENABLE_CDKEY_1_CHECK
	cs.nCommandID = CMD_EDIT_CDKEY;
	cs.szHelpID = "IDS_CDKEY_HELP";
	m_pCDKeyCtrl =  AddColumnCtrl( cs );
	m_pCDKeyCtrl->AddColumn(LoadString("IDS_CDKEY"), kColumn0);
#endif

	CLTGUICycleCtrl_create ccs;
	ccs.nHeaderWidth = kColumn0;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(kColumn0, nScreenFontSize);

	ccs.szHelpID = "ScreenPlayer_Insignia_Help";
	ccs.pnValue = &m_nPatch;
	m_pPatchCycle = AddCycle(LoadString("ScreenPlayer_Insignia"),ccs);
	m_pPatchCycle->SetCommandID(CMD_PATCH);
	m_pPatchCycle->SetCommandHandler(this);

	uint32 nNum = m_sPatches.size();
	for (uint32 n = 0; n < nNum; ++n ) 
	{
		m_pPatchCycle->AddString( L" " );
	}

	// Setup presale warning.  Tab it over a little.
	cs.rnBaseRect.m_vMin.Init( );
	cs.rnBaseRect.m_vMax = LTVector2n(kColumn0,nScreenFontSize);
	cs.nCommandID = NULL;
	cs.pCommandHandler = NULL;
	cs.szHelpID = "";
	m_pPatchPreSaleWarning = AddTextItem(LoadString("IDS_PLAYER_PRESALE_WARNING"), cs, true );
	m_pPatchPreSaleWarning->Show( false );
	m_pPatchPreSaleWarning->SetBasePos( LTVector2n( m_pPatchPreSaleWarning->GetBasePos( ).x + nScreenFontSize, 
		m_pPatchPreSaleWarning->GetBasePos( ).y ));

	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nScreenFontSize);
	tcs.szHelpID = "IDS_HELP_ALLOW_BROADCAST";
	tcs.pbValue = &m_bAllowBroadcast;
	tcs.nHeaderWidth = kColumn0;
	CLTGUIToggle* pToggle = AddToggle("IDS_ALLOW_BROADCAST",tcs );
	pToggle->SetOnString(LoadString("IDS_YES"));
	pToggle->SetOffString(LoadString("IDS_NO"));


	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),nScreenFontSize);
	ccs.szHelpID = "IDS_HELP_BANDWIDTH_CYCLE";
	ccs.pnValue = &m_nBandwidth;
	m_pBandwidthCycle = AddCycle("IDS_BANDWIDTH_CYCLE",ccs);

//	m_pBandwidthCycle->AddString(LoadString("IDS_56K"));
	m_pBandwidthCycle->AddString(LoadString("IDS_DSL_LOW"));
	m_pBandwidthCycle->AddString(LoadString("IDS_DSL_HIGH"));
	m_pBandwidthCycle->AddString(LoadString("IDS_CABLE"));
	m_pBandwidthCycle->AddString(LoadString("IDS_T1"));
	m_pBandwidthCycle->AddString(LoadString("IDS_T3"));
	m_pBandwidthCycle->AddString(LoadString("IDS_CUSTOM"));

	cs.nCommandID = CMD_EDIT_BANDWIDTH;
	cs.pCommandHandler = this;
	cs.szHelpID = "IDS_HELP_BANDWIDTH_EDIT";
	m_pBandwidth = AddColumnCtrl( cs, false );
	m_pBandwidth->AddColumn(LoadString("IDS_BANDWIDTH_EDIT"), kColumn0);
	m_pBandwidth->AddColumn(L"<bandwidth>", kColumn1, true);

	tcs.nCommandID = CMD_TOGGLE_DL;
	tcs.szHelpID = "SCREENPLAYER_ALLOW_DL_HELP";
	tcs.pbValue = &m_bAllowContentDownload;
	tcs.nHeaderWidth = kColumn0;
	tcs.pCommandHandler = this;
	m_pAllowDownload = AddToggle("SCREENPLAYER_ALLOW_DL",tcs );
	m_pAllowDownload->SetOnString(LoadString("IDS_YES"));
	m_pAllowDownload->SetOffString(LoadString("IDS_NO"));

	tcs.nCommandID = 0;
	tcs.pCommandHandler = NULL;
	tcs.szHelpID = "SCREENPLAYER_ALLOW_REDIR_HELP";
	tcs.pbValue = &m_bAllowRedirect;
	m_pAllowRedirect = AddToggle("SCREENPLAYER_ALLOW_REDIR",tcs );
	m_pAllowRedirect->SetOnString(LoadString("IDS_YES"));
	m_pAllowRedirect->SetOffString(LoadString("IDS_NO"));

	// Add punkbuster control.
	tcs.nCommandID = CMD_TOGGLE_PUNKBUSTER;
	tcs.pCommandHandler = this;
	tcs.szHelpID = "SCREENPLAYER_PUNKBUSTER_HELP";
	tcs.pbValue = &m_bUsePunkbuster;
	m_pUsePunkbuster = AddToggle("SCREENPLAYER_PUNKBUSTER",tcs );
	m_pUsePunkbuster->SetOnString(LoadString("IDS_YES"));
	m_pUsePunkbuster->SetOffString(LoadString("IDS_NO"));

	ccs.szHelpID = "SCREENPLAYER_MAX_DL_HELP";
	ccs.pnValue = &m_nMaxDownload;
	m_pMaxDownload = AddCycle("SCREENPLAYER_MAX_DL",ccs,false);

	
	wchar_t	wsTmp[32];
	for	(uint32 n = 0; n < nNumDL-1; ++n)
	{
		if (nDL[n] < (1<<20))
		{
			FormatString("SCREENPLAYER_DL_KB",wsTmp,LTARRAYSIZE(wsTmp),(nDL[n]/1024));
		}
		else
		{
			FormatString("SCREENPLAYER_DL_MB",wsTmp,LTARRAYSIZE(wsTmp),(nDL[n]/(1<<20)));
		}
		m_pMaxDownload->AddString(wsTmp);
	}
	m_pMaxDownload->AddString(LoadString("SCREENPLAYER_DL_NOLIMIT"));

	cs.rnBaseRect.m_vMax = LTVector2n(kColumn0,nScreenFontSize);
	cs.nCommandID = CMD_DMMODEL;
	cs.pCommandHandler = this;
	cs.szHelpID = "ScreenPlayer_DMModel_Help";
	m_pDMModel = AddTextItem(LoadString("ScreenPlayer_DMModel"), cs );

	cs.rnBaseRect.m_vMax = LTVector2n(kColumn0,nScreenFontSize);
	cs.nCommandID = CMD_TEAMMODEL;
	cs.pCommandHandler = this;
	cs.szHelpID = "ScreenPlayer_TeamModel_Help";
	m_pTeamModel = AddTextItem(LoadString("ScreenPlayer_TeamModel"), cs );

	LTVector2n arrowPos = m_pDMModel->GetBasePos();
	arrowPos.x += kColumn0 + (kColumn1 / 4);

	TextureReference hLeft,hLeftH;
	hLeft.Load("interface\\menu\\sprtex\\arrowlt.dds");
	hLeftH.Load("interface\\menu\\sprtex\\arrowlt_h.dds");


	CLTGUITextureButton_create bcs;


	bcs.rnBaseRect.m_vMin = arrowPos;
	bcs.rnBaseRect.m_vMax = arrowPos + LTVector2n(16,16);
	bcs.hNormal = hLeft;
	bcs.hSelected = hLeftH;
	bcs.nCommandID = CMD_LEFT;

	m_pLeft = debug_new(CLTGUITextureButton);
	m_pLeft->Create(bcs);
	AddControl(m_pLeft);
	m_pLeft->SetCommandHandler(this);
	m_pLeft->Enable(true);


	arrowPos.x = m_ScreenRect.GetWidth();

	TextureReference hRight, hRightH;
	
	hRight.Load("interface\\menu\\sprtex\\arrowrt.dds");
	hRightH.Load("interface\\menu\\sprtex\\arrowrt_h.dds");

	bcs.rnBaseRect.m_vMin = arrowPos;
	bcs.rnBaseRect.m_vMax = arrowPos + LTVector2n(16,16);
	bcs.hNormal = hRight;
	bcs.hSelected = hRightH;
	bcs.nCommandID = CMD_RIGHT;

	m_pRight = debug_new(CLTGUITextureButton);
	m_pRight->Create(bcs);
	AddControl(m_pRight);
	m_pRight->SetCommandHandler(this);
	m_pRight->Enable(true);

	// Setup presale warning.  Tab it over a little.
	LTVector2n vModelPreSaleWarningScreenPos = g_pLayoutDB->GetPosition(m_hLayout,LDB_ScreenAdditionalPos, 1);
	cs.rnBaseRect.m_vMin = vModelPreSaleWarningScreenPos - LTVector2n(kColumn0/2,0);
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + LTVector2n(kColumn0,nScreenFontSize);
	cs.nCommandID = NULL;
	cs.pCommandHandler = NULL;
	cs.szHelpID = "";
	m_pModelPreSaleWarning = AddTextItem(LoadString("IDS_PLAYER_PRESALE_WARNING"), cs, true );
	m_pModelPreSaleWarning->Show( false );
	m_pModelPreSaleWarning->SetAlignment( kCenter );

	if( !m_sPatches.empty())
	{
		TextureReference hPatch;
		hPatch.Load(m_sPatches[0].c_str());
		bcs.rnBaseRect.m_vMin = m_pPatchCycle->GetBasePos();
		bcs.rnBaseRect.m_vMin.x += kColumn0;
		bcs.rnBaseRect.m_vMax = bcs.rnBaseRect.m_vMin + LTVector2n(32,32);
		bcs.hNormal = hPatch;
		bcs.hSelected = NULL;
		bcs.nCommandID = CMD_PATCH_ICON;
		bcs.pCommandHandler = this;

		m_pPatch = debug_new(CLTGUITextureButton);
		m_pPatch->Create(bcs);
		AddControl(m_pPatch);
	}

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenPlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pFilterFn = PlayerNameCharacterFilter;
			mb.pUserData = this;
			mb.pString = m_sPlayerName.c_str();
			mb.nMaxChars = MAX_PLAYER_NAME;
			g_pInterfaceMgr->ShowMessageBox("IDS_PLAYER_NAME",&mb);
		} break;
	case CMD_EDIT_CDKEY:
		{
			// Show the CD key change dialog
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditCDKeyCallBack;
			mb.pUserData = this;
			mb.nMaxChars = kMaxCDKeyLength + 1;
			mb.pFilterFn = CDKeyFilter;
			g_pInterfaceMgr->ShowMessageBox("IDS_CDKEY_ENTER",&mb);
		} break;

	case CMD_EDIT_BANDWIDTH:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pUserData = this;
			mb.pString = m_sBandwidth.c_str();
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox("IDS_BANDWIDTH_EDIT",&mb);
		} break;
	case CMD_TOGGLE_DL:
		{
			m_pAllowDownload->UpdateData();
			m_pAllowRedirect->Enable(m_bAllowContentDownload);
			m_pMaxDownload->Enable(m_bAllowContentDownload);

		} break;
	case CMD_TOGGLE_PUNKBUSTER:
		{
			// Get the data from the control.
			m_pUsePunkbuster->UpdateData(true);

			// Store the change the profile now, since the control might get switched back to a different
			// value if it fails to init pb.  This way the desired setting persists, but the true setting
			// is reflected in the control.
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			pProfile->SetUsePunkbuster( m_bUsePunkbuster );

			// Update the punkbuster api and ui.
			UpdatePunkBuster();

			// Update the control with the current setting.
			m_pUsePunkbuster->UpdateData(false);

		} break;
	case CMD_DMMODEL:
		{
			// If our active model is already the dm model, go to the next model.
			if( m_pModel == m_pDMModel )
			{
				NextModel();
			}
			// Otherwise, set our active model to the dm model.
			else
			{
				m_pModel = m_pDMModel;
				UpdateChar();
			}
		} break;
	case CMD_TEAMMODEL:
		{
			// If our active model is already the team model, go to the next model.
			if( m_pModel == m_pTeamModel )
			{
				NextModel();
			}
			// Otherwise, set our active model to the team model.
			else
			{
				m_pModel = m_pTeamModel;
				UpdateChar();
			}
		} break;
	case CMD_LEFT:
		{
			PrevModel();
		} break;
	case CMD_RIGHT:
		{
			NextModel();
		} break;
	case CMD_PATCH:
		{
			UpdateData();
			if( !m_sPatches.empty( ))
			{
				TextureReference hPatch(m_sPatches[m_nPatch].c_str());
				m_pPatch->SetTexture(hPatch);

				// Show the presale warning if this is part of the presale patches.
				StringArray::iterator iter = std::find( m_sPreSalePatches.begin(), m_sPreSalePatches.end( ), m_sPatches[m_nPatch] );
				m_pPatchPreSaleWarning->Show( iter != m_sPreSalePatches.end( ));
			}
			
		} break;
	case CMD_PATCH_ICON:
		{
			// Tell the cycle control to go right when they click on the icon.
			m_pPatchCycle->OnRight();
		} break;



	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenPlayer::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	bool bInMPGame = (IsMultiplayerGameClient() && g_pGameClientShell->IsWorldLoaded());		
	if (bFocus)
	{
		m_bAllowBroadcast = pProfile->m_bAllowBroadcast;
		m_bAllowContentDownload = pProfile->m_bAllowContentDownload;
		m_bAllowRedirect = pProfile->m_bAllowContentDownloadRedirect;

		// Get the punkbuster setting.  Set control to the actually state of punkbuster, not the
		// desired profile setting.  The profile setting may want pb on, but it may not be available.  It
		// is a requirement of pb to show the true status of pb in the setting.
		IPunkBusterClient* pPunkBusterClient = g_pGameClientShell->GetPunkBusterClient();
		if( pPunkBusterClient )
		{
			m_bUsePunkbuster = pPunkBusterClient->IsEnabledRequested( );
		}
		else
		{
			m_bUsePunkbuster = false;
		}

		// If the user tries disable, but the real state is enabled, then tell them this will be possible on the
		// next restart.
		bool bShowNoOnRestart = ( !m_bUsePunkbuster && pPunkBusterClient->IsEnabled( ));
		if( bShowNoOnRestart )
		{
			m_pUsePunkbuster->SetOffString(LoadString("EnablePunkbuster_Yes_NoOnNextRestart"));
		}
		else
		{
			m_pUsePunkbuster->SetOffString(LoadString("IDS_NO"));
		}

		m_nMaxDownload = 0;
		while (m_nMaxDownload < (nNumDL-1) && nDL[m_nMaxDownload] < pProfile->m_nMaximumDownloadSize)
		{
			m_nMaxDownload++;
		}

		m_pAllowRedirect->Enable(m_bAllowContentDownload);
		m_pMaxDownload->Enable(m_bAllowContentDownload);

		m_nCurrentDMModel = pProfile->m_nDMPlayerModel;
		m_nCurrentTeamModel = pProfile->m_nTeamPlayerModel;

		m_pModel = m_pDMModel;
		m_pLeft->Show(true);
		m_pRight->Show(true);
		m_pModel->Show(true);

		m_sPlayerName = pProfile->m_sPlayerName;
		m_pName->SetString(1,m_sPlayerName.c_str());
		
#ifdef ENABLE_CDKEY_1_CHECK
		char szCDKey[256];
		g_pVersionMgr->GetCDKey( szCDKey, ARRAY_LEN( szCDKey ));
		m_sCurCDKey = szCDKey;
		m_sLastValidCDKey = m_sCurCDKey;
#endif

		m_nBandwidth = pProfile->m_nBandwidthClient;
		UpdateBandwidth();
		
		LTVector2n vScreenPos = g_pLayoutDB->GetPosition(m_hLayout,LDB_ScreenAdditionalPos);
		LTVector2 vScalePos = g_pInterfaceResMgr->ConvertScreenPos(vScreenPos);
		float fDepth = float(g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt));
		g_vPos = g_pInterfaceMgr->GetWorldFromScreenPos( LTVector2n(int32(vScalePos.x),int32(vScalePos.y)), fDepth);

		uint32 nNum = m_sPatches.size();
		uint32 n = 0;
		m_nPatch = -1;
		while (n < nNum && m_nPatch >= nNum)
		{
			if (m_sPatches[n].compare(pProfile->m_sPlayerPatch) == 0 )
			{
				m_nPatch = n;
			}
			++n;
		}

		if (m_nPatch >= nNum) 
		{
			m_nPatch = 0;
		}

		if( !m_sPatches.empty( ))
		{
			TextureReference hPatch(m_sPatches[m_nPatch].c_str());
			m_pPatch->SetTexture(hPatch);
		}

		// Show the presale warning if this is part of the presale patches.
		StringArray::iterator iter = std::find( m_sPreSalePatches.begin(), m_sPreSalePatches.end( ), m_sPatches[m_nPatch] );
		m_pPatchPreSaleWarning->Show( iter != m_sPreSalePatches.end( ));

        UpdateData(false);
	}
	else
	{
		// Force close any message box that we opened.
		g_pInterfaceMgr->CloseMessageBox( false );

		UpdateData();
		pProfile->m_sPlayerName = m_sPlayerName;

		pProfile->m_sPlayerPatch = ( !m_sPatches.empty( )) ? m_sPatches[m_nPatch] : "";

		pProfile->m_nDMPlayerModel = m_nCurrentDMModel;
		pProfile->m_nTeamPlayerModel = m_nCurrentTeamModel;

		pProfile->m_bAllowBroadcast = m_bAllowBroadcast;

		pProfile->m_bAllowContentDownload = m_bAllowContentDownload;
		pProfile->m_bAllowContentDownloadRedirect = m_bAllowRedirect;

		pProfile->m_nMaximumDownloadSize = nDL[m_nMaxDownload];
	 		
 		pProfile->m_nBandwidthClient = m_nBandwidth;
		pProfile->m_nBandwidthClientCustom = (uint16)_wtol(m_sBandwidth.c_str());

		pProfile->Save();

		if (bInMPGame)
		{
			g_pClientConnectionMgr->SetupClientBandwidth( g_pClientConnectionMgr->IsConnectedToLANServer() );
			g_pClientConnectionMgr->SendMultiPlayerInfo();
		}
	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		UpdateChar();
	else
	{
		g_pInterfaceMgr->RemoveInterfaceFX(&g_Link);
	}
}

bool	CScreenPlayer::OnUp()
{
	m_pLeft->Enable(false);
	m_pRight->Enable(false);
	bool bHandled = CBaseScreen::OnUp();
	m_pLeft->Enable(true);
	m_pRight->Enable(true);
	return bHandled;
}

bool	CScreenPlayer::OnDown()
{
	m_pLeft->Enable(false);
	m_pRight->Enable(false);
	bool bHandled = CBaseScreen::OnDown();
	m_pLeft->Enable(true);
	m_pRight->Enable(true);
	return bHandled;
}


bool	CScreenPlayer::OnLeft()
{

	if (m_pModel->IsSelected())
	{
		PrevModel();
		return true;
	}

	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return true;
	}

	return CBaseScreen::OnLeft();
}


bool	CScreenPlayer::OnRight()
{
	if (m_pModel->IsSelected())
	{
		NextModel();
		return true;
	}

	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return true;
	}

	return CBaseScreen::OnRight();

}

/******************************************************************/

/******************************************************************/
bool CScreenPlayer::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return true;
	}

	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
bool CScreenPlayer::OnRButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return true;
	}
	return CBaseScreen::OnRButtonUp(x,y);
}
bool CScreenPlayer::OnMouseMove(int x, int y)
{
	bool bHandled = CBaseScreen::OnMouseMove(x, y);
	if (GetSelectedControl() == m_pLeft || GetSelectedControl() == m_pRight)
	{
		m_pModel->Select(true);
	}
	else if (GetSelectedControl() != m_pModel)
	{
		m_pModel->Select(false);
	}
	return bHandled;
}

void CScreenPlayer::NextModel()
{
	bool bUpdateChar = false;

	if( m_pModel == m_pDMModel )
	{
		uint32 nOld = m_nCurrentDMModel;
		m_nCurrentDMModel++;
		if (m_nCurrentDMModel >= g_pModelsDB->GetNumDMModels())
			m_nCurrentDMModel = 0;
		bUpdateChar = nOld != m_nCurrentDMModel;
	}
	else
	{
		uint32 nOld = m_nCurrentTeamModel;
		m_nCurrentTeamModel++;
		if (m_nCurrentTeamModel >= g_pModelsDB->GetNumTeamModels())
			m_nCurrentTeamModel = 0;
		bUpdateChar = nOld != m_nCurrentTeamModel;
	}

	if( bUpdateChar )
		UpdateChar();
}

void CScreenPlayer::PrevModel()
{
	bool bUpdateChar = false;

	if( m_pModel == m_pDMModel )
	{
		uint32 nOld = m_nCurrentDMModel;
		if (!m_nCurrentDMModel)
			m_nCurrentDMModel = g_pModelsDB->GetNumDMModels();
		m_nCurrentDMModel--;
		bUpdateChar = nOld != m_nCurrentDMModel;
	}
	else
	{
		uint32 nOld = m_nCurrentTeamModel;
		if (!m_nCurrentTeamModel)
			m_nCurrentTeamModel = g_pModelsDB->GetNumTeamModels();
		m_nCurrentTeamModel--;
		bUpdateChar = nOld != m_nCurrentTeamModel;
	}
		
	if( bUpdateChar )
		UpdateChar();
}

void CScreenPlayer::UpdateChar()
{

	g_pInterfaceMgr->RemoveInterfaceFX(&g_Link);

	ModelsDB::HMODEL hModel = NULL;
	if( m_pModel == m_pDMModel )
	{
		hModel = g_pModelsDB->GetDMModel(m_nCurrentDMModel);
	}
	else
	{
		hModel = g_pModelsDB->GetFriendlyTeamModel( m_nCurrentTeamModel );
	}

	bool bPreSale = g_pModelsDB->GetModelPreSale( hModel );
	m_pModelPreSaleWarning->Show( bPreSale );
	
	const char* pszFX = g_pModelsDB->GetInterfaceFX(hModel);

	if (pszFX && pszFX[0])
	{
		g_pInterfaceMgr->AddInterfaceFX(&g_Link,pszFX,g_vPos,true,false);
	}
}


void CScreenPlayer::UpdateBandwidth()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	wchar_t	wsTmp[32];
	if ( m_nBandwidth >= eBandwidth_Custom )
	{
		LTSNPrintF(wsTmp,LTARRAYSIZE(wsTmp),L"%d",pProfile->m_nBandwidthClientCustom);
		m_sBandwidth = wsTmp;
		m_pBandwidth->SetString(1,m_sBandwidth.c_str());
	}
	else
	{
		LTSNPrintF(wsTmp,LTARRAYSIZE(wsTmp),L"%d",g_BandwidthClient[m_nBandwidth].m_nBandwidthTargetClient);
		m_sBandwidth = wsTmp;
		m_pBandwidth->SetString(1,m_sBandwidth.c_str());
	}
}


void CScreenPlayer::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_BANDWIDTH:
		{
			wchar_t *pszBandwidth = (wchar_t *)dwParam1;
			uint16 nBandwidth = (uint16)_wtol(pszBandwidth);
			if ( IsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = pszBandwidth;
				if (m_sBandwidth.size() > kMaxBandwidthStrLen)
				{
					m_sBandwidth.resize(kMaxBandwidthStrLen);
				}
				
				m_nBandwidth = eBandwidth_Custom;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				pProfile->m_nBandwidthClientCustom = nBandwidth;
				m_pBandwidthCycle->UpdateData(false);
				UpdateBandwidth();
			}
			else
			{
				wchar_t wszBuffer[384];
				FormatString( "IDS_BANDWIDTH_INVALID", wszBuffer, LTARRAYSIZE(wszBuffer), GetMinimumBandwidth(), GetMaximumBandwidth() );
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(wszBuffer,&mb);
			}
		}
		break;
	case CMD_EDIT_NAME:
		{
			wchar_t const* pName = FixPlayerName((wchar_t const*)dwParam1);
			if (pName && !LTStrEmpty(pName))
			{
				m_sPlayerName = pName;
				m_pName->SetString(1,m_sPlayerName.c_str( ));
			}
		} break;
	case CMD_EDIT_CDKEY :
		{
			char newCdKey[kMaxCDKeyLength+1];
			MPW2A wsStr(( const wchar_t* )dwParam1);
			const char* pStr = wsStr.c_str();
			int c = 0;
			int n = 0;
			while (*pStr && c < kMaxCDKeyLength)
			{
				newCdKey[c] = *pStr;
				c++;
				pStr++;
				n++;
				//insert dashes every 4th char as necessary
				if (n == 4 && c < kMaxCDKeyLength)
				{
					n = 0;
					//if there is a dash, copy it and move on
					if (*pStr == '-')
					{
						newCdKey[c] = *pStr;
						c++;
						pStr++;
					}
					else
					{
						newCdKey[c] = '-';
						c++;
					}
				}

			}
			newCdKey[c] = 0;


			// Check if they entered a valid cdkey.
#ifdef ENABLE_CDKEY_1_CHECK
			if( !g_pLTGameUtil->IsCDKeyValid( newCdKey))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox("IDS_CDKEY_INVALID",&mb);
			}
			else
			{
				m_sCurCDKey = newCdKey;
				g_pVersionMgr->SetCDKey( m_sCurCDKey.c_str( ));
			}
#endif
		}
	}
	UpdateData();
}

void CScreenPlayer::SetupInsigniaPatches()
{
	char szPatchDir[MAX_PATH] = "";
	g_pModelsDB->GetInsigniaFolder( szPatchDir, LTARRAYSIZE( szPatchDir ));

	FileEntry *pFiles = g_pLTClient->FileMgr()->GetFileList( szPatchDir );
	if( !pFiles )
		return;

	char strBaseName[256];
	char strBaseExt[256];
	FileEntry* ptr = pFiles;

	while (ptr)
	{
		if (ptr->m_Type == eLTFileEntryType_File)
		{
			
			LTFileOperations::SplitPath(ptr->m_pBaseFilename,NULL,strBaseName,strBaseExt);
			if (LTStrICmp(strBaseExt, RESEXT_DOT(RESEXT_TEXTURE)) == 0)
			{
				LTStrCpy(strBaseName, szPatchDir, LTARRAYSIZE(strBaseName));
				LTStrCat(strBaseName, ptr->m_pBaseFilename, LTARRAYSIZE(strBaseName));
				// add this to the array
				m_sPatches.push_back(strBaseName);
			}
		}

		ptr = ptr->m_pNext;
	}

	// Find all the presale patches.
	HATTRIBUTE hPreSaleInsignia = g_pModelsDB->GetPreSaleInsigniaAttribute();
	uint32 nNumPreSale = g_pLTDatabase->GetNumValues( hPreSaleInsignia );
	for( uint32 nIndex = 0; nIndex < nNumPreSale; nIndex++ )
	{
		char const* pszPreSalePatch = g_pLTDatabase->GetString( hPreSaleInsignia, nIndex, "" );
		if( LTStrEmpty( pszPreSalePatch ))
			continue;

		m_sPreSalePatches.push_back( pszPreSalePatch );
	}
}


// Updates the punkbuster api and ui based on current user setting.
void CScreenPlayer::UpdatePunkBuster()
{
	// Try to set the setting on punkbuster.  If it failed, then reset the 
	// UI control.
	IPunkBusterClient* pPunkBusterClient = g_pGameClientShell->GetPunkBusterClient();
	if( pPunkBusterClient )
	{
		if( m_bUsePunkbuster && !pPunkBusterClient->IsEnabledRequested( ))
		{
			pPunkBusterClient->Enable( );
		}
		else if( !m_bUsePunkbuster && pPunkBusterClient->IsEnabledRequested( ))
		{
			pPunkBusterClient->Disable( );
		}

		// Get the requested setting.
		m_bUsePunkbuster = pPunkBusterClient->IsEnabledRequested( );

		// If the user tries disable, but the real state is enabled, then tell them this will be possible on the
		// next restart.
		bool bShowNoOnRestart = ( !m_bUsePunkbuster && pPunkBusterClient->IsEnabled( ));
		if( bShowNoOnRestart )
		{
			m_pUsePunkbuster->SetOffString(LoadString("EnablePunkbuster_Yes_NoOnNextRestart"));
		}
		else
		{
			m_pUsePunkbuster->SetOffString(LoadString("IDS_NO"));
		}
	}
	else
	{
		// Pb not available.
		m_bUsePunkbuster = false;
	}
}