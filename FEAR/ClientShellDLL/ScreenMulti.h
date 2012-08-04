// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.h
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_MULTI_H_
#define _SCREEN_MULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "ServerBrowserCtrl.h"
#include "sys/win/WinSync.h"

class IGameSpyBrowser;
class LabeledEditCtrl;

class MOTDData
{
public:
	MOTDData() {}
	~MOTDData() {};

	std::wstring m_wsDate;
	std::wstring m_wsImageURL;
	std::wstring m_wsLinkURL;
	std::wstring m_wsText;

	bool Read(const char* pszFile);
	bool Write(const char* pszFile) const;

	MOTDData& operator = ( const MOTDData& b )
	{
		m_wsDate = b.m_wsDate;
		m_wsImageURL = b.m_wsImageURL;
		m_wsLinkURL = b.m_wsLinkURL;
		m_wsText = b.m_wsText;
		return *this;
	}

};


class CScreenMulti : public CBaseScreen
{
public:
	CScreenMulti();
	virtual ~CScreenMulti();

	virtual void	Term();

	// Build the screen
    bool   Build();

	bool	Render();

    void    OnFocus(bool bFocus);

	bool	HandleKeyDown(int key, int rep);
	bool	HandleKeyUp(int key);

	virtual void Escape();

	// Deletes serverlist data, which can be very large.
	void	TermServerList( TServerEntryMap& serverMap, CLTGUIListCtrlEx* pServerListCtrl );

	void MOTDIniCallback(const char* pBuffer,const uint32 nBufferLen);
	void MOTDImageCallback(const char* pBuffer,const uint32 nBufferLen);

protected:

    uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	bool	OnLButtonDblClick(int x, int y);

	uint32	HandleCallback(uint32 dwParam1, uint32 dwParam2);
	void	ChangeCDKey();
	void	RequestValidate();
	bool	RequestServerDetails( char const* pszPublicAddress, bool bDirectConnect, bool bFullDetails );
	bool	JoinServer( ServerEntry& serverEntry );

	void	FindServers();

	void	Update();

	void	UpdateServerListControls();

	// Dump the server list into the display
	void DisplayCurServerList( ServerBrowserCtrl::EColumn eColumn, bool bToggleDirection );

	// Read the server details
	void DisplayDetails();
	void ReadDMDetails(std::string& sOptions,CLTMsgRef_Read& cRead);

	// Completes the joining of server already setup in JoinServer.
	void JoinCurGame( ServerEntry& serverEntry );

	// Called by gamespy.
	static void ServerInfoCallback( IGameSpyBrowser& gameSpyBrowser, 
		IGameSpyBrowser::HGAMESERVER hGameServer, void* pUserData, 
		IGameSpyBrowser::EServerInfoCallbackReason eReason,
		uint16 nPing );

	// Sets the controls showing detail information.
	bool SetDetailInfo( ServerEntry const& serverEntry );

	bool AddServerBrowserColumns( CLTGUIHeaderCtrl* pHeaderCtrl );

	// State functions.

		// Current state of control.
		enum EState 
		{
			eState_Inactive,
			eState_Startup,
			eState_UpdateDir,
			eState_Waiting,
			eState_Transition,
			eState_Error
		};
		EState m_eCurState;

		// Called when entering a state
		bool PreState(EState eNewState);
		// Called when leaving a state
		// Returns false if that is an invalid state transition
		bool PostState(EState eNewState);
		// Change states
		bool ChangeState(EState eNewState);

		// Pre-state handling for each state
		bool PreState_Inactive();
		bool PreState_Startup();
		bool PreState_UpdateDir();
		bool PreState_Waiting();

		// Post-state handling for each state
		bool PostState_UpdateDir(EState eNewState);
		bool PostState_Waiting(EState eNewState);

		// Update handling for each state
		void Update_State_Startup();
		void Update_State_UpdateDir();

		static void PortValueChangingCB( std::wstring& wsValue, void* pUserData );
		
		// Read the callback responses from the response queue
		void ReadCallbackResponses();

		// Read the callback responses from the MOTD system
		void ReadMOTDCallbackResponses();

		// Populate the friends list control with the friends in the profile.
		void PopulateFriendsListControl( );

		// Filter the server list to those with players using the given nickname.
		void FilterServersToFriendNickName( wchar_t const* pwszFriendNickName );

		// Accessors to the friend filter.
		void SetFriendFilter( char const* pszFriendNickName )
		{
			delete[] m_pszFriendFilter;
			m_pszFriendFilter = LTStrDup( pszFriendNickName );
		}
		char const* GetFriendFilter( ) const { return m_pszFriendFilter; }

		// Refresh the details info on all the servers we know about.
		void RefreshDetailsOnServers( );

		// Handles making the request when doing the updatedir state.
		void UpdateDir_Request( );

		// Adds the serverentry to the favorites.
		bool AddServerEntryToFavorites( ServerEntry& serverEntry );

		// Adds the server IP to the favorites.
		bool AddServerIPToFavorites( char const* pszServerIPandPort );

		// Remove server from favorites.
		bool RemoveServerIPFromFavorites( char const* pszServerIPandPort );

		// Populates the favorite server information from the profile information.
		bool PopulateFavoriteServers( );

		// Saves the current state of the server info to the profile for the favorites.
		void SaveFavoritesToProfile( );

		// Create a column control for a ServerEntry.
		CLTGUIColumnCtrlEx* CreateServerEntryColumnControl( ServerEntry& serverEntry );

		// Add the server to the servermap and list control.  Specify if this is a lan search or not.
		void UpdateServerMapAndList( ServerEntry const& serverEntry, TServerEntryMap& serverMap, 
			CLTGUIListCtrlEx& serverListControl, CLTGUIHeaderCtrl& headerControl,
			bool bIsLan, bool bApplyFilters, bool bAllowAdds, bool& bWasUpdated );

		// Handle the ui cmd CMD_SOURCE.
		void HandleCmd_Source( );

		void CheckForMOTD();
		void SetMOTDDownloadUI(bool	bDownload);
		void UpdateMOTD();
		bool UpdateMOTDIni(); //returns true if an image should be downloaded
		void UpdateMOTDImage();


private:

	CLTGUITextCtrl*	m_pFindCtrl;
	CLTGUIListCtrlEx*	m_pInternetServerListCtrl;
	CLTGUIListCtrlEx*	m_pLANServerListCtrl;
	CLTGUIListCtrlEx*	m_pFavoriteServerListCtrl;
	CLTGUIListCtrlEx*	m_pServerListCtrl;
	CLTGUITextCtrl*	m_pOptions;
	CLTGUITextCtrl* m_pMission;
	CLTGUITextCtrl* m_pStatus;
	CLTGUITextCtrl* m_pMOTD;
	CLTGUITextCtrl* m_pMOTDDownload;
	CLTGUIWindow*	m_pMOTDDlg;
	CLTGUIFrame*	m_pMOTDImage;
	CLTGUITextCtrl* m_pMOTDLink;
	HDYNAMICTEXTURE m_hMOTDImage;
	LT_POLYGT4		m_MOTDQuad;
	CLTGUITextCtrl*	m_pMOTDText;

	CLTGUITextCtrl* m_pServerCount;
	CLTGUICycleCtrl* m_pSourceCtrl;
	CLTGUIScrollBar* m_pLANScrollBar;
	CLTGUIScrollBar* m_pInternetScrollBar;
	CLTGUIScrollBar* m_pFavoriteServerScrollBar;
	CLTGUIHeaderCtrl* m_pLANHeaderCtrl;
	CLTGUIHeaderCtrl* m_pInternetHeaderCtrl;
	CLTGUIHeaderCtrl* m_pFavoriteServerHeaderCtrl;

	CLTGUITabCtrl*	m_pTabCtrl;

	CLTGUIHeaderCtrl*	m_pPlayerHeaderCtrl;
	CLTGUIScrollBar*	m_pPlayerScrollBar;
	CLTGUIListCtrlEx*	m_pPlayerListCtrl;

	CLTGUIHeaderCtrl*	m_pRulesHeaderCtrl;
	CLTGUIScrollBar*	m_pRulesScrollBar;
	CLTGUIListCtrlEx*	m_pRulesListCtrl;

	CLTGUIHeaderCtrl*	m_pCustomizersHeaderCtrl;
	CLTGUIScrollBar*	m_pCustomizersScrollBar;
	CLTGUIListCtrlEx*	m_pCustomizersListCtrl;

	CLTGUIScrollBar*	m_pFiltersScrollBar;
	CLTGUIListCtrlEx*	m_pFiltersListCtrl;

	CLTGUIWindow*		m_pFriendsTab;
	CLTGUIListCtrlEx*	m_pFriendsListCtrl;
	CLTGUITextCtrl*		m_pFriendsAddButton;
	CLTGUITextCtrl*		m_pFriendsRemoveButton;
	CLTGUITextCtrl*		m_pFriendsDeselectButton;
	CLTGUITextCtrl*		m_pFriendsRefreshButton;

	CLTGUIWindow*		m_pFavoriteServersTab;
	CLTGUITextCtrl*		m_pFavoriteServersAddButton;
	CLTGUITextCtrl*		m_pFavoriteServersAddIPButton;
	CLTGUITextCtrl*		m_pFavoriteServersRemoveButton;
	CLTGUITextCtrl*		m_pFavoriteServersRemoveAllButton;

	CLTGUIFillFrame* m_pFrameLowerLeft;
	CLTGUIFillFrame* m_pFrameLowerRight;

	uint8	m_nVersionFilter;
	uint8	m_nPlayersFilter;
	uint8	m_nPingFilter;
	uint8	m_nGameTypeFilter;
	uint8	m_nCustomizedFilter;
	uint8	m_nRequiresDownloadFilter;
	uint8	m_nPunkbusterFilter;
	char*	m_pszFriendFilter;

	bool	m_bAscending;

	// This index is used by the UI control.  Because it gets
	// updated before we can react, we have another enum
	// that will actually be used to determine what we're searching for.
	// Only use m_eServerSearchSource for determine what the source is 
	// set to.
	uint8	m_nSourceIndex;
	ServerSearchSource m_eServerSearchSource;

	typedef std::vector<HRECORD> GameTypeList;
	GameTypeList m_lstGameTypes;

	ServerBrowserCtrl::EColumn m_eLastSort;

	// Password to send to server when joining.
	std::wstring	m_sPassword;

	// Port to query.
	LabeledEditCtrl*	m_pPort;
	std::wstring m_sPort;

	// Type manages list of servers reported.
	TServerEntryMap m_cInternetServerMap;
	TServerEntryMap m_cLANServerMap;
	TServerEntryMap m_cFavoriteServerMap;
	TServerEntryMap* m_pcServerMap;

	// The currently selected server.
	std::string m_sSelectedServerAddress;

	// UpdateDir states.
	enum EUpdateDirState
	{
		eUpdateDirState_Idle,
		eUpdateDirState_Request,
		eUpdateDirState_Processing,
		eUpdateDirState_Finished,

		eUpdateDirState_NumStates,
	};
	EUpdateDirState m_eUpdateDirState;

	CWinSync_CS m_csAddServer;

	// current LAN refresh count
	uint32  m_nLANRefreshCount;

	uint32	m_nTotalServerCount;

	IGameSpyBrowser::EBrowserStatus m_eLastBrowserStatus;

	typedef std::vector<ServerEntry> TServerEntryList;
	TServerEntryList m_aServerCallbackResponses;

	MOTDData m_MOTD;
	bool	 m_bCheckForMOTD;
	bool	 m_bDisplayMOTD;
	bool	m_bMOTDIniBuffer;
	bool	m_bMOTDImageBuffer;
	bool	m_bMOTDDownloadFailed;
	StopWatchTimer m_DownloadTimer;

};

#endif // _SCREEN_MULTI_H_
