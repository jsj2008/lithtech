// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostLevels.cpp
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostLevels.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "VarTrack.h"
#include "NetDefs.h"
#include "profileMgr.h"
#include "ClientConnectionMgr.h"
#include "WinUtil.h"
#include "sys/win/mpstrconv.h"
#include "ltprofileutils.h"
#include "GameClientShell.h"
#include "GameModeMgr.h"

namespace
{
	uint32 nListFontSize = 12;
	int32 nAvailWidth = 0;
	int32 nSelWidth = 0;
}

void AddCallBack(bool bReturn, void *pData, void* pUserData)
{
	CScreenHostLevels *pThisScreen = (CScreenHostLevels *)pUserData;
	if (bReturn && pThisScreen)
	{
		int	nMission = (int)pData;
		pThisScreen->AddMissionToList(nMission,false,true);
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostLevels::CScreenHostLevels() :
	m_pAvailMissionsScrollBar(NULL),
	m_pSelMissionsScrollBar(NULL)
{
    m_pAvailMissions	= NULL;
    m_pSelMissions    = NULL;
	m_pAdd			= NULL;
	m_pRemove		= NULL;
	m_pAddAll		= NULL;
	m_pRemoveAll    = NULL;

}

CScreenHostLevels::~CScreenHostLevels()
{
	Term();
}

// Build the screen
bool CScreenHostLevels::Build()
{
	LTRect2n rcAvailRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	LTRect2n rcSelRect = g_pLayoutDB->GetListRect(m_hLayout,1);
	uint32 nOffset = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize) + 4;

	LTVector2n addPos = rcAvailRect.GetTopLeft();
	addPos.y -= nOffset;
	LTVector2n removePos = rcSelRect.GetTopLeft();
	removePos.y -= nOffset;
	
	LTVector2n commandPos = rcAvailRect.GetTopRight();
	commandPos.x += nOffset;
	commandPos.x += g_pLayoutDB->GetScrollBarSize();
	int32 nCommandWidth = rcSelRect.Left() - commandPos.x;

	nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

	CreateTitle("IDS_TITLE_HOST_MISSIONS");
	//background frame
	CLTGUICtrl_create frameCs;
	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));

	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame, frameCs);
	AddControl(pFrame);

	

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nCommandWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_ADD_LEVEL;
	cs.szHelpID = "IDS_HELP_ADD_MISSION";
	m_pAdd = AddTextItem("IDS_HOST_ADD_MISSION", cs, true);
	m_pAdd->SetBasePos(addPos);

	m_DefaultPos = commandPos;
	cs.nCommandID = CMD_ADD_ALL;
	cs.szHelpID = "IDS_HELP_ADD_ALL_LEVELS";
	m_pAddAll = AddTextItem("IDS_HOST_ADD_ALL_LEVELS", cs);

	cs.nCommandID = CMD_REMOVE_ALL;
	cs.szHelpID = "IDS_HELP_REM_ALL_LEVELS";
	m_pRemoveAll = AddTextItem("IDS_HOST_REMOVE_ALL_LEVELS", cs);

	cs.nCommandID = CMD_REMOVE_LEVEL;
	cs.szHelpID = "IDS_HELP_REM_MISSION";
	m_pRemove = AddTextItem("IDS_HOST_REMOVE_MISSION", cs, true);
	m_pRemove->SetBasePos(removePos);

	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();

		m_pAvailMissionsScrollBar = CreateScrollBar( csb );
		if( m_pAvailMissionsScrollBar )
		{
			m_pAvailMissionsScrollBar->SetFrameWidth( 1 );
			m_pAvailMissionsScrollBar->Enable( true );
			m_pAvailMissionsScrollBar->Show( true );
		}
	}
	
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,1);
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();

		m_pSelMissionsScrollBar = CreateScrollBar( csb );
		if( m_pSelMissionsScrollBar )
		{
			m_pSelMissionsScrollBar->SetFrameWidth( 1 );
			m_pSelMissionsScrollBar->Enable( true );
			m_pSelMissionsScrollBar->Show( true );
		}
	}

	//	int32 nListHeight = rcAvailRect.GetHeight();
	nAvailWidth = rcAvailRect.GetWidth();

	CLTGUIListCtrlEx_create listCs;
	listCs.rnBaseRect = rcAvailRect;
	listCs.nTextIdent = g_pLayoutDB->GetListIndent(m_hLayout,0).x;
	listCs.pScrollBar = m_pAvailMissionsScrollBar;

	m_pAvailMissions = AddListEx(listCs);
	m_pAvailMissions->SetFrameWidth( 1 );
	m_pAvailMissions->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pAvailMissions->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));

	if( m_pAvailMissionsScrollBar )
		AddControl( m_pAvailMissionsScrollBar );

	nSelWidth = rcSelRect.GetWidth();
	listCs.rnBaseRect = rcSelRect;
	listCs.pScrollBar = m_pSelMissionsScrollBar;

	m_pSelMissions = AddListEx(listCs);
	m_pSelMissions->SetFrameWidth( 1 );
	m_pSelMissions->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,1));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,0));
	hSelFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,1));
	m_pSelMissions->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,1));

	if( m_pSelMissionsScrollBar )
		AddControl( m_pSelMissionsScrollBar );

 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	UseBack(true,true);

	return true;

}



uint32 CScreenHostLevels::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_ADD_LEVEL:
		{
			if (m_pAvailMissions->GetSelectedIndex() >= 0 && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS))
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetSelectedControl();
				if (pCtrl)
				{
					AddMissionToList(pCtrl->GetParam1(),true,false);
					m_pSelMissions->ClearSelection();
				}
			}
			UpdateButtons();
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailMissions->GetNumControls())
			{
				for (uint32 i = 0; i < m_pAvailMissions->GetNumControls() && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS); i++)
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetControl(i);
					if (pCtrl)
					{
						AddMissionToList(pCtrl->GetParam1(),false,false);
					}
				}
			}
			m_pSelMissions->ClearSelection();
			UpdateButtons();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			int nIndex = m_pSelMissions->GetSelectedIndex();
			if (nIndex >= 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveControl(nIndex);
				int numLeft = m_pSelMissions->GetNumControls();
				if (numLeft > 0)
				{
					if (nIndex >= numLeft)
						nIndex = numLeft-1;
					m_pSelMissions->SetSelection(nIndex);
				}
			}
			UpdateButtons();

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelMissions->GetNumControls() > 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveAll();
			}
			UpdateButtons();
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenHostLevels::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
		m_setRequiredMapFeatures.clear();
		DelimitedStringToStringContainer(gameModeMgr.m_grsRequiredMapFeatures.GetValue(),m_setRequiredMapFeatures,",");

		FillAvailList();
		LoadMissionList();

		if (!m_pSelMissions->GetNumControls())
		{
			MakeDefaultMissionList();
		}

		UpdateButtons();
        UpdateData(false);

	}
	else
	{
		UpdateData();

		SaveMissionList();

		m_pAvailMissions->RemoveAll();
		m_pSelMissions->RemoveAll();
	}
	CBaseScreen::OnFocus(bFocus);

}

bool CScreenHostLevels::FillAvailList()
{
	// Sanity checks...

    if (!m_pAvailMissions) return(false);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nAvailWidth,nListFontSize);
	cs.nCommandID = CMD_ADD_LEVEL;
	cs.pCommandHandler = this;


	for (uint32 nMission = 0; nMission < g_pMissionDB->GetNumMissions(); nMission++)
	{
		HRECORD hMission = g_pMissionDB->GetMission(nMission);
		HRECORD	hLevel = g_pMissionDB->GetLevel(hMission,0);
		bool bValidForGameMode = true;
		if (hMission && hLevel)
		{
			if (!g_pMissionDB->CheckMPLevelRequirements(hMission,m_setRequiredMapFeatures))
			{
				continue;
			}

			const char* szNameId = g_pMissionDB->GetString(hMission,MDB_Name);
			std::wstring sName;
			if( szNameId[0] != '\0' )
			{
				sName = LoadString(szNameId);
			}
			else
			{
				sName = g_pMissionDB->GetWString(hMission,MDB_NameStr);
				if (!sName.length())
				{
					sName = MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str();
				}
			}
			wchar_t wszPlayers[64] = L"";
			uint32 nMin = g_pMissionDB->GetInt32(hLevel,MDB_MinPlayers,0,0);
			uint32 nMax = g_pMissionDB->GetInt32(hLevel,MDB_MaxPlayers,0,0);
			if (nMin >= 2 && nMax >= nMin)
			{
				FormatString("MP_Players",wszPlayers,LTARRAYSIZE(wszPlayers),nMin,nMax);
				sName += L" ";
				sName += wszPlayers;
			}

			char szWorldTitle[MAX_PATH] = "";
			LTStrCpy(szWorldTitle,g_pMissionDB->GetWorldName(hLevel,false),LTARRAYSIZE(szWorldTitle));

			CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
			pColumnCtrl->Create(cs);
			pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
			pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
			pColumnCtrl->SetParam1(nMission);

			char const* pszListFont = g_pLayoutDB->GetListFont(m_hLayout,0);
			const uint32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

			pColumnCtrl->SetFont( CFontInfo(pszListFont, nListFontSize) );
			pColumnCtrl->AddTextColumn( sName.c_str(), nAvailWidth, true );

			m_pAvailMissions->AddControl( pColumnCtrl );
		}
	}


    return (true);
}

void CScreenHostLevels::LoadMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	char szMissionName[256];
	uint8 numMissions = 0;
	for( ;; )
	{
		if( !GameModeMgr::Instance( ).GetMissionByIndex( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ), 
			numMissions, szMissionName, LTARRAYSIZE( szMissionName )))
			break;

		// Stop as soon as the mission entries end.
		if( LTStrEmpty( szMissionName ))
			break;

		HRECORD hMissionRec = g_pLTDatabase->GetRecord( g_pMissionDB->GetMissionCat( ), szMissionName );
		int nMissionId = g_pLTDatabase->GetRecordIndex( hMissionRec );
		AddMissionToList(nMissionId,false,true);
		numMissions++;
	}
}

void CScreenHostLevels::MakeDefaultMissionList()
{
	// Sanity checks...

	if (!m_pAvailMissions) return;
	if (!m_pSelMissions) return;

	for (uint32 nMission = 0; nMission < g_pMissionDB->GetNumMissions(); nMission++)
	{
		HRECORD hMission = g_pMissionDB->GetMission(nMission);
		if (hMission)
		{
			AddMissionToList(nMission,false,false);
		}
	}
}

void CScreenHostLevels::SaveMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	char szMission[MAX_PATH*2];
	for (uint32 n = 0; n < m_pSelMissions->GetNumControls(); n++)
	{
		CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelMissions->GetControl(n);
		if( !pCtrl )
			continue;

		LTStrCpy( szMission, g_pLTDatabase->GetRecordName( g_pMissionDB->GetMission( pCtrl->GetParam1( ))), 
			LTARRAYSIZE( szMission ));
		GameModeMgr::Instance( ).SetMissionByIndex( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ),
			n, szMission );
	}

	// Write out an empty entry to indicate the end of the list.
	GameModeMgr::Instance( ).SetNumMissions( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ), 
		m_pSelMissions->GetNumControls( ));
}			
		



void CScreenHostLevels::UpdateButtons()
{
	m_pAddAll->Enable( m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS && m_pAvailMissions->GetNumControls() > 0);
	m_pRemoveAll->Enable(m_pSelMissions->GetNumControls() > 0);

}

void CScreenHostLevels::AddMissionToList(int nMissionId, bool bVerifySize, bool bForce)
{
	// Sanity checks...

	if (!m_pSelMissions) return;
	if (m_pSelMissions->GetNumControls() == MAX_GAME_LEVELS) return;


	// Add the level to the list...

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nSelWidth,nListFontSize);
	cs.nCommandID = CMD_REMOVE_LEVEL;
	cs.pCommandHandler = this;

	HRECORD hMission = g_pMissionDB->GetMission(nMissionId);
	HRECORD	hLevel = g_pMissionDB->GetLevel(hMission,0);
	if (hMission && hLevel)
	{
		if (!g_pMissionDB->CheckMPLevelRequirements(hMission,m_setRequiredMapFeatures))
		{
			return;
		}

		if (!bForce)
		{
			uint32 nMapMin = g_pMissionDB->GetInt32(hLevel,MDB_MinPlayers,0,0);
			uint32 nMapMax = g_pMissionDB->GetInt32(hLevel,MDB_MaxPlayers,0,0);
			uint32 nGameMax = GameModeMgr::Instance( ).m_grnMaxPlayers.GetValue();
			if (nMapMax > 0 && nGameMax > nMapMax)
			{
				if (bVerifySize)
				{

					MBCreate mb;
					mb.eType = LTMB_YESNO;
					mb.pFn = AddCallBack;
					mb.pData = (void *)nMissionId;
					mb.pUserData = this;
					g_pInterfaceMgr->ShowMessageBox("Screen_Host_LevelTooSmall",&mb);

				}
				return;
			}
			if (nMapMin > 0 && nGameMax < nMapMin)
			{
				if (bVerifySize)
				{

					MBCreate mb;
					mb.eType = LTMB_YESNO;
					mb.pFn = AddCallBack;
					mb.pData = (void *)nMissionId;
					mb.pUserData = this;
					g_pInterfaceMgr->ShowMessageBox("Screen_Host_LevelTooLarge",&mb);

				}
				return;
			}
		}

		
		const char* szNameId = g_pMissionDB->GetString(hMission,MDB_Name);
		std::wstring sName;
		if( szNameId[0] != '\0' )
		{
			sName = LoadString(szNameId);
		}
		else
		{
			sName = g_pMissionDB->GetWString(hMission,MDB_NameStr);
			if (!sName.length())
			{
				sName = MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str();
			}
		}
		wchar_t wszPlayers[64] = L"";
		uint32 nMin = g_pMissionDB->GetInt32(hLevel,MDB_MinPlayers,0,0);
		uint32 nMax = g_pMissionDB->GetInt32(hLevel,MDB_MaxPlayers,0,0);
		if (nMin >= 2 && nMax >= nMin)
		{
			FormatString("MP_Players",wszPlayers,LTARRAYSIZE(wszPlayers),nMin,nMax);
			sName += L" ";
			sName += wszPlayers;
		}

		cs.szHelpID = "";

		CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
		pColumnCtrl->Create(cs);
		pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pColumnCtrl->SetParam1( nMissionId );

		char const* pszListFont = g_pLayoutDB->GetListFont(m_hLayout,0);
		const uint32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

		pColumnCtrl->SetFont( CFontInfo(pszListFont, nListFontSize) );
		pColumnCtrl->AddTextColumn( sName.c_str(), nSelWidth, true );

		m_pSelMissions->AddControl( pColumnCtrl );
	
	}
	else
	{
		ASSERT(!"Invalid mission id");
	}

}
