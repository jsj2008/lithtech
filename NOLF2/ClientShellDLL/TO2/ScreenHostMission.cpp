// ScreenHostMission.cpp: implementation of the CScreenHostMission class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ScreenHostMission.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "NetDefs.h"
#include "profileMgr.h"
#include "clientmultiplayermgr.h"
#include "WinUtil.h"
#include <IO.h>

#include "GameClientShell.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	std::string	sOldFN;

	LTBOOL bCreate = LTFALSE;

	void DeleteCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostMission *pThisScreen = (CScreenHostMission *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_MISSION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_DELETE);
	}
	void CreateCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostMission *pThisScreen = (CScreenHostMission *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_MISSION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_CREATE);
	}
	void LoadCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostMission *pThisScreen = (CScreenHostMission *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_MISSION);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_LOAD);
	}
	void EditCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHostMission *pThisScreen = (CScreenHostMission *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST_MISSION);
		if (bReturn && pThisScreen)
		{
			pThisScreen->SendCommand(CMD_EDIT,(uint32)pData,0);
		}
	};

	int kDlgHt = 240;
	int kDlgWd = 240;
	uint32 nCommand = 0;
	uint8 nListFont = 1;
	uint8 nListFontSz = 14;

}

CScreenHostMission::CScreenHostMission()
{
	m_pCurrent = LTNULL;
	m_pLoad = LTNULL;
	m_pDelete = LTNULL;
	m_pRename = LTNULL;
	m_pLevels = LTNULL;

}

CScreenHostMission::~CScreenHostMission()
{
}

// Build the screen
LTBOOL CScreenHostMission::Build()
{
	// Set the title's text
	CreateTitle(IDS_TITLE_HOST_CAMPAIGN);

	// Get edit controls position and create it.
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"MissionNamePos");
	m_pCurrent = AddTextItem(" ", LTNULL, LTNULL, pos, LTTRUE);

	AddTextItem(IDS_CREATE, CMD_CREATE, IDS_HELP_CAMPAIGN_CREATE);

	AddTextItem(IDS_LOAD, CMD_LOAD, IDS_HELP_CAMPAIGN_LOAD);

	m_pRename = AddTextItem(IDS_RENAME, CMD_RENAME, IDS_HELP_CAMPAIGN_RENAME);

	m_pDelete = AddTextItem(IDS_DELETE, CMD_DELETE, IDS_HELP_CAMPAIGN_DELETE);

	m_pLevels = AddTextItem(IDS_HOST_MISSIONS, CMD_SET_LEVELS, IDS_HELP_HOST_MISSIONS);



	LTIntPt dlgPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"DialogPos");
	LTIntPt dlgSz = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"DialogSize");
	kDlgHt = dlgSz.y;
	kDlgWd = dlgSz.x;
	uint8 nDlgFont = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"DialogFontFace");
	uint8 nDlgFontSz = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"DialogFontSize");

	nListFont = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontFace");
	nListFontSz = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");

	char szBack[128] = "";
	g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,"DialogFrame",szBack,sizeof(szBack));

	m_pDlg = debug_new(CLTGUIWindow);
	m_pDlg->Create(g_pInterfaceResMgr->GetTexture(szBack),kDlgHt,kDlgWd);

	LTIntPt tmp(8,8);
	CUIFont *pFont = g_pInterfaceResMgr->GetFont(nDlgFont);

	CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_CAMPAIGN_LIST, LTNULL, LTNULL, kDefaultPos, LTTRUE);
	pCtrl->SetFont(pFont,nDlgFontSz);
	m_pDlg->AddControl(pCtrl, tmp);

	tmp.y += 24;

	
	// Make a list controller
	m_pListCtrl = debug_new(CLTGUIListCtrl);
    if (m_pListCtrl->Create(kDlgHt-64))
	{
		HTEXTURE hUp = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup.dtx");
		HTEXTURE hUpH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowup_h.dtx");
		HTEXTURE hDown = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn.dtx");
		HTEXTURE hDownH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowdn_h.dtx");
		m_pListCtrl->UseArrows(kDlgWd-48,1.0f,hUp,hUpH,hDown,hDownH);
		m_pListCtrl->SetIndent(LTIntPt(4,4));
		m_pListCtrl->SetFrameWidth(2);
		m_pListCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);

		m_pDlg->AddControl(m_pListCtrl, tmp);

	}

	pCtrl = CreateTextItem(IDS_CANCEL, CMD_CANCEL, LTNULL);
	pCtrl->SetFont(pFont,nDlgFontSz);
	tmp.x = (kDlgWd - pCtrl->GetBaseWidth()) / 2;
	tmp.y = (kDlgHt - pCtrl->GetBaseHeight()) - 8;
	m_pDlg->AddControl(pCtrl, tmp);


	m_pDlg->SetBasePos(dlgPos);
	m_pDlg->SetScale(g_pInterfaceResMgr->GetXRatio());
	m_pDlg->Show(LTFALSE);

	m_pDlg->SetSelection(1);
	AddControl(m_pDlg);

	UseBack(LTTRUE);


	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHostMission::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_OK:
		{
			m_pDlg->Show(LTFALSE);
			SetCapture(LTNULL);
			HandleDlgCommand(nCommand,(uint16)dwParam1);
			SetSelection(1);
		}
		break;
	case CMD_CANCEL:
		{
			m_pDlg->Show(LTFALSE);
			SetCapture(LTNULL);
			SetSelection(1);
		}
		break;

	case CMD_SET_LEVELS:
		{

			if (m_pProfile->m_ServerGameOptions.IsDefaultCampaign() )
				return 0;
			else
			{
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_LEVELS);
			}
		}
		break;


	case CMD_LOAD:
		{
			m_pDefaultTextCtrl->Show(LTTRUE);
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(LTTRUE);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_CONFIRM:
		{
			uint16 nIndex = (uint16)dwParam1;
			if (dwParam2 == CMD_DELETE)
			{
				if (nIndex != m_nDefaultCampaign && nIndex < m_pListCtrl->GetNumControls()) 
				{
					StringSet::iterator iter = m_CampaignList.begin();
					for (int i = 0; i < nIndex && iter != m_CampaignList.end(); ++i, ++iter);

					if (iter != m_CampaignList.end())
					{
						DeleteCampaign(*iter);

					}
				}
			}
			else if (dwParam2 == CMD_LOAD)
			{
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					StringSet::iterator iter = m_CampaignList.begin();
					for (int i = 0; i < nIndex && iter != m_CampaignList.end(); ++i, ++iter);

					if (iter != m_CampaignList.end())
					{
						SetCampaignName( (*iter).c_str());
					}

				}
			}
			else if (dwParam2 == CMD_CREATE)
			{
				m_sOldCampaign = m_pProfile->m_ServerGameOptions.GetCampaignName();
				std::string sNew = m_sOldCampaign;
				if (sNew.compare(DEFAULT_CAMPAIGN) == 0)
					sNew = m_pProfile->m_sName;
				bCreate = LTTRUE;

				//show edit box here	
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = sNew.c_str();
				mb.nMaxChars = MAX_PROFILE_NAME;
				mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENTER_CAMPAIGN_NAME,&mb);
			}
		}
		break;
	case CMD_DELETE:
		{
			m_pDefaultTextCtrl->Show(LTFALSE);
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(LTTRUE);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_CREATE:
		{
			if (g_pLTClient->IsConnected())
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = CreateCallBack,
				g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_NEWPROFILE,&mb);
			}
			else
				SendCommand(CMD_CONFIRM,0,CMD_CREATE);
		}
		break;
	case CMD_RENAME:
		{
			m_pDefaultTextCtrl->Show(LTFALSE);
			m_pListCtrl->SetStartIndex(0);
			m_pDlg->Show(LTTRUE);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_EDIT:
		{
			std::string campaignName = (char *)dwParam1;

			StringSet::iterator iter = m_CampaignList.find(campaignName);
			if (iter != m_CampaignList.end())
				campaignName = (*iter);

			if (bCreate)
			{
				NewCampaign(campaignName);
			}
			else
			{
				RenameCampaign(sOldFN,campaignName);
			}
			
		}
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CScreenHostMission::Escape()
{
	if (m_pDlg->IsVisible())
	{
		m_pDlg->Show(LTFALSE);
		SetCapture(LTNULL);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else
		CBaseScreen::Escape();
}

LTBOOL	CScreenHostMission::OnLButtonUp(int x, int y)
{
	return CBaseScreen::OnLButtonUp(x,y);
}

LTBOOL	CScreenHostMission::OnRButtonUp(int x, int y)
{
	return CBaseScreen::OnRButtonUp(x,y);
}

void CScreenHostMission::CreateCampaignList()
{
	// Empty the list
	m_pListCtrl->RemoveAll();
	m_CampaignList.clear();

	CUIFont *pFont = g_pInterfaceResMgr->GetFont(nListFont);

	m_CampaignList.insert(DEFAULT_CAMPAIGN);

	// Get new stuff
	struct _finddata_t file;
	long hFile;

	CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
	std::string directory = GetCampaignDir( g_pProfileMgr->GetCurrentProfileName( ), 
		pUserProfile->m_ServerGameOptions.m_eGameType );
	directory += "*.txt"; 

	// find first file
	if((hFile = _findfirst(directory.c_str(), &file)) != -1L)
	{
		do
		{
			if (_stricmp(file.name,DEFAULT_CAMPAIGN_FILE) != 0)
			{
				char *pName = strtok(file.name,".");
				m_CampaignList.insert(pName);
			}

		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);

	// add campaigns to the list control
	for (StringSet::iterator iter = m_CampaignList.begin(); iter != m_CampaignList.end(); ++iter)
	{
		CLTGUITextCtrl* pTextCtrl = NULL;
		uint16 ndx = 0;
		if (iter->compare(DEFAULT_CAMPAIGN) == 0)
		{
			m_pDefaultTextCtrl = CreateTextItem(IDS_HOST_CAMPAIGN_DEFAULT, CMD_OK, LTNULL);
			ndx = m_pListCtrl->AddControl(m_pDefaultTextCtrl);
			m_nDefaultCampaign = ndx;
			m_pDefaultTextCtrl->Show(LTFALSE);
			m_pDefaultTextCtrl->SetParam1(ndx);
			m_pDefaultTextCtrl->SetFont(pFont,nListFontSz);
		}
		else
		{
			pTextCtrl = CreateTextItem((char *)iter->c_str(), CMD_OK, LTNULL);
			pTextCtrl->SetFont(pFont,nListFontSz);
			ndx = m_pListCtrl->AddControl(pTextCtrl);
			pTextCtrl->SetParam1(ndx);
		}

		
		

		if (iter->compare(m_pProfile->m_ServerGameOptions.GetCampaignName()) == 0)
			m_pListCtrl->SetSelection(ndx);
	}

}

void CScreenHostMission::UpdateCampaignName()
{
	bool isDefault = (m_pProfile->m_ServerGameOptions.IsDefaultCampaign());
	if (!isDefault)
	{
		char const* pszCampaignFile = GetCampaignFile( m_pProfile->m_ServerGameOptions );

		if (!CWinUtil::FileExist( pszCampaignFile ))
		{
			m_pProfile->m_ServerGameOptions.SetCampaignName(DEFAULT_CAMPAIGN);
			isDefault = true;
		}
	}



	if (isDefault)
	{
		char szTmp[64];
		LoadString(IDS_HOST_CAMPAIGN_DEFAULT,szTmp,sizeof(szTmp));
		m_pCurrent->SetString(FormatTempString(IDS_CURRENT_CAMPAIGN,szTmp));
		m_pLevels->Enable(false);
	}
	else
	{
		m_pCurrent->SetString(FormatTempString(IDS_CURRENT_CAMPAIGN,m_pProfile->m_ServerGameOptions.GetCampaignName()));
		m_pLevels->Enable(true);
	}
}

void CScreenHostMission::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		m_eGameType = g_pGameClientShell->GetGameType();
		UpdateCampaignName();
		CreateCampaignList();
	}
	CBaseScreen::OnFocus(bFocus);
}



void CScreenHostMission::HandleDlgCommand(uint32 dwCommand, uint16 nIndex)
{
	switch (dwCommand)
	{
	case CMD_LOAD:
		{
			m_sOldCampaign = m_pProfile->m_ServerGameOptions.GetCampaignName();
			if (nIndex < m_pListCtrl->GetNumControls()) 
			{
				StringSet::iterator iter = m_CampaignList.begin();
				for (int i = 0; i < nIndex && iter != m_CampaignList.end(); ++i, ++iter);

				if (iter != m_CampaignList.end())
				{
					std::string sCampaign = *iter;
					if (g_pLTClient->IsConnected() && stricmp(sCampaign.c_str(),m_pProfile->m_ServerGameOptions.GetCampaignName()) != 0)
					{

						MBCreate mb;
						mb.eType = LTMB_YESNO;
						mb.pFn = LoadCallBack,
						mb.pData = (void *)nIndex;
						g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_NEWPROFILE,&mb);
					}
					else
						SendCommand(CMD_CONFIRM,nIndex,CMD_LOAD);

				}
			}
		}
		break;
	case CMD_DELETE:
		{
			if (nIndex != m_nDefaultCampaign)
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = DeleteCallBack,
				mb.pData = (void *)nIndex;
				g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_DELETE,&mb);
			}
		}
		break;
	case CMD_RENAME:
		m_sOldCampaign = m_pProfile->m_ServerGameOptions.GetCampaignName();
		if (nIndex != m_nDefaultCampaign && nIndex < m_pListCtrl->GetNumControls()) 
		{
			StringSet::iterator iter = m_CampaignList.begin();
			for (int i = 0; i < nIndex && iter != m_CampaignList.end(); ++i, ++iter);

			if (iter != m_CampaignList.end())
			{
				std::string sCampaign = *iter;
				sOldFN = sCampaign;
				bCreate = LTFALSE;
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = sCampaign.c_str();
				mb.nMaxChars = 16;
				mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENTER_CAMPAIGN_NAME,&mb);
			}
		}
		break;
	}

};


void CScreenHostMission::DeleteCampaign(const std::string& campaignName)
{
	CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
	std::string fn = GetCampaignDir( g_pProfileMgr->GetCurrentProfileName( ), 
		pUserProfile->m_ServerGameOptions.m_eGameType );
	fn += campaignName; 
	fn += ".txt"; 


	remove(fn.c_str());


	if (stricmp(campaignName.c_str(),m_pProfile->m_ServerGameOptions.GetCampaignName()) == 0)
	{
		SetCampaignName(DEFAULT_CAMPAIGN);
	}

	CreateCampaignList();

}

void CScreenHostMission::NewCampaign(const std::string& campaignName)
{
	CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
	std::string fn = GetCampaignDir( g_pProfileMgr->GetCurrentProfileName( ), 
		pUserProfile->m_ServerGameOptions.m_eGameType );

	if( !CWinUtil::DirExist( fn.c_str( )))
	{
		if( !CWinUtil::CreateDir( fn.c_str( )))
		{
			//TODO: error message
			return;
		}
	}
	
	
	fn += campaignName; 
	fn += ".txt";

	if (CWinUtil::FileExist(fn.c_str()))
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox(IDS_CAMPAIGN_EXISTS,&mb);
		return;
	}


	char szString[256];
	char szNum[4];

	CWinUtil::WinWritePrivateProfileString( "MissionList", "LoopMissions", "0", fn.c_str());

	CWinUtil::WinWritePrivateProfileString( "MissionList", "MissionSourceFile", g_pMissionButeMgr->GetAttributeFile(), fn.c_str());

	uint16 nCount = 0;
	bool bAdd =false;
	for (int n = 0; n < g_pMissionButeMgr->GetNumMissions(); n++)
	{
		MISSION *pMission = g_pMissionButeMgr->GetMission(n);
		if (pMission)
		{
			char szWorldTitle[MAX_PATH] = "";
			_splitpath( pMission->aLevels[0].szLevel, NULL, NULL, szWorldTitle, NULL );

			bAdd = false;
			switch (g_pGameClientShell->GetGameType())
			{
			case eGameTypeDeathmatch:
			case eGameTypeTeamDeathmatch:
				if (strnicmp(szWorldTitle,"DM_",3) == 0)
				{
					bAdd = true;
				}
				break;
			case eGameTypeDoomsDay:
				if (strnicmp(szWorldTitle,"DD_",3) == 0)
				{
					bAdd = true;
				}
				break;
			default:
				bAdd = true;
				break;
			}

			if (bAdd)
			{
				sprintf(szString,"Mission%d",nCount);
				sprintf(szNum,"%d",n);
				CWinUtil::WinWritePrivateProfileString( "MissionList", szString, szNum, fn.c_str());
				nCount++;
			}
		}
	}

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, fn.c_str());

	SetCampaignName(campaignName.c_str());

	CreateCampaignList();

	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_LEVELS);


}

void CScreenHostMission::RenameCampaign(const std::string& oldName,const std::string& newName)
{
	if (!newName.length()) return;


	CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
	char const* pszCampaignDir = GetCampaignDir( g_pProfileMgr->GetCurrentProfileName( ), 
		pUserProfile->m_ServerGameOptions.m_eGameType );
	std::string ofn = pszCampaignDir;
	ofn += oldName; 
	ofn += ".txt"; 

	std::string nfn = pszCampaignDir;
	nfn += newName; 
	nfn += ".txt"; 


	if (CWinUtil::FileExist(nfn.c_str()))
	{
		MBCreate mb;
		mb.eType = LTMB_OK;
		g_pInterfaceMgr->ShowMessageBox(IDS_CAMPAIGN_EXISTS,&mb);
		return;
	}


	rename(ofn.c_str(),nfn.c_str());

	if (oldName.compare(m_pProfile->m_ServerGameOptions.GetCampaignName()) == 0)
	{
		SetCampaignName(newName.c_str());
	}

	CreateCampaignList();


}


void CScreenHostMission::SetCampaignName(const char* newName)
{
	if (stricmp(m_pProfile->m_ServerGameOptions.GetCampaignName(),newName) == 0)
		return;

	m_pProfile->m_ServerGameOptions.SetCampaignName(newName);

	UpdateCampaignName();

};
