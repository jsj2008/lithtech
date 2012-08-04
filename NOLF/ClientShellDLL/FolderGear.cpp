// FolderGear.cpp: implementation of the CFolderGear class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderGear.h"
#include "FolderCommands.h"
#include "MissionData.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFolderGear::CFolderGear()
{
	m_fSFXRot = 0.0f;
}

CFolderGear::~CFolderGear()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::Build
//
//	PURPOSE:	Build the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderGear::Build()
{
	CreateTitle(IDS_TITLE_GEAR);
    LTBOOL success = CBaseSelectionFolder::Build();
	return success;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::IsAvailable
//
//	PURPOSE:	Check to see if there any selections to be made here
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderGear::IsAvailable()
{
	int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);

	if (pMission->nNumGear == 0) return LTFALSE;

	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();
		
	if (pMission->nNumRequiredGear > 0) return LTTRUE;
	if (pMission->nNumOneTimeGear > 0) return LTTRUE;
	if (pMission->nNumAllowedGear > 0) return LTTRUE;

	int numGear = g_pWeaponMgr->GetNumGearTypes();
	for (int i=0; i < numGear; i++)
	{
		GEAR *pGear = g_pWeaponMgr->GetGear(i);
		if (pStats->CanUseGear(i) && pGear && pGear->bExclusive)
		{

			LTBOOL bDenied = LTFALSE;
			for (int j=0; j < pMission->nNumDeniedGear && !bDenied; j++)
			{
				bDenied = (pMission->aDeniedGear[j] == i);
			}
			return !bDenied;

		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::OnFocus
//
//	PURPOSE:	Handle gaining or losing focus
//
// ----------------------------------------------------------------------- //

void CFolderGear::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		UseBack(LTTRUE);
		SetContinue();
		m_szModel[0] = '\0';
		m_szSkin[0] = '\0';
		BuildGearList();
		SetSelection(kNoSelection);
	}
	else
	{
		SetSelection(kNoSelection);
		SaveGearData();
		ClearGearList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::BuildGearList
//
//	PURPOSE:	Create the list of Gear
//
// ----------------------------------------------------------------------- //

void	CFolderGear::BuildGearList()
{
	//get info from MissionMgr
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();


	m_nNumSlots	= pMission->nNumGear;
	if (m_nNumSlots == -1 || m_nNumSlots > MAX_SELECTION_SLOTS)
		m_nNumSlots = MAX_SELECTION_SLOTS;


	//no choices allowed... don't go here
	if (m_nNumSlots <= 0) return;

	int nGID = WMGR_INVALID_ID;
    GEAR* pGear = LTNULL;
	int i=0;

	for (i=0; i< pMission->nNumRequiredGear; i++)
	{
		nGID = pMission->aRequiredGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		AddToSlot(nGID,pGear->nNameId,LTTRUE);
	}
	for (i=0; i< pMission->nNumOneTimeGear; i++)
	{
		nGID = pMission->aOneTimeGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		AddToSlot(nGID,pGear->nNameId,LTTRUE);
	}

	int numCurrGear = pData->GetNumGearTypes();
	if (numCurrGear)
	{
		CGearData* currGear[255];

		GEAR* pGear = LTNULL;

		numCurrGear = pData->GetGear(currGear,255);
		for (i=0; i< numCurrGear; i++)
		{
			nGID = currGear[i]->m_nID;
			pGear = g_pWeaponMgr->GetGear(nGID);
			if (pGear && pGear->bExclusive)
			{
				AddToSlot(nGID,pGear->nNameId,LTFALSE);
			}
		}
	}


	for (i=0; i< pMission->nNumAllowedGear; i++)
	{
		nGID = pMission->aAllowedGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		AddItem(nGID,pGear->nNameId);
	}


	int numGear = g_pWeaponMgr->GetNumGearTypes();
	for (i=0; i < numGear; i++)
	{
		pGear = g_pWeaponMgr->GetGear(i);
		if (pStats->CanUseGear(i) && pGear && pGear->bExclusive)
		{
			AddItem(i,pGear->nNameId);
		}
	}

	for (i=0; i< pMission->nNumDeniedGear; i++)
	{
		nGID = pMission->aDeniedGear[i];
		RemoveFromSlot(nGID);
		RemoveItem(nGID);
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
				int nGID = pCtrl->GetParam1();
				GEAR* pGear = g_pWeaponMgr->GetGear(nGID);
				if (pGear && pGear->bExclusive)
					ItemToSlot(nGID,pGear->nNameId);
			}
		}
	}

	for (i=0;i < nNumFree;i++)
	{
		AddEmptySlot();
	}
	
	return;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::UpdateSelection
//
//	PURPOSE:	Show info based on current selection
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderGear::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		int gearId = pCtrl->GetParam1();
		GEAR* pGear = g_pWeaponMgr->GetGear(gearId);
		if (pGear)
		{

			m_pName->RemoveAll();
			m_pName->AddString(pGear->nNameId);

			m_pDescription->SetString(pGear->nDescriptionId);

			SAFE_STRCPY(m_szModel, pGear->szInterfaceModel);
			SAFE_STRCPY(m_szSkin, pGear->szInterfaceSkin);
			VEC_COPY(m_vOffset, pGear->vInterfaceOffset);
			m_fScale = pGear->fInterfaceScale;
			CreateModelSFX();
		}
	}
	return bChanged;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::ClearGearList
//
//	PURPOSE:	Remove all of the controls
//
// ----------------------------------------------------------------------- //

void CFolderGear::ClearGearList()
{
	// Terminate the ctrls
	RemoveFree();
	ClearSlots();
	ClearSelection();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::SaveGearData
//
//	PURPOSE:	Save the players selections
//
// ----------------------------------------------------------------------- //

void CFolderGear::SaveGearData()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	pData->ClearGear();

	if (m_bSaveSelection)
	{
		for (int slot = 0; slot < m_nSlotsFilled; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl && (int)pCtrl->GetParam1() != kEmptySlot)
				pData->AddGear(pCtrl->GetParam1());
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderGear::OnCommand
//
//	PURPOSE:	Handle activation of items
//
// ----------------------------------------------------------------------- //

uint32 CFolderGear::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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
			int nGID = (int)dwParam1;
			GEAR *pGear = g_pWeaponMgr->GetGear(nGID);
			SlotToItem(nGID,pGear->nNameId);
			return 1;
		} break;
		
	case FOLDER_CMD_SELECT_ITEM:
		{
			int nGID = (int)dwParam1;
			GEAR *pGear = g_pWeaponMgr->GetGear(nGID);
			ItemToSlot(nGID,pGear->nNameId);
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


HSTRING CFolderGear::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
    GEAR* pGear = LTNULL;
	char pStr[512] = "";

	//slots are fixed controls so count  negatively ( 0 > first > last )
	int nLastSlot = (m_nFirstSlot - m_nNumSlots) + 1;
	if (nControlIndex >= 0 || (nControlIndex <= m_nFirstSlot && nControlIndex >= nLastSlot))
	{
		CLTGUICtrl *pCtrl = GetControl(nControlIndex);
		int gearId = pCtrl->GetParam1();
		if (gearId == kEmptySlot) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		pGear = g_pWeaponMgr->GetGear(gearId);
		if (!pGear) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		int nameId = pGear->nNameId;

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


void CFolderGear::SetContinue()
{
    int nHelp = LTNULL;
	eFolderID eNext = GetNextSelectionFolder(FOLDER_ID_GEAR,&nHelp);

	if (eNext != FOLDER_ID_NONE)
	{
		UseContinue(eNext,nHelp);
	}
	else
	{
		UseContinue(FOLDER_ID_MAIN,IDS_HELP_START,IDS_START_MISSION);
	}

}

void CFolderGear::CreateModelSFX()
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
		g_pInterfaceMgr->AddInterfaceSFX(&m_ModelSFX, IFX_NORMAL);
		m_fSFXRot = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"ModelRotSpeed");
	}

}


void CFolderGear::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();
	g_pInterfaceMgr->RemoveInterfaceSFX(&m_ModelSFX);
	m_ModelSFX.Term();
}


void CFolderGear::UpdateInterfaceSFX()
{
	CBaseSelectionFolder::UpdateInterfaceSFX();
	if (m_ModelSFX.GetObject())
	{
        LTFLOAT spin = g_pLTClient->GetFrameTime() * m_fSFXRot;
        LTVector vU, vR, vF;
        LTRotation rRot;
        g_pLTClient->GetObjectRotation(m_ModelSFX.GetObject(), &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
        g_pLTClient->RotateAroundAxis(&rRot, &vU, spin);
        g_pLTClient->SetObjectRotation(m_ModelSFX.GetObject(),&rRot);
	}
}


void CFolderGear::SkipOutfitting()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	pData->ClearGear();
	int nGID = WMGR_INVALID_ID;
    GEAR* pGear = LTNULL;

	int nSlotsUsed = 0;

	for (int i=0; i< pMission->nNumRequiredGear; i++)
	{
		nGID = pMission->aRequiredGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		pData->AddGear(nGID);
		nSlotsUsed++;
	}
	for (i=0; i< pMission->nNumOneTimeGear; i++)
	{
		nGID = pMission->aOneTimeGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		pData->AddGear(nGID);
		nSlotsUsed++;
	}

	int nNumSlots	= pMission->nNumGear;
	if (nNumSlots == -1 || nNumSlots > MAX_SELECTION_SLOTS)
		nNumSlots = MAX_SELECTION_SLOTS;
	for (i=0; nSlotsUsed < nNumSlots && i < pMission->nNumOneTimeGear; i++)
	{
		nGID = pMission->aOneTimeGear[i];
		pGear = g_pWeaponMgr->GetGear(nGID);
		pData->AddGear(nGID);
		nSlotsUsed++;
	}

}


