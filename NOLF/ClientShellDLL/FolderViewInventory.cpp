// FolderViewInventory.cpp: implementation of the CFolderViewInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderViewInventory.h"
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
	LTBOOL bOKtoDismiss= LTFALSE;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFolderViewInventory::CFolderViewInventory()
{
	m_fSFXRot = 0.0f;
	m_nAmmo = LTNULL;
	m_nLastWeapon = WMGR_INVALID_ID;
	m_nKey = -1;
}

CFolderViewInventory::~CFolderViewInventory()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderViewInventory::Build
//
//	PURPOSE:	Build the folder
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderViewInventory::Build()
{
	CreateTitle(IDS_TITLE_INVENTORY);
	m_pFont = GetLargeFont();
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		m_pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		m_pFont = GetMediumFont();

    UseArrows(LTTRUE);
	UseBack(LTTRUE,LTFALSE,LTTRUE);

    LTBOOL success = CBaseSelectionFolder::Build();


	return success;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderViewInventory::OnFocus
//
//	PURPOSE:	Handle gaining or losing focus
//
// ----------------------------------------------------------------------- //

void CFolderViewInventory::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_nLastWeapon = WMGR_INVALID_ID;
		m_szModel[0] = '\0';
		m_szSkin[0] = '\0';
		BuildInventoryList();

		if (!m_controlArray.GetSize())
		{
			CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_NO_INVENTORY,LTNULL,LTNULL,LTTRUE,GetLargeFont());
			pCtrl->Enable(LTFALSE);
		}
		m_nKey = GetCommandKey(COMMAND_ID_INVENTORY);
		bOKtoDismiss = !IsKeyDown(m_nKey);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearInventoryList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderViewInventory::BuildInventoryList
//
//	PURPOSE:	Create the list of Inventory items
//
// ----------------------------------------------------------------------- //

void	CFolderViewInventory::BuildInventoryList()
{
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();

	m_nNumSlots	= 0;

	uint8 nID = WMGR_INVALID_ID;
    WEAPON* pWeapon = LTNULL;

	//calculate ammo
	int nNumAmmo = g_pWeaponMgr->GetNumAmmoTypes();
	if (nNumAmmo > 0)
	{
        m_nAmmo = debug_newa(int, nNumAmmo);
        memset(m_nAmmo, -1, sizeof(int) * nNumAmmo);
	}
	for (nID=0; nID < nNumAmmo; nID++)
	{
		m_nAmmo[nID] = pStats->GetAmmoCount(nID);
	}


	//add weapons
	int numWeapons = g_pWeaponMgr->GetNumWeapons();
	for (nID = 0; nID < numWeapons; nID++)
	{
		if (pStats->HaveWeapon(nID))
		{
			pWeapon = g_pWeaponMgr->GetWeapon(nID);
			if (pWeapon && !pWeapon->IsAGadget())
				AddWeapon(nID);
		}
	}

	//add gadgets
	for (nID = 0; nID < numWeapons; nID++)
	{
		if (pStats->HaveWeapon(nID))
		{
			pWeapon = g_pWeaponMgr->GetWeapon(nID);
			if (pWeapon && pWeapon->IsAGadget())
				AddWeapon(nID);
		}
	}

	//add gear
	uint8 numGear = g_pWeaponMgr->GetNumGearTypes();
	for (nID = 0; nID < numGear; nID++)
	{
		if (pStats->HaveGear(nID))
			AddGear(nID);
	}


	return;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderViewInventory::UpdateSelection
//
//	PURPOSE:	Show info based on current selection
//
// ----------------------------------------------------------------------- //

LTBOOL CFolderViewInventory::UpdateSelection()
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
					WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
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
//	ROUTINE:	CFolderViewInventory::ClearInventoryList
//
//	PURPOSE:	Remove all of the controls
//
// ----------------------------------------------------------------------- //

void CFolderViewInventory::ClearInventoryList()
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
//	ROUTINE:	CFolderViewInventory::OnCommand
//
//	PURPOSE:	Handle activation of items
//
// ----------------------------------------------------------------------- //

uint32 CFolderViewInventory::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseSelectionFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


HSTRING CFolderViewInventory::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
	return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);
}


void CFolderViewInventory::CreateModelSFX()
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


void CFolderViewInventory::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();
	g_pInterfaceMgr->RemoveInterfaceSFX(&m_Inventory);
	m_Inventory.Term();
}


void CFolderViewInventory::UpdateInterfaceSFX()
{
	CBaseSelectionFolder::UpdateInterfaceSFX();

	if (m_Inventory.GetObject())
	{
        LTFLOAT spin = g_pGameClientShell->GetFrameTime() * m_fSFXRot;
        LTVector vU, vR, vF;
        LTRotation rRot;
        g_pLTClient->GetObjectRotation(m_Inventory.GetObject(), &rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
        g_pLTClient->RotateAroundAxis(&rRot, &vU, spin);
        g_pLTClient->SetObjectRotation(m_Inventory.GetObject(),&rRot);
	}
}




void CFolderViewInventory::AddWeapon(int nID)
{

	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();

	WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nID);
	if (!pWeapon) return;

	AMMO *pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoType);
	if (!pAmmo || pAmmo->eInstDamageType == DT_MELEE) return;


	CLTGUITextItemCtrl* pCtrl = AddTextItem(pWeapon->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);

	pCtrl->SetParam1(nID);
	pCtrl->SetParam2(1000*INV_WEAPON + nID);


	for (int m = 0; m < pWeapon->nNumModTypes; m++)
	{
		int nMID = pWeapon->aModTypes[m];
		if (pStats->HaveMod(nMID))
			AddMod(nID, nMID);
	}
	for (int a = 0; a < pWeapon->nNumAmmoTypes; a++)
	{
		int nAID = pWeapon->aAmmoTypes[a];
		if (m_nAmmo[nAID] > 0)
			AddAmmo(nID, nAID,pWeapon->bInfiniteAmmo);
	}
}

void CFolderViewInventory::AddMod(int nWeaponID, int nID)
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


void CFolderViewInventory::AddAmmo(int nWeaponID, int nID, LTBOOL bInfinite)
{
	
	AMMO *pAmmo = g_pWeaponMgr->GetAmmo(nID);
	if (!pAmmo) return;


	CLTGUITextItemCtrl* pCtrl = CreateTextItem(pAmmo->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);
	CLTGUITextItemCtrl* pSpace = CreateTextItem("-",LTNULL,  LTNULL,LTFALSE,m_pFont);

	CLTGUITextItemCtrl* pCount = LTNULL;


	if (!bInfinite)
	{
		char szCount[16] = "";
		sprintf(szCount,"%d",m_nAmmo[nID]);
		pCount = CreateTextItem(szCount,LTNULL,  LTNULL,LTFALSE,m_pFont);
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


void CFolderViewInventory::AddGear(int nID)
{
	
	GEAR *pGear = g_pWeaponMgr->GetGear(nID);
	if (!pGear) return;

	CLTGUITextItemCtrl* pCtrl = AddTextItem(pGear->nNameId,LTNULL, LTNULL,LTFALSE,m_pFont);

	pCtrl->SetParam1(nID);
	pCtrl->SetParam2(1000*INV_GEAR+nID);

}

LTBOOL CFolderViewInventory::HandleKeyDown(int key, int rep)
{

	if (key == m_nKey && bOKtoDismiss)
	{
		Escape();
        return LTTRUE;
	}
	return CBaseFolder::HandleKeyDown(key, rep);
}
LTBOOL CFolderViewInventory::HandleKeyUp(int key)
{
	bOKtoDismiss = LTTRUE;
	return CBaseFolder::HandleKeyUp(key);
}
