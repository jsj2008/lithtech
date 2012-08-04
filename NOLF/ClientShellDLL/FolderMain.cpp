// FolderMain.cpp: implementation of the CFolderMain class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMain.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"

extern CGameClientShell* g_pGameClientShell;
namespace
{
	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderMain *pThisFolder = (CFolderMain *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(FOLDER_CMD_EXIT,0,0);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMain::CFolderMain()
{
    m_BuildVersion = LTNULL;
    m_pResume = LTNULL;
}

CFolderMain::~CFolderMain()
{
    if (g_pLTClient && m_BuildVersion)
	{
        g_pLTClient->FreeString(m_BuildVersion);
	}

}


// Build the folder
LTBOOL CFolderMain::Build()
{

	SetTitleColor(SETRGB(192,224,128));

	// CreateTitle(IDS_TITLE_MAINMENU);

    LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"SinglePlayerPos");
	AddLink(IDS_SINGLEPLAYER,	FOLDER_CMD_SINGLE_PLAYER,	IDS_HELP_SINGLEPLAYER,	pos.x,	pos.y);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ResumePos");
	m_pResume = AddLink(IDS_CONTINUE_GAME, FOLDER_CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME, pos.x, pos.y);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"MultiPlayerPos");
	CLTGUITextItemCtrl* pCtrl = AddLink(IDS_MULTIPLAYER,	FOLDER_CMD_MULTI_PLAYER,	IDS_HELP_MULTIPLAYER,	pos.x,	pos.y);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"OptionsPos");
	AddLink(IDS_OPTIONS,FOLDER_CMD_OPTIONS,	IDS_HELP_OPTIONS,pos.x,	pos.y);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"QuitPos");
	AddLink(IDS_EXIT,FOLDER_CMD_QUIT,IDS_HELP_EXIT,	pos.x,	pos.y);

	m_BuildVersion = g_pLTClient->CreateString((char*)g_pVersionMgr->GetVersion());
	m_BuildPos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"VersionPos");

 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

    UseArrows(LTFALSE);
    UseBack(LTFALSE);

	return LTTRUE;

}

void CFolderMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetSelection(-1);
		m_pResume->RemoveAll();
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsInWorld() && hPlayerObj && !g_pGameClientShell->IsPlayerDead())
		{
			m_pResume->AddString(IDS_RESUME);
			m_pResume->SetHelpID(IDS_HELP_RESUME);
			m_pResume->CLTGUICtrl::Create(FOLDER_CMD_RESUME,0,0);
            m_pResume->Enable(LTTRUE);
		}
		else
		{
			m_pResume->AddString(IDS_CONTINUE_GAME);
			m_pResume->SetHelpID(IDS_HELP_CONTINUE_GAME);
			m_pResume->CLTGUICtrl::Create(FOLDER_CMD_CONTINUE_GAME,0,0);
			char strSaveGameSetting[256];
			memset (strSaveGameSetting, 0, 256);
			char strKey[32] = "Continue";
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
			m_pResume->Enable( (strlen (strSaveGameSetting) > 0) );
		}
	}
	CBaseFolder::OnFocus(bFocus);
}

CLTGUITextItemCtrl* CFolderMain::AddLink(int stringID, uint32 commandID, int helpID, int xpos, int ypos)
{
    HSTRING hTxt=g_pLTClient->FormatString(stringID);
	CLTGUITextItemCtrl* pCtrl= CreateTextItem(stringID,commandID,helpID);

    LTIntPt pos(xpos ,ypos);
	AddFixedControl(pCtrl,pos);

	return pCtrl;

}

uint32 CFolderMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_SINGLE_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_SINGLE);
			break;
		}
	case FOLDER_CMD_MULTI_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTIPLAYER);
			break;

		}
	case FOLDER_CMD_OPTIONS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_OPTIONS);
			break;
		}
	case FOLDER_CMD_QUIT:
		{
            HSTRING hString = g_pLTClient->FormatString(IDS_SUREWANTQUIT);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,QuitCallBack,this);
			g_pLTClient->FreeString(hString);
			break;
		}
	case FOLDER_CMD_EXIT:
		{
#ifdef _DEMO
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case FOLDER_CMD_RESUME:
		{
			Escape();
			break;
		}
	case FOLDER_CMD_CONTINUE_GAME:
		{
			char strSaveGameSetting[256];
			memset (strSaveGameSetting, 0, 256);
			char strKey[32] = "Continue";
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
			char* pWorldName = strtok(strSaveGameSetting,"|");
            char* pFileName = strtok(LTNULL,"|");
			if (g_pGameClientShell->LoadGame(pWorldName, pFileName))
			{
				g_pInterfaceMgr->ChangeState(GS_PLAYING);
				return 1;
			}
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Folder specific rendering
LTBOOL   CFolderMain::Render(HSURFACE hDestSurf)
{
	if (CBaseFolder::Render(hDestSurf))
	{
		CLTGUIFont* pFont = g_pInterfaceResMgr->GetHelpFont();

		pFont->Draw(m_BuildVersion, hDestSurf, m_BuildPos.x+g_pInterfaceResMgr->GetXOffset(), 
			m_BuildPos.y+g_pInterfaceResMgr->GetYOffset(), LTF_JUSTIFY_RIGHT, kBlack);

/* font test 
		pFont = g_pInterfaceResMgr->GetTitleFont();
        LTIntPt pos(320+g_pInterfaceResMgr->GetXOffset(),50+g_pInterfaceResMgr->GetYOffset());
		int ht = pFont->GetHeight();
		pFont->Draw("! \" # $ % & ' ( ) *",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("+ , - . / 0 1 2 3 4",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("5 6 7 8 9 : ; < = > ?",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("@ A B C D E F G H",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("I J K L M N O P Q",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("R S T U V W X Y Z",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("[ \\ ] # _ ` a b c",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("d e f g h i j k l",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("m n o p q r s t u",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
		pos.y += ht;
		pFont->Draw("v w x y z { | } ~",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT);
/*
		pFont = g_pInterfaceResMgr->GetMsgForeFont();
        pos = LTIntPt(320+g_pInterfaceResMgr->GetXOffset(),50+g_pInterfaceResMgr->GetYOffset());
		ht = pFont->GetHeight();
		pFont->Draw("! \" # $ % & ' ( ) *",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("+ , - . / 0 1 2 3 4",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("5 6 7 8 9 : ; < = > ?",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("@ A B C D E F G H",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("I J K L M N O P Q",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("R S T U V W X Y Z",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("[ \\ ] # _ ` a b c",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("d e f g h i j k l",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("m n o p q r s t u",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);
		pos.y += ht;
		pFont->Draw("v w x y z { | } ~",hDestSurf,pos.x,pos.y,LTF_JUSTIFY_LEFT, kWhite);

//	*/
        return LTTRUE;
	}

    return LTFALSE;

}


void CFolderMain::Escape()
{
	//quit or return to game?
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (g_pGameClientShell->IsInWorld() && hPlayerObj && 
		(!g_pGameClientShell->IsPlayerDead() || g_pGameClientShell->GetGameType() != SINGLE))
	{
		g_pInterfaceMgr->ChangeState(GS_PLAYING);
	}

//	CBaseFolder::Escape();
}

/******************************************************************/

LTBOOL CFolderMain::OnUp()
{
	int select;
	if (m_nSelection == kNoSelection)
		select = -1;
	else
		select = FixedIndex(m_nSelection);

	do
	{
		select--;

		if (select < 0)
			select = (int)m_fixedControlArray.GetSize()-1;
	} while (!m_fixedControlArray[select]->IsEnabled());

	SetSelection(FixedIndex(select));

    return LTTRUE;
}

/******************************************************************/

LTBOOL CFolderMain::OnDown()
{
	int select;
	if (m_nSelection == kNoSelection)
		select = -1;
	else
		select = FixedIndex(m_nSelection);
	do
	{
		select++;

		if (select >= (int)m_fixedControlArray.GetSize())
			select = 0;
	} while (!m_fixedControlArray[select]->IsEnabled());

	SetSelection(FixedIndex(select));

    return LTTRUE;
}
