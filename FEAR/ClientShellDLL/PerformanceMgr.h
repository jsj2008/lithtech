// ----------------------------------------------------------------------- //
//
// MODULE  : PerformanceMgr.h
//
// PURPOSE : Manage performance related settings
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PERFORM_MGR_H
#define PERFORM_MGR_H

#include "ClientUtilities.h"
#include "DatabaseUtils.h"
#include "PerformanceStats.h"

#include <vector>

enum DetailLevel
{
	ePO_Minimum,
	ePO_Low,
	ePO_Medium,
	ePO_High,
	ePO_Maximum,
	ePO_Undefined,
};

const DetailLevel g_DefaultCPUDetailLevel = ePO_High;
const DetailLevel g_DefaultGPUDetailLevel = ePO_Medium;

enum ActivationTypeFlag
{
	eAT_RestartWorld		= 0x00000001,
	eAT_RestartRender		= 0x00000002,
	eAT_RebindTextures		= 0x00000004,
	eAT_RebindShaders		= 0x00000008,
	eAT_NotifyServer		= 0x00000010,
};

enum PerformanceType
{
	ePT_CPU,
	ePT_GPU,
	kNumPerformanceTypes
};

enum PeformanceStatsState
{
	ePeformanceStatsState_Invalid,
	ePeformanceStatsState_CapsOnly,
	ePeformanceStatsState_Full
};

class CPerformanceMgr : public CGameDatabaseReader, public CGameDatabaseCreator
{
	DECLARE_SINGLETON( CPerformanceMgr );
private:
	struct SAutoDetect
	{
		// name of the value
		const char* szValue;
		// current value after auto detect
		float fValue;
	};

public:

	bool	Init();
	void	Term();

	void		Load(HDATABASE hDB, bool bLoadDisplaySettings); //optionally defer applying these settings
	void		Save(HDATABASECREATOR hDBC);

	// Handle setting performance options when entering a level...
	void		OnEnterWorld( );

	uint32		GetNumGroups(uint32 nType) const {return ((nType>=kNumPerformanceTypes) ? 0 : m_nNumGroups[nType]);}
	uint32		GetNumOptions(uint32 nType, uint32 nGroup) const;

	HRECORD		GetGroupRecord(uint32 nType, uint32 nGroup) const;
	HRECORD		GetOptionRecord(uint32 nType, uint32 nGroup, uint32 nOption) const;

	bool		IsOptionAtOverallLevel(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOption, DetailLevel eOverall) const;
	bool		IsOptionAtOverallLevel(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOverall) const;
	int32		GetOptionLevelFromOverall(uint32 nType, uint32 nGroup, uint32 nOption, DetailLevel eOverall) const;

	DetailLevel GetDetailLevel(uint32 nType) const;
	bool		IsDetailLevel(uint32 nType, DetailLevel eLevel) const;
	int32		GetOptionLevel(HRECORD hOptRec) const;
	int32		GetOptionLevel(uint32 nType, uint32 nGroup, uint32 nOption) const;

	void		SetDetailLevel(uint32 nType, DetailLevel eLevel);
	void		SetOptionLevel(uint32 nType, uint32 nGroup, uint32 nOption, int32 nLevel);
	
	// returns the value of a console variable that has been modified
	bool		GetQueuedConsoleVariable( const char* szVariable, float& fValue ) const;
	// adds a console variable to the queue or changes the existing one. 
	// WARNING: the character string pointed to by szVariable must exist for the 
	// lifetime of this class, the function does not make a copy
	bool		SetQueuedConsoleVariable( const char* szVariable, float fValue, uint32 atFlags );

	// applies changes that have been queued
	bool		ApplyQueuedConsoleChanges(bool bApplyResolution);
	// reverts any changes that have been queued
	void		RevertQueuedConsoleChanges();

	// stat methods
	// returns true if the performance struct returned by GetPerformanceStats() is valid
	bool				ArePerformanceStatsValid() { return m_ValidStats == ePeformanceStatsState_Full; }
	bool				ArePerformanceCapsValid() { return m_ValidStats >= ePeformanceStatsState_CapsOnly; }
	// returns the performance struct
	SPerformanceStats&	GetPerformanceStats() { return m_Stats; }
	// runs benchmarks on the current system to detect machine performance and fill out 
	// the performance struct
	bool				DetectPerformanceStats( bool bQuick = false );
	// sets all of the settings based on the performance options
	void				SetBasedOnPerformanceStats();

	// estimates video memory usage based on the current performance settings
	float				EstimateVideoMemoryUsage();

	// Apply ambient LOD whenever LOD settings change or level changes.
	void				ApplyAmbientLOD( );

	// Set physics update rates based on multiplayer or single player games.
	void ApplyPhysicsUpdateRates( );


#ifndef _FINAL
	void				PrintCurrentValues();
#endif // _FINAL

protected:

	uint32	m_nNumGroups[ kNumPerformanceTypes ];
	DetailLevel m_eLastDetailSetting[ kNumPerformanceTypes ];

	// stats that can be accessed from the rest of the screens
	SPerformanceStats		m_Stats;
	PeformanceStatsState	m_ValidStats;

	// a list of console variables that have been modified but not yet set
	struct SConsoleVariable
	{
		const char*	m_szVariable;
		float		m_fValue;
		uint32		m_atFlags;

		SConsoleVariable( const char* szVariable, float	fValue, uint32 atFlags ) :
			m_szVariable(szVariable),
			m_fValue(fValue),
			m_atFlags(atFlags)
		{}

		bool operator==( const char* rhs ) const{
			return LTStrIEquals(m_szVariable, rhs);
		}
	};
	typedef std::vector<SConsoleVariable> TVariableQueue;
	TVariableQueue m_VariableQueue;

	// apply any special activations require since the last activation
	void		ActivateChanges( uint32 nATFlags );
};


#endif	// PERFORM_MGR_H
