// ScreenProfile.cpp: implementation of the CScreenProfile class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ScreenProfile.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "NetDefs.h"

#include "GameClientShell.h"
#include "profileMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	char	szOldFN[MAX_PROFILE_LEN];

	LTBOOL bCreate = LTFALSE;

	void DeleteCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PROFILE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_DELETE);
	}
	void CreateCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PROFILE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_CREATE);
	}
	void LoadCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PROFILE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_LOAD);
	}
	void EditCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PROFILE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EDIT,(uint32)pData,0);
	};

	int kDlgHt = 240;
	int kDlgWd = 240;
	uint32 nCommand = 0;

}

CScreenProfile::CScreenProfile()
{
	m_pCurrent = LTNULL;
	m_pLoad = LTNULL;
	m_pDelete = LTNULL;
	m_pRename = LTNULL;
	m_pProfile = LTNULL;

}

CScreenProfile::~CScreenProfile()
{
}

// Build the screen
LTBOOL CScreenProfile::Build()
{
	// Set the title's text
	CreateTitle(IDS_TITLE_PROFILE);

	// Get edit controls position and create it.
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"ProfileNamePos");
	m_pCurrent = AddTextItem(" ", LTNULL, LTNULL, pos, LTTRUE);

	AddTextItem(IDS_CREATE, CMD_CREATE, IDS_HELP_PROFILE_CREATE);

	AddTextItem(IDS_LOAD, CMD_LOAD, IDS_HELP_PROFILE_LOAD);

	m_pRename = AddTextItem(IDS_RENAME, CMD_RENAME, IDS_HELP_PROFILE_RENAME);

	m_pDelete = AddTextItem(IDS_DELETE, CMD_DELETE, IDS_HELP_PROFILE_DELETE);



	char szBack[128] = "";
	g_pLayoutMgr->GetDialogFrame(szBack,sizeof(szBack));

	m_pDlg = debug_new(CLTGUIWindow);
	m_pDlg->Create(g_pInterfaceResMgr->GetTexture(szBack),kDlgHt,kDlgWd);

	LTIntPt tmp(8,8);

	CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_PROFILE_LIST, LTNULL, LTNULL, kDefaultPos, LTTRUE);
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

		m_pDlg->AddControl(m_pListCtrl, tmp);

	}

	pCtrl = CreateTextItem(IDS_CANCEL, CMD_CANCEL, LTNULL);
	pCtrl->SetScale(1.0f);
	tmp.x = (kDlgWd - pCtrl->GetWidth()) / 2;
	tmp.y = (kDlgHt - pCtrl->GetHeight()) - 8;
	m_pDlg->AddControl(pCtrl, tmp);

	pos.x = 320 - m_pDlg->GetWidth() / 2;
	pos.x = 240 - m_pDlg->GetHeight() / 2;


	m_pDlg->SetBasePos(pos);
	m_pDlg->SetScale(g_pInterfaceResMgr->GetXRatio());
	m_pDlg->Show(LTFALSE);

	m_pDlg->SetSelection(1);
	AddControl(m_pDlg);

	UseBack(LTTRUE);


	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenProfile::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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

	case CMD_LOAD:
		{
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
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					StringSet::iterator iter = m_ProfileList.begin();
					for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

					if (iter != m_ProfileList.end())
					{
						std::string profile = *iter;
						g_pProfileMgr->DeleteProfile(profile);

					}
					m_pProfile = g_pProfileMgr->GetCurrentProfile();
					SAFE_STRCPY(m_szProfile, m_pProfile->m_sName.c_str());
					UpdateProfileName();

				}
			}
			else if (dwParam2 == CMD_LOAD)
			{
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					StringSet::iterator iter = m_ProfileList.begin();
					for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

					if (iter != m_ProfileList.end())
					{
						std::string profile = *iter;
						g_pProfileMgr->NewProfile(profile);
						m_pProfile = g_pProfileMgr->GetCurrentProfile();

						SAFE_STRCPY(m_szProfile, profile.c_str());
						UpdateProfileName();
					}
				}
			}
			else if (dwParam2 == CMD_CREATE)
			{
				SAFE_STRCPY(m_szOldProfile,m_szProfile);
				bCreate = LTTRUE;

				//show edit box here	
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = m_szProfile;
				mb.nMaxChars = MAX_PROFILE_NAME;
				mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENTER_PROFILE_NAME,&mb);
			}
		}
		break;
	case CMD_DELETE:
		{
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
			m_pDlg->Show(LTTRUE);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_EDIT:
		{
			SAFE_STRCPY(m_szProfile,(char *)dwParam1);
			std::string profileName = m_szProfile;
			StringSet::iterator iter = m_ProfileList.find(profileName);
			if (iter != m_ProfileList.end())
				profileName = (*iter);

			if (bCreate)
			{
				g_pProfileMgr->NewProfile(profileName);
			}
			else
			{
				g_pProfileMgr->RenameProfile(szOldFN,profileName);
			}
			m_pProfile = g_pProfileMgr->GetCurrentProfile();
			SAFE_STRCPY(m_szProfile,m_pProfile->m_sName.c_str());
			UpdateProfileName();
		}
		break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CScreenProfile::Escape()
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

LTBOOL	CScreenProfile::OnLButtonUp(int x, int y)
{
	return CBaseScreen::OnLButtonUp(x,y);
}

LTBOOL	CScreenProfile::OnRButtonUp(int x, int y)
{
	return CBaseScreen::OnRButtonUp(x,y);
}

void CScreenProfile::CreateProfileList()
{
	// Empty the list
	m_pListCtrl->RemoveAll();

	// Get new stuff
	m_ProfileList.clear();
	g_pProfileMgr->GetProfileList(m_ProfileList);

	// Profiles to the list
	for (StringSet::iterator iter = m_ProfileList.begin(); iter != m_ProfileList.end(); ++iter)
	{
		char *pStr = (char *)iter->c_str();
		CLTGUITextCtrl* pTextCtrl = CreateTextItem(pStr, CMD_OK, LTNULL);
		uint16 ndx = m_pListCtrl->AddControl(pTextCtrl);
		pTextCtrl->SetParam1(ndx);

		if (stricmp(iter->c_str(),m_pProfile->m_sName.c_str()) == 0)
			m_pListCtrl->SetSelection(ndx);
	}

}

void CScreenProfile::UpdateProfileName()
{
	m_pCurrent->SetString(FormatTempString(IDS_CURRENT_PROFILE,m_szProfile));

	CreateProfileList();

}

void CScreenProfile::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		SAFE_STRCPY(m_szProfile,m_pProfile->m_sName.c_str());
		UpdateProfileName();

	}
	CBaseScreen::OnFocus(bFocus);
}



void CScreenProfile::HandleDlgCommand(uint32 dwCommand, uint16 nIndex)
{
	switch (dwCommand)
	{
	case CMD_LOAD:
		{
			SAFE_STRCPY(m_szOldProfile,m_szProfile);
			if (nIndex < m_pListCtrl->GetNumControls()) 
			{
				StringSet::iterator iter = m_ProfileList.begin();
				for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

				if (iter != m_ProfileList.end())
				{
					std::string profile = *iter;
					if (g_pLTClient->IsConnected() && stricmp(profile.c_str(), m_szProfile))
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
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = DeleteCallBack,
			mb.pData = (void *)nIndex;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRM_DELETE,&mb);
		}
		break;
	case CMD_RENAME:
		SAFE_STRCPY(m_szOldProfile,m_szProfile);
		if (nIndex < m_pListCtrl->GetNumControls()) 
		{
			StringSet::iterator iter = m_ProfileList.begin();
			for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

			if (iter != m_ProfileList.end())
			{
				std::string profile = *iter;
				SAFE_STRCPY(szOldFN,profile.c_str());
				SAFE_STRCPY(m_szProfile, profile.c_str());
				bCreate = LTFALSE;
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = m_szProfile;
				mb.nMaxChars = 16;
				mb.eInput = CLTGUIEditCtrl::kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENTER_PROFILE_NAME,&mb);
			}
		}
		break;
	}

};
