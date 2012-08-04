// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerTeam.cpp
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayerTeam.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int CMD_TEAM1 = (CMD_CUSTOM+1);
	const int CMD_TEAM2 = (CMD_CUSTOM+2);
	#define INVALID_ANI			((HMODELANIM)-1)

	uint32 nTeamColors[2] = {argbBlack,argbBlack};
	uint8 nListFontSize = 14;
	uint16 nListWidth = 100;

	CTimer			UpdateTimer;
	CTimer			AutoSelectTimer;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayerTeam::CScreenPlayerTeam()
{
	memset(m_pTeams,0,sizeof(m_pTeams));
	memset(m_pPlayers,0,sizeof(m_pPlayers));
	memset(m_pFrame,0,sizeof(m_pFrame));
}

CScreenPlayerTeam::~CScreenPlayerTeam()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayerTeam::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayerTeam::Term()
{
	CBaseScreen::Term();
}

// Build the screen
LTBOOL CScreenPlayerTeam::Build()
{

	CreateTitle(IDS_TITLE_TEAM);

	nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");
	nListWidth = (uint16)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListWidth");

	LTRect rTeam[2];
	char szAtt[128];

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		sprintf(szAtt,"Team%dRect",nTeam);
		rTeam[nTeam] = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_PLAYER_TEAM,szAtt);

		sprintf(szAtt,"Team%dColor",(nTeam+1));
		LTVector vCol = g_pLayoutMgr->GetVector("Scores",szAtt);
		uint8 nR = (uint8)vCol.x;
		uint8 nG = (uint8)vCol.y;
		uint8 nB = (uint8)vCol.z;
		nTeamColors[nTeam]=  SET_ARGB(0xB0,nR,nG,nB);

		uint16 nWidth = rTeam[nTeam].right - rTeam[nTeam].left;
		uint16 nHeight = rTeam[nTeam].bottom - rTeam[nTeam].top;

		LTIntPt pos(rTeam[nTeam].left,rTeam[nTeam].top);
		m_pFrame[nTeam] = debug_new(CLTGUIFrame);
		m_pFrame[nTeam]->Create((uint32)0,nWidth,nHeight);
		m_pFrame[nTeam]->SetBasePos(pos);
		m_pFrame[nTeam]->SetBorder(2,argbBlack);
		AddControl(m_pFrame[nTeam]);


	};

	LTIntPt pos(rTeam[0].left+4,rTeam[0].top+2);
	m_pTeams[0] = AddTextItem( "<team 0>", CMD_TEAM1, IDS_HELP_CHOOSE_TEAM_1, pos);
	pos.y += 22;
	
	uint16 nHeight = (rTeam[0].bottom-pos.y) - 8;
	m_pPlayers[0] = AddList(pos,nHeight, LTTRUE, nListWidth);
	if (m_pPlayers[0])
	{
		m_pPlayers[0]->SetIndent(LTIntPt(4,4));
		m_pPlayers[0]->SetFrameWidth(2);
//		m_pPlayers[0]->SetColors(nTeamColors[0],nTeamColors[0],nTeamColors[0]);
		m_pPlayers[0]->Enable(LTFALSE);
	}

	pos= LTIntPt(rTeam[1].left+4,rTeam[1].top+2);
	m_pTeams[1] = AddTextItem( "<team 0>", CMD_TEAM2, IDS_HELP_CHOOSE_TEAM_2, pos);
	pos.y += 22;
	
	nHeight = (rTeam[1].bottom - pos.y) - 8;
	m_pPlayers[1] = AddList(pos,nHeight, LTTRUE, nListWidth);
	if (m_pPlayers[1])
	{
		m_pPlayers[1]->SetIndent(LTIntPt(4,4));
		m_pPlayers[1]->SetFrameWidth(2);
//		m_pPlayers[1]->SetColors(nTeamColors[1],nTeamColors[1],nTeamColors[1]);
		m_pPlayers[1]->Enable(LTFALSE);
	}

	pos.x = GetPageLeft();
	pos.y = GetPageBottom()-20;
	m_pAuto = AddTextItem(IDS_AUTO_SELECT,CMD_LAUNCH,IDS_HELP_AUTO_SELECT,pos);

 	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;
	UseBack(LTFALSE);
	return LTTRUE;

}

uint32 CScreenPlayerTeam::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_TEAM1:
		m_nTeam = 0;
		Escape();
		break;
	case CMD_TEAM2:
		m_nTeam = 1;
		Escape();
		break;
	case CMD_LAUNCH:
		m_nTeam = INVALID_TEAM;
		Escape();
		break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenPlayerTeam::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_nTeam = INVALID_TEAM;

		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( pLocalCI )
			SelectTeam(pLocalCI->nTeamID);

		m_pAuto->Show(!g_pClientMultiplayerMgr->HasSelectedTeam());

		UpdateTeam();

		AutoSelectTimer.Start(30.0f);

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenPlayerTeam::Escape()
{

	if (!g_pClientMultiplayerMgr->HasSelectedTeam())
	{
		g_pClientMultiplayerMgr->SelectTeam(m_nTeam,true);

		g_pClientMultiplayerMgr->UpdateMultiPlayer();

		// Tell the client we're ready to play.
		g_pGameClientShell->SendClientLoadedMessage( );

		

		g_pInterfaceMgr->ChangeState(GS_PLAYING);
	}
	else
	{
		g_pClientMultiplayerMgr->SelectTeam(m_nTeam,true);
		g_pClientMultiplayerMgr->UpdateMultiPlayer();

		CBaseScreen::Escape();
	}
}



bool CScreenPlayerTeam::UpdateInterfaceSFX()
{
	if (AutoSelectTimer.Stopped())
	{
		Escape();
		return true;
	}

	if (UpdateTimer.Stopped())
		UpdateTeam();

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{

		for (int i = 0; i < m_nNumTeamAttachments[nTeam]; i++)
		{
			CBaseScaleFX *pSFX = &m_aTeamAttachment[nTeam][i].sfx;
			
			HMODELSOCKET hSocket = m_aTeamAttachment[nTeam][i].socket;
			LTransform transform;
			if (g_pModelLT->GetSocketTransform(m_TeamSFX[nTeam].GetObject(), hSocket, transform, LTTRUE) == LT_OK)
			{
				g_pLTClient->SetObjectPosAndRotation(pSFX->GetObject(), &transform.m_Pos, &transform.m_Rot);

			}
		}
	}


	return CBaseScreen::UpdateInterfaceSFX();
}



void CScreenPlayerTeam::SelectTeam(uint8 nTeam)
{
	if (nTeam >= 2)
	{
		m_pFrame[0]->SetBorder(2,argbBlack);
		m_pFrame[1]->SetBorder(2,argbBlack);
		return;
	}

	m_pFrame[1-nTeam]->SetBorder(2,argbBlack);
	m_pFrame[nTeam]->SetBorder(4,nTeamColors[nTeam]);

	SetSelection(GetIndex(m_pTeams[nTeam]));
}

void CScreenPlayerTeam::UpdateTeam()
{
	char szTemp[128];
	uint8 nPlayers[2] = {0,0};

	UpdateTimer.Start(3.0f);

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO* pCI = pCIMgr->GetFirstClient();
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);

	CLTGUITextCtrl* pItem;

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		m_pPlayers[nTeam]->RemoveAll();
	}
	
	while (pCI)
	{
		if (pCI->nTeamID != INVALID_TEAM)
		{
			LTStrCpy(szTemp,pCI->sName.c_str(),sizeof(szTemp));

			uint32 nColor = nTeamColors[pCI->nTeamID];

			pItem = CreateTextItem(szTemp,NULL,NULL,kDefaultPos,LTTRUE);
			pItem->SetColors(nColor,nColor,nColor);
			pItem->SetFont(LTNULL, nListFontSize);
			pItem->SetFixedWidth(nListWidth,LTTRUE);
			m_pPlayers[pCI->nTeamID]->AddControl(pItem);
			nPlayers[pCI->nTeamID]++;
		}
		
		pCI = pCI->pNext;
	}

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeam);
		if (!pTeam) continue;

		sprintf(szTemp,"%s - %s %d", pTeam->GetName(), LoadTempString(IDS_SCORE_PLAYERS), nPlayers[nTeam]);
		m_pTeams[nTeam]->SetString(szTemp);
	};



}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayerTeam::CreateInterfaceSFX
//
//	PURPOSE:	Create the SFX to render on this screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayerTeam::CreateInterfaceSFX()
{
	CBaseScreen::CreateInterfaceSFX();

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		CreateTeamFX(nTeam);
	}
}


void CScreenPlayerTeam::CreateTeamFX(uint8 nTeam)
{
	char szName[128];
	sprintf(szName,"Team%dModel",nTeam);
	INT_CHAR *pChar = g_pLayoutMgr->GetScreenCustomCharacter(SCREEN_ID_PLAYER_TEAM,szName);

	CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeam);
	ModelId id = g_pModelButeMgr->GetTeamModel(pTeam->GetModel());
	if (pChar)
	{
		

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = s_rRot;

		VEC_COPY(vPos,s_vPos);
		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pChar->fScale);

		LTVector vModPos = pChar->vPos;
	    LTFLOAT fRot = pChar->fRot;
		fRot  = MATH_PI + DEG2RAD(fRot);
		rRot.Rotate(s_vU, fRot);

		VEC_MULSCALAR(vTemp, s_vF, vModPos.z);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, s_vR, vModPos.x);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_MULSCALAR(vTemp, s_vU, vModPos.y);
		VEC_ADD(vPos, vPos, vTemp);

		VEC_COPY(bcs.vPos, vPos);
		bcs.rRot = rRot;
		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = g_pModelButeMgr->GetModelFilename(id);
		bcs.pSkinReader = g_pModelButeMgr->GetSkinReader(id);
		bcs.pRenderStyleReader = g_pModelButeMgr->GetRenderStyleReader(id);
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

		bcs.nType = OT_MODEL;
		bcs.fInitialAlpha = 0.99f;
		bcs.fFinalAlpha = 0.99f;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;

		bcs.fMinRotateVel = 1.0f;
		bcs.fMaxRotateVel = 1.0f;

		bcs.nMenuLayer = pChar->nMenuLayer;


		if (m_TeamSFX[nTeam].Init(&bcs))
		{
			m_TeamSFX[nTeam].CreateObject(g_pLTClient);
			if (m_TeamSFX[nTeam].GetObject())
			{
				g_pInterfaceMgr->AddInterfaceSFX(&m_TeamSFX[nTeam], IFX_NORMAL);

				int reqID[MAX_INT_ATTACHMENTS];
				int numReq = g_pAttachButeMgr->GetRequirementIDs(bcs.pFilename,reqID,MAX_INT_ATTACHMENTS);
				int i;
				for (i = 0; i < numReq; i++)
				{
					INT_ATTACH acs;
					acs.fScale = pChar->fScale;
					acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
					g_pAttachButeMgr->GetRequirementSocket(reqID[i],acs.szSocket,sizeof(acs.szSocket));

					CreateTeamAttachFX(&acs,nTeam);
				}

			}
		}

	}
}

void CScreenPlayerTeam::CreateTeamAttachFX(INT_ATTACH *pAttach, uint8 nTeam)
{
	if (m_nNumTeamAttachments[nTeam] < MAX_INT_ATTACHMENTS)
	{
		int nCount = m_nNumTeamAttachments[nTeam];

		BSCREATESTRUCT bcs;
	    LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);
	    LTRotation rRot = s_rRot;

		char szModel[128];

		g_pAttachButeMgr->GetAttachmentModel(pAttach->nAttachmentID,szModel,sizeof(szModel));

		VEC_SET(vScale,1.0f,1.0f,1.0f);
		VEC_MULSCALAR(vScale, vScale, pAttach->fScale);

		VEC_COPY(bcs.vInitialScale, vScale);
		VEC_COPY(bcs.vFinalScale, vScale);
		VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
		VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
		bcs.bUseUserColors = LTTRUE;

		bcs.pFilename = szModel;
		bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

		CButeListReader blrSkinReader;
		g_pAttachButeMgr->GetAttachmentSkins(pAttach->nAttachmentID, &blrSkinReader);
		bcs.pSkinReader = &blrSkinReader;

		CButeListReader blrRenderStyleReader;
		g_pAttachButeMgr->GetAttachmentRenderStyles(pAttach->nAttachmentID, &blrRenderStyleReader);
		bcs.pRenderStyleReader = &blrRenderStyleReader;

		bcs.fInitialAlpha = 1.0f;
		bcs.fFinalAlpha = 1.0f;
		bcs.nType = OT_MODEL;
		bcs.fLifeTime = 1000000.0f;
		bcs.bLoop = LTTRUE;
		bcs.nMenuLayer = m_TeamSFX[nTeam].GetMenuLayer();

		CBaseScaleFX *pSFX = &m_aTeamAttachment[nTeam][nCount].sfx;

		if (!pSFX->Init(&bcs)) return;
		
		pSFX->CreateObject(g_pLTClient);
		if (!pSFX->GetObject()) return;

		HOBJECT hChar = m_TeamSFX[nTeam].GetObject();
		if (!hChar) return;
		if (g_pModelLT->GetSocket(hChar, pAttach->szSocket, m_aTeamAttachment[nTeam][nCount].socket) != LT_OK)
			return;

		g_pInterfaceMgr->AddInterfaceSFX(pSFX, IFX_ATTACH);
		m_nNumTeamAttachments[nTeam]++;
	}
}


void CScreenPlayerTeam::RemoveInterfaceSFX()
{

	for (uint8 nTeam = 0; nTeam < 2; nTeam++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_TeamSFX[nTeam]);
		
		m_TeamSFX[nTeam].Reset();
		m_TeamSFX[nTeam].Term();

		ClearTeamAttachFX(nTeam);
	}

	CBaseScreen::RemoveInterfaceSFX();
}

void CScreenPlayerTeam::ClearTeamAttachFX(uint8 nTeam)
{
	for (int i = 0; i < MAX_INT_ATTACHMENTS; i++)
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(&m_aTeamAttachment[nTeam][i].sfx);
		m_aTeamAttachment[nTeam][i].sfx.Reset();
		m_aTeamAttachment[nTeam][i].sfx.Term();
		m_aTeamAttachment[nTeam][i].socket = INVALID_MODEL_SOCKET;
	}
	m_nNumTeamAttachments[nTeam] = 0;
}



LTBOOL CScreenPlayerTeam::OnMouseMove(int x, int y)
{
	if (m_pFrame[0]->IsOnMe(x,y))
	{
		if (GetSelectedControl() != m_pTeams[0])
		{
			SelectTeam(0);
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
			return LTTRUE;
		}
	} 
	else if (m_pFrame[1]->IsOnMe(x,y))
	{
		if (GetSelectedControl() != m_pTeams[1])
		{
			SelectTeam(1);
			g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
			UpdateHelpText();
			return LTTRUE;
		}
	}
	else
	{
		SelectTeam(-1);
		return CBaseScreen::OnMouseMove(x,y);
	}

	return LTFALSE;

}

LTBOOL CScreenPlayerTeam::OnUp()
{
	LTBOOL bHandled = CBaseScreen::OnUp();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pTeams[0])
			SelectTeam(0);
		else  if (GetSelectedControl() == m_pTeams[1])
			SelectTeam(1);
		else
			SelectTeam(-1);
	}
	return bHandled;
}

LTBOOL CScreenPlayerTeam::OnDown()
{
	LTBOOL bHandled = CBaseScreen::OnDown();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pTeams[0])
			SelectTeam(0);
		else  if (GetSelectedControl() == m_pTeams[1])
			SelectTeam(1);
		else
			SelectTeam(-1);
	}
	return bHandled;
}


/******************************************************************/
LTBOOL CScreenPlayerTeam::OnLButtonDown(int x, int y)
{
	if (m_pFrame[0]->IsOnMe(x,y))
	{
		SelectTeam(0);
		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.

		m_nLMouseDownItemSel=GetIndex(m_pTeams[0]);

		return m_pTeams[0]->OnLButtonDown(x,y);
	} 
	else if (m_pFrame[1]->IsOnMe(x,y))
	{
		SelectTeam(1);
		// Record this control as the one being selected from the mouse click.
		// If the mouse is still over it on the UP message, then the "enter" message will be sent.

		m_nLMouseDownItemSel=GetIndex(m_pTeams[1]);

		return m_pTeams[1]->OnLButtonDown(x,y);
	}

	SelectTeam(-1);
	return CBaseScreen::OnLButtonDown( x, y);
}

/******************************************************************/
LTBOOL CScreenPlayerTeam::OnLButtonUp(int x, int y)
{
	if (m_pFrame[0]->IsOnMe(x,y) && m_nLMouseDownItemSel == GetIndex(m_pTeams[0]))
	{
		LTBOOL bHandled = m_pTeams[0]->OnLButtonUp(x,y);
		if (bHandled)
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		return bHandled;
	} 
	else if (m_pFrame[1]->IsOnMe(x,y) && m_nLMouseDownItemSel == GetIndex(m_pTeams[1]))
	{
		LTBOOL bHandled = m_pTeams[1]->OnLButtonUp(x,y);
		if (bHandled)
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		return bHandled;
	}
	return CBaseScreen::OnLButtonUp( x, y);
}

