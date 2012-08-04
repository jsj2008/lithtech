// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMessageQueue.cpp
//
// PURPOSE : Implementation of CHUDMessageQueue to display messages
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDMessageQueue.h"
#include "InterfaceMgr.h"

VarTrack g_vtHUDMessageQueueRender;

const uint16 CHUDMessageQueue::kMaxHistory = 255;

void SimulateChatFn(int /*argc*/, char ** /*argv*/)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	const char* szStrings[] = 
	{
		"14062",
		"14089",
		"14278",
		"14302",
		"14320",
		"14346",
		"14490",
		"14492",
	};

	
	int num = GetRandom(0,7);
	wchar_t szMsg[256];
	szMsg[0] = 0;
	for (int n = 0; n < 20; n++)
	{
		int x = (n+num)%8;
		LTSNPrintF(szMsg,LTARRAYSIZE(szMsg),L"%d: %s",n,LoadString(szStrings[x]));

		g_pChatMsgs->AddMessage(szMsg,(eChatMsgType)GetRandom(0,2));	
	}
}


CHUDMessageQueue::CHUDMessageQueue()
{
	m_UpdateFlags = kHUDFrame;
	m_nMaxActiveMsgs = 5;
	m_nMaxHistoryMsgs = 1;
	m_bTopJustify = true;
	m_eLevel = kHUDRenderText;
	m_bShowHistory = false;
	m_nHistoryOffset = 0;
	m_bShowNextArrow = false;
	m_bShowPrevArrow = false;
	m_bDraw	= true;
	m_eHUDRenderLayer = eHUDRenderLayer_Front;
}
	

bool CHUDMessageQueue::Init()
{
	UpdateLayout();

	g_vtHUDMessageQueueRender.Init( g_pLTClient, "HUDMessageQueue", NULL, 1.0f );

	g_pLTClient->RegisterConsoleProgram("SimulateChat", SimulateChatFn);

	ASSERT(m_nMaxActiveMsgs);
	if (!m_nMaxActiveMsgs) return false;

	m_ActiveMsgs.reserve(m_nMaxActiveMsgs);
	m_HistoryMsgs.reserve(m_nMaxHistoryMsgs);

	m_History.reserve(32);


	MsgCreate fmt = m_MsgFormat;
	fmt.sString = L"";
	fmt.fDuration = 0.0f;

	for (int i = 0; i < m_nMaxActiveMsgs; i++)
	{
		CHUDMessage *pMsg = debug_new(CHUDMessage);
		if (pMsg->Create(fmt))
		{
			pMsg->Show(false);
			m_ActiveMsgs.push_back(pMsg);
		}
		else
		{
			debug_delete(pMsg);
		}
	}

	for (i = 0; i < m_nMaxHistoryMsgs; i++)
	{
		CHUDMessage *pMsg = debug_new(CHUDMessage);
		if (pMsg->Create(fmt))
		{
			pMsg->Show(false);
			m_HistoryMsgs.push_back(pMsg);
		}
		else
		{
			debug_delete(pMsg);
		}
	}

	return true;

}
void CHUDMessageQueue::Term()
{
	MessageArray::iterator iter = m_ActiveMsgs.begin();
	while (iter !=  m_ActiveMsgs.end())
	{
		CHUDMessage *pMsg = (*iter);
		pMsg->Destroy();
		debug_delete(pMsg);
		iter++;
	}

	iter = m_HistoryMsgs.begin();
	while (iter !=  m_HistoryMsgs.end())
	{
		CHUDMessage *pMsg = (*iter);
		pMsg->Destroy();
		debug_delete(pMsg);
		iter++;
	}

	MCArray::iterator mcIter = m_History.begin();
	while (mcIter != m_History.end())
	{
		debug_delete((*mcIter));
		mcIter++;
	}


}

void CHUDMessageQueue::Render()
{
	if( g_vtHUDMessageQueueRender.GetFloat( ) < 1.0f )
		return;

	if( !m_bDraw )
		return;

	LTVector2n pos = m_vBasePos;
	LTVector2n lastpos = m_vBasePos;

	g_pInterfaceResMgr->ScaleScreenPos(pos);

	if (m_bShowHistory && m_bShowPrevArrow)
	{
		LT_POLYGT4 Quad;

		float fSize = (float)m_sTextFont.m_nHeight * g_pInterfaceResMgr->GetXRatio();

		//setup the quad to be in the correct position and render the icon once
		DrawPrimSetXYWH(Quad, (float)pos.x-fSize, (float)pos.y-fSize, fSize, fSize);
		DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
		SetupQuadUVs(Quad, m_Up, 0.0f, 0.0f, 1.0f, 1.0f);

		//setup the draw prim for rendering
		if (m_bTopJustify)
		{
			g_pLTClient->GetDrawPrim()->SetTexture(m_Up);
		}
		else
		{
			g_pLTClient->GetDrawPrim()->SetTexture(m_Down);
		}


		//and render away
		g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
	}


	if (m_bTopJustify)
	{
		MessageArray::iterator iter;
		if (m_bShowHistory)
		{

			iter = m_HistoryMsgs.begin();
			while (iter !=  m_HistoryMsgs.end())
			{
				CHUDMessage *pMsg = *iter;

				if (pMsg->IsVisible())
				{
					pMsg->SetBasePos(pos);
					pMsg->Render(true);
					lastpos = pos;		
					pos.y += pMsg->GetBaseHeight();
				}

				iter++;
			}
		}
		else
		{

			iter = m_ActiveMsgs.begin();
			while (iter !=  m_ActiveMsgs.end())
			{
				CHUDMessage *pMsg = *iter;
				if (pMsg->IsVisible())
				{
					pMsg->SetBasePos(pos);
					pMsg->Render();
					lastpos = pos;
					pos.y += pMsg->GetBaseHeight();
				}

				iter++;
			}
		}


	}
	else
	{
		MessageArray::reverse_iterator iter;
		if (m_bShowHistory)
		{
			iter = m_HistoryMsgs.rbegin();
			while (iter !=  m_HistoryMsgs.rend())
			{
				CHUDMessage *pMsg = *iter;
				if (pMsg->IsVisible())
				{
					lastpos = pos;
					pos.y -= pMsg->GetBaseHeight();
					pMsg->SetBasePos(pos);
					pMsg->Render(true);
				}

				iter++;
			}
		}
		else
		{
			iter = m_ActiveMsgs.rbegin();
			while (iter !=  m_ActiveMsgs.rend())
			{
				CHUDMessage *pMsg = *iter;
				if (pMsg->IsVisible())
				{
					lastpos = pos;
					pos.y -= pMsg->GetBaseHeight();

					pMsg->SetBasePos(pos);
					pMsg->Render();
				}

				iter++;
			}
		}
	}

	if (m_bShowHistory && m_bShowNextArrow)
	{
		LT_POLYGT4 Quad;

		float fSize = (float)m_sTextFont.m_nHeight * g_pInterfaceResMgr->GetXRatio();

		//setup the quad to be in the correct position and render the icon once
		DrawPrimSetXYWH(Quad, (float)lastpos.x-fSize, (float)lastpos.y-fSize, fSize, fSize);
		DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
		SetupQuadUVs(Quad, m_Down, 0.0f, 0.0f, 1.0f, 1.0f);

		//setup the draw prim for rendering
		if (m_bTopJustify)
		{
			g_pLTClient->GetDrawPrim()->SetTexture(m_Down);
		}
		else
		{
			g_pLTClient->GetDrawPrim()->SetTexture(m_Up);
		}


		//and render away
		g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
	}
	


}

void CHUDMessageQueue::Update()
{
	MessageArray::iterator iter = m_ActiveMsgs.begin();
	while (iter !=  m_ActiveMsgs.end())
	{
		CHUDMessage *pMsg = *iter;
		pMsg->Update();
		iter++;
	}

}
void CHUDMessageQueue::ScaleChanged()
{
}
void CHUDMessageQueue::AddMessage(MsgCreate &fmt, bool bHistoryOnly)
{

	if (!bHistoryOnly)
	{
		CHUDMessage *pMsg = 0;

		if (!m_ActiveMsgs.empty())
		{
			//take the oldest active message out of the list
			MessageArray::iterator iter = m_ActiveMsgs.begin();
			pMsg = (*iter);
			m_ActiveMsgs.erase(iter);
		}
		else
		{
			pMsg = debug_new(CHUDMessage);
		}

		//overwrite the data
		uint32 nOldHeight = fmt.Font.m_nHeight;
		uint32 nOldWidth = fmt.nWidth;
		if (GetConsoleBool("UseTextScaling",false))
		{
			fmt.Font.m_nHeight = (uint32)(g_pInterfaceResMgr->GetYRatio() * (float)nOldHeight);
		}

		fmt.nWidth = (uint32)(g_pInterfaceResMgr->GetXRatio() * (float)nOldWidth);
		pMsg->Create(fmt);
		fmt.Font.m_nHeight = nOldHeight;
		fmt.nWidth = nOldWidth;
		pMsg->Show(true);

		//and add it back to the list
		m_ActiveMsgs.push_back(pMsg);
	}

	MCArray::iterator mcIter;
	while (m_History.size() >= kMaxHistory)
	{
		mcIter = m_History.begin();
		debug_delete(*mcIter);
		m_History.erase(mcIter);
	}

	MsgCreate *pMC = debug_new(MsgCreate);
	*pMC = fmt;
	m_History.push_back(pMC);

	if (m_bShowHistory)
	{
		SetHistoryOffset(m_nHistoryOffset);
	}

}


void CHUDMessageQueue::ShowHistory(bool bShow) 
{
	m_bShowHistory = bShow;
	if (bShow)
	{
		SetHistoryOffset(0);
	}
}


void CHUDMessageQueue::ClearHistory() 
{
	MCArray::iterator mcIter = m_History.begin();
	while (mcIter !=  m_History.end())
	{
		debug_delete((*mcIter));
		mcIter++;
	}
	m_History.clear();

	MessageArray::iterator iter = m_ActiveMsgs.begin();
	while (iter !=  m_ActiveMsgs.end())
	{
		(*iter)->Show(false);
		iter++;
	}

}

void CHUDMessageQueue::SetHistoryOffset(uint16 nOffset) 
{
	uint32 nMaxOffset;
	if (m_History.size() <= m_HistoryMsgs.size())
		nMaxOffset = 0;
	else
		nMaxOffset = (m_History.size() - m_HistoryMsgs.size());
	
	uint32 nCurOffset = nOffset;
	if (nCurOffset > nMaxOffset)
		nCurOffset = nMaxOffset;

	ASSERT( nCurOffset == ( uint16 )nCurOffset );
	m_nHistoryOffset = ( uint16 )nCurOffset;

	m_bShowNextArrow = nCurOffset < nMaxOffset;
	m_bShowPrevArrow = nCurOffset > 0;

	uint32 nMsg = m_HistoryMsgs.size() - 1;
	uint32 nIndex = (m_History.size() - m_nHistoryOffset) - 1;

	while (nMsg < m_HistoryMsgs.size())
	{
		CHUDMessage* pMsg = m_HistoryMsgs[nMsg];
		if (  nIndex < m_History.size() )
		{
			pMsg->Create(*(m_History[nIndex]));
			pMsg->Show(true);
			nIndex--;
		}
		else
			pMsg->Show(false);

		nMsg--;
		

	}


}


//display an earlier page of history
void CHUDMessageQueue::IncHistoryOffset() 
{
	uint32 nMaxOffset;
	if (m_History.size() <= m_HistoryMsgs.size())
		nMaxOffset = 0;
	else
		nMaxOffset = (m_History.size() - m_HistoryMsgs.size());

	uint32 nOffset = m_nHistoryOffset + m_HistoryMsgs.size();

	if (nOffset > nMaxOffset)
		nOffset = nMaxOffset;

	ASSERT( nOffset == ( uint16 )nOffset );
	SetHistoryOffset(( uint16 )nOffset);
}

//display an later page of history
void CHUDMessageQueue::DecHistoryOffset() 
{
	uint32 nOffset = m_nHistoryOffset;

	if (nOffset > m_HistoryMsgs.size())
		nOffset -= m_HistoryMsgs.size();
	else
		nOffset = 0;

	ASSERT( nOffset == ( uint16 )nOffset );
	SetHistoryOffset(( uint16 )nOffset);

}

void CHUDChatMsgQueue::UpdateLayout()
{
	m_bTopJustify = false;

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDChatMessage");
	}

	CHUDItem::UpdateLayout();

	m_MsgFormat.Font = m_sTextFont;

	m_MsgFormat.nTextColor = argbWhite;
	m_MsgFormat.fDuration = m_fHoldTime;
	m_MsgFormat.fFadeDur  = m_fFadeTime;
	m_MsgFormat.eJustify = kLeft;
	m_MsgFormat.nWidth  = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);

	m_nMaxActiveMsgs = (uint8)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);
	m_nMaxHistoryMsgs = (uint8)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,2);

	m_nMsgColors[kMsgDefault] = m_cTextColor;
	m_nMsgColors[kMsgChat] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,0);
	m_nMsgColors[kMsgTeam] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,1);
	m_nMsgColors[kMsgTransmission] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,2);
	m_nMsgColors[kMsgCheatConfirm] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,3);
	m_nMsgColors[kMsgScmd] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,4);
	m_nMsgColors[kMsgOtherTeam] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,5);

	m_Up.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_Down.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,1));

}

void CHUDChatMsgQueue::AddMessage(const wchar_t *pszString, eChatMsgType type)
{

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	fmt.nTextColor = m_nMsgColors[type];
	bool bHistoryOnly = (type == kMsgTransmission);
	CHUDMessageQueue::AddMessage(fmt,bHistoryOnly);

}

void CHUDChatMsgQueue::AddMessage(const char* szMessageID, eChatMsgType type)
{
	AddMessage(LoadString(szMessageID),type);
}

void CHUDChatMsgQueue::AddMessage(const wchar_t *pszHeader, const wchar_t *pszString, eChatMsgType headertype /* = kMsgDefault */, eChatMsgType type /* = kMsgDefault */)
{

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	fmt.nTextColor = m_nMsgColors[type];
	fmt.sHeaderString = pszHeader;
	fmt.nHeaderColor = m_nMsgColors[headertype];

	bool bHistoryOnly = (type == kMsgTransmission);
	CHUDMessageQueue::AddMessage(fmt,bHistoryOnly);

}

void CHUDChatMsgQueue::AddMessage(const wchar_t *pszHeader, const char* szMessageID, eChatMsgType headertype /* = kMsgDefault */, eChatMsgType type /* = kMsgDefault */)
{
	AddMessage(pszHeader,LoadString(szMessageID),headertype,type);
}




void CHUDGameMsgQueue::UpdateLayout()
{
	m_bTopJustify = true;

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDGameMessage");
	}

	CHUDItem::UpdateLayout();

	m_MsgFormat.Font = m_sTextFont;
	m_MsgFormat.nTextColor = argbWhite;
	m_MsgFormat.fDuration = m_fHoldTime;
	m_MsgFormat.fFadeDur  = m_fFadeTime;
	m_MsgFormat.eJustify = kRight;
	m_MsgFormat.nWidth  = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);


	m_nMaxActiveMsgs = (uint8)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);
	m_nMaxHistoryMsgs = (uint8)g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,2);


	m_nMsgColors[kMsgDefault] = m_cTextColor;
	m_nMsgColors[kMsgChat] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,0);
	m_nMsgColors[kMsgTeam] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,1);
	m_nMsgColors[kMsgTransmission] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,2);
	m_nMsgColors[kMsgCheatConfirm] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,3);
	m_nMsgColors[kMsgScmd] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,4);
	m_nMsgColors[kMsgOtherTeam] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddColor,5);


}

void CHUDGameMsgQueue::AddMessage(const wchar_t *pszString, eChatMsgType type)
{

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	fmt.nTextColor = m_nMsgColors[type];
	CHUDMessageQueue::AddMessage(fmt,false);

}

void CHUDGameMsgQueue::AddMessage(const char* szMessageID, eChatMsgType type)
{
	AddMessage(LoadString(szMessageID),type);
}
