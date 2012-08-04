// FolderMods.cpp: implementation of the CFolderMods class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMods.h"
#include "FolderCommands.h"
#include "MissionData.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	HSTRING *hModDesc = LTNULL;
	char		sIntegratedStr[32] = "";

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFolderMods::CFolderMods()
{
	m_nMods = LTNULL;
	m_nAvailMods = 0;
}

CFolderMods::~CFolderMods()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::Build
//
//	PURPOSE:	Build the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderMods::Build()
{
	CreateTitle(IDS_TITLE_MODS);
    LTBOOL success = CBaseSelectionFolder::Build();
	if (strlen(sIntegratedStr) == 0)
	{
        HSTRING hTemp = g_pLTClient->FormatString(IDS_INTEGRATED);
        char *pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(sIntegratedStr,pTemp,ARRAY_LEN(sIntegratedStr));
        g_pLTClient->FreeString(hTemp);
	}
	return success;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::IsAvailable
//
//	PURPOSE:	Check to see if there any selections to be made here
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderMods::IsAvailable()
{
	int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);

	ClearArray();
	FillArray();

	return (pMission->nNumWeaponMods != 0 && m_nAvailMods > 0);

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::OnFocus
//
//	PURPOSE:	Handle gaining or losing focus
//
// ----------------------------------------------------------------------- //

void CFolderMods::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		ClearArray();
		FillArray();
		UseBack(LTTRUE);
		SetContinue();
		m_szModel[0] = '\0';
		m_szSkin[0] = '\0';
		BuildModsList();
		SetSelection(kNoSelection);
	}
	else
	{
		SetSelection(kNoSelection);
		SaveModData();
		ClearModsList();
		ClearArray();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::BuildModsList
//
//	PURPOSE:	Create the list of Mods
//
// ----------------------------------------------------------------------- //

void	CFolderMods::BuildModsList()
{
	//get info from MissionMgr
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();


	m_nNumSlots	= pMission->nNumWeaponMods;
	if (m_nNumSlots == -1 || m_nNumSlots > MAX_SELECTION_SLOTS)
		m_nNumSlots = MAX_SELECTION_SLOTS;


	//no choices allowed... don't go here
	if (m_nNumSlots <= 0) return;


	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
        hModDesc = debug_newa(HSTRING, nNumMods);
        memset(hModDesc, LTNULL, sizeof(HSTRING) * nNumMods);
	}

	int nMID = WMGR_INVALID_ID;
	for (nMID=0; nMID < nNumMods; nMID++)
	{
		MOD* pMod = g_pWeaponMgr->GetMod(nMID);
		switch (m_nMods[nMID])
		{
		case SEL_INTEGRAL:
			{
				AddToSlot(nMID,pMod->nDescriptionId,LTTRUE);
				m_nNumSlots++;
			} break;
		case SEL_REQUIRED:
			{
				AddToSlot(nMID,pMod->nDescriptionId,LTTRUE);
			} break;
		case SEL_DEFAULT:
			{
				AddToSlot(nMID,pMod->nDescriptionId,LTFALSE);
			} break;
		case SEL_ALLOWED:
			{
				AddItem(nMID,pMod->nDescriptionId);
			} break;
		}

		if (m_nMods[nMID] != SEL_NOT_ALLOWED)
		{
			int descId = 0;

			switch (pMod->eType)
			{
			case SILENCER:
				descId = IDS_SILENCER;
				break;
			case LASER:
				descId = IDS_LASER;
				break;
			case SCOPE:
				if (pMod->bNightVision)
					descId = IDS_NIGHT_SCOPE;
				else
					descId = IDS_SCOPE;
				break;
			}
			if (descId)
			{
				if (pMod->bIntegrated)
				{
					HSTRING hTemp = g_pLTClient->FormatString(descId);
					char szStr[512];
					sprintf(szStr,"%s\n%s",g_pLTClient->GetStringData(hTemp),sIntegratedStr);
					hModDesc[nMID] = g_pLTClient->CreateString(szStr);
					g_pLTClient->FreeString(hTemp);
				}
				else
				{
					hModDesc[nMID] = g_pLTClient->FormatString(descId);
				}
			}

		}
	}


	int nNumSelectable = (int)m_controlArray.GetSize();
	int nNumFree = m_nNumSlots - m_nSlotsFilled;
	if  (nNumFree > nNumSelectable)
	{
		nNumFree = nNumSelectable;
		m_nNumSlots = m_nSlotsFilled + nNumFree;
		while (m_controlArray.GetSize())
		{
			nNumFree--;
			CLTGUICtrl *pCtrl = GetControl(0);
			if (pCtrl)
			{
				int nMID = pCtrl->GetParam1();
				MOD* pMod = g_pWeaponMgr->GetMod(nMID);
				ItemToSlot(nMID,pMod->nDescriptionId);
			}
		}

	}

	for (int i=0;i < nNumFree;i++)
	{
		AddEmptySlot();
	}

	
	return;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::UpdateSelection
//
//	PURPOSE:	Show info based on current selection
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderMods::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		int modId = pCtrl->GetParam1();
		MOD* pMod = g_pWeaponMgr->GetMod(modId);
		if (pMod)
		{

			m_pName->RemoveAll();
			m_pName->AddString(pMod->nDescriptionId);

			m_pDescription->SetString(hModDesc[modId]);

			
			SAFE_STRCPY(m_szModel, pMod->szInterfaceModel);
			SAFE_STRCPY(m_szSkin, pMod->szInterfaceSkin);
			VEC_COPY(m_vOffset, pMod->vInterfaceOffset);
			m_fScale = pMod->fInterfaceScale;

			CreateModelSFX();
		}
	}
	return bChanged;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::ClearModsList
//
//	PURPOSE:	Remove all of the controls
//
// ----------------------------------------------------------------------- //

void CFolderMods::ClearModsList()
{
	// Terminate the ctrls
	RemoveFree();
	ClearSlots();
	ClearSelection();

	if (hModDesc)
	{
		int nNumMods = g_pWeaponMgr->GetNumModTypes();

		int nMID = WMGR_INVALID_ID;
		for (nMID=0; nMID < nNumMods; nMID++)
		{
			if (hModDesc[nMID])
				g_pLTClient->FreeString(hModDesc[nMID]);
		}
		debug_deletea(hModDesc);
		hModDesc = LTNULL;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::SaveModData
//
//	PURPOSE:	Save the players selections
//
// ----------------------------------------------------------------------- //

void CFolderMods::SaveModData()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	pData->ClearMods();

	if (m_bSaveSelection)
	{
		for (int slot = 0; slot < m_nSlotsFilled; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl && (int)pCtrl->GetParam1() != kEmptySlot)
				pData->AddMod(pCtrl->GetParam1());
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderMods::OnCommand
//
//	PURPOSE:	Handle activation of items
//
// ----------------------------------------------------------------------- //

uint32 CFolderMods::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case FOLDER_CMD_SELECT_SLOT:
		{
			if (dwParam2)
			{
				g_pInterfaceMgr->RequestInterfaceSound(IS_NO_SELECT);
				return 0;
			}
			int nMID = (int)dwParam1;
			MOD *pMod = g_pWeaponMgr->GetMod(nMID);
			SlotToItem(nMID,pMod->nDescriptionId);
			return 1;
		} break;
		
	case FOLDER_CMD_SELECT_ITEM:
		{
			int nMID = (int)dwParam1;
			MOD *pMod = g_pWeaponMgr->GetMod(nMID);
			ItemToSlot(nMID,pMod->nDescriptionId);
			return 1;
		} break;

	case FOLDER_CMD_CONTINUE:
		{
			if (m_pContinue->GetHelpID() == IDS_HELP_START)
			{
				int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();

				g_pGameClientShell->StartMission(missionNum);
				return 1;
			}
		} break;
	}

	return CBaseSelectionFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


HSTRING CFolderMods::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
    MOD* pMod = LTNULL;
	char pStr[512] = "";

	//slots are fixed controls so count  negatively ( 0 > first > last )
	int nLastSlot = (m_nFirstSlot - m_nNumSlots) + 1;
	if (nControlIndex >= 0 || (nControlIndex <= m_nFirstSlot && nControlIndex >= nLastSlot))
	{
		CLTGUICtrl *pCtrl = GetControl(nControlIndex);
		int weaponId = pCtrl->GetParam1();
		if (weaponId == kEmptySlot) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		pMod = g_pWeaponMgr->GetMod(weaponId);
		if (!pMod) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		int nameId = pMod->nDescriptionId;

        HSTRING hTemp = g_pLTClient->FormatString(nameId);
        char *pName = g_pLTClient->GetStringData(hTemp);



		if (nControlIndex < 0)
		{
			//over a slot
			if (pCtrl->GetParam2())
			{
				sprintf(pStr,"%s %s",pName,m_sRequiredStr);
			}
			else if (nControlIndex < 0)
			{
				sprintf(pStr,"%s %s",m_sUnselectStr,pName);
			}
		}
		else
		{
			sprintf(pStr,"%s %s",m_sSelectStr,pName);
		}

        g_pLTClient->FreeString(hTemp);
		
        HSTRING hStr = g_pLTClient->CreateString(pStr);
		return hStr;
	}
	else
		return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);
}


void CFolderMods::SetContinue()
{
    int nHelp = LTNULL;
	eFolderID eNext = GetNextSelectionFolder(FOLDER_ID_MODS,&nHelp);

	if (eNext != FOLDER_ID_NONE)
	{
		UseContinue(eNext,nHelp);
	}
	else
	{
		UseContinue(FOLDER_ID_MAIN,IDS_HELP_START,IDS_START_MISSION);
	}

}

void CFolderMods::CreateModelSFX()
{

	// no model = no SFX
	if (!strlen(m_szModel)) return;

	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


	BSCREATESTRUCT bcs;

    LTVector vPos, vU, vR, vF, vTemp, vScale(1.0f,1.0f,1.0f);
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

    g_pLTClient->RotateAroundAxis(&rRot, &vU, MATH_HALFPI);
    g_pLTClient->RotateAroundAxis(&rRot, &vR, -0.3f);

	VEC_MULSCALAR(vScale, vScale, m_fScale);

    LTVector vModPos = g_pLayoutMgr->GetFolderCustomVector((eFolderID)m_nFolderID,"ModelPos");
	VEC_ADD(vModPos,vModPos,m_vOffset);

	VEC_MULSCALAR(vTemp, vF, vModPos.z);
	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vR, vModPos.x);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, vU, vModPos.y);
	VEC_ADD(vPos, vPos, vTemp);


	VEC_COPY(bcs.vPos, vPos);
    bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);
	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
    bcs.bUseUserColors = LTTRUE;

	bcs.pFilename = m_szModel;
	bcs.pSkin = m_szSkin;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 1.0f;
	bcs.fFinalAlpha = 1.0f;
	bcs.fLifeTime = 1000000.0f;

	if (m_ModelSFX.Init(&bcs))
	{
        m_ModelSFX.CreateObject(g_pLTClient);
		if (m_ModelSFX.GetObject())
		{
			g_pInterfaceMgr->AddInterfaceSFX(&m_ModelSFX, IFX_NORMAL);
			m_fSFXRot = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"ModelRotSpeed");
		}
	}

}


void CFolderMods::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();
	g_pInterfaceMgr->RemoveInterfaceSFX(&m_ModelSFX);
	m_ModelSFX.Term();
}


void CFolderMods::UpdateInterfaceSFX()
{
	CBaseSelectionFolder::UpdateInterfaceSFX();

	if (m_ModelSFX.GetObject())
	{
        LTFLOAT spin = g_pGameClientShell->GetFrameTime() * m_fSFXRot;
        LTVector vU, vR, vF;
        LTRotation rRot;
        g_pLTClient->GetObjectRotation(m_ModelSFX.GetObject(), &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
        g_pLTClient->RotateAroundAxis(&rRot, &vU, spin);
        g_pLTClient->SetObjectRotation(m_ModelSFX.GetObject(),&rRot);
	}
}


void CFolderMods::SkipOutfitting()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	pData->ClearMods();

	ClearArray();
	FillArray();

	int nMID = WMGR_INVALID_ID;
	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	for (nMID=0; nMID < nNumMods; nMID++)
	{
		MOD* pMod = g_pWeaponMgr->GetMod(nMID);
		if (m_nMods[nMID] == SEL_INTEGRAL	||
			m_nMods[nMID] == SEL_DEFAULT	||
			m_nMods[nMID] == SEL_REQUIRED	)

		{
			pData->AddMod(nMID);		
		}
	}


}




void CFolderMods::FillArray()
{
	//get info from MissionMgr
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();


	int nMID = WMGR_INVALID_ID;
	int nNumMods = g_pWeaponMgr->GetNumModTypes();
	if (nNumMods > 0)
	{
        m_nMods = debug_newa(int, nNumMods);
        memset(m_nMods, SEL_NOT_ALLOWED, sizeof(int) * nNumMods);
	}

	for (nMID=0; nMID < nNumMods; nMID++)
	{
		if (pStats->CanUseMod(nMID))
			m_nMods[nMID] = SEL_ALLOWED;
	}

	for (int i=0; i < pMission->nNumAllowedMods; i++)
	{
		nMID = pMission->aAllowedMods[i];
		m_nMods[nMID] = SEL_ALLOWED;
	}

	int numCurrMods = pData->GetNumModTypes();
	if (numCurrMods)
	{
		CModData* currMods[255];

		numCurrMods = pData->GetMods(currMods,255);
		for (i=0; i< numCurrMods; i++)
		{
			nMID = currMods[i]->m_nID;
			m_nMods[nMID] = SEL_DEFAULT;
		}

	}
	else
	{

		for (i=0; i < pMission->nNumDefaultMods; i++)
		{
			nMID = pMission->aDefaultMods[i];
			m_nMods[nMID] = SEL_DEFAULT;
		}
	}

	for (i=0; i < pMission->nNumRequiredMods; i++)
	{
		nMID = pMission->aRequiredMods[i];
		m_nMods[nMID] = SEL_REQUIRED;
	}


	// step through all the weapons
	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	for (int wid = 0; wid < nNumWeapons; wid++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(wid);
		// for each weapon, step through all of its mods
		for (i = 0; i < pWeapon->nNumModTypes; i++)
		{
			nMID = pWeapon->aModTypes[i];
			if (!g_pWeaponMgr->IsValidModType(nMID))
				continue;
			if (pData->GetWeaponData(wid))
			{
				// if we've selected the weapon, check for integral mods
				MOD* pMod = g_pWeaponMgr->GetMod(nMID);
				if (pMod && pMod->bIntegrated)
					m_nMods[nMID] = SEL_INTEGRAL;
				
			}
			else
			{
				//if the weapon is not selected, we can't select its mods
				m_nMods[nMID] = SEL_NOT_ALLOWED;
			}

		}
	}

	m_nAvailMods = 0;
	for (nMID=0; nMID < nNumMods; nMID++)
	{
		if (m_nMods[nMID] != SEL_NOT_ALLOWED)
			 m_nAvailMods++;
	}

		

}
void CFolderMods::ClearArray()
{
	if (m_nMods)
	{
		debug_deletea(m_nMods);
		m_nMods = LTNULL;
	}
	m_nAvailMods = 0;
}
