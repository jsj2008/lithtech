// FolderPerformance.cpp: implementation of the CFolderPerformance class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderPerformance.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	enum eLocalCommands
	{
		CMD_SFX = FOLDER_CMD_CUSTOM+1,
	};
	int kGap = 200;
	int kWidth = 0;

	int nInitVal[6];
	int nInitTB;


	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderPerformance *pThisFolder = (CFolderPerformance *)pData;
		if (pThisFolder)
		{
			pThisFolder->ConfirmSetting(bReturn);
		}
	}
	LTBOOL g_bSettingOverall = LTFALSE;
}

typedef struct sPerformSetting_t
{
	char		szVar[32];
	int			nSetting[3];
} sPerformSetting;

const int kNumPresets = 24;

sPerformSetting sPresets[kNumPresets] =
{
	"SoundFilters",0,0,0,
	"TripleBuffer",0,0,1,
	"DrawPortals",0,0,1,
	"LightMap",0,1,1,
	"DrawShadows",0,1,1,
	"MaxModelShadows",0,1,1,
	"DetailTextures",0,1,1,
	"EnvMapWorld",0,0,1,
	"EnvMapEnable",0,1,1,
	"Trilinear",0,0,1,
	"MuzzleLight",0,1,1,
	"Tracers",0,1,1,
	"ShellCasings",0,1,1,
	"EnableWeatherFX",0,1,1,
	"ImpactFXLevel",0,1,2,
	"DebrisFXLevel",0,1,2,
	"GroupOffset1",1,0,0,
	"GroupOffset2",1,1,0,
	"GroupOffset3",1,1,0,
	"GroupOffset4",1,1,0,
	"GroupOffset5",2,1,0,
	"GroupOffset6",2,1,0,


};


extern VarTrack	g_vtUseSoundFilters;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderPerformance::CFolderPerformance()
{
	m_bSoundFilters = LTFALSE;

	m_pSoundFilters = LTNULL;

}

CFolderPerformance::~CFolderPerformance()
{

}

// Build the folder
LTBOOL CFolderPerformance::Build()
{
	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	//disable triple buffer 
	if ((dwAdvancedOptions & AO_TRIPLEBUFFER))
		sPresets[1].nSetting[2] = 1;
	else
		sPresets[1].nSetting[2] = 0;

	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_PERFORMANCE,"ColumnWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_PERFORMANCE,"SliderWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_PERFORMANCE,"SliderWidth");

	CreateTitle(IDS_TITLE_PERFORMANCE);

	m_pPerformance = AddCycleItem(IDS_OVERALL_PERFORM,IDS_HELP_OVERALL_PERFORM,kGap-25,25,&m_nOverall);
	m_pPerformance->AddString(IDS_PERFORM_HIGH);
	m_pPerformance->AddString(IDS_PERFORM_MEDIUM);
	m_pPerformance->AddString(IDS_PERFORM_LOW);
	m_pPerformance->AddString(IDS_CUSTOMIZED);


	AddTextItem(IDS_SCREEN,			FOLDER_CMD_DISPLAY,		IDS_HELP_ADVDISPLAY);
	AddTextItem(IDS_SFX,			CMD_SFX,				IDS_HELP_SFX);

	m_pSoundFilters = AddToggle(IDS_SOUNDFILTERS, IDS_HELP_SOUNDFILTERS, kGap, &m_bSoundFilters );
	m_pSoundFilters->SetOnString(IDS_ON);
	m_pSoundFilters->SetOffString(IDS_OFF);
	

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CFolderPerformance::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{

	case FOLDER_CMD_DISPLAY:
	{
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_ADVDISPLAY);
	}	break;
	case CMD_SFX:
	{
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_EFFECTS);	
	}	break;

	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CFolderPerformance::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		//disable mirrors at 16bit screen depth
		if (GetConsoleInt("BitDepth",16) == 32)
			sPresets[2].nSetting[2] = 1;
		else
			sPresets[2].nSetting[2] = 0;

		m_nOverall = GetOverall();
		g_bSettingOverall = LTTRUE;
		m_bSoundFilters = (LTBOOL)g_vtUseSoundFilters.GetFloat();
		for (int i = 0; i < 6; i++)
		{
			char szKey[16];
			sprintf(szKey,"GroupOffset%d",i+1);
			nInitVal[i] = GetConsoleInt(szKey,0);
		}
		nInitTB = GetConsoleInt("TripleBuffer",0);

        UpdateData(LTFALSE);
		g_bSettingOverall = LTFALSE;
	}
	else
	{
		LTBOOL bRebind = LTFALSE;
		UpdateData();

		g_vtUseSoundFilters.WriteFloat( (LTFLOAT)m_bSoundFilters );
		WriteConsoleInt("PerformanceLevel",m_nOverall);

		for (int i = 0; i < 6; i++)
		{
			char szKey[16];
			sprintf(szKey,"GroupOffset%d",i+1);
			if (nInitVal[i] != GetConsoleInt(szKey,0))
			{
				bRebind = LTTRUE;
			}
		}
		if (nInitTB != GetConsoleInt("TripleBuffer",0))
		{
			// Set the renderer mode
			g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
			g_pLTClient->RunConsoleString("RestartRender");
			g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);

			//no need to rebind if we restarted renderer
			bRebind = LTFALSE;
		}

		if (bRebind)
		{
            g_pLTClient->Start3D();
            g_pLTClient->StartOptimized2D();

			g_pInterfaceResMgr->DrawMessage(GetSmallFont(),IDS_REBINDING_TEXTURES);

            g_pLTClient->EndOptimized2D();
            g_pLTClient->End3D();
            g_pLTClient->FlipScreen(0);

			g_pLTClient->RunConsoleString("RebindTextures");
		}


        g_pLTClient->WriteConfigFile("autoexec.cfg");


	}
	CBaseFolder::OnFocus(bFocus);
}

void CFolderPerformance::ConfirmSetting(LTBOOL bConfirm)
{
	switch (m_nSettingToConfirm)
	{
	case IDS_CONFIRM_SOUNDFILTERS:
		if (bConfirm)
		{
			m_nOverall = 3;
			m_bSoundFilters = LTTRUE;
		}
		else
		{
			m_bSoundFilters = LTFALSE;
		}
		UpdateData(LTFALSE);
		break;
	}

}

int CFolderPerformance::GetOverall()
{
	
	LTBOOL bPotential[3] = {LTTRUE,LTTRUE,LTTRUE};
	for (int i = 0; i < kNumPresets; i++)
	{
		char szKey[32];
		strcpy(szKey,sPresets[i].szVar);
		int settting = GetConsoleInt(szKey,0);
		
		for (int j = 0; j < 3; j++)
		{
			if (bPotential[j])
			{
				if (settting != sPresets[i].nSetting[j])
				{
					bPotential[j] = LTFALSE;
//					g_pLTClient->CPrint("performance %d failed: %s (%d != %d)",j,sPresets[i].szVar,settting,sPresets[i].nSetting[j]);
				}
			}
		}
	}

	int nOverall = 0;
	while (nOverall < 3 && !bPotential[nOverall])
		nOverall++;
	return nOverall;
}

void CFolderPerformance::SetOverall(int n)
{
	//disable mirrors at 16bit screen depth
	if (GetConsoleInt("BitDepth",16) == 32)
		sPresets[2].nSetting[2] = 1;
	else
	{
		sPresets[2].nSetting[2] = 0;
	}

	if (n < 0) n = 0;
	if (n > 2) return;

	g_bSettingOverall = LTTRUE;

	for (int i = 0; i < kNumPresets; i++)
	{
		char szKey[32];
		strcpy(szKey,sPresets[i].szVar);
		WriteConsoleInt(szKey,sPresets[i].nSetting[n]);
	}
	WriteConsoleInt("PerformanceLevel",n);

	g_pLTClient->WriteConfigFile("autoexec.cfg");
	g_bSettingOverall = LTFALSE;
}

LTBOOL CFolderPerformance::OnLeft()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		--detailLevel;
		if (detailLevel < 0)
			detailLevel = 2;
		m_pPerformance->SetSelIndex(detailLevel);
		m_nOverall = detailLevel;
		if (m_nOverall != 3)
		{
			m_bSoundFilters = LTFALSE;
			SetOverall(m_nOverall);
			UpdateData(LTFALSE);
		}
        return LTTRUE;
	}
	LTBOOL bHandled = CBaseFolder::OnLeft();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pSoundFilters)
		{
			UpdateData(LTTRUE);
			if (m_bSoundFilters)
			{
				m_nSettingToConfirm = IDS_CONFIRM_SOUNDFILTERS;
				HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_SOUNDFILTERS);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,AreYouSureCallBack,this,LTFALSE,LTFALSE);
				g_pLTClient->FreeString(hString);
			}
			else
			{
				WriteConsoleInt(sPresets[0].szVar,m_bSoundFilters);
				m_nOverall = GetOverall();
				UpdateData(LTFALSE);
			}
		}
	}
	return bHandled;
}

LTBOOL CFolderPerformance::OnRight()
{
	if (GetSelectedControl() == m_pPerformance)
	{
		int detailLevel = m_pPerformance->GetSelIndex();
		++detailLevel;
		if (detailLevel > 2)
			detailLevel = 0;
		m_pPerformance->SetSelIndex(detailLevel);
		m_nOverall = detailLevel;
		if (m_nOverall != 3)
		{
			m_bSoundFilters = LTFALSE;
			SetOverall(m_nOverall);
			UpdateData(LTFALSE);
		}

		return LTTRUE;
	}

	LTBOOL bHandled = CBaseFolder::OnRight();
	if (bHandled)
	{
		if (GetSelectedControl() == m_pSoundFilters)
		{
			UpdateData(LTTRUE);
			if (m_bSoundFilters)
			{
				m_nSettingToConfirm = IDS_CONFIRM_SOUNDFILTERS;
				HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRM_SOUNDFILTERS);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,AreYouSureCallBack,this,LTFALSE,LTFALSE);
				g_pLTClient->FreeString(hString);
			}
			else
			{
				WriteConsoleInt(sPresets[0].szVar,m_bSoundFilters);
				m_nOverall = GetOverall();
				UpdateData(LTFALSE);
			}
		}
	}
	return bHandled;
	
}


LTBOOL CFolderPerformance::OnLButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance ||
			pCtrl == m_pSoundFilters)
		{
			return OnRight();
		}
	}
	return CBaseFolder::OnLButtonUp(x, y);
}

LTBOOL CFolderPerformance::OnRButtonUp(int x, int y)
{
	int nControlIndex=0;
	if (GetControlUnderPoint(x, y, &nControlIndex))
	{
		CLTGUICtrl* pCtrl = GetControl(nControlIndex);
		if (pCtrl == m_pPerformance ||
			pCtrl == m_pSoundFilters)
		{
			return OnLeft();
		}
	}
	return CBaseFolder::OnRButtonUp(x, y);
}
