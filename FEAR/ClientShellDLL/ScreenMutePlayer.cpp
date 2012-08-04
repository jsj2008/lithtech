// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMutePlayer.cpp
//
// PURPOSE : Interface screen to mute specific players
//
// CREATED : 06/21/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMutePlayer.h"
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
#include "ClientInfoMgr.h"

namespace
{
	uint32 nListFontSize = 12;
	int32 nUnmutedWidth = 0;
	int32 nMutedWidth = 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMutePlayer::CScreenMutePlayer() :
	m_pUnmutedScrollBar(NULL),
	m_pMutedScrollBar(NULL)
{
    m_pUnmuted	= NULL;
    m_pMuted    = NULL;
	m_pAdd			= NULL;
	m_pRemove		= NULL;
	m_pAddAll		= NULL;
	m_pRemoveAll    = NULL;

}

CScreenMutePlayer::~CScreenMutePlayer()
{
	Term();
}

// Build the screen
bool CScreenMutePlayer::Build()
{
	LTRect2n rcUnmutedRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	LTRect2n rcMutedRect = g_pLayoutDB->GetListRect(m_hLayout,1);
	uint32 nOffset = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize) + 4;

	LTVector2n addPos = rcUnmutedRect.GetTopLeft();
	addPos.y -= nOffset;
	LTVector2n removePos = rcMutedRect.GetTopLeft();
	removePos.y -= nOffset;
	
	LTVector2n commandPos = rcUnmutedRect.GetTopRight();
	commandPos.x += nOffset;
	commandPos.x += g_pLayoutDB->GetScrollBarSize();
	int32 nCommandWidth = rcMutedRect.Left() - commandPos.x;

	nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

	CreateTitle("Menu_Mute");
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
	cs.szHelpID = "";
	m_pAdd = AddTextItem("ScreenMute_UnmutedPlayers", cs, true);
	m_pAdd->SetBasePos(addPos);

	m_DefaultPos = commandPos;
	cs.nCommandID = CMD_ADD_ALL;
	cs.szHelpID = "ScreenMute_MuteAll_Help";
	m_pAddAll = AddTextItem("ScreenMute_MuteAll", cs);

	cs.nCommandID = CMD_REMOVE_ALL;
	cs.szHelpID = "ScreenMute_UnmuteAll_Help";
	m_pRemoveAll = AddTextItem("ScreenMute_UnmuteAll", cs);

	cs.nCommandID = CMD_REMOVE_LEVEL;
	cs.szHelpID = "";
	m_pRemove = AddTextItem("ScreenMute_MutedPlayers", cs, true);
	m_pRemove->SetBasePos(removePos);

	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();

		m_pUnmutedScrollBar = CreateScrollBar( csb );
		if( m_pUnmutedScrollBar )
		{
			m_pUnmutedScrollBar->SetFrameWidth( 1 );
			m_pUnmutedScrollBar->Enable( true );
			m_pUnmutedScrollBar->Show( true );
		}
	}
	
	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,1);
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();

		m_pMutedScrollBar = CreateScrollBar( csb );
		if( m_pMutedScrollBar )
		{
			m_pMutedScrollBar->SetFrameWidth( 1 );
			m_pMutedScrollBar->Enable( true );
			m_pMutedScrollBar->Show( true );
		}
	}

	//	int32 nListHeight = rcUnmutedRect.GetHeight();
	nUnmutedWidth = rcUnmutedRect.GetWidth();

	CLTGUIListCtrlEx_create listCs;
	listCs.rnBaseRect = rcUnmutedRect;
	listCs.nTextIdent = g_pLayoutDB->GetListIndent(m_hLayout,0).x;
	listCs.pScrollBar = m_pUnmutedScrollBar;

	m_pUnmuted = AddListEx(listCs);
	m_pUnmuted->SetFrameWidth( 1 );
	m_pUnmuted->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hMutedFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pUnmuted->SetFrame(hFrame,hMutedFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));

	if( m_pUnmutedScrollBar )
		AddControl( m_pUnmutedScrollBar );

	nMutedWidth = rcMutedRect.GetWidth();
	listCs.rnBaseRect = rcMutedRect;
	listCs.pScrollBar = m_pMutedScrollBar;

	m_pMuted = AddListEx(listCs);
	m_pMuted->SetFrameWidth( 1 );
	m_pMuted->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,1));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,0));
	hMutedFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,1));
	m_pMuted->SetFrame(hFrame,hMutedFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,1));

	if( m_pMutedScrollBar )
		AddControl( m_pMutedScrollBar );

 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	UseBack(true,true);

	return true;

}



uint32 CScreenMutePlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_ADD_LEVEL:
		{
			if (m_pUnmuted->GetSelectedIndex() >= 0 )
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pUnmuted->GetSelectedControl();
				if (pCtrl)
				{

					g_pInterfaceMgr->MutePlayer(pCtrl->GetParam1());
					UpdateLists();
				}
			}
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pUnmuted->GetNumControls())
			{
				for (uint32 i = 0; i < m_pUnmuted->GetNumControls(); i++)
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pUnmuted->GetControl(i);
					if (pCtrl)
					{
						g_pInterfaceMgr->MutePlayer(pCtrl->GetParam1());
					}
				}
			}
			UpdateLists();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			if (m_pMuted->GetSelectedIndex() >= 0 )
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pMuted->GetSelectedControl();
				if (pCtrl)
				{

					g_pInterfaceMgr->UnmutePlayer(pCtrl->GetParam1());
					UpdateLists();
				}
			}
		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pMuted->GetNumControls())
			{
				for (uint32 i = 0; i < m_pMuted->GetNumControls(); i++)
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pMuted->GetControl(i);
					if (pCtrl)
					{
						g_pInterfaceMgr->UnmutePlayer(pCtrl->GetParam1());
					}
				}
			}
			UpdateLists();
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenMutePlayer::OnFocus(bool bFocus)
{
	if (bFocus)
	{

		UpdateLists();

		UpdateData(false);

	}
	else
	{
		UpdateData();

		m_pUnmuted->RemoveAll();
		m_pMuted->RemoveAll();
	}
	CBaseScreen::OnFocus(bFocus);

}

void CScreenMutePlayer::UpdateLists()
{
	// Sanity checks...
	if (!m_pUnmuted) return;
	if (!m_pMuted) return;

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	m_pUnmuted->RemoveAll();
	m_pMuted->RemoveAll();


	CLTGUICtrl_create ucs;
	ucs.rnBaseRect.m_vMin.Init();
	ucs.rnBaseRect.m_vMax = LTVector2n(nUnmutedWidth,nListFontSize);
	ucs.nCommandID = CMD_ADD_LEVEL;
	ucs.pCommandHandler = this;
	char const* pszListFont = g_pLayoutDB->GetListFont(m_hLayout,0);
	const uint32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

	CLTGUICtrl_create mcs;
	mcs.rnBaseRect.m_vMin.Init();
	mcs.rnBaseRect.m_vMax = LTVector2n(nUnmutedWidth,nListFontSize);
	mcs.nCommandID = CMD_REMOVE_LEVEL;
	mcs.pCommandHandler = this;


	CLIENT_INFO* pClient = pCIMgr->GetFirstClient();

	while (pClient)
	{
		uint32 nLocalID = 0;
		g_pLTClient->GetLocalClientID (&nLocalID);
		if (pClient->nID != nLocalID)
		{
			bool bMuted = g_pInterfaceMgr->IsPlayerMuted(pClient->nID);
			if (bMuted)
			{
				CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
				pColumnCtrl->Create(mcs);
				pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
				pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
				pColumnCtrl->SetParam1(pClient->nID);
				pColumnCtrl->SetFont( CFontInfo(pszListFont, nListFontSize) );
				pColumnCtrl->AddTextColumn( pClient->sName.c_str(), nUnmutedWidth, true );

				m_pMuted->AddControl( pColumnCtrl );
			}
			else
			{
				CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
				pColumnCtrl->Create(ucs);
				pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
				pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
				pColumnCtrl->SetParam1(pClient->nID);
				pColumnCtrl->SetFont( CFontInfo(pszListFont, nListFontSize) );
				pColumnCtrl->AddTextColumn( pClient->sName.c_str(), nUnmutedWidth, true);

				m_pUnmuted->AddControl( pColumnCtrl );
			}

		}

		pClient = pClient->pNext;
	}

	m_pAddAll->Enable( m_pUnmuted->GetNumControls() > 0);
	m_pRemoveAll->Enable(m_pMuted->GetNumControls() > 0);

    return;
}

