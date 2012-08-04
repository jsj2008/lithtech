// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMission.cpp
//
// PURPOSE : In-Game Menu to display mission status
//
// CREATED : 07/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "MenuMission.h"
#include "MenuMgr.h"
#include "InterfaceMgr.h"
#include "ObjectiveDB.h"
#include "vkdefs.h"

CMenuMission::CMenuMission( )
{
	m_pMissionCtrl = NULL;
	m_pObjectiveCtrl = NULL;
}

bool CMenuMission::Init( CMenuMgr& menuMgr )
{
	m_MenuID = MENU_ID_MISSION;

	if (!CBaseMenu::Init( menuMgr )) 
		return false;

	HRECORD hMenuRec = GetMenuRecord();

	uint16 nMissionCtrlId = AddControl(L"Mission",NULL,true);
	m_pMissionCtrl = ( CLTGUITextCtrl* )m_List.GetControl( nMissionCtrlId );
	m_pMissionCtrl->SetWordWrap(true);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.Bottom() = g_pLayoutDB->GetInt32(hMenuRec,LDB_nMenuAddInt,0);
	cs.rnBaseRect.Right() = m_vDefaultSize.x - 2*m_Indent.x;
	TextureReference hFrame(g_pLayoutDB->GetString(hMenuRec,LDB_sMenuAddTex,0));
	m_pLine = debug_new(CLTGUIFrame);
	m_pLine->Create(hFrame,cs);
	m_List.AddControl(m_pLine);

	CFontInfo sTextFont(m_FontFace.c_str(),g_pLayoutDB->GetInt32(hMenuRec,LDB_nMenuAddInt,1));

	CLTGUITextureButton_create bcs;
	bcs.rnImageRect.m_vMin.Init();
	bcs.rnImageRect.m_vMax = g_pLayoutDB->GetPosition(hMenuRec,LDB_vMenuAddPoint,0);

	bcs.rnBaseRect.Bottom() = bcs.rnImageRect.Bottom();
	bcs.rnBaseRect.Right() = m_vDefaultSize.x - 2*m_Indent.x;

	bcs.rnTextRect.m_vMin = g_pLayoutDB->GetPosition(hMenuRec,LDB_vMenuAddPoint,1);
	bcs.rnTextRect.m_vMax = bcs.rnBaseRect.m_vMax;
	bcs.bCenterImage = false;
	bcs.bCenterText = false;
	bcs.hNormal = hFrame;

	m_pObjectiveCtrl = debug_new(CLTGUITextureButton);
	m_pObjectiveCtrl->Create(bcs);
	m_pObjectiveCtrl->SetFont(sTextFont);
	m_pObjectiveCtrl->SetAlignment(kLeft);
	m_pObjectiveCtrl->SetColor(m_SelectedColor);
	m_pObjectiveCtrl->Enable(false);
	m_pObjectiveCtrl->Show(true);
	m_List.AddControl(m_pObjectiveCtrl);

	m_fWait = g_pLayoutDB->GetFloat(hMenuRec,LDB_fMenuAddFloat,0);
	m_KeyWaitTimer.SetEngineTimer(RealTimeTimer::Instance( ));

	return true;
}

uint32 CMenuMission::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
}

bool CMenuMission::HandleKeyDown(int /*key*/, int /*rep*/)
{
	if (m_KeyWaitTimer.IsStarted() && m_KeyWaitTimer.IsTimedOut())
	{
		m_bKeyDown = true;
	}
	
	return true;
}

// Mouse messages
bool CMenuMission::OnLButtonDown(int /*x*/, int /*y*/)
{
	if (m_KeyWaitTimer.IsStarted() && m_KeyWaitTimer.IsTimedOut())
	{
		m_bLBDown = true;
	}
	return true;
}
// Mouse messages
bool CMenuMission::OnRButtonDown(int /*x*/, int /*y*/)
{
	if (m_KeyWaitTimer.IsStarted() && m_KeyWaitTimer.IsTimedOut())
	{
		m_bRBDown = true;
	}
	return true;
}

bool CMenuMission::HandleKeyUp(int /*key*/)
{
	if (m_bKeyDown)
	{
		m_pMenuMgr->Close();
		m_bKeyDown = false;
	}
	return true;
}

// Mouse messages
bool CMenuMission::OnLButtonUp(int /*x*/, int /*y*/)
{
	if (m_bLBDown)
	{
		m_pMenuMgr->Close();
		m_bLBDown = false;
	}
	return true;
}
// Mouse messages
bool CMenuMission::OnRButtonUp(int /*x*/, int /*y*/)
{
	if (m_bRBDown)
	{
		m_pMenuMgr->Close();
		m_bRBDown = false;
	}
	return true;
}

void CMenuMission::OnFocus(bool bFocus)
{
	ClearSelection();
	m_List.ClearSelection();
	m_bKeyDown = false;
	m_bLBDown = false;
	m_bRBDown = false;

	CBaseMenu::OnFocus(bFocus);

	if (bFocus)
	{
		m_KeyWaitTimer.Start(m_fWait);

		m_pMissionCtrl->SetString(LoadString(g_pPlayerStats->GetMission()));
		LTRect2n rExtents;
		m_pMissionCtrl->RecreateTextureStrings();
		m_pMissionCtrl->GetExtents(rExtents);
		LTVector2n vSz;
		vSz.x = m_pMissionCtrl->GetBaseWidth();
		vSz.y = (uint32)( (float)rExtents.GetHeight() / m_vfScale.y);
		m_pMissionCtrl->SetSize( vSz );


		HRECORD hObjective = g_pPlayerStats->GetObjective();
		if (hObjective)
		{
			const char* pszString = DATABASE_CATEGORY( Objective ).GETRECORDATTRIB( hObjective, Objective );
			const wchar_t* wszObj = LoadString(pszString);
			const char* pszIcon = DATABASE_CATEGORY( Objective ).GETRECORDATTRIB( hObjective, IconTexture );
			m_pObjectiveCtrl->SetText(wszObj,false);
			m_pObjectiveCtrl->SetTexture( TextureReference(pszIcon) );
			m_pObjectiveCtrl->Show(true);
			m_pLine->Show(true);
		}
		else
		{
			m_pObjectiveCtrl->Show(false);
			m_pLine->Show(false);
		}

		m_List.CalculatePositions();
		
	}
}

// Render the control
void CMenuMission::Render ( )
{
	if (!IsVisible()) return;

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	LTPoly_G4 back;
	DrawPrimSetRGBA(back, SET_ARGB(0xB0,0,0,0));
	DrawPrimSetXYWH(back,-0.5f,-0.5f,(float)g_pInterfaceResMgr->GetScreenWidth(),(float)g_pInterfaceResMgr->GetScreenHeight());
	g_pDrawPrim->DrawPrim(&back,1);

	CBaseMenu::Render();
}



