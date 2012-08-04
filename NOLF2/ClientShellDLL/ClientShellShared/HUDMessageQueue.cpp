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


const uint16 CHUDMessageQueue::kMaxHistory = 255;

void SimulateChatFn(int argc, char **argv)
{
//	int num = GetRandom(3,7);
	char szTmp[128];
	for (int n = 0; n < 25; n++)
	{
		sprintf(szTmp,"%d",n);
		g_pChatMsgs->AddMessage(szTmp,(eChatMsgType)GetRandom(0,2));	
	}
}


CHUDMessageQueue::CHUDMessageQueue()
{
	m_UpdateFlags = kHUDFrame;
	m_nMaxActiveMsgs = 5;
	m_nMaxHistoryMsgs = 1;
	m_bTopJustify = LTTRUE;
	m_eLevel = kHUDRenderText;
	m_bShowHistory = LTFALSE;
	m_nHistoryOffset = 0;
	m_bDraw	= true;
}
	

LTBOOL CHUDMessageQueue::Init()
{
	UpdateLayout();

	g_pLTClient->RegisterConsoleProgram("SimulateChat", SimulateChatFn);

	ASSERT(m_nMaxActiveMsgs);
	if (!m_nMaxActiveMsgs) return LTFALSE;

	m_ActiveMsgs.reserve(m_nMaxActiveMsgs);
	m_HistoryMsgs.reserve(m_nMaxHistoryMsgs);

	m_History.reserve(32);


	MsgCreate fmt = m_MsgFormat;
	fmt.sString = "";
	fmt.fDuration = 0.0f;

	for (int i = 0; i < m_nMaxActiveMsgs; i++)
	{
		CHUDMessage *pMsg = debug_new(CHUDMessage);
		if (pMsg->Create(fmt))
		{
			pMsg->Show(LTFALSE);
			m_ActiveMsgs.push_back(pMsg);
		}
		else
		{
			debug_delete(pMsg);
		}
	}

	for (int i = 0; i < m_nMaxHistoryMsgs; i++)
	{
		CHUDMessage *pMsg = debug_new(CHUDMessage);
		if (pMsg->Create(fmt))
		{
			pMsg->Show(LTFALSE);
			m_HistoryMsgs.push_back(pMsg);
		}
		else
		{
			debug_delete(pMsg);
		}
	}

	return LTTRUE;

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
	if( !m_bDraw )
		return;

	LTIntPt pos = m_BasePos;
	
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
					pMsg->Render(LTTRUE);
					
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

					pos.y -= pMsg->GetBaseHeight();
					pMsg->SetBasePos(pos);
					pMsg->Render(LTTRUE);
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
					pos.y -= pMsg->GetBaseHeight();
					pMsg->SetBasePos(pos);
					pMsg->Render();
				}

				iter++;
			}
		}
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

void CHUDMessageQueue::AddMessage(MsgCreate &fmt, bool bHistoryOnly)
{

	if (!bHistoryOnly)
	{
		//take the oldest active message out of the list
		MessageArray::iterator iter = m_ActiveMsgs.begin();
		CHUDMessage *pMsg = (*iter);
		m_ActiveMsgs.erase(iter);

		//overwrite the data
		pMsg->Create(fmt);
		pMsg->Show(LTTRUE);

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

void CHUDMessageQueue::ShowHistory(LTBOOL bShow) 
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
		(*iter)->Show(LTFALSE);
		iter++;
	}

}

void CHUDMessageQueue::SetHistoryOffset(uint16 nOffset) 
{
	uint16 nMaxOffset;
	if (m_History.size() <= m_HistoryMsgs.size())
		nMaxOffset = 0;
	else
		nMaxOffset = (m_History.size() - m_HistoryMsgs.size());
	
	if (nOffset > nMaxOffset)
		nOffset = nMaxOffset;

	m_nHistoryOffset = nOffset;

	uint8 nMsg = m_HistoryMsgs.size() - 1;
	uint8 nIndex = (m_History.size() - m_nHistoryOffset) - 1;

	while (nMsg < m_HistoryMsgs.size())
	{
		CHUDMessage* pMsg = m_HistoryMsgs[nMsg];
		if (  nIndex < m_History.size() )
		{
			pMsg->Create(*(m_History[nIndex]));
			pMsg->Show(LTTRUE);
			nIndex--;
		}
		else
			pMsg->Show(LTFALSE);

		nMsg--;
		

	}


}


//display an earlier page of history
void CHUDMessageQueue::IncHistoryOffset() 
{
	uint16 nMaxOffset;
	if (m_History.size() <= m_HistoryMsgs.size())
		nMaxOffset = 0;
	else
		nMaxOffset = (m_History.size() - m_HistoryMsgs.size());

	uint16 nOffset = m_nHistoryOffset + m_HistoryMsgs.size();

	if (nOffset > nMaxOffset)
		nOffset = nMaxOffset;

	SetHistoryOffset(nOffset);
}

//display an later page of history
void CHUDMessageQueue::DecHistoryOffset() 
{
	uint16 nOffset = m_nHistoryOffset;

	if (nOffset > m_HistoryMsgs.size())
		nOffset -= m_HistoryMsgs.size();
	else
		nOffset = 0;

	SetHistoryOffset(nOffset);

}

void CHUDChatMsgQueue::UpdateLayout()
{

	char *pTag = "ChatMessageQueue";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");

	m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	m_MsgFormat.nTextColor = argbWhite;
	m_MsgFormat.fDuration = g_pLayoutMgr->GetFloat(pTag,"MessageTime");
	m_MsgFormat.fFadeDur  = g_pLayoutMgr->GetFloat(pTag,"MessageFade");
	m_MsgFormat.nWidth  = (uint16) g_pLayoutMgr->GetInt(pTag,"Width");


	m_nMaxActiveMsgs = (uint8)g_pLayoutMgr->GetInt(pTag,"MaxMessages");
	m_nMaxHistoryMsgs = (uint8)g_pLayoutMgr->GetInt(pTag,"MaxHistory");



	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_nMsgColors[kMsgDefault] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"ChatColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgChat] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"CheatColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgCheatConfirm] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector("Transmission","TextColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgTransmission] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector( pTag, "ScmdColor" );
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgScmd] = SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"TeamColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgTeam] = SET_ARGB(0xFF,nR,nG,nB);
	
	vCol = g_pLayoutMgr->GetVector( pTag, "RedTeamColor" );
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgRedTeam] = SET_ARGB( 0xFF, nR, nG, nB );

	vCol = g_pLayoutMgr->GetVector( pTag, "BlueTeamColor" );
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nMsgColors[kMsgBlueTeam] = SET_ARGB( 0xFF, nR, nG, nB );

}

void CHUDChatMsgQueue::AddMessage(const char *pszString, eChatMsgType type)
{

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	fmt.nTextColor = m_nMsgColors[type];
	bool bHistoryOnly = (type == kMsgTransmission);
	CHUDMessageQueue::AddMessage(fmt,bHistoryOnly);

}

void CHUDChatMsgQueue::AddMessage(int nMessageID, eChatMsgType type)
{
	AddMessage(LoadTempString(nMessageID),type);
}


void CHUDPickupMsgQueue::UpdateLayout()
{
	m_bTopJustify = LTFALSE;

	char *pTag = "PickupMessageQueue";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");

	m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");
	m_MsgFormat.nImageSize = (uint8)g_pLayoutMgr->GetInt(pTag,"IconSize");

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_MsgFormat.nTextColor = SET_ARGB(0xFF,nR,nG,nB);

	m_MsgFormat.fDuration = g_pLayoutMgr->GetFloat(pTag,"MessageTime");
	m_MsgFormat.fFadeDur  = g_pLayoutMgr->GetFloat(pTag,"MessageFade");
	m_MsgFormat.nWidth  = (uint16) g_pLayoutMgr->GetInt(pTag,"Width");


	m_nMaxActiveMsgs = (uint8)g_pLayoutMgr->GetInt(pTag,"MaxMessages");

}


void CHUDPickupMsgQueue::AddMessage(const char *pszString,const char *pszImage)
{
	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	fmt.hImage = g_pInterfaceResMgr->GetTexture(pszImage);
	CHUDMessageQueue::AddMessage(fmt);

}

void CHUDPickupMsgQueue::AddMessage(int nMessageID,const char *pszImage)
{
	AddMessage(LoadTempString(nMessageID),pszImage);
}

void CHUDPickupMsgQueue::Render()
{
	// [KLS 7/17/02] Don't show pickup messages during cinematics...
	if (!g_pPlayerMgr->IsUsingExternalCamera())
	{
		CHUDMessageQueue::Render();
	}
}

void CHUDRewardMsgQueue::UpdateLayout()
{
	m_bTopJustify = LTFALSE;

	char *pTag = "RewardMessageQueue";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");

	m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	m_MsgFormat.nTextColor = argbWhite;
	m_MsgFormat.fDuration = g_pLayoutMgr->GetFloat(pTag,"MessageTime");
	m_MsgFormat.fFadeDur  = g_pLayoutMgr->GetFloat(pTag,"MessageFade");
	m_MsgFormat.nWidth  = (uint16) g_pLayoutMgr->GetInt(pTag,"Width");

	m_nMaxActiveMsgs = (uint8)g_pLayoutMgr->GetInt(pTag,"MaxMessages");
	m_nMaxHistoryMsgs = (uint8)g_pLayoutMgr->GetInt(pTag,"MaxHistory");

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_nMsgColors[kMsgDefault] = SET_ARGB(0xFF,nR,nG,nB);
}