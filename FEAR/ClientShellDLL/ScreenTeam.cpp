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
#include "ClientConnectionMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

uint8 g_nCurTeam;

namespace
{
	void EditNameCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenTeam *pThisScreen = (CScreenTeam *)pUserData;
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
	m_pName			= NULL;
//	m_pModel		= NULL;
//	m_pLeft			= NULL;
//	m_pRight		= NULL;
//	m_nCurrentModel = 0;

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
bool CScreenTeam::Build()
{

	CreateTitle("IDS_TITLE_TEAM");
	int kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	int kColumn1 = m_ScreenRect.GetWidth() - kColumn0;
//	int kArrow = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	cs.nCommandID = CMD_EDIT_NAME;
	cs.szHelpID = "IDS_HELP_TEAM_NAME";
	cs.pCommandHandler = this;
	m_pName = AddColumnCtrl(cs );
	m_pName->AddColumn(LoadString("IDS_TEAM_NAME"), kColumn0);
	m_pName->AddColumn(L"<Team name>", kColumn1, true);
/*
	m_pModel = AddTextItem(IDS_TEAM_MODEL, NULL, IDS_HELP_TEAM_MODEL);

	LTIntPt arrowPos = m_pModel->GetBasePos();
	arrowPos.x += kColumn0;

	TextureReference hLeft("interface\\menu\\sprtex\\arrowlt.dds");
	TextureReference hLeftH("interface\\menu\\sprtex\\arrowlt_h.dds");

	m_pLeft = debug_new(CLTGUIButton);
	if (m_pLeft)
	{
		m_pLeft->Create(CMD_LEFT,NULL,16,16,hLeft,hLeftH);
		m_pLeft->SetBasePos(arrowPos);
		AddControl(m_pLeft);
		m_pLeft->SetCommandHandler(this);
	}
	arrowPos.x += kArrow;

	TextureReference hRight("interface\\menu\\sprtex\\arrowrt.dds");
	TextureReference hRightH("interface\\menu\\sprtex\\arrowrt_h.dds");
	m_pRight = debug_new(CLTGUIButton);
	if (m_pRight)
	{
		m_pRight->Create(CMD_RIGHT,NULL,16,16,hRight,hRightH);
		m_pRight->SetBasePos(arrowPos);
		AddControl(m_pRight);
		m_pRight->SetCommandHandler(this);
	}
*/

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
	/*
	switch(dwCommand)
	{
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
	*/

	return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
};


// Change in focus
void CScreenTeam::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	bool bInMPGame = (IsMultiplayerGameClient() && g_pGameClientShell->IsWorldLoaded());		
	if (bFocus)
	{
        UpdateData(false);
	}
	else
	{
		UpdateData();

		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
/*
	if (bFocus)
		UpdateChar();
*/
}
/*
bool	CScreenTeam::OnUp()
{
	m_pLeft->Enable(false);
	m_pRight->Enable(false);
	bool bHandled = CBaseScreen::OnUp();
	m_pLeft->Enable(true);
	m_pRight->Enable(true);
	return bHandled;
}

bool	CScreenTeam::OnDown()
{
	m_pLeft->Enable(false);
	m_pRight->Enable(false);
	bool bHandled = CBaseScreen::OnDown();
	m_pLeft->Enable(true);
	m_pRight->Enable(true);
	return bHandled;
}


bool	CScreenTeam::OnLeft()
{
	if (m_pModel->IsSelected())
	{
		PrevModel();
		return true;
	}

	return CBaseScreen::OnLeft();

}


bool	CScreenTeam::OnRight()
{
	if (m_pModel->IsSelected())
	{
		NextModel();
		return true;
	}
	return CBaseScreen::OnRight();

}
*/

/*
bool CScreenTeam::OnMouseMove(int x, int y)
{
	bool bHandled = CBaseScreen::OnMouseMove(x, y);
	if (GetSelectedControl() == m_pLeft || GetSelectedControl() == m_pRight)
		m_pModel->Select(true);
	else if (GetSelectedControl() != m_pModel)
		m_pModel->Select(false);
	return bHandled;
}

void CScreenTeam::NextModel()
{
	m_nCurrentModel++;
	if (m_nCurrentModel >= g_pModelsDB->GetNumTeamModels())
		m_nCurrentModel = 0;

	if (m_nCurrentModel == m_nSkipModel)
		NextModel();
	else
		UpdateChar();
}

void CScreenTeam::PrevModel()
{
	if (!m_nCurrentModel)
		m_nCurrentModel = g_pModelsDB->GetNumTeamModels();
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
		if (m_nCurrentModel >= g_pModelsDB->GetNumTeamModels())
		{
			m_nCurrentModel = 0;
		}
		ModelsDB::HMODEL hModel = g_pModelsDB->GetTeamModel(m_nCurrentModel);

		ObjectCreateStruct createStruct;

		createStruct.SetFileName( g_pModelsDB->GetModelFilename(hModel));
		if(g_pModelsDB->GetMaterialReader(hModel))
		{
			g_pModelsDB->GetMaterialReader(hModel)->CopyList(0, createStruct.m_Materials[0], MAX_CS_FILENAME_LEN+1);
		}
		
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
		uint8 nDefaultAttachments = g_pModelsDB->GetNumDefaultAttachments( hModel );

		const char *pszAttachment;
		
		for( uint8 i = 0; i < nDefaultAttachments; ++i )
		{
			INT_ATTACH acs;
			
			pszAttachment		= g_pModelsDB->GetDefaultAttachment( hModel, i );

			acs.nAttachmentID	= AttachmentDB::Instance().GetAttachmentIDByName( pszAttachment );
			acs.fScale			= pChar->fScale;

			CreateAttachFX( &acs );
		}
	}
}
*/

void CScreenTeam::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	UpdateData();
}

