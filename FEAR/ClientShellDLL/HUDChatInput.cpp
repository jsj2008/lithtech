// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChatInput.cpp
//
// PURPOSE : HUDItem to display chat input
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDChatInput.h"
#include "HUDMessageQueue.h"
#include "sys/win/mpstrconv.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "ClientSoundMgr.h"
#include "ScmdConsole.h"
#include "BindMgr.h"

//******************************************************************************************
//**
//** HUD Chat display
//**
//******************************************************************************************

CHUDChatInput::CHUDChatInput()
{
	m_UpdateFlags = kHUDNone;
	m_bVisible = false;
	m_eLevel = kHUDRenderText;
	m_bTeamMessage = false;
	m_eHUDRenderLayer = eHUDRenderLayer_Front;
}


bool CHUDChatInput::Init()
{
	for (int i = 0; i < kMaxChatHistory; i++)
	{
		m_szChatHistory[i][0] = L'';
	}

	UpdateLayout();
	ScaleChanged();

	return true;
}

void CHUDChatInput::Term()
{
	m_EditCtrl.Destroy();

}

void CHUDChatInput::Render()
{

	if (!m_bVisible) return;
//	SetRenderState();

	m_Text.Render();
	m_EditCtrl.Render();


}

void CHUDChatInput::Update()
{
}

void CHUDChatInput::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	LTVector2n nPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(nPos);
	m_EditCtrl.SetBasePos(nPos);
	
	LTVector2n sz(m_nInputWidth,m_sTextFont.m_nHeight);
	sz.x = int32(g_pInterfaceResMgr->GetXRatio() * float(sz.x));
	m_EditCtrl.SetSize(sz);
}

void CHUDChatInput::OnExitWorld() 
{
	if (m_bVisible)
	{
		m_bVisible = false;
		m_nHistory = -1;
		g_pChatMsgs->ShowHistory(false);

		m_EditCtrl.Show(false);
		m_EditCtrl.UpdateData(true);

		g_pGameClientShell->SetInputState(true);
		CBindMgr::GetSingleton().ClearAllCommands();
	}
}

void CHUDChatInput::Show(bool bShow, bool bTeam) 
{
	m_bVisible = bShow;
	m_nHistory = -1;
	g_pChatMsgs->ShowHistory(bShow);


	//only set the team flag when showing the ChatInput. We will need to
	// access the flag after it is hidden.
	if (bShow)
		m_bTeamMessage = bTeam;

	// Pause the server in single player while chatting... 
	if( !IsMultiplayerGameClient() )
	{
		g_pGameClientShell->PauseGame( !!bShow, true );
	}

	m_EditCtrl.Show(bShow);
	g_pGameClientShell->SetInputState(!bShow);
	CBindMgr::GetSingleton().ClearAllCommands();

	if (bShow)
	{
		if (bTeam)
			m_Text.SetText(LoadString("IDS_TEAM_SAY"));
		else
			m_Text.SetText(LoadString("IDS_SAY"));
		m_szChatStr[0] = 0;
	
		Update();

	}
	
	// Send the Message to the server
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_CHATMODE);
    cMsg.Writeuint8((uint8)bShow);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	m_EditCtrl.UpdateData(!bShow);

}

void CHUDChatInput::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDChatInput");
	}

	CHUDItem::UpdateLayout();

	m_nInputWidth = (uint32)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);


	CLTGUIEditCtrl_create ecs;
	ecs.bUseCaret = true;
	ecs.nMaxLength = kMaxChatLength;
	ecs.rnBaseRect.Right() = m_nInputWidth;
	ecs.rnBaseRect.Bottom() = m_sTextFont.m_nHeight;
	ecs.argbCaretColor = m_cTextColor;
	ecs.pszValue = m_szChatStr;

	m_EditCtrl.Create(g_pLTClient,m_sTextFont,ecs);
	m_EditCtrl.SetColor(m_cTextColor);

}

bool CHUDChatInput::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	switch (key)
	{
	case VK_ESCAPE:
	{
		Show(false,false);
		return true;
	} break;

	case VK_RETURN:
	{
		Send();
		return true;
	} break;

	case VK_UP:
	{
		int nTest = m_nHistory+1;
		if (nTest >= 0 && nTest < kMaxChatHistory && !LTStrEmpty(m_szChatHistory[nTest]))
		{
			m_nHistory = nTest;
			LTStrCpy(m_szChatStr,m_szChatHistory[nTest], LTARRAYSIZE(m_szChatStr));
			m_EditCtrl.UpdateData(false);
		}
		return true;
	} break;

	case VK_DOWN:
	{
		int nTest = m_nHistory-1;
		if (nTest >= 0 && nTest < kMaxChatHistory && !LTStrEmpty(m_szChatHistory[nTest]))
		{
			m_nHistory = nTest;
			LTStrCpy(m_szChatStr,m_szChatHistory[nTest], LTARRAYSIZE(m_szChatStr));
			m_EditCtrl.UpdateData(false);
		}
		return true;
	} break;

	case VK_PRIOR:
	{
		g_pChatMsgs->IncHistoryOffset();
		return true;
	} break;

	case VK_NEXT:
	{
		g_pChatMsgs->DecHistoryOffset();
		return true;
	} break;

	default:
		return m_EditCtrl.HandleKeyDown(key,rep);
	}

#else // PLATFORM_XENON
	return false;
#endif // PLATFORM_XENON

}

bool CHUDChatInput::HandleChar(wchar_t c)
{
	return m_EditCtrl.HandleChar(c);
}

void CHUDChatInput::Send()
{

	Show(false,false);

	if ( IsMultiplayerGameClient() ) {
		// Check to see if this is a PunkBuster command
		if ( !strncmp ( MPW2A(m_szChatStr).c_str(), "pb_", 3 ) ||
			 !strncmp ( MPW2A(m_szChatStr).c_str(), "\\pb_", 4 ) ||
			 !strncmp ( MPW2A(m_szChatStr).c_str(), "/pb_", 4 ) ) 
		{
			// cache this string in the chat history
			for (int i = kMaxChatHistory-1; i > 0; i--)
			{
				LTStrCpy(m_szChatHistory[i], m_szChatHistory[i-1], LTARRAYSIZE(m_szChatHistory[i]));
			}

			LTStrCpy(m_szChatHistory[0], m_szChatStr, LTARRAYSIZE(m_szChatHistory[0]));

			// Strip off the back/forward slash if present before we send to PB SDK
			if ( !strncmp ( MPW2A(m_szChatStr).c_str(), "\\pb_", 4 ) ||
				 !strncmp ( MPW2A(m_szChatStr).c_str(), "/pb_", 4 ) )
				LTStrCpy ( m_szChatStr, m_szChatStr+1, LTStrLen ( m_szChatStr ) ) ;
			g_pGameClientShell->PunkBusterProcessCommand ( (char *) MPW2A(m_szChatStr).c_str() ) ;

			return ;
		}
	}

	// Ignore empty messages.
	if( !m_szChatStr[0] )
		return;

	// First check and see if it was a cheat that was entered...

	ConParseW cParse( m_szChatStr );
	if( LT_OK == g_pCommonLT->Parse( &cParse ))
	{
		CParsedMsgW parsedMsg( cParse.m_nArgs, cParse.m_Args );

		if (g_pCheatMgr->Check( parsedMsg ))
		{
//			g_pClientSoundMgr->PlayInterfaceDBSound("Cheat");
			return;
		}

		// Check if this is an scmd command.
		else if( ScmdConsole::Instance( ).SendParsedCommand( parsedMsg ))
		{
			// If it was, it was sent.  Don't send chat.
			return;
		}

	}

	// Send the Message to the server
	CAutoMessage cMsg;
	uint8 nMsgID;
	if (g_pPlayerMgr->IsSpectating())
		nMsgID = MID_PLAYER_GHOSTMESSAGE;
	else
        nMsgID = MID_PLAYER_MESSAGE;
	cMsg.Writeuint8(nMsgID);
    cMsg.WriteWString(m_szChatStr);
	cMsg.Writebool(m_bTeamMessage);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	// cache this string in the chat history
	for (int i = kMaxChatHistory-1; i > 0; i--)
	{
		LTStrCpy(m_szChatHistory[i], m_szChatHistory[i-1], LTARRAYSIZE(m_szChatHistory[i]));
	}

	LTStrCpy(m_szChatHistory[0], m_szChatStr, LTARRAYSIZE(m_szChatHistory[0]));
}
