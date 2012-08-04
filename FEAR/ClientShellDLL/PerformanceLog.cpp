#include "StdAfx.h"
#include "PerformanceLog.h"
#include "resourceextensions.h"
#include "WinUtil.h"
#include <time.h>
#include "iperformancemonitor.h"
#include "PlayerCamera.h"
#include "iltfilemgr.h"

//-----------------------------------------------------------------------------------------------
// HTML Formatting options
#define HTML_NORMAL_FONT_PROPERTIES		"size=\"2\" face=\"Verdana,Arial,Helvetica\""
#define HTML_SMALL_FONT_PROPERTIES		"size=\"1\" face=\"Verdana,Arial,Helvetica\""
#define HTML_CATEGORY_BACKGROUND		"#AFAFAF"
#define HTML_VALUE_BACKGROUND1			"#F3F3ED"
#define HTML_VALUE_BACKGROUND2			"#FAFAFA"
#define HTML_DESCRIPTION_COLOR			"#606060"


//-----------------------------------------------------------------------------------------------
// PerformanceValueMonitor comparison functions
//  Different comparison functions that can be associated with performance monitors

static bool PerfMonCompareLessThan(double f1, double f2)
{
	return f1 < f2;
}

static bool PerfMonCompareGreaterThan(double f1, double f2)
{
	return f1 > f2;
}

//-----------------------------------------------------------------------------------------------
// CFrameRateValue
//  This implementation of the performance value is used to track frame rate and discard samples that
// could mess up results
class CFrameRateValue :
	public CPerformanceValue
{
public:

	CFrameRateValue(const char* pszName, const char* pszGroup, const char* pszDescription) :
		CPerformanceValue(pszName, pszGroup, pszDescription)
	{
        //setup our initial framerate to be fairly high, so that it can converge
		static const float kfDefaultFPS = 100.0f;

		for(uint32 nCurrSample = 0; nCurrSample < knHistorySamples; nCurrSample++)
		{
			m_fHistory[nCurrSample] = kfDefaultFPS;
		}
	}

	//called to update the framerate given the provided frame time
	void	UpdateFrameRate(double fUpdateTimeS)
	{
		//convert the update time into fps
		double fUpdateFPS = 1.0 / fUpdateTimeS;

		//determine our current average frame rate and also make room for the new sample
		double fTotal = m_fHistory[0];
		for(uint32 nCurrSample = 0; nCurrSample < knHistorySamples - 1; nCurrSample++)
		{
			m_fHistory[nCurrSample] = m_fHistory[nCurrSample + 1];
			fTotal += m_fHistory[nCurrSample];
		}

		//we now have our average, allow this value to increase unbounded, but only allow for
		//a percentage of decay per frame in order to avoid hitches from causing lots of spikes

		//only allow the framerate to decrease for a sample 10% per frame
		static const double kfMaxFrameDecay = 0.9;	

		double fAverage		= fTotal / (double)knHistorySamples;
		double fMinSample	= fAverage * kfMaxFrameDecay;

		m_fHistory[knHistorySamples - 1] = LTMAX(fUpdateFPS, fMinSample);
	}

	virtual double GetValue() const
	{
		return m_fHistory[knHistorySamples - 1];
	}

private:

	//the number of samples we use for a history log
	static const uint32 knHistorySamples = 5;
	double		m_fHistory[knHistorySamples];
};

//-----------------------------------------------------------------------------------------------
// CFrameStatValue
//  This implementation of the performance value is used to track renderer frame stats
class CFrameStatValue :
	public CPerformanceValue
{
public:

	CFrameStatValue(ERendererFrameStats eFrameStat, double fValueScale, const char* pszName, const char* pszGroup, const char* pszDescription) :
		CPerformanceValue(pszName, pszGroup, pszDescription),
		m_eFrameStat(eFrameStat),
		m_fScale(fValueScale)
	{
	}

	virtual double GetValue() const
	{
		CRendererFrameStats* pFrameStats;
		g_pLTClient->GetRenderer()->GetFrameStats(pFrameStats);
		if(!pFrameStats)
			return 0.0;
		
		return (double)pFrameStats->GetFrameStat(m_eFrameStat) * m_fScale;
	}

private:

	//a scale for the number to allow for different values to be presented in different scales
	double				m_fScale;

	//the frame statistic that we track
	ERendererFrameStats m_eFrameStat;
};

//-----------------------------------------------------------------------------------------------
// CMemoryStatValue
//  This implementation of the performance value is used to track a memory statistic
class CMemoryStatValue :
	public CPerformanceValue
{
public:

	//the different memory stats we can track
	enum EMemoryStat
	{
		eMemStat_MemUsage,
		eMemStat_Allocations,
		eMemStat_TotalMemUsage,
		eMemStat_TotalAllocations,
		eMemStat_ChildMemUsage,
		eMemStat_ChildAllocations,
		eMemStat_ChildTotalMemUsage,
		eMemStat_ChildTotalAllocations,
	};

	CMemoryStatValue(uint32 nCategoryID, EMemoryStat eStat, const char* pszName, const char* pszGroup, const char* pszDescription) :
		CPerformanceValue(pszName, pszGroup, pszDescription),
		m_nCategoryID(nCategoryID),
		m_eMemoryStat(eStat)
	{
	}

	virtual double GetValue() const
	{
		//get our memory interface
		ILTMemory* pILTMem = LTMemGetILTMemory();
		if(!pILTMem)
			return 0.0;

		//get our category informaiton
		uint32 nMemUsage, nAllocations, nTotalMemUsage, nTotalAllocations;
		if(!pILTMem->GetCategorySelfStats(m_nCategoryID, &nAllocations, &nMemUsage, &nTotalAllocations, &nTotalMemUsage))
			return 0.0;

		uint32 nChildMemUsage, nChildAllocations, nChildTotalMemUsage, nChildTotalAllocations;
		if(!pILTMem->GetCategoryChildStats(m_nCategoryID, &nChildAllocations, &nChildMemUsage, &nChildTotalAllocations, &nChildTotalMemUsage))
			return 0.0;

		//constant for converting to megabytes
		static const double kfMegabyte = 1024.0 * 1024.0;

		switch(m_eMemoryStat)
		{
		case eMemStat_MemUsage:				return (double)nMemUsage / kfMegabyte;			break;
		case eMemStat_Allocations:			return (double)nAllocations;					break;
		case eMemStat_TotalMemUsage:		return (double)nTotalMemUsage / kfMegabyte;		break;
		case eMemStat_TotalAllocations:		return (double)nTotalAllocations;				break;
		case eMemStat_ChildMemUsage:		return (double)nChildMemUsage / kfMegabyte;		break;
		case eMemStat_ChildAllocations:		return (double)nChildAllocations;				break;
		case eMemStat_ChildTotalMemUsage:	return (double)nChildTotalMemUsage / kfMegabyte;break;
		case eMemStat_ChildTotalAllocations:return (double)nChildTotalAllocations;			break;
		default:							return 0.0;										break;
		}
	}

private:

	//the memory category that we track
	uint32		m_nCategoryID;

	//the memory stat that we track
	EMemoryStat	m_eMemoryStat;
};

//-----------------------------------------------------------------------------------------------
// CPerformanceMonitorStatValue
//  This implementation of the performance value is used to track performance monitor information
class CPerformanceMonitorStatValue :
	public CPerformanceValue
{
public:

	//the different performance stats we can track
	enum EPerformanceStat
	{
		eMemStat_FrameTime,
		eMemStat_NestedTime,
		eMemStat_FrameEntries,
	};

	CPerformanceMonitorStatValue(const char* pszSystem, EPerformanceStat eStat, const char* pszName, const char* pszGroup, const char* pszDescription) :
		CPerformanceValue(pszName, pszGroup, pszDescription),
		m_sSystem(pszSystem),
		m_ePerformanceStat(eStat)
	{
	}

	virtual double GetValue() const
	{
		//get our memory interface
		IPerformanceMonitor* pIPerfMon = GetPerformanceMonitor();
		if(!pIPerfMon)
			return 0.0;

		//get our category informaiton
		double fFrameTime, fNestedTime;
		uint64 nNumEntries;
		if(!pIPerfMon->GetSystemFrameStats(m_sSystem.c_str(), &fFrameTime, &fNestedTime, &nNumEntries))
			return 0.0;

		switch(m_ePerformanceStat)
		{
		case eMemStat_FrameTime:		return fFrameTime;			break;
		case eMemStat_NestedTime:		return fNestedTime;			break;
		case eMemStat_FrameEntries:		return (double)nNumEntries;	break;
		default:						return 0.0;					break;
		}
	}

private:

	//the performance system we are tracking
	std::string			m_sSystem;

	//the performance stat that we track
	EPerformanceStat	m_ePerformanceStat;
};

//-----------------------------------------------------------------------------------------------
// CPerformanceValueMonitor
CPerformanceValueMonitor::CPerformanceValueMonitor() :
	m_pfnCB(NULL),
	m_fTriggerElapse(0.0),
	m_fRemainingTriggerElapse(0.0),
	m_fThreshold(0.0)
{
}

CPerformanceValueMonitor::~CPerformanceValueMonitor()
{
}

//called to update the performance value monitor by the specified time interval, and also
//to determine if this monitor should be triggered or not
bool CPerformanceValueMonitor::Update(double fElapsedTime)
{
	//update our remaining trigger elapse if we are positive, but make sure it doesn't
	//go negative
	if(m_fRemainingTriggerElapse > 0.0)
		m_fRemainingTriggerElapse = LTMAX(0.0, m_fRemainingTriggerElapse - fElapsedTime);

	//see if we are waiting for a trigger elapse
	if(m_fRemainingTriggerElapse != 0.0)
		return false;

	//we do not need to wait for an elapse, see if we have a valid callback
	if(!m_pfnCB)
		return false;

	//accumulate all of our performance vaues
	double fAccumVal = 0.0;
	for(TPerformanceValueList::const_iterator itVal = m_Values.begin(); itVal != m_Values.end(); itVal++)
	{
		fAccumVal += (*itVal)->GetValue();
	}

	//see if we triggered
	if(!m_pfnCB(fAccumVal, m_fThreshold))
		return false;
		
	//we triggered, so update our elapsed time and indicate success
	m_fRemainingTriggerElapse = m_fTriggerElapse;
	return true;
}

//called to clear out the list of values that we are tracking
void CPerformanceValueMonitor::ClearValues()
{
	m_Values.clear();
}

//called to add a performance value to the list that should be monitored
void CPerformanceValueMonitor::AddValue(const CPerformanceValue* pValue)
{
	//only add valid values
	if(pValue)
		m_Values.push_back(pValue);
}


//-----------------------------------------------------------------------------------------------
// CPerformanceValueDB

CPerformanceValueDB::CPerformanceValueDB()
{
}

CPerformanceValueDB::~CPerformanceValueDB()
{
	Term();
}

//called to delete and clear out all performance values
void CPerformanceValueDB::Term()
{
	for(TPerformanceValueList::iterator itVal = m_ValueList.begin(); itVal != m_ValueList.end(); itVal++)
	{
		delete *itVal;
	}
	m_ValueList.clear();
}

//called to register a performance value with the database, this database will take ownership
//of this value and will handle deleting it
bool CPerformanceValueDB::AddPerformanceValue(CPerformanceValue* pPerfValue)
{
	if(!pPerfValue)
		return false;

	m_ValueList.push_back(pPerfValue);
	return true;
}

//called to determine the number of performance values that are currently registered
uint32 CPerformanceValueDB::GetNumPerformanceValues() const
{
	return (uint32)m_ValueList.size();
}

//called to get the specified performance value by index, this will return NULL if the index is out
//of range
const CPerformanceValue* CPerformanceValueDB::GetPerformanceValue(uint32 nIndex) const
{
	if(nIndex >= m_ValueList.size())
		return NULL;

	return m_ValueList[nIndex];
}

//called to find the performance value by name, this is case insensitive and will return NULL if
//no match is found
const CPerformanceValue* CPerformanceValueDB::FindPerformanceValue(const char* pszName) const
{
	for(TPerformanceValueList::const_iterator itVal = m_ValueList.begin(); itVal != m_ValueList.end(); itVal++)
	{
		if(LTStrIEquals(pszName, (*itVal)->GetName()))
			return *itVal;
	}

	//no match
	return NULL;
}

//-----------------------------------------------------------------------------------------------
// CPerformanceLog

CPerformanceLog::CPerformanceLog() :
	m_fLogFrequency(0.0),
	m_fElapsedLogTime(0.0),
	m_nCurrLogSample(0),
	m_bSkipNextUpdate(true),
	m_pFrameRateValue(NULL)
{
}

CPerformanceLog::~CPerformanceLog()
{
	Term();
}

//singleton support
CPerformanceLog& CPerformanceLog::Singleton()
{
	static CPerformanceLog s_Singleton;
	return s_Singleton;
}

//called to initialize the performance values
void CPerformanceLog::Init()
{
	CreateCommonPerformanceValues();
	CreateFrameStatPerformanceValues();
	CreateMemoryPerformanceValues();
	CreatePerfMonPerformanceValues();

	//install our console program
	g_pLTClient->RegisterConsoleProgram("PerfLog", ConsoleCommandHandlerCB);
}

//called to remove all performance values and monitors
void CPerformanceLog::Term()
{
	//remove our console program
	g_pLTClient->UnregisterConsoleProgram("PerfLog");

	//stop any logging that may be going on
	StopLog();

	//clear out our monitor list
	for(TMonitorList::iterator itMonitor = m_Monitors.begin(); itMonitor != m_Monitors.end(); itMonitor++)
	{
		delete *itMonitor;
	}
	m_Monitors.clear();

	//clear out our value database
	m_ValueDB.Term();

	//and our frame rate perf val which was in the database
	m_pFrameRateValue = NULL;
}

//called to set the name of the current level, which is used for report information
void CPerformanceLog::SetCurrentLevel(const char* pszLevel)
{
	//we want to strop the path off of our provided level
	uint32 nCopyFrom = 0;
	for(uint32 nCurrChar = 0; pszLevel[nCurrChar] != '\0'; nCurrChar++)
	{
		if((pszLevel[nCurrChar] == '\\') || (pszLevel[nCurrChar] == '/'))
			nCopyFrom = nCurrChar + 1;
	}

	//save our level name
	m_sCurrentLevel = pszLevel + nCopyFrom;	

	//and strip off our extension
	CResExtUtil::StripFileExtension(m_sCurrentLevel);
}

//called to update all of the performance monitors and logs
void CPerformanceLog::Update(double fFrameTimeS)
{
	//see if we should skip this update (we may have done something last frame to skew the results
	//significantly)
	if(m_bSkipNextUpdate)
	{
		m_bSkipNextUpdate = false;
		return;
	}

	//handle updating our log
	UpdateLog(fFrameTimeS);

	//handle updating our FPS value
	if((fFrameTimeS > 0.001) && m_pFrameRateValue)
	{
		m_pFrameRateValue->UpdateFrameRate(fFrameTimeS);
	}

	//handle updating of all our monitors, and see if any trigger, and add them onto our monitor list
	//if they trigger
	TMonitorList TriggeredList;
	for(TMonitorList::iterator itMonitor = m_Monitors.begin(); itMonitor != m_Monitors.end(); itMonitor++)
	{
		CPerformanceValueMonitor* pMonitor = *itMonitor;
		if(pMonitor->Update(fFrameTimeS))
		{
			TriggeredList.push_back(pMonitor);
		}
	}

	//now if we triggered, generate a report
	if(!TriggeredList.empty())
		GenerateReport(&TriggeredList);
}

//called to obtain the database of performance values for this performance log
const CPerformanceValueDB& CPerformanceLog::GetValueDB() const
{
	return m_ValueDB;
}

//----------------------------
// Log support

//called to start logging the specified listing of performance values. This will close any
//currently opened logs
bool CPerformanceLog::StartLog(const char* pszLogFile, double fSampleFrequencyS, uint32 nNumValues,  const CPerformanceValue* const* ppValues)
{
	//stop any currently open log
	StopLog();

	//clear out any error state that could be left over from the last log
	m_LogFile.clear();

	//and try to open our new log file
	m_LogFile.open(pszLogFile, std::ios::out);
	if(!m_LogFile.good())
	{
		StopLog();
		return false;
	}

	//save our list of values to track
	for(uint32 nCurrValue = 0; nCurrValue < nNumValues; nCurrValue++)
	{
		if(ppValues[nCurrValue])
			m_LogValues.push_back(ppValues[nCurrValue]);
	}

	//make sure they provided a value
	if(m_LogValues.empty())
	{
		StopLog();
		return false;
	}

	//we have a valid log, so write out the header
	m_LogFile << "Sample";
	for(TLogValueList::const_iterator itLogVal = m_LogValues.begin(); itLogVal < m_LogValues.end(); itLogVal++)
	{
		m_LogFile << ", " << (*itLogVal)->GetName();
	}

	//and terminate that line so the samples will start on a fresh line
	m_LogFile << std::endl;

	//and reset our timing and sequence information (use a fully elapsed time so the first update will
	//be logged)
	m_fLogFrequency		= LTMAX(fSampleFrequencyS, 0.0);
	m_fElapsedLogTime	= m_fLogFrequency;
	m_nCurrLogSample	= 0;

	return true;
}

//called to stop any currently opened logs
void CPerformanceLog::StopLog()
{
	//close any currently open file
	m_LogFile.close();

	//and clear our list of tracked values
	m_LogValues.clear();
}

//called to update the log based upon the provided time interval
void CPerformanceLog::UpdateLog(double fFrameTimeS)
{
	//first off see if we are even logging!
	if(!m_LogFile.good() || m_LogValues.empty())
		return;

	//see if we need to wait for our next sample
	m_fElapsedLogTime += fFrameTimeS;
	if(m_fElapsedLogTime < m_fLogFrequency)
		return;

	//we need to reset our time until the next sample
	m_fElapsedLogTime = 0.0;

	//and write out our sample
	m_LogFile << m_nCurrLogSample;
	for(TLogValueList::const_iterator itLogVal = m_LogValues.begin(); itLogVal < m_LogValues.end(); itLogVal++)
	{
		m_LogFile << ", " << (*itLogVal)->GetValue();
	}
	m_LogFile << std::endl;

	//update our sample number for the next sample
	m_nCurrLogSample++;
}

//----------------------------
// Performance Value Creation

//called to create the common performance values
void CPerformanceLog::CreateCommonPerformanceValues()
{
	//frame rate
	m_pFrameRateValue = new CFrameRateValue("FrameRate", "Common", "The number of frames per second that the game is running at.");
	m_ValueDB.AddPerformanceValue(m_pFrameRateValue);

	//total memory usage
	m_ValueDB.AddPerformanceValue(new CMemoryStatValue(LT_MEM_TYPE_ALL, CMemoryStatValue::eMemStat_ChildMemUsage, "TotalMemory", "Common", "The total memory in megabytes that the game has currently allocated"));
	//texture memory usage
	m_ValueDB.AddPerformanceValue(new CMemoryStatValue(LT_MEM_TYPE_RENDER_TEXTURE, CMemoryStatValue::eMemStat_MemUsage, "TextureMemory", "Common", "The total memory in megabytes that the game has currently allocated for textures"));
	//mesh memory usage
	m_ValueDB.AddPerformanceValue(new CMemoryStatValue(LT_MEM_TYPE_RENDER_MESH, CMemoryStatValue::eMemStat_MemUsage, "MeshMemory", "Common", "The total memory in megabytes that the game has currently allocated for model and world meshes"));
	//sound memory usage
	m_ValueDB.AddPerformanceValue(new CMemoryStatValue(LT_MEM_TYPE_SOUND, CMemoryStatValue::eMemStat_MemUsage, "SoundMemory", "Common", "The total memory in megabytes that the game has currently allocated for sounds"));
}

//called to create the frame stat performance values
void CPerformanceLog::CreateFrameStatPerformanceValues()
{
	//common scales
	static const double kfNoScale = 1.0;
	static const double kfInvMeg = 1.0 / (1024.0 * 1024.0);

	//the listing of the frame stats that we support
	struct SFrameStatInfo
	{
		ERendererFrameStats		m_eFrameStat;
		double					m_fScale;
		const char*				m_pszName;
		const char*				m_pszCategory;
		const char*				m_pszDescription;
	};

	SFrameStatInfo FrameStatInfo[] = 
	{
		//texture memory amounts
		{ eFS_VidMemory_Textures,				kfInvMeg, "VidMemory_Textures",				"VideoMem",		"Shows the amount of video memory that was referenced by textures for this frame" },
		{ eFS_VidMemory_TexturesUncompressed,	kfInvMeg, "VidMemory_TexturesUncompressed",	"VideoMem",		"Shows the amount of video memory that was referenced for this frame if had been all uncompressed" },
		{ eFS_VidMemory_SystemTextures,			kfInvMeg, "VidMemory_SystemTextures",		"VideoMem",		"Shows the amount of texture memory that was referenced by internal engine textures for this frame" },
		{ eFS_VidMemory_ScreenBuffers,			kfInvMeg, "VidMemory_ScreenBuffers",		"VideoMem",		"Shows the amount of texture memory that was referenced by screen for this frame for the current image, back buffer, and depth buffer" },
		{ eFS_VidMemory_RenderTargets,			kfInvMeg, "VidMemory_RenderTargets",		"VideoMem",		"Shows the amount of texture memory that was referenced by render targets, including surfaces such as the fog surfacess or previous frame maps" },
		{ eFS_VidMemory_ApproxAvailable,		kfInvMeg, "VidMemory_ApproxAvailable",		"VideoMem",		"A very inaccurate estimate of the remaining free video memory on the video card" },
		{ eFS_VidMemory_VertexBuffers,			kfInvMeg, "VidMemory_VertexBuffers",		"VideoMem",		"Shows the size of the vertex buffers that were referenced for this frame" },
		{ eFS_VidMemory_IndexBuffers,			kfInvMeg, "VidMemory_IndexBuffers",			"VideoMem",		"Shows the size of the index buffers that were referenced for this frame" },

		//mesh rendering counts
		{ eFS_NumMeshes_World,				kfNoScale, "Meshes_World",				"Meshes",	"The number of meshes that were used for rendering the world this frame" },
		{ eFS_NumMeshes_WorldShadow,		kfNoScale, "Meshes_WorldShadow",		"Meshes",	"The number of meshes that were used for rendering the world shadow this frame" },
		{ eFS_NumMeshes_Model,				kfNoScale, "Meshes_Model",				"Meshes",	"The number of meshes that were used for rendering models this frame" },
		{ eFS_NumMeshes_ModelShadow,		kfNoScale, "Meshes_ModelShadow",		"Meshes",	"The number of meshes that were used for rendering model shadows this frame" },
		{ eFS_NumMeshes_WorldModel,			kfNoScale, "Meshes_WorldModels",		"Meshes",	"The number of meshes that were used for rendering world models this frame" },
		{ eFS_NumMeshes_WorldModelShadow,	kfNoScale, "Meshes_WorldModelShadow",	"Meshes",	"The number of meshes that were used for rendering world model shadows this frame" },
		{ eFS_NumMeshes_CustomRender,		kfNoScale, "Meshes_CustomRender",		"Meshes",	"The number of meshes that were used for rendering custom render objects this frame" },

		//world rendering counts
		{ eFS_WorldRender_SkyPortals,		kfNoScale, "Render_SkyPortals",			"Render",	"The number of sky portals that were in view this frame" },

		//rendering counts
		{ eFS_Render_MaterialChanges,		kfNoScale, "Render_ShaderChanges",		"Render",	"The number of times the shader had to be changed to render this frame" },
		{ eFS_Render_MaterialInstanceChanges,kfNoScale, "Render_MaterialChanges",	"Render",	"The number of times the active material had to be changed to render this scene" },
		{ eFS_Render_PassChanges,			kfNoScale, "Render_PassChanges",		"Render",	"The number of times the active pass had to be changed (includes number of shader changes)" },
		{ eFS_Render_ObjectChanges,			kfNoScale, "Render_ObjectChanges",		"Render",	"The number of times the active object had to be changed to render this scene" },
		{ eFS_Render_RenderCalls,			kfNoScale, "Render_RenderCalls",		"Render",	"The number of batches that had to be submitted to render this scene" },
		{ eFS_Render_BatchedMeshes,			kfNoScale, "Render_BatchedMeshes",		"Render",	"The number of meshes that could be batched and therefore avoided batch submissions" },
		{ eFS_Render_MissedBatchedMeshes,	kfNoScale, "Render_MissedBatches",		"Render",	"The number of batches that had to be unnecessarily submitted because of the layout of the meshes" },

		//triangle counts
		{ eFS_Triangles_Model,				kfNoScale, "Tris_Model",			"Tris",	"The number of triangles rendered for models this frame" },
		{ eFS_Triangles_ModelShadow,		kfNoScale, "Tris_ModelShadow",		"Tris",	"The number of triangles rendered for model shadows this frame" },
		{ eFS_Triangles_World,				kfNoScale, "Tris_World",			"Tris",	"The number of triangles rendered for the world this frame" },
		{ eFS_Triangles_WorldShadow,		kfNoScale, "Tris_WorldShadow",		"Tris",	"The number of triangles rendered for the world shadows frame" },
		{ eFS_Triangles_WorldModel,			kfNoScale, "Tris_WorldModel",		"Tris",	"The number of triangles rendered for world models this frame" },
		{ eFS_Triangles_WorldModelShadow,	kfNoScale, "Tris_WorldModelShadow",	"Tris",	"The number of triangles rendered for world model shadows this frame" },
		{ eFS_Triangles_CustomRender,		kfNoScale, "Tris_CustomRender",		"Tris",	"The number of triangles rendered for custom render objects this frame" },

		//visibility
		{ eFS_Vis_SectorsVisible,			kfNoScale, "Vis_Sectors",			"Vis",	"The number of sectors that are directly visible from cameras rendered this frame" },
		{ eFS_Vis_ObjectsVisible,			kfNoScale, "Vis_Objects",			"Vis",	"The number of objects that are directly visible from cameras rendered this frame" },
		{ eFS_Vis_TotalSectorsVisited,		kfNoScale, "Vis_SectorsVisited",	"Vis",	"The number of sectors that were traversed for all cameras and lights this frame" },
		{ eFS_Vis_TotalPortalsTested,		kfNoScale, "Vis_PortalsTested",		"Vis",	"The number of portals that were tested for all cameras and lights this frame" },
		{ eFS_Vis_TotalPortalsVisible,		kfNoScale, "Vis_PortalsVisible",	"Vis",	"The number of portals that were tested and visible for all cameras and lights this frame" },
		{ eFS_Vis_TotalObjectsTested,		kfNoScale, "Vis_ObjectsTested",		"Vis",	"The number of objects that were tested for all cameras and lights this frame" },
		{ eFS_Vis_TotalObjectsVisible,		kfNoScale, "Vis_ObjectsVisible",	"Vis",	"The number of objects that were tested and visible for all cameras and lights this frame" },
		{ eFS_Vis_VisQueries,				kfNoScale, "Vis_VisQueries",		"Vis",	"The number of visibility queries that were submitted, typically there is one per camera and light rendered" },

		//visible objects
		{ eFS_VisObjects_Models,			kfNoScale, "VisObjects_Models",			"VisObjects",	"The number of models that were directly visible in this frame" },
		{ eFS_VisObjects_WorldModels,		kfNoScale, "VisObjects_WorldModels",	"VisObjects",	"The number of world models that were directly visible in this frame" },
		{ eFS_VisObjects_CustomRender,		kfNoScale, "VisObjects_CustomRenders",	"VisObjects",	"The number of custom render objects that were directly visible in this frame" },

		//light counts
		{ eFS_Light_PointLights,			kfNoScale, "Light_Point",				"Light",	"The number of point lights that were rendered this frame" },
		{ eFS_Light_PointLightShadows,		kfNoScale, "Light_PointShadows",		"Light",	"The number of shadow casting point lights that were rendered this frame" },
		{ eFS_Light_PointFillLights,		kfNoScale, "Light_PointFill",			"Light",	"The number of point fill lights that were rendered this frame" },
		{ eFS_Light_SpotProjectors,			kfNoScale, "Light_Spot",				"Light",	"The number of spot projector lights that were rendered this frame" },
		{ eFS_Light_SpotProjectorShadows,	kfNoScale, "Light_SpotShadows",			"Light",	"The number of shadow casting spot projector lights that were rendered this frame" },
		{ eFS_Light_CubeProjectors,			kfNoScale, "Light_Cube",				"Light",	"The number of cube projector lights that were rendered this frame" },
		{ eFS_Light_CubeProjectorShadows,	kfNoScale, "Light_CubeShadows",			"Light",	"The number of shadow casting cube projector lights that were rendered this frame" },
		{ eFS_Light_Directionals,			kfNoScale, "Light_Directional",			"Light",	"The number of directional lights that were rendered this frame" },
		{ eFS_Light_DirectionalShadows,		kfNoScale, "Light_DirectionalShadows",	"Light",	"The number of shadow casting directional lights that were rendered this frame" },
		{ eFS_Light_BlackLights,			kfNoScale, "Light_BlackLight",			"Light",	"The number of black lights that were rendered this frame" },
		{ eFS_Light_BlackLightShadows,		kfNoScale, "Light_BlackLightShadows",	"Light",	"The number of shadow casting black lights that were rendered this frame" },
	};

	for(uint32 nCurrFS = 0; nCurrFS < LTARRAYSIZE(FrameStatInfo); nCurrFS++)
	{
		SFrameStatInfo& Info = FrameStatInfo[nCurrFS];
		m_ValueDB.AddPerformanceValue(new CFrameStatValue(Info.m_eFrameStat, Info.m_fScale, Info.m_pszName, Info.m_pszCategory, Info.m_pszDescription));
	}
}

//called to create the memory performance values
void CPerformanceLog::CreateMemoryPerformanceValues()
{
	ILTMemory* pLTMemory = LTMemGetILTMemory();
	if(!pLTMemory)
		return;

	uint32 nNumCategories = pLTMemory->GetNumCategories();

	//run through each available memory category, and setup a memory value
	for(uint32 nCurrCategory = 0; nCurrCategory < nNumCategories; nCurrCategory++)
	{
		//get the category ID
		uint32 nCategoryID = pLTMemory->GetCategoryID(nCurrCategory);

		//now get the name of this category
		const char* pszCategoryName = pLTMemory->GetCategoryName(nCategoryID);
		if(!pszCategoryName)
			continue;

		char pszValueName[64];
		//setup a value to track memory usage for this category
		LTSNPrintF(pszValueName, LTARRAYSIZE(pszValueName), "%sMemory", pszCategoryName);
		m_ValueDB.AddPerformanceValue(new CMemoryStatValue(nCategoryID, CMemoryStatValue::eMemStat_MemUsage, pszValueName, "Memory", "The total memory in megabytes that the game has currently allocated for this memory category"));
	}

	//run through each available memory category, and setup an allocation value
	for(uint32 nCurrCategory = 0; nCurrCategory < nNumCategories; nCurrCategory++)
	{
		//get the category ID
		uint32 nCategoryID = pLTMemory->GetCategoryID(nCurrCategory);

		//now get the name of this category
		const char* pszCategoryName = pLTMemory->GetCategoryName(nCategoryID);
		if(!pszCategoryName)
			continue;

		char pszValueName[64];
		//setup a value to track allocations for this category
		LTSNPrintF(pszValueName, LTARRAYSIZE(pszValueName), "%sAllocs", pszCategoryName);
		m_ValueDB.AddPerformanceValue(new CMemoryStatValue(nCategoryID, CMemoryStatValue::eMemStat_Allocations, pszValueName, "Allocations", "The total number of allocations that the game has currently allocated for this memory category"));
	}
}

//called to create the performance monitor performance values
void CPerformanceLog::CreatePerfMonPerformanceValues()
{
	IPerformanceMonitor* pIPerfMon = GetPerformanceMonitor();
	if(!pIPerfMon)
		return;

	uint32 nNumSystems = pIPerfMon->GetNumSystems();
	
	//run through each available performance category for the timings
	for(uint32 nCurrSystem = 0; nCurrSystem < nNumSystems; nCurrSystem++)
	{
		//get the name of this system
		const char* pszSystem = pIPerfMon->GetSystem(nCurrSystem);
		if(!pszSystem)
			continue;

		//add the common performance values
		char pszValueName[64];
		//setup a value to track the frame time in this system
		LTSNPrintF(pszValueName, LTARRAYSIZE(pszValueName), "%sTime", pszSystem);
		m_ValueDB.AddPerformanceValue(new CPerformanceMonitorStatValue(pszSystem, CPerformanceMonitorStatValue::eMemStat_FrameTime, pszValueName, "PerfMonTime", "The amount of time in milliseconds spent inside of this system during this frame"));
	}

	//run through each available performance category for the entries
	for(uint32 nCurrSystem = 0; nCurrSystem < nNumSystems; nCurrSystem++)
	{
		//get the name of this system
		const char* pszSystem = pIPerfMon->GetSystem(nCurrSystem);
		if(!pszSystem)
			continue;

		//add the common performance values
		char pszValueName[64];
		//setup a value to track the number of entries into this system
		LTSNPrintF(pszValueName, LTARRAYSIZE(pszValueName), "%sEntries", pszSystem);
		m_ValueDB.AddPerformanceValue(new CPerformanceMonitorStatValue(pszSystem, CPerformanceMonitorStatValue::eMemStat_FrameEntries, pszValueName, "PerfMonEntries", "The number of times this system was entered during this frame"));
	}
}


//----------------------------
// Report support

//called to generate a report to the specified file. This will append the log onto the file if
//it already exists.
bool CPerformanceLog::GenerateReport(const char* pszReportFile, const TMonitorList* pMonitorList)
{
	//get the directory of this report and make sure that it exists
	char pszDirectory[MAX_PATH];
	ExtractFilePath(pszReportFile, pszDirectory, LTARRAYSIZE(pszDirectory));

	//make sure that this directory exists
	if(!CreateDirectory(pszDirectory))
		return false;

	//try and open the file in append mode
	std::ofstream OutFile;
	OutFile.open(pszReportFile, std::ios::out | std::ios::app);
	if(!OutFile.good())
		return false;

	//now we can generate our report, which is an HTML format and laid out in a single table

	//start our table
	OutFile << "<table border=\"0\" width=\"100%\" cellpadding=\"4\" cellspacing=\"1\">" << std::endl;

	//generte our screenshot
	CreateReportScreenshot(OutFile, pszDirectory);

	//generate our header information
	CreateReportHeader(OutFile);	

	//now we need to actually write out each value in our database

	//keep track of our currently active category
	const char* pszCurrCategory = "";
	for(uint32 nCurrValue = 0; nCurrValue < m_ValueDB.GetNumPerformanceValues(); nCurrValue++)
	{
		//get our performance value
		const CPerformanceValue* pValue = m_ValueDB.GetPerformanceValue(nCurrValue);

		//now determine if this performance value is in one of the provided monitors and if it is
		//color it differently
		bool bHighlight = false;
		if(pMonitorList)
		{
			for(TMonitorList::const_iterator itMonitor = pMonitorList->begin(); itMonitor != pMonitorList->end() && !bHighlight; itMonitor++)
			{
				//cache our monitor
				const CPerformanceValueMonitor* pMonitor = *itMonitor;
				if(!pMonitor)
					continue;

				//run through the values that are monitored
				for(uint32 nCurrMonValue = 0; nCurrMonValue < pMonitor->GetNumValues(); nCurrMonValue++)
				{
					//if this is our value, we want to highlight it
					if(pMonitor->GetValue(nCurrMonValue) == pValue)
					{
						bHighlight = true;
						break;
					}
				}
			}
		}

		//see if we are starting a new category
		if(!LTStrIEquals(pValue->GetCategory(), pszCurrCategory))
		{
			//different category, write out our category header
			OutFile << "  <tr>" << std::endl;
			OutFile << "    <td colspan=\"3\" bgcolor=\"" << HTML_CATEGORY_BACKGROUND << "\" align=\"left\">" << std::endl;
			OutFile << "      <font " << HTML_NORMAL_FONT_PROPERTIES << "><B>" << std::endl;
			OutFile << "        " << pValue->GetCategory() << std::endl;
			OutFile << "      </font></B>" << std::endl;
			OutFile << "    </td>" << std::endl;
			OutFile << "  </tr>" << std::endl;

			//and update our current category
			pszCurrCategory = pValue->GetCategory();
		}

		//format the value so we don't get exponential formatting
		char pszValue[64];
		LTSNPrintF(pszValue, LTARRAYSIZE(pszValue), "%.2f", pValue->GetValue());

		//determine the color of this font (if it is highlighted use red, otherwise black)
		const char* pszFontColor = (bHighlight) ? "#ff0000" : "#000000";

		//determine the background color (alternate between white and grey to make it more legible)
		const char* pszBGColor = (nCurrValue % 2) ? HTML_VALUE_BACKGROUND1 : HTML_VALUE_BACKGROUND2;


		//now write out this value
		OutFile << "  <tr>" << std::endl;
		OutFile << "    <td bgcolor=\"" << pszBGColor << "\">" << std::endl;
		OutFile << "      <font " << HTML_NORMAL_FONT_PROPERTIES << " color=\"" << pszFontColor << "\">" << std::endl;
		OutFile << "        " << pValue->GetName() << std::endl;
		OutFile << "      </font>" << std::endl;
		OutFile << "    </td>" << std::endl;
		OutFile << "    <td align=\"right\" bgcolor=\"" << pszBGColor << "\">" << std::endl;
		OutFile << "      <font " << HTML_NORMAL_FONT_PROPERTIES << " color=\"" << pszFontColor << "\">" << std::endl;
		OutFile << "        " << pszValue << std::endl;
		OutFile << "      </font>" << std::endl;
		OutFile << "    </td>" << std::endl;
		OutFile << "    <td bgcolor=\"" << pszBGColor << "\">" << std::endl;
		OutFile << "      <font " << HTML_SMALL_FONT_PROPERTIES << " color=\"" << HTML_DESCRIPTION_COLOR << "\">" << std::endl;
		OutFile << "        " << pValue->GetDescription() << std::endl;
		OutFile << "      </font>" << std::endl;
		OutFile << "    </td>" << std::endl;
		OutFile << "  </tr>" << std::endl;
	}

	//end our table
	OutFile << "</table>" << std::endl;

	//and place a break to make it a little easier to separate reports
	OutFile << "<br><hr><br>" << std::endl;

	//success
	return true;
}

//called to generate a report based upon the current level
bool CPerformanceLog::GenerateReport(const TMonitorList* pMonitorList)
{
	//determine the name of the report file for this level
	char pszReportFile[MAX_PATH];
	GetCurrentLevelReportFileName(pszReportFile, LTARRAYSIZE(pszReportFile));

	//and generate it
	return GenerateReport(pszReportFile, pMonitorList);
}

//using the current level name, this will determine the name of the report file that should be used
void CPerformanceLog::GetCurrentLevelReportFileName(char* pszBuffer, uint32 nBufferLen)
{
	const char* pszLevelName = m_sCurrentLevel.empty() ? "General" : m_sCurrentLevel.c_str();

	//build up the relative filename
	char pszRelative[MAX_PATH];
	LTSNPrintF(pszRelative, LTARRAYSIZE(pszRelative), "PerfReports\\%s\\%s.html", pszLevelName, pszLevelName);

	//and now create the full user file path
	g_pLTClient->FileMgr()->GetAbsoluteUserFileName(pszRelative, pszBuffer, nBufferLen);
}

//using the current level name and other properties, this will determine the name of the screenshot
//image that should be used for a report
void CPerformanceLog::GetScreenShotFileName(char* pszBuffer, uint32 nBufferLen)
{
	const char* pszLevelName = m_sCurrentLevel.empty() ? "General" : m_sCurrentLevel.c_str();

	//get our second time and ms time
	time_t CurrTime = time(NULL);
	struct tm* pCurrTime = localtime(&CurrTime);
	clock_t tMS = clock();
	
	char pszTimeInfo[MAX_PATH];
	if(pCurrTime)
		strftime(pszTimeInfo, LTARRAYSIZE(pszTimeInfo), "_%b-%d-%y_%H-%M-%S_", pCurrTime); 
	else
		LTStrCpy(pszTimeInfo, "", LTARRAYSIZE(pszTimeInfo));

	//now tack on the date and time
	LTSNPrintF(pszBuffer, nBufferLen, "%s%s%d.jpg", pszLevelName, pszTimeInfo, (uint32)tMS);
}

//given a filename, this will extract the directory out of this file 
void CPerformanceLog::ExtractFilePath(const char* pszFilename, char* pszBuffer, uint32 nBufferLen)
{
	//run through and copy up to the first separator we encounter
	uint32 nCopyTo = 0;
	for(uint32 nCurrChar = 0; pszFilename[nCurrChar] != '\0'; nCurrChar++)
	{
		if((pszFilename[nCurrChar] == '\\') || (pszFilename[nCurrChar] == '/'))
			nCopyTo = nCurrChar;
	}

	LTSubStrCpy(pszBuffer, pszFilename, nBufferLen, nCopyTo);
	LTStrCat(pszBuffer, "\\", nBufferLen);
}

//given a directory, this will make sure that it exists so that files can be created inside of it
bool CPerformanceLog::CreateDirectory(const char* pszDirectory)
{
	if (!CWinUtil::DirExist(pszDirectory))
	{
		return CWinUtil::CreateDir(pszDirectory);
	}

	return true;
}

//called to generate the report screenshot, and if successful, write out the screenshot into the
//table
void CPerformanceLog::CreateReportScreenshot(std::ofstream& OutFile, const char* pszDirectory)
{
	//try and generate a screenshot for this
	char pszScreenshot[MAX_PATH];
	GetScreenShotFileName(pszScreenshot, LTARRAYSIZE(pszScreenshot));

	//create an absolute screenshot name
	char pszAbsoluteScreenshot[MAX_PATH];
	LTSNPrintF(pszAbsoluteScreenshot, LTARRAYSIZE(pszAbsoluteScreenshot), "%s%s", pszDirectory, pszScreenshot);

	//try and capture this screenshot
	if(g_pLTClient->GetRenderer()->MakeScreenShot(pszAbsoluteScreenshot) == LT_OK)
	{
		//we successfully captured a screenshot, so add this to the report
		OutFile << "  <tr>" << std::endl;
		OutFile << "    <td colspan=\"3\">" << std::endl;
		OutFile << "      <img src=\"" << pszScreenshot << "\" alt=\"Screenshot of time of report\">" << std::endl;
		OutFile << "    </td>" << std::endl;
		OutFile << "  </tr>" << std::endl;
	}
}

//called to create the base header information for a report, such as the position, build, time, etc
void CPerformanceLog::CreateReportHeader(std::ofstream& OutFile)
{
	time_t CurrTime = time(NULL);
	struct tm* pCurrTime = localtime(&CurrTime);
    
	//create our time structures
	char pszTime[256];
	if(pCurrTime)
		strftime(pszTime, LTARRAYSIZE(pszTime), "%I:%M%p - %A, %B %d, %Y ", pCurrTime);
	else
		LTStrCpy(pszTime, "Unable to determine time", LTARRAYSIZE(pszTime));

	//get the camera position and make sure that it is in the same space as the source level
	LTVector vWorldOffset;
	g_pLTClient->GetSourceWorldOffset(vWorldOffset);
	LTVector vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos() + vWorldOffset;

	//and format our camera string
	char pszCameraPos[64];
	LTSNPrintF(pszCameraPos, LTARRAYSIZE(pszCameraPos), "(%.2f, %.2f, %.2f)", VEC_EXPAND(vCameraPos));
	
	//now we need to write out the basic information that we have to allow easier reproduction of issues
	OutFile << "  <tr>" << std::endl;
	OutFile << "    <td colspan=\"3\" bgcolor=\"" << HTML_VALUE_BACKGROUND1 << "\">" << std::endl;
	OutFile << "      <font " << HTML_NORMAL_FONT_PROPERTIES << ">" << std::endl;
	OutFile << "        Level: " << m_sCurrentLevel.c_str() << "<br>" << std::endl;
	OutFile << "        Time: " << pszTime << "<br>"  << std::endl;
	OutFile << "        Build: " << g_pVersionMgr->GetBuild() << "<br>" << std::endl;
	OutFile << "        Camera: " << pszCameraPos << "<br>" <<std::endl;
	OutFile << "      </font>" << std::endl;
	OutFile << "    </td>" << std::endl;
	OutFile << "  </tr>" << std::endl;
}

//----------------------------
// Monitor support

//called to register a monitor object with the performance log. This will be updated each frame
//and when triggered will generate a report. This will take ownership of the monitor.
bool CPerformanceLog::AddMonitor(CPerformanceValueMonitor* pMonitor)
{
	//make sure that the monitor is valid
	if(!pMonitor)
		return false;

	//add it to our list
	m_Monitors.push_back(pMonitor);
	return true;
}

//called to remove all performance monitors with the provided name, this will return whether 
//or not a performance monitor with that name was found
bool CPerformanceLog::RemoveMonitor(const char* pszMonitorName)
{
	//keep track of whether or not we found a match
	bool bFoundMatch = false;

	for(TMonitorList::iterator itMonitor = m_Monitors.begin(); itMonitor != m_Monitors.end(); )
	{
		//see if this monitor matches in name
		if(LTStrIEquals(pszMonitorName, (*itMonitor)->m_sName.c_str()))
		{
			//it does, so erase it, but keep looking for possible other monitors with the same name
			bFoundMatch = true;
			delete *itMonitor;
			itMonitor = m_Monitors.erase(itMonitor);
		}
		else
		{
			//it doesn't match, so move on in the list
			itMonitor++;
		}
	}

	return bFoundMatch;
}

//---------------------------------
// Console Commands
void CPerformanceLog::ConsoleCommandHandlerCB(int nArgs, char** ppszArgs)
{
	CPerformanceLog::Singleton().HandleConsoleCommand((uint32)nArgs, ppszArgs);
}

//called to handle the provided console command
void CPerformanceLog::HandleConsoleCommand(uint32 nNumArgs, const char* const* ppszArgs)
{
	//handle if no command was specified
	if(nNumArgs == 0)
	{
		DisplayMainHelp();
		return;
	}

	//dispatch the command
	const char* pszCommand = ppszArgs[0];

	if(LTStrIEquals(pszCommand, "Help"))
		HandleHelpCommand(nNumArgs, ppszArgs);
	else if(LTStrIEquals(pszCommand, "AddMonitor"))
		HandleAddMonitorCommand(nNumArgs, ppszArgs);
	else if(LTStrIEquals(pszCommand, "RemoveMonitor"))
		HandleRemoveMonitorCommand(nNumArgs, ppszArgs);
	else if(LTStrIEquals(pszCommand, "Log"))
		HandleLogCommand(nNumArgs, ppszArgs);
	else if(LTStrIEquals(pszCommand, "StopLog"))
		HandleStopLogCommand();
	else if(LTStrIEquals(pszCommand, "Report"))
		HandleReportCommand(nNumArgs, ppszArgs);
	else if(LTStrIEquals(pszCommand, "List"))
		HandleListCommand(nNumArgs, ppszArgs);
	else
	{
		//not a valid command
		DisplayMainHelp();
	}
}

//called to handle the various commands
void CPerformanceLog::HandleAddMonitorCommand(uint32 nArgC, const char* const* ppszArgs)
{
	//must specify a certain number of arguments
	if(nArgC < 6)
	{
		DisplayHelp("AddMonitor");
		return;
	}

	//determine the comparison function that we want to use
	CPerformanceValueMonitor::TTriggerCB pfnCmp = NULL;
	const char* pszCmpFunc = ppszArgs[nArgC - 3];
	if(LTStrIEquals(pszCmpFunc, "<"))
		pfnCmp = PerfMonCompareLessThan;
	else if(LTStrIEquals(pszCmpFunc, ">"))
		pfnCmp = PerfMonCompareGreaterThan;
	else
	{
		//invalid comparison
		DisplayHelp("AddMonitor");
		return;
	}

	//now get the name of this monitor, the threshold, and the frequency
	const char* pszName = ppszArgs[1];
	double fThreshold = atof(ppszArgs[nArgC - 2]);
	double fFrequency = atof(ppszArgs[nArgC - 1]);

	//create our new performance value monitor
	CPerformanceValueMonitor* pNewMonitor = new CPerformanceValueMonitor;
	if(!pNewMonitor)
		return;

	//setup our initial state
	pNewMonitor->m_sName = pszName;
	pNewMonitor->SetThreshold(fThreshold);
	pNewMonitor->SetTriggerElapse(fFrequency);
	pNewMonitor->SetTriggerCB(pfnCmp);

    //and now build up our list of performance values to track
	uint32 nNumValidValues = 0;
	for(uint32 nCurrArg = 2; nCurrArg < nArgC - 3; nCurrArg++)
	{
		const char* pszValue = ppszArgs[nCurrArg];
		const CPerformanceValue* pValue = m_ValueDB.FindPerformanceValue(pszValue);
		if(pValue)
		{
            pNewMonitor->AddValue(pValue);
			nNumValidValues++;
		}
	}

	//make sure a value was valid
	if(nNumValidValues == 0)
	{
		delete pNewMonitor;
		DisplayHelp("AddMonitor");
		return;
	}

	//and register this monitor
	AddMonitor(pNewMonitor);
}

void CPerformanceLog::HandleRemoveMonitorCommand(uint32 nArgC, const char* const* ppszArgs)
{
	if(nArgC < 2)
	{
		DisplayHelp("RemoveMonitor");
		return;
	}

	RemoveMonitor(ppszArgs[1]);
}

void CPerformanceLog::HandleLogCommand(uint32 nArgC, const char* const* ppszArgs)
{
	//extract out our log name, frequency, and values
	if(nArgC < 4)
	{
		DisplayHelp("Log");
		return;
	}

	//extract out our log name
	const char* pszLogFile = ppszArgs[1];

	//extract our frequency
	double fLogFrequency = atof(ppszArgs[2]);

	TLogValueList ValueList;

	//and now build up our list of parameters
	for(uint32 nCurrArg = 3; nCurrArg < nArgC; nCurrArg++)
	{
		//try and convert this over into a value
		const char* pszValName = ppszArgs[nCurrArg];
		const CPerformanceValue* pValue = m_ValueDB.FindPerformanceValue(pszValName);
		if(pValue)
			ValueList.push_back(pValue);
	}

	//now see if we have a valid value list
	if(ValueList.empty())
	{
		g_pLTClient->CPrint("Error: Unable to find any performance values in the provided list");
		DisplayHelp("Log");
		return;
	}

	//now try to open up the log
	if(!StartLog(pszLogFile, fLogFrequency, ValueList.size(), &ValueList[0]))
	{
		g_pLTClient->CPrint("Error: Unable to open log file \'%s\' for writing", pszLogFile);
		return;
	}
}

void CPerformanceLog::HandleStopLogCommand()
{
	StopLog();
}

void CPerformanceLog::HandleReportCommand(uint32 nArgC, const char* const* ppszArgs)
{
	if(nArgC < 2)
	{
		//they didn't provide a name, so use the default level's
		if(!GenerateReport(NULL))
		{
			g_pLTClient->CPrint("Error: Unable to generate report for level \'%s\'", m_sCurrentLevel.c_str());
		}
	}
	else
	{
		//they provided a name, use that
		if(!GenerateReport(ppszArgs[1], NULL))
		{
			g_pLTClient->CPrint("Error: Unable to generate report \'%s\'", ppszArgs[1]);
		}
	}
}

void CPerformanceLog::HandleListCommand(uint32 nArgC, const char* const* ppszArgs)
{
	//see if they provided a category
	const char* pszCategory = NULL;
	if(nArgC > 1)
	{
		pszCategory = ppszArgs[1];
	}

	//now run through and list those that match the category if one was provided
	for(uint32 nCurrValue = 0; nCurrValue < m_ValueDB.GetNumPerformanceValues(); nCurrValue++)
	{
		const CPerformanceValue* pValue = m_ValueDB.GetPerformanceValue(nCurrValue);

		//see if we either didn't provide a category, or if this matches our category
		if(pszCategory && !LTStrIEquals(pszCategory, pValue->GetCategory()))
			continue;

		g_pLTClient->CPrint("%s - %s - %s", pValue->GetName(), pValue->GetCategory(), pValue->GetDescription());
	}
}

void CPerformanceLog::HandleHelpCommand(uint32 nArgC, const char* const* ppszArgs)
{
	//verify that they provided a topic
	if(nArgC < 2)
	{
		DisplayMainHelp();
		return;
	}

	//extract the topic and display the help
	if(!DisplayHelp(ppszArgs[1]))
		DisplayMainHelp();
}

//--------------------------------
// Help support

//displays a help title for the provided topic
void CPerformanceLog::DisplayHelpTitle(const char* pszTopic)
{
	g_pLTClient->CPrint("-----------------------------------");
	g_pLTClient->CPrint("Perf Log Help: %s", pszTopic);
	g_pLTClient->CPrint("-----------------------------------");
}

//given the name of a topic, this will display the associated help, or return false if
//that topic could not be found
bool CPerformanceLog::DisplayHelp(const char* pszTopic)
{
	if(LTStrIEquals(pszTopic, "AddMonitor"))
	{
		DisplayHelpTitle("AddMonitor");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("AddMonitor <Name> <PerfVal 0> [PerfVal N] <Operation> <Threshold> <Frequency>");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("Creates a monitor with the provided name that each frame will add together the listed performance");
		g_pLTClient->CPrint("values and determine if it is less than or greater than the provided threshold, and if so will");
		g_pLTClient->CPrint("generate a report. The supported operations are < and >. The frequency is the amount of time");
		g_pLTClient->CPrint("in seconds after this monitor has fired before it can fire again with <=0 indicating never to fire ");
		g_pLTClient->CPrint("once the monitor initially fires.");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Parameters---");
		g_pLTClient->CPrint("Name - The name of this monitor, used to disable the monitor");
		g_pLTClient->CPrint("PerfValue - The name of a performance value that should be accumulated into the tested value");
		g_pLTClient->CPrint("Operation - Either < or > depending upon how the value should be tested against the threshold");
		g_pLTClient->CPrint("Threshold - The value to test the performance values against to see if it should trigger");
		g_pLTClient->CPrint("Frequency - How long to wait after firing before the monitor can fire again in seconds");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("See Also - \'RemoveMonitor\'");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - AddMonitor FrameRateMon FrameRate < 30 -1");
	}
	else if(LTStrIEquals(pszTopic, "RemoveMonitor"))
	{
		DisplayHelpTitle("RemoveMonitor");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("RemoveMonitor <Name>");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("This will remove a previously created monitor with the provided name");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Parameters---");
		g_pLTClient->CPrint("Name - The name of the monitor to remove");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("See Also - \'AddMonitor\'");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - RemoveMonitor FrameRateMon");
	}
	else if(LTStrIEquals(pszTopic, "Log"))
	{
		DisplayHelpTitle("Log");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("Log <File> <Frequency> <PerfVal 0> [PerfVal N]");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("This opens up a log to the specified file, and will write out samples from the");
		g_pLTClient->CPrint("specified performance values at the specified frequency. This closes any previously");
		g_pLTClient->CPrint("opened logs by this system.");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Parameters---");
		g_pLTClient->CPrint("File - The name of the log file, typically with a .csv extension.");
		g_pLTClient->CPrint("Frequency - The time in seconds between each sample that should be written to the log");
		g_pLTClient->CPrint("PerfVal - A listing of the performance values that should be logged");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("See Also - \'StopLog\'");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - Log TestLog.csv 1.0 FrameRate TotalMemory");
	}
	else if(LTStrIEquals(pszTopic, "StopLog"))
	{
		DisplayHelpTitle("StopLog");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("StopLog");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("This stops and closes any currently opened logs");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("See Also - \'Log\'");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - StopLog");
	}
	else if(LTStrIEquals(pszTopic, "Report"))
	{
		DisplayHelpTitle("Report");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("Report [File]");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("This generates a report of the current performance information and");
		g_pLTClient->CPrint("writes it out to the level's performance log, or to the provided file");
		g_pLTClient->CPrint("if one is specified. Note that this provided file must be relative to");
		g_pLTClient->CPrint("the user directory.");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Parameters---");
		g_pLTClient->CPrint("File - Optional filename to have the report generate into");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - Report PerfLogs\\MyLog.html");
	}
	else if(LTStrIEquals(pszTopic, "List"))
	{
		DisplayHelpTitle("List");
		g_pLTClient->CPrint("---Syntax---");
		g_pLTClient->CPrint("List [Category]");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Description---");
		g_pLTClient->CPrint("This lists all of the available performance values that are supported.");
		g_pLTClient->CPrint("Optionally a category can be specified to restrict the values that are listed.");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("---Parameters---");
		g_pLTClient->CPrint("Category - The name of the category to list values in (may be omitted to list all)");
		g_pLTClient->CPrint("");
		g_pLTClient->CPrint("Example - List General");
	}
	else
	{
		//no matching topic
		return false;
	}

	//we found our topic
	return true;
}

//called to display the main help menu
void CPerformanceLog::DisplayMainHelp()
{
	DisplayHelpTitle("Main");
	g_pLTClient->CPrint("To get information on how to use the performance log system, you can use the \'Help\' command");
	g_pLTClient->CPrint("followed by the command or topic that help is needed for. Below is an example:");
	g_pLTClient->CPrint(" PerfLog Help StopLog");
	g_pLTClient->CPrint("The following topics are supported:");
	g_pLTClient->CPrint("");
	g_pLTClient->CPrint("Commands:");
	g_pLTClient->CPrint("---------");
	g_pLTClient->CPrint("AddMonitor, RemoveMonitor, Log, StopLog, Report, List");
}

