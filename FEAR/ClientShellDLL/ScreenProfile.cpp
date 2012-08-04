// ScreenProfile.cpp: implementation of the CScreenProfile class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ScreenProfile.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "NetDefs.h"
#include "WinUtil.h"

#include "GameClientShell.h"
#include "profileMgr.h"

#include "sys/win/mpstrconv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	char	szOldFN[MAX_PROFILENAME_LEN];

	bool bCreate = false;

	void DeleteCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_DELETE);
	}
	void CreateCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_CREATE);
	}
	void LoadCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_CONFIRM,(uint32)pData,CMD_LOAD);
	}
	void EditCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenProfile *pThisScreen = (CScreenProfile *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EDIT,(uint32)pData,0);
	};

	int kDlgHt = 240;
	int kDlgWd = 240;
	uint32 nCommand = 0;
	std::string sListFont;
	uint32 nListFontSz = 14;

}

CScreenProfile::CScreenProfile()
{
	m_pCurrent = NULL;
	m_pLoad = NULL;
	m_pDelete = NULL;
	m_pRename = NULL;
	m_pProfile = NULL;

}

CScreenProfile::~CScreenProfile()
{
}

// Build the screen
bool CScreenProfile::Build()
{
	// Set the title's text
	CreateTitle("IDS_TITLE_PROFILE");


	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	m_pCurrent = AddTextItem(L" ", cs, true);

	cs.nCommandID = CMD_CREATE;
	cs.szHelpID = "IDS_HELP_PROFILE_CREATE";
	AddTextItem("IDS_CREATE", cs );

	cs.nCommandID = CMD_LOAD;
	cs.szHelpID = "IDS_HELP_PROFILE_LOAD";
	AddTextItem("IDS_LOAD", cs);

	cs.nCommandID = CMD_DELETE;
	cs.szHelpID = "IDS_HELP_PROFILE_DELETE";
	m_pDelete = AddTextItem("IDS_DELETE", cs);


	LTRect2n dlgRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenDialogRect);
	std::string sDlgFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenDialogFont);
	uint8 nDlgFontSz = ( uint8 )g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenDialogSize);

	sListFont = g_pLayoutDB->GetListFont(m_hLayout,0);
	nListFontSz = g_pLayoutDB->GetListSize(m_hLayout,0);

	CLTGUICtrl_create dcs;
	dcs.rnBaseRect = dlgRect;
	m_pDlg = debug_new(CLTGUIWindow);
	m_pDlg->Create(TextureReference(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenDialogFrame)),dcs);

	LTVector2n tmp(8,8);
	cs.nCommandID = NULL;
	cs.szHelpID = "";
	CLTGUITextCtrl *pCtrl = CreateTextItem("IDS_PROFILE_LIST", cs, true, sDlgFont.c_str(), nDlgFontSz);
	m_pDlg->AddControl(pCtrl, tmp);

	tmp.y += 24;

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect.m_vMin.Init();
	listCs.rnBaseRect.Right() = kDlgWd - 48;
	listCs.rnBaseRect.Bottom() = kDlgWd - 64;
	listCs.bArrows = true;
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	m_pListCtrl = CreateList(listCs);
	if (m_pListCtrl)
	{
		m_pListCtrl->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
		m_pListCtrl->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
		m_pListCtrl->Show(true);

		m_pDlg->AddControl(m_pListCtrl, tmp);
	}


	cs.nCommandID = CMD_CANCEL;
	cs.szHelpID = "";
	cs.rnBaseRect.Bottom() = nDlgFontSz;
	cs.rnBaseRect.Right() = kDlgWd / 2;

	pCtrl = CreateTextItem("IDS_CANCEL", cs, false, sDlgFont.c_str(), nDlgFontSz);
	pCtrl->SetAlignment(kCenter);
	tmp.x = kDlgWd / 2;
	tmp.y = (kDlgHt - nDlgFontSz) - 8;
	m_pDlg->AddControl(pCtrl, tmp);

	AddControl(m_pDlg);

	m_pDlg->Show(false);

	m_pDlg->SetSelection(1);

	UseBack(true);


	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenProfile::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case CMD_OK:
		{
			m_pDlg->Show(false);
			SetCapture(NULL);
			HandleDlgCommand(nCommand,(uint16)dwParam1);
			SetSelection(1);
		}
		break;
	case CMD_CANCEL:
		{
			m_pDlg->Show(false);
			SetCapture(NULL);
			SetSelection(1);
		}
		break;

	case CMD_LOAD:
		{
			m_pDlg->Show(true);
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
					ProfileArray::iterator iter = m_ProfileList.begin();
					for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

					if (iter != m_ProfileList.end())
					{
						g_pProfileMgr->DeleteProfile(iter->m_sFileName.c_str());

					}
					m_pProfile = g_pProfileMgr->GetCurrentProfile();
					LTStrCpy(m_szProfile, m_pProfile->m_sFriendlyName.c_str(), LTARRAYSIZE(m_szProfile));
					UpdateProfileName();

				}
			}
			else if (dwParam2 == CMD_LOAD)
			{
				if (nIndex < m_pListCtrl->GetNumControls()) 
				{
					ProfileArray::iterator iter = m_ProfileList.begin();
					for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

					if (iter != m_ProfileList.end())
					{
						g_pProfileMgr->NewProfile((*iter).m_sFileName.c_str(),L"",true);
						m_pProfile = g_pProfileMgr->GetCurrentProfile();

						LTStrCpy(m_szProfile, (*iter).m_sFriendlyName.c_str(), LTARRAYSIZE(m_szProfile));
						UpdateProfileName();
					}
				}
			}
			else if (dwParam2 == CMD_CREATE)
			{
				LTStrCpy(m_szOldProfile,m_szProfile, LTARRAYSIZE(m_szOldProfile));
				bCreate = true;

				//show edit box here	
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pUserData = this;
				mb.pString = g_pProfileMgr->GetCurrentProfile()->m_sFriendlyName.c_str();
				mb.nMaxChars = MAX_PROFILE_NAME;
//				mb.eInput = kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox("IDS_ENTER_PROFILE_NAME",&mb);
			}
		}
		break;
	case CMD_DELETE:
		{
			m_pDlg->Show(true);
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
				mb.pFn = CreateCallBack;
				mb.pUserData = this;
				g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRM_NEWPROFILE",&mb);
			}
			else
				SendCommand(CMD_CONFIRM,0,CMD_CREATE);
		}
		break;
	case CMD_RENAME:
		{
			m_pDlg->Show(true);
			SetCapture(m_pDlg);
			nCommand = dwCommand;
		}
		break;
	case CMD_EDIT:
		{
			std::wstring profileName = (wchar_t *)dwParam1;
			
			ProfileArray::iterator iter = m_ProfileList.begin();
			while (iter != m_ProfileList.end() && !LTStrIEquals( (*iter).m_sFriendlyName.c_str(), profileName.c_str() ) )
			{
				iter++;
			}

			if (iter != m_ProfileList.end())
			{
				//don't proceed renaming or creating a new one if we already have a profile of that
				//name
				MBCreate mb;
				mb.eType = LTMB_OK;
				mb.pFn = NULL,
				g_pInterfaceMgr->ShowMessageBox("IDS_PROFILE_ALREADY_EXISTS",&mb);
			}
			else
			{
				if (bCreate)
				{
					char szName[32] = "";
					uint32 n = 0;
					do 
					{
						LTSNPrintF(szName,LTARRAYSIZE(szName),"Profile%03d",n);
						n++;
					} while (CWinUtil::FileExist(GetAbsoluteProfileFile( g_pLTClient, szName)));
					g_pProfileMgr->NewProfile(szName, profileName.c_str(), false);
				}
				else
				{
					g_pProfileMgr->RenameProfile(szOldFN,profileName.c_str());
				}
				m_pProfile = g_pProfileMgr->GetCurrentProfile();
				LTStrCpy(m_szProfile,m_pProfile->m_sFriendlyName.c_str(), LTARRAYSIZE(m_szProfile));
				UpdateProfileName();
			}
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
		m_pDlg->Show(false);
		SetCapture(NULL);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else
		CBaseScreen::Escape();
}

bool	CScreenProfile::OnLButtonUp(int x, int y)
{
	return CBaseScreen::OnLButtonUp(x,y);
}

bool	CScreenProfile::OnRButtonUp(int x, int y)
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

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kDlgWd - 48,nListFontSz);
	cs.nCommandID = CMD_OK;

	// Profiles to the list
	for (ProfileArray::iterator iter = m_ProfileList.begin(); iter != m_ProfileList.end(); ++iter)
	{
		CLTGUITextCtrl* pTextCtrl = CreateTextItem( iter->m_sFriendlyName.c_str(), cs, false, sListFont.c_str(), nListFontSz);
		if (pTextCtrl)
		{
			uint32 ndx = m_pListCtrl->AddControl(pTextCtrl);
			pTextCtrl->SetParam1(ndx);

			if (LTStrIEquals(iter->m_sFriendlyName.c_str(),m_pProfile->m_sFriendlyName.c_str()))
				m_pListCtrl->SetSelection(ndx);

		}
	}

}

void CScreenProfile::UpdateProfileName()
{
	wchar_t wszBuffer[MAX_PROFILENAME_LEN + 64];
	FormatString( "IDS_CURRENT_PROFILE", wszBuffer, LTARRAYSIZE(wszBuffer), m_szProfile );

	m_pCurrent->SetString( wszBuffer );

	CreateProfileList();

}

void CScreenProfile::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_pProfile = g_pProfileMgr->GetCurrentProfile();
		LTStrCpy(m_szProfile,m_pProfile->m_sFriendlyName.c_str(), LTARRAYSIZE(m_szProfile));
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
			LTStrCpy(m_szOldProfile,m_szProfile, LTARRAYSIZE(m_szOldProfile));
			if (nIndex < m_pListCtrl->GetNumControls()) 
			{
				ProfileArray::iterator iter = m_ProfileList.begin();
				for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

				if (iter != m_ProfileList.end())
				{
					std::wstring profile = (*iter).m_sFriendlyName;
					if (g_pLTClient->IsConnected() && !LTStrIEquals(profile.c_str(), m_szProfile))
					{

						MBCreate mb;
						mb.eType = LTMB_YESNO;
						mb.pFn = LoadCallBack,
						mb.pData = (void *)nIndex;
						mb.pUserData = this;
						g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRM_NEWPROFILE",&mb);
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
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRM_DELETE_PROFILE",&mb);
		}
		break;
	case CMD_RENAME:
		LTStrCpy(m_szOldProfile,m_szProfile, LTARRAYSIZE(m_szOldProfile));
		if (nIndex < m_pListCtrl->GetNumControls()) 
		{
			ProfileArray::iterator iter = m_ProfileList.begin();
			for (int i = 0; i < nIndex && iter != m_ProfileList.end(); ++i, ++iter);

			if (iter != m_ProfileList.end())
			{
				LTStrCpy(szOldFN,(*iter).m_sFileName.c_str(), LTARRAYSIZE(szOldFN));
				LTStrCpy(m_szProfile, (*iter).m_sFriendlyName.c_str(), LTARRAYSIZE(m_szProfile));

				bCreate = false;
				MBCreate mb;
				mb.eType = LTMB_EDIT;
				mb.pFn = EditCallBack;
				mb.pString = m_szProfile;
				mb.nMaxChars = 16;
//				mb.eInput = kInputFileFriendly;
				g_pInterfaceMgr->ShowMessageBox("IDS_ENTER_PROFILE_NAME",&mb);
			}
		}
		break;
	}

};
