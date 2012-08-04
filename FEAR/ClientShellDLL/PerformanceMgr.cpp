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
#include "sys/win/mpstrconv.h"
#include "PerformanceDB.h"

#include <algorithm>

static const char * const g_prf_PerformanceCat		= "Performance";

VarTrack g_vtPerformanceDebug;
const char* g_szPerfNames[ePO_Undefined+1] = 
{
	"ePO_Minimum",
	"ePO_Low",
	"ePO_Medium",
	"ePO_High",
	"ePO_Maximum",
	"ePO_Undefined"
};

// ----------------------------------------------------------------------- //
//
//	constructor/destructor
//
// ----------------------------------------------------------------------- //
CPerformanceMgr::CPerformanceMgr() :
	m_ValidStats(ePeformanceStatsState_Invalid)
{
}

CPerformanceMgr::~CPerformanceMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::Init/Term
//
//	PURPOSE:	set up/clean up 
//
// ----------------------------------------------------------------------- //

bool CPerformanceMgr::Init()
{
	HRECORD hRec = DATABASE_CATEGORY( PerformanceGlobal ).GetCPURecord();
	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"GroupOrder");
	m_nNumGroups[ePT_CPU] = g_pLTDatabase->GetNumValues(hAtt);

	hRec = DATABASE_CATEGORY( PerformanceGlobal ).GetGPURecord();
	hAtt = g_pLTDatabase->GetAttribute(hRec,"GroupOrder");
	m_nNumGroups[ePT_GPU] = g_pLTDatabase->GetNumValues(hAtt);

	m_eLastDetailSetting[ePT_CPU] = g_DefaultGPUDetailLevel;
	m_eLastDetailSetting[ePT_GPU] = g_DefaultCPUDetailLevel;

	g_vtPerformanceDebug.Init(g_pLTClient,"PerformanceDebug",NULL,0.0f);
	return true;
}

void CPerformanceMgr::Term()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::Load/Save
//
//	PURPOSE:	write to/read from profile
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::Load(HDATABASE hDB, bool bLoadDisplaySettings ) //optionally defer applying these settings
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_PerformanceCat,g_prf_PerformanceCat);
	if (!hRec) 
	{
		SetDetailLevel(ePT_CPU,g_DefaultCPUDetailLevel);
		SetDetailLevel(ePT_GPU,g_DefaultGPUDetailLevel);
		return;
	}

	for (uint32 nType = 0; nType < kNumPerformanceTypes; ++nType)
	{
		DetailLevel eLevel;
		if (nType == ePT_CPU)
		{
			eLevel = (DetailLevel)GetInt32(hRec,"CPUDetailLevel");
		}
		else
		{
			eLevel = (DetailLevel)GetInt32(hRec,"GPUDetailLevel");
		}

		if (eLevel == ePO_Undefined)
		{
			for (uint32 nGroup = 0; nGroup < GetNumGroups(nType); ++nGroup)
			{
				uint32 nNumOptions = GetNumOptions(nType,nGroup);
				for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
				{
					HRECORD hOptRec = GetOptionRecord(nType,nGroup,nOption);

					HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hOptRec);
					uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);
					for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
					{
						const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
						if (!pszVar || !pszVar[0])
							continue;

						const uint32 atFlags = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIB( hOptRec, ActivationFlags );

						const float fValue = GetFloat(hRec,pszVar);
						SetQueuedConsoleVariable(pszVar, fValue, atFlags);
					}

				}
			}
		}
		else
		{
			SetDetailLevel(nType,eLevel);
		}
	}

	ApplyQueuedConsoleChanges(bLoadDisplaySettings);
}

void CPerformanceMgr::Save(HDATABASECREATOR hDBC)
{
	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_PerformanceCat,"Profile.Performance");
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_PerformanceCat);
	if (!hRec) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	WriteConsoleInt("Performance_ScreenWidth",pProfile->m_nScreenWidth);
	WriteConsoleInt("Performance_ScreenHeight",pProfile->m_nScreenHeight);

	for (uint32 nType = 0; nType < kNumPerformanceTypes; ++nType)
	{
		DetailLevel eLevel = GetDetailLevel(nType);
		if (nType == ePT_CPU)
		{
			CreateInt32(hRec,"CPUDetailLevel",(int32)eLevel);
		}
		else
		{
			CreateInt32(hRec,"GPUDetailLevel",(int32)eLevel);
		}
		if (eLevel == ePO_Undefined)
		{
			for (uint32 nGroup = 0; nGroup < GetNumGroups(nType); ++nGroup)
			{
				uint32 nNumOptions = GetNumOptions(nType,nGroup);
				for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
				{
					HRECORD hOptRec = GetOptionRecord(nType,nGroup,nOption);
					HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hOptRec);
					uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);
					for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
					{
						const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
						if (!pszVar || !pszVar[0])
							continue;
						float fValue = GetConsoleFloat(pszVar,0.0f);
						CreateFloat(hRec,pszVar,fValue);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetNumOptions
//
//	PURPOSE:	Handle setting performance options when entering a level...
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::OnEnterWorld( )
{
	ApplyPhysicsUpdateRates( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetNumOptions
//
//	PURPOSE:	retrieve the number of options in the given group
//
// ----------------------------------------------------------------------- //

uint32 CPerformanceMgr::GetNumOptions(uint32 nType, uint32 nGroup) const
{
	if (nType >= kNumPerformanceTypes || nGroup >= m_nNumGroups[nType]) 
	{
		return 0;
	}
	HRECORD hRec = GetGroupRecord(nType,nGroup);
	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"Options");
	return g_pLTDatabase->GetNumValues(hAtt);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetGroupRecord
//
//	PURPOSE:	retrieve the record for a group
//
// ----------------------------------------------------------------------- //

HRECORD CPerformanceMgr::GetGroupRecord(uint32 nType, uint32 nGroup) const
{

	if (nType >= kNumPerformanceTypes || nGroup >= m_nNumGroups[nType]) 
	{
		return NULL;
	}

	if (nType == ePT_CPU)
	{
		return DATABASE_CATEGORY( PerformanceGlobal ).GETRECORDATTRIBINDEX( DATABASE_CATEGORY( PerformanceGlobal ).GetCPURecord(), GroupOrder, nGroup );
	}

	return DATABASE_CATEGORY( PerformanceGlobal ).GETRECORDATTRIBINDEX( DATABASE_CATEGORY( PerformanceGlobal ).GetGPURecord(), GroupOrder, nGroup );

}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetOptionRecord
//
//	PURPOSE:	retrieve the record for an option in a particular group
//
// ----------------------------------------------------------------------- //

HRECORD	CPerformanceMgr::GetOptionRecord(uint32 nType, uint32 nGroup, uint32 nOption) const
{	
	if (nType >= kNumPerformanceTypes || nOption < GetNumOptions(nType,nGroup))
	{
		HRECORD hRec = GetGroupRecord(nType,nGroup);
		return DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIBINDEX( hRec, Options, nOption );
	}
	else
	{
		return NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::IsOptionAtOverallLevel
//
//	PURPOSE:	given an overall level of detail and a level for a particular option
//				determine if the option is set to a value that is valid for the overall
//				detail setting
//
//  NOTE:		this version does not actual check any current settings, just defaults
//				from the DB
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::IsOptionAtOverallLevel(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOption, DetailLevel eOverall) const
{
	//simple cases
	if (eOverall == ePO_Maximum)
		return (eOption == ePO_High);
	if (eOverall == ePO_Minimum)
		return (eOption == ePO_Low);

	return (eOption	== GetOptionLevelFromOverall(nType, nGroup,nOption,eOverall));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::IsOptionAtOverallLevel
//
//	PURPOSE:	given an overall level of detail determine if the option is set 
//				to a value that is valid for the overall detail setting
//
//  NOTE:		this version checks current settings
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::IsOptionAtOverallLevel(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOverall) const
{
	int32 nCurrentLevel = GetOptionLevel( nType, nGroup, nOption );
	int32 nOverallLevel = GetOptionLevelFromOverall( nType, nGroup, nOption, eOverall );

	return (nCurrentLevel == nOverallLevel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetOptionLevelFromOverall
//
//	PURPOSE:	given an overall level of detail determine the corresponding
//				level for a particular option (Low, Medium, or High)
//
//  NOTE:		this does not actual check any current settings, just defaults
//				from the DB
//
// ----------------------------------------------------------------------- //

int32 CPerformanceMgr::GetOptionLevelFromOverall(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOverall) const
{
	HRECORD hRec = GetOptionRecord(nType,nGroup,nOption);
	if( !hRec )
		return -1;

	if (eOverall == ePO_Maximum)
	{
		return 0;
	}
	else if (eOverall == ePO_Minimum)
	{
		HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"DetailNames");
		const uint32 nLevelCount = g_pLTDatabase->GetNumValues(hAtt);
		return nLevelCount - 1;
	}

	int32 nLevel = -1;
	if (eOverall >= ePO_Low && eOverall <= ePO_High) 
	{
		uint32 nIndex = (uint32)(ePO_High-eOverall);
		nLevel = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIBINDEX( hRec, Configurations, nIndex ) - 1;
	}

	return nLevel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetOptionLevel
//
//	PURPOSE:	get the current level of an option based on its console
//				variable
//
// ----------------------------------------------------------------------- //
int32 CPerformanceMgr::GetOptionLevel(uint32 nType, uint32 nGroup, uint32 nOption) const
{
	HRECORD hOptRec = GetOptionRecord(nType,nGroup,nOption);
	return GetOptionLevel(hOptRec);
}

int32 CPerformanceMgr::GetOptionLevel(HRECORD hOptRec) const
{
	HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hOptRec);
	uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);

	int32 nLevel = -1;
	bool bRecheckCurrentLevel = false;

	do 
	{
		bRecheckCurrentLevel = false;

		for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
		{
			const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
			if (!pszVar || !pszVar[0])
			{
				continue;
			}

			float fValue = 0.0f;

			if( !GetQueuedConsoleVariable(pszVar, fValue) )
				fValue = GetConsoleFloat(pszVar,0.0f);

			HATTRIBUTE hAtt = GetStructAttribute( hVars, nVar, "DetailValues" );
			int32 nValueCount = g_pLTDatabase->GetNumValues(hAtt);

			for( int32 nValue = 0; nValue < nValueCount; ++nValue )
			{
				if( LTNearlyEquals(fValue,DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIBINDEX( Variables, hVars, nVar, DetailValues, nValue ), MATH_EPSILON) )
				{
					if( nValue == nLevel )
						break;

					if( nValue > nLevel )
					{
						nLevel = nValue;
						bRecheckCurrentLevel = true;
						break;
					}
				}
			}
		}
	}while( bRecheckCurrentLevel );

	return nLevel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetDetailLevel()
//
//	PURPOSE:	get the overall level based on the various groups
//
// ----------------------------------------------------------------------- //
DetailLevel CPerformanceMgr::GetDetailLevel(uint32 nType) const
{
	bool bMatch = false;
	DetailLevel eLevel = ePO_Undefined;

	// First check to see if it is our last level.
	// This must be done so that if two levels are identical,
	// the system will be biased towards the one the user last
	// set.
	if( m_eLastDetailSetting[nType] != ePO_Undefined )
	{
		eLevel = m_eLastDetailSetting[nType];
		bMatch = IsDetailLevel(nType, eLevel);
	}

	//keep looking at levels until we find one that matches...
	for (uint32 nLevel = ePO_Minimum; !bMatch && nLevel < ePO_Undefined; ++nLevel)
	{
		//assume this level will match...
		eLevel = (DetailLevel)nLevel;
		bMatch = IsDetailLevel(nType, eLevel);
	}

	//we found a level that matches our settings
	if (bMatch)
	{
		if (g_vtPerformanceDebug.GetFloat() > 0.0f)
		{
			DebugCPrint(0,"CPerformanceMgr::GetDetailLevel() - %s",g_szPerfNames[eLevel]);
		}
		return eLevel;
	}

	if (g_vtPerformanceDebug.GetFloat() > 0.0f)
	{
		DebugCPrint(0,"CPerformanceMgr::GetDetailLevel() - %s",g_szPerfNames[ePO_Undefined]);
	}
	return ePO_Undefined;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetDetailLevel()
//
//	PURPOSE:	returns true if all the options match the specified group's level.
//
// ----------------------------------------------------------------------- //

bool CPerformanceMgr::IsDetailLevel(uint32 nType, DetailLevel eLevel) const
{
	//while this level matches, keep checking groups
	for (uint32 nGroup = 0; nGroup < GetNumGroups(nType); ++nGroup)
	{
		uint32 nNumOptions = GetNumOptions(nType,nGroup);
		//while this group matches, keep checking options
		for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
		{
			//do we still match?
			if( !IsOptionAtOverallLevel(nType,nGroup,nOption,eLevel) )
			{
				if (g_vtPerformanceDebug.GetFloat() > 0.0f)
				{
					HRECORD hOption = GetOptionRecord(nType,nGroup,nOption);
					DebugCPrint(0,"CPerformanceMgr::GetDetailLevel() - %s is not at %s",g_pLTDatabase->GetRecordName(hOption),g_szPerfNames[eLevel]);
				}

				return false;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::SetDetailLevel()
//
//	PURPOSE:	set the overall level
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::SetDetailLevel(uint32 nType, DetailLevel eLevel)
{
	for (uint32 nGroup = 0; nGroup < GetNumGroups(nType); ++nGroup)
	{
		uint32 nNumOptions = GetNumOptions(nType, nGroup);
		for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
		{
			int32 nLevel = GetOptionLevelFromOverall(nType, nGroup,nOption,eLevel);
			SetOptionLevel(nType,nGroup,nOption,nLevel);
		}
	}

	m_eLastDetailSetting[nType] = eLevel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::SetOptionLevel()
//
//	PURPOSE:	set the level for a particular option
//
// ----------------------------------------------------------------------- //
#if 0
void CPerformanceMgr::SetOptionLevel(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eLevel)
{
	DetailLevel eOldLevel = ePO_Low;//GetOptionLevel(nType,nGroup,nOption);

	HRECORD hOptRec = GetOptionRecord(nType,nGroup,nOption);
	HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hOptRec);
	uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);
	for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
	{
		const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
		if (!pszVar || !pszVar[0])
			continue;

		switch(eLevel) 
		{
		case ePO_Minimum:
		case ePO_Low:
			WriteConsoleFloat(pszVar,DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIBINDEX( Variables, hVars, nVar, DetailValues, 2 ));
			break;
		case ePO_Medium:
			WriteConsoleFloat(pszVar,DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIBINDEX( Variables, hVars, nVar, DetailValues, 1 ));
			break;
		case ePO_Maximum:
		case ePO_High:
			WriteConsoleFloat(pszVar,DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIBINDEX( Variables, hVars, nVar, DetailValues, 0 ));
			break;
		}
	}

	if (eLevel == ePO_Minimum)
	{
		eLevel = ePO_Low; //change for the comparison below
	}
	if (eLevel == ePO_Maximum)
	{
		eLevel = ePO_High; //change for the comparison below
	}
	
	if (eOldLevel != eLevel)
	{
		uint32 atFlags = DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIB( hOptRec, ActivationFlags );

		m_bNeedRestartGame |= !!(atFlags & eAT_RestartWorld);
		m_bNeedRestartRenderer |= !!(atFlags & eAT_RestartRender);
		m_bNeedRebindTextures |= !!(atFlags & eAT_RebindTextures);
		m_bNeedRebindShaders |= !!(atFlags & eAT_RebindShaders);

	}
}
#endif

void CPerformanceMgr::SetOptionLevel(uint32 nType, uint32 nGroup, uint32 nOption, int32 nLevel)
{
	LTASSERT( nLevel >= 0, "Invalid level" );
	if( nLevel < 0 )
		return;

	HRECORD hOptRec = GetOptionRecord(nType,nGroup,nOption);
	HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hOptRec);
	uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);
	for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
	{
		const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
		if (!pszVar || !pszVar[0])
			continue;

		HATTRIBUTE hAtt = GetStructAttribute( hVars, nVar, "DetailValues" );
		int32 nValueCount = g_pLTDatabase->GetNumValues(hAtt);

		LTASSERT( nLevel < nValueCount, "Invalid level" );
		if( nLevel >= nValueCount )
			continue;

		float fValue	= DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIBINDEX( Variables, hVars, nVar, DetailValues, nLevel );
		uint32 atFlags	= DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIB( hOptRec, ActivationFlags );
		SetQueuedConsoleVariable( pszVar, fValue, atFlags );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::ActivateChanges()
//
//	PURPOSE:	apply any special activations require since the last activation
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::ActivateChanges( uint32 nATFlags )
{
	//no player mgr, means we're just starting up, so we don't need to do anything...
	if (g_pPlayerMgr)
	{
		if( (nATFlags & eAT_RestartWorld) && g_pPlayerMgr->IsPlayerInWorld())
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox("Performance_RestartWarning",&mb);
		}

		if( nATFlags & eAT_RestartRender ) 
		{
			//DebugCPrint(0,"CPerformanceMgr::ActivateChanges() - restarting render.");
			g_pInterfaceMgr->SetSwitchingRenderModes(LTTRUE);
			g_pLTClient->RunConsoleCommand("RestartRender");
			g_pInterfaceMgr->SetSwitchingRenderModes(LTFALSE);
		}

		if( nATFlags & eAT_RebindTextures ) 
		{
			//		DebugCPrint(0,"CPerformanceMgr::ActivateChanges() - rebinding textures.");
			g_pInterfaceResMgr->DrawMessage("IDS_REBINDING_TEXTURES");
			g_pLTClient->RunConsoleCommand("RebindTextures");

		}
		
		if( nATFlags & eAT_RebindShaders ) 
		{
			//		DebugCPrint(0,"CPerformanceMgr::ActivateChanges() - rebinding textures.");
			g_pInterfaceResMgr->DrawMessage("IDS_REBINDING_SHADERS");
			g_pLTClient->RunConsoleCommand("RebindShaders");

		}

		if( nATFlags & eAT_NotifyServer )
		{
			// Update the server of the changes...
			g_pGameClientShell->SendPerformanceSettingsToServer( );
		}
	}
	else
	{
		// If we ARE starting up, we may still need to change some settings, since the
		// renderer is probably already initialized.
		
		if( nATFlags & eAT_RestartRender ) 
		{
			g_pLTClient->RunConsoleCommand("RestartRender");
		}

		if( nATFlags & eAT_RebindTextures ) 
		{
			g_pLTClient->RunConsoleCommand("RebindTextures");
		}

		if( nATFlags & eAT_RebindShaders )
		{
			g_pLTClient->RunConsoleCommand("RebindShaders");
		}
	}

	// Update the ambient light based on LOD.
	ApplyAmbientLOD( );

	// Update the correct physics update rates.
	ApplyPhysicsUpdateRates( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::GetQueuedConsoleVariable()
//
//	PURPOSE:	returns the value of a console variable that has been modified
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::GetQueuedConsoleVariable( const char* szVariable, float& fValue ) const 
{
	TVariableQueue::const_iterator itrVariable;
	itrVariable = std::find( m_VariableQueue.begin(), m_VariableQueue.end(), szVariable );
	if( itrVariable == m_VariableQueue.end() )
		return false;

	fValue = itrVariable->m_fValue;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::SetQueuedConsoleVariable()
//
//	PURPOSE:	adds a console variable to the queue or changes the existing one. 
//
//  WARNING:	the character string pointed to by szVariable must exist for the 
//				lifetime of this class, the function does not make a copy
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::SetQueuedConsoleVariable( const char* szVariable, float fValue, uint32 atFlags )
{
	TVariableQueue::iterator itrVariable;
	itrVariable = std::find( m_VariableQueue.begin(), m_VariableQueue.end(), szVariable );
	if( itrVariable == m_VariableQueue.end() )
	{
		// check if it differs from the current setting
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(szVariable);
		float fValueCurrent = 0.0f;
		if( hVar )
		{
			fValueCurrent = g_pLTClient->GetConsoleVariableFloat(hVar);
		}

		if( !hVar || !LTNearlyEquals(fValueCurrent, fValue, MATH_EPSILON) )
		{
			// add a new entry
			m_VariableQueue.push_back( SConsoleVariable(szVariable, fValue, atFlags) );
		}
		return true;
	}

	// modify existing entry
	itrVariable->m_fValue = fValue;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::ApplyQueuedConsoleChanges()
//
//	PURPOSE:	applies changes that have been queued
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::ApplyQueuedConsoleChanges(bool bApplyResolution)
{
	uint32 nATFlags = 0;
	for(uint32 nChange=0;nChange<m_VariableQueue.size();++nChange)
	{
		WriteConsoleFloat( m_VariableQueue[nChange].m_szVariable, m_VariableQueue[nChange].m_fValue );
		nATFlags |= m_VariableQueue[nChange].m_atFlags;
	}

	ltstd::free_vector( m_VariableQueue );


	if (bApplyResolution)
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		uint32 nWidth	= GetConsoleInt("Performance_ScreenWidth",640);
		uint32 nHeight	= GetConsoleInt("Performance_ScreenHeight",480);
		if( pProfile->m_nScreenWidth != nWidth || pProfile->m_nScreenHeight != nHeight )
		{
			//force the console variable back to the old values, so that the profile will properly 
			pProfile->m_nScreenWidth = nWidth;
			pProfile->m_nScreenHeight = nHeight;
			pProfile->ApplyDisplay();
			nATFlags &= ~eAT_RestartRender;
		}
	}

	ActivateChanges( nATFlags );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::RevertQueuedConsoleChanges()
//
//	PURPOSE:	reverts any changes that have been queued
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::RevertQueuedConsoleChanges()
{
	ltstd::free_vector( m_VariableQueue );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::DetectPerformanceStats()
//
//	PURPOSE:	runs benchmarks on the current system to detect machine 
//				performance and fill out the performance struct
//
// ----------------------------------------------------------------------- //
bool CPerformanceMgr::DetectPerformanceStats( bool bQuick /*= false*/ )
{
	// if we are doing a quick query
	if( bQuick )
	{
		if( (g_pLTClient->QueryPerformanceStats(&m_Stats, true) == LT_OK) )
		{
			if( m_ValidStats < ePeformanceStatsState_CapsOnly )
				m_ValidStats = ePeformanceStatsState_CapsOnly;
		}
		return ArePerformanceCapsValid();
	}

	if( !(g_pLTClient->QueryPerformanceStats(&m_Stats, false) == LT_OK ) )
	{
		LTERROR("QueryPerformanceStats failed!");
		return ArePerformanceStatsValid();
	}

	m_ValidStats = ePeformanceStatsState_Full;

	// round the CPU frequency to the nearest 1/10th GHz
	m_Stats.m_fCPUFrequency *= 10.0f;
	m_Stats.m_fCPUFrequency += 0.5f;
	m_Stats.m_fCPUFrequency = floorf(m_Stats.m_fCPUFrequency);
	m_Stats.m_fCPUFrequency /= 10.0f;

	return ArePerformanceStatsValid();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::EstimateVideoMemoryUsage()
//
//	PURPOSE:	estimates video memory usage based on the current 
//				performance settings
//
// ----------------------------------------------------------------------- //
float CPerformanceMgr::EstimateVideoMemoryUsage()
{
	float fTotalMemoryUsage = 0.0f;

	for(uint32 nType=0;nType<DATABASE_CATEGORY( PerformanceGlobal ).GetNumRecords();++nType)
	{
		HRECORD hRecordGlobal = DATABASE_CATEGORY( PerformanceGlobal ).GetRecordByIndex( nType );
		const char* pszRecordName = g_pLTDatabase->GetRecordName( hRecordGlobal );

		// skip this one, it's old for backward compatibility
		if( LTStrIEquals(pszRecordName, "Global") )
			continue;

		HATTRIBUTE hAttribGroupOrder = g_pLTDatabase->GetAttribute( hRecordGlobal, "GroupOrder" );
		if( !hAttribGroupOrder )
			continue;

		uint32 nGroupCount = g_pLTDatabase->GetNumValues( hAttribGroupOrder );
		
		for(uint32 nGroup=0;nGroup<nGroupCount;++nGroup)
		{
			HRECORD hRecordGroup = DATABASE_CATEGORY( PerformanceGlobal ).GETRECORDATTRIBINDEX( hRecordGlobal, GroupOrder, nGroup );
			if( !hRecordGroup )
				continue;

			HATTRIBUTE hAttribOptions = g_pLTDatabase->GetAttribute( hRecordGroup, "Options" );
			if( !hAttribOptions )
				continue;

			uint32 nOptionCount = g_pLTDatabase->GetNumValues( hAttribOptions );
			for(uint32 nOption=0;nOption<nOptionCount;++nOption)
			{
				HRECORD hRecordOption = DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIBINDEX( hRecordGroup, Options, nOption );
				if( !hRecordOption )
					continue;

				const char* pszRecordOptionName = g_pLTDatabase->GetRecordName( hRecordOption );

				int32 nLevel = GetOptionLevel( hRecordOption );
				if( nLevel < 0 )
					continue;

				HATTRIBUTE hVideoMemory = g_pLTDatabase->GetAttribute( hRecordOption, "VideoMemory" );
				if( !hVideoMemory )
					continue;

				int32 nNumVideoMemory = (int32)g_pLTDatabase->GetNumValues(hVideoMemory);

				if( nLevel < nNumVideoMemory )
				{
					fTotalMemoryUsage += DATABASE_CATEGORY( PerformanceOption ).GETRECORDATTRIBINDEX( hRecordOption, VideoMemory, nLevel );
				}
			}
		}
	}

	// now let's add the amount of memory that the frame buffer takes up
	float fWidth;
	if( !GetQueuedConsoleVariable("Performance_ScreenWidth", fWidth) )
		fWidth = GetConsoleFloat("Performance_ScreenWidth", 640.0f);

	float fHeight;
	if( !GetQueuedConsoleVariable("Performance_ScreenHeight", fHeight) )
		fHeight = GetConsoleFloat("Performance_ScreenHeight", 480.0f);

	float fAntiAlias;
	if( !GetQueuedConsoleVariable("AntiAliasFSOverSample", fAntiAlias) )
		fAntiAlias = GetConsoleFloat("AntiAliasFSOverSample", 0.0f);

	float fBufferSizeMB = fWidth * fHeight * 4.0f / (1024.0f*1024.0f);

	HRECORD hGeneral = DATABASE_CATEGORY( PerformanceGeneral ).GetGeneralRecord();
	if( hGeneral )
	{
		int32 nBufferCount = DATABASE_CATEGORY( PerformanceGeneral ).GETRECORDATTRIB( hGeneral, FrameBufferCount );
		fTotalMemoryUsage += fBufferSizeMB * (float)nBufferCount;
		
		// add additional back and depth buffers used by AA
		fTotalMemoryUsage += fBufferSizeMB * 2.0f * (fAntiAlias - 1.0f);

		// now add addition total memory
		int32 nAdditionalVideoMemory = DATABASE_CATEGORY( PerformanceGeneral ).GETRECORDATTRIB( hGeneral, AdditionalVideoMemory );
		fTotalMemoryUsage += (float)nAdditionalVideoMemory;

		// now add addition AGP memory
		int32 nAGPMemory = DATABASE_CATEGORY( PerformanceGeneral ).GETRECORDATTRIB( hGeneral, AGPMemory );
		fTotalMemoryUsage += (float)nAGPMemory;
	}
	else
	{
		fTotalMemoryUsage += fBufferSizeMB * 4.0f;

		// add additional back and depth buffers used by AA
		fTotalMemoryUsage += fBufferSizeMB * 2.0f * (fAntiAlias - 1.0f);
	}

#ifndef _FINAL
	DebugCPrint( 0, "Total Memory Usage: %f", fTotalMemoryUsage );
#endif

	return fTotalMemoryUsage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::SetBasedOnPerformanceStats()
//
//	PURPOSE:	sets all of the settings based on the performance options
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::SetBasedOnPerformanceStats()
{
	if( !ArePerformanceStatsValid() )
	{
		if( !DetectPerformanceStats() )
			return;
	}

#ifndef _FINAL
	// dump the stats to the console to aid in tweaking
	DebugCPrint( 0, "QueryPerformanceStats -->\n"
		"    CPU Frequency (GHz): %f\n"
		"    CPU Count: %d\n"
		"    Physical Memory (MB): %d\n"
		"    GPU Pixel Shader: %f\n"
		"    GPU Vertex Shader: %f\n"
		"    GPU Memory (MB): %d\n"
		"    GPU Fill Rate PS 1.1: %f\n"
		"    GPU Fill Rate PS 2.0: %f\n",
		m_Stats.m_fCPUFrequency,
		m_Stats.m_nCPUCount,
		m_Stats.m_nCPUMemory,
		m_Stats.m_fGPUMaxPS,
		m_Stats.m_fGPUMaxVS,
		m_Stats.m_nGPUMemory,
		m_Stats.m_fGPUPS11Fill,
		m_Stats.m_fGPUPS20Fill
		);
#endif

	const SAutoDetect pAutoDetectData[] = {
		{ "CPU Frequency (GHz)", m_Stats.m_fCPUFrequency },
		{ "CPU Count", (float)m_Stats.m_nCPUCount },
		{ "Physical Memory (MB)", (float)m_Stats.m_nCPUMemory },
		{ "GPU Pixel Shader", m_Stats.m_fGPUMaxPS },
		{ "GPU Vertex Shader", m_Stats.m_fGPUMaxVS },
		{ "GPU Memory (MB)", (float)m_Stats.m_nGPUMemory },
		{ "GPU Fill Rate PS 1.1 (MegaPixels/sec)", m_Stats.m_fGPUPS11Fill },
		{ "GPU Fill Rate PS 2.0 (MegaPixels/sec)", m_Stats.m_fGPUPS20Fill },
	};

	for (uint32 nType = 0; nType < kNumPerformanceTypes; nType++)
	{
		for (uint32 nGroup = 0; nGroup < GetNumGroups(nType); ++nGroup)
		{
			uint32 nNumOptions = GetNumOptions(nType,nGroup);
			for (uint32 nOption = 0; nOption < nNumOptions; ++nOption)
			{
				HRECORD hOptRec = GetOptionRecord( nType, nGroup, nOption );
				if( !hOptRec )
					continue;

				HATTRIBUTE hAutoDetect = DATABASE_CATEGORY( PerformanceOption ).GetAutoDetect( hOptRec );
				if( !hAutoDetect )
					continue;

				DetailLevel eLevel = ePO_Undefined;

				uint32 nNumValues = g_pLTDatabase->GetNumValues( hAutoDetect );
				for(uint32 nValue=0;nValue<nNumValues;++nValue)
				{
					const char* pszValue = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( AutoDetect, hAutoDetect, nValue, Value );
					if (!pszValue || !pszValue[0])
						continue;

					if( LTStrIEquals(pszValue, "none") )
						continue;

					for(uint32 nStat=0;nStat<LTARRAYSIZE(pAutoDetectData);++nStat)
					{
						if( LTStrIEquals(pszValue, pAutoDetectData[nStat].szValue) )
						{
							LTVector2 vThreshold = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( AutoDetect, hAutoDetect, nValue, Threshold );

							if( vThreshold.x > pAutoDetectData[nStat].fValue )
								eLevel = ePO_Low;
							else if( vThreshold.y > pAutoDetectData[nStat].fValue )
								eLevel = ePO_Medium;
							else if( eLevel != ePO_Medium )
								eLevel = ePO_High;
							break;
						}
					}

					// if it's as low as it can get then quit now, can't get any worse
					if( eLevel == ePO_Low )
						break;
				}

				if( eLevel != ePO_Undefined )
				{
					int32 nLevel = GetOptionLevelFromOverall( nType, nGroup, nOption, eLevel );
					SetOptionLevel( nType, nGroup, nOption, nLevel );
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::ApplyAmbientLOD
//
//	PURPOSE:	Applies ambient light LOD
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::ApplyAmbientLOD( )
{
	// Base the additional ambient light off of the lod for lights.  This is done because some
	// lights are removed for lower details which needs to be compensated for using ambient light.
	float fAmbientAdd = 0.0f;
	switch(( int32 )GetConsoleInt( "LODLights", 2 ))
	{
	case 0:
		fAmbientAdd = GetConsoleFloat( "AddAmbientLightLow", 0.0f );
		break;
	case 1:
		fAmbientAdd = GetConsoleFloat( "AddAmbientLightMed", 0.0f );
		break;
	case 2:
		fAmbientAdd = GetConsoleFloat( "AddAmbientLightHigh", 0.0f );
		break;
	}

	// Adjust the ambient light by the level of white light.
	WriteConsoleFloat( "Light_AmbientR", GetConsoleFloat( "Server_Light_AmbientR", 0.0f ) + fAmbientAdd );
	WriteConsoleFloat( "Light_AmbientG", GetConsoleFloat( "Server_Light_AmbientG", 0.0f ) + fAmbientAdd );
	WriteConsoleFloat( "Light_AmbientB", GetConsoleFloat( "Server_Light_AmbientB", 0.0f ) + fAmbientAdd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::ApplyPhysicsUpdateRates
//
//	PURPOSE:	Set physics update rates based on multiplayer or single player games.
//
// ----------------------------------------------------------------------- //
void CPerformanceMgr::ApplyPhysicsUpdateRates( )
{
	if( IsMultiplayerGameClient( ))
	{
		g_pLTClient->SetConsoleVariableFloat( "PhysicsClientUpdateRate", GetConsoleFloat( "MPPhysicsClientUpdateRate", 45.0f ));
		g_pLTClient->SetConsoleVariableFloat( "PhysicsServerUpdateRate", GetConsoleFloat( "MPPhysicsServerUpdateRate", 45.0f ));
	}
	else
	{
		g_pLTClient->SetConsoleVariableFloat( "PhysicsClientUpdateRate", GetConsoleFloat( "SPPhysicsClientUpdateRate", 45.0f ));
		g_pLTClient->SetConsoleVariableFloat( "PhysicsServerUpdateRate", GetConsoleFloat( "SPPhysicsServerUpdateRate", 45.0f ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPerformanceMgr::PrintCurrentValues()
//
//	PURPOSE:	prints out the current values of the performance variables, 
//				this is useful for debugging purposes
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL
void CPerformanceMgr::PrintCurrentValues()
{
	DebugCPrint( 0, "Performance Variables -->" );

	for(uint32 nType=0;nType<DATABASE_CATEGORY( PerformanceGlobal ).GetNumRecords();++nType)
	{
		HRECORD hRecordGlobal = DATABASE_CATEGORY( PerformanceGlobal ).GetRecordByIndex( nType );
		const char* pszRecordName = g_pLTDatabase->GetRecordName( hRecordGlobal );

		// skip this one, it's old for backward compatibility
		if( LTStrIEquals(pszRecordName, "Global") )
			continue;

		HATTRIBUTE hAttribGroupOrder = g_pLTDatabase->GetAttribute( hRecordGlobal, "GroupOrder" );
		if( !hAttribGroupOrder )
			continue;

		uint32 nGroupCount = g_pLTDatabase->GetNumValues( hAttribGroupOrder );

		for(uint32 nGroup=0;nGroup<nGroupCount;++nGroup)
		{
			HRECORD hRecordGroup = DATABASE_CATEGORY( PerformanceGlobal ).GETRECORDATTRIBINDEX( hRecordGlobal, GroupOrder, nGroup );
			if( !hRecordGroup )
				continue;

			HATTRIBUTE hAttribOptions = g_pLTDatabase->GetAttribute( hRecordGroup, "Options" );
			if( !hAttribOptions )
				continue;

			uint32 nOptionCount = g_pLTDatabase->GetNumValues( hAttribOptions );
			for(uint32 nOption=0;nOption<nOptionCount;++nOption)
			{
				HRECORD hRecordOption = DATABASE_CATEGORY( PerformanceGroup ).GETRECORDATTRIBINDEX( hRecordGroup, Options, nOption );
				if( !hRecordOption )
					continue;

				const char* pszRecordOptionName = g_pLTDatabase->GetRecordName( hRecordOption );

				int32 nLevel = GetOptionLevel( hRecordOption );
				if( nLevel < 0 )
					continue;


				HATTRIBUTE hVars = DATABASE_CATEGORY( PerformanceOption ).GetVariables(hRecordOption);
				uint32 nNumVar = g_pLTDatabase->GetNumValues(hVars);
				for (uint32 nVar = 0; nVar < nNumVar; ++nVar)
				{
					const char* pszVar = DATABASE_CATEGORY( PerformanceOption ).GETSTRUCTATTRIB( Variables, hVars, nVar, Variable );
					if (!pszVar || !pszVar[0])
						continue;

					float fValue = GetConsoleFloat( pszVar, 0.0f );

					DebugCPrint( 0, "    %s: %f", pszVar, fValue );
				}
			}
		}
	}
}
#endif // _FINAL
