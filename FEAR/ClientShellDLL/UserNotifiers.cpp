#include "stdafx.h"
#include "UserNotificationMgr.h"

//---------------------------------------------------------------------------------
// Utility functions

//the console variable that controls the LOD that we should be using for the value ranges
//0-low, 1-med, 2-high
static VarTrack	g_vtUserNotifierLOD;

//this function takes in four numbers. The first number indicates a value, and the following
//three are the beginning of the low warning, medium, and high respectively. It will then determine
//which range the value falls in and return the appropriate severity
static ENotificationSeverity PlaceIntoRange(uint32 nValue, uint32 nLow, uint32 nMed, uint32 nHigh)
{
	if(nValue >= nHigh)
		return eNotificationSeverity_High;
	if(nValue >= nMed)
		return eNotificationSeverity_Medium;
	if(nValue >= nLow)
		return eNotificationSeverity_Low;

	return eNotificationSeverity_Disabled;
}

//---------------------------------------------------------------------------------
// Bute file support

//we have to derive from the game bute mgr, otherwise we can't access anything in the bute. No real
//other reason...
class CNotifierDB :	public CGameDatabaseMgr
{
	DECLARE_SINGLETON(CNotifierDB);

public:

	//handle initialization of the attribute file
	bool Init(const char* pszFilename)
	{
		if (!OpenDatabase(pszFilename)) 
			return false;

		m_hNotifierCat = g_pLTDatabase->GetCategory(m_hDatabase,"Notifier");
		if (!m_hNotifierCat)
			return false;

		//success
		return true;
	}

	//funciton that given a tag name will read in the high, medium, and low keys placed below it
	bool ReadRange(const char* pszTag, const char* pszGroup, int32& nLow, int32& nMedium, int32& nHigh)
	{
		//make sure that the entire range exists
		HRECORD hNot = g_pLTDatabase->GetRecord(m_hNotifierCat, pszTag);
		if (!hNot)
		{
			//it doesn't exist, fail
			return false;
		}

		//read in the values
		nLow	= GetInt32(hNot, pszGroup, 0);
		nMedium = GetInt32(hNot, pszGroup, 1);
		nHigh	= GetInt32(hNot, pszGroup, 2);

		//success
		return true;
	}

	const char* ReadIcon(const char* pszTag)
	{
		//make sure that the entire range exists
		HRECORD hNot = g_pLTDatabase->GetRecord(m_hNotifierCat, pszTag);
		if (!hNot)
		{
			return "";
		}

		return GetString(hNot, "IconImage");
	}

private:
	HCATEGORY m_hNotifierCat;
};

CNotifierDB::CNotifierDB() :
	m_hNotifierCat(NULL)
{
}
CNotifierDB::~CNotifierDB()
{
}


//utility class that will use the requested name to load in a high, medium, and low value. It will
//then store this for subsequent calls.
class CNotifierRange
{
public:

	CNotifierRange(const char* pszTagName) : 
		m_bValidValues(false)
	{
		//try and load up the range from the bute file
		if( CNotifierDB::Instance().ReadRange(pszTagName, "LowLOD", m_nLow[0], m_nMedium[0], m_nHigh[0]) &&
			CNotifierDB::Instance().ReadRange(pszTagName, "MediumLOD", m_nLow[1], m_nMedium[1], m_nHigh[1]) &&
			CNotifierDB::Instance().ReadRange(pszTagName, "HighLOD", m_nLow[2], m_nMedium[2], m_nHigh[2]))
		{
			m_bValidValues = true;
		}
	}

	//given a value, this will place it within the range that is specified. It will attempt
	//to load this range from an attribute fiel if necessary, and if it should fail it will simply
	//always return disabled
	ENotificationSeverity GetRange(int32 nValue)
	{
		if(!m_bValidValues)
			return eNotificationSeverity_Disabled;

		uint32 nLOD = LTCLAMP((int32)(g_vtUserNotifierLOD.GetFloat() + 0.5f), 0, 2);
		return PlaceIntoRange(nValue, m_nLow[nLOD], m_nMedium[nLOD], m_nHigh[nLOD]);
	}

private:

	//flag indicating whether our values are valid or not (meaning properly loaded from the
	//bute file)
	bool				m_bValidValues;

	//the range loaded from the bute file
	int32				m_nLow[3];
	int32				m_nMedium[3];
	int32				m_nHigh[3];
};


//---------------------------------------------------------------------------------
// Light Count Notification

static ENotificationSeverity Notification_VisibleLights(void* pUser)
{
	static CNotifierRange s_ButeRange("VisibleLights");

	CRendererFrameStats& Stats = *((CRendererFrameStats*)pUser);

	//accumulate our score for the lights (low = good)
	uint32 nScore = 0;

	//for normal lights we have to do a visibility query and also a rendering pass
	nScore += 2 * Stats.GetFrameStat(eFS_Light_Directionals);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_PointLights);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_SpotProjectors);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_CubeProjectors);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_BlackLights);

	//for shadows, we need to perform a hull query and a lot of vertex and fill rate
	nScore += 2 * Stats.GetFrameStat(eFS_Light_DirectionalShadows);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_PointLightShadows);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_SpotProjectorShadows);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_CubeProjectorShadows);
	nScore += 2 * Stats.GetFrameStat(eFS_Light_BlackLightShadows);

	//for point fill lights determine the number of batches, and score each batch (which must do
	//up to 3 vis queries and is heavy on fill rate)
	nScore += 4 * ((Stats.GetFrameStat(eFS_Light_PointFillLights) + 2) / 3);

	return s_ButeRange.GetRange(nScore);
}

//---------------------------------------------------------------------------------
// Visible Sector Notification

static ENotificationSeverity Notification_VisibleSectors(void* pUser)
{
	static CNotifierRange s_ButeRange("VisibleSectors");

	//determine our total memory usage in MB
	CRendererFrameStats* pFrameStats = (CRendererFrameStats*)pUser;
	uint32 nVisibleSectors = pFrameStats->GetFrameStat(eFS_Vis_SectorsVisible);

	//and determine the range for that value
	return s_ButeRange.GetRange(nVisibleSectors);
}

//---------------------------------------------------------------------------------
// Texture Memory Notification

static ENotificationSeverity Notification_TextureMemoryMB(void* pUser)
{
	static CNotifierRange s_ButeRange("TextureMemoryMB");

	//determine our total memory usage in MB
	CRendererFrameStats* pFrameStats = (CRendererFrameStats*)pUser;
	uint32 nTextureMemory = pFrameStats->GetFrameStat(eFS_VidMemory_Textures) + 
							pFrameStats->GetFrameStat(eFS_VidMemory_SystemTextures);
	nTextureMemory /= 1024 * 1024;


	//and determine the range for that value
	return s_ButeRange.GetRange(nTextureMemory);
}

//---------------------------------------------------------------------------------
// Frame Rate Notification

static ENotificationSeverity Notification_FrameRateMS(void* pUser)
{
	static CNotifierRange s_ButeRange("FrameRateMS");

	//what we do to avoid lots hitches such as asset loadings from constantly triggering this notifier
	//is to fill up a window and then take the average of that window in order to determine the severity.
	//This has the benefit of smoothing out the framerate and also keeping the color consistent for longer
	static const uint32 knWindowSize = 10;
	static uint32 nWindowTotal = 0;
	static uint32 nFilledWindow = 0;
	static ENotificationSeverity eLastSeverity = eNotificationSeverity_Disabled;

	//store our current value in our window
	nWindowTotal += RealTimeTimer::Instance().GetTimerElapsedMS();
	nFilledWindow++;

	//see if we have filled our window
	if(nFilledWindow >= knWindowSize)
	{
		//update our severity based upon our average over this window
		eLastSeverity = s_ButeRange.GetRange(nWindowTotal / knWindowSize);

		//reset our acumulated information
		nWindowTotal  = 0;
		nFilledWindow = 0;
	}

	return eLastSeverity;
}

//---------------------------------------------------------------------------------
// Global registration function


//this is called to install the notifiers that are found in UserNotifiers.cpp into the specified
//notification manager.
void InstallUserNotifiers(CUserNotificationMgr& NotificationMgr)
{
	//initialize our LOD console variable
	g_vtUserNotifierLOD.Init(g_pLTClient, "UserNotifierLOD", NULL, 2.0f);

	//we need to make sure to setup our global bute manager
	CNotifierDB::Instance().Init(DB_Default_File);

	//the activation time to use for the notifications, this is how long they will be displayed on screen
	static const float kfActivationTimeS = 1.0f;

	//the renderer frame statistics, cached since many notifiers use this
	void*	pRenderFrameStats = (void*)&g_pGameClientShell->GetRendererFrameStats();

	//now register each one of the notifications we have
	NotificationMgr.RegisterNotification(Notification_VisibleLights, pRenderFrameStats, CNotifierDB::Instance().ReadIcon("VisibleLights"), kfActivationTimeS);
	NotificationMgr.RegisterNotification(Notification_VisibleSectors, pRenderFrameStats, CNotifierDB::Instance().ReadIcon("VisibleSectors"), kfActivationTimeS);
	NotificationMgr.RegisterNotification(Notification_TextureMemoryMB, pRenderFrameStats, CNotifierDB::Instance().ReadIcon("TextureMemoryMB"), kfActivationTimeS);
	NotificationMgr.RegisterNotification(Notification_FrameRateMS, pRenderFrameStats, CNotifierDB::Instance().ReadIcon("FrameRateMS"), kfActivationTimeS);
}



