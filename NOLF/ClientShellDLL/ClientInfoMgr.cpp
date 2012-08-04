#include "stdafx.h"
#include "ClientInfoMgr.h"
#include "iltclient.h"
#include "GameClientShell.h"
#include "clientres.h"
#include <stdio.h>

#define VERT_SPACING 3

extern CGameClientShell* g_pGameClientShell;

namespace
{
	char szUnknownPlayer[32] = "";
}


void ListClientFn(int argc, char **argv)
{
	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pCIMgr)
	{
		CLIENT_INFO *ptr = pCIMgr->GetFirstClient();
		while (ptr)
		{
			g_pLTClient->CPrint("%d %s %d",ptr->nID,g_pLTClient->GetStringData(ptr->hstrName),ptr->nFrags);
			ptr = ptr->pNext;
		}
	}

}

void AddClientFn(int argc, char **argv)
{
	HSTRING hstrName = LTNULL; 
	uint32 nID = 0;
	uint8 nTeam = 0;
	if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
	{
		nTeam = (uint8)GetRandom(1,2);
	}
	int nFragCount =GetRandom(3);

	if (argc > 0)
	{
		hstrName = g_pLTClient->CreateString(argv[0]);
		if (argc > 1)
		{
			nID = atoi(argv[1]);
		}
	}

	
	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pCIMgr && hstrName)
	{
		if (!nID)
			nID = pCIMgr->GetNumClients()+1;
		pCIMgr->AddClient(hstrName,nID,nFragCount,nTeam);
		ListClientFn(0,LTNULL);
	}

}

void AddABunchFn(int argc, char **argv)
{
	char szNames[15][16] = 
	{
		"aaa", "bbb", "ccc",
		"ddd", "eee", "fff",
		"ggg", "hhh", "iii",
		"jjj", "kkk", "lll",
		"mmm", "nnn", "ooo",
	};
	char *pName = &szNames[0][0];
	for (int i = 0; i < 15;i++)
	{
		
		AddClientFn(1,&pName);
		pName += 16;
	}
}


void RemoveClientFn(int argc, char **argv)
{
	HSTRING hstrName = LTNULL; 
	uint32 nID = 0;
	int nFragCount =0;

	if (argc > 0)
	{
		nID = atoi(argv[0]);
	}
	
	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pCIMgr)
	{
		pCIMgr->RemoveClient(nID);
		ListClientFn(0,LTNULL);
	}

}


void AddFragFn(int argc, char **argv)
{
	uint32 nID = 0;
	int nFragCount =0;
	if (argc > 0)
	{
		nID =  (uint32)atoi(argv[0]);
		if (argc > 1)
		{
			nFragCount = atoi(argv[1]);
		}
	}

	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pCIMgr)
	{
		CLIENT_INFO *ptr = pCIMgr->GetFirstClient();
		while (ptr)
		{
			if (ptr->nID == nID)
			{
				pCIMgr->AddFrag(nID);
				break;
			}
			ptr = ptr->pNext;
		}
	}

}

TEAM_INFO::TEAM_INFO()
{
	szName[0] = '\0';
	hColor = LTNULL;
	hBanner = LTNULL;
	nScore = 0;
}

TEAM_INFO::~TEAM_INFO()
{
	if (hBanner)
	{
		g_pLTClient->DeleteSurface (hBanner);
		hBanner = LTNULL;
	}
}


CClientInfoMgr::CClientInfoMgr()
{
    m_pClients = LTNULL;

    m_hFragString = LTNULL;
    m_hTeamScore = LTNULL;
    m_hOppScore = LTNULL;

	m_Teams[0].nScore = 0;
	m_Teams[1].nScore = 0;
	m_bFragScore = LTTRUE;

	m_hTeamColor = kWhite;
	m_hOppColor = kWhite;


}

CClientInfoMgr::~CClientInfoMgr()
{
    CLIENT_INFO* ptr = LTNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		g_pLTClient->FreeString (m_pClients->hstrName);
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

	if (m_hFragString)
	{
		g_pLTClient->FreeString(m_hFragString);
	    m_hFragString = LTNULL;
	}
	if (m_hTeamScore)
	{
		g_pLTClient->FreeString(m_hTeamScore);
	    m_hTeamScore = LTNULL;
	}
	if (m_hOppScore)
	{
		g_pLTClient->FreeString(m_hOppScore);
	    m_hOppScore = LTNULL;
	}

}

void CClientInfoMgr::Init()
{
    g_pLTClient->RegisterConsoleProgram("AddClient", AddClientFn);
    g_pLTClient->RegisterConsoleProgram("AddABunch", AddABunchFn);
    g_pLTClient->RegisterConsoleProgram("ListClients", ListClientFn);
    g_pLTClient->RegisterConsoleProgram("RemoveClient", RemoveClientFn);
    g_pLTClient->RegisterConsoleProgram("AddFrag", AddFragFn);

	HSTRING hTeam = g_pLTClient->FormatString(IDS_PLAYER_UNITY);
	SAFE_STRCPY(m_Teams[0].szName,g_pLTClient->GetStringData(hTeam));
	g_pLTClient->FreeString(hTeam);

	hTeam = g_pLTClient->FormatString(IDS_PLAYER_HARM);
	SAFE_STRCPY(m_Teams[1].szName,g_pLTClient->GetStringData(hTeam));
	g_pLTClient->FreeString(hTeam);

	hTeam = g_pLTClient->FormatString(IDS_UNKNOWN_PLAYER);
	SAFE_STRCPY(szUnknownPlayer,g_pLTClient->GetStringData(hTeam));
	g_pLTClient->FreeString(hTeam);

	m_Teams[0].hColor = g_pLayoutMgr->GetTeam1Color();
	m_Teams[1].hColor = g_pLayoutMgr->GetTeam2Color();

    LTRect rect(0,0,2,2);
	for (int i = 0; i < 2; i++)
	{
		m_Teams[i].hBanner = g_pLTClient->CreateSurface(2,2);
		g_pLTClient->FillRect(m_Teams[i].hBanner,&rect,m_Teams[i].hColor);
		g_pLTClient->OptimizeSurface (m_Teams[i].hBanner, LTNULL);
		g_pLTClient->SetSurfaceAlpha(m_Teams[i].hBanner,0.3f);

		m_Teams[i].nScore = 0;
	}


	m_nLocalID = 0;
	m_bIsTeamGame = LTFALSE;
}

void CClientInfoMgr::AddClient (HSTRING hstrName, uint32 nID, int nFragCount, uint8 team)
{
	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	m_bIsTeamGame = (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT);

	// if we already have this client in the list, just return

	CLIENT_INFO* pDup = m_pClients;
	while (pDup)
	{
		if (pDup->nID == nID)
		{
			if (pDup->hstrName)
			{
				g_pLTClient->FreeString(pDup->hstrName);
			}

			pDup->hstrName = hstrName;

			
			if (pDup->team != team)
			{
				ChangeClientTeam(nID,team);
				UpdateClientSort(pDup);
			}
			else
			{
				UpdateClientSort(pDup);
				// update the frag display
				if (m_nLocalID == nID)
				{
					UpdateFragDisplay();
				}
			}

			return;
		}
		pDup = pDup->pNext;
	}

	// create the new object

	CLIENT_INFO* pNew = debug_new(CLIENT_INFO);
	if (!pNew) return;

	pNew->nID = nID;
	pNew->hstrName = hstrName;

	pNew->nFrags = nFragCount;
	pNew->team = team;

	// if this client is us, update our frag display
	if (pNew->nID == m_nLocalID)
	{
		UpdateFragDisplay();
	}

	// if we don't have a list yet, set the list pointer to the new object

	if (!m_pClients)
	{
		m_pClients = pNew;
		return;
	}

	// we do have a list - insert the object
	CLIENT_INFO* ptr = m_pClients;

	//insert at head?
	if (ptr->nFrags < pNew->nFrags)
	{
		pNew->pNext = m_pClients;
		m_pClients = pNew;
		return;
	}

	CLIENT_INFO* pNext = ptr->pNext;
	while (pNext && pNext->nFrags >= pNew->nFrags)
	{
		ptr = pNext;
		pNext = ptr->pNext;
	}
	if (pNext)
	{
		pNext->pPrev = pNew;
	}
	ptr->pNext = pNew;
	pNew->pNext = pNext;
	pNew->pPrev = ptr;

}

void CClientInfoMgr::RemoveClient (uint32 nID)
{
	if (!m_pClients) return;

	m_nLocalID = 0;
	g_pLTClient->GetLocalClientID (&m_nLocalID);

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID) break;
		ptr = ptr->pNext;
	}
	if (!ptr) return;

	// remove the client from the list

	if (ptr->pNext) 
		ptr->pNext->pPrev = ptr->pPrev;
	if (ptr->pPrev) 
		ptr->pPrev->pNext = ptr->pNext;
	if (m_pClients == ptr) 
		m_pClients = ptr->pNext;

	g_pLTClient->FreeString (ptr->hstrName);
	debug_delete(ptr);

}

void CClientInfoMgr::RemoveAllClients()
{

    CLIENT_INFO* ptr = LTNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		g_pLTClient->FreeString (m_pClients->hstrName);
		debug_delete(m_pClients);
		m_pClients = ptr;
	}

	m_Teams[0].nScore = 0;
	m_Teams[1].nScore = 0;

	UpdateFragDisplay();
}

void CClientInfoMgr::ChangeClientTeam(uint32 nID, uint8 teamId)
{
	if (!m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// add to the frag count
			ptr->team = teamId;

			// update the frag display
			if (m_nLocalID == nID)
			{	
				UpdateFragDisplay();
			}

			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) g_pInterfaceMgr->UpdateClientList();
}

void CClientInfoMgr::SetLives (uint32 nID, uint8 nLives)
{
	if (!m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// add to the frag count
			ptr->nLives = nLives;

			// update the frag display
			if (nID == m_nLocalID)
				UpdateFragDisplay();

			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) g_pInterfaceMgr->UpdateClientList();

}

void CClientInfoMgr::AddFrag (uint32 nID)
{
	if (!m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// add to the frag count
			ptr->nFrags++;

			if ((ptr->team == 1 || ptr->team == 2) && m_bFragScore)
			{
				m_Teams[ptr->team-1].nScore++;
			}

			UpdateClientSort (ptr);

			// update the frag display
			if (nID == m_nLocalID || m_bIsTeamGame)
			{	
				UpdateFragDisplay();
			}


			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) g_pInterfaceMgr->UpdateClientList();

}

void CClientInfoMgr::UpdateClientSort(CLIENT_INFO* pCur)
{
	if (!pCur) return;

	// put this client in the correct position in the list (most frags to least frags)
	CLIENT_INFO* pTmp = pCur;

	LTBOOL bBack = LTFALSE;
	//try going forward
	while (pTmp->pPrev && pCur->nFrags > pTmp->pPrev->nFrags)	
		pTmp = pTmp->pPrev;
	if (pTmp == pCur)
	{
		//try going backward
		while (pTmp->pNext && pCur->nFrags < pTmp->pNext->nFrags)	
			pTmp = pTmp->pNext;
		if (pTmp == pCur) return;
		bBack = LTTRUE;

	}

	// we found a new position - remove current from the list
	if (pCur->pPrev)
		pCur->pPrev->pNext = pCur->pNext;
	else
		m_pClients = pCur->pNext;
	if (pCur->pNext) 
		pCur->pNext->pPrev = pCur->pPrev;

	// put us back in in the correct position
	if (bBack)
	{
		pCur->pPrev = pTmp;
		pCur->pNext = pTmp->pNext;
		pTmp->pNext = pCur;
		if (pCur->pNext)
			pCur->pNext->pPrev = pCur;
	}
	else
	{
		pCur->pPrev = pTmp->pPrev;
		pCur->pNext = pTmp;
		if (pTmp->pPrev) 
			pTmp->pPrev->pNext = pCur;
		else
			m_pClients = pCur;
		pTmp->pPrev = pCur;
	}


}


void CClientInfoMgr::AddScore(uint32 nID, uint8 teamId, uint32 nScore)
{
	if (!m_pClients) return;
	if (teamId == 1 || teamId == 2)
	{
		
		m_Teams[teamId-1].nScore += nScore;
		CLIENT_INFO* ptr = GetClientByID(nID);
		if (ptr)
		{
			ptr->nFrags += nScore;
			UpdateClientSort(ptr);
		}
		else
			g_pInterfaceMgr->UpdateClientList();


		// update the frag display
		if (nID == m_nLocalID || m_bIsTeamGame)
		{	
			UpdateFragDisplay();
		}

	}
}

void CClientInfoMgr::RemoveFrag (uint32 nID)
{
	if (!m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// remove from the frag count
			ptr->nFrags--;

			UpdateClientSort (ptr);

			// update the frag display
			if (nID == m_nLocalID || m_bIsTeamGame)
			{	
				UpdateFragDisplay();
			}


			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) g_pInterfaceMgr->UpdateClientList();;
}

uint32 CClientInfoMgr::GetNumClients()
{
	if (!m_pClients) return 0;

	CLIENT_INFO* ptr = m_pClients;

    uint32 nCount = 0;
	while (ptr)
	{
		nCount++;
		ptr = ptr->pNext;
	}

	return nCount;
}

char* CClientInfoMgr::GetPlayerName (uint32 nID)
{
    if (!m_pClients) return szUnknownPlayer;

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID) return g_pLTClient->GetStringData (ptr->hstrName);
		ptr = ptr->pNext;
	}
	
	g_pInterfaceMgr->UpdateClientList();
	return szUnknownPlayer;
}

void CClientInfoMgr::UpdateFragDisplay ()
{
	if (!m_pClients || !g_pInterfaceMgr) return;

	if (m_hFragString)
	{
		g_pLTClient->FreeString(m_hFragString);
	    m_hFragString = LTNULL;
	}
	if (m_hTeamScore)
	{
		g_pLTClient->FreeString(m_hTeamScore);
	    m_hTeamScore = LTNULL;
	}
	if (m_hOppScore)
	{
		g_pLTClient->FreeString(m_hOppScore);
	    m_hOppScore = LTNULL;
	}

	// get our local id
	CLIENT_INFO *pLocal = GetLocalClient();
	if (!pLocal) return;


	char strTeam1[32];
	char strTeam2[32];
	char strPlayer[32];
	sprintf(strTeam1,"%s: %d ",m_Teams[0].szName,m_Teams[0].nScore);
	sprintf(strTeam2,"%s: %d ",m_Teams[1].szName,m_Teams[1].nScore);
	if (pLocal->hstrName)
		sprintf(strPlayer,"%s: %d ",g_pLTClient->GetStringData(pLocal->hstrName),pLocal->nFrags);
	if (pLocal->team == 1)
	{
		m_hFragString = g_pLTClient->CreateString(strPlayer);
		m_hTeamScore = g_pLTClient->CreateString(strTeam1);
		m_hOppScore = g_pLTClient->CreateString(strTeam2);
		m_hTeamColor = m_Teams[0].hColor;
		m_hOppColor = m_Teams[1].hColor;
	}
	else if (pLocal->team == 2)
	{
		m_hFragString = g_pLTClient->CreateString(strPlayer);
		m_hTeamScore = g_pLTClient->CreateString(strTeam2);
		m_hOppScore = g_pLTClient->CreateString(strTeam1);
		m_hTeamColor = m_Teams[1].hColor;
		m_hOppColor = m_Teams[0].hColor;
	}
	else
	{
		sprintf(strPlayer,"%d ",pLocal->nFrags);
		m_hFragString = g_pLTClient->CreateString(strPlayer);
		m_hTeamColor = kWhite;
	}

	int nPos = 4;
	if (m_hTeamScore)
	{
		LTIntPt sz = g_pInterfaceResMgr->GetMsgForeFont()->GetTextExtents(m_hTeamScore);
		m_TeamPos.x = sz.x+4;
		m_TeamPos.y = nPos;
		nPos += sz.y;
	}
	if (m_hOppScore)
	{
		LTIntPt sz = g_pInterfaceResMgr->GetMsgForeFont()->GetTextExtents(m_hOppScore);
		m_OppPos.x = sz.x+4;
		m_OppPos.y = nPos;
		nPos += sz.y;
		nPos += 4;
	}
	if (m_hFragString)
	{
		LTIntPt sz = g_pInterfaceResMgr->GetMsgForeFont()->GetTextExtents(m_hFragString);
		m_FragPos.x = sz.x+4;
		m_FragPos.y = nPos;
	}
}


void CClientInfoMgr::Draw (LTBOOL bDrawSingleFragCount, LTBOOL bDrawAllFragCounts, HSURFACE hDestSurf)
{
	if (!m_pClients || (!bDrawSingleFragCount && !bDrawAllFragCounts)) return;

	// make sure we're in a network game
	if (g_pGameClientShell->GetGameType() == SINGLE) return;

	HSURFACE hScreen = hDestSurf;
	if (!hScreen)
		hScreen = g_pLTClient->GetScreenSurface();
	if (!hScreen)
		return;

    uint32 nScreenWidth = 0;
    uint32 nScreenHeight = 0;
	g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	CLTGUIFont *pFont = g_pInterfaceResMgr->GetMsgForeFont();

	// should we draw our frag count?
	if (bDrawSingleFragCount)
	{
		
		if (m_hTeamScore)
		{
			pFont->Draw(m_hTeamScore,hScreen,nScreenWidth-m_TeamPos.x+1,m_TeamPos.y+1,LTF_JUSTIFY_LEFT);	
			pFont->Draw(m_hTeamScore,hScreen,nScreenWidth-m_TeamPos.x,m_TeamPos.y,LTF_JUSTIFY_LEFT,m_hTeamColor);	
		}
		if (m_hOppScore)
		{
			pFont->Draw(m_hOppScore,hScreen,nScreenWidth-m_OppPos.x+1,m_OppPos.y+1,LTF_JUSTIFY_LEFT);	
			pFont->Draw(m_hOppScore,hScreen,nScreenWidth-m_OppPos.x,m_OppPos.y,LTF_JUSTIFY_LEFT,m_hOppColor);	
		}
		if (m_hFragString)
		{
			pFont->Draw(m_hFragString,hScreen,nScreenWidth-m_FragPos.x+1,m_FragPos.y+1,LTF_JUSTIFY_LEFT);	
			pFont->Draw(m_hFragString,hScreen,nScreenWidth-m_FragPos.x,m_FragPos.y,LTF_JUSTIFY_LEFT,m_hTeamColor);	
		}
	}

	// should we draw all the frag counts?

	if (bDrawAllFragCounts)
	{
		int nHeight[2] = {0,0};
		int nClients = 0;
		int nTeams[2] = {0,0};
		int nLineHeight = pFont->GetHeight() + VERT_SPACING;

		int nMaxHeight = nScreenHeight - 16;

		CLIENT_INFO* pClient = m_pClients;
		while (pClient)
		{
			if (pClient->team == 2)
			{
				nHeight[1] += nLineHeight;
				nTeams[1]++;
			}
			else
			{
				nHeight[0] += nLineHeight;
				nTeams[0]++;
			}
			++nClients;
			pClient = pClient->pNext;
		}

		int nTotalHeight = Max(nHeight[0],nHeight[1]);
		if (nTotalHeight > nMaxHeight)
			nTotalHeight = nMaxHeight;

		int nY = ((int)nScreenHeight - nTotalHeight) / 2;
		if (nY < 0) nY = 0;
		int nY2 = nY;

		int nX  = 64;
		int nX2 = 32 + (int)nScreenWidth / 2;
		int nTab = ((int)nScreenWidth / 2) - 32;
		int nTab2 = (int)nScreenWidth - 64;

		
		if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
		{
			LTRect rcBanner(nX-8,nY-8,nTab+8,nY+nTotalHeight);
			if (m_Teams[0].hBanner)
				g_pLTClient->ScaleSurfaceToSurface(hScreen, m_Teams[0].hBanner, &rcBanner, LTNULL);

			rcBanner = LTRect(nX2-8,nY-8,nTab2+8,nY+nTotalHeight);
			if (m_Teams[1].hBanner)
				g_pLTClient->ScaleSurfaceToSurface(hScreen, m_Teams[1].hBanner, &rcBanner, LTNULL);
		}

        LTBOOL filled[2] = {LTFALSE, LTFALSE};
		
		pClient = m_pClients;
		while (pClient)
		{
			char str[64];
			sprintf(str,"%d (%d)",pClient->nFrags,(uint32)pClient->m_Ping);
			LTBOOL bIsLocal = (pClient->nID == m_nLocalID);
			HLTCOLOR hColor = kWhite;
			if (bIsLocal)
			{
				hColor = SETRGB(255,255,0);
			}
			
			if (pClient->team == 2)
			{
				if (!filled[1])
				{
					// Ok.. draw.
					pFont->Draw(pClient->hstrName,hScreen,nX2+1,nY2+1,LTF_JUSTIFY_LEFT);	
					pFont->Draw(pClient->hstrName,hScreen,nX2,nY2,LTF_JUSTIFY_LEFT,hColor);
					pFont->Draw(str,hScreen,nTab2+1,nY2+1,LTF_JUSTIFY_RIGHT);	
					pFont->Draw(str,hScreen,nTab2,nY2,LTF_JUSTIFY_RIGHT,hColor);


					nY2 += nLineHeight;
					if (nY2 + nLineHeight > (int)nMaxHeight)
					{
                        filled[1] = LTTRUE;
					}

				}
			}
			else if (!filled[0])
			{
				// Ok.. draw.
				pFont->Draw(pClient->hstrName,hScreen,nX+1,nY+1,LTF_JUSTIFY_LEFT);	
				pFont->Draw(str,hScreen,nTab+1,nY+1,LTF_JUSTIFY_RIGHT);	
				pFont->Draw(pClient->hstrName,hScreen,nX,nY,LTF_JUSTIFY_LEFT,hColor);
				pFont->Draw(str,hScreen,nTab,nY,LTF_JUSTIFY_RIGHT,hColor);

				nY += nLineHeight;
				if (nY + nLineHeight > (int)nMaxHeight)
				{
                    filled[0] = LTTRUE;
				}

			}

			pClient = pClient->pNext;

		}
	}
}

CLIENT_INFO* CClientInfoMgr::GetLocalClient()
{
	return GetClientByID(m_nLocalID);
}

CLIENT_INFO* CClientInfoMgr::GetClientByID(uint32 nID)
{
	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
			return ptr;

		ptr = ptr->pNext;
	}
	g_pInterfaceMgr->UpdateClientList();
    return LTNULL;
}
