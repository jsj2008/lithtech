// FolderIntel.cpp: implementation of the CFolderIntel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderIntel.h"
#include "FolderMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;
namespace
{
	const int kNumSFXTypes = 14;
	int nSFXIDs[kNumSFXTypes] =
	{
		171,
		172,
		173,
		174,
		175,
		176,
		177,
		178,
		193,
		194,
		195,
		196,
		197,
		198,
	};
	int nSFXNameIDs[kNumSFXTypes] =
	{
		IDS_INTEL_LETTER,
		IDS_INTEL_FILM,
		IDS_INTEL_BLUEPRINT,
		IDS_INTEL_BRIEFCASE,
		IDS_INTEL_DOSSIER,
		IDS_INTEL_FILES,  
		IDS_INTEL_ENVELOPE,
		IDS_INTEL_TAPE,
		IDS_INTEL_NOTE,
		IDS_INTEL_TICKET,
		IDS_INTEL_SECURITY,
		IDS_INTEL_MANIFEST,
		IDS_INTEL_LOG,
		IDS_INTEL_ROSTER,
	};
	int nSFXCounts[kNumSFXTypes];
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderIntel::CFolderIntel()
{
	m_nMissionNum = -1;
	m_vScale.Init(1.0f,1.0f,1.0f);
	m_bChromakey = LTFALSE;
}

CFolderIntel::~CFolderIntel()
{
	ClearIntelList();
	CBaseSelectionFolder::Term();
}

LTBOOL CFolderIntel::Build()
{

	LTIntPt header = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"HeaderPos");
	CLTGUITextItemCtrl *pCtrl = CreateTextItem(IDS_INTEL_ITEMS,0,0,LTTRUE,GetMediumFont());
	AddFixedControl(pCtrl,header,LTFALSE);

    UseArrows(LTTRUE);
	UseBack(LTTRUE,LTFALSE,LTTRUE);

	return CBaseSelectionFolder::Build();
}

void CFolderIntel::Term()
{

}


uint32 CFolderIntel::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseSelectionFolder::OnCommand(dwCommand, dwParam1, dwParam2);
}


void CFolderIntel::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildIntelList();
		SetSelection(0);

	}
	else
	{
		m_nMissionNum = -1;
		SetSelection(kNoSelection);
		ClearIntelList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
	if (bFocus)
	{
		SetSelection(GetIndex(m_pBack));
	}
}




void CFolderIntel::BuildIntelList()
{
	if (m_nMissionNum < 0) return;
	MISSION* pMission = g_pMissionMgr->GetMission(m_nMissionNum);
	if (!pMission) return;

	CreateTitle(pMission->nNameId);


	for (int n =0;n < kNumSFXTypes;n++)
	{
		nSFXCounts[n] = 0;
	}
	CLTGUIFont *pFont;
	if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
		pFont = GetSmallFont();
	else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
		pFont = GetMediumFont();
	else
		pFont = GetLargeFont();

	int numItems = g_pGameClientShell->GetIntelItemMgr()->GetNumItems(m_nMissionNum);
	for (int i = 0; i < numItems; i++)
	{
		IntelItem *pItem = debug_new(IntelItem);
		g_pGameClientShell->GetIntelItemMgr()->GetItem(m_nMissionNum,i,pItem);
		int nNameID = 0;
		int nCount = 0;
		if (pItem->nID)
		{
			for (int n =0;!nNameID && n < kNumSFXTypes;n++)
			{
				if (pItem->nType == nSFXIDs[n])
				{
					nSFXCounts[n]++;
					nNameID = nSFXNameIDs[n];
					nCount = nSFXCounts[n];
					
				}
			}
		}
		if (nNameID)
		{
			m_intelArray.Add(pItem);
			HSTRING hStr = g_pLTClient->FormatString(nNameID,nCount);
			CLTGUITextItemCtrl *pCtrl = AddTextItem(hStr,0,0,LTFALSE,pFont);
			g_pLTClient->FreeString(hStr);
			pCtrl->SetParam1(m_intelArray.GetSize());
		}
		else
		{
			debug_delete(pItem);
		}

	}
}

void CFolderIntel::ClearIntelList()
{

	RemoveFree();
	while (m_intelArray.GetSize() > 0)
	{
		debug_delete(m_intelArray[0]);
		m_intelArray.Remove(0);
	}

}



		


LTBOOL CFolderIntel::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CLTGUICtrl *pCtrl = GetControl(m_nLastListItem);
		if (!pCtrl) return LTFALSE;
		int nIndex = pCtrl->GetParam1();
		if (!nIndex) return LTFALSE;



		IntelItem *pItem = m_intelArray[nIndex-1];
		HSTRING hStr = g_pLTClient->FormatString(pItem->nID);
		m_pDescription->SetString(hStr);
        g_pLTClient->FreeString(hStr);

		CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(pItem->nType);
		SAFE_STRCPY(m_szModel, pScaleFX->szFile);
		SAFE_STRCPY(m_szSkin, pScaleFX->szSkin);
		m_vScale = pScaleFX->vInitialScale;
		m_bChromakey = pScaleFX->bChromakey;
		
		if (pScaleFX->fDirOffset > 0.0f)
		{
			LTFLOAT fScaleMult = (100.0f/pScaleFX->fDirOffset);
			VEC_MULSCALAR(m_vScale, m_vScale, fScaleMult);
		}
		CreateModelSFX();

	}
	return bChanged;	

}


void CFolderIntel::CreateModelSFX()
{
	// no model = no SFX
	if (!strlen(m_szModel)) return;

	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


	BSCREATESTRUCT bcs;

    LTVector vPos, vU, vR, vF, vTemp;
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

    g_pLTClient->RotateAroundAxis(&rRot, &vU, MATH_HALFPI);
    g_pLTClient->RotateAroundAxis(&rRot, &vR, -0.3f);

	
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
	VEC_COPY(bcs.vInitialScale, m_vScale);
	VEC_COPY(bcs.vFinalScale, m_vScale);
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
	bcs.bChromakey = m_bChromakey;

	if (m_ModelSFX.Init(&bcs))
	{
        m_ModelSFX.CreateObject(g_pLTClient);
		g_pInterfaceMgr->AddInterfaceSFX(&m_ModelSFX, IFX_NORMAL);
		m_fSFXRot = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"ModelRotSpeed");
	}

}


void CFolderIntel::RemoveInterfaceSFX()
{
	CBaseSelectionFolder::RemoveInterfaceSFX();

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_ModelSFX);
	m_ModelSFX.Term();
}


void CFolderIntel::UpdateInterfaceSFX()
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

