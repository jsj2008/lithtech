// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSave.cpp
//
// PURPOSE : Interface screen for saving games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionDB.h"
#include "ScreenSave.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"

#include "sys/win/mpstrconv.h"

#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = SLMGR_MAX_SAVE_SLOTS;
	int	 g_nSaveSlot = -1;
	int	 g_nSaveIndex = -1;
	void OverwriteCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OVERWRITE,0,0);
	}
	CLTGUIColumnCtrl* g_pColCtrl = NULL;

	wchar_t g_wszOldName[40]	= L"";
	wchar_t g_wszOldTime[40]	= L"";

	int kNameWidth = 0;
	int kTimeWidth = 0;
	int kIndent = 0;
	int kSmallFontSize = 4;
	const char* pszFont = NULL;

	void EditCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)pUserData;

		if(pThisScreen)
		{
			if (bReturn)
			{
				pThisScreen->SendCommand(CMD_EDIT_NAME,(uint32)pData,0);
			}
			else
			{
				//the user cancelled we need to restore the original text, which has been saved in the
				//globals above.
				if(g_pColCtrl)
				{
					g_pColCtrl->SetString(1, g_wszOldName );
					g_pColCtrl->SetString(2, g_wszOldTime );
				}
			}
		}
	};

	wchar_t SaveNameFilter(wchar_t c, uint32 nPos)
	{
		if (c == '|')
			return NULL;
		else
			return c;
	}


}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSave::CScreenSave()
{
}

CScreenSave::~CScreenSave()
{

}

bool CScreenSave::Build()
{
	CreateTitle("IDS_TITLE_SAVEGAME");

	kNameWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	kTimeWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);
	kSmallFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	pszFont = g_pLayoutDB->GetListFont(m_hLayout,0);
	kIndent = 2 * kSmallFontSize;

	if (!CBaseScreen::Build())
	{
		return false;
	} 

	m_DefaultPos = m_ScreenRect.m_vMin;
	return true;
}


void CScreenSave::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_vfLastScale.Init();
		BuildSavedLevelList();
		SetSelection(0);
		UseBack(true);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
		UseBack(false);
	}
	CBaseScreen::OnFocus(bFocus);
}

uint32 CScreenSave::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM && dwCommand <= CMD_CUSTOM+kMaxSave)
	{
        uint32 slot = dwCommand - CMD_CUSTOM;

		g_nSaveSlot = slot;
		g_nSaveIndex = (int)dwParam1;

		if (g_pClientSaveLoadMgr->SlotSaveExists( g_nSaveSlot ))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = OverwriteCallBack,
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_CONFIRMSAVE",&mb);
			return 1;
		}
		if (g_nSaveSlot > 0)
		{
			m_wszSaveName[0] = NULL;		
			NameSaveGame(g_nSaveSlot,g_nSaveIndex);
		}
		else if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox("IDS_SAVEGAMEFAILED",&mb);
		}


	}
	else if (dwCommand == CMD_OVERWRITE)
	{
		NameSaveGame(g_nSaveSlot,g_nSaveIndex);
	}
	else if (dwCommand == CMD_EDIT_NAME)
	{
		LTStrCpy(m_wszSaveName, (wchar_t*)dwParam1, LTARRAYSIZE(m_wszSaveName));

		ForceMouseUpdate();
		if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox("IDS_SAVEGAMEFAILED",&mb);
			return 0;
		}
	}

	else
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

void CScreenSave::BuildSavedLevelList()
{
	uint32 nFontSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);
	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kNameWidth,nFontSize);

	cs.nCommandID = CMD_CUSTOM;
	cs.szHelpID = "IDS_HELP_QUICKSAVE";
	AddTextItem("IDS_QUICKSAVE",cs);

	int nFirstEmpty = -1;
	
	cs.nCommandID = NULL;
	cs.szHelpID = "";
	AddTextItem("IDS_LOAD_USERGAME", cs, true);

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect.m_vMin = m_DefaultPos;
	listCs.rnBaseRect.Left() += kIndent;
	listCs.rnBaseRect.Right() = (m_ScreenRect.Right() - 32);
	listCs.rnBaseRect.Bottom() = (m_ScreenRect.Bottom());
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 

	m_pList = AddList(listCs);

	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pList->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	m_pList->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
	m_pList->SetScrollWrap(false);



 	for (int i = 0; i < kMaxSave; i++)
	{
		bool bEmpty = true;

		if (g_pClientSaveLoadMgr->SlotSaveExists( i+1 ))
		{
			struct tm* pTimeDate = NULL;
			char strTime[128] = "";

			wchar_t wszSaveTitle[SLMGR_MAX_INISTR_LEN+1];
			char szWorldName[MAX_PATH*2];
			time_t saveTime;

			if( g_pClientSaveLoadMgr->ReadSaveINI( g_pClientSaveLoadMgr->GetSlotSaveKey( i+1 ), 
				wszSaveTitle, LTARRAYSIZE( wszSaveTitle ), szWorldName, LTARRAYSIZE( szWorldName ), &saveTime ))
			{
				pTimeDate = localtime (&saveTime);
				if (pTimeDate)
				{
					if (g_pInterfaceResMgr->IsEnglish())
					{
						LTSNPrintF( strTime, LTARRAYSIZE( strTime ), "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}
					else
					{
						LTSNPrintF( strTime, LTARRAYSIZE( strTime ), "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}
				}
			}


			wchar_t *pWorld = NULL;
			if ( wszSaveTitle[0] )
			{
				pWorld = wszSaveTitle;
				bEmpty = false;
			}
			else if ( szWorldName[0] )
			{
				LTStrCpy( wszSaveTitle, MPA2W( szWorldName ).c_str( ), LTARRAYSIZE( wszSaveTitle ));
				pWorld = wszSaveTitle;
				bEmpty = false;
			}


			if (pWorld)
			{
				CLTGUICtrl_create cs;
				cs.rnBaseRect.m_vMin.Init();
				cs.rnBaseRect.m_vMax = LTVector2n(kNameWidth+kTimeWidth,kSmallFontSize);
				cs.nCommandID = CMD_CUSTOM+1+i;
				cs.pCommandHandler = this;
				cs.szHelpID = "IDS_HELP_SAVEGAME";

				CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(cs,false,pszFont,kSmallFontSize );

				// This is a spacer
				pCtrl->AddColumn(L"", kIndent, true);

				// The world name column
				pCtrl->AddColumn(pWorld, kNameWidth);

				if( strTime[0] != '\0' )
				{
					// The column that contains the date/time
					pCtrl->AddColumn(MPA2W(strTime).c_str(), kTimeWidth);
				}

				uint32 nIndex = m_pList->AddControl(pCtrl);
				pCtrl->SetParam1( nIndex );
			}


		}


		if (bEmpty && nFirstEmpty < 0) 
			nFirstEmpty = i;

	}

	if (nFirstEmpty >= 0)
	{
		std::wstring wsTmp;

		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(kNameWidth+kTimeWidth,kSmallFontSize);
		cs.nCommandID = CMD_CUSTOM+1+nFirstEmpty;
		cs.pCommandHandler = this;
		cs.szHelpID = "IDS_HELP_SAVEGAME";

		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(cs,false,pszFont,kSmallFontSize );
		// This is a spacer
		pCtrl->AddColumn(L"", kIndent, true);

		// The world name column
		pCtrl->AddColumn(LoadString("IDS_EMPTY"), kNameWidth);


		// The column that contains the date/time
		pCtrl->AddColumn(L"", kTimeWidth);

		uint32 nIndex = m_pList->AddControl(pCtrl);
		pCtrl->SetParam1( nIndex );
	}

	LTVector2n listpos = m_pList->GetBasePos();
	uint32 listH = m_pList->GetBaseHeight();
	uint32 listW = m_pList->GetBaseWidth();
	uint16 nLast = m_pList->GetNumControls() - 1;
	m_pList->CalculatePositions();
	CLTGUICtrl *pCtrl = m_pList->GetControl(nLast);
	if (pCtrl)
	{
		LTVector2n pos = pCtrl->GetBasePos();
		pos.y += pCtrl->GetBaseHeight();
		listH = (pos.y - listpos.y) + 8;

		m_pList->SetSize( LTVector2n(listW,listH) );

	}

}

void CScreenSave::ClearSavedLevelList()
{
	RemoveAll();

}

bool CScreenSave::SaveGame(uint32 slot)
{
	if (slot > 0)
	{
		if( !g_pClientSaveLoadMgr->SaveGameSlot( slot, m_wszSaveName ))
			return false;
		}
	else
	{
		if( !g_pClientSaveLoadMgr->QuickSave() )
			return false;
	}

//	g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_GAMESAVED);
	g_pInterfaceMgr->ChangeState(GS_PLAYING);
	return true;
}


bool CScreenSave::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	if (key == VK_F6)
	{
		SendCommand(CMD_CUSTOM,0,0);
        return true;
	}
#endif // !PLATFORM_XENON

    return CBaseScreen::HandleKeyDown(key,rep);

}


void CScreenSave::NameSaveGame(uint32 slot, int index)
{
	LTStrCpy(m_wszSaveName, MPA2W( g_pMissionMgr->GetCurrentWorldName()).c_str( ), LTARRAYSIZE( m_wszSaveName ));

	if (!g_pMissionMgr->IsCustomLevel())
	{
		int missionNum = g_pMissionMgr->GetCurrentMission();
		int sceneNum = g_pMissionMgr->GetCurrentLevel();
		HRECORD hMission = g_pMissionDB->GetMission(missionNum);
		if (hMission)
		{
			const char* szMissionId = g_pMissionDB->GetString(hMission,MDB_Name);
			const wchar_t* wszMission = LoadString( szMissionId );
			const wchar_t* wszLevel = NULL;
			HRECORD hLevel = g_pMissionDB->GetLevel(missionNum,sceneNum);
			if (hLevel)
			{
				wszLevel = LoadString(g_pMissionDB->GetString(hLevel,MDB_Name));
			}

			if( wszLevel )
			{
				// strip whitespace
				while( (*wszLevel == ' ' || *wszLevel == '\t') && *wszLevel != '\0' )
					wszLevel++;
				LTStrCpy( m_wszSaveName, wszLevel, LTARRAYSIZE( m_wszSaveName ));
			}
			else if( wszMission )
				LTStrCpy( m_wszSaveName, wszMission, LTARRAYSIZE( m_wszSaveName ));
		}
	}
	

	g_pColCtrl = (CLTGUIColumnCtrl*)m_pList->GetControl(index);
	if (!g_pColCtrl)
		return;

	LTStrCpy(g_wszOldName, g_pColCtrl->GetColumn(1)->GetString(), LTARRAYSIZE(g_wszOldName));
	LTStrCpy(g_wszOldTime, g_pColCtrl->GetColumn(2)->GetString(), LTARRAYSIZE(g_wszOldTime));

	g_pColCtrl->SetString(1, L"  ");
	g_pColCtrl->SetString(2, L"  ");

	//show edit box here	
	MBCreate mb;
	mb.eType = LTMB_EDIT;
	mb.pFn = EditCallBack;
	mb.pUserData = this;
	mb.pFilterFn = SaveNameFilter;
	mb.pString = m_wszSaveName;
	mb.nMaxChars = LTARRAYSIZE( m_wszSaveName );
	g_pInterfaceMgr->ShowMessageBox("IDS_ENTER_NAME",&mb);

}
