// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTeam.h
//
// PURPOSE : Interface screen for Team setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenTeam.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

extern uint8 g_nCurTeam;

namespace
{
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenTeam *pThisScreen = (CScreenTeam *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_TEAM);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};

	#define INVALID_ANI			((HMODELANIM)-1)
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenTeam::CScreenTeam()
{
	m_pName			= LTNULL;
	m_pModel		= LTNULL;
	m_pLeft			= LTNULL;
	m_pRight		= LTNULL;
	m_nCurrentModel = 0;

}

CScreenTeam::~CScreenTeam()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenTeam::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenTeam::Term()
{
	CBaseScreen::Term();
}

// Build the screen
LTBOOL CScreenTeam::Build()
{

	CreateTitle(IDS_TITLE_TEAM);
	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_TEAM,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;
	int kArrow = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_TEAM,"ArrowWidth");

	m_pName = AddColumnCtrl(CMD_EDIT_NAME, IDS_HELP_TEAM_NAME);
	m_pName->AddColumn(LoadTempString(IDS_TEAM_NAME), kColumn0);
	m_pName->AddColumn("<Team name>", kColumn1, LTTRUE);

	m_pModel = AddTextItem(IDS_TEAM_MODEL, LTNULL, IDS_HELP_TEAM_MODEL);

	LTIntPt arrowPos = m_pModel->GetBasePos();
	arrowPos.x += kColumn0;

	HTEXTURE hLeft = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt.dtx");
	HTEXTURE hLeftH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowlt_h.dtx");

	m_pLeft = debug_new(CLTGUIButton);
	if (m_pLeft)
	{
		m_pLeft->Create(CMD_LEFT,LTNULL,hLeft,hLeftH);
		m_pLeft->SetBasePos(arrowPos);
		AddControl(m_pLeft);
		m_pLeft->SetCommandHandler(this);
	}
	arrowPos.x += kArrow;

	HTEXTURE hRight = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt.dtx");
	HTEXTURE hRightH = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\arrowrt_h.dtx");
	m_pRight = debug_new(CLTGUIButton);
	if (m_pRight)
	{
		m_pRight->Create(CMD_RIGHT,LTNULL,hRight,hRightH);
		m_pRight->SetBasePos(arrowPos);
		AddControl(m_pRight);
		m_pRight->SetCommandHandler(this);
	}


 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenTeam::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.eInput = CLTGUIEditCtrl::kInputSprintfFriendly;
			mb.pString = m_sTeamName.c_str();
			mb.nMaxChars = MAX_PLAYER_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_TEAM_NAME,&mb);
		} break;

	case CMD_LEFT:
		{
			PrevModel();
		} break;
	case CMD_RIGHT:
		{
			NextModel();
		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenTeam::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	LTBOOL bInMPGame = (IsMultiplayerGame() && g_pGameClientShell->IsWorldLoaded());		
	if (bFocus)
	{
		int n = 0;
		for (n = 0; n < g_pModelButeMgr->GetNumTeamModels(); n++)
		{
			ModelId id = g_pModelButeMgr->GetTeamModel(n);
			HMODELDB dummy = NULL;
			g_pILTModelClient->CacheModelDB(g_pModelButeMgr->GetModelFilename(id),dummy);
		}

		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeTeamDeathmatch:
			m_nCurrentModel = pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nTeamModel[g_nCurTeam];
			m_nSkipModel = pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nTeamModel[1-g_nCurTeam];
			m_sTeamName = pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sTeamName[g_nCurTeam].c_str();
			break;
		case eGameTypeDoomsDay:
			m_nCurrentModel = pProfile->m_ServerGameOptions.GetDoomsday().m_nTeamModel[g_nCurTeam];
			m_nSkipModel = pProfile->m_ServerGameOptions.GetDoomsday().m_nTeamModel[1-g_nCurTeam];
			m_sTeamName = pProfile->m_ServerGameOptions.GetDoomsday().m_sTeamName[g_nCurTeam].c_str();
			break;
		};
		
		m_pName->SetString(1,m_sTeamName.c_str());

		m_pName->Enable(!bInMPGame);
		m_pModel->Enable(!bInMPGame);
		m_pLeft->Enable(!bInMPGame);
		m_pRight->Enable(!bInMPGame);


        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		int n = 0;
		for (n = 0; n < g_pModelButeMgr->GetNumTeamModels(); n++)
		{
			ModelId id = g_pModelButeMgr->GetTeamModel(n);
			g_pILTModelClient->UncacheModelDB(g_pModelButeMgr->GetModelFilename(id));
		}

		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeTeamDeathmatch:
			pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nTeamModel[g_nCurTeam] = m_nCurrentModel;
			pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sTeamName[g_nCurTeam] = m_sTeamName;
			break;
		case eGameTypeDoomsDay:
			pProfile->m_ServerGameOptions.GetDoomsday().m_nTeamModel[g_nCurTeam] = m_nCurrentModel;
			pProfile->m_ServerGameOptions.GetDoomsday().m_sTeamName[g_nCurTeam] = m_sTeamName;
			break;
		};

		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		UpdateChar();
}

LTBOOL	CScreenTeam::OnUp()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnUp();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	return bHandled;
}

LTBOOL	CScreenTeam::OnDown()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnDown();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	return bHandled;
}


LTBOOL	CScreenTeam::OnLeft()
{
	if (m_pModel->IsSelected())
	{
		PrevModel();
		return LTTRUE;
	}

	return CBaseScreen::OnLeft();

}


LTBOOL	CScreenTeam::OnRight()
{
	if (m_pModel->IsSelected())
	{
		NextModel();
		return LTTRUE;
	}
	return CBaseScreen::OnRight();

}

/******************************************************************/

/******************************************************************/
LTBOOL CScreenTeam::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
LTBOOL CScreenTeam::OnRButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	return CBaseScreen::OnRButtonUp(x,y);
}

LTBOOL CScreenTeam::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseScreen::OnMouseMove(x, y);
	if (GetSelectedControl() == m_pLeft || GetSelectedControl() == m_pRight)
		m_pModel->Select(LTTRUE);
	else if (GetSelectedControl() != m_pModel)
		m_pModel->Select(LTFALSE);
	return bHandled;
}

void CScreenTeam::NextModel()
{
	m_nCurrentModel++;
	if (m_nCurrentModel >= g_pModelButeMgr->GetNumTeamModels())
		m_nCurrentModel = 0;

	if (m_nCurrentModel == m_nSkipModel)
		NextModel();
	else
		UpdateChar();
}

void CScreenTeam::PrevModel()
{
	if (!m_nCurrentModel)
		m_nCurrentModel = g_pModelButeMgr->GetNumTeamModels();
	m_nCurrentModel--;
		
	if (m_nCurrentModel == m_nSkipModel)
		PrevModel();
	else
		UpdateChar();
}

void CScreenTeam::UpdateChar()
{

	HOBJECT hChar = m_CharSFX.GetObject();
	if (hChar)
	{
		if (m_nCurrentModel >= g_pModelButeMgr->GetNumTeamModels())
		{
			m_nCurrentModel = 0;
		}
		ModelId id = g_pModelButeMgr->GetTeamModel(m_nCurrentModel);

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		SAFE_STRCPY(createStruct.m_Filename, g_pModelButeMgr->GetModelFilename(id));
		if(g_pModelButeMgr->GetSkinReader(id))
		{
			g_pModelButeMgr->GetSkinReader(id)->CopyList(0, createStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		}
		
		g_pModelButeMgr->CopyRenderStyleFilenames( id, &createStruct );

		g_pLTClient->SetModelAnimation(hChar, 0);
		g_pCommonLT->SetObjectFilenames(hChar, &createStruct);

		uint32 dwAni = g_pLTClient->GetAnimIndex(hChar, "Interface");
		if (dwAni != INVALID_ANI)
		{
			g_pLTClient->SetModelAnimation(hChar, dwAni);
		}

		// Remove old attachments...
		
		ClearAttachFX();
	
		// Create the required attachments for this model...

		INT_CHAR *pChar = g_pLayoutMgr->GetScreenCharacter((eScreenID)m_nScreenID);
		uint8 nDefaultAttachments = g_pModelButeMgr->GetNumDefaultAttachments( id );

		const char *pszAttachmentPos;
		const char *pszAttachment;
		
		for( uint8 i = 0; i < nDefaultAttachments; ++i )
		{
			INT_ATTACH acs;
			
			g_pModelButeMgr->GetDefaultAttachment( id, i, pszAttachmentPos, pszAttachment );

			acs.nAttachmentID	= g_pAttachButeMgr->GetAttachmentIDByName( pszAttachment );
			acs.fScale			= pChar->fScale;

			LTStrCpy( acs.szSocket, pszAttachmentPos, ARRAY_LEN( acs.szSocket ));
						
			CreateAttachFX( &acs );
		}
	}
}


void CScreenTeam::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_NAME:
		{
			char* pName = ((char *)dwParam1);
			if (pName && strlen(pName))
			{
				m_sTeamName = pName;
				m_pName->SetString(1,m_sTeamName.c_str());
			}
		} break;
	}
	UpdateData();
}

