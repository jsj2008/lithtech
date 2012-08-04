// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileMgr.cpp
//
// PURPOSE : Manages player profiles
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "profileMgr.h"
#include "clientutilities.h"
#include "commandids.h"
#include "interfacemgr.h"
#include "winutil.h"
#include "GameClientShell.h"
#include "GameSettings.h"
#include "MenuMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientConnectionMgr.h"
#include "VarTrack.h"
#include "CMoveMgr.h"
#include "MsgIds.h"
#include "VersionMgr.h"
#include "iltrenderer.h"
#include "GameDataBaseMgr.h"
#include "HUDSwap.h"
#include "sys/win/mpstrconv.h"
#include "PlayerCamera.h"
#include "HUDWeaponList.h"
#include "GameModeMgr.h"
#include "ClientDB.h"
#include "ltprofileutils.h"
#include "iltfilemgr.h"
#include "ltoutnullconverter.h"
#include "HostOptionsMapMgr.h"

#include <Direct.h>			// For _rmdir
#include <set>
#include <IO.H>
#if !defined(PLATFORM_XENON)
#include <objbase.h>		// For CoCreateGUID
#endif // !PLATFORM_XENON

#include "iltinput.h"
#include "bindmgr.h"

static ILTInput *g_pLTInput;
define_holder(ILTInput, g_pLTInput);

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_vtMouseMinSensitivity;
VarTrack	g_vtMouseMaxSensitivity;
VarTrack	g_vtMouseMinInputRate;
VarTrack	g_vtMouseMaxInputRate;


CProfileMgr* g_pProfileMgr = NULL;


#define DEFAULT_PROFILE ("ProfileDatabase/Defaults.Gamdb00p")

namespace
{
	char szDefProfileName[16] = "Profile000";
}

static std::wstring wsWheelUp;
static std::wstring wsWheelDown;

typedef std::vector< CommandData, LTAllocator<CommandData, LT_MEM_TYPE_GAMECODE> > CommandDataArray;
CommandDataArray g_CommandArray;

// Sorts friends list alphabetically with no case.
static bool FriendsListSort( wchar_t const* pwszLeft, wchar_t const* pwszRight )
{
	return LTStrICmp( pwszLeft, pwszRight ) < 0;
}

// Sorts favorite servers by IP.
static bool FavoriteServerSort( FavoriteServer const* p1, FavoriteServer const* p2 )
{
	return ( LTStrICmp( p1->GetServerIPandPort(), p2->GetServerIPandPort()) < 0 );
}


static const char * const g_prf_BindingsCat		= "Bindings";
	static const char * const g_prf_DeviceAtt			= "Bindings.0.Device";
	static const char * const g_prf_ObjectAtt			= "Bindings.0.Object";
	static const char * const g_prf_DefaultAtt			= "Bindings.0.Default";
	static const char * const g_prf_OffsetAtt			= "Bindings.0.Offset";
	static const char * const g_prf_ScaleAtt			= "Bindings.0.Scale";
	static const char * const g_prf_DeadZoneMinAtt		= "Bindings.0.DeadZoneMin";
	static const char * const g_prf_DeadZoneMaxAtt		= "Bindings.0.DeadZoneMax";
	static const char * const g_prf_DeadZoneValueAtt	= "Bindings.0.DeadZoneValue";
	static const char * const g_prf_CommandMinAtt		= "Bindings.0.CommandMin";
	static const char * const g_prf_CommandMaxAtt		= "Bindings.0.CommandMax";


static const char * const g_prf_ControlsCat		= "Controls";
	static const char * const g_prf_InvertMouse		= "InvertMouse";
	static const char * const g_prf_InputRate		= "InputRate";
	static const char * const g_prf_Sensitivity		= "Sensitivity";
	static const char * const g_prf_NormalTurn		= "NormalTurn";
	static const char * const g_prf_FastTurn		= "FastTurn";
	static const char * const g_prf_LookUp			= "LookUp";

static const char * const g_prf_MultiplayerCat		= "Multiplayer";
	static const char * const g_prf_PlayerName					 = "PlayerName";
	static const char * const g_prf_PlayerPatch					 = "PlayerPatch";
	static const char * const g_prf_Guid						 = "Guid";
	static const char * const g_prf_DMPlayerModel				 = "DMPlayerModel";
	static const char * const g_prf_TeamPlayerModel				 = "TeamPlayerModel";
	static const char * const g_prf_ServerOptionsFile			 = "ServerOptionsFile";
	static const char * const g_prf_BandwidthClient				 = "BandwidthClient";
	static const char * const g_prf_BandwidthClientCustom		 = "BandwidthClientCustom";
	static const char * const g_prf_ClientPort					 = "ClientPort";
	static const char * const g_prf_AllowContentDownload		 = "AllowContentDownload";
	static const char * const g_prf_AllowContentDownloadRedirect = "AllowContentDownloadRedirect";
	static const char * const g_prf_MaximumDownloadSize			 = "MaximumDownloadSize";
	static const char * const g_prf_UsePunkbuster				 = "UsePunkbuster";
	static const char * const g_prf_AllowBroadcast				 = "AllowBroadcast";
	static const char * const g_prf_FilterNavMarkers			 = "FilterNavMarkers";
	static const char * const g_prf_VersionFilter				 = "VersionFilter";
	static const char * const g_prf_PlayersFilter				 = "PlayersFilter";
	static const char * const g_prf_PingFilter					 = "PingFilter";
	static const char * const g_prf_GameTypeFilter				 = "GameTypeFilter";
	static const char * const g_prf_ServerSearchSource			 = "ServerSearchSource";
	static const char * const g_prf_CustomizedFilter			 = "CustomizedFilter";
	static const char * const g_prf_RequiresDownloadFilter		 = "RequiresDownloadFilter";
	static const char * const g_prf_PunkbusterFilter			 = "PunkbusterFilter";
	static const char * const g_prf_Friends						 = "Friends";
	static const char * const g_prf_FavoriteServers				 = "FavoriteServers";
		static const char * const g_prf_FavoriteServers_IP		 = "IP";
		static const char * const g_prf_FavoriteServers_SessionName = "SessionName";
		static const char * const g_prf_FavoriteServers_LAN		 = "LAN";

static const char * const g_prf_GameCat			= "Game";
	static const char * const g_prf_Difficulty			= "Difficulty";
	static const char * const g_prf_Subtitles			= "Subtitles";
	static const char * const g_prf_Gore				= "Gore";
	static const char * const g_prf_bSlowMoFX			= "SlowMoFX";
	static const char * const g_prf_bUseTextScaling		= "UseTextScaling";
	static const char * const g_prf_AlwaysRun			= "AlwaysRun";
	static const char * const g_prf_CrouchToggle		= "CrouchToggle";
	static const char * const g_prf_HeadBob				= "HeadBob";
	static const char * const g_prf_MessageDur			= "MessageDur";
	static const char * const g_prf_SPAutoWeaponSwitch	= "SPAutoWeaponSwitch";
	static const char * const g_prf_MPAutoWeaponSwitch	= "MPAutoWeaponSwitch";
	static const char * const g_prf_PersistentHUD		= "PersistentHUD";
	static const char * const g_prf_HUDFadeSpeed		= "HUDFadeSpeed";
	static const char * const g_prf_CrosshairColor		= "CrosshairColor";
	static const char * const g_prf_CrosshairSize		= "CrosshairSize";

static const char * const g_prf_VersionCat			= "Version";
	static const char * const g_prf_FriendlyName		= "FriendlyName";
	static const char * const g_prf_VersionNumber		= "VersionNumber";

static const char * const g_prf_SoundCat			= "Sound";
	static const char * const g_prf_SoundVolume		= "SoundVolume";
	static const char * const g_prf_MusicVolume		= "MusicVolume";
	static const char * const g_prf_SpeechVolume	= "SpeechVolume";
	static const char * const g_prf_UseHWSound		= "UseHWSound";
	static const char * const g_prf_UseEAX			= "UseEAX";
	static const char * const g_prf_UseEAX4			= "UseEAX4";

static const char * const g_prf_WeaponsCat			= "Weapons";
	static const char * const g_prf_WeaponPriority	= "WeaponPriority";


//Called to write out the display.cfg file which holds data for restoring the video
//mode the next time the game is run
void SaveDisplaySettings()
{
	//save out the display configuration
	const char* pszValsToSave[] = {	"ScreenWidth",
									"ScreenHeight",
									"BitDepth",
									"HardwareCursor",
									"VSyncOnFlip",
									"RestartRenderBetweenMaps",
									"GammaR",
									"GammaG",
									"GammaB"	
								};

	uint32 nNumVals = sizeof(pszValsToSave) / sizeof(pszValsToSave[0]);

	/** @author Jeff Cotton
	 *  @date 02/04/05
     *  Added to put the display settings into the user directory and not into
	 * the application folder 
	 */
	char pszAbsoluteFile[MAX_PATH];
	g_pLTClient->FileMgr()->GetAbsoluteUserFileName( "display.cfg", pszAbsoluteFile, MAX_PATH );

	g_pLTClient->WriteConfigFileFields( pszAbsoluteFile, nNumVals, pszValsToSave );
}

/** @author Jeff Cotton
 *  @date   02/02/2005
 *
 *  Called to write out the "settings.cfg" file which holds data about the
 * display and some advanced options. This is a replacement for "display.cfg" 
 */
void SaveSettings()
{
	//save out the display configuration
	const char* pszValsToSave[] = { "DeviceName",
									"Renderer",
									"ScreenWidth",
	                                "ScreenHeight",
	                                "BitDepth",
	                                "HardwareCursor",
	                                "VSyncOnFlip",
									"RestartRenderBetweenMaps",
	                                "GammaR",
	                                "GammaG",
	                                "GammaB",
									"MusicEnable",
									"DisableJoystick",
									"DisableHardwareCursor",
									"DisableMovies",
									"DisableSoundFilters",
									"DisableHardwareSound",
									"DisableSound" };

	/** @author Jeff Cotton
	 *  @date 02/04/05
     *  Get the absolute path to the file in the users directory 
	 */
	char pszAbsoluteFile[MAX_PATH];
	g_pLTClient->FileMgr()->GetAbsoluteUserFileName( "settings.cfg", pszAbsoluteFile, MAX_PATH );

	g_pLTClient->WriteConfigFileFields( pszAbsoluteFile, LTARRAYSIZE(pszValsToSave), pszValsToSave );
}

//-------------------------------------------------------------------------------------------
// GetCommandName
//
// Retrieves the command name from a command number
// Arguments:
//		nCommand - command number
// Return:
//		String containing the name of the action
//-------------------------------------------------------------------------------------------

const char* GetCommandName(int nCommand)
{
	for (int i=0; i < GetNumCommands(); i++)
	{
		if (g_CommandArray[i].m_nCommandID == nCommand)
		{
			return g_CommandArray[i].m_szActionString;
		}
	}

	return "Error in CommandName()!";
}


const char* GetCommandStringID(int nCommand)
{
	static char buffer[128];

	for (int i=0; i < GetNumCommands(); i++)
	{
		if (g_CommandArray[i].m_nCommandID == nCommand)
		{
			return g_CommandArray[i].m_szStringID;
		}
	}

	LTStrCpy(buffer, "Error in CommandName()!", LTARRAYSIZE(buffer));
	return buffer;
}

const int GetCommandID(const char* szAction)
{
	for (int i=0; i < GetNumCommands(); i++)
	{
		if (  LTStrIEquals(szAction,	g_CommandArray[i].m_szActionString) )
		{
			return g_CommandArray[i].m_nCommandID;
		}
	}

	return COMMAND_ID_UNASSIGNED;
}



const int GetNumCommands()
{
	return g_CommandArray.size();
}

const CommandData* GetCommandData(int nIndex)
{
	if (nIndex < 0 || nIndex >= GetNumCommands())
		return NULL;
	return &g_CommandArray[nIndex];
}


CUserProfile::CUserProfile()
{
	m_sFileName		= "";		
	m_sFriendlyName	= L"";		
	m_bInitted		= false;

	//controls
    m_bInvertY = false;
	m_nInputRate = 0;
	m_nSensitivity = 0;

	m_nNormalTurn = 15;
	m_nFastTurn = 23;
	m_nLookUp = 25;


	//game options
	m_nDifficulty = 1;
    m_bSubtitles  = false;
    m_bGore = false;
	m_bSlowMoFX = true;
	m_bUseTextScaling = false;

#ifdef PROJECT_DARK

	m_bAlwaysRun = false;

#else

	m_bAlwaysRun = true;

#endif
	m_bCrouchToggle = false;

	m_nHeadBob = 0;
	m_nMsgDur = 0;
	m_bSPAutoWeaponSwitch = false;
	m_bMPAutoWeaponSwitch = false;
	m_bPersistentHUD = false;
	m_fHUDFadeSpeed = 1.0f;

	m_CrosshairR = 0x1F;
	m_CrosshairG = 0xFF;
	m_CrosshairB = 0xFF;
	m_CrosshairA = 0xBF;
	m_nCrosshairSize = 12;

	//sound
	m_fSoundVolume = 1.0f;
	m_fSpeechVolume= 1.0f;
	m_fMusicVolume= 1.0f;
	m_bUseHWSound = false;
	m_bUseEAX = false;
	m_bUseEAX4 = false;

	//display
	m_bHardwareCursor = true;
	m_bVSync = false;
	m_bRestartRenderBetweenMaps = false;
	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_nScreenDepth = 0;
	m_fGamma = 1.0f;

	//multi player
	m_nDMPlayerModel = 0;
	m_nTeamPlayerModel = 0;
	m_nBandwidthClient = 3;
	m_nBandwidthClientCustom = g_BandwidthClient[m_nBandwidthClient].m_nBandwidthTargetClient;
	m_nClientPort = 27888;

	m_nVersionFilter = 0;
	m_nPlayersFilter = 0;
	m_nPingFilter = 0;
	m_nGameTypeFilter = 0;
	m_eServerSearchSource = eServerSearchSource_Internet;
	m_nCustomizedFilter = 0;
	m_nRequiresDownloadFilter = 0;
	m_nPunkbusterFilter = 0;

	m_nSaveVersion = 0;
}

CUserProfile::~CUserProfile()
{
	ClearFriendsList( );
	ClearFavoriteServerList( );
}

static bool CreatePlayerGuid( LTGUID& playerGuid )
{
#if defined(PLATFORM_XENON)

	// Just use the tick count in place of a GUID.
	// Note : This may need to be replaced with a more robust solution for a GUID replacement
	playerGuid.guid.a = GetTickCount();

#else // !PLATFORM_XENON

	// Create the guid.
	GUID guid;
	if( !SUCCEEDED( CoCreateGuid( &guid )))
		return false;

	playerGuid = *( LTGUID* )&guid;

#endif // !PLATFORM_XENON

	return true;
}

bool CUserProfile::Init(const char* profileName, const wchar_t* friendlyName, bool bCreate, bool bLoadDisplaySettings )
{
	std::string fn = GetAbsoluteProfileFile( g_pLTClient, profileName);

	g_vtMouseMinSensitivity.Init(g_pLTClient, "MouseSensitivityMin", NULL, 0.0);
	g_vtMouseMaxSensitivity.Init(g_pLTClient, "MouseSensitivityMax", NULL, 15.0);
	g_vtMouseMinInputRate.Init(g_pLTClient, "MouseInputRateMin", NULL, 0.0);
	g_vtMouseMaxInputRate.Init(g_pLTClient, "MouseInputRateMax", NULL, 20.0);


	// Resize the device binding array to match the profilemgr commands
	m_aCommandDeviceBindings.resize(GetNumCommands());

	// Handle case where we had a previous profile file.
	if (CWinUtil::FileExist(fn.c_str()))
	{
		m_sFileName = profileName;
		Load( false, bLoadDisplaySettings );


		if (LTStrEmpty(m_sFriendlyName.c_str()))
		{
			if (LTStrEmpty(friendlyName))
			{
				m_sFriendlyName = g_pProfileMgr->GenerateFriendlyName();
			}
			else
			{
				m_sFriendlyName = friendlyName;
			}
			Save();
		}
	}
	// Handle brand new profile.
	else
	{
		// No previous profile and not told to create one.
		if (!bCreate)
			return false;

		// Specify the name to use before load since operations in load will depend on it.
		m_sFileName = profileName;

		wchar_t szName[64] = L"";
		
		if (LTStrEmpty(friendlyName))
		{
			LTStrCpy(szName,g_pProfileMgr->GenerateFriendlyName(), LTARRAYSIZE(szName));
		}
		else
		{
			LTStrCpy(szName,friendlyName, LTARRAYSIZE(szName));
		}

		m_sFriendlyName = szName;

		//load defaults
		Load(true, false);

		Save();
	}
	
	// Since we changed our profile, we'll need to re-init the saveloadmgr, which
	// uses the profile name.
	if( !g_pClientSaveLoadMgr->Init( m_sFileName.c_str( ), IsMultiplayerGameClient() ))
		return false;

	m_bInitted = true;

	//the swap HUD element reports the keypress needed, so it might need to get updated here
	CHUDSwap::UpdateTriggerName();

	return true;

}

void CUserProfile::Load(bool bLoadDefaults, bool bLoadDisplaySettings)
{
	HDATABASE hDB = NULL;
	if (bLoadDefaults)
	{
		ILTInStream	*pDBStream = g_pLTBase->FileMgr()->OpenFile( DEFAULT_PROFILE );
		if( !pDBStream )
		{
			g_pLTBase->CPrint( "ERROR CGameDatabaseMgr couldn't open file: %s!", DEFAULT_PROFILE );
			return;
		}

		// Open the database...
		hDB = g_pLTDatabase->OpenNewDatabase( DEFAULT_PROFILE, *pDBStream );

		// Free up the stream...
		pDBStream->Release( );
	}
	else
	{
		const char* pszFilename = GetRelativeProfileFile( g_pLTClient, m_sFileName.c_str( ));
		ILTInStream* pInFile = g_pLTClient->FileMgr()->OpenUserFileForReading(pszFilename);

		if(pInFile)
		{
			hDB = g_pLTDatabase->OpenNewDatabase(pszFilename, *pInFile);
			LTSafeRelease(pInFile);
		}
		else
		{
			LTERROR_PARAM1("Unable to load profile %s.",pszFilename);
		}

	}

	if (!hDB)
	{
		LTASSERT(!bLoadDefaults, "Error opening default user profile database");
		return;
	}

	LoadVersion(hDB);
	LoadControls(hDB);
	LoadMultiplayer(hDB,bLoadDefaults);
	LoadGameOptions(hDB);
	LoadSound(hDB,true);

	// Load performance options. (Default settings for performance settings is handled separately)
	SetDisplay();
	if (bLoadDefaults)
	{
		// If the current performance settings are valid, go ahead and keep them.
		// This guard was put in so that when a new profile is created while an
		// old one exists, the performance stats of the old one will be carried
		// into the new profile.
		if( !CPerformanceMgr::Instance().ArePerformanceStatsValid() )
		{
			CPerformanceMgr::Instance().SetDetailLevel(ePT_CPU,g_DefaultCPUDetailLevel);
			CPerformanceMgr::Instance().SetDetailLevel(ePT_GPU,g_DefaultGPUDetailLevel);
		}
	}
	else
	{
		CPerformanceMgr::Instance().Load(hDB, bLoadDisplaySettings);
	}


	// load weapon priorities
	LoadWeaponPriorities(hDB,bLoadDefaults);

	g_pLTDatabase->ReleaseDatabase(hDB);
}

void CUserProfile::SetCommandBinding(uint32 uiCommand, SCommandBindingInfo binding )
{
	// Find the command ID in the global command list
	uint32 nCommandIndex = (uint32)-1;
	for (CommandDataArray::const_iterator iCurCommand = g_CommandArray.begin();
		iCurCommand != g_CommandArray.end(); ++iCurCommand)
	{
		if ((uint32)iCurCommand->m_nCommandID == uiCommand)
		{
			nCommandIndex = iCurCommand - g_CommandArray.begin();
			break;
		}
	}
	LTASSERT(nCommandIndex < g_CommandArray.size(), "Command ID not found in global array");
	// Load the binding
	if (nCommandIndex != (uint32)-1)
	{
		m_aCommandDeviceBindings[nCommandIndex].resize(0);
		m_aCommandDeviceBindings[nCommandIndex].push_back(binding);
	}
}

void CUserProfile::SetAxisBinding(uint32 command, WCHAR* DeviceName, WCHAR* ObjectName, float fScale)
{
	const float k_fAxisDeadZone = 0.4f;
	
	SCommandBindingInfo BindingInfo;
	BindingInfo.m_sDeviceName = DeviceName;
	BindingInfo.m_sObjectName = ObjectName;
	BindingInfo.m_fDefaultValue = 0.0f;
	BindingInfo.m_fOffset = -(1.0f + k_fAxisDeadZone) * fScale;
	BindingInfo.m_fScale = (2.0f + k_fAxisDeadZone* 2.0f) * fScale;
	BindingInfo.m_fDeadZoneMin = -k_fAxisDeadZone * fabsf(fScale);
	BindingInfo.m_fDeadZoneMax = k_fAxisDeadZone * fabsf(fScale);
	BindingInfo.m_fDeadZoneValue = 0.0f;
	BindingInfo.m_fCommandMin = 0.0f;
	BindingInfo.m_fCommandMax = 1.0f;

	SetCommandBinding(command, BindingInfo);
}

void CUserProfile::SetButtonBinding(uint32 command, WCHAR* DeviceName, WCHAR* ObjectName)
{
	SCommandBindingInfo BindingInfo;
	BindingInfo.m_sDeviceName = DeviceName;
	BindingInfo.m_sObjectName = ObjectName;
	BindingInfo.m_fDefaultValue = 0.0f;
	BindingInfo.m_fOffset = 0.0f;
	BindingInfo.m_fScale = 255.0f;
	BindingInfo.m_fDeadZoneMin = FLT_MAX;
	BindingInfo.m_fDeadZoneMax = -FLT_MAX;
	BindingInfo.m_fDeadZoneValue = 0.0f;
	BindingInfo.m_fCommandMin = 0.1f;
	BindingInfo.m_fCommandMax = FLT_MAX;

	SetCommandBinding(command, BindingInfo);
}

void CUserProfile::LoadVersion(HDATABASE hDB)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_VersionCat,g_prf_VersionCat);
	if (!hRec) return;

	m_nSaveVersion = GetInt32(hRec,g_prf_VersionNumber);
	m_sFriendlyName = GetWString(hRec,g_prf_FriendlyName );
	//m_sFriendlyName = GetWString(hRec,g_prf_FriendlyName, 0,  MPA2W(m_sFileName.c_str()).c_str() );

}


void CUserProfile::LoadControls(HDATABASE hDB)
{
	HRECORD hRec;
  
#if defined(PLATFORM_XENON)
	// XENON: For now, we load a hard-coded list of controls

	SetAxisBinding(COMMAND_ID_STRAFE_AXIS, L"Pad0", L"Left Stick X", 1.0f);
	SetAxisBinding(COMMAND_ID_FORWARD_AXIS, L"Pad0", L"Left Stick Y", 1.0f);
	SetAxisBinding(COMMAND_ID_YAW_ACCEL, L"Pad0", L"Right Stick X", 1.0f);
	SetAxisBinding(COMMAND_ID_PITCH_ACCEL, L"Pad0", L"Right Stick Y", -1.0f);

	SetButtonBinding(COMMAND_ID_FIRING,	L"Pad0", L"Right Trigger");
	SetButtonBinding(COMMAND_ID_FOCUS, L"Pad0", L"Left Trigger");
	SetButtonBinding(COMMAND_ID_KNEEL, L"Pad0", L"D-Pad Down");
	SetButtonBinding(COMMAND_ID_STAND, L"Pad0", L"D-Pad Up");
	SetButtonBinding(COMMAND_ID_AMMOCHECK, L"Pad0", L"Left Shoulder Button");
	SetButtonBinding(COMMAND_ID_STUNGUN, L"Pad0", L"Right Shoulder Button");
	SetButtonBinding(COMMAND_ID_BLOCK, L"Pad0", L"Right Stick Button");
	SetButtonBinding(COMMAND_ID_RUN, L"Pad0", L"Left Stick Button");
	SetButtonBinding(COMMAND_ID_ACTIVATE, L"Pad0", L"Y Button");
	SetButtonBinding(COMMAND_ID_TOGGLEMELEE, L"Pad0", L"A Button");
	SetButtonBinding(COMMAND_ID_TOOLS, L"Pad0", L"X Button");
	SetButtonBinding(COMMAND_ID_FLASHLIGHT, L"Pad0", L"B Button");
	SetButtonBinding(COMMAND_ID_LEAN_LEFT, L"Pad0", L"D-Pad Left");
	SetButtonBinding(COMMAND_ID_LEAN_RIGHT, L"Pad0", L"D-Pad Right");

#else // !PLATFORM_XENON

	// Read the command bindings
	HCATEGORY hCat = g_pLTDatabase->GetCategory(hDB, g_prf_BindingsCat);
	for (uint32 nCurCommand = 0; nCurCommand < (uint32)GetNumCommands(); ++nCurCommand)
	{
		m_aCommandDeviceBindings[nCurCommand].resize(0);
		const CommandData *pCommand = GetCommandData(nCurCommand);
  
		hRec = g_pLTDatabase->GetRecord(hCat, pCommand->m_szActionString);
		if (!hRec) 
		{
			continue;
		}
  
		// Look up the attributes
		HATTRIBUTE hDeviceAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_DeviceAtt);
		HATTRIBUTE hObjectAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_ObjectAtt);
		HATTRIBUTE hDefaultAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_DefaultAtt);
		HATTRIBUTE hOffsetAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_OffsetAtt);
		HATTRIBUTE hScaleAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_ScaleAtt);
		HATTRIBUTE hDeadZoneMinAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_DeadZoneMinAtt);
		HATTRIBUTE hDeadZoneMaxAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_DeadZoneMaxAtt);
		HATTRIBUTE hDeadZoneValueAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_DeadZoneValueAtt);
		HATTRIBUTE hCommandMinAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_CommandMinAtt);
		HATTRIBUTE hCommandMaxAttribute = g_pLTDatabase->GetAttribute(hRec, g_prf_CommandMaxAtt);
		if (!hDeviceAttribute || !hObjectAttribute || !hDefaultAttribute || !hOffsetAttribute || 
			!hScaleAttribute || !hDeadZoneMinAttribute || !hDeadZoneMaxAttribute || !hDeadZoneValueAttribute ||
			!hCommandMinAttribute || !hCommandMaxAttribute)
			continue;
		// Figure out how many bindings we've got
		uint32 nNumBindings = g_pLTDatabase->GetNumValues(hDeviceAttribute);
  
		// Read in the bindings
		for (uint32 nCurBinding = 0; nCurBinding < nNumBindings; ++nCurBinding)
  		{
			m_aCommandDeviceBindings[nCurCommand].push_back(SCommandBindingInfo());
			SCommandBindingInfo &sBinding = m_aCommandDeviceBindings[nCurCommand].back();
			sBinding.m_sDeviceName = g_pLTDatabase->GetWString(hDeviceAttribute, nCurBinding, L"Invalid");
			sBinding.m_sObjectName = g_pLTDatabase->GetWString(hObjectAttribute, nCurBinding, L"Invalid");
			sBinding.m_fDefaultValue = g_pLTDatabase->GetFloat(hDefaultAttribute, nCurBinding, 0.0f);
			sBinding.m_fOffset = g_pLTDatabase->GetFloat(hOffsetAttribute, nCurBinding, 0.0f);
			sBinding.m_fScale = g_pLTDatabase->GetFloat(hScaleAttribute, nCurBinding, 1.0f);
			sBinding.m_fDeadZoneMin = g_pLTDatabase->GetFloat(hDeadZoneMinAttribute, nCurBinding, FLT_MAX);
			sBinding.m_fDeadZoneMax = g_pLTDatabase->GetFloat(hDeadZoneMaxAttribute, nCurBinding, -FLT_MAX);
			sBinding.m_fDeadZoneValue = g_pLTDatabase->GetFloat(hDeadZoneValueAttribute, nCurBinding, 0.0f);
			sBinding.m_fCommandMin = g_pLTDatabase->GetFloat(hCommandMinAttribute, nCurBinding, 0.1f);
			sBinding.m_fCommandMax = g_pLTDatabase->GetFloat(hCommandMaxAttribute, nCurBinding, FLT_MAX);
  		}
  	}
#endif // !PLATFORM_XENON

	hRec = g_pLTDatabase->GetRecord(hDB,g_prf_ControlsCat,g_prf_ControlsCat);

	m_bInvertY = GetBool(hRec,g_prf_InvertMouse);
	m_nInputRate = GetInt32(hRec,g_prf_InputRate);
	m_nSensitivity = GetInt32(hRec,g_prf_Sensitivity);
	m_nNormalTurn	= GetInt32(hRec,g_prf_NormalTurn);
	m_nFastTurn		= GetInt32(hRec,g_prf_FastTurn);
	m_nLookUp		= GetInt32(hRec,g_prf_LookUp);

	ApplyControls();
	ApplyBindings();
	ApplyMouse();
	ApplyKeyboard();
	ApplyJoystick();

}


void CUserProfile::LoadMultiplayer(HDATABASE hDB,bool bLoadDefaults)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_MultiplayerCat,g_prf_MultiplayerCat);
	if (!hRec) return;

	m_sPlayerName = GetWString(hRec,g_prf_PlayerName,0, m_sFriendlyName.c_str() );
	m_sPlayerName = FixPlayerName(m_sPlayerName.c_str( ));
	if (!m_sPlayerName.length())
	{
		m_sPlayerName = LoadString("IDS_PLAYER");
	}
	memset( &m_PlayerGuid, 0, sizeof( m_PlayerGuid ));
	char const* pszPlayerGuid = GetString( hRec,g_prf_Guid );
	if( LTStrEmpty( pszPlayerGuid ))
	{
		CreatePlayerGuid( m_PlayerGuid );
	}
	else
	{
		StringToLTGUID( pszPlayerGuid, m_PlayerGuid );
	}

	m_sPlayerPatch = GetString(hRec,g_prf_PlayerPatch,0, ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_sClientShared_DefaultPatch ) );

	m_nDMPlayerModel = (uint8)GetInt32(hRec,g_prf_DMPlayerModel);
	m_nTeamPlayerModel = (uint8)GetInt32(hRec,g_prf_TeamPlayerModel);

	
	// Load the serveroptions file name. Check if there is no options file specified.  If not, then
	// we need to make a default one based on the profile name.
	m_sServerOptionsFile = GetString(hRec,g_prf_ServerOptionsFile);
	if( m_sServerOptionsFile.empty( ))
	{
		char const* pszFileName = CHostOptionsMapMgr::Instance().GetFileNameFromFriendlyName( m_sPlayerName.c_str( ));
		if( LTStrEmpty( pszFileName ))
		{
			// create a new file name that is in ANSI
			char szFileTitle[64];
			char szPath[MAX_PATH*2];
			for(uint32 nFile=0;;++nFile)
			{
				LTSNPrintF( szFileTitle, LTARRAYSIZE(szFileTitle), "ServerOptions%.4d", nFile );
				GameModeMgr::Instance( ).GetOptionsFilePath( szFileTitle, szPath, LTARRAYSIZE( szPath ));
				if( !LTFileOperations::FileExists(szPath) && 
					!CHostOptionsMapMgr::Instance().IsFileNameMapped(szPath) )
					break;
			}

			// Set it as the options file.
			m_sServerOptionsFile = szFileTitle;

			// add this combination
			CHostOptionsMapMgr::Instance().Add( szFileTitle, m_sPlayerName.c_str( ));
		}
		else
		{
			// Set it as the options file.
			m_sServerOptionsFile = pszFileName;
		}
	}
	else
	{
		// Make sure it only has the title.  The full path is created when needed.
		char szFileTitle[MAX_PATH*2];
		LTFileOperations::SplitPath( m_sServerOptionsFile.c_str( ), NULL, szFileTitle, NULL );
		m_sServerOptionsFile = szFileTitle;
	}
	
	m_nBandwidthClient = (uint8)GetInt32(hRec,g_prf_BandwidthClient);
	m_nBandwidthClientCustom = (uint16)GetInt32(hRec,g_prf_BandwidthClientCustom);
	m_nClientPort = (uint16)GetInt32(hRec,g_prf_ClientPort);
	
	m_bAllowContentDownload			= (bool)GetBool(hRec,g_prf_AllowContentDownload);
    m_bAllowContentDownloadRedirect = (bool)GetBool(hRec,g_prf_AllowContentDownloadRedirect);
	m_nMaximumDownloadSize			= (uint32)GetInt32(hRec,g_prf_MaximumDownloadSize, 0, -1);

	// Read in use punkbuster settings.  If it's different than the current setting then change it.
	SetUsePunkbuster((bool)GetBool(hRec,g_prf_UsePunkbuster));
	IPunkBusterClient* pPunkBusterClient = g_pGameClientShell->GetPunkBusterClient();
	if( pPunkBusterClient )
	{
		if( GetUsePunkbuster() && !pPunkBusterClient->IsEnabled( ))
		{
			pPunkBusterClient->Enable( );
		}
		else if( !GetUsePunkbuster() && pPunkBusterClient->IsEnabled( ))
		{
			pPunkBusterClient->Disable( );
		}
	}

	m_bAllowBroadcast				= (bool)GetBool(hRec,g_prf_AllowBroadcast,0,true);
	m_bFilterNavMarkers				= (bool)GetBool(hRec,g_prf_FilterNavMarkers,0,false);

	m_nVersionFilter = (uint8)GetInt32(hRec,g_prf_VersionFilter,0);
	m_nPlayersFilter = (uint8)GetInt32(hRec,g_prf_PlayersFilter,0);
	m_nPingFilter = (uint8)GetInt32(hRec,g_prf_PingFilter,0);
	m_nGameTypeFilter = (uint8)GetInt32(hRec,g_prf_GameTypeFilter,0);
	m_eServerSearchSource = ( ServerSearchSource )GetInt32(hRec,g_prf_ServerSearchSource, 0, eServerSearchSource_Internet);
	m_nCustomizedFilter = (uint8)GetInt32(hRec,g_prf_CustomizedFilter,0);
	m_nRequiresDownloadFilter = (uint8)GetInt32(hRec,g_prf_RequiresDownloadFilter,0);
	m_nPunkbusterFilter = (uint8)GetInt32(hRec,g_prf_PunkbusterFilter,0);

	// Make sure friends list is free before we load it in.
	ClearFriendsList( );

	// Read the friends from the profile.
	HATTRIBUTE hFriendsAttrib = GetAttribute( hRec, g_prf_Friends );
	uint32 nNumFriends = g_pLTDatabase->GetNumValues( hFriendsAttrib );
	for( uint32 nFriendIndex = 0; nFriendIndex < nNumFriends; nFriendIndex++ )
	{
		// Get the nickname.
		wchar_t const* pszFriendNickName = GetWString( hFriendsAttrib, nFriendIndex );
		if( LTStrEmpty( pszFriendNickName ))
			continue;

		// Add the nickname to the list.
		wchar_t* pszNewFriend = LTStrDup( pszFriendNickName );
		m_lstFriends.push_back( pszNewFriend );
	}

	// If we have any friends listed, then make sure the list is sorted.
	if( !m_lstFriends.empty( ))
	{
		std::sort( m_lstFriends.begin( ), m_lstFriends.end( ), FriendsListSort );
	}

	// Make sure favorite server list is free before we load it in.
	ClearFavoriteServerList( );

	// Read the favorite server from the profile.
	HATTRIBUTE hFavoriteServersAttrib = GetAttribute( hRec, g_prf_FavoriteServers );
	uint32 nNumFavoriteServers = g_pLTDatabase->GetNumValues( hFavoriteServersAttrib );
	for( uint32 nFavoriteServerIndex = 0; nFavoriteServerIndex < nNumFavoriteServers; nFavoriteServerIndex++ )
	{
		// Get the IP and Port value.
		HATTRIBUTE hFavoriteServerIPandPort = CGameDatabaseReader::GetStructAttribute( hFavoriteServersAttrib, 
			nFavoriteServerIndex, g_prf_FavoriteServers_IP );
		char const* pszFavoriteServerIPandPort = GetString( hFavoriteServerIPandPort );
		if( LTStrEmpty( pszFavoriteServerIPandPort ))
			continue;
		// Get the SessionName value.
		HATTRIBUTE hFavoriteServerSessionName = CGameDatabaseReader::GetStructAttribute( hFavoriteServersAttrib, 
			nFavoriteServerIndex, g_prf_FavoriteServers_SessionName );
		wchar_t const* pwszFavoriteServerSessionName = GetWString( hFavoriteServerSessionName );
		// Get the LAN status.
		HATTRIBUTE hFavoriteServerLAN = CGameDatabaseReader::GetStructAttribute( hFavoriteServersAttrib, 
			nFavoriteServerIndex, g_prf_FavoriteServers_LAN );
		bool bLAN = GetBool( hFavoriteServerLAN );

		// Add the Favorite Server to the list.
		FavoriteServer* pNewFavoriteServer = new FavoriteServer( pszFavoriteServerIPandPort, pwszFavoriteServerSessionName, bLAN );
		m_lstFavoriteServers.push_back( pNewFavoriteServer );
	}

	// If we have any favorite servers listed, then make sure the list is sorted.
	if( !m_lstFavoriteServers.empty( ))
	{
		std::sort( m_lstFavoriteServers.begin( ), m_lstFavoriteServers.end( ), FavoriteServerSort );
	}
}


void CUserProfile::LoadGameOptions(HDATABASE hDB)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_GameCat,g_prf_GameCat);
	if (!hRec) return;

	m_nDifficulty = GetInt32(hRec,g_prf_Difficulty);
	m_bSubtitles  = GetBool(hRec,g_prf_Subtitles);
	m_bGore = (!g_pVersionMgr->IsLowViolence() && (GetBool(hRec,g_prf_Gore)));
	m_bSlowMoFX  = GetBool(hRec,g_prf_bSlowMoFX,0,true);
	m_bUseTextScaling = GetBool(hRec,g_prf_bUseTextScaling,0,false);

#ifdef PROJECT_DARK

	m_bAlwaysRun = false;
	m_bCrouchToggle = false;

#else

	m_bAlwaysRun = GetBool(hRec,g_prf_AlwaysRun);
	m_bCrouchToggle = GetBool(hRec,g_prf_CrouchToggle);

#endif


	m_nHeadBob = GetInt32(hRec,g_prf_HeadBob);
	m_nMsgDur = GetInt32(hRec,g_prf_MessageDur);
	m_bSPAutoWeaponSwitch = GetBool(hRec,g_prf_SPAutoWeaponSwitch);
	m_bMPAutoWeaponSwitch = GetBool(hRec,g_prf_MPAutoWeaponSwitch);
	m_bPersistentHUD = GetBool(hRec,g_prf_PersistentHUD);
	m_fHUDFadeSpeed = GetFloat(hRec,g_prf_HUDFadeSpeed);

//	m_bCrosshair = GetBool(hRec,g_prf_UseCrosshair);
	uint32 nCrosshairColor = GetInt32(hRec,g_prf_CrosshairColor);
	GET_ARGB(nCrosshairColor,m_CrosshairA,m_CrosshairR,m_CrosshairG,m_CrosshairB)
	m_nCrosshairSize = (uint8)GetInt32(hRec,g_prf_CrosshairSize);
	
	ApplyGameOptions();
	ApplyCrosshair();

}

void CUserProfile::LoadSound(HDATABASE hDB,bool bApply)
{
	HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_SoundCat,g_prf_SoundCat);
	if (!hRec) return;

	m_fSoundVolume = GetFloat(hRec,g_prf_SoundVolume,0,1.0f);
	m_fMusicVolume = GetFloat(hRec,g_prf_MusicVolume,0,1.0f);
	m_fSpeechVolume = GetFloat(hRec,g_prf_SpeechVolume,0,1.0f);

	m_bUseHWSound = GetBool(hRec,g_prf_UseHWSound,0,false);
	m_bUseEAX = GetBool(hRec,g_prf_UseEAX,0,false);
	m_bUseEAX4 = GetBool(hRec,g_prf_UseEAX4,0,false);

	if (bApply)
		ApplySound();

}
void CUserProfile::LoadWeaponPriorities(HDATABASE hDB, bool bLoadDefaults)
{
	m_vecWeapons.clear();

	if (bLoadDefaults)
	{
		m_vecWeapons.reserve(g_pWeaponDB->GetNumDefaultWeaponPriorities());
		//load defaults from WeaponDB
		for (uint8 nWpn = 0; nWpn < g_pWeaponDB->GetNumDefaultWeaponPriorities(); ++nWpn)
		{
			HWEAPON hWpn = g_pWeaponDB->GetWeaponFromDefaultPriority(nWpn);
			m_vecWeapons.push_back(hWpn);
		}
	}
	else
	{
		HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_WeaponsCat,g_prf_WeaponsCat);
		if (!hRec) return;

		HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,g_prf_WeaponPriority);
		if (!hAtt) return;

		m_vecWeapons.reserve(g_pLTDatabase->GetNumValues(hAtt));
		for (uint32 nWpn = 0; nWpn < g_pLTDatabase->GetNumValues(hAtt); ++nWpn)
		{
			std::string sWpn = g_pLTDatabase->GetString(hAtt,nWpn,"");
			HWEAPON hWpn = g_pWeaponDB->GetWeaponRecord(sWpn.c_str());
			if (hWpn)
			{
				m_vecWeapons.push_back(hWpn);
			}
		}
	}


	
	ApplyWeaponPriorities();
}




void CUserProfile::Save()
{

#if defined(PLATFORM_XENON) // JPW Xenon saves will need to be signed and this code byteswaps the on-disk profile so it has to be refactored at some point before ship anyway
	return;
#endif // PLATFORM_XENON

	//make sure the root profile folder exists
	std::string fn = GetAbsoluteProfileDir( g_pLTClient, "");
	if( !CWinUtil::DirExist( fn.c_str() ))
	{
		if( !CWinUtil::CreateDir( fn.c_str() ))
		{
			//TODO: error message
			return;
		}
	}


	HDATABASECREATOR hDBC = g_pLTDatabaseCreator->CreateNewDatabase();
	if (!hDBC) return; //assert?

	SaveVersion(hDBC);
	SaveControls(hDBC);
	SaveMultiplayer(hDBC);
	SaveGameOptions(hDBC);
	SaveSound(hDBC);
	SaveWeaponPriorities(hDBC);
	
	CPerformanceMgr::Instance().Save(hDBC);

	const char* pszFilename = GetRelativeProfileFile( g_pLTClient, m_sFileName.c_str( ));

	ILTOutStream* pOutFile = g_pLTClient->FileMgr()->OpenUserFileForWriting(pszFilename);
	if(pOutFile)
	{
		LTOutNullConverter OutConverter(*pOutFile);
		g_pLTDatabaseCreator->SaveDatabase(hDBC, OutConverter);
		g_pLTDatabaseCreator->ReleaseDatabase(hDBC);
	}
	else
	{
		LTERROR_PARAM1("Unable to save profile %s.",pszFilename);
	}
	

}

void CUserProfile::SaveVersion(HDATABASECREATOR hDBC)
{
	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_VersionCat,"Profile.Version");
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_VersionCat);
	if (!hRec) return;

	m_nSaveVersion = g_pVersionMgr->GetSaveVersion( );
	CreateInt32(hRec,g_prf_VersionNumber, m_nSaveVersion );

	CreateWString(hRec,g_prf_FriendlyName,m_sFriendlyName.c_str());
}


void CUserProfile::SaveControls(HDATABASECREATOR hDBC)
{

	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_BindingsCat,"Profile.Bindings");
	if (!hCat) return;

	HRECORDCREATOR hRec;
  
	// Write the command bindings
	for (TCommandBindingList::const_iterator iCurCommand = m_aCommandDeviceBindings.begin();
		iCurCommand != m_aCommandDeviceBindings.end();
		++iCurCommand)
  	{
		if (iCurCommand->empty())
			continue;

		const CommandData *pCommand = GetCommandData(iCurCommand - m_aCommandDeviceBindings.begin());

		hRec = g_pLTDatabaseCreator->CreateRecord(hCat, pCommand->m_szActionString);
		if (!hRec) 
			continue;

		// Look up the attributes
		HATTRIBUTECREATOR hDeviceAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_DeviceAtt, eAttributeType_WString,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hObjectAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_ObjectAtt, eAttributeType_WString,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hDefaultAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_DefaultAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hOffsetAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_OffsetAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hScaleAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_ScaleAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hDeadZoneMinAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_DeadZoneMinAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hDeadZoneMaxAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_DeadZoneMaxAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hDeadZoneValueAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_DeadZoneValueAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hCommandMinAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_CommandMinAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		HATTRIBUTECREATOR hCommandMaxAttribute = g_pLTDatabaseCreator->CreateAttribute(hRec, g_prf_CommandMaxAtt, eAttributeType_Float,eAttributeUsage_Default, iCurCommand->size());
		if (!hDeviceAttribute || !hObjectAttribute || !hDefaultAttribute || !hOffsetAttribute || 
			!hScaleAttribute || !hDeadZoneMinAttribute || !hDeadZoneMaxAttribute || !hDeadZoneValueAttribute ||
			!hCommandMinAttribute || !hCommandMaxAttribute)
  			continue;
  
		// Write out the bindings
		for (TCommandBindingInfoList::const_iterator iCurBinding = iCurCommand->begin();
			iCurBinding != iCurCommand->end();
			++iCurBinding)
  		{
			uint32 nCurBinding = iCurBinding - iCurCommand->begin();
			g_pLTDatabaseCreator->SetWString(hDeviceAttribute, nCurBinding, iCurBinding->m_sDeviceName.c_str());
			g_pLTDatabaseCreator->SetWString(hObjectAttribute, nCurBinding, iCurBinding->m_sObjectName.c_str());
			g_pLTDatabaseCreator->SetFloat(hDefaultAttribute, nCurBinding, iCurBinding->m_fDefaultValue);
			g_pLTDatabaseCreator->SetFloat(hOffsetAttribute, nCurBinding, iCurBinding->m_fOffset);

			g_pLTDatabaseCreator->SetFloat(hScaleAttribute, nCurBinding, iCurBinding->m_fScale);
			g_pLTDatabaseCreator->SetFloat(hDeadZoneMinAttribute, nCurBinding, iCurBinding->m_fDeadZoneMin);
			g_pLTDatabaseCreator->SetFloat(hDeadZoneMaxAttribute, nCurBinding, iCurBinding->m_fDeadZoneMax);
			g_pLTDatabaseCreator->SetFloat(hDeadZoneValueAttribute, nCurBinding, iCurBinding->m_fDeadZoneValue);
			g_pLTDatabaseCreator->SetFloat(hCommandMinAttribute, nCurBinding, iCurBinding->m_fCommandMin);
			g_pLTDatabaseCreator->SetFloat(hCommandMaxAttribute, nCurBinding, iCurBinding->m_fCommandMax);

  		}
  	}

	//************************ save controls
	hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_ControlsCat,"Profile.Controls");
	if (!hCat) return;

	hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_ControlsCat);
	if (!hRec) return;

	//save mouse
	CreateBool(hRec,g_prf_InvertMouse,m_bInvertY);

    CreateInt32(hRec,g_prf_InputRate,m_nInputRate);
    CreateInt32(hRec,g_prf_Sensitivity,m_nSensitivity);

	CreateInt32(hRec,g_prf_NormalTurn,m_nNormalTurn);
	CreateInt32(hRec,g_prf_FastTurn,m_nFastTurn);
	CreateInt32(hRec,g_prf_LookUp,m_nLookUp);
}


void CUserProfile::SaveMultiplayer(HDATABASECREATOR hDBC)
{
	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_MultiplayerCat,"Profile.Multiplayer");
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_MultiplayerCat);
	if (!hRec) return;

	CreateWString(hRec,g_prf_PlayerName,m_sPlayerName.c_str());
	CreateString(hRec,g_prf_PlayerPatch,m_sPlayerPatch.c_str());
	char szPlayerGuid[64];
	LTGUIDToString( szPlayerGuid, LTARRAYSIZE( szPlayerGuid ), m_PlayerGuid );
	CreateString(hRec,g_prf_Guid, szPlayerGuid);
	CreateInt32(hRec,g_prf_DMPlayerModel,m_nDMPlayerModel);
	CreateInt32(hRec,g_prf_TeamPlayerModel,m_nTeamPlayerModel);
	CreateString(hRec,g_prf_ServerOptionsFile,m_sServerOptionsFile.c_str( ));

	CreateInt32(hRec,g_prf_BandwidthClient,m_nBandwidthClient);
	CreateInt32(hRec,g_prf_BandwidthClientCustom,m_nBandwidthClientCustom);
	CreateInt32(hRec,g_prf_ClientPort,m_nClientPort);

	CreateBool(hRec,g_prf_AllowContentDownload,m_bAllowContentDownload);
	CreateBool(hRec,g_prf_AllowContentDownloadRedirect,m_bAllowContentDownloadRedirect);
	CreateInt32(hRec,g_prf_MaximumDownloadSize,m_nMaximumDownloadSize);

	CreateBool(hRec,g_prf_UsePunkbuster,GetUsePunkbuster());

	CreateBool(hRec,g_prf_AllowBroadcast, m_bAllowBroadcast);
	CreateBool(hRec,g_prf_FilterNavMarkers, m_bFilterNavMarkers);

	CreateInt32(hRec,g_prf_VersionFilter,m_nVersionFilter);
	CreateInt32(hRec,g_prf_PlayersFilter,m_nPlayersFilter);
	CreateInt32(hRec,g_prf_PingFilter,m_nPingFilter);
	CreateInt32(hRec,g_prf_GameTypeFilter,m_nGameTypeFilter);
	CreateInt32(hRec,g_prf_ServerSearchSource,m_eServerSearchSource);
	CreateInt32(hRec,g_prf_CustomizedFilter,m_nCustomizedFilter);
	CreateInt32(hRec,g_prf_RequiresDownloadFilter,m_nRequiresDownloadFilter);
	CreateInt32(hRec,g_prf_PunkbusterFilter,m_nPunkbusterFilter);

	// Create the attribute for the friends.
	HATTRIBUTECREATOR hFriendsAttrib = g_pLTDatabaseCreator->CreateAttribute( hRec, g_prf_Friends, eAttributeType_WString, eAttributeUsage_Default, m_lstFriends.size( ));

	// Iterate through the list of friends and save them out.
	uint32 nFriendIndex = 0;
	for( TFriendList::iterator iter = m_lstFriends.begin(); iter != m_lstFriends.end( ); iter++, nFriendIndex++ )
	{
		wchar_t const* pwszFriendNickName = *iter;
		g_pLTDatabaseCreator->SetWString( hFriendsAttrib, nFriendIndex, pwszFriendNickName );
	}

	// Create the attribute for the favorite servers.
	HATTRIBUTECREATOR hFavoriteServersAttrib = g_pLTDatabaseCreator->CreateAttribute( hRec, g_prf_FavoriteServers, eAttributeType_WString, eAttributeUsage_Default, m_lstFavoriteServers.size( ));

	// Iterate through the list of favorite servers and save them out.
	uint32 nFavoriteServerIndex = 0;
	char szAttributeCompleteName[256];
	for( TFavoriteServerList::iterator iter = m_lstFavoriteServers.begin(); iter != m_lstFavoriteServers.end( ); iter++, nFavoriteServerIndex++ )
	{
		FavoriteServer const* pFavoriteServer = *iter;

		// Create the struct attribute for the IP and Port.
		LTSNPrintF( szAttributeCompleteName, LTARRAYSIZE( szAttributeCompleteName ), "%s.%d.%s", g_prf_FavoriteServers, 
			nFavoriteServerIndex, g_prf_FavoriteServers_IP );
		HATTRIBUTECREATOR hIPandPort = g_pLTDatabaseCreator->CreateAttribute( hRec, szAttributeCompleteName, eAttributeType_String, eAttributeUsage_Default, 1 );
		g_pLTDatabaseCreator->SetString( hIPandPort, 0, pFavoriteServer->GetServerIPandPort( ));

		// Create the struct attribute for the Server SessionName.
		LTSNPrintF( szAttributeCompleteName, LTARRAYSIZE( szAttributeCompleteName ), "%s.%d.%s", g_prf_FavoriteServers, 
			nFavoriteServerIndex, g_prf_FavoriteServers_SessionName );
		HATTRIBUTECREATOR hSessionName = g_pLTDatabaseCreator->CreateAttribute( hRec, szAttributeCompleteName, eAttributeType_WString, eAttributeUsage_Default, 1 );
		g_pLTDatabaseCreator->SetWString( hSessionName, 0, pFavoriteServer->GetServerSessionName( ));

		// Create the struct attribute for the Server LAN status.
		LTSNPrintF( szAttributeCompleteName, LTARRAYSIZE( szAttributeCompleteName ), "%s.%d.%s", g_prf_FavoriteServers, 
			nFavoriteServerIndex, g_prf_FavoriteServers_LAN );
		HATTRIBUTECREATOR hBool = g_pLTDatabaseCreator->CreateAttribute( hRec, szAttributeCompleteName, eAttributeType_Bool, eAttributeUsage_Default, 1 );
		g_pLTDatabaseCreator->SetBool( hBool, 0, pFavoriteServer->GetLAN( ));
	}
}

void CUserProfile::SaveGameOptions(HDATABASECREATOR hDBC)
{
	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_GameCat,"Profile.Game");
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_GameCat);
	if (!hRec) return;

	CreateInt32(hRec,g_prf_Difficulty,m_nDifficulty);
    CreateBool(hRec,g_prf_Subtitles,m_bSubtitles);
    CreateBool(hRec,g_prf_Gore,(!g_pVersionMgr->IsLowViolence() && m_bGore));
	CreateBool(hRec,g_prf_bSlowMoFX,m_bSlowMoFX);
	CreateBool(hRec,g_prf_bUseTextScaling,m_bUseTextScaling);
	CreateBool(hRec,g_prf_AlwaysRun,m_bAlwaysRun);
	CreateBool(hRec,g_prf_CrouchToggle,m_bCrouchToggle);
	CreateInt32(hRec,g_prf_HeadBob,m_nHeadBob);
	CreateInt32(hRec,g_prf_MessageDur,m_nMsgDur);
	CreateBool(hRec,g_prf_SPAutoWeaponSwitch,m_bSPAutoWeaponSwitch);
	CreateBool(hRec,g_prf_MPAutoWeaponSwitch,m_bMPAutoWeaponSwitch);
	CreateBool(hRec,g_prf_PersistentHUD,m_bPersistentHUD);
	CreateFloat(hRec,g_prf_HUDFadeSpeed,m_fHUDFadeSpeed);
	

	uint32 nCC = SET_ARGB(m_CrosshairA,m_CrosshairR,m_CrosshairG,m_CrosshairB);
	
	CreateInt32(hRec,g_prf_CrosshairColor,nCC);
	CreateInt32(hRec,g_prf_CrosshairSize,m_nCrosshairSize);
	CreateInt32(hRec,g_prf_VersionNumber, m_nSaveVersion );

}


void CUserProfile::SaveSound(HDATABASECREATOR hDBC)
{
	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_SoundCat,"Profile.Sound");
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_SoundCat);
	if (!hRec) return;

	CreateFloat(hRec,g_prf_SoundVolume,m_fSoundVolume);
	CreateFloat(hRec,g_prf_SpeechVolume,m_fSpeechVolume);
	CreateFloat(hRec,g_prf_MusicVolume,m_fMusicVolume);

	CreateBool(hRec,g_prf_UseHWSound,m_bUseHWSound);
	CreateBool(hRec,g_prf_UseEAX,m_bUseEAX);
	CreateBool(hRec,g_prf_UseEAX4,m_bUseEAX4);
}


void CUserProfile::SaveWeaponPriorities(HDATABASECREATOR hDBC)
{

	HCATEGORYCREATOR hCat = g_pLTDatabaseCreator->CreateCategory(hDBC,g_prf_WeaponsCat,g_prf_WeaponsCat);
	if (!hCat) return;

	HRECORDCREATOR hRec = g_pLTDatabaseCreator->CreateRecord(hCat,g_prf_WeaponsCat);
	if (!hRec) return;

	HATTRIBUTECREATOR hAtt = g_pLTDatabaseCreator->CreateAttribute(hRec,g_prf_WeaponPriority,eAttributeType_String,eAttributeUsage_Default,m_vecWeapons.size());
	if (!hAtt) return;

	for (uint32 nWpn = 0; nWpn < m_vecWeapons.size(); ++nWpn)
	{
		g_pLTDatabaseCreator->SetString(hAtt,nWpn,g_pWeaponDB->GetRecordName(m_vecWeapons[nWpn]));
	}
	
}




void CUserProfile::Term()
{
	m_bInitted = false;
}

const wchar_t* CUserProfile::GetTriggerNameFromCommandID(int nCommand)
{
	SetBindings();
	static CBindMgr::TBindingList s_aBindings;
	s_aBindings.resize(0);
	CBindMgr::GetSingleton().GetCommandBindings(nCommand, &s_aBindings);

	char szDeviceObjectName[256] = "";
	static wchar_t strControls[256] = L"";
	strControls[0] = NULL;

	if (s_aBindings.empty())
		return strControls;

	// Temporary implementation that uses just the first binding
	ILTInput::SDeviceObjectDesc sObjDesc;
	ILTInput::SDeviceDesc sDeviceDesc;
	g_pLTInput->GetDeviceDesc(s_aBindings.front().m_nDevice, &sDeviceDesc); 
	g_pLTInput->GetDeviceObjectDesc(s_aBindings.front().m_nDevice, s_aBindings.front().m_nObject, &sObjDesc); 
	if (sDeviceDesc.m_eCategory == ILTInput::eDC_Keyboard)
		LTStrCpy(strControls, sObjDesc.m_sDisplayName, LTARRAYSIZE(strControls));
	else
	{
		LTStrCpy(strControls, sDeviceDesc.m_sDisplayName, LTARRAYSIZE(strControls));
		LTStrCat(strControls, L" ", LTARRAYSIZE(strControls));
		LTStrCat(strControls, sObjDesc.m_sDisplayName, LTARRAYSIZE(strControls));
	}
	return strControls;
}


uint8 CUserProfile::GetWeaponPriority(HWEAPON hWpn) const
{
	for (uint8 nIndex = 0; nIndex < m_vecWeapons.size(); ++nIndex)
	{
		if (m_vecWeapons[nIndex] == hWpn)
			return nIndex;
	}
	return WDB_INVALID_WEAPON_INDEX;
}


//**************************************************************************
//functions to read settings from the profile and write them to the console
//**************************************************************************

	//take bindings from profile and apply them to the game
void CUserProfile::ApplyBindings()
{
	CBindMgr::GetSingleton().ClearBindings();
  
	for (TCommandBindingList::const_iterator iCurCommand = m_aCommandDeviceBindings.begin();
		iCurCommand != m_aCommandDeviceBindings.end();
		++iCurCommand)
  	{
		uint32 nCommandID = GetCommandData(iCurCommand - m_aCommandDeviceBindings.begin())->m_nCommandID;
		for (TCommandBindingInfoList::const_iterator iCurBinding = iCurCommand->begin();
			iCurBinding != iCurCommand->end();
			++iCurBinding)
  		{
			// Look up the device by this name
			uint32 nDeviceIndex;
			if (g_pLTInput->FindDeviceByName(iCurBinding->m_sDeviceName.c_str(), &nDeviceIndex) != LT_OK)
				continue;
  
			// Look up the object on the device
			uint32 nObjectIndex;
			if (g_pLTInput->FindDeviceObjectByName(nDeviceIndex, iCurBinding->m_sObjectName.c_str(), &nObjectIndex) != LT_OK)
				continue;
  
			// Bind it..
			CBindMgr::SBinding sBinding;
			sBinding.m_nCommand = nCommandID;
			sBinding.m_nDevice = nDeviceIndex;
			sBinding.m_nObject = nObjectIndex;
			sBinding.m_fDefaultValue = iCurBinding->m_fDefaultValue;
			sBinding.m_fOffset = iCurBinding->m_fOffset;
			sBinding.m_fScale = iCurBinding->m_fScale;
			sBinding.m_fDeadZoneMin = iCurBinding->m_fDeadZoneMin;
			sBinding.m_fDeadZoneMax = iCurBinding->m_fDeadZoneMax;
			sBinding.m_fDeadZoneValue = iCurBinding->m_fDeadZoneValue;
			sBinding.m_fCommandMin = iCurBinding->m_fCommandMin;
			sBinding.m_fCommandMax = iCurBinding->m_fCommandMax;
			CBindMgr::GetSingleton().SetBinding(sBinding);
  		}
	}
  
	// Apply the mouse bindings too so they don't get lost.
	ApplyMouse();

	if( g_pHUDWeaponList )
		g_pHUDWeaponList->UpdateTriggerNames();

	if( g_pHUDGrenadeList )
		g_pHUDGrenadeList->UpdateTriggerNames();
}


void CUserProfile::ApplyMouse()
{
	WriteConsoleInt("MouseInvertY",m_bInvertY);
	WriteConsoleFloat("MouseSensitivity",(float)m_nSensitivity);
	WriteConsoleFloat("InputRate",(float)m_nInputRate);

//	float fTemp = (float)m_nVehicleTurn / 100.0f;
//	WriteConsoleFloat("VehicleTurnRateScale",fTemp);

	// Always bind the X & Y axes of the mouse to pitch and yaw
	uint32 nMouseDevice;
	if (g_pLTInput->FindFirstDeviceByCategory(ILTInput::eDC_Mouse, &nMouseDevice) == LT_OK)
	{
		// Figure out which axes are the X & Y
		uint32 nXAxis = ILTInput::k_InvalidIndex, nYAxis = ILTInput::k_InvalidIndex;

		uint32 nFirstAxis;
		g_pLTInput->FindFirstDeviceObjectByCategory(nMouseDevice, ILTInput::eDOC_Axis, &nFirstAxis);
		uint32 nNumAxes;
		g_pLTInput->GetNumDeviceObjectsByCategory(nMouseDevice, ILTInput::eDOC_Axis, &nNumAxes);

		for (uint32 nCurAxis = 0; nCurAxis < nNumAxes; ++nCurAxis)
		{
			ILTInput::SDeviceObjectDesc sDesc;
			g_pLTInput->GetDeviceObjectDesc(nMouseDevice, nCurAxis + nFirstAxis, &sDesc);
			if (sDesc.m_nControlCode == 0)
				nXAxis = nCurAxis + nFirstAxis;
			else if (sDesc.m_nControlCode == 1)
				nYAxis = nCurAxis + nFirstAxis;
		}

		// Set up the common binding information
		float fSensitivityMultiplier = (float)(m_nSensitivity + 1) * 0.25f;

		CBindMgr::SBinding sBinding;
		sBinding.m_nDevice = nMouseDevice;
		sBinding.m_fOffset = -1.0f * fSensitivityMultiplier;
		sBinding.m_fScale = 2.0f * fSensitivityMultiplier;
		sBinding.m_fDefaultValue = 0.0f;

		// Bind the axes
		sBinding.m_nObject = nXAxis;
		sBinding.m_nCommand = COMMAND_ID_YAW;
		CBindMgr::GetSingleton().SetBinding(sBinding);
		sBinding.m_nObject = nYAxis;
		sBinding.m_nCommand = COMMAND_ID_PITCH;
		CBindMgr::GetSingleton().SetBinding(sBinding);
	}
}

void CUserProfile::ApplyKeyboard()
{

	float fTemp = (float)m_nNormalTurn / 10.0f;
	WriteConsoleFloat("NormalTurnRate",fTemp);

	fTemp = (float)m_nFastTurn / 10.0f;
	WriteConsoleFloat("FastTurnRate",fTemp);

	fTemp = (float)m_nLookUp / 10.0f;
	WriteConsoleFloat("LookUpRate",fTemp);

//	fTemp = (float)m_nVehicleTurn / 100.0f;
//	WriteConsoleFloat("VehicleTurnRateScale",fTemp);
}

void CUserProfile::ApplyGameOptions()
{
	int nGore = (int)(!g_pVersionMgr->IsLowViolence() && m_bGore);
	WriteConsoleInt("Gore", nGore);
	WriteConsoleBool("Subtitles",m_bSubtitles);
	WriteConsoleInt("Difficulty",m_nDifficulty);

	GameDifficulty eDiff = GD_NORMAL;
	if (GameModeMgr::Instance( ).m_grbUsesDifficulty)
	{
		eDiff = (GameDifficulty)m_nDifficulty;
	}
	g_pGameClientShell->SetDifficulty(eDiff);

	

	WriteConsoleFloat("HeadBob",((float)m_nHeadBob / 10.0f));
	WriteConsoleFloat("MessageDuration",((float)m_nMsgDur / 10.0f));

	WriteConsoleBool("SPAutoWeaponSwitch",m_bSPAutoWeaponSwitch);
	WriteConsoleBool("MPAutoWeaponSwitch",m_bMPAutoWeaponSwitch);

	g_pMoveMgr->SetRunLock(m_bAlwaysRun);

	if (m_bCrouchToggle)
	{
		g_pMoveMgr->SetDuckLock(g_pMoveMgr->IsDucking());
	}
	else
	{
		g_pMoveMgr->SetDuckLock(false);
	}

	g_pGameClientShell->UpdateGoreSettings();
}

void CUserProfile::ApplyCrosshair()
{
    WriteConsoleInt("CrosshairRed",m_CrosshairR );
    WriteConsoleInt("CrosshairGreen",m_CrosshairG );
    WriteConsoleInt("CrosshairBlue",m_CrosshairB );
    WriteConsoleInt("CrosshairAlpha",m_CrosshairA );
	WriteConsoleInt("CrosshairSize",m_nCrosshairSize );

}

void CUserProfile::ApplySound()
{
	const float fMusicVolume = GetConsoleInt("MusicEnable",1) != 0 ? m_fMusicVolume : 0.0f;
	g_pGameClientShell->GetMixer()->SetMainVolumeLevels(m_fSoundVolume,fMusicVolume,m_fSpeechVolume);

	// set the preferences

	SoundCapabilities SoundCaps;

	SoundCaps.bSupportsHWSounds = m_bUseHWSound;
	SoundCaps.bSupportsEAX = m_bUseEAX;
	SoundCaps.bSupportsEAX4 = m_bUseEAX4;
	g_pLTClient->SoundMgr()->SetSoundCapabilityPreferences(SoundCaps);
}



void CUserProfile::ApplyDisplay()
{

	bool initVS = !!GetConsoleInt("VSyncOnFlip",0);
	WriteConsoleInt("HardwareCursor",m_bHardwareCursor);
	WriteConsoleInt("VSyncOnFlip",m_bVSync);
	WriteConsoleInt("RestartRenderBetweenMaps",m_bRestartRenderBetweenMaps);

	bool initTS = GetConsoleBool("UseTextScaling",false);
	WriteConsoleBool("UseTextScaling",m_bUseTextScaling);
	bool bScalingChanged = (initTS != GetConsoleBool("UseTextScaling",false));

	

	bool bGammaChanged = (m_fGamma != GetConsoleFloat("GammaR",kDefaultGamma));
	WriteConsoleFloat("GammaR",m_fGamma);
	WriteConsoleFloat("GammaG",m_fGamma);
	WriteConsoleFloat("GammaB",m_fGamma);

	bool bRestart = (initVS != m_bVSync) || bGammaChanged;
	
	RMode currentMode;
	if (g_pGameClientShell->IsRendererInitted() && g_pLTClient->GetRenderMode(&currentMode) == LT_OK)
	{
		// Build the list of render modes
		RMode *pRenderModes = g_pLTClient->GetRenderer()->GetRenderModes();
		RMode *pMode=pRenderModes;
		bool bFound = false;
		while (pMode != NULL && !bFound)
		{
			bFound = (	pMode->m_Width == m_nScreenWidth &&
						pMode->m_Height == m_nScreenHeight &&
						pMode->m_BitDepth == m_nScreenDepth &&
						pMode->m_bHWTnL == currentMode.m_bHWTnL);
			if (!bFound)
				pMode = pMode->m_pNext;
		}


		if (pMode && !IsRendererEqual(pMode,&currentMode))
		{
			//switch renderers
			g_pInterfaceResMgr->DrawMessage("IDS_REINITIALIZING_RENDERER");
			g_pLTClient->SetRenderMode(pMode);
			bScalingChanged = true;

			//update the game camera to have the new dimensions
			if (g_pPlayerMgr && g_pPlayerMgr->GetPlayerCamera())
			{
				g_pLTClient->SetCameraRect(g_pPlayerMgr->GetPlayerCamera()->GetCamera(), g_pInterfaceMgr->GetViewportRect());
			}
			
			bRestart = false;
		}

		// Free the linked list of render modes
		g_pLTClient->GetRenderer()->RelinquishRenderModes(pRenderModes);
	}

	
	//make sure to save out these changed settings so that they will be applied next
	//time we run
	//SaveDisplaySettings();
	SaveSettings();

	if (bRestart)
	{
		// Set the renderer mode
		g_pInterfaceMgr->SetSwitchingRenderModes(true);
		g_pLTClient->RunConsoleCommand("RestartRender");
		g_pInterfaceMgr->SetSwitchingRenderModes(false);

		//force this because restarting the renderer may affect the cursor
		g_pCursorMgr->UseHardwareCursor(m_bHardwareCursor,true);
	}

	if (bScalingChanged)
	{
		g_pInterfaceMgr->ScreenDimsChanged();
	}

}

void CUserProfile::ApplyWeaponPriorities()
{
	if (g_pLTClient->IsConnected())
		SendWeaponPriorityMsg();
}

void CUserProfile::SendWeaponPriorityMsg()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_PRIORITY );
	uint8 nNum = m_vecWeapons.size();

	cMsg.Writeuint8( nNum );

	for (uint8 n = 0; n < nNum; ++n)
	{
		cMsg.WriteDatabaseRecord(g_pLTDatabase,m_vecWeapons[n]);
	}
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

void CUserProfile::ApplyControls()
{
	// Nothing to do
}

void CUserProfile::ApplyJoystick()
{
	// Nothing to do
}


//**************************************************************************
//functions to read settings from the console and write them to the profile
//**************************************************************************

//read bindings from the game and save them in the profile
void CUserProfile::SetBindings()
{
	CBindMgr::TBindingList aBindings;

	for (TCommandBindingList::iterator iCurCommand = m_aCommandDeviceBindings.begin();
		iCurCommand != m_aCommandDeviceBindings.end();
		++iCurCommand)
  	{
		iCurCommand->resize(0);
		uint32 nCommandID = GetCommandData(iCurCommand - m_aCommandDeviceBindings.begin())->m_nCommandID;
		CBindMgr::GetSingleton().GetCommandBindings(nCommandID, &aBindings);
		for (CBindMgr::TBindingList::const_iterator iCurBinding = aBindings.begin();
			iCurBinding != aBindings.end();
			++iCurBinding)
  		{
			// Look up the device name
			ILTInput::SDeviceDesc sDeviceDesc;
			if (g_pLTInput->GetDeviceDesc(iCurBinding->m_nDevice, &sDeviceDesc) != LT_OK)
				continue;
  
			// Look up the object name
			ILTInput::SDeviceObjectDesc sObjectDesc;
			if (g_pLTInput->GetDeviceObjectDesc(iCurBinding->m_nDevice, iCurBinding->m_nObject, &sObjectDesc) != LT_OK)
				continue;
  
			// Add a binding entry
			iCurCommand->push_back(SCommandBindingInfo());
			SCommandBindingInfo &sBinding = iCurCommand->back();
  
			// Read the values...
			sBinding.m_sDeviceName = sDeviceDesc.m_sName;
			sBinding.m_sObjectName = sObjectDesc.m_sName;
			sBinding.m_fDefaultValue = iCurBinding->m_fDefaultValue;
			sBinding.m_fOffset = iCurBinding->m_fOffset;
			sBinding.m_fScale = iCurBinding->m_fScale;
			sBinding.m_fDeadZoneMin = iCurBinding->m_fDeadZoneMin;
			sBinding.m_fDeadZoneMax = iCurBinding->m_fDeadZoneMax;
			sBinding.m_fDeadZoneValue = iCurBinding->m_fDeadZoneValue;
			sBinding.m_fCommandMin = iCurBinding->m_fCommandMin;
			sBinding.m_fCommandMax = iCurBinding->m_fCommandMax;
  		}
  	}
}

void CUserProfile::SetMouse()
{
	int nMin, nMax;

	m_bInvertY = !!GetConsoleInt("MouseInvertY",0);

	m_nInputRate = GetConsoleInt("InputRate",20);
	
	nMin = int(g_vtMouseMinInputRate.GetFloat());
	nMax = int(g_vtMouseMaxInputRate.GetFloat());

	m_nInputRate = LTCLAMP(m_nInputRate, nMin, nMax);

	m_nSensitivity = GetConsoleInt("MouseSensitivity",7);

	nMin = int(g_vtMouseMinSensitivity.GetFloat());
	nMax = int(g_vtMouseMaxSensitivity.GetFloat());

	m_nSensitivity = LTCLAMP(m_nSensitivity, nMin, nMax);

//	float fTemp = GetConsoleFloat("VehicleTurnRateScale",1.0f);
//	m_nVehicleTurn = (int)(100.0f * fTemp);
}

void CUserProfile::SetKeyboard()
{
	//keyboard settings

	float fTemp = GetConsoleFloat("NormalTurnRate",1.5f);
	m_nNormalTurn = (int)(10.0f * fTemp);

	fTemp = GetConsoleFloat("FastTurnRate",2.3f);
	m_nFastTurn = (int)(10.0f * fTemp);

	fTemp = GetConsoleFloat("LookUpRate",2.5f);
	m_nLookUp = (int)(10.0f * fTemp);

}


void CUserProfile::SetJoystick()
{
	// Nothing to do
}


void CUserProfile::SetControls()
{
	// Nothing to do
}


void CUserProfile::SetGameOptions()
{
	m_bGore = !g_pVersionMgr->IsLowViolence() && !!GetConsoleInt("Gore",0);
	m_nDifficulty = g_pGameClientShell->GetDifficulty();
	m_bSubtitles = GetConsoleBool("Subtitles",false);

#ifdef PROJECT_DARK

	m_bAlwaysRun = false;

#else

	m_bAlwaysRun = (bool)g_pMoveMgr->RunLock();

#endif


	m_nHeadBob = (int)(10.0f * GetConsoleFloat("HeadBob",1.0f));
	m_nMsgDur = (int)(10.0f * GetConsoleFloat("MessageDuration",1.0f));

	m_bSPAutoWeaponSwitch = !!GetConsoleInt("SPAutoWeaponSwitch",1);
	m_bMPAutoWeaponSwitch = !!GetConsoleInt("MPAutoWeaponSwitch",0);
}


void CUserProfile::SetCrosshair()
{
	m_CrosshairR = GetConsoleInt("CrosshairRed",0x1F);
	m_CrosshairG = GetConsoleInt("CrosshairGreen",0xFF);
	m_CrosshairB = GetConsoleInt("CrosshairBlue",0xFF);
	m_CrosshairA = GetConsoleInt("CrosshairAlpha",0xBF);
	m_nCrosshairSize = GetConsoleInt("CrosshairSize",12 );

}

void CUserProfile::SetSound()
{
}

void CUserProfile::SetDisplay()
{
	m_bHardwareCursor = (GetConsoleInt("HardwareCursor",1) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);
	m_bVSync = (GetConsoleInt("VSyncOnFlip",0) > 0 );
	m_bRestartRenderBetweenMaps = (GetConsoleInt("RestartRenderBetweenMaps",0) > 0 );

	//get the average
	m_fGamma = (GetConsoleFloat("GammaR",kDefaultGamma) + GetConsoleFloat("GammaG",kDefaultGamma) + GetConsoleFloat("GammaB",kDefaultGamma)) / 3.0f;

	// The current render mode
	RMode currentMode;
	g_pLTClient->GetRenderMode(&currentMode);

	m_nScreenWidth = currentMode.m_Width;
	m_nScreenHeight = currentMode.m_Height;
	m_nScreenDepth = currentMode.m_BitDepth;

	DebugCPrint(1,"CUserProfile::SetDisplay() : %d x %d %d",m_nScreenWidth,m_nScreenHeight,m_nScreenDepth);

}

// Clear friends list of all entries.
void CUserProfile::ClearFriendsList()
{
	// Iterate through friends list and free the memory.
	for( TFriendList::iterator iter = m_lstFriends.begin( ); iter != m_lstFriends.end( ); iter++ )
	{
		wchar_t const* pszFriendNickName = *iter;
		delete[] pszFriendNickName;
	}
	ltstd::free_vector( m_lstFriends );
}


// Adds a friend to the list in alphabetical sorted case insensitive manner.  
// Nicknames are unique and only appear once in the list.  Returns index into list.
uint32 CUserProfile::AddFriend( wchar_t const* pwszNewFriendNickName )
{
	// Ignore empty names.
	if( LTStrEmpty( pwszNewFriendNickName ))
	{
		LTERROR( "Invalid friend nickname specified." );
		return eFriendsInfo_Invalid;
	}

	// Don't allow more than fixed amount of friends.
	if( m_lstFriends.size( ) == eFriendsInfo_MaxCount )
	{
		LTERROR( "Friend list capacity exceeded." );
		return eFriendsInfo_Invalid;
	}

	// Search for instance of the nickname in the current list.
	std::pair<TFriendList::iterator, TFriendList::iterator> pair = std::equal_range( m_lstFriends.begin( ), m_lstFriends.end( ), pwszNewFriendNickName, FriendsListSort );
	// Already in the list.
	if( pair.first != pair.second )
	{
		// Return index to the existing entry.
		TFriendList::difference_type iDiff = std::distance( m_lstFriends.begin( ), pair.first );
		return ( uint32 )iDiff;
	}

	// Add the nickname to the list.  second already points to sorted insertion point.
	wchar_t* pwszNewFriendNickNameCopy = LTStrDup( pwszNewFriendNickName );
	TFriendList::iterator iter = m_lstFriends.insert( pair.second, pwszNewFriendNickNameCopy );

	// Return index to new entry.
	TFriendList::difference_type iDiff = std::distance( m_lstFriends.begin( ), iter );
	return ( uint32 )iDiff;
}

// Remove a friend nickname from the friends list.
void CUserProfile::RemoveFriend( uint32 nFriendIndex )
{
	// Check the boundaries.
	if( nFriendIndex >= m_lstFriends.size( ))
	{
		LTERROR( "Invalid index to friend." );
		return;
	}

	// Find the iterator to this index.
	TFriendList::difference_type iterIndex = nFriendIndex;
	TFriendList::iterator iter = m_lstFriends.begin( ) + iterIndex;

	// Delete the string.
	wchar_t const* pwszFriendNickName = *iter;
	delete[] pwszFriendNickName;

	// Delete the list entry.
	m_lstFriends.erase( iter );
}

// Clear FavoriteServer list of all entries.
void CUserProfile::ClearFavoriteServerList()
{
	// Iterate through friends list and free the memory.
	for( TFavoriteServerList::iterator iter = m_lstFavoriteServers.begin( ); iter != m_lstFavoriteServers.end( ); iter++ )
	{
		FavoriteServer* pFavoriteServer = *iter;
		delete pFavoriteServer;
	}
	ltstd::free_vector( m_lstFavoriteServers );
}

// Get the index to the favorite server by IPandPort.  Returns eFavoriteServerInfo_Invalid if not found.
uint32 CUserProfile::GetFavoriteServerIndex( char const* pszServerIPandPort ) const
{
	// Search for instance of the FavoriteServer in the current list.
	FavoriteServer favoriteServer( pszServerIPandPort, NULL, false );

	std::pair<TFavoriteServerList::const_iterator, TFavoriteServerList::const_iterator> pair = std::equal_range( m_lstFavoriteServers.begin( ), m_lstFavoriteServers.end( ), &favoriteServer, FavoriteServerSort );
	if( pair.first != pair.second )
	{
		// Return index to existing entry.
		TFavoriteServerList::const_iterator citer = pair.first;
		TFavoriteServerList::difference_type iDiff = std::distance( m_lstFavoriteServers.begin( ), citer );
		return ( uint32 )iDiff;
	}

	// Not found.
	return eFavoriteServerInfo_Invalid;
}


// Adds a FavoriteServer to the list in sorted by IP order.
// FavoriteServers are unique and only appear once in the list.  
// Returns index into list or eFavoriteServerInfo_Invalid on error.
uint32 CUserProfile::AddFavoriteServer( char const* pszServerIPandPort, wchar_t const* pwszServerSessionName, bool bLAN, bool& bWasAdded )
{
	// Default the out parameter.
	bWasAdded = false;

	// Ignore empty names.
	if( LTStrEmpty( pszServerIPandPort ))
	{
		LTERROR( "Invalid favorite server specified." );
		return eFavoriteServerInfo_Invalid;
	}

	// Don't allow more than fixed amount of favorites.
	if( m_lstFavoriteServers.size( ) == eFavoriteServerInfo_MaxCount )
	{
		LTERROR( "FavoriteServer list capacity exceeded." );
		return eFavoriteServerInfo_Invalid;
	}

	// Search for instance of the FavoriteServer in the current list.
	FavoriteServer* pNewFavoriteServer = new FavoriteServer( pszServerIPandPort, pwszServerSessionName, bLAN );
	std::pair<TFavoriteServerList::iterator, TFavoriteServerList::iterator> pair = std::equal_range( m_lstFavoriteServers.begin( ), m_lstFavoriteServers.end( ), pNewFavoriteServer, FavoriteServerSort );
	if( pair.first != pair.second )
	{
		// Don't need a new instance.
		delete pNewFavoriteServer;

		// Return index to existing entry.
		TFavoriteServerList::difference_type iDiff = std::distance( m_lstFavoriteServers.begin( ), pair.first );
		return ( uint32 )iDiff;
	}

	// Set the outparameter that it was added.
	bWasAdded = true;

	// Add the favorite server to the list.  second already points to sorted insertion point.
	TFavoriteServerList::iterator iter = m_lstFavoriteServers.insert( pair.second, pNewFavoriteServer );

	// Return the index to the new favorite.
	TFavoriteServerList::difference_type iDiff = std::distance( m_lstFavoriteServers.begin( ), iter );
	return ( uint32 )iDiff;
}

// Remove a favorite server from the list.
void CUserProfile::RemoveFavoriteServer( uint32 nFavoriteServerIndex )
{
	// Check the boundaries.
	if( nFavoriteServerIndex >= GetNumFavoriteServers( ))
	{
		LTERROR( "Invalid index to favorite server." );
		return;
	}

	// Find the iterator to this index.
	TFavoriteServerList::difference_type iterIndex = nFavoriteServerIndex;
	TFavoriteServerList::iterator iter = m_lstFavoriteServers.begin( ) + iterIndex;

	// Delete the string.
	FavoriteServer* pFavoriteServer = *iter;
	delete pFavoriteServer;

	// Delete the list entry.
	m_lstFavoriteServers.erase( iter );
}

bool CProfileMgr::Init()
{
	wsWheelUp = LoadString("IDS_WHEEL_UP");
	wsWheelDown = LoadString("IDS_WHEEL_DOWN");

	g_pProfileMgr = this;

	InitCommands();

	char szProfileName[128] = "";

	//get the profile name
	LTProfileUtils::ReadString( "Game", "ProfileName", "", szProfileName, LTARRAYSIZE(szProfileName), g_pVersionMgr->GetGameSystemIniFile());

	//are we supposed to set a particular performance level?
	char *pszPerf = GetConsoleTempString("SetPerformanceLevel","");
	//if we have a profile name try to init it
	if (!LTStrEmpty(szProfileName))
	{
		if (m_profile.Init(szProfileName, L"", false, false))
		{
			//if we've been told to restore, do so...
			if (GetConsoleInt("RestoreDefaults",0))
			{
				m_profile.RestoreDefaults(PROFILE_ALL);
				WriteConsoleInt("RestoreDefaults",0);
			}
			
		
//			m_profile.SetPerformanceCfg(pszPerf);
			WriteConsoleString("SetPerformanceLevel","");


			//we also want to make sure to save the display configuration so that parameters passed
			//through the command line will be saved for the next run
			//SaveDisplaySettings();
			SaveSettings();
			return true;
		}
	}

	//we either don't have a profile or it couldn't be initted, create one
	NewProfile(szDefProfileName,LoadString("IDS_PLAYER"), true);

	if (!m_profile.IsInitted()) 
		return false;

//	m_profile.SetPerformanceCfg(pszPerf);
	WriteConsoleString("SetPerformanceLevel","");


	//we also want to make sure to save the display configuration so that parameters passed
	//through the command line will be saved for the next run
	//SaveDisplaySettings();
	SaveSettings();

	return true;
}

void CProfileMgr::Term()
{
	if (m_profile.IsInitted())
	{
		m_profile.SetBindings();

		m_profile.Save();
		m_profile.Term();
	}

	g_pProfileMgr = NULL;
}

void CProfileMgr::GetProfileList(ProfileArray& profileList)
{
	LTFINDFILEINFO file;
	LTFINDFILEHANDLE hFile;

	std::string directory = GetAbsoluteProfileFile( g_pLTClient, "*" );

	// find first file
	if(LTFileOperations::FindFirst(directory.c_str(), hFile, &file))
	{
		do
		{
			char *pName = strtok(file.name,".");
			const char* pszDatabase = GetRelativeProfileFile( g_pLTClient, pName);
			ILTInStream* pInFile = g_pLTClient->FileMgr()->OpenUserFileForReading(pszDatabase);

			if(pInFile)
			{
				HDATABASE hDB = g_pLTDatabase->OpenNewDatabase(pszDatabase, *pInFile);
				if (hDB)
				{
					CUserProfileFileData fd;
					fd.m_sFileName = pName;

					HRECORD hRec = g_pLTDatabase->GetRecord(hDB,g_prf_VersionCat,g_prf_VersionCat);
					if (hRec)
					{
						fd.m_sFriendlyName = g_pLTDatabase->GetWString( g_pLTDatabase->GetAttribute( hRec, g_prf_FriendlyName ), 0,  MPA2W(pName).c_str() );
					}
					else
					{
						fd.m_sFriendlyName = MPA2W(pName).c_str();
					}

					profileList.push_back(fd);

					g_pLTDatabase->ReleaseDatabase(hDB);
				}
				LTSafeRelease(pInFile);
			}

		}
		while(LTFileOperations::FindNext(hFile, &file));
		LTFileOperations::FindClose(hFile);
	}
}

void CProfileMgr::NewProfile(const char* profileName, const wchar_t* friendlyName, bool bLoadDisplaySettings)
{
	if (LTStrEmpty(profileName))
		return;
	if (m_profile.IsInitted())
	{
		m_profile.Save();
		m_profile.Term();
	}

	// Make sure we're disconnected from server.
	if(g_pLTClient->IsConnected())
	{
		g_pInterfaceMgr->SetIntentionalDisconnect( true );
		g_pClientConnectionMgr->ForceDisconnect();
	}
	
	if (!m_profile.Init(profileName, friendlyName, true, bLoadDisplaySettings)) return;

	m_profile.Save();
	
	LTProfileUtils::WriteString( "Game", "ProfileName", m_profile.m_sFileName.c_str(), g_pVersionMgr->GetGameSystemIniFile());
}

void CProfileMgr::DeleteProfile(const char* profileName)
{
	bool bNeedNew = false;
	if (m_profile.IsInitted() && m_profile.m_sFileName.compare(profileName) == 0)
	{
		m_profile.Term();
		bNeedNew = true;
	}
		
	std::string fn = GetAbsoluteProfileFile( g_pLTClient, profileName);
	remove(fn.c_str());

	g_pClientSaveLoadMgr->DeleteSpecificSaveDir( profileName );

	if (bNeedNew)
	{
		ProfileArray	profileList;
		GetProfileList(profileList);

		ProfileArray::iterator iter = profileList.begin();
		if (iter == profileList.end())
		{
			//empty list
			NewProfile(szDefProfileName,LoadString("IDS_PLAYER"), false);
		}
		else
			NewProfile(iter->m_sFileName.c_str(),LoadString("IDS_PLAYER"), true);

	}

}
void CProfileMgr::RenameProfile(const char* profileName, const wchar_t* newName)
{
	if (LTStrEmpty(newName))
		return;
	bool bIsCurrent = (m_profile.IsInitted() && m_profile.m_sFileName.compare(profileName) == 0);

	if (bIsCurrent && m_profile.IsInitted())
	{
		m_profile.m_sFriendlyName = newName;
		m_profile.Save();
		return;
	}

	CUserProfile profile;
	profile.Init(profileName,newName,false,false);

	profile.m_sFriendlyName = newName;
	profile.Save();

		
}

const wchar_t* CProfileMgr::GenerateFriendlyName()
{
	static wchar_t szName[16] = L"";
	
	ProfileArray profileList;
	GetProfileList(profileList);

	LTStrCpy(szName,LoadString("IDS_PLAYER"),LTARRAYSIZE(szName));
	uint32 nNum = 0;
	while (IsProfileInList(profileList,szName))
	{
		LTSNPrintF(szName,LTARRAYSIZE(szName),L"%s%d",LoadString("IDS_PLAYER"),nNum);
		nNum++;
	}

	return szName;
	
}

bool CProfileMgr::IsProfileInList(ProfileArray& profileList, const wchar_t* szName)
{
	// Profiles to the list
	for (ProfileArray::iterator iter = profileList.begin(); iter != profileList.end(); ++iter)
	{
		if (LTStrIEquals(iter->m_sFriendlyName.c_str(), szName ))
		{
			return true;
		}
	}

	return false;
}

bool CUserProfile::RestoreDefaults(uint8 nFlags)
{
	ILTInStream	*pDBStream = g_pLTBase->FileMgr()->OpenFile( DEFAULT_PROFILE );
	if( !pDBStream )
	{
		g_pLTBase->CPrint( "ERROR CGameDatabaseMgr couldn't open file: %s!", DEFAULT_PROFILE );
		return false;
	}

	HDATABASE hDB = g_pLTDatabase->OpenNewDatabase(DEFAULT_PROFILE, *pDBStream);
	if (!hDB) return false; //assert?

	if (nFlags & PROFILE_CONTROLS)
		LoadControls(hDB);
	if (nFlags & PROFILE_MULTI)
		LoadMultiplayer(hDB,true);
	if (nFlags & PROFILE_GAME)
		LoadGameOptions(hDB);
	if (nFlags & PROFILE_SOUND)
		LoadSound(hDB,true);
	if (nFlags & PROFILE_WEAPONS)
		LoadWeaponPriorities(hDB,true);

	g_pLTDatabase->ReleaseDatabase(hDB);
	m_bInitted =  true;

	return true;

}

//initialize command binding data
void CProfileMgr::InitCommands()
{
	g_CommandArray.reserve(50);

#ifdef PROJECT_DARK

	g_CommandArray.push_back( CommandData("IDS_CONTROL_FORWARD",		COMMAND_ID_FORWARD,			"Forward",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_BACKWARD",		COMMAND_ID_REVERSE,			"Backward",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STRAFELEFT",		COMMAND_ID_STRAFE_LEFT,		"StrafeLeft",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STRAFERIGHT",	COMMAND_ID_STRAFE_RIGHT,	"StrafeRight",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_RUN",			COMMAND_ID_RUN,				"Run",				COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LEANLEFT",		COMMAND_ID_LEAN_LEFT,		"LeanLeft",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LEANRIGHT",		COMMAND_ID_LEAN_RIGHT,		"LeanRight",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_FORWARD_AXIS,	"ForwardAxis",		COM_MOVE,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_STRAFE_AXIS,		"StrafeAxis",		COM_MOVE,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STAND",			COMMAND_ID_STAND,			"Stand",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_KNEEL",			COMMAND_ID_KNEEL,			"Kneel",			COM_MOVE,	false ) );

	g_CommandArray.push_back( CommandData("IDS_CONTROL_FIRE",			COMMAND_ID_FIRING,			"Fire",				COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_ACTIVATE",		COMMAND_ID_ACTIVATE,		"Activate",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_FLASHLIGHT",		COMMAND_ID_FLASHLIGHT,		"FlashLight",		COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_FOCUS",			COMMAND_ID_FOCUS,			"Focus",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TOGGLEMELEE",	COMMAND_ID_TOGGLEMELEE,		"Toggle Melee",		COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_AMMOCHECK",		COMMAND_ID_AMMOCHECK,		"Ammo Check",		COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STUNGUN",		COMMAND_ID_STUNGUN,			"Taser",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_MENU",			COMMAND_ID_MENU,			"Start Menu",		COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_CELLPHONE",		COMMAND_ID_CELLPHONE,		"Cell Phone",		COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_BLOCK",			COMMAND_ID_BLOCK,			"Block",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TOOLS",			COMMAND_ID_TOOLS,			"Tools",			COM_INV,	false ) );

	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_PITCH_ACCEL,		"PitchAccel",		COM_VIEW,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_YAW_ACCEL,		"YawAccel",			COM_VIEW,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_QUICKTURN",		COMMAND_ID_ACCEL_TURN,		"AccelTurn",		COM_VIEW,	false ) );

#else

	g_CommandArray.push_back( CommandData("IDS_CONTROL_FORWARD",		COMMAND_ID_FORWARD,			"FORWARD",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_BACKWARD",		COMMAND_ID_REVERSE,			"BACKWARD",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STRAFELEFT",		COMMAND_ID_STRAFE_LEFT,		"STRAFELEFT",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STRAFERIGHT",	COMMAND_ID_STRAFE_RIGHT,	"STRAFERIGHT",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TURNLEFT",		COMMAND_ID_YAW_NEG,			"TURNLEFT",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TURNRIGHT",		COMMAND_ID_YAW_POS,			"TURNRIGHT",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_STRAFE",			COMMAND_ID_STRAFE,			"STRAFE",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_RUN",			COMMAND_ID_RUN,				"RUN",				COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_JUMP",			COMMAND_ID_JUMP,			"JUMP",				COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_DUCK",			COMMAND_ID_DUCK,			"DUCK",				COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LEANLEFT",		COMMAND_ID_LEAN_LEFT,		"LEANLEFT",			COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LEANRIGHT",		COMMAND_ID_LEAN_RIGHT,		"LEANRIGHT",		COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_RUNLOCKTOGGLE",	COMMAND_ID_RUNLOCK,			"RUNLOCKTOGGLE",	COM_MOVE,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_FORWARD_AXIS,	"ForwardAxis",		COM_MOVE,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_STRAFE_AXIS,		"StrafeAxis",		COM_MOVE,	true ) );

	g_CommandArray.push_back( CommandData("IDS_CONTROL_FIRE",			COMMAND_ID_FIRING,			"Fire",				COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_ALT_FIRING",		COMMAND_ID_ALT_FIRING,		"Alt-Fire",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_THROWGRENADE",	COMMAND_ID_THROW_GRENADE,	"ThrowGrenade",		COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_ACTIVATE",		COMMAND_ID_ACTIVATE,		"Activate",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_RELOAD",			COMMAND_ID_RELOAD,			"Reload",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_ZOOM_IN",		COMMAND_ID_ZOOM_IN,			"ZoomIn",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_SLOWMO",			COMMAND_ID_SLOWMO,			"SlowMo",			COM_INV,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_FLASHLIGHT",		COMMAND_ID_FLASHLIGHT,		"FlashLight",		COM_INV,	false ) );

	g_CommandArray.push_back( CommandData("IDS_CONTROL_STATUS",			COMMAND_ID_STATUS,			"Status",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_MISSION",		COMMAND_ID_MISSION,			"Mission",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_SAY",			COMMAND_ID_MESSAGE,			"Say",				COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TEAM_SAY",		COMMAND_ID_TEAM_MESSAGE,	"TeamSay",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_RADIO",			COMMAND_ID_RADIO,			"Radio",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LOOKUP",			COMMAND_ID_PITCH_NEG,		"LookUp",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LOOKDOWN",		COMMAND_ID_PITCH_POS,		"LookDown",			COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_CENTERVIEW",		COMMAND_ID_CENTERVIEW,		"CenterView",		COM_MISC,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_PITCH,			"Pitch",			COM_MISC,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_YAW,				"Yaw",				COM_MISC,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_PITCH_ACCEL,		"PitchAccel",		COM_MISC,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_UNASSIGNED",		COMMAND_ID_YAW_ACCEL,		"YawAccel",			COM_MISC,	true ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_TOGGLE_NAVMARKER",	COMMAND_ID_TOGGLE_NAVMARKER, "ToggleNavMarker",	COM_MISC,	false ) );


#ifndef _FINAL
	g_CommandArray.push_back( CommandData("IDS_DEBUGMSG",				COMMAND_ID_DEBUGMSG,		"DebugMsg",			COM_MISC,	false ) );
#endif
	

#endif

	const char* pszWeaponStringID[] = {
		"IDS_CONTROL_WEAPON1",	"IDS_CONTROL_WEAPON2", "IDS_CONTROL_WEAPON3", "IDS_CONTROL_WEAPON4",
		"IDS_CONTROL_WEAPON5",	"IDS_CONTROL_WEAPON6", "IDS_CONTROL_WEAPON7", "IDS_CONTROL_WEAPON8",
		"IDS_CONTROL_WEAPON9",	"IDS_CONTROL_WEAPON10" };

	int nCommand = COMMAND_ID_WEAPON_BASE;
	char szAction[64] = "";
	for (uint8 i = 0; i < g_pWeaponDB->GetWeaponCapacity() && nCommand <= COMMAND_ID_WEAPON_MAX; ++i, ++nCommand)
	{
		// check if we didn't exceed the allocated weapon array
		if( i >= LTARRAYSIZE(pszWeaponStringID) )
		{
			LTERROR( "Exceeded Weapon StringID array!" );
			break;
		}

		LTSNPrintF(szAction,LTARRAYSIZE(szAction),"WEAPON_%d",i);
		g_CommandArray.push_back( CommandData(pszWeaponStringID[i], nCommand, szAction, COM_WEAP,	false  ) );
	}

#if defined(PROJECT_FEAR)
	g_CommandArray.push_back( CommandData("IDS_CONTROL_NEXTWEAPON",		COMMAND_ID_NEXT_WEAPON,		"NextWeapon",		COM_WEAP,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_PREVIOUSWEAPON",	COMMAND_ID_PREV_WEAPON,		"PreviousWeapon",	COM_WEAP,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_LASTWEAPON",		COMMAND_ID_LASTWEAPON,		"LastWeapon",		COM_WEAP,	false ) );
	g_CommandArray.push_back( CommandData("IDS_CONTROL_DROPWEAPON",		COMMAND_ID_DROPWEAPON,		"DropWeapon",			COM_WEAP,	false  ) );
#endif

	const char* pszGrenadeStringID[] = {
		"IDS_CONTROL_WEAPON5",	"IDS_CONTROL_WEAPON6", "IDS_CONTROL_WEAPON7", "IDS_CONTROL_WEAPON8",
		"IDS_CONTROL_WEAPON9",	"IDS_CONTROL_WEAPON10" };

	nCommand = COMMAND_ID_GRENADE_BASE;
	for (uint8 i = 0; nCommand <= COMMAND_ID_GRENADE_MAX && i < g_pWeaponDB->GetNumPlayerGrenades(); ++i, ++nCommand)
	{
		// check if we didn't exceed the allocated grenade array
		if( i >= LTARRAYSIZE(pszGrenadeStringID) )
		{
			LTERROR( "Exceeded Grenade StringID array!" );
			break;
		}

		const char* szString = pszGrenadeStringID[i];
		LTSNPrintF(szAction,LTARRAYSIZE(szAction),"GRENADE_%d",i);
		HWEAPON hGrenade = g_pWeaponDB->GetPlayerGrenade(i);
		if (hGrenade)
		{
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hGrenade,!USE_AI_DATA);
			szString = g_pWeaponDB->GetString(hWpnData,WDB_ALL_nDescriptionId);
			g_CommandArray.push_back( CommandData(szString, nCommand, szAction, COM_WEAP,	false  ) );
		}
		
	}

#if defined(PROJECT_FEAR)
	g_CommandArray.push_back( CommandData("IDS_CONTROL_NEXT_GRENADE",	COMMAND_ID_NEXT_GRENADE,	"NextGrenade",		COM_WEAP,	false ) );

	g_CommandArray.push_back( CommandData("IDS_CONTROL_MEDKIT",			COMMAND_ID_MEDKIT,			"MedKit",			COM_WEAP,	false ) );

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(g_pWeaponDB->GetUnarmedRecord(),!USE_AI_DATA);
	g_CommandArray.push_back( CommandData("IDS_CONTROL_HOLSTERWEAPON", COMMAND_ID_HOLSTER, "Holster", COM_WEAP,	false  ) );
#endif


};


bool IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	LTASSERT(pRenderer1, "Invalid parameter");
	LTASSERT(pRenderer2, "Invalid parameter");
	if (!pRenderer1 || !pRenderer2) return false;

	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
        return false;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
        return false;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
        return false;
	}

    return true;
}
