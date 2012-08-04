// FolderHostOptions.cpp: implementation of the CFolderHostOptions class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderHostOptions.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "WeaponMgr.h"
#include "NetDefs.h"
#include "MsgIds.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;



extern VarTrack	g_vtNetGameType;


namespace
{
	int kHeaderWidth = 300;
	int kSpacerWidth = 25;
	int kSliderWidth = 300;
	int kTotalWidth  = kHeaderWidth + kSpacerWidth;
	int	g_nWeaponCommands[50];
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderHostOptions::CFolderHostOptions()
{
    m_anValues = LTNULL;
}

CFolderHostOptions::~CFolderHostOptions()
{

}

// Build the folder
LTBOOL CFolderHostOptions::Build()
{
	if (!g_vtNetGameType.IsInitted())
	{
        g_vtNetGameType.Init(g_pLTClient,"NetGameType",LTNULL,0.0f);
	}

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_HOST_OPTIONS,"ColumnWidth"))
	{
		kTotalWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_OPTIONS,"ColumnWidth");
		kHeaderWidth = kTotalWidth - kSpacerWidth;
	}
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_HOST_OPTIONS,"SliderWidth"))
		kSliderWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_OPTIONS,"SliderWidth");


	CreateTitle(IDS_TITLE_HOST_OPTIONS);

	if (!m_anValues)
	{
		m_anValues = debug_newa(int, g_pServerOptionMgr->GetNumOptions());
		memset(m_anValues,0,sizeof(m_anValues));
	}

 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;
}

void CFolderHostOptions::Term()
{
	if (m_anValues)
	{
		debug_deletea(m_anValues);
        m_anValues = LTNULL;
	}
	// Make sure to call the base class
	CBaseFolder::Term();
}

uint32 CFolderHostOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


// Change in focus
void    CFolderHostOptions::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_nGameType = (int)g_vtNetGameType.GetFloat();
		int defStrId = IDS_SPACER;
		for (int i = 0; i < g_pServerOptionMgr->GetNumOptions(); i++)
		{
			OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
			if (!IsAllowedGameType((int)pOpt->eGameType)) continue;

			if (pOpt->fSliderScale == 0.0f)
				m_anValues[i] = (int) pOpt->GetValue();
			else
				m_anValues[i] = (int) (pOpt->GetValue() / pOpt->fSliderScale);


			switch (pOpt->eType)
			{
			case SO_TOGGLE:
				{
                    CToggleCtrl *pToggle = AddToggle(pOpt->nNameId,pOpt->nHelpId,kTotalWidth,(LTBOOL *)&m_anValues[i]);
					int onId = IDS_ON;
					int offId = IDS_OFF;
					if (pOpt->nNumStrings >= 1 && pOpt->nStringId[0])
					{
						offId = pOpt->nStringId[0];
					}
					if (pOpt->nNumStrings >= 2  && pOpt->nStringId[1])
					{
						onId = pOpt->nStringId[1];
					}
					pToggle->SetOffString(offId);
					pToggle->SetOnString(onId);
				} break;
			case SO_CYCLE:
				{
					CCycleCtrl *pCycle = AddCycleItem(pOpt->nNameId,pOpt->nHelpId,kHeaderWidth,kSpacerWidth,&m_anValues[i]);
					for (int s = 0; s < pOpt->nNumStrings; s++)
					{
						if (pOpt->nStringId[s])
							pCycle->AddString(pOpt->nStringId[s]);
						else
							pCycle->AddString(defStrId);
					}
				} break;
			case SO_SLIDER:
			case SO_SLIDER_NUM:
				{
					CSliderCtrl *pSlider = AddSlider(pOpt->nNameId,pOpt->nHelpId,kTotalWidth, kSliderWidth, &m_anValues[i]);
					pSlider->SetSliderRange(pOpt->nSliderMin, pOpt->nSliderMax);
					pSlider->SetSliderIncrement(pOpt->nSliderInc);
					pSlider->SetNumericDisplay( (SO_SLIDER_NUM == pOpt->eType) );
				} break;
			case SO_SPECIAL:
				{
					if (stricmp(pOpt->szVariable,"NetDefaultWeapon") == 0)
					{
						CCycleCtrl *pCycle = AddCycleItem(pOpt->nNameId,pOpt->nHelpId,kHeaderWidth,kSpacerWidth,&m_anValues[i]);
						WEAPON* pWeapon = LTNULL;
						pCycle->AddString(IDS_NONE);
						int nNumWeaponsCommands = 0;
						for (int nCommand = g_pWeaponMgr->GetFirstWeaponCommandId() + 1; nCommand <= g_pWeaponMgr->GetLastWeaponCommandId(); nCommand++)
						{
							int nWId = g_pWeaponMgr->GetWeaponId(nCommand);
							pWeapon = g_pWeaponMgr->GetWeapon(nWId);
							if (pWeapon && pWeapon->bCanBeDefault)
							{
								pCycle->AddString(pWeapon->nNameId);
								g_nWeaponCommands[nNumWeaponsCommands] = nCommand;
								nNumWeaponsCommands++;

							}
						}

						//translate from commandId to string index
						int oldCommandId = m_anValues[i];
						int nIndex = 0;
						for (int c = 0; c < nNumWeaponsCommands && !nIndex; c++)
						{
							if (g_nWeaponCommands[c] == oldCommandId)
								nIndex = (c+1);
						}
						m_anValues[i] = nIndex;


					}
				}

			}
		}

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		for (int i = 0; i < g_pServerOptionMgr->GetNumOptions(); i++)
		{
			OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
			if (IsAllowedGameType((int)pOpt->eGameType))
			{
				if (stricmp(pOpt->szVariable,"NetDefaultWeapon") == 0 && m_anValues[i] > 0)
				{
					//translate from string index to command id
					pOpt->SetValue((LTFLOAT)g_nWeaponCommands[m_anValues[i]-1]);
				}
				else if (pOpt->fSliderScale == 0.0f)
                    pOpt->SetValue((LTFLOAT)m_anValues[i]);
				else
                    pOpt->SetValue((LTFLOAT)m_anValues[i] * pOpt->fSliderScale);
			}
		}
		RemoveFree();
		if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->IsHosting())
		{

			int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
			if (nNumOptions > MAX_GAME_OPTIONS)
				nNumOptions = MAX_GAME_OPTIONS;
			
			HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_UPDATE_OPTIONS);
			for (i = 0; i < nNumOptions; i++)
			{
				OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
				if (g_pGameClientShell->GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
				{
					LTFLOAT fVal = pOpt->GetValue();
					g_pLTClient->WriteToMessageFloat(hWrite,fVal);
				}
			}

		    g_pLTClient->EndMessage(hWrite);
		}

        g_pLTClient->WriteConfigFile("autoexec.cfg");

	}
	CBaseFolder::OnFocus(bFocus);
}

