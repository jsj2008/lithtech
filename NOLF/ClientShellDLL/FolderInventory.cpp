// FolderInventory.cpp: implementation of the CFolderInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderInventory.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
	enum InvType
	{
		INV_WEAPON,
		INV_AMMO,
		INV_MOD,
		INV_GEAR
	};
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFolderInventory::CFolderInventory()
{
	m_fSFXRot = 0.0f;
	m_nAmmo = LTNULL;
	m_pData = LTNULL;
	m_nLastWeapon = WMGR_INVALID_ID;
	m_pMissionCtrl = LTNULL;
}

CFolderInventory::~CFolderInventory()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::Build
//
//	PURPOSE:	Build the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderInventory::Build()
{
	CreateTitle(IDS_TITLE_INVENTORY);
	m_pData = g_pInterfaceMgr->GetMissionData();
	m_pFont = GetLargeFont();
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		m_pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		m_pFont = GetMediumFont();

	m_pMissionCtrl = CreateTextItem( IDS_START_MISSION, FOLDER_CMD_CONTINUE, IDS_HELP_START);
	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_INVENTORY,"StartPos");
	AddFixedControl(m_pMissionCtrl,pos);

    LTBOOL success = CBaseSelectionFolder::Build();
	return success;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::OnFocus
//
//	PURPOSE:	Handle gaining or losing focus
//
// ----------------------------------------------------------------------- //

void CFolderInventory::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		UseBack(LTTRUE);
		UseContinue(LTFALSE);
		m_nLastWeapon = WMGR_INVALID_ID;
		m_szModel[0] = '\0';
		m_szSkin[0] = '\0';
		BuildInventoryList();
		if (!m_controlArray.GetSize())
		{
			CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_NO_INVENTORY,LTNULL,LTNULL,LTTRUE,GetLargeFont());
			pCtrl->Enable(LTFALSE);
		}

		SetSelection(kNoSelection);
	}
	else
	{
		SetSelection(kNoSelection);
		SaveInventoryData();
		ClearInventoryList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
	SetSelection(GetIndex(m_pMissionCtrl));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::BuildInventoryList
//
//	PURPOSE:	Create the list of Inventory items
//
// ----------------------------------------------------------------------- //

void	CFolderInventory::BuildInventoryList()
{
	//get info from MissionMgr
	int missionNum = m_pData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();


	m_nNumSlots	= 0;

	int nID = WMGR_INVALID_ID;
    WEAPON* pWeapon = LTNULL;

	CWeaponData* currWeapons[MAX_SELECTION_SLOTS*2];

	//calculate ammo
	m_pData->ClearAllAmmo();
	int nNumAmmo = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmo > 0)
	{
        m_nAmmo = debug_newa(int, nNumAmmo);
        memset(m_nAmmo, -1, sizeof(int) * nNumAmmo);
	}
	for (nID=0; nID < nNumAmmo; nID++)
	{
		if (pStats->CanUseAmmo(nID))
			m_nAmmo[nID] = 0;
	}
	for (int i=0; i < pMission->nNumAllowedAmmo; i++)
	{
		nID = pMission->aAllowedAmmo[i];
		m_nAmmo[nID] = 0;
	}
	for (i=0; i < pMission->nNumRequiredAmmo; i++)
	{
		nID = pMission->aRequiredAmmo[i];
		m_nAmmo[nID] = 0;
	}
	for (i=0; i < pMission->nNumOneTimeAmmo; i++)
	{
		nID = pMission->aOneTimeAmmo[i];
		m_nAmmo[nID] = 0;
	}
	for (i=0; i < pMission->nNumDeniedAmmo; i++)
	{
		nID = pMission->aDeniedAmmo[i];
		m_nAmmo[nID] = -1;
	}

	int numCurrWeapons = m_pData->GetWeapons(currWeapons,MAX_SELECTION_SLOTS*2);
	for (i=0; i< numCurrWeapons; i++)
	{
		nID = currWeapons[i]->m_nID;
		CalculateAmmo(nID);
	}


	//add weapons
	for (i=0; i< numCurrWeapons; i++)
	{
		nID = currWeapons[i]->m_nID;
		pWeapon = g_pWeaponMgr->GetWeapon(nID);
		if (pWeapon && !pWeapon->IsAGadget())
			AddWeapon(nID);
	}

	//add gadgets
	for (i=0; i< numCurrWeapons; i++)
	{
		nID = currWeapons[i]->m_nID;
		pWeapon = g_pWeaponMgr->GetWeapon(nID);
		if (pWeapon && pWeapon->IsAGadget())
			AddWeapon(nID);
	}

	//add gear
	CGearData* currGear[MAX_SELECTION_SLOTS];
	int numCurrGear = m_pData->GetGear(currGear,MAX_SELECTION_SLOTS);
	for (i=0; i< numCurrGear; i++)
	{
		nID = currGear[i]->m_nID;
		AddGear(nID);
	}


	return;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::UpdateSelection
//
//	PURPOSE:	Show info based on current selection
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderInventory::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		int nId = pCtrl->GetParam1();
		int nType = pCtrl->GetParam2() / 1000;
		int nWeaponId = pCtrl->GetParam2() % 1000;
		InvType eType = (InvType)nType;
		switch (eType)
		{
		case INV_WEAPON:
			{
				m_nLastWeapon = nWeaponId;
				WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
				if (pWeapon)
				{
					m_pName->RemoveAll();
					m_pName->AddString(pWeapon->nNameId);

					SetPhotoBitmap(pWeapon->szIcon);

					m_pDescription->SetString(pWeapon->nDescriptionId);

					SAFE_STRCPY(m_szModel, pWeapon->szInterfaceModel);
					SAFE_STRCPY(m_szSkin, pWeapon->szInterfaceSkin);
					m_fScale = pWeapon->fInterfaceScale;
					CreateModelSFX();
					
				}
			} break;
		case INV_MOD:
			{
				WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
				if (nWeaponId != m_nLastWeapon)
				{
					m_nLastWeapon = nWeaponId;
					if (pWeapon)
					{
						SAFE_STRCPY(m_szModel, pWeapon->szInterfaceModel);
						SAFE_STRCPY(m_szSkin, pWeapon->szInterfaceSkin);
						m_fScale = pWeapon->fInterfaceScale;
						CreateModelSFX();
					}
				}
				MOD* pMod = g_pWeaponMgr->GetMod(nId);
				if (pMod)
				{
					m_pName->RemoveAll();
					m_pName->AddString(pMod->nDescriptionId);

					SetPhotoBitmap(pMod->szIcon);

					int descId = 0;

					switch (pMod->eType)
					{
						case SILENCER:
						{
							descId = IDS_SILENCER;
						}
						break;
						case LASER:
						{
							descId = IDS_LASER;
						}
						break;
						case SCOPE:
						{
							if (pMod->bNightVision) 
							{
								descId = IDS_NIGHT_SCOPE;
							}
							else if (pWeapon && pWeapon->IsAGadget())
							{
								descId = IDS_CAMERA_SCOPE;
							}
							else 
							{
								descId = IDS_SCOPE;
							}
						}
						break;
					}
					if (descId)
					{
						m_pDescription->SetString(descId);
					}
					else
					{
						m_pDescription->RemoveString();
					}
				}
			} break;
		case INV_AMMO:
			{
				if (nWeaponId != m_nLastWeapon)
				{
					m_nLastWeapon = nWeaponId;
					WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
					if (pWeapon)
					{
						SAFE_STRCPY(m_szModel, pWeapon->szInterfaceModel);
						SAFE_STRCPY(m_szSkin, pWeapon->szInterfaceSkin);
						m_fScale = pWeapon->fInterfaceScale;
						CreateModelSFX();
					}
				}
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nId);
				if (pAmmo)
				{

					m_pName->RemoveAll();
					m_pName->AddString(pAmmo->nNameId);

					SetPhotoBitmap(pAmmo->szIcon);

					m_pDescription->SetString(pAmmo->nDescId);

				}
			} break;

		case INV_GEAR:
			{
				GEAR* pGear = g_pWeaponMgr->GetGear(nId);
				if (pGear)
				{
					m_nLastWeapon = WMGR_INVALID_ID;

					m_pName->RemoveAll();
					m_pName->AddString(pGear->nNameId);

					SetPhotoBitmap(pGear->szIcon);

					m_pDescription->SetString(pGear->nDescriptionId);

					SAFE_STRCPY(m_szModel, pGear->szInterfaceModel);
					SAFE_STRCPY(m_szSkin, pGear->szInterfaceSkin);
					m_fScale = pGear->fInterfaceScale;
					CreateModelSFX();
				}
			} break;
		}

	}
	return bChanged;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::ClearInventoryList
//
//	PURPOSE:	Remove all of the controls
//
// ----------------------------------------------------------------------- //

void CFolderInventory::ClearInventoryList()
{
	// Terminate the ctrls
	RemoveFree();
	ClearSlots();
	ClearSelection();
	if (m_nAmmo)
	{
		debug_deletea(m_nAmmo);
		m_nAmmo = LTNULL;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::SaveInventoryData
//
//	PURPOSE:	Save the players selections
//
// ----------------------------------------------------------------------- //

void CFolderInventory::SaveInventoryData()
{
	
	m_pData->ClearAmmo();

	if (m_bSaveSelection)
	{
		m_pData->ClearAllAmmo();
		int nNumAmmo = g_pWeaponMgr->GetNumAmmoTypes();
		for (int i = 0; i < nNumAmmo; i++)
		{
			if (m_nAmmo[i] > 0)
				m_pData->AddAmmo(i,m_nAmmo[i]);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderInventory::OnCommand
//
//	PURPOSE:	Handle activation of items
//
// ----------------------------------------------------------------------- //

uint32 CFolderInventory::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch (dwCommand)
	{
	case FOLDER_CMD_CONTINUE:
		{
			int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
			g_pGameClientShell->StartMission(missionNum);

			return 1;
		} break;
	}

	return CBaseSelectionFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


HSTRING CFolderInventory::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
	char pStr[512] = "";
/*
	if (nControlIndex >= 0)
	{
		CLTGUICtrl *pCtrl = GetControl(nControlIndex);
		int gearId = pCtrl->GetParam1();
		if (gearId == kEmptySlot) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		pInventory = g_pWeaponMgr->GetInventory(gearId);
		if (!pInventory) return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);

		int nameId = pInventory->nNameId;

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
*/
		return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);
}


void CFolderInventory::CreateModelSFX()
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

	if (m_Inventory.Init(&bcs))
	{
        m_Inventory.CreateObject(g_pLTClient);
		g_pInterfaceMgr->AddInterfaceSFX(&m_Inventory, IFX_NORMAL);
		m_fSFXRot = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"ModelRotSpeed");
	}

}


void CFolderInventory::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();
	g_pInterfaceMgr->RemoveInterfaceSFX(&m_Inventory);
	m_Inventory.Term();
}


void CFolderInventory::UpdateInterfaceSFX()
{
	CBaseSelectionFolder::UpdateInterfaceSFX();

	if (m_Inventory.GetObject())
	{
        LTFLOAT spin = g_pLTClient->GetFrameTime() * m_fSFXRot;
        LTVector vU, vR, vF;
        LTRotation rRot;
        g_pLTClient->GetObjectRotation(m_Inventory.GetObject(), &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
        g_pLTClient->RotateAroundAxis(&rRot, &vU, spin);
        g_pLTClient->SetObjectRotation(m_Inventory.GetObject(),&rRot);
	}
}


void CFolderInventory::CalculateAmmo(int nWeaponID)
{
	WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	for (int a = 0; a < pWeapon->nNumAmmoTypes; a++)
	{
		int nID = pWeapon->aAmmoTypes[a];

		//if it is the weapon's default ammo, always select some
		if (a==0 &&  m_nAmmo[nID] < 0) 
			m_nAmmo[nID] = 0;

		if (m_nAmmo[nID] >= 0)
		{
			AMMO *pAmmo = g_pWeaponMgr->GetAmmo(nID);
			int nAmount = pAmmo->nSelectionAmount * pWeapon->nAmmoMultiplier;
			int nMax = pAmmo->GetMaxAmount(LTNULL);
			m_nAmmo[nID] += nAmount;
			if (m_nAmmo[nID] > nMax)
				m_nAmmo[nID] = nMax;

		}

	}
}

void CFolderInventory::AddWeapon(int nID)
{
	
	WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nID);
	if (!pWeapon) return;

	CLTGUITextItemCtrl* pCtrl = AddTextItem(pWeapon->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);

	pCtrl->SetParam1(nID);
	pCtrl->SetParam2(1000*INV_WEAPON + nID);


	for (int m = 0; m < pWeapon->nNumModTypes; m++)
	{
		int nMID = pWeapon->aModTypes[m];
		CModData *pMData = m_pData->GetModData(nMID);
		if (pMData)
			AddMod(nID, nMID);
	}
	for (int a = 0; a < pWeapon->nNumAmmoTypes; a++)
	{
		int nAID = pWeapon->aAmmoTypes[a];
		if (m_nAmmo[nAID] > 0)
			AddAmmo(nID, nAID,pWeapon->bInfiniteAmmo);
	}
}

void CFolderInventory::AddMod(int nWeaponID, int nID)
{
	
	MOD *pMod = g_pWeaponMgr->GetMod(nID);
	if (!pMod) return;

	CLTGUITextItemCtrl* pCtrl = CreateTextItem(pMod->nDescriptionId,LTNULL, LTNULL,LTFALSE,m_pFont);
	CLTGUITextItemCtrl* pSpace = CreateTextItem("-",LTNULL, LTNULL,LTFALSE,m_pFont);

	int nIndent = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"Indent");
	LTIntPt offset(nIndent,0);
	int nWidth = (m_ListRect.right - m_ListRect.left);
	int nHeight = pCtrl->GetHeight() + 2;
	CGroupCtrl*	pGroup = CreateGroup(nWidth,nHeight,LTNULL);

	offset.y = (nHeight - pSpace->GetHeight()) / 2;
	pGroup->AddControl(pSpace,offset);

	offset.x = nIndent + pSpace->GetWidth() + 8;
	offset.y = (nHeight - pCtrl->GetHeight()) / 2;
	pGroup->AddControl(pCtrl,offset,LTTRUE);

	pGroup->SetParam1(nID);
	pGroup->SetParam2(1000*INV_MOD+nWeaponID);

	AddFreeControl(pGroup);

}


void CFolderInventory::AddAmmo(int nWeaponID, int nID, LTBOOL bInfinite)
{
	
	AMMO *pAmmo = g_pWeaponMgr->GetAmmo(nID);
	if (!pAmmo) return;


	CLTGUITextItemCtrl* pCtrl = CreateTextItem(pAmmo->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);
	CLTGUITextItemCtrl* pSpace = CreateTextItem("-",LTNULL, LTNULL,LTFALSE,m_pFont);

	CLTGUITextItemCtrl* pCount = LTNULL;


	if (!bInfinite)
	{
		char szCount[16] = "";
		sprintf(szCount,"%d",m_nAmmo[nID]);
		pCount = CreateTextItem(szCount,LTNULL, LTNULL,LTFALSE,m_pFont);
	}


	int nIndent = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"Indent");
	LTIntPt offset(nIndent,0);
	int nWidth = (m_ListRect.right - m_ListRect.left);
	int nHeight = pCtrl->GetHeight() + 2;
	CGroupCtrl*	pGroup = CreateGroup(nWidth,nHeight,LTNULL);

	offset.y = (nHeight - pSpace->GetHeight()) / 2;
	pGroup->AddControl(pSpace,offset);
	offset.x += pSpace->GetWidth() + 8;

	if (!bInfinite)
	{
		
		offset.y = (nHeight - pCtrl->GetHeight()) / 2;
		pGroup->AddControl(pCount,offset,LTTRUE);
		offset.x += pCount->GetWidth() + 8;

	}


	offset.y = (nHeight - pCtrl->GetHeight()) / 2;
	pGroup->AddControl(pCtrl,offset,LTTRUE);

	pGroup->SetParam1(nID);
	pGroup->SetParam2(1000*INV_AMMO+nWeaponID);

	AddFreeControl(pGroup);

}


void CFolderInventory::AddGear(int nID)
{
	
	GEAR *pGear = g_pWeaponMgr->GetGear(nID);
	if (!pGear) return;

	CLTGUITextItemCtrl* pCtrl = AddTextItem(pGear->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);

	pCtrl->SetParam1(nID);
	pCtrl->SetParam2(1000*INV_GEAR+nID);

}
