// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayer.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientMultiplayerMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

static const int kMaxBandwidthStrLen = 8;

extern bool g_bLAN;

namespace
{
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};
	void EditBandwidthCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	};

	#define INVALID_ANI			((HMODELANIM)-1)
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayer::CScreenPlayer()
{
	m_pName			= LTNULL;
	m_pModel		= LTNULL;
	m_pLeft			= LTNULL;
	m_pRight		= LTNULL;
	m_nCurrentModel = 0;

	m_pBandwidthCycle	= LTNULL;
	m_pBandwidth		= LTNULL;
	m_nBandwidth		= 0;

}

CScreenPlayer::~CScreenPlayer()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayer::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayer::Term()
{
	CBaseScreen::Term();
}

// Build the screen
LTBOOL CScreenPlayer::Build()
{

	CreateTitle(IDS_TITLE_PLAYER);
	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;
	int kArrow = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"ArrowWidth");

	m_pName = AddColumnCtrl(CMD_EDIT_NAME, IDS_HELP_PLAYER_NAME);
	m_pName->AddColumn(LoadTempString(IDS_PLAYER_NAME), kColumn0);
	m_pName->AddColumn("<player name>", kColumn1, LTTRUE);

	m_pModel = AddTextItem(IDS_PLAYER_MODEL, LTNULL, IDS_HELP_PLAYER_MODEL);

	m_pSkills = AddTextItem(IDS_PLAYER_SKILLS, CMD_SKILLS, IDS_HELP_PLAYER_SKILLS);

	m_pBandwidthCycle = AddCycle(IDS_BANDWIDTH_CYCLE,IDS_HELP_BANDWIDTH_CYCLE,kColumn0,&m_nBandwidth);
	m_pBandwidthCycle->AddString(LoadTempString(IDS_56K));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_DSL_LOW));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_DSL_HIGH));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CABLE));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T1));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T3));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CUSTOM));

	m_pBandwidth = AddColumnCtrl(CMD_EDIT_BANDWIDTH, IDS_HELP_BANDWIDTH_EDIT);
	m_pBandwidth->AddColumn(LoadTempString(IDS_BANDWIDTH_EDIT), kColumn0);
	m_pBandwidth->AddColumn("<bandwidth>", kColumn1, LTTRUE);

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

uint32 CScreenPlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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
			mb.pString = m_sPlayerName.c_str();
			mb.nMaxChars = MAX_PLAYER_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_PLAYER_NAME,&mb);
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pString = m_sBandwidth;
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_BANDWIDTH_EDIT,&mb);
		} break;
	case CMD_SKILLS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PLAYER_SKILLS);
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
void CScreenPlayer::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	LTBOOL bInMPGame = (IsMultiplayerGame() && g_pGameClientShell->IsWorldLoaded());		
	if (bFocus)
	{
		if (g_bLAN)
		{
			m_pBandwidthCycle->Show(LTFALSE);
			m_pBandwidth->Show(LTFALSE);
		}
		else
		{
			m_pBandwidthCycle->Show(LTTRUE);
			m_pBandwidth->Show(LTTRUE);
		}

		int n = 0;
		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeCooperative:
			for (n = 0; n < g_pModelButeMgr->GetNumCPModels(); n++)
			{
				ModelId id = g_pModelButeMgr->GetCPModel(n);
				HMODELDB dummy = NULL;
				g_pILTModelClient->CacheModelDB(g_pModelButeMgr->GetModelFilename(id),dummy);
			}
			m_nCurrentModel = pProfile->m_nCPPlayerModel;
			m_pLeft->Show(LTTRUE);
			m_pRight->Show(LTTRUE);
			m_pModel->Show(LTTRUE);
			break;
		case eGameTypeDeathmatch:
			for (n = 0; n < g_pModelButeMgr->GetNumDMModels(); n++)
			{
				ModelId id = g_pModelButeMgr->GetDMModel(n);
				HMODELDB dummy = NULL;
				g_pILTModelClient->CacheModelDB(g_pModelButeMgr->GetModelFilename(id),dummy);
			}
			m_nCurrentModel = pProfile->m_nDMPlayerModel;
			m_pLeft->Show(LTTRUE);
			m_pRight->Show(LTTRUE);
			m_pModel->Show(LTTRUE);
			break;
		case eGameTypeTeamDeathmatch:
		case eGameTypeDoomsDay:
			m_pLeft->Show(LTFALSE);
			m_pRight->Show(LTFALSE);
			m_pModel->Show(LTFALSE);
			break;
		};

		m_sPlayerName = pProfile->m_sPlayerName;
		

		m_pName->SetString(1,m_sPlayerName.c_str());

		m_nBandwidth = pProfile->m_nBandwidthClient;
		UpdateBandwidth();

		
		m_pSkills->Enable(!bInMPGame && (g_pGameClientShell->GetGameType() == eGameTypeCooperative));
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		pProfile->m_sPlayerName = m_sPlayerName;
		int n = 0;
		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeCooperative:
			for (n = 0; n < g_pModelButeMgr->GetNumCPModels(); n++)
			{
				ModelId id = g_pModelButeMgr->GetCPModel(n);
				g_pILTModelClient->UncacheModelDB(g_pModelButeMgr->GetModelFilename(id));
			}
			pProfile->m_nCPPlayerModel = m_nCurrentModel;
			break;
		case eGameTypeDeathmatch:
			for (n = 0; n < g_pModelButeMgr->GetNumDMModels(); n++)
			{
				ModelId id = g_pModelButeMgr->GetDMModel(n);
				g_pILTModelClient->UncacheModelDB(g_pModelButeMgr->GetModelFilename(id));
			}
			pProfile->m_nDMPlayerModel = m_nCurrentModel;
			break;
		};

		pProfile->m_nBandwidthClient = m_nBandwidth;
		pProfile->m_nBandwidthClientCustom = (uint16)atol(m_sBandwidth);

		pProfile->Save();
		pProfile->ApplyMultiplayer(g_bLAN);


		if (bInMPGame)
			g_pClientMultiplayerMgr->UpdateMultiPlayer();

	}
	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
		UpdateChar();
}

LTBOOL	CScreenPlayer::OnUp()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnUp();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	return bHandled;
}

LTBOOL	CScreenPlayer::OnDown()
{
	m_pLeft->Enable(LTFALSE);
	m_pRight->Enable(LTFALSE);
	LTBOOL bHandled = CBaseScreen::OnDown();
	m_pLeft->Enable(LTTRUE);
	m_pRight->Enable(LTTRUE);
	return bHandled;
}


LTBOOL	CScreenPlayer::OnLeft()
{
	if (m_pModel->IsSelected())
	{
		PrevModel();
		return LTTRUE;
	}
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}

	return CBaseScreen::OnLeft();

}


LTBOOL	CScreenPlayer::OnRight()
{
	if (m_pModel->IsSelected())
	{
		NextModel();
		return LTTRUE;
	}
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}

	return CBaseScreen::OnRight();

}

/******************************************************************/

/******************************************************************/
LTBOOL CScreenPlayer::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
LTBOOL CScreenPlayer::OnRButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnRButtonUp(x,y);
}
LTBOOL CScreenPlayer::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseScreen::OnMouseMove(x, y);
	if (GetSelectedControl() == m_pLeft || GetSelectedControl() == m_pRight)
		m_pModel->Select(LTTRUE);
	else if (GetSelectedControl() != m_pModel)
		m_pModel->Select(LTFALSE);
	return bHandled;
}

void CScreenPlayer::NextModel()
{
	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeCooperative:
		m_nCurrentModel++;
		if (m_nCurrentModel >= g_pModelButeMgr->GetNumCPModels())
			m_nCurrentModel = 0;
		break;
	case eGameTypeDeathmatch:
		m_nCurrentModel++;
		if (m_nCurrentModel >= g_pModelButeMgr->GetNumDMModels())
			m_nCurrentModel = 0;
		break;
	case eGameTypeTeamDeathmatch:
	case eGameTypeDoomsDay:
	default:
		return;
	};

	UpdateChar();
}

void CScreenPlayer::PrevModel()
{
	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeCooperative:
		if (!m_nCurrentModel)
			m_nCurrentModel = g_pModelButeMgr->GetNumCPModels();
		m_nCurrentModel--;

		break;
	case eGameTypeDeathmatch:
		if (!m_nCurrentModel)
			m_nCurrentModel = g_pModelButeMgr->GetNumDMModels();
		m_nCurrentModel--;
		break;
	case eGameTypeTeamDeathmatch:
	case eGameTypeDoomsDay:
	default:
		return;
	};
		
	UpdateChar();
}

void CScreenPlayer::UpdateChar()
{

	HOBJECT hChar = m_CharSFX.GetObject();
	if (hChar)
	{
		ModelId id = eModelIdInvalid;

		switch (g_pGameClientShell->GetGameType())
		{
		case eGameTypeCooperative:
			if (m_nCurrentModel >= g_pModelButeMgr->GetNumCPModels())
			{
				m_nCurrentModel = 0;
			}
			id = g_pModelButeMgr->GetCPModel(m_nCurrentModel);
			g_pLTClient->Common()->SetObjectFlags( hChar, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
			break;
		case eGameTypeDeathmatch:
			if (m_nCurrentModel >= g_pModelButeMgr->GetNumDMModels())
			{
				m_nCurrentModel = 0;
			}
			id = g_pModelButeMgr->GetDMModel(m_nCurrentModel);
			g_pLTClient->Common()->SetObjectFlags( hChar, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
			break;
		case eGameTypeTeamDeathmatch:
		case eGameTypeDoomsDay:
		default:
			g_pInterfaceMgr->RemoveInterfaceSFX(&m_CharSFX);
			m_CharSFX.Reset();
			m_CharSFX.Term();
			return;
		};


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


void CScreenPlayer::UpdateBandwidth()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if ( m_nBandwidth >= eBandwidth_Custom )
	{
		m_sBandwidth.Format( "%d", pProfile->m_nBandwidthClientCustom);
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
	else
	{
		m_sBandwidth.Format( "%d", g_BandwidthClient[m_nBandwidth]);
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
}

void CScreenPlayer::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_BANDWIDTH:
		{
			char *pszBandwidth = (char *)dwParam1;
			uint32 nBandwidth = (uint32)atoi(pszBandwidth);
			if ( IsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = pszBandwidth;
				m_sBandwidth = m_sBandwidth.Left( kMaxBandwidthStrLen );
				m_nBandwidth = eBandwidth_Custom;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				pProfile->m_nBandwidthClientCustom = (uint16)atol(m_sBandwidth);
				m_pBandwidthCycle->UpdateData(LTFALSE);
				UpdateBandwidth();
			}
			else
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(FormatTempString(IDS_BANDWIDTH_INVALID,GetMinimumBandwidth(),GetMaximumBandwidth()),&mb);
			}
		}
		break;
	case CMD_EDIT_NAME:
		{
			char* pName = ((char *)dwParam1);
			if (pName && strlen(pName))
			{
				m_sPlayerName = pName;
				m_pName->SetString(1,m_sPlayerName.c_str());
			}
		} break;
	}
	UpdateData();
}

