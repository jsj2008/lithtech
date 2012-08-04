// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.h
//
// PURPOSE :  loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000, 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOADINGSCREEN_H__
#define __LOADINGSCREEN_H__

#include "BaseScreen.h"
#include "ScreenMgr.h"
#include "HUDBar.h"
#include "iltloadingprogress.h"
#include "clientconnectionmgr.h"

class CLoadingScreen
{
// External functions
public:
	CLoadingScreen();
	~CLoadingScreen();

	// Note : Init/Term will be called automatically when needed, so
	// you shouldn't ever actually need to call them.

	// Initialize and load the screen resources
	// Returns false if not in STATE_NONE
	bool Init();
	// Terminate the screen and its resources
	// Returns false if not in STATE_INIT
	bool Term();

	// Show the loading screen
	// Set bNew to true, if this is a new load, false if it's resuming 
	// Returns LTFALES if the state isn't STATE_NONE or STATE_INIT
	bool Show(bool bNew);
	// Update the loading screen
	// Returns false if the state isn't STATE_SHOW
	bool Update();
	// Take the loading screen out of threaded mode
	// Returns false if the state isn't STATE_ACTIVE
	bool Pause();
	// Resume threaded mode
	// Returns false if the state isn't STATE_SHOW
	bool Resume();
	// Hide the loading screen
	// Returns false if the state isn't STATE_SHOW or STATE_ACTIVE
	bool Hide();

	// Returns whether or not the loading screen is currently being actively updated
	inline bool IsActive() const { return m_eCurState == STATE_ACTIVE; };
	// Returns whether or not the loading screen is visible
	inline bool IsVisible() const { return ((m_eCurState == STATE_ACTIVE) || (m_eCurState == STATE_SHOW)); };

	//is loading a new mission?
	bool	NeedsPostLoadScreen() const;

	void	UpdateMissionInfo();

	void	UpdateLayout();

	void	SetRenderScreen( CBaseScreen* pScreen ) { m_pRenderScreen = (m_eCurState == STATE_NONE ? pScreen : NULL); }

	// content download notification
	virtual void ClearContentDownloadInfo();
	virtual void ClientContentTransferStartingNotification(uint32 nTotalBytes,
															uint32 nTotalFiles);

	virtual void FileReceiveProgressNotification(const std::string& strFilename,
											     uint32				nFileBytesReceived,
											     uint32				nFileBytesTotal,
												 uint32				nTransferBytesTotal,
												 float				fTransferRate);

	virtual void FileReceiveCompletedNotification(const std::string& strFilename,
												  uint32			 nFileBytesTotal,
												  uint32			 nTransferBytesTotal,
												  float				 fTransferRate);

	virtual void ClientContentTransferCompletedNotification();

	virtual void LoadingProgressCallback(ELoadingProgressTask eTask, float fProgress, void* pUser);


// Internal functions
protected:

	static void OnClientLoggedInEvent( CLoadingScreen* pLoadingScreen, ClientConnectionMgr* pClientConnectionMgr, EventCaster::NotifyParams& notifyParams )
	{
		pLoadingScreen->UpdateServerInfo();
	}
	Delegate< CLoadingScreen, ClientConnectionMgr, OnClientLoggedInEvent > m_dlgClientLoggedIn;


	// Updates the session name when it changes.
	void	UpdateSessionName( );

	void	ReadMissionInfo(HRECORD hMission);
	void	UpdateServerInfo();

	void	UpdateProgressBar(float fPercent);
	void	UpdateCurrentBar(float fPercent);
	void	UpdateTotalBar(float fPercent);

private:


	CRITICAL_SECTION m_MissionUpdate;

	bool	m_bOldFogEnable;
	int		m_nOldFarZ;

	// State of the loading screen
	enum EScreenState { 
		STATE_NONE, // Not active in any way
		STATE_INIT, // It's been initialized, but that's it..
		STATE_SHOW, // It's on screen
		STATE_ACTIVE // It's in a thread and running
	};
	EScreenState m_eCurState;

	//////////////////////////////////////////////////////////////////////////////
	// Variables needed for updating (i.e. stuff only Init/Update/Term should touch)

	CLTGUITextCtrl	m_MissionName;
	CLTGUITextCtrl	m_LevelName;
	CLTGUITextCtrl	m_Briefing;
	CLTGUITextCtrl	m_ServerMsg;
	CLTGUITextCtrl	m_BriefingHeader;
	CLTGUITextCtrl	m_ServerMsgHeader;
	CLTGUITextCtrl	m_Help;

	CHUDBar			m_LoadProgress;
	LTRect2f		m_rfLoadProgress;

	CLTGUITextCtrl	m_CurrentFileName;
	CLTGUITextCtrl	m_CurrentFileTime;
	CLTGUITextCtrl	m_FilesLeft;
	CLTGUITextCtrl	m_TotalTime;
	CHUDBar			m_CurrentFile;
	CHUDBar			m_Total;

	LTRect2f		m_rfCurrentFile;
	LTRect2f		m_rfTotal;

	bool			m_bContentTransfer;
	double			m_fLastUpdateTime;
	
	uint32			m_nTotalBytes;
	uint32			m_nCompletedBytes;
	uint32			m_nCurrentBytes;

	CLTGUIFrame		m_ContentFrame;


	uint32			m_nCurrentFiles;

	HRECORD			m_layout;
	std::string		m_photo;

	TextureReference	m_hPhoto;
	LTPoly_GT4			m_photoPoly;

	TextureReference		m_hBackTexture;
	LTPoly_GT4		m_BackPoly;

	//layout info
	bool	m_bReadLayout;
	bool	m_bHaveServerInfo;

	uint32 m_nFrameCounter; // Frame tracking
	float m_fLastFrameTime, m_fCurFrameDelta;
	
	// If this is set the loading screen will render this rather than draw its self...
	CBaseScreen		*m_pRenderScreen;

	float			m_fLoadProgress;

};

#endif //__LOADINGSCREEN_H__