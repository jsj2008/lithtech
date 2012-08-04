//*********************************************************************************

#include <stdio.h>
#include "FragInfo.h"
#include "cpp_client_de.h"
#include "TeamMgr.h"
#include "BloodClientShell.h"

#define FRAG_NAME_PIXEL_WIDTH	145

//*********************************************************************************

CFragInfo::CFragInfo()
{
	m_pClients = DNULL;
	m_pClientDE = DNULL;

	m_pFont = DNULL;
	m_pCursor = DNULL;

	for (int i = 0; i < NUM_FRAG_PICS; i++)
	{
		m_hFragPics[i] = 0;
	}

	m_hFragBar = 0;
	m_hFragTable = 0;

	m_nFragTableX = 0;
	m_nFragTableY = 0;
	m_nFragTableHeight = 0;
	m_nFragTableMaxDisp = 0;

	m_nScreenWidth = 0;
	m_nScreenHeight = 0;

	m_hTransColor = 0;
}

//*********************************************************************************

CFragInfo::~CFragInfo()
{
	if (!m_pClientDE) return;

	CLIENT_INFO* ptr = DNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		m_pClientDE->FreeString (m_pClients->hstrName);
		delete m_pClients;
		m_pClients = ptr;
	}

	if (m_pFont) 
		delete m_pFont;

	if (m_pCursor)
		delete m_pCursor;

	for (int i = 0; i < NUM_FRAG_PICS; i++)
	{
		if(m_hFragPics[i]) { m_pClientDE->DeleteSurface(m_hFragPics[i]); m_hFragPics[i] = 0; }
	}

	if(m_hFragBar)			{ m_pClientDE->DeleteSurface(m_hFragBar); m_hFragBar = 0; }
	if(m_hFragTable)		{ m_pClientDE->DeleteSurface(m_hFragTable); m_hFragTable = 0; }
}

//*********************************************************************************

void CFragInfo::Init (CClientDE* pClientDE, CTeamMgr* pTeamMgr)
{
	// Sanity checks
	ASSERT(pClientDE);
	ASSERT(pTeamMgr);

	// Setup main pointers
	m_pClientDE = pClientDE;
	m_pTeamMgr  = pTeamMgr;

	// Setup surfaces
	m_hFragPics[0]	= m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/c_frag.pcx");
	m_hFragPics[1]	= m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/o_frag.pcx");
	m_hFragPics[2]	= m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/i_frag.pcx");
	m_hFragPics[3]	= m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/g_frag.pcx");

#ifdef _ADDON
	m_hFragPics[4]	= m_pClientDE->CreateSurfaceFromBitmap("interface_ao/fragbar/mc_frag.pcx");
	m_hFragPics[5]	= m_pClientDE->CreateSurfaceFromBitmap("interface_ao/fragbar/fc_frag.pcx");
	m_hFragPics[6]	= m_pClientDE->CreateSurfaceFromBitmap("interface_ao/fragbar/sd_frag.pcx");
	m_hFragPics[7]	= m_pClientDE->CreateSurfaceFromBitmap("interface_ao/fragbar/p_frag.pcx");
#endif

	m_hFragPics[FRAG_PIC_VOICE]   = m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/v_frag.pcx");
	m_hFragPics[FRAG_PIC_UNKNOWN] = m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/u_frag.pcx");

	m_hFragBar		= m_pClientDE->CreateSurfaceFromBitmap("interface/fragbar/fragbar.pcx");

	// Setup fonts
	m_pFont = new CoolFont();
	m_pFont->Init(m_pClientDE, "interface/fragbar/frag_font_1.pcx");
	m_pFont->LoadXWidths("interface/fragbar/frag_font_1.fnt");

	// Setup cursor
	m_pCursor = new CoolFontCursor();
	m_pCursor->SetFont(m_pFont);

	// Setup transparency color
	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);

	// Setup starting team id for single column drawing
	m_nTeamID = 0;
}

//*********************************************************************************

void CFragInfo::AddClient (HSTRING hstrName, DDWORD nID, int nFragCount, DBYTE byCharacter)
{
	if (!m_pClientDE) return;

	// if we already have this client in the list, just return

	CLIENT_INFO* pDup = m_pClients;
	while (pDup)
	{
		if (pDup->nID == nID) return;
		pDup = pDup->pNext;
	}

	// create the new object

	CLIENT_INFO* pNew = new CLIENT_INFO;
	if (!pNew) return;

	pNew->nID = nID;
	pNew->hstrName = m_pClientDE->CopyString(hstrName);
	pNew->nFrags = nFragCount;
	pNew->byCharacter = byCharacter;

	m_pClientDE->CPrint(m_pClientDE->GetStringData(hstrName));

	// if we don't have a list yet, set the list pointer to the new object

	if (!m_pClients)
	{
		m_pClients = pNew;
		return;
	}

	// we do have a list - add the new object at the end

	CLIENT_INFO* ptr = m_pClients;
	while (ptr->pNext)
	{
		ptr = ptr->pNext;
	}
	ptr->pNext = pNew;
	pNew->pPrev = ptr;

	// Add zero frags so that the player will get sorted properly...

	DDWORD dwLocalID = 0;
	m_pClientDE->GetLocalClientID(&dwLocalID);
	AddFrags(dwLocalID, nID, 0);
}

//*********************************************************************************

void CFragInfo::RemoveClient (DDWORD nID)
{
	if (!m_pClientDE || !m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID) break;
		ptr = ptr->pNext;
	}
	if (!ptr) return;

	// remove the client from the list

	if (ptr->pNext) ptr->pNext->pPrev = ptr->pPrev;
	if (ptr->pPrev) ptr->pPrev->pNext = ptr->pNext;
	if (m_pClients == ptr) m_pClients = ptr->pNext;

	m_pClientDE->FreeString (ptr->hstrName);
	delete ptr;
}

//*********************************************************************************

CLIENT_INFO* CFragInfo::GetClientInfo(DDWORD dwID)
{
	// Sanity checks...

	if (!m_pClients) return(NULL);


	// Find the client...

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == dwID) return(ptr);
		ptr = ptr->pNext;
	}


	// If we get here, we didn't find the client...

	return(NULL);
}

//*********************************************************************************

void CFragInfo::RemoveAllClients()
{
	if (!m_pClientDE) return;

	CLIENT_INFO* ptr = DNULL;
	while (m_pClients)
	{
		ptr = m_pClients->pNext;
		m_pClientDE->FreeString (m_pClients->hstrName);
		delete m_pClients;
		m_pClients = ptr;
	}
}

//*********************************************************************************

void CFragInfo::AddFrag (DDWORD nLocalID, DDWORD nID)
{
	if (!m_pClientDE || !m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// add to the frag count
			ptr->nFrags++;
			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) return;

	// put this client in the correct position in the list (most frags to least frags)

	CLIENT_INFO* pCurrent = ptr;
	while (ptr->pPrev && pCurrent->nFrags > ptr->pPrev->nFrags)	ptr = ptr->pPrev;
	if (ptr == pCurrent) return;

	// we found a new position - remove current from the list

	pCurrent->pPrev->pNext = pCurrent->pNext;
	if (pCurrent->pNext) pCurrent->pNext->pPrev = pCurrent->pPrev;

	// put us back in in the correct position

	if (!ptr->pPrev)
	{
		m_pClients = pCurrent;
	}

	pCurrent->pPrev = ptr->pPrev;
	pCurrent->pNext = ptr;
	if (ptr->pPrev) ptr->pPrev->pNext = pCurrent;
	ptr->pPrev = pCurrent;
}

//*********************************************************************************

void CFragInfo::AddFrags (DDWORD nLocalID, DDWORD nID, int nFrags)
{
	if (!m_pClientDE || !m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// add to the frag count
			ptr->nFrags += nFrags;
			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) return;

	// put this client in the correct position in the list (most frags to least frags)

	CLIENT_INFO* pCurrent = ptr;
	while (ptr->pPrev && pCurrent->nFrags > ptr->pPrev->nFrags)	ptr = ptr->pPrev;
	if (ptr == pCurrent) return;

	// we found a new position - remove current from the list

	pCurrent->pPrev->pNext = pCurrent->pNext;
	if (pCurrent->pNext) pCurrent->pNext->pPrev = pCurrent->pPrev;

	// put us back in in the correct position

	if (!ptr->pPrev)
	{
		m_pClients = pCurrent;
	}

	pCurrent->pPrev = ptr->pPrev;
	pCurrent->pNext = ptr;
	if (ptr->pPrev) ptr->pPrev->pNext = pCurrent;
	ptr->pPrev = pCurrent;
}

//*********************************************************************************

void CFragInfo::RemoveFrag (DDWORD nLocalID, DDWORD nID)
{
	if (!m_pClientDE || !m_pClients) return;

	// find the client

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID)
		{
			// remove from the frag count
			ptr->nFrags--;
			break;
		}
		ptr = ptr->pNext;
	}
	if (!ptr) return;

	// put this client in the correct position in the list (most frags to least frags)

	CLIENT_INFO* pCurrent = ptr;
	while (ptr->pNext && pCurrent->nFrags < ptr->pNext->nFrags)	ptr = ptr->pNext;
	if (ptr == pCurrent) return;

	// we found a new position - remove current from the list

	pCurrent->pNext->pPrev = pCurrent->pPrev;
	if (pCurrent->pPrev) pCurrent->pPrev->pNext = pCurrent->pNext;
	if (m_pClients == pCurrent) m_pClients = pCurrent->pNext;

	// put us back in in the correct position

	pCurrent->pPrev = ptr;
	pCurrent->pNext = ptr->pNext;
	if (ptr->pNext) ptr->pNext->pPrev = pCurrent;
	ptr->pNext = pCurrent;
}

//*********************************************************************************

DDWORD CFragInfo::GetNumClients()
{
	if (!m_pClientDE) return 0;

	CLIENT_INFO* ptr = m_pClients;

	DDWORD nCount = 0;
	while (ptr)
	{
		nCount++;
		ptr = ptr->pNext;
	}

	return nCount;
}

//*********************************************************************************

char* CFragInfo::GetPlayerName (DDWORD nID)
{
	if (!m_pClientDE) return DNULL;

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID) return m_pClientDE->GetStringData (ptr->hstrName);
		ptr = ptr->pNext;
	}

	return DNULL;
}

//*********************************************************************************

DBYTE CFragInfo::GetPlayerCharacter (DDWORD nID)
{
	if (!m_pClientDE) return DNULL;

	CLIENT_INFO* ptr = m_pClients;
	while (ptr)
	{
		if (ptr->nID == nID) return ptr->byCharacter;
		ptr = ptr->pNext;
	}

	return 0;
}

//*********************************************************************************

void CFragInfo::Draw (DBOOL bDrawSingleFragCount, DBOOL bDrawAllFragCounts)
{
	if (!m_pClientDE) return;// || (!bDrawSingleFragCount && !bDrawAllFragCounts)) return;

	// make sure we're in a network game
	int nGameMode = 0;
	m_pClientDE->GetGameMode(&nGameMode);
	if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return;

	// Update resolution stuff
	AdjustRes();

	// Draw the team or normal frag table as necessary
	if (g_pBloodClientShell->IsMultiplayerTeamBasedGame())	
	{
		DrawTeams();
	}
	else
	{
		UpdateFragTable();
		HSURFACE	hScreen = m_pClientDE->GetScreenSurface();
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hFragTable, DNULL, m_nFragTableX, m_nFragTableY, m_hTransColor);
	}
}

//*********************************************************************************

void CFragInfo::Draw (DBOOL bDrawSingleFragCount, DBOOL bDrawAllFragCounts, HSURFACE hSurf)
{
	if (!m_pClientDE) return;// || (!bDrawSingleFragCount && !bDrawAllFragCounts)) return;

	// make sure we're in a network game
	int nGameMode = 0;
	m_pClientDE->GetGameMode(&nGameMode);
	if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return;

	// Update resolution stuff
	AdjustRes();

	// Draw the team or normal frag table as necessary
	if (g_pBloodClientShell->IsMultiplayerTeamBasedGame())	
	{
		DrawTeams();
	}
	else
	{
		UpdateFragTable();
		HSURFACE	hScreen = m_pClientDE->GetScreenSurface();
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hFragTable, DNULL, m_nFragTableX, m_nFragTableY, m_hTransColor);
	}
}

//*********************************************************************************

void CFragInfo::AdjustRes()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen = m_pClientDE->GetScreenSurface();
	DDWORD		width, height;

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);

	// If we're at the same resolution, just return
	if((width == m_nScreenWidth) && (height == m_nScreenHeight))
		return;

	// Otherwise set the variables to the new resolution
	m_nScreenWidth = width;
	m_nScreenHeight = height;

	m_pClientDE->GetSurfaceDims(m_hFragBar, &width, &height);
	m_nFragTableX = (m_nScreenWidth - width) / 2;
	m_nFragTableY = height;
	m_nFragTableHeight = m_nScreenHeight - height - height;
	m_nFragTableMaxDisp = m_nFragTableHeight / height;

	if(m_hFragTable)	{ m_pClientDE->DeleteSurface(m_hFragTable); m_hFragTable = 0; }
	m_hFragTable = m_pClientDE->CreateSurface(width, m_nFragTableHeight);
	m_pClientDE->FillRect(m_hFragTable, DNULL, m_hTransColor);
}

//*********************************************************************************

void CFragInfo::UpdateFragTable()
{
	if(!m_pClientDE || !m_pClients || !m_hFragTable) return;

	DDWORD		barWidth, barHeight;
	DDWORD		picWidth, picHeight;
	DBYTE		currentPic = 0;
	DDWORD		numClients = 0;
	DDWORD		i = 0, numDisp = 0;
	DDWORD		startY = 0;

	CLIENT_INFO* pClient = m_pClients;

	// Get the width and height of a frag display for just one character
	m_pClientDE->GetSurfaceDims(m_hFragBar, &barWidth, &barHeight);

	// Count the number of clients in the game
	while(pClient)
		{ numClients++; pClient = pClient->pNext; }

	// Find the maximum number of clients to display frags for
	if(numClients <= m_nFragTableMaxDisp)
		numDisp = numClients;
	else
		numDisp = m_nFragTableMaxDisp;

	// Check if we should be in scrunch mode...
	DBOOL bScrunch = DFALSE;
	if (m_nScreenHeight < 350)
	{
		bScrunch = DTRUE;
	}

	// Find out where to start drawing within the frag table
	startY = (m_nFragTableHeight - (numDisp * barHeight)) / 2;

	// Clear the frag screen and setup the variables
	m_pClientDE->FillRect(m_hFragTable, DNULL, m_hTransColor);
	m_pCursor->SetDest(m_hFragTable);
	pClient = m_pClients;

	while(pClient && (i < numDisp))
	{
		char temp[100];

		// Choose which icon to draw for this client
		currentPic = pClient->byCharacter;

		if(currentPic < 0 || currentPic > (NUM_FRAG_PICS-2))
			currentPic = FRAG_PIC_UNKNOWN;

		// Get the width and height of a the character pic
		m_pClientDE->GetSurfaceDims(m_hFragPics[currentPic], &picWidth, &picHeight);

		// Draw the fragbar and icon for this client
		if (!bScrunch)
		{
			m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragBar, DNULL, 0, startY);
			m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragPics[currentPic], DNULL, 0, startY);
		}

		// Draw the name of the client
		m_pCursor->SetLoc((short)picWidth, (short)(startY + ((barHeight - m_pFont->height) / 2)));
		m_pCursor->SetJustify(CF_JUSTIFY_LEFT);
		DrawClip(GetDisplayPingAndName(pClient, temp), FRAG_NAME_PIXEL_WIDTH);

		// Draw the number of frag the client has
		sprintf(temp, "%d", pClient->nFrags);
		m_pCursor->SetLoc((short)(barWidth - FRAG_COUNT_OFFSET), (short)(startY + ((barHeight - m_pFont->height) / 2)));
		m_pCursor->SetJustify(CF_JUSTIFY_CENTER);
		m_pCursor->Draw(temp);

		// Go to the next client
		pClient = pClient->pNext;
		startY += barHeight;
		if (bScrunch) startY -= 18;
		i++;
	}
}

//*********************************************************************************

void CFragInfo::DrawTeams()
{
	// Check if only one colum will fit on the screen...

	if (m_nScreenWidth < 500)
	{
		DrawSingleColumnTeams();
		return;
	}


	// Sort the players by frag counts...

	m_pTeamMgr->SortPlayersByFrags();


	// Get the width and height of a frag display for just one character..

	DDWORD	barWidth;
	DDWORD	barHeight;

	m_pClientDE->GetSurfaceDims(m_hFragBar, &barWidth, &barHeight);


	// Calculate the X draw coordinate...

	int cTeams = m_pTeamMgr->GetNumTeams();
	if (cTeams <= 0) return;

	int nTotalWidth = (cTeams * barWidth) + ((cTeams - 1) * FRAG_COLUMN_SPACE);
	int xDraw       = (m_nScreenWidth / 2) - (nTotalWidth / 2);

	if (xDraw < 0) xDraw = 0;


	// Calculate the Y draw coordinate...

	CTeam* pMostTeam = m_pTeamMgr->GetTeamWithMostPlayers(FALSE);
	int    cPlayers  = m_nFragTableMaxDisp;

	if (pMostTeam)
	{
		cPlayers = pMostTeam->GetNumPlayers();
	}

	cPlayers++;		// add one more row for the colulmn title

	int nTotalHeight = barHeight * cPlayers;
	int yDraw        = ((m_nScreenHeight / 2) - (nTotalHeight / 2)) - barHeight;

	if (yDraw < 0) yDraw = 0;


	// Draw each team...

	CTeam* pTeam  = m_pTeamMgr->GetFirstTeam();

	while (pTeam)
	{
		DrawTeamColumn(pTeam, xDraw, yDraw, m_nFragTableMaxDisp);

		pTeam = m_pTeamMgr->GetNextTeam(pTeam);

		xDraw += barWidth + FRAG_COLUMN_SPACE;
	}
}

//*********************************************************************************

void CFragInfo::DrawTeamColumn(CTeam* pTeam, int xDraw, int yDraw, int nMax)
{
	// Sanity checks...

	if (!pTeam) return;
	if (!m_hFragBar) return;


	// Clear the frag screen and setup the variables...

	m_pClientDE->FillRect(m_hFragTable, DNULL, m_hTransColor);
	m_pCursor->SetDest(m_hFragTable);


	// Get the width and height of a frag display for just one character...

	DDWORD	barWidth;
	DDWORD	barHeight;

	m_pClientDE->GetSurfaceDims(m_hFragBar, &barWidth, &barHeight);


	// Check if we should be in scrunch mode...

	DBOOL bScrunch = DFALSE;

	if (m_nScreenHeight < 350)
	{
		bScrunch = DTRUE;
	}


	// Calculate the xStart and yStart values...

	int xStart = 0;
	int yStart = (m_nFragTableHeight - (m_nFragTableMaxDisp * barHeight)) / 2;


	// Draw the column title with the team name and total score...

	DDWORD picWidth, picHeight;
	m_pClientDE->GetSurfaceDims(m_hFragPics[FRAG_PIC_VOICE], &picWidth, &picHeight);

	if (!bScrunch)
	{
		m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragBar, DNULL, xStart, yStart);
		m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragPics[FRAG_PIC_VOICE], DNULL, xStart, yStart);
	}

	m_pCursor->SetLoc((short)(xStart + picWidth), (short)(yStart + ((barHeight - m_pFont->height) / 2)));
	m_pCursor->SetJustify(CF_JUSTIFY_LEFT);
	m_pCursor->Draw(pTeam->GetName());

	char sTemp[16];
	sprintf(sTemp, "%d", pTeam->GetFrags());
	m_pCursor->SetLoc((short)(xStart + (barWidth - FRAG_COUNT_OFFSET)), (short)(yStart + ((barHeight - m_pFont->height) / 2)));
	m_pCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pCursor->Draw(sTemp);

	yStart += barHeight;
	yStart += m_pFont->height / 2;

	if (bScrunch) yStart -= 20;


	// Draw each player's frag count on this team...

	CTeamPlayer* pPlr   = pTeam->GetFirstPlayer();
	int          nCount = 0;

	while (pPlr)
	{
		// Find the client info for this player...

		CLIENT_INFO* pClient = GetClientInfo(pPlr->GetID());

		nCount++;

		if (pClient && nCount < nMax)
		{
			// Choose which icon to draw for this client
			DBYTE byPic = pClient->byCharacter;
			if (byPic < 0 || byPic > (NUM_FRAG_PICS-2)) byPic = FRAG_PIC_UNKNOWN;

			// Get the width and height of a the character pic
			DDWORD picWidth, picHeight;
			m_pClientDE->GetSurfaceDims(m_hFragPics[byPic], &picWidth, &picHeight);

			// Draw the fragbar and icon for this client
			if (!bScrunch)
			{
				m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragBar, DNULL, xStart, yStart);
				m_pClientDE->DrawSurfaceToSurface(m_hFragTable, m_hFragPics[byPic], DNULL, xStart, yStart);
			}

			// Draw the name of the client
			char sTemp[100];
			m_pCursor->SetLoc((short)(xStart + picWidth), (short)(yStart + ((barHeight - m_pFont->height) / 2)));
			m_pCursor->SetJustify(CF_JUSTIFY_LEFT);
			DrawClip(GetDisplayPingAndName(pClient, sTemp), FRAG_NAME_PIXEL_WIDTH);

			// Draw the number of frag the client has
			sprintf(sTemp, "%d", pClient->nFrags);
			m_pCursor->SetLoc((short)(xStart + (barWidth - FRAG_COUNT_OFFSET)), (short)(yStart + ((barHeight - m_pFont->height) / 2)));
			m_pCursor->SetJustify(CF_JUSTIFY_CENTER);
			m_pCursor->Draw(sTemp);

			// Update the drawing position
			yStart += barHeight;

			if (bScrunch) yStart -= 18;
		}


		// Get the next player...

		pPlr = pTeam->GetNextPlayer(pPlr);
	}

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	DRESULT  dr      = m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hFragTable, DNULL, xDraw, yDraw, m_hTransColor);
}


//*********************************************************************************

void CFragInfo::DrawSingleColumnTeams()
{
	// Sort the players by frag counts...

	m_pTeamMgr->SortPlayersByFrags();


	// Get the width and height of a frag display for just one character..

	DDWORD	barWidth;
	DDWORD	barHeight;

	m_pClientDE->GetSurfaceDims(m_hFragBar, &barWidth, &barHeight);


	// Calculate the X draw coordinate...

	int cTeams = 1;

	int nTotalWidth = (cTeams * barWidth) + ((cTeams - 1) * FRAG_COLUMN_SPACE);
	int xDraw       = (m_nScreenWidth / 2) - (nTotalWidth / 2);

	if (xDraw < 0) xDraw = 0;


	// Calculate the Y draw coordinate...

	CTeam* pTeam = m_pTeamMgr->GetTeam(m_nTeamID);
	if (!pTeam) return;

	int cPlayers  = pTeam->GetNumPlayers();

	cPlayers++;		// add one more row for the colulmn title

	int nTotalHeight = barHeight * cPlayers;
	int yDraw        = ((m_nScreenHeight / 2) - (nTotalHeight / 2)) - barHeight;

	if (yDraw < 0) yDraw = 0;


	// Draw the team...

	DrawTeamColumn(pTeam, xDraw, yDraw, m_nFragTableMaxDisp);
}

//*********************************************************************************

void CFragInfo::TurnOn()
{
	// Get the next team to draw for team based toggle draw mode...

	m_nTeamID++;
	
	CTeam* pTeam = m_pTeamMgr->GetTeam(m_nTeamID);
	if (!pTeam)
	{
		m_nTeamID = 1;
	}
}

void CFragInfo::TurnOff()
{

}

//*********************************************************************************

char* CFragInfo::GetDisplayPingAndName(CLIENT_INFO* pClient, char* sBuf)
{
	// Sanity checks...

	if (!sBuf) return(DNULL);
	strcpy(sBuf, "");
	if (!pClient) return(sBuf);


	// Copy the ping and name into the given buffer...

	if (pClient->nPing <= 0)
	{
		sprintf(sBuf, "(-) %s", m_pClientDE->GetStringData(pClient->hstrName));
	}
	else
	{
		sprintf(sBuf, "(%i) %s", pClient->nPing, m_pClientDE->GetStringData(pClient->hstrName));
	}


	// All done...

	return(sBuf);
}

//*********************************************************************************

void CFragInfo::UpdatePlayerPing(DDWORD dwPlayerID, int nPing)
{
	CLIENT_INFO* pClient = GetClientInfo(dwPlayerID);
	if (!pClient) return;

	if (nPing == 0) nPing = 1;

	pClient->nPing = nPing;
}

//*********************************************************************************

void CFragInfo::DrawClip(char* sText, int nMax)
{
	// Sanity checks...

	if (!sText) return;
	if (nMax <= 0) return;


	// Check if it's ok to write the entire string...

	DIntPt pt =	m_pFont->GetTextExtents(sText);

	if (pt.x < nMax)
	{
		m_pCursor->Draw(sText);
	}


	// Draw the biggest sub-string that will fit...

	int nLen = strlen(sText);

	while (nLen > 1)
	{
		nLen--;
		sText[nLen] = '\0';

		DIntPt pt =	m_pFont->GetTextExtents(sText);

		if (pt.x < nMax)
		{
			m_pCursor->Draw(sText);
			return;
		}
	}
}




