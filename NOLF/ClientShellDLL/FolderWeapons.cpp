// FolderWeapons.cpp: implementation of the CFolderWeapons class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderWeapons.h"
#include "FolderCommands.h"
#include "MissionData.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFolderWeapons::CFolderWeapons()
{
}

CFolderWeapons::~CFolderWeapons()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::Build
//
//	PURPOSE:	Build the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderWeapons::Build()
{
	CreateTitle(IDS_TITLE_WEAPONS);
    LTBOOL success = CBaseSelectionFolder::Build();
	return success;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::IsAvailable
//
//	PURPOSE:	Check to see if there any selections to be made here
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderWeapons::IsAvailable()
{
	int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);

	return (pMission->nNumWeapons != 0);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::OnFocus
//
//	PURPOSE:	Handle gaining or losing focus
//
// ----------------------------------------------------------------------- //

void CFolderWeapons::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		UseBack(LTTRUE);
		SetContinue();
		m_szModel[0] = '\0';
		m_szSkin[0] = '\0';
		BuildWeaponsList();
		SetSelection(kNoSelection);
	}
	else
	{
		SetSelection(kNoSelection);
		SaveWeaponData();
		ClearWeaponsList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::BuildWeaponsList
//
//	PURPOSE:	Create the list of weapons
//
// ----------------------------------------------------------------------- //

void	CFolderWeapons::BuildWeaponsList()
{
	//get info from MissionMgr
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();


	m_nNumSlots	= pMission->nNumWeapons;
	if (m_nNumSlots == -1 || m_nNumSlots > MAX_SELECTION_SLOTS)
		m_nNumSlots = MAX_SELECTION_SLOTS;


	//no choices allowed... don't go here
	if (m_nNumSlots <= 0) return;

	int nWID = WMGR_INVALID_ID;
    WEAPON* pWeapon = LTNULL;
	for (int i=0; i< pMission->nNumRequiredWeapons; i++)
	{
		nWID = pMission->aRequiredWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		AddToSlot(nWID,pWeapon->nNameId,LTTRUE);
	}
	for (i=0; i< pMission->nNumOneTimeWeapons; i++)
	{
		nWID = pMission->aOneTimeWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		AddToSlot(nWID,pWeapon->nNameId,LTTRUE);
	}

	int numCurrWeapons = pData->GetNumWeapons();
	if (numCurrWeapons)
	{
		CWeaponData* currWeapons[255];

		WEAPON* pWeapon = LTNULL;

		numCurrWeapons = pData->GetWeapons(currWeapons,255);
		for (i=0; i< numCurrWeapons; i++)
		{
			nWID = currWeapons[i]->m_nID;
			pWeapon = g_pWeaponMgr->GetWeapon(nWID);
			if (pWeapon && !pWeapon->IsAGadget())
				AddToSlot(nWID,pWeapon->nNameId,LTFALSE);
		}
	}
	else
	{
		for (i=0; i< pMission->nNumDefaultWeapons; i++)
		{
			nWID = pMission->aDefaultWeapons[i];
			pWeapon = g_pWeaponMgr->GetWeapon(nWID);
			AddToSlot(nWID,pWeapon->nNameId,LTFALSE);
		}
	}


	for (i=0; i< pMission->nNumAllowedWeapons; i++)
	{
		nWID = pMission->aAllowedWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		AddItem(nWID,pWeapon->nNameId);
	}

	int numWeapons = g_pWeaponMgr->GetNumWeapons();
	for (i=0; i < numWeapons; i++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(i);
		AMMO *pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoType);
		if (pStats->CanUseWeapon(i) && !pWeapon->IsAGadget() && pAmmo->eInstDamageType != DT_MELEE)
		{
			AddItem(i,pWeapon->nNameId);
		}
	}


	for (i=0; i< pMission->nNumDeniedWeapons; i++)
	{
		nWID = pMission->aDeniedWeapons[i];
		RemoveFromSlot(nWID);
		RemoveItem(nWID);
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
				int nWID = pCtrl->GetParam1();
				WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWID);
				ItemToSlot(nWID,pWeapon->nNameId);
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
//	ROUTINE:	CFolderWeapons::UpdateSelection
//
//	PURPOSE:	Show info based on current selection
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderWeapons::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		int weaponId = pCtrl->GetParam1();
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(weaponId);
		if (pWeapon)
		{

			m_pName->RemoveAll();
			m_pName->AddString(pWeapon->nNameId);

			m_pDescription->SetString(pWeapon->nDescriptionId);

			SAFE_STRCPY(m_szModel, pWeapon->szInterfaceModel);
			SAFE_STRCPY(m_szSkin, pWeapon->szInterfaceSkin);
			VEC_COPY(m_vOffset, pWeapon->vInterfaceOffset);
			m_fScale = pWeapon->fInterfaceScale;
			CreateModelSFX();
		}
	}
	return bChanged;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::ClearWeaponsList
//
//	PURPOSE:	Remove all of the controls
//
// ----------------------------------------------------------------------- //

void CFolderWeapons::ClearWeaponsList()
{
	// Terminate the ctrls
	RemoveFree();
	ClearSlots();
	ClearSelection();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::SaveWeaponData
//
//	PURPOSE:	Save the players selections
//
// ----------------------------------------------------------------------- //

void CFolderWeapons::SaveWeaponData()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	pData->ClearWeapons();

	if (m_bSaveSelection)
	{
		for (int slot = 0; slot < m_nSlotsFilled; slot++)
		{
			CLTGUICtrl *pCtrl = GetControl(m_nFirstSlot - slot);
			if (pCtrl && (int)pCtrl->GetParam1() != kEmptySlot)
				pData->AddWeapon(pCtrl->GetParam1());
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderWeapons::OnCommand
//
//	PURPOSE:	Handle activation of items
//
// ----------------------------------------------------------------------- //

uint32 CFolderWeapons::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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
			int nWID = (int)dwParam1;
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWID);
			SlotToItem(nWID,pWeapon->nNameId);
			return 1;
		} break;
		
	case FOLDER_CMD_SELECT_ITEM:
		{
			int nWID = (int)dwParam1;
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWID);
			ItemToSlot(nWID,pWeapon->nNameId);
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


HSTRING CFolderWeapons::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
    WEAPON* pWeapon = LTNULL;
	char pStr[512] = "";

	//slots are fixed controls so count  negatively ( 0 > first > last )
	int nLastSlot = (m_nFirstSlot - m_nNumSlots) + 1;
	if (nControlIndex >= 0 || (nControlIndex <= m_nFirstSlot && nControlIndex >= nLastSlot))
	{
		CLTGUICtrl *pCtrl = GetControl(nControlIndex);
		int weaponId = pCtrl->GetParam1();
		if (weaponId == kEmptySlot) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		pWeapon = g_pWeaponMgr->GetWeapon(weaponId);
		if (!pWeapon) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		int nameId = pWeapon->nNameId;

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


void CFolderWeapons::SetContinue()
{
    int nHelp = LTNULL;
	eFolderID eNext = GetNextSelectionFolder(FOLDER_ID_WEAPONS,&nHelp);

	if (eNext != FOLDER_ID_NONE)
	{
		UseContinue(eNext,nHelp);
	}
	else
	{
		UseContinue(FOLDER_ID_MAIN,IDS_HELP_START,IDS_START_MISSION);
	}

}

void CFolderWeapons::CreateModelSFX()
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


void CFolderWeapons::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();
	g_pInterfaceMgr->RemoveInterfaceSFX(&m_ModelSFX);
	m_ModelSFX.Term();
}


void CFolderWeapons::UpdateInterfaceSFX()
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


void CFolderWeapons::SkipOutfitting()
{
	CMissionData *pData = g_pInterfaceMgr->GetMissionData();
	int missionNum = pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	pData->ClearWeapons();
	int nWID = WMGR_INVALID_ID;
    WEAPON* pWeapon = LTNULL;
	for (int i=0; i< pMission->nNumRequiredWeapons; i++)
	{
		nWID = pMission->aRequiredWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		pData->AddWeapon(nWID);
	}
	for (i=0; i< pMission->nNumOneTimeWeapons; i++)
	{
		nWID = pMission->aOneTimeWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		pData->AddWeapon(nWID);
	}

	for (i=0; i< pMission->nNumDefaultWeapons; i++)
	{
		nWID = pMission->aDefaultWeapons[i];
		pWeapon = g_pWeaponMgr->GetWeapon(nWID);
		pData->AddWeapon(nWID);
	}
}


