// FolderPlayer.cpp: implementation of the CFolderPlayer class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderPlayer.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "NetDefs.h"
#include "MsgIds.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

extern VarTrack g_vtPlayerName;
VarTrack g_vtPlayerModel;
VarTrack g_vtPlayerTeam;
VarTrack g_vtTargetNameTransparency;
VarTrack g_vtTargetNameSize;
namespace
{
	char	szOldPlayerName[MAX_PLAYER_NAME];

	enum eLocalCommands
	{
		CMD_EDIT_NAME = FOLDER_CMD_CUSTOM+1,
		CMD_MODEL_CHANGE,
		CMD_SKIN_CHANGE,
		CMD_HEAD_CHANGE
	};

	const int kConnectSpeeds[4] = {4, 10, 20, 30};
	int nInitConnect = 0;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPlayer::CFolderPlayer()
{
    m_pEdit = LTNULL;
    m_pNameGroup = LTNULL;
    m_pLabel = LTNULL;
    m_szPlayerName[0] = LTNULL;
    m_szPlayerModel[0] = LTNULL;
    m_pModelCtrl = LTNULL;
    m_pHeadCtrl = LTNULL;
    m_pSkinCtrl = LTNULL;
	m_nModNum = 0;
	m_nSkinNum = 0;
	m_nHeadNum = 0;
	m_bRestoreSkinHead = LTTRUE;
	m_bAutoSwitchWeapons = LTFALSE;
	m_bAutoSwitchAmmo = LTFALSE;
	m_bIgnoreTaunts = LTFALSE;
	m_nTargetNameSize = 0;
	m_nTargetNameTransparency = 100;
}

CFolderPlayer::~CFolderPlayer()
{
}

// Build the folder
LTBOOL CFolderPlayer::Build()
{

	CreateTitle(IDS_TITLE_PLAYER_SETUP);

    g_vtPlayerModel.Init(g_pLTClient, "NetPlayerModel", "Hero,action", 0.0f);
    g_vtPlayerTeam.Init(g_pLTClient, "NetPlayerTeam", LTNULL, 0.0f);
    g_vtTargetNameTransparency.Init(g_pLTClient, "TargetNameTransparency", LTNULL, 1.0f);
    g_vtTargetNameSize.Init(g_pLTClient, "TargetNameSize", LTNULL, 0.0f);

	m_pLabel = CreateTextItem(IDS_PLAYER_NAME, CMD_EDIT_NAME, IDS_HELP_PLAYER_NAME);

    m_pEdit = CreateEditCtrl(" ", CMD_EDIT_NAME, LTNULL, m_szPlayerName, sizeof(m_szPlayerName), 25, LTTRUE, GetMediumFont());
	m_pEdit->EnableCursor();
    m_pEdit->Enable(LTFALSE);
	m_pEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	m_pNameGroup = AddGroup(640,m_pLabel->GetHeight(),IDS_HELP_PLAYER_NAME);

    LTIntPt offset(0,0);
    m_pNameGroup->AddControl(m_pLabel,offset,LTTRUE);
	offset.x = 200;
    m_pNameGroup->AddControl(m_pEdit,offset,LTFALSE);

	AddBlankLine();

	CToggleCtrl* pToggle = AddToggle(IDS_AUTOSWITCH_WEAPONS, IDS_HELP_AUTOSWITCH_WEAPONS, 225, &m_bAutoSwitchWeapons );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	pToggle = AddToggle(IDS_AUTOSWITCH_AMMO, IDS_HELP_AUTOSWITCH_AMMO, 225, &m_bAutoSwitchAmmo );
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

	pToggle = AddToggle(IDS_IGNORE_TAUNTS, IDS_HELP_IGNORE_TAUNTS, 225, &m_bIgnoreTaunts );
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

	CCycleCtrl *pCycle = AddCycleItem(IDS_CONNECT_SPEED,IDS_CONNECT_SPEED,200,25,&m_nConnect);
	pCycle->AddString(IDS_CONNECT_VSLOW);
	pCycle->AddString(IDS_CONNECT_SLOW);
	pCycle->AddString(IDS_CONNECT_MEDIUM);
	pCycle->AddString(IDS_CONNECT_FAST);

	pCycle = AddCycleItem(IDS_PLAYER_TEAM,IDS_HELP_PLAYER_TEAM,200,25,&m_nTeam);
	pCycle->AddString(IDS_PLAYER_EITHER);
	pCycle->AddString(IDS_PLAYER_UNITY);
	pCycle->AddString(IDS_PLAYER_HARM);

	CSliderCtrl* pSlider = AddSlider(IDS_TARGETNAMETRANSPARENCY, IDS_TARGETNAMETRANSPARENCY_HELP, 225, 200, &m_nTargetNameTransparency);
	pSlider->SetSliderRange(0, 100);
	pSlider->SetSliderIncrement(5);

	pCycle = AddCycleItem(IDS_TARGETNAMESIZE,IDS_TARGETNAMESIZE_HELP,200,25,&m_nTargetNameSize);
	pCycle->AddString(IDS_SMALL);
	pCycle->AddString(IDS_MEDIUM);
	pCycle->AddString(IDS_LARGE);

	m_pModelCtrl = AddCycleItem(IDS_PLAYER_MODEL,IDS_HELP_PLAYER_MODEL,200,25,&m_nModNum);
	m_pModelCtrl->NotifyOnChange(CMD_MODEL_CHANGE,this);

	m_pSkinCtrl = AddCycleItem(IDS_PLAYER_SKIN,IDS_HELP_PLAYER_SKIN,200,25,&m_nSkinNum);
	m_pSkinCtrl->NotifyOnChange(CMD_SKIN_CHANGE,this);

	m_pHeadCtrl = AddCycleItem(IDS_PLAYER_HEAD,IDS_HELP_PLAYER_HEAD,200,25,&m_nHeadNum);
	m_pHeadCtrl->NotifyOnChange(CMD_HEAD_CHANGE,this);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

void CFolderPlayer::Term()
{
	ClearModelList();
}

uint32 CFolderPlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{

	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		if (GetCapture())
		{
            SetCapture(LTNULL);
			m_pEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
            m_pEdit->Select(LTFALSE);
            m_pLabel->Select(LTTRUE);
			ForceMouseUpdate();
		}
		else
		{
			strcpy(szOldPlayerName,m_szPlayerName);
			SetCapture(m_pEdit);
			m_pEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
            m_pEdit->Select(LTTRUE);
            m_pLabel->Select(LTFALSE);
		}

		break;
	case CMD_MODEL_CHANGE:
		{
			UpdateData();
			HSTRING hStr = m_pModelCtrl->GetString(m_nModNum);
            strcpy(m_szPlayerModel,g_pLTClient->GetStringData(hStr));
			m_nSkinNum = 0;
			m_nHeadNum = 0;
			CreatePlayerModel();


		}break;

	case CMD_SKIN_CHANGE:
		{
			UpdateData();
            sprintf(m_szPlayerSkin, "%s,%s", g_pLTClient->GetStringData(m_pSkinCtrl->GetString(m_nSkinNum)), m_aszSkins[m_nSkinNum]);
			CreatePlayerModel(LTFALSE);


		}break;

	case CMD_HEAD_CHANGE:
		{
			UpdateData();
            sprintf(m_szPlayerHead, "%s,%s", g_pLTClient->GetStringData(m_pHeadCtrl->GetString(m_nHeadNum)), m_aszHeads[m_nHeadNum]);
			CreatePlayerModel(LTFALSE);


		}break;

	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};


void	CFolderPlayer::Escape()
{
	if (GetCapture())
	{
        SetCapture(LTNULL);
		strcpy(m_szPlayerName,szOldPlayerName);
        m_pEdit->UpdateData(LTFALSE);
		m_pEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pEdit->Select(LTFALSE);
        m_pLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else
	{
		CBaseFolder::Escape();
	}

}


// Change in focus
void    CFolderPlayer::OnFocus(LTBOOL bFocus)
{

	if (bFocus)
	{
		m_bRestoreSkinHead = LTTRUE;

		m_nModNum = -1;
		m_nSkinNum = 0;
		m_nHeadNum = 0;

		int nUpdateRate = GetConsoleInt("UpdateRate",10);
		m_nConnect = 3;
		while (m_nConnect && nUpdateRate < kConnectSpeeds[m_nConnect])
			m_nConnect--;

		nInitConnect = m_nConnect;

		SAFE_STRCPY(m_szPlayerModel,g_vtPlayerModel.GetStr() );
		GetConsoleString("NetPlayerSkin",m_szPlayerSkin,"");
		GetConsoleString("NetPlayerHead",m_szPlayerHead,"");

		if ( !m_szPlayerSkin[0] || !m_szPlayerHead[0] )
		{
			char szTemp[512];
			strcpy(szTemp, m_szPlayerModel);
			if ( strchr(szTemp, ',') )
			{
				*strchr(szTemp, ',') = '_';
				_strlwr(szTemp);
				strcpy(m_szPlayerSkin, g_pModelButeMgr->GetButeMgr()->GetString(szTemp, "Skin0"));
				strcpy(m_szPlayerHead, g_pModelButeMgr->GetButeMgr()->GetString(szTemp, "Head0"));
			} 
		}

		BuildModelList();
		SAFE_STRCPY(m_szPlayerName,g_vtPlayerName.GetStr() );

		if (m_nModNum < 0)
		{
			m_nModNum = 0;
			HSTRING hStr = m_pModelCtrl->GetString(0);
            strcpy(m_szPlayerModel,g_pLTClient->GetStringData(hStr));

		}

		m_nTeam = (int)g_vtPlayerTeam.GetFloat();
		m_nTargetNameSize = (int)g_vtTargetNameSize.GetFloat();
		m_nTargetNameTransparency = (int)(100.0f*g_vtTargetNameTransparency.GetFloat());
		m_bAutoSwitchWeapons =  (LTBOOL)GetConsoleInt("AutoWeaponSwitch",1);
		m_bAutoSwitchAmmo =  (LTBOOL)GetConsoleInt("AutoAmmoSwitch",1);
		m_bIgnoreTaunts =  (LTBOOL)GetConsoleInt("IgnoreTaunts",0);
	
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		if (nInitConnect != m_nConnect)
		{
			WriteConsoleInt("UpdateRate",kConnectSpeeds[m_nConnect]);
		}

		LTBOOL bChanged = LTFALSE;
		char szTemp[32];
		SAFE_STRCPY(szTemp,g_vtPlayerModel.GetStr() );
		if (strcmp(szTemp,m_szPlayerModel) != 0)
			bChanged = LTTRUE;


		GetConsoleString("NetPlayerSkin",szTemp,"");
		if (strcmp(szTemp,m_szPlayerSkin) != 0)
			bChanged = LTTRUE;

		GetConsoleString("NetPlayerHead",szTemp,"");
		if (strcmp(szTemp,m_szPlayerHead) != 0)
			bChanged = LTTRUE;

		SAFE_STRCPY(szTemp,g_vtPlayerName.GetStr() );
		if (strcmp(szTemp,m_szPlayerName) != 0)
			bChanged = LTTRUE;

		if (m_nTeam != (int)g_vtPlayerTeam.GetFloat())
			bChanged = LTTRUE;

		g_vtPlayerName.SetStr(m_szPlayerName);
		g_vtPlayerModel.SetStr(m_szPlayerModel);
		WriteConsoleString("NetPlayerHead",m_szPlayerHead);
		WriteConsoleString("NetPlayerSkin",m_szPlayerSkin);
        g_vtPlayerTeam.WriteFloat((LTFLOAT)m_nTeam);
        g_vtTargetNameSize.WriteFloat((LTFLOAT)m_nTargetNameSize);
        g_vtTargetNameTransparency.WriteFloat((LTFLOAT)m_nTargetNameTransparency/100.0f);
		WriteConsoleInt("AutoWeaponSwitch",(int)m_bAutoSwitchWeapons);
		WriteConsoleInt("AutoAmmoSwitch",(int)m_bAutoSwitchAmmo);
		WriteConsoleInt("IgnoreTaunts",(int)m_bIgnoreTaunts);

        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (bChanged && g_pGameClientShell->IsInWorld() && hPlayerObj && g_pGameClientShell->IsMultiplayerGame())
		{
            HSTRING hstrName = g_pLTClient->CreateString(m_szPlayerName);
			if (!hstrName) return;
            HSTRING hstrModel = g_pLTClient->CreateString(m_szPlayerModel);
			if (!hstrModel) return;
            HSTRING hstrSkin = g_pLTClient->CreateString(strchr(m_szPlayerSkin, ',')+1);
			if (!hstrSkin) return;
            HSTRING hstrHead = g_pLTClient->CreateString(strchr(m_szPlayerHead, ',')+1);
			if (!hstrHead) return;

			// Init multiplayer info on server...

            HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_PLAYER_MULTIPLAYER_CHANGE);
            g_pLTClient->WriteToMessageHString(hWrite, hstrName);
            g_pLTClient->WriteToMessageHString(hWrite, hstrModel);
            g_pLTClient->WriteToMessageHString(hWrite, hstrSkin);
            g_pLTClient->WriteToMessageHString(hWrite, hstrHead);
            g_pLTClient->WriteToMessageByte(hWrite, (uint8)m_nTeam);
            g_pLTClient->EndMessage(hWrite);

			g_pLTClient->FreeString(hstrName);
			g_pLTClient->FreeString(hstrModel);
			g_pLTClient->FreeString(hstrSkin);
			g_pLTClient->FreeString(hstrHead);

			ClearModelList();
		}

		g_pLTClient->FreeUnusedModels();

        g_pLTClient->WriteConfigFile("autoexec.cfg");

	}
	CBaseFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderPlayer::CreateInterfaceSFX
//
//	PURPOSE:	Create a BaseScaleFX to render behind the folder
//
// ----------------------------------------------------------------------- //

void	CFolderPlayer::CreateInterfaceSFX()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


	CreatePlayerModel();

	CBaseFolder::CreateInterfaceSFX();
}


void CFolderPlayer::UpdateInterfaceSFX()
{
	CBaseFolder::UpdateInterfaceSFX();
}


LTBOOL   CFolderPlayer::CreatePlayerModel(LTBOOL bNewSkin /* = LTTRUE */)
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
    if (!hCamera) return LTFALSE;

	if (!strlen(m_szPlayerModel)) return LTFALSE;
	char temp[128];
	strcpy(temp,m_szPlayerModel);
	char *pTemp = strtok(temp,",");
	strcpy(m_szModName,pTemp);
	pTemp = strtok(NULL,",");
	if (pTemp)
		strcpy(m_szStyleName,pTemp);

	if ( bNewSkin )
	{
		uint32 cSkins = 0;
		uint32 cHeads = 0;

		char szTag[128];
		char szValue[128];
		sprintf(szTag, "%s_%s", temp, m_szStyleName);
		_strlwr(szTag);

		m_pSkinCtrl->RemoveAll();
		m_pHeadCtrl->RemoveAll();

		char szRestoreSkin[256];
		char szRestoreHead[256];

		strcpy(szRestoreSkin, m_szPlayerSkin);
		*strchr(szRestoreSkin, ',') = 0;
		strcpy(szRestoreHead, m_szPlayerHead);
		*strchr(szRestoreHead, ',') = 0;

		if (g_pModelButeMgr->GetButeMgr()->Exist(szTag))
		{
			sprintf(szValue, "Skin%d", cSkins);
			while ( g_pModelButeMgr->GetButeMgr()->Exist(szTag, szValue) )
			{
				char szName[256];
				strcpy(szName, (const char*)g_pModelButeMgr->GetButeMgr()->GetString(szTag, szValue));
				char* pch = strchr(szName, ',');
				*pch++ = 0;
				strcpy(m_aszSkins[cSkins], pch);

				HSTRING hstr = g_pLTClient->CreateString(szName);
				m_pSkinCtrl->AddString(hstr);
				g_pLTClient->FreeString(hstr);

				if ( m_bRestoreSkinHead )
				{
					if ( !strcmp(szName, szRestoreSkin) )
					{
						m_nSkinNum = cSkins;
						m_pSkinCtrl->UpdateData(LTFALSE);
					}
				}

				cSkins++;
				sprintf(szValue, "Skin%d", cSkins);
			}

			sprintf(szValue, "Head%d", cHeads);
			while ( g_pModelButeMgr->GetButeMgr()->Exist(szTag, szValue) )
			{
				char szName[256];
				strcpy(szName, (const char*)g_pModelButeMgr->GetButeMgr()->GetString(szTag, szValue));
				char* pch = strchr(szName, ',');
				*pch++ = 0;
				strcpy(m_aszHeads[cHeads], pch);

				HSTRING hstr = g_pLTClient->CreateString(szName);
				m_pHeadCtrl->AddString(hstr);
				g_pLTClient->FreeString(hstr);

				if ( m_bRestoreSkinHead )
				{
					if ( !strcmp(szName, szRestoreHead) )
					{
						m_nHeadNum = cHeads;
						m_pHeadCtrl->UpdateData(LTFALSE);
					}
				}

				cHeads++;
				sprintf(szValue, "Head%d", cHeads);
			}
		}

		ASSERT(cSkins != 0 && cHeads != 0);
	}

	m_bRestoreSkinHead = LTFALSE;

	BSCREATESTRUCT bcs;
    LTVector vPos, vU, vR, vF, vTemp, vScale(1.0f,1.0f,1.0f);
    LTRotation rRot;

	char modName[128];
	char animName[128];
	char skinName[128];
	char skin2Name[128];

	if ( bNewSkin )
	{
		sprintf(m_szPlayerSkin, "%s,%s", g_pLTClient->GetStringData(m_pSkinCtrl->GetString(m_nSkinNum)), m_aszSkins[m_nSkinNum]);
		sprintf(m_szPlayerHead, "%s,%s", g_pLTClient->GetStringData(m_pHeadCtrl->GetString(m_nHeadNum)), m_aszHeads[m_nHeadNum]);
	}

	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"CharAnim",animName,sizeof(animName));

	ModelId eModelId = g_pModelButeMgr->GetModelId(m_szModName);
	ModelStyle	eModelStyle = g_pModelButeMgr->GetStyle(m_szStyleName);
	const char* pFilename = g_pModelButeMgr->GetMultiModelFilename(eModelId, eModelStyle);
	SAFE_STRCPY(modName, pFilename);
	const char* pSkin = strchr(m_szPlayerSkin, ',')+1;
	SAFE_STRCPY(skinName, pSkin);
	const char* pSkin2 = strchr(m_szPlayerHead, ',')+1;
	SAFE_STRCPY(skin2Name, pSkin2);

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_SET(vScale,1.0f,1.0f,1.0f);
	VEC_MULSCALAR(vScale, vScale, g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"CharScale"));

    LTVector vModPos = g_pLayoutMgr->GetFolderCustomVector((eFolderID)m_nFolderID,"CharPos");
    LTFLOAT fRot = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"CharRotation");
	fRot  = MATH_PI + DEG2RAD(fRot);
    g_pLTClient->RotateAroundAxis(&rRot, &vU, fRot);

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

	bcs.pFilename = modName;
	bcs.pSkin = skinName;
	bcs.pSkin2 = skin2Name;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;

	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 1.0f;
	bcs.fFinalAlpha = 1.0f;
	bcs.fLifeTime = 1000000.0f;
    bcs.bLoop = LTTRUE;


	if (m_CharSFX.Init(&bcs))
	{
        m_CharSFX.CreateObject(g_pLTClient);
		g_pInterfaceMgr->AddInterfaceSFX(&m_CharSFX, IFX_WORLD);
		if (m_CharSFX.GetObject())
		{
            HMODELANIM  hAnim = g_pLTClient->GetAnimIndex(m_CharSFX.GetObject(),animName);
			if (hAnim != -1)
                g_pLTClient->SetModelAnimation(m_CharSFX.GetObject(),hAnim);

			ClearAttachFX();

			int reqID[MAX_INT_ATTACHMENTS];
			int numReq = g_pAttachButeMgr->GetRequirementIDs(m_szModName,m_szStyleName,reqID,MAX_INT_ATTACHMENTS);
			for (int i = 0; i < numReq; i++)
			{
				INT_ATTACH acs;
				acs.fScale = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"CharScale");
				acs.nAttachmentID = g_pAttachButeMgr->GetRequirementAttachment(reqID[i]);
				CString socket = g_pAttachButeMgr->GetRequirementSocket(reqID[i]);
				acs.pszSocket = (char *)(LPCTSTR)socket;

				CreateAttachFX(&acs);
			}
			int numAtt = g_pLayoutMgr->GetFolderNumAttachments((eFolderID)m_nFolderID);
			for (i = 0; i < numAtt; i++)
			{
				char szTemp[128];
				char *pName = LTNULL;
				char *pSocket = LTNULL;
				g_pLayoutMgr->GetFolderAttachment( (eFolderID)m_nFolderID, i, szTemp, 128);

				pName = strtok(szTemp,";");
				pSocket = strtok(NULL,";");

				INT_ATTACH acs;

				acs.fScale = g_pLayoutMgr->GetFolderCustomFloat((eFolderID)m_nFolderID,"CharScale");
				acs.nAttachmentID = g_pAttachButeMgr->GetAttachmentIDByName(pName);
				acs.pszSocket = pSocket;

				CreateAttachFX(&acs);
			}

		}

        return LTTRUE;

	}

    return LTFALSE;

}

void CFolderPlayer::RemoveInterfaceSFX()
{
	CBaseFolder::RemoveInterfaceSFX();
}


void CFolderPlayer::BuildModelList()
{
    FileEntry* pFiles = g_pLTClient->GetFileList("chars\\models\\multi");
	if (!pFiles) return;

	FileEntry* ptr = pFiles;
	int num = 0;

	CMoArray<CString> stringList;
	stringList.SetSize(0);

	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			char szTemp[128];
			strcpy(szTemp,ptr->m_pBaseFilename);
			strupr(szTemp);
			if (strstr(szTemp,"_MULTI.ABC"))
			{
				int len =  strlen(szTemp);
				if (len > 10)
					szTemp[len-10] = NULL;
				char szTag[1024];
				strcpy(szTag, szTemp);
				_strlwr(szTag);
				char *pTemp = strchr(szTemp,'_');
				*pTemp = ',';

				if (g_pModelButeMgr->GetButeMgr()->Exist(szTag))
				{
					stringList.Add(CString(szTemp));
					num++;
				}
			}
		}

		ptr = ptr->m_pNext;

	}

	num = 0;
	while (stringList.GetSize())
	{
		uint8 nLowest = 0;
		uint8 nTest = 1;
		CString tmp;
		while (nTest < stringList.GetSize())
		{
			if (stringList[nTest].Compare(stringList[nLowest]) < 0)
				nLowest = nTest;
			nTest++;
		}

        HSTRING hStr = g_pLTClient->CreateString((char *)(LPCSTR)stringList[nLowest]);
		if (stricmp(m_szPlayerModel,stringList[nLowest]) == 0)
			m_nModNum = num;
		m_pModelCtrl->AddString(hStr);
        g_pLTClient->FreeString(hStr);
		num++;

		stringList.Remove(nLowest);

	}

	HSTRING hstrDefault = g_pLTClient->CreateString("Default");
	m_pSkinCtrl->AddString(hstrDefault);
	m_pHeadCtrl->AddString(hstrDefault);
	g_pLTClient->FreeString(hstrDefault);

    g_pLTClient->FreeFileList(pFiles);



}

void CFolderPlayer::ClearModelList()
{
	if (m_pModelCtrl)
		m_pModelCtrl->RemoveAll();
	if (m_pSkinCtrl)
		m_pSkinCtrl->RemoveAll();
	if (m_pHeadCtrl)
		m_pHeadCtrl->RemoveAll();
}



LTBOOL	CFolderPlayer::OnLButtonUp(int x, int y)
{

	if (GetCapture())
	{
		return m_pEdit->OnEnter();
	}
	return CBaseFolder::OnLButtonUp(x,y);
}
LTBOOL	CFolderPlayer::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}
