// FolderMultiBriefing.cpp: implementation of the CFolderMultiBriefing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMultiBriefing.h"
#include "BaseSelectionFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "SoundMgr.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMultiBriefing::CFolderMultiBriefing()
{

}

CFolderMultiBriefing::~CFolderMultiBriefing()
{

}


LTBOOL CFolderMultiBriefing::Build()
{

	CreateTitle(IDS_TITLE_BRIEFING);


	UseBack(LTFALSE);
	UseMain(LTFALSE);
	UseContinue(LTFALSE);

	return 	CBaseFolder::Build();
;
}


void CFolderMultiBriefing::Escape()
{
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);
	// Force the loading screen to avoid the "has a new level loaded" check
	g_pInterfaceMgr->OnEnterWorld();
	g_pGameClientShell->InitMultiPlayer();
}

uint32 CFolderMultiBriefing::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


void CFolderMultiBriefing::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		int missionId = g_pGameClientShell->GetMPMissionName();
		int briefId = g_pGameClientShell->GetMPMissionBriefing();

		if (missionId)
		{
			CLTGUIFont *pFont = (briefId ? GetMediumFont() : GetLargeFont());
			CLTGUITextItemCtrl* pCtrl =AddTextItem(missionId,LTNULL,0,LTTRUE,pFont);
			pCtrl->SetFixedWidth(480);
			pCtrl->Enable(LTFALSE);
		}
		if (briefId)
		{
			if (missionId) AddBlankLine();

			CLTGUITextItemCtrl* pCtrl=AddTextItem(IDS_BRIEFING,LTNULL,0,LTTRUE,GetMediumFont());
			pCtrl->Enable(LTFALSE);

			CStaticTextCtrl *pBrief = AddStaticTextItem(briefId,LTNULL,LTNULL,480,0,LTTRUE,GetSmallFont());
			if (pBrief)
			{
				pBrief->Enable(LTFALSE);
			}

		}

		UseBack(LTFALSE);
	}
	else
	{
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);

}

