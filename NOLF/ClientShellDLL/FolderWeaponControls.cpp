// FolderWeaponControls.cpp: implementation of the CFolderWeaponControls class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderWeaponControls.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameSettings.h"
#include "InterfaceMgr.h"
#include "GameSettings.h"

namespace
{
	int kWidth = 200;
	int kSlider = 200;

	char szTriggers[10][2] = 
	{
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"0"
	};
	int kFirstCommand = COMMAND_ID_WEAPON_BASE;
	int kLastCommand = COMMAND_ID_WEAPON_MAX;

	uint8 nLastId = WMGR_INVALID_ID;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderWeaponControls::CFolderWeaponControls()
{
	memset(m_nActions,0,sizeof(m_nActions));
	m_bLargeIcon = LTFALSE;
	m_nIconAlpha = 0;
	m_bUseNumbers = LTFALSE;

	m_szModel[0] = NULL;
	m_szSkin[0] = NULL;
	m_fSFXRot = 0.0f;
	m_vOffset = LTVector(0.0f,0.0f,0.0f);
	m_fScale = 1.0f;


}

CFolderWeaponControls::~CFolderWeaponControls()
{

}

// Build the folder
LTBOOL CFolderWeaponControls::Build()
{
	CreateTitle(IDS_TITLE_WEAPON_CONTROLS);
	
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_WPN_CONTROLS,"ColumnWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_WPN_CONTROLS,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_WPN_CONTROLS,"SliderWidth"))
		kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_WPN_CONTROLS,"SliderWidth");

	kFirstCommand = g_pWeaponMgr->GetFirstWeaponCommandId();
	kLastCommand = g_pWeaponMgr->GetLastWeaponCommandId();



	UseBack(LTTRUE,LTTRUE);

	return CBaseFolder::Build();
}

uint32 CFolderWeaponControls::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


void CFolderWeaponControls::ReadBindings()
{
    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return;
	}
	for (int i = 0; i < 10; i++)
	{
        LTBOOL bFound = LTFALSE;
		DeviceBinding* ptr = pBindings;
		while (!bFound && ptr)
		{
			if (stricmp(ptr->strTriggerName,szTriggers[i])==0)
			{
				GameAction* pAction = ptr->pActionHead;
				if (pAction && pAction->nActionCode >= kFirstCommand && pAction->nActionCode <= kLastCommand)
				{
					m_nActions[i] = pAction->nActionCode;
					bFound = LTTRUE;
				}

			}

			ptr = ptr->pNext;
		}
		if (!bFound)
			m_nActions[i] = kFirstCommand;

	}
    g_pLTClient->FreeDeviceBindings (pBindings);

}


void CFolderWeaponControls::WriteBindings()
{
    if (!g_pLTClient) return;

    DeviceBinding* pBindings = g_pLTClient->GetDeviceBindings (DEVICETYPE_KEYBOARD);
	if (!pBindings)
	{
		return;
	}
	DeviceBinding* ptr = pBindings;
	char action[32];
	char str[128];
	for (int i = 0; i < 10; i++)
	{
//		sprintf(str, "rangebind \"%s\" \"%s\" 0 0 \"\"", ptr->strDeviceName, szTrigger[i]);
//      g_pLTClient->RunConsoleString(str);
		int nWpnNum = 1+m_nActions[i]-kFirstCommand;
		if (nWpnNum == 10) nWpnNum = 0;

		sprintf(action,"Weapon_%d",nWpnNum);
		if (i < 9)
			sprintf(str, "rangebind \"%s\" \"%s\" 0 0 \"%s\"", ptr->strDeviceName, szTriggers[i], action);
		else
			sprintf(str, "rangebind \"%s\" \"##11\" 0 0 \"%s\"", ptr->strDeviceName, action);
			
        g_pLTClient->RunConsoleString(str);
	}
}


// Change in focus
void CFolderWeaponControls::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		nLastId = WMGR_INVALID_ID;
		ReadBindings();		
		CLTGUIColumnTextCtrl *pCtrl=AddColumnText(LTNULL, LTNULL, LTTRUE, GetMediumFont());

		HSTRING hText = g_pLTClient->FormatString(IDS_KEY);
		pCtrl->AddColumn(hText, 50, LTF_JUSTIFY_LEFT);
		g_pLTClient->FreeString(hText);

		hText = g_pLTClient->FormatString(IDS_WEAPON);
		pCtrl->AddColumn(hText, kWidth, LTF_JUSTIFY_LEFT);
		pCtrl->Enable(LTFALSE);
		g_pLTClient->FreeString(hText);

//		AddBlankLine();


		for (int i = 0; i < 10; i++)
		{
			pCtrl=AddColumnText(LTNULL, IDS_HELP_SETWEAPON, LTFALSE, GetSmallFont());
			pCtrl->SetParam1(i+1);

			char str[16];
			sprintf(str,"  %s",szTriggers[i]);
			hText = g_pLTClient->CreateString(str);
			pCtrl->AddColumn(hText, 50, LTF_JUSTIFY_LEFT);
			g_pLTClient->FreeString(hText);

			int nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[i]);
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (pWeapon)
				pCtrl->AddColumn(pWeapon->nNameId, kWidth, LTF_JUSTIFY_LEFT);
			else
				pCtrl->AddColumn(IDS_CONTROL_UNASSIGNED, kWidth, LTF_JUSTIFY_LEFT);


		}

		AddBlankLine();

		m_bLargeIcon = (GetConsoleInt("BindingIconSize",0) > 0);
		CToggleCtrl *pToggle = AddToggle(IDS_WPN_ICON_SZ,IDS_HELP_WPN_ICON_SZ, kWidth, &m_bLargeIcon,LTFALSE,GetSmallFont());
		pToggle->SetOnString(IDS_LARGE);
		pToggle->SetOffString(IDS_SMALL);

		m_nIconAlpha = (int)(GetConsoleFloat("BindingIconAlpha",0.7f) * 10.0f);
		CSliderCtrl *pSlider = AddSlider(IDS_WPN_ICON_A,IDS_HELP_WPN_ICON_A,kWidth,kSlider,&m_nIconAlpha,LTFALSE,GetSmallFont());
		pSlider->SetSliderIncrement(1);
		pSlider->SetSliderRange(0,10);

		m_bUseNumbers = (GetConsoleInt("BindingNumbers",1) > 0);
		pToggle = AddToggle(IDS_WPN_USE_NUMS,IDS_HELP_WPN_USE_NUMS, kWidth, &m_bUseNumbers,LTFALSE,GetSmallFont());
		pToggle->SetOnString(IDS_YES);
		pToggle->SetOffString(IDS_NO);


        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		WriteConsoleInt("BindingIconSize",m_bLargeIcon);
		WriteConsoleInt("BindingNumbers",m_bUseNumbers);
		WriteConsoleFloat("BindingIconAlpha",((LTFLOAT)m_nIconAlpha) / 10.0f);
		WriteBindings();

		g_pInterfaceMgr->GetPlayerStats()->UpdateWeaponBindings();

		// Just to be safe save the config incase anything changed...
        g_pLTClient->WriteConfigFile("autoexec.cfg");
		RemoveFree();

	}
	CBaseFolder::OnFocus(bFocus);
	if (bFocus) UpdateSelection();

}

LTBOOL	CFolderWeaponControls::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->GetParam1())
	{
		CLTGUIColumnTextCtrl *pColumn=(CLTGUIColumnTextCtrl *)pCtrl;
		int nAct = pCtrl->GetParam1()-1;
		m_nActions[nAct] = GetPreviousCommand(m_nActions[nAct]);

		int nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[nAct]);
		WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon)
			pColumn->SetString(1,pWeapon->nNameId);
		else
			pColumn->SetString(1,IDS_CONTROL_UNASSIGNED);
		return LTTRUE;
	}
	return CBaseFolder::OnLeft();
}

LTBOOL	CFolderWeaponControls::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl && pCtrl->GetParam1())
	{
		CLTGUIColumnTextCtrl *pColumn=(CLTGUIColumnTextCtrl *)pCtrl;
		int nAct = pCtrl->GetParam1()-1;
		m_nActions[nAct] = GetNextCommand(m_nActions[nAct]);

		int nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[nAct]);
		WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon)
			pColumn->SetString(1,pWeapon->nNameId);
		else
			pColumn->SetString(1,IDS_CONTROL_UNASSIGNED);
		return LTTRUE;
	}
	return CBaseFolder::OnRight();
}

LTBOOL	CFolderWeaponControls::OnLButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl && pCtrl->GetParam1())
		{
			CLTGUIColumnTextCtrl *pColumn=(CLTGUIColumnTextCtrl *)pCtrl;
			int nAct = pCtrl->GetParam1()-1;
			m_nActions[nAct] = GetNextCommand(m_nActions[nAct]);

			int nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[nAct]);
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (pWeapon)
				pColumn->SetString(1,pWeapon->nNameId);
			else
				pColumn->SetString(1,IDS_CONTROL_UNASSIGNED);
			UpdateSelection();
			return LTTRUE;
		}
	}
	return CBaseFolder::OnLButtonUp(x,y);

}

LTBOOL	CFolderWeaponControls::OnRButtonUp(int x, int y)
{
	// Get the control that the click was on
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl && pCtrl->GetParam1())
		{
			CLTGUIColumnTextCtrl *pColumn=(CLTGUIColumnTextCtrl *)pCtrl;
			int nAct = pCtrl->GetParam1()-1;
			m_nActions[nAct] = GetPreviousCommand(m_nActions[nAct]);

			int nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[nAct]);
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if (pWeapon)
				pColumn->SetString(1,pWeapon->nNameId);
			else
				pColumn->SetString(1,IDS_CONTROL_UNASSIGNED);
			UpdateSelection();
			return LTTRUE;
		}
	}
	return CBaseFolder::OnRButtonUp(x, y);
}


int CFolderWeaponControls::GetNextCommand(int nCommand)
{
	LTBOOL bOK = LTFALSE;
	int nInit = nCommand;
	while (!bOK)
	{
		nCommand++;
		if (nCommand > kLastCommand)
			nCommand = kFirstCommand;
		int nWeaponId = g_pWeaponMgr->GetWeaponId(nCommand);
		WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon)
			bOK = pWeapon->bCanBeMapped;
		if (nCommand == nInit) bOK = LTFALSE;
	} 
	return nCommand;
}
int CFolderWeaponControls::GetPreviousCommand(int nCommand)
{
	LTBOOL bOK = LTFALSE;
	int nInit = nCommand;
	while (!bOK)
	{
		nCommand--;
		if (nCommand < kFirstCommand)
			nCommand = kLastCommand;
		int nWeaponId = g_pWeaponMgr->GetWeaponId(nCommand);
		WEAPON *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon)
			bOK = pWeapon->bCanBeMapped;
		if (nCommand == nInit) bOK = LTFALSE;
	} 
	return nCommand;
}


// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CFolderWeaponControls::HandleKeyDown(int key, int rep)
{
	if (CBaseFolder::HandleKeyDown(key,rep))
	{
		UpdateSelection();
        return LTTRUE;
	}
    return LTFALSE;

}


void CFolderWeaponControls::UpdateSelection()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (!pCtrl) return;

	int nComm = pCtrl->GetParam1();
	if (!nComm) return;

	uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(m_nActions[nComm-1]);
	
	if (nWeaponId != nLastId)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon)
		{

			SAFE_STRCPY(m_szModel, pWeapon->szInterfaceModel);
			SAFE_STRCPY(m_szSkin, pWeapon->szInterfaceSkin);
			VEC_COPY(m_vOffset, pWeapon->vInterfaceOffset);
			m_fScale = pWeapon->fInterfaceScale;
			CreateModelSFX();
		}
		nLastId = nWeaponId;
	}

}


void CFolderWeaponControls::CreateModelSFX()
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


void CFolderWeaponControls::RemoveInterfaceSFX()
{
	CBaseFolder::RemoveInterfaceSFX();

	g_pInterfaceMgr->RemoveInterfaceSFX(&m_ModelSFX);
	m_ModelSFX.Term();
}


void CFolderWeaponControls::UpdateInterfaceSFX()
{
	CBaseFolder::UpdateInterfaceSFX();
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

LTBOOL CFolderWeaponControls::OnMouseMove(int x, int y)
{
	if (CBaseFolder::OnMouseMove(x,y))
	{
		UpdateSelection();
        return LTTRUE;
	}
    return LTFALSE;
}
