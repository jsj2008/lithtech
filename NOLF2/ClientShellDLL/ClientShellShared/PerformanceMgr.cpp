// ----------------------------------------------------------------------- //
//
// MODULE  : PerformanceMgr.cpp
//
// PURPOSE : Manage performance related settings
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "PerformanceMgr.h"
#include "interfacemgr.h"

#include <iostream>			// For input and output
#include <fstream>			// For the files
#include <IO.h>				// Find first, find next, etc.


namespace
{
}
	const std::string CONFIG_DIR("Config\\");
	const std::string CONFIG_EXT(".txt");

	sDetailSetting sTextureGroups[kNumTextureGroups] =
	{
		"GroupOffset1","RegularTex",	1,0,0,
		"GroupOffset2","LargeTex",		1,1,0,
		"GroupOffset3","XLargeTex",		2,1,0,
	};

	sDetailSetting sShadowDetails[] =
	{
		{"modelshadow_proj_blurenable","ShadowBlending",0,1,1},
		{"modelshadow_proj_textureres","ShadowTextureRes",128,256,512},
		{"modelshadow_proj_backfacecull","ShadowBackfaceCull",0,0,1},
		{"modelshadow_proj_maxshadowsperframe","ShadowMaxCount",1,2,3},
	};
	uint8 kNumShadowDetails = ARRAY_LEN( sShadowDetails );

	sDetailSetting sFXDetails[] =
	{
		{"ClientFXDetailLevel","ClientFX",0,1,2},
		{"DebrisFXLevel","DebrisFX",0,1,2},
	};
	uint8 kNumFXDetails = ARRAY_LEN( sFXDetails );



	sDetailSetting sSettings[kNumDetailSettings] =
	{
#define INCLUDE_AS_SETTING
#include "PerformanceEnum.h"
#undef INCLUDE_AS_SETTING
	};


	sPerformCfg cfgDefaultLow =
	{
		".DefaultLow",
		"",
		1024,
		1600,
#define INCLUDE_AS_LOW
#include "PerformanceEnum.h"
#undef INCLUDE_AS_LOW
	};

	sPerformCfg cfgDefaultMid =
	{
		".DefaultMid",
		"",
		800,
		800,
#define INCLUDE_AS_MED
#include "PerformanceEnum.h"
#undef INCLUDE_AS_MED
	};

	sPerformCfg cfgDefaultHigh =
	{
		".DefaultHigh",
		"",
		640,
		640,
#define INCLUDE_AS_HIGH
#include "PerformanceEnum.h"
#undef INCLUDE_AS_HIGH
	};

CPerformanceMgr* g_pPerformanceMgr = NULL;

LTBOOL CPerformanceMgr::Init()
{
	g_pPerformanceMgr = this;

	LoadString(IDS_PERFORM_HIGH,cfgDefaultHigh.szNiceName,sizeof(cfgDefaultHigh.szNiceName));
	LoadString(IDS_PERFORM_MEDIUM,cfgDefaultMid.szNiceName,sizeof(cfgDefaultHigh.szNiceName));
	LoadString(IDS_PERFORM_LOW,cfgDefaultLow.szNiceName,sizeof(cfgDefaultHigh.szNiceName));


//	char szCfg[64];
//	GetConsoleString("PerformanceConfig",szCfg,".DefaultLow");

	BuildConfigList();

	SetPerformanceCfg(0);

	return LTTRUE;

}

void CPerformanceMgr::Term()
{
	unsigned int i;
	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();

	g_pPerformanceMgr = LTNULL;
}


int CPerformanceMgr::GetPerformanceCfg(bool bIgnoreResolution)
{
	uint32 nCfg = 0;

	while (nCfg < m_ConfigList.size() && !IsCurrentConfig(nCfg,bIgnoreResolution))
		nCfg++;

	return nCfg;
}


void CPerformanceMgr::SetPerformanceCfg(char *szName)
{
	uint32 i = 0;

	while (i < m_ConfigList.size() && stricmp(m_ConfigList[i]->szName,szName) != 0)
		i++;


	SetPerformanceCfg(i);
}


void CPerformanceMgr::SetPerformanceCfg(int nCfg)
{
	if (nCfg < 0 || (uint32)nCfg >= m_ConfigList.size())
	{
		WriteConsoleString("PerformanceConfig",".CustomConfig");
//		g_pLTClient->WriteConfigFile("autoexec.cfg");
		return;
	}

	sPerformCfg *pCfg = m_ConfigList[nCfg];

	SetPerformanceOptions(pCfg,nCfg);

}

void CPerformanceMgr::BuildConfigList()
{
	unsigned int i;

	if (m_ConfigList.size() >= 3)
	{
		m_ConfigList.erase(m_ConfigList.begin());
		m_ConfigList.erase(m_ConfigList.begin());
		m_ConfigList.erase(m_ConfigList.begin());
	};
	for (i=0; i < m_ConfigList.size(); i++)
	{
		delete m_ConfigList[i];
	}
	m_ConfigList.clear();

	m_ConfigList.push_back(&cfgDefaultHigh);
	m_ConfigList.push_back(&cfgDefaultMid);
	m_ConfigList.push_back(&cfgDefaultLow);

	struct _finddata_t file;
	long hFile;

	std::string directory = CONFIG_DIR + "*" + CONFIG_EXT;

	// find first file
	if((hFile = _findfirst(directory.c_str(), &file)) != -1L)
	{
		do
		{
			std::string fn = CONFIG_DIR + file.name;

			m_buteMgr.Term();
			if (m_buteMgr.Parse(fn.c_str()))
			{
				sPerformCfg* pCfg = new sPerformCfg;
				char *pName = strtok(file.name,".");
				SAFE_STRCPY(pCfg->szName,pName);

				m_buteMgr.GetString("General","Name",pCfg->szNiceName,sizeof(pCfg->szNiceName));

				LoadPerformanceOptions(m_buteMgr,pCfg);

				m_ConfigList.push_back(pCfg);
			}
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);
		

}


LTBOOL	CPerformanceMgr::IsCurrentConfig(int nCfg,bool bIgnoreResolution)
{
	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	if (nCfg < 0 || (uint32)nCfg >= m_ConfigList.size())
	{
		return LTFALSE;
	}

	sPerformCfg *pCfg = m_ConfigList[nCfg];

	if (!bIgnoreResolution)
	{
		uint16 nScreenSize = (uint16)GetConsoleInt("screenwidth",640);
		if (nScreenSize < pCfg->nMinScreenSize) 
		{
//			g_pLTClient->CPrint("CPerformanceMgr::IsCurrentConfig(%d) : failed on MinScreenSize",nCfg);
			return LTFALSE;
		}
		if (nScreenSize > pCfg->nMaxScreenSize) 
		{
//			g_pLTClient->CPrint("CPerformanceMgr::IsCurrentConfig(%d) : failed on MaxScreenSize",nCfg);
			return LTFALSE;
		}
	}

	for (int i = 0; i < kNumDetailSettings; i++)
	{
		if (kPerform_TripleBuffering == i && !(dwAdvancedOptions & AO_TRIPLEBUFFER))
			continue;
		if (nCfg == 2 && kPerform_TripleBuffering == i)
		{
			//for high detail, the triple buffering default value depends on the VSyncOnFlip setting
			int buff = GetConsoleInt(sSettings[i].szVar,0);
			int vsync = !!GetConsoleInt("VSyncOnFlip",1);
			if (buff != (vsync + 1))
			{
//				g_pLTClient->CPrint("CPerformanceMgr::IsCurrentConfig(%d) : failed on %s",nCfg,sSettings[i].szName);
				return LTFALSE;
			}
			continue;		
		}
		if (kPerform_MusicActive == i && !(dwAdvancedOptions & AO_MUSIC))
			continue;
		int n = GetConsoleInt(sSettings[i].szVar,0);
		if (n != pCfg->nSettings[i])
		{
//			g_pLTClient->CPrint("CPerformanceMgr::IsCurrentConfig(%d) : failed on %s",nCfg,sSettings[i].szName);
			return LTFALSE;
		}

	}

	int nDetail = pCfg->nSettings[kPerform_DetailLevel];
	if ( nDetail < 0 || nDetail >= kNumDetailLevels)
	{
		for (int grp = 0; grp < kNumTextureGroups; grp++)
		{
			if (pCfg->nDetails[grp] != GetConsoleInt(sTextureGroups[grp].szVar,0) )
			{
//				g_pLTClient->CPrint("CPerformanceMgr::IsCurrentConfig(%d) : failed on %s",nCfg,sTextureGroups[grp].szName);
				return LTFALSE;
			}
		}
	}

	return LTTRUE;


}

int CPerformanceMgr::GetDetailLevel(const int* pOffsetArray)
{
	int level;
	for (level = 0; level < kNumDetailLevels; level++)
	{
		int grp;
		for (grp = 0; grp < kNumTextureGroups; grp++)
		{
			if (pOffsetArray[grp] != sTextureGroups[grp].nSetting[level])
				break;
		}
		if (grp >= kNumTextureGroups)
			break;
	}
	return level;
}

void CPerformanceMgr::SetDetailLevels(int nLevel, int* pOffsetArray)
{
	if (nLevel < 0 || nLevel >= kNumDetailLevels) return;
	for (int grp = 0; grp < kNumTextureGroups; grp++)
	{
		pOffsetArray[grp] = sTextureGroups[grp].nSetting[nLevel];
	}
}



//sets current options from given cfg
void CPerformanceMgr::SetPerformanceOptions(sPerformCfg *pCfg, int nCfg)
{
	for (int i = 0; i < kNumDetailSettings; i++)
	{
		WriteConsoleInt(sSettings[i].szVar,pCfg->nSettings[i]);
	}

	uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	//disable triple buffer 
	if ( !(dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
		
	}
	if (nCfg == 2 && kPerform_TripleBuffering)
	{
		//for high detail, the triple buffering default value depends on the VSyncOnFlip setting
		int vsync = !!GetConsoleInt("VSyncOnFlip",1);
		WriteConsoleInt(sSettings[kPerform_TripleBuffering].szVar,vsync+1);
	}


	//disable music
	if ( !(dwAdvancedOptions & AO_MUSIC))
	{
		WriteConsoleInt(sSettings[kPerform_MusicActive].szVar,0);
	}

	if (pCfg->nSettings[kPerform_MusicActive] == 0)
	{
		WriteConsoleInt("musicvolume",MUSIC_MIN_VOL);
	}

	if (pCfg->nSettings[kPerform_ShadowDetail] <= 0)
	{
		//turn off shadows if appropriate
		WriteConsoleInt("ModelShadow_Proj_Enable",   0 );
	}
	else
	{
		WriteConsoleInt("ModelShadow_Proj_Enable",   1 );
		int8 nDetail = pCfg->nSettings[kPerform_ShadowDetail] - 1;
		for (int sub = 0; sub < kNumShadowDetails; sub++)
		{
			WriteConsoleInt(sShadowDetails[sub].szVar,sShadowDetails[sub].nSetting[nDetail]);
		}

	}

	uint8 nDetail = pCfg->nSettings[kPerform_FXDetail];
	for (int sub = 0; sub < kNumFXDetails; sub++)
	{
		WriteConsoleInt(sFXDetails[sub].szVar,sFXDetails[sub].nSetting[nDetail]);
	}

	// Set the fx detail setting here (it is set in the loop above) so we know it is 
	// always correct...
	g_pGameClientShell->GetClientFXMgr()->SetDetailLevel( GetConsoleInt("ClientFXDetailLevel", 0) );


	switch (pCfg->nSettings[kPerform_EnvironmentalDetail])
	{
	case 0: //off
		WriteConsoleInt("ScatterEnable",  0 );
		WriteConsoleInt("SnowEnable",  0 );
		break;
	case 1: //low
		WriteConsoleInt("ScatterEnable",  0 );
		WriteConsoleInt("SnowEnable",  1 );
		WriteConsoleFloat("SnowDensityScale",  0.25f );
		break;
	case 2: //medium
		WriteConsoleInt("ScatterEnable",  1 );
		WriteConsoleInt("SnowEnable",  1 );
		WriteConsoleFloat("SnowDensityScale",  0.5f );
		break;
	case 3: //high
		WriteConsoleInt("ScatterEnable",  1 );
		WriteConsoleInt("SnowEnable",  1 );
		WriteConsoleFloat("SnowDensityScale",  1.0f );
		break;
	}
	
	

	nDetail = pCfg->nSettings[kPerform_DetailLevel];
	if ( nDetail >= 0 && nDetail < kNumDetailLevels)
	{
		for (int grp = 0; grp < kNumTextureGroups; grp++)
		{
			WriteConsoleInt(sTextureGroups[grp].szVar,sTextureGroups[grp].nSetting[nDetail]);
		}
	}
	else
	{
		for (int grp = 0; grp < kNumTextureGroups; grp++)
		{
			WriteConsoleInt(sTextureGroups[grp].szVar,pCfg->nDetails[grp]);
		}
	}

	if (nCfg < 0)
		nCfg = GetPerformanceCfg(false);

	if (nCfg < 0 || (uint32)nCfg >= m_ConfigList.size())
	{
		WriteConsoleString("PerformanceConfig",".CustomConfig");
	}
	else
	{
		sPerformCfg *pPresetCfg = m_ConfigList[nCfg];
		WriteConsoleString("PerformanceConfig",pPresetCfg->szName);
	}

//	g_pLTClient->WriteConfigFile("autoexec.cfg");

}

//fills cfg with current options
void CPerformanceMgr::GetPerformanceOptions(sPerformCfg *pCfg)
{
	for (int i = 0; i < kNumDetailSettings; i++)
	{
		pCfg->nSettings[i] = GetConsoleInt(sSettings[i].szVar,pCfg->nSettings[i]);
	}
	for (int grp = 0; grp < kNumTextureGroups; grp++)
	{
		pCfg->nDetails[grp] = GetConsoleInt(sTextureGroups[grp].szVar,pCfg->nDetails[grp]);
	}

	GetConsoleString("PerformanceConfig",pCfg->szName,pCfg->szName);

}

void CPerformanceMgr::LoadPerformanceOptions(CButeMgr &buteMgr,sPerformCfg *pCfg)
{
	for (int i = 0; i < kNumDetailSettings; i++)
	{
		pCfg->nSettings[i] = buteMgr.GetInt("Settings",sSettings[i].szName, 0);
	}

	int nDetail = pCfg->nSettings[kPerform_DetailLevel];
	if ( nDetail >= 0 && nDetail < kNumDetailLevels)
	{
		for (int grp = 0; grp < kNumTextureGroups; grp++)
		{
			pCfg->nDetails[grp] = (int8)sTextureGroups[grp].nSetting[nDetail];
		}

	}
	else
	{
		for (int grp = 0; grp < kNumTextureGroups; grp++)
		{
			pCfg->nDetails[grp] = buteMgr.GetInt("Details",sTextureGroups[grp].szName, 0);
		}
	}
}

void CPerformanceMgr::SavePerformanceOptions(CButeMgr &buteMgr,sPerformCfg *pCfg)
{

	for (int i = 0; i < kNumDetailSettings; i++)
	{
		buteMgr.SetInt("Settings",sSettings[i].szName, pCfg->nSettings[i]);
	}

	for (int grp = 0; grp < kNumTextureGroups; grp++)
	{
		buteMgr.SetInt("Details",sTextureGroups[grp].szName, pCfg->nDetails[grp]);
	}
}

//gets the current value of the specified setting
int	CPerformanceMgr::GetSetting(eDetailID eDetail)
{
	return GetConsoleInt(sSettings[eDetail].szVar,sSettings[eDetail].nSetting[0]);
}
