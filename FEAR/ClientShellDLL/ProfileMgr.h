// ----------------------------------------------------------------------- //
//
// MODULE  : ProfileMgr.h
//
// PURPOSE : Manages player profiles
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef PROFILE_MGR_H
#define PROFILE_MGR_H

#pragma warning( disable : 4786 )
#include <string>			// For strings

#include "ClientUtilities.h"
#include "PerformanceMgr.h"
#include "NetDefs.h"
#include "DatabaseUtils.h"
#include "ProfileUtils.h"

class CProfileMgr;
extern CProfileMgr* g_pProfileMgr;

const int SOUND_BASE_VOL = 80;


const float	kMinGamma		= 0.5f;
const float kDefaultGamma   = 1.0f;
const float	kMaxGamma		= 6.0f;

//section flags
const uint8 PROFILE_CONTROLS	= 0x01;
const uint8 PROFILE_MULTI		= 0x02;
const uint8 PROFILE_GAME		= 0x04;
const uint8 PROFILE_CROSSHAIR	= 0x08;
const uint8 PROFILE_SOUND		= 0x10;
const uint8 PROFILE_WEAPONS		= 0x20;
const uint8 PROFILE_ALL			= 0xFF;


enum CommandType
{
	COM_MOVE,
	COM_INV,
	COM_VIEW,
	COM_MISC,
	COM_WEAP,
	kNumCommandTypes
};

enum ServerSearchSource
{
	eServerSearchSource_Internet,
	eServerSearchSource_LAN,
	eServerSearchSource_Favorites,
};

class CommandData
{
public:
	CommandData(const char*	szStringID, int nCommandID, const char* pActionString, int nCommandType, bool bAxis) :
		m_szStringID(szStringID),
		m_nCommandID(nCommandID),
		m_nCommandType(nCommandType),
		m_bAxis(bAxis)
	{
		LTStrCpy(m_szActionString,pActionString,LTARRAYSIZE(m_szActionString));
	}

	virtual ~CommandData() {};
	const char*	m_szStringID;
	int			m_nCommandID;
	char		m_szActionString[64];
	int			m_nCommandType;
	bool		m_bAxis;
};

const char*			GetCommandName(int nCommand);
const char*			GetCommandStringID(int nCommand);
const int			GetCommandID(const char* szAction);
const int			GetNumCommands();
const CommandData*	GetCommandData(int nIndex);
inline uint16 GetMinimumBandwidth() {return (g_BandwidthServer[0]/2);}
inline uint16 GetMaximumBandwidth() {return (g_BandwidthClient[eBandwidth_Custom-1].m_nBandwidthTargetClient);}
inline bool IsValidBandwidth(uint16 nBandwidth) 
{
	return (nBandwidth >= GetMinimumBandwidth() && nBandwidth <= GetMaximumBandwidth());
}

class CUserProfileFileData
{
public:
	CUserProfileFileData() {}
	virtual ~CUserProfileFileData() {}

	std::string		m_sFileName;
	std::wstring	m_sFriendlyName;
};
typedef std::vector<CUserProfileFileData, LTAllocator<CUserProfileFileData, LT_MEM_TYPE_GAMECODE> > ProfileArray;

// Contains data associated with saving the Favorite Servers for each profile.
// This object is intended to be managed within a vector of pointers to these objects.
class FavoriteServer
{
public:
	FavoriteServer( char const* pszServerIPandPort, wchar_t const* pwszServerSessionName, bool bLAN )
	{
		m_pszServerIPandPort = LTStrDup( pszServerIPandPort );
		m_pwszServerSessionName = LTStrDup( pwszServerSessionName );
		m_bLAN = bLAN;
	}
	~FavoriteServer( )
	{
		delete[] m_pszServerIPandPort;
		delete[] m_pwszServerSessionName;
	}

	char const* GetServerIPandPort( ) const { return m_pszServerIPandPort; }

	void SetServerSessionName( wchar_t const* pwszServerSessionName )
	{
		delete[] m_pwszServerSessionName;
		m_pwszServerSessionName = LTStrDup( pwszServerSessionName );
	}
	wchar_t const* GetServerSessionName( ) const { return m_pwszServerSessionName; }

	void SetLAN( bool bLAN ) { m_bLAN = bLAN; }
	bool GetLAN( ) const { return m_bLAN; }

private:
	char* m_pszServerIPandPort;
	wchar_t* m_pwszServerSessionName;
	bool m_bLAN;
private:
	// Don't bother supporting copying since this object doesn't need to be
	// allocated by many clients.
	PREVENT_OBJECT_COPYING( FavoriteServer );
};

class CUserProfile : public CGameDatabaseReader, public CGameDatabaseCreator
{
public:
	CUserProfile();
	~CUserProfile( );

	bool		Init(const char* profileName, const wchar_t* friendlyName, bool bCreate, bool bLoadDisplaySettings);
	bool		RestoreDefaults(uint8 nFlags);
	void		Term();

	//take data from profile and apply them to the game
	void		ApplyControls();
	void		ApplyBindings();
	void		ApplyMouse();
	void		ApplyKeyboard();
	void		ApplyJoystick();

	void		ApplyGameOptions();
	void		ApplyCrosshair();

	void		ApplySound();
	void		ImplementSoundVolume();

	void		ApplyDisplay();

	void		ApplyWeaponPriorities();
	void		SendWeaponPriorityMsg(); 

	//read data from the game and save them in the profile
	void		SetControls();
	void		SetBindings();
	void		SetMouse();
	void		SetKeyboard();
	void		SetJoystick();

	void		SetAxisBinding(uint32 command, WCHAR* DeviceName, WCHAR* ObjectName, float fScale);
	void		SetButtonBinding(uint32 command, WCHAR* DeviceName, WCHAR* ObjectName);

	void		SetGameOptions();
	void		SetCrosshair();

	void		SetUsePunkbuster( bool bUsePunkbuster ) { m_bUsePunkbuster = bUsePunkbuster; }
	bool		GetUsePunkbuster( ) const { return m_bUsePunkbuster; }

	void		SetSound();

	void		SetDisplay();

	// load/save profile to file
	void		Load(bool bLoadDefaults, bool bLoadDisplaySettings );
	void		Save();

	bool		IsInitted() {return m_bInitted;}

	const wchar_t* GetTriggerNameFromCommandID(int nCommand);

	uint8		GetWeaponPriority(HWEAPON hWpn) const;

	// Constants for friends information.
	enum FriendsInfo
	{
		// Maximum number of friends that can be stored in the friends list.
		eFriendsInfo_MaxCount = 64,
		// Used for return of AddFriend
		eFriendsInfo_Invalid = -1,
	};

	// Accessors for getting friend information.
	uint32 GetNumFriends( ) const { return m_lstFriends.size( ); }
	wchar_t const* GetFriendNickName( uint32 nFriendIndex ) const 
	{ 
		if( nFriendIndex >= m_lstFriends.size( ))
		{
			LTERROR( "Invalid friend index." );
			return NULL;
		}
		return m_lstFriends[nFriendIndex]; 
	}

	// Adds a friend to the list in alphabetical sorted case insensitive manner.  
	// Nicknames are unique and only appear once in the list. 
	// Returns index into list or eFriendsInfo_Invalid on error.
	uint32		AddFriend( wchar_t const* pszNewFriendNickName );

	// Remove a friend nickname from the friends list.
	void		RemoveFriend( uint32 nFriendIndex );

	// Constants for favorites information.
	enum FavoriteServerInfo
	{
		// Maximum number of favorites that can be stored in the favorites list.
		eFavoriteServerInfo_MaxCount = 64,
		// Used for return of AddFavoriteServer
		eFavoriteServerInfo_Invalid = -1,
	};

	// Accessors for getting favorite server information.
	uint32 GetNumFavoriteServers( ) const { return m_lstFavoriteServers.size( ); }
	FavoriteServer* GetFavoriteServer( uint32 nFavoriteServerIndex )
	{ 
		if( nFavoriteServerIndex >= GetNumFavoriteServers( ))
		{
			LTERROR( "Invalid FavoriteServer index." );
			return NULL;
		}
		return m_lstFavoriteServers[nFavoriteServerIndex]; 
	}
	// Get the index to the favorite server by IPandPort.
	uint32 GetFavoriteServerIndex( char const* pszServerIPandPort ) const;

	// Adds a FavoriteServer to the list in sorted by IP order.
	// FavoriteServers are unique and only appear once in the list.  
	// Returns index into list or eFavoriteServerInfo_Invalid on error.
	// Can check the bWasAdded out parameter to see if the server was added or already there.
	uint32		AddFavoriteServer( char const* pszServerIPandPort, wchar_t const* pwszServerSessionName, bool bLAN, bool& bWasAdded );

	// Remove a FavoriteServer from the FavoriteServer list.
	void		RemoveFavoriteServer( uint32 nFavoriteServerIndex );

public:

	//****************** controls
	//mouse
    bool		m_bInvertY;
	int			m_nInputRate;
	int			m_nSensitivity;

	//keyboard
	int			m_nNormalTurn;
	int	  		m_nFastTurn;
	int			m_nLookUp;

	//shared
//	int	  		m_nVehicleTurn;

	//****************** multiplayer
	std::wstring	m_sPlayerName;
	std::string		m_sPlayerPatch;
	LTGUID			m_PlayerGuid;
	uint8			m_nDMPlayerModel;
	uint8			m_nTeamPlayerModel;
	std::string		m_sServerOptionsFile;
	uint8			m_nBandwidthClient;
	uint16			m_nBandwidthClientCustom;
	uint16			m_nClientPort;
	bool		    m_bAllowContentDownload;
	bool			m_bAllowContentDownloadRedirect;
	uint32			m_nMaximumDownloadSize;
	bool		    m_bAllowBroadcast;
	bool		    m_bFilterNavMarkers;

	uint8			m_nVersionFilter;
	uint8			m_nPlayersFilter;
	uint8			m_nPingFilter;
	uint8			m_nGameTypeFilter;
	ServerSearchSource m_eServerSearchSource;
	uint8			m_nCustomizedFilter;
	uint8			m_nRequiresDownloadFilter;
	uint8			m_nPunkbusterFilter;

	//****************** game options
    bool	            m_bSubtitles;
	bool				m_bGore;
	uint8				m_nDifficulty;
	bool				m_bAlwaysRun;
	bool				m_bCrouchToggle;
	int					m_nHeadBob;
	int					m_nMsgDur;
	bool				m_bSPAutoWeaponSwitch;
	bool				m_bMPAutoWeaponSwitch;
	bool				m_bPersistentHUD;
	float				m_fHUDFadeSpeed;
	bool				m_bSlowMoFX;
	bool				m_bUseTextScaling;

	std::string			m_sFileName;
	std::wstring		m_sFriendlyName;


	//crosshair
	uint8				m_CrosshairR;
	uint8				m_CrosshairG;
	uint8				m_CrosshairB;
	uint8				m_CrosshairA;
	uint8				m_nCrosshairSize;

	//****************** sound
	float	m_fSoundVolume;
	float	m_fMusicVolume;
	float	m_fSpeechVolume;
	bool	m_bUseHWSound;
	bool	m_bUseEAX;
	bool	m_bUseEAX4;

	//****************** display
	bool	m_bHardwareCursor;
	bool	m_bVSync;
	bool	m_bRestartRenderBetweenMaps;
	uint32	m_nScreenWidth;
	uint32	m_nScreenHeight;
	uint32	m_nScreenDepth;
	float	m_fGamma;

	//****************** weapon priorities
	typedef std::vector<HWEAPON, LTAllocator<HWEAPON, LT_MEM_TYPE_GAMECODE> > WeaponArray;
	WeaponArray m_vecWeapons;


private:
	//load parts of profile to file (file must already have been parsed by m_buteMgr)
	void		LoadVersion(HDATABASE hDB);
	void		LoadControls(HDATABASE hDB);
	void		LoadMultiplayer(HDATABASE hDB,bool bLoadDefaults);
	void		LoadGameOptions(HDATABASE hDB);
	void		LoadSound(HDATABASE hDB,bool bApply); //optionally defer applying these settings
	void		LoadWeaponPriorities(HDATABASE hDB, bool bLoadDefaults);

	//save parts of profile to file
	void		SaveVersion(HDATABASECREATOR hDBC);
	void		SaveControls(HDATABASECREATOR hDBC);
	void		SaveMultiplayer(HDATABASECREATOR hDBC);
	void		SaveGameOptions(HDATABASECREATOR hDBC);
	void		SaveSound(HDATABASECREATOR hDBC);
	void		SaveWeaponPriorities(HDATABASECREATOR hDBC);

	// Clear friends list of all entries.
	void		ClearFriendsList( );

	// Clear FavoriteServer list of all entries.
	void		ClearFavoriteServerList( );

private:

	bool		m_bInitted;
	uint32		m_nSaveVersion;

	struct SCommandBindingInfo
	{
		std::wstring m_sDeviceName, m_sObjectName;
		float m_fDefaultValue, m_fOffset, m_fScale;
		float m_fDeadZoneMin, m_fDeadZoneMax, m_fDeadZoneValue;
		float m_fCommandMin, m_fCommandMax;
	};
	typedef std::vector<SCommandBindingInfo> TCommandBindingInfoList;
	typedef std::vector<TCommandBindingInfoList> TCommandBindingList;
  
	void		SetCommandBinding(uint32 uiCommand, SCommandBindingInfo binding );

	bool			m_bUsePunkbuster;

	TCommandBindingList m_aCommandDeviceBindings;

	typedef std::vector<wchar_t const*> TFriendList;
	TFriendList m_lstFriends;

	typedef std::vector<FavoriteServer*> TFavoriteServerList;
	TFavoriteServerList m_lstFavoriteServers;
};

class CProfileMgr
{
public:
	bool			Init();
	void			Term();

	void			GetProfileList(ProfileArray& profileList);

	//saves current profile and loads specified one (creating it if necessary)
	void			NewProfile(const char* profileName, const wchar_t* friendlyName, bool bLoadDisplaySettings);
	void			DeleteProfile(const char* profileName);
	void			RenameProfile(const char* profileName, const wchar_t* newName);
	CUserProfile*	GetCurrentProfile() {return &m_profile;}
	const char*		GetCurrentProfileFileName() {return m_profile.m_sFileName.c_str();}

	const wchar_t*	GenerateFriendlyName();

private:
	bool			IsProfileInList(ProfileArray& profileList, const wchar_t* szName);

	void			InitCommands();

	CUserProfile	m_profile;

};


bool IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2);

#endif	// PROFILE_MGR_H
