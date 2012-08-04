// ----------------------------------------------------------------------- //
//
// MODULE  : PerformanceMgr.h
//
// PURPOSE : Manage performance related settings
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PERFORM_MGR_H
#define PERFORM_MGR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientUtilities.h"
#include "ButeMgr.h"

class CPerformanceMgr;
extern CPerformanceMgr* g_pPerformanceMgr;

const int kNumDetailLevels = 3;
const int kNumTextureGroups = 3;
typedef struct sDetailSetting_t
{
	char		szVar[64];
	char		szName[64];
	int16		nSetting[kNumDetailLevels];
} sDetailSetting;

enum eDetailID
{
#define INCLUDE_AS_ENUM
#include "PerformanceEnum.h"
#undef INCLUDE_AS_ENUM

	kNumDetailSettings
};


typedef struct sPerformCfg_t
{
	char		szName[64];
	char		szNiceName[64];
	uint16		nMinScreenSize;
	uint16		nMaxScreenSize;
	uint8		nSettings[kNumDetailSettings];
	int8		nDetails[kNumTextureGroups];
} sPerformCfg;



class CPerformanceMgr
{
public:

	LTBOOL	Init();
	void	Term();


	int		GetPerformanceCfg(bool bIgnoreResolution);

	//gets the current value of the specified setting
	int		GetSetting(eDetailID eDetail);

	void	SetPerformanceCfg(char *szName);
	void	SetPerformanceCfg(int nCfg);
	
	typedef std::vector<sPerformCfg *>	ConfigList;
	ConfigList	m_ConfigList;

	int		GetDetailLevel(const int* pOffsetArray);
	void	SetDetailLevels(int nLevel, int* pOffsetArray);


	//sets current options from given cfg
	void	SetPerformanceOptions(sPerformCfg *pCfg, int nCfg = -1);
	//fills cfg with current options
	void	GetPerformanceOptions(sPerformCfg *pCfg);

	//used by ProfileMgr to load/save performance options
	void	LoadPerformanceOptions(CButeMgr &buteMgr,sPerformCfg *pCfg);
	void	SavePerformanceOptions(CButeMgr &buteMgr,sPerformCfg *pCfg);


protected:
	void	BuildConfigList();
	LTBOOL	IsCurrentConfig(int nCfg,bool bIgnoreResolution);


	CButeMgr	m_buteMgr;


};


#endif	// PERFORM_MGR_H
