// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMission.cpp
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuMission.h"
#include "InterfaceMgr.h"
#include "MissionButeMgr.h"
#include "ClientResShared.h"
#include "ObjectiveControl.h"
#include "MissionMgr.h"

namespace
{
	char szOptional[256] = "";
	uint32 argbCompleted = 0xFF303030;
	uint8 nObjSize = 10;
}

CMenuMission::CMenuMission()
{
	m_pNameCtrl = LTNULL;
	m_pLevelCtrl = LTNULL;
	m_pObjLabel = LTNULL;
	m_pParameters = LTNULL;
	m_nFirstObj = -1;
}


LTBOOL CMenuMission::Init()
{
	m_MenuID = MENU_ID_MISSION;

	if (!CBaseMenu::Init()) return LTFALSE;

	SetTitle(IDS_TITLE_BRIEFING);

	uint16 nWidth = s_Size.x - 2*m_Indent.x;

	uint8 nMissSize = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"MissionFontSize");
	uint16 ndx = AddControl("<mission name>",0,LTTRUE);
	m_pNameCtrl = (CLTGUITextCtrl *)m_List.GetControl(ndx);
	m_pNameCtrl->SetFont(NULL,nMissSize);
	m_pNameCtrl->SetFixedWidth(nWidth);

	uint8 nLevSize = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"LevelFontSize");
	nWidth -= nLevSize;
	ndx = AddControl("<level name>",0,LTTRUE);
	m_pLevelCtrl = (CLTGUITextCtrl *)m_List.GetControl(ndx);
	m_pLevelCtrl->SetFont(NULL,nLevSize);
	m_pLevelCtrl->SetIndent(nLevSize);
	m_pLevelCtrl->SetFixedWidth(nWidth);

	AddControl(" ",0,LTTRUE);

	nObjSize = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"ObjectiveFontSize");

	m_nFirstObj = m_List.GetNumControls();

	m_List.SetItemSpacing(4);
	m_List.SetScrollByPage(LTTRUE);

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_MISSION,MENU_ID_MISSION);

	LTVector vColor = g_pLayoutMgr->GetMenuCustomVector(m_MenuID,"CompletedColor");
	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	argbCompleted = SET_ARGB(nA,nR,nG,nB);

	LoadString(IDS_OBJECTIVES_OPTIONAL,szOptional,sizeof(szOptional));

	m_BulletTex = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\bullet.dtx");
	m_CompletedTex = g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\completed.dtx");


	return LTTRUE;
}

void CMenuMission::Term()
{

	//take anything we've cached out of the list, so that we
	// don't try to delete them twice
	RemoveObjectives();

	//delete our cache
	ControlArray::iterator iter = m_Objectives.begin();
	while (iter != m_Objectives.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_Objectives.clear();

	if (m_pObjLabel)
	{
		debug_delete(m_pObjLabel);
		m_pObjLabel = LTNULL;
	}
	if (m_pParameters)
	{
		debug_delete(m_pParameters);
		m_pParameters = LTNULL;
	}

	CBaseMenu::Term();
}

uint32 CMenuMission::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
//	switch (nCommand)
//	{
//	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
//	}
//	return 1;
}

// This is called when the screen gets or loses focus
void CMenuMission::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		int mission = g_pMissionMgr->GetCurrentMission();
		int level = g_pMissionMgr->GetCurrentLevel();
		MISSION* pMission = g_pMissionButeMgr->GetMission(mission);
		if (pMission)
		{
			m_pNameCtrl->SetString(LoadTempString(pMission->nNameId));
			LEVEL* pLevel = g_pMissionButeMgr->GetLevel(mission,level);
			if (pLevel)
			{
				m_pLevelCtrl->SetString(LoadTempString(pLevel->nNameId));
			}
			else
				m_pLevelCtrl->SetString(" ");

		}
		else
		{
			m_pNameCtrl->SetString(LoadTempString(IDS_CUSTOM_LEVEL));
			m_pLevelCtrl->SetString(" ");
		}

		uint16 nextItem = m_nFirstObj;
		uint16 numItems = 0;
		uint16 nWidth = s_Size.x - 2*m_Indent.x;

		g_pPlayerStats->SetObjectivesSeen();

		// add required objectives
		IDList* pObj = g_pPlayerStats->GetObjectives();

		//check to see if we need to add a label
		if (pObj->m_IDArray.size())
		{
			// if we already have created a objectives label, use it
			if (m_pObjLabel)
			{
				m_List.AddControl(m_pObjLabel);
			}
			else
			{
				// otherwise create a label
				uint16 ndx = AddControl(IDS_OBJECTIVES,0,LTTRUE);
				m_pObjLabel = (CLTGUITextCtrl *)m_List.GetControl(ndx);
			}
		}

		for (int i = pObj->m_IDArray.size()-1; i >= 0 ; i--)
		{
			CObjectiveCtrl* pCtrl = LTNULL;
			uint32 objID = pObj->m_IDArray[i];
			// if we don't have a cached objective control, create a new one
			if (numItems >= m_Objectives.size())
			{
				uint16 ndx = AddObjectiveControl(objID);
				pCtrl = (CObjectiveCtrl*)m_List.GetControl(ndx);
				pCtrl->SetFixedWidth(nWidth);
				numItems++;
			}
			else
			{
				//otherwise, grab one from the cache
				pCtrl = (CObjectiveCtrl*)m_Objectives[numItems];
				m_List.AddControl(pCtrl);
				pCtrl->SetString(LoadTempString(objID));
				pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
				pCtrl->SetTexture(m_BulletTex);
				numItems++;

			}

		}

		// add optional objectives
		pObj = g_pPlayerStats->GetOptionalObjectives();

		//check to see if we need to add a label
		if (pObj->m_IDArray.size() && !numItems)
		{
			// if we already have created a objectives label, use it
			if (m_pObjLabel)
			{
				m_List.AddControl(m_pObjLabel);
			}
			else
			{
				// otherwise create a label
				uint16 ndx = AddControl(IDS_OBJECTIVES,0,LTTRUE);
				m_pObjLabel = (CLTGUITextCtrl *)m_List.GetControl(ndx);
			}
		}
		for (int i = pObj->m_IDArray.size()-1; i >= 0 ; i--)
		{
			CObjectiveCtrl* pCtrl = LTNULL;
			uint32 objID = pObj->m_IDArray[i];
			// if we don't have a cached objective control, create a new one
			if (numItems >= m_Objectives.size())
			{
				uint16 ndx = AddObjectiveControl(objID,LTFALSE,LTTRUE);
				pCtrl = (CObjectiveCtrl*)m_List.GetControl(ndx);
				pCtrl->SetFixedWidth(nWidth);
				numItems++;
			}
			else
			{
				//otherwise, grab one from the cache
				pCtrl = (CObjectiveCtrl*)m_Objectives[numItems];
				m_List.AddControl(pCtrl);
				char szString[256] = "";
				LoadString(objID,szString,sizeof(szString));
				strcat(szString,szOptional);

				pCtrl->SetString(szString);
				pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
				pCtrl->SetTexture(m_BulletTex);
				numItems++;

			}

		}

		// add completed objectives
		pObj = g_pPlayerStats->GetCompletedObjectives();

		//check to see if we need to add a label
		if (pObj->m_IDArray.size() && !numItems)
		{
			// if we already have created a objectives label, use it
			if (m_pObjLabel)
			{
				m_List.AddControl(m_pObjLabel);
			}
			else
			{
				// otherwise create a label
				uint16 ndx = AddControl(IDS_OBJECTIVES,0,LTTRUE);
				m_pObjLabel = (CLTGUITextCtrl *)m_List.GetControl(ndx);
			}
		}

		for (int i = pObj->m_IDArray.size()-1; i >= 0 ; i--)
		{
			CObjectiveCtrl* pCtrl = LTNULL;
			uint32 objID = pObj->m_IDArray[i];
			// if we don't have a cached objective control, create a new one
			if (numItems >= m_Objectives.size())
			{
				uint16 ndx = AddObjectiveControl(objID,LTTRUE);
				pCtrl = (CObjectiveCtrl*)m_List.GetControl(ndx);
				pCtrl->SetFixedWidth(nWidth);
				numItems++;
			}
			else
			{
				//otherwise, grab one from the cache
				pCtrl = (CObjectiveCtrl*)m_Objectives[numItems];
				m_List.AddControl(pCtrl);
				pCtrl->SetString(LoadTempString(objID));
				pCtrl->SetColors(argbCompleted,argbCompleted,argbCompleted);
				pCtrl->SetTexture(m_CompletedTex);
				numItems++;

			}

		}

		// add mission parameters
		pObj = g_pPlayerStats->GetParameters();

		//check to see if we need to add a label
		if (pObj->m_IDArray.size())
		{
			// if we already have created a parameters label, use it
			if (m_pParameters)
			{
				m_List.AddControl(m_pParameters);
			}
			else
			{
				// otherwise create a label
				uint16 ndx = AddControl(IDS_PARAMETERS,0,LTTRUE);
				m_pParameters = (CLTGUITextCtrl *)m_List.GetControl(ndx);
			}
		}
		for (int i = pObj->m_IDArray.size()-1; i >= 0 ; i--)
		{
			CObjectiveCtrl* pCtrl = LTNULL;
			uint32 objID = pObj->m_IDArray[i];

			// if we don't have a cached objective control, create a new one
			if (numItems >= m_Objectives.size())
			{
				uint16 ndx = AddObjectiveControl(objID);
				pCtrl = (CObjectiveCtrl*)m_List.GetControl(ndx);
				pCtrl->SetFixedWidth(nWidth);
				numItems++;
			}
			else
			{
				//otherwise, grab one from the cache
				pCtrl = (CObjectiveCtrl*)m_Objectives[numItems];
				m_List.AddControl(pCtrl);
				pCtrl->SetString(LoadTempString(objID));
				pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
				numItems++;

			}

		}

		m_List.SetStartIndex(0);
		
	}
	else
	{
		// take the objectives and parameters out of the list 
		// (they are still cached for re-use)
		RemoveObjectives();
	}
	CBaseMenu::OnFocus(bFocus);

}


// create and add an objective control
uint16 CMenuMission::AddObjectiveControl (int stringID, LTBOOL bCompleted, LTBOOL bOptional)
{
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	if (!pFont) return -1;

	CObjectiveCtrl* pCtrl=debug_new(CObjectiveCtrl);

	char szString[256] = "";
	LoadString(stringID,szString,sizeof(szString));

	if (!bCompleted && bOptional)
	{
		strcat(szString,szOptional);
	}

    if (!pCtrl->Create(szString, pFont, nObjSize, 16, m_BulletTex))
	{
		debug_delete(pCtrl);
        return -1;
	}

	pCtrl->SetBasePos(m_nextPos);
	pCtrl->Enable(LTFALSE);
	if (bCompleted)
	{
		pCtrl->SetColors(argbCompleted,argbCompleted,argbCompleted);
		pCtrl->SetTexture(m_CompletedTex);
	}
	else
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->SetTexture(m_BulletTex);
	}

	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	//cache the control for re-use
	m_Objectives.push_back(pCtrl);
	return m_List.AddControl(pCtrl);

}


void CMenuMission::RemoveObjectives()
{
	//take everything starting from the first objective out of the list
	// but don't delete them, because we might reuse them
	while (m_nFirstObj < m_List.GetNumControls())
		m_List.RemoveControl(m_nFirstObj,LTFALSE);
}