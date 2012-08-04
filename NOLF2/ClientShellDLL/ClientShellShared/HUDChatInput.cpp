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
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "ClientSoundMgr.h"
#include "ScmdConsole.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDChatInput::CHUDChatInput()
{
	m_UpdateFlags = kHUDNone;
	m_bVisible = LTFALSE;
	m_pStr = LTNULL;
	m_eLevel = kHUDRenderText;
	m_bTeamMessage = false;
}


LTBOOL CHUDChatInput::Init()
{
	char *pTag = "ChatInput";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	CUIFont* pFont	= g_pInterfaceResMgr->GetFont(nFont);

	m_nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"Color");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	uint32 color = SET_ARGB(0xFF,nR,nG,nB);

	uint8 h = (uint8)((float)m_nFontSize * g_pInterfaceResMgr->GetYRatio());

	float x = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetYRatio();


	m_pStr = g_pFontManager->CreatePolyString(pFont," ",x, y);
	m_pStr->SetCharScreenHeight(h);
	m_pStr->SetColor(color);

	m_EditCtrl.Create(g_pLTClient,LTNULL,pFont,m_nFontSize,kMaxChatLength,LTNULL,m_szChatStr);
	m_EditCtrl.SetColors(color,color,color);
	m_EditCtrl.EnableCaret(LTTRUE);
	m_EditCtrl.SetInputMode(CLTGUIEditCtrl::kInputSprintfFriendly);

	m_EditCtrl.SetBasePos(m_BasePos);
	m_EditCtrl.SetScale(g_pInterfaceResMgr->GetXRatio());

	UpdateLayout();


	return LTTRUE;
}

void CHUDChatInput::Term()
{
	m_EditCtrl.Destroy();
	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}

}

void CHUDChatInput::Render()
{

	if (!m_bVisible) return;
//	SetRenderState();

	m_pStr->Render();
	m_EditCtrl.Render();


}

void CHUDChatInput::Update()
{

	uint8 h = (uint8)((float)m_nFontSize * g_pInterfaceResMgr->GetYRatio());
	m_pStr->SetCharScreenHeight(h);

	float x = ((float)(m_BasePos.x - 5)  * g_pInterfaceResMgr->GetXRatio()) - m_pStr->GetWidth();
	float y = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetYRatio();
	m_pStr->SetPosition(x,y);

	m_EditCtrl.SetScale(g_pInterfaceResMgr->GetXRatio());
}

void CHUDChatInput::OnExitWorld() 
{
	if (m_bVisible)
	{
		m_bVisible = LTFALSE;
		m_nHistory = -1;
		g_pChatMsgs->ShowHistory(LTFALSE);

		m_EditCtrl.Show(LTFALSE);
		m_EditCtrl.UpdateData(LTTRUE);

		g_pGameClientShell->SetInputState(LTTRUE);
		g_pLTClient->ClearInput();
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

	m_EditCtrl.Show(bShow);
	g_pGameClientShell->SetInputState(!bShow);
	g_pLTClient->ClearInput();

	if (bShow)
	{
		m_pStr->SetText(LoadTempString(IDS_SAY));
		m_szChatStr[0] = 0;
	
		Update();

	}
	
	// Pause the server in single player while chatting... 

	if( !IsMultiplayerGame() )
	{
		g_pGameClientShell->PauseGame( !!bShow, true );
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

}

LTBOOL CHUDChatInput::HandleKeyDown(int key, int rep)
{
	switch (key)
	{
	case VK_ESCAPE:
	{
		Show(false,false);
		return LTTRUE;
	} break;

	case VK_RETURN:
	{
		Send();
		return LTTRUE;
	} break;

	case VK_UP:
	{
		int nTest = m_nHistory+1;
		if (nTest >= 0 && nTest < kMaxChatHistory && strlen(m_szChatHistory[nTest]))
		{
			m_nHistory = nTest;
			SAFE_STRCPY(m_szChatStr,m_szChatHistory[nTest]);
			m_EditCtrl.UpdateData(LTFALSE);
		}
		return LTTRUE;
	} break;

	case VK_DOWN:
	{
		int nTest = m_nHistory-1;
		if (nTest >= 0 && nTest < kMaxChatHistory && strlen(m_szChatHistory[nTest]))
		{
			m_nHistory = nTest;
			SAFE_STRCPY(m_szChatStr,m_szChatHistory[nTest]);
			m_EditCtrl.UpdateData(LTFALSE);
		}
		return LTTRUE;
	} break;

	case VK_PRIOR:
	{
		g_pChatMsgs->IncHistoryOffset();
		return LTTRUE;
	} break;

	case VK_NEXT:
	{
		g_pChatMsgs->DecHistoryOffset();
		return LTTRUE;
	} break;

	default:
		return m_EditCtrl.HandleKeyDown(key,rep);
	}
}

LTBOOL CHUDChatInput::HandleChar(unsigned char c)
{
	return m_EditCtrl.HandleChar(c);
}

void CHUDChatInput::Send()
{

	Show(false,false);

	// Ignore empty messages.
	if( !m_szChatStr[0] )
		return;

	// First check and see if it was a cheat that was entered...

	ConParse cParse( m_szChatStr );
	if( LT_OK == g_pCommonLT->Parse( &cParse ))
	{
		CParsedMsg parsedMsg( cParse.m_nArgs, cParse.m_Args );

		if (g_pCheatMgr->Check( parsedMsg ))
		{
			g_pClientSoundMgr->PlayInterfaceSound("Interface\\Menu\\Snd\\Cheat.wav");
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
	if (g_pPlayerMgr->GetPlayerState() == PS_GHOST)
		nMsgID = MID_PLAYER_GHOSTMESSAGE;
	else
        nMsgID = MID_PLAYER_MESSAGE;
	cMsg.Writeuint8(nMsgID);
    cMsg.WriteString(m_szChatStr);
	cMsg.Writebool(m_bTeamMessage);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	// cache this string in the chat history
	for (int i = kMaxChatHistory-1; i > 0; i--)
	{
		SAFE_STRCPY(m_szChatHistory[i], m_szChatHistory[i-1]);
	}

	SAFE_STRCPY(m_szChatHistory[0], m_szChatStr);
}
