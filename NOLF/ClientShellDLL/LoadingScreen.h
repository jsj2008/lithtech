// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.h
//
// PURPOSE : Background-thread loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LOADINGSCREEN_H__
#define __LOADINGSCREEN_H__

#include "BaseFolder.h"

class CLoadingScreen
{
// External functions
public:
	CLoadingScreen();
	~CLoadingScreen();

	// Note : Init/Term will be called automatically when needed, so
	// you shouldn't ever actually need to call them.

	// Initialize and load the screen resources
	// Returns LTFALSE if not in STATE_NONE
	LTBOOL Init();
	// Terminate the screen and its resources
	// Returns LTFALSE if not in STATE_INIT
	LTBOOL Term();

	// Show the loading screen
	// Setting bRun to LTTRUE will start the update thread
	// Returns LTFALES if the state isn't STATE_NONE or STATE_INIT
	LTBOOL Show(LTBOOL bRun = LTTRUE);
	// Update the loading screen
	// Returns LTFALSE if the state isn't STATE_SHOW
	LTBOOL Update();
	// Take the loading screen out of threaded mode
	// Returns LTFALSE if the state isn't STATE_ACTIVE
	LTBOOL Pause();
	// Resume threaded mode
	// Returns LTFALSE if the state isn't STATE_SHOW
	LTBOOL Resume();
	// Hide the loading screen
	// Returns LTFALSE if the state isn't STATE_SHOW or STATE_ACTIVE
	LTBOOL Hide();

	// Returns whether or not the loading screen is currently being actively updated
	inline LTBOOL IsActive() const { return m_eCurState == STATE_ACTIVE; };
	// Returns whether or not the loading screen is visible
	inline LTBOOL IsVisible() const { return ((m_eCurState == STATE_ACTIVE) || (m_eCurState == STATE_SHOW)); };

	//sets the name of the world being loaded
	void	SetWorldName(HSTRING hWorld);

	//sets the path to a photo for the world being loaded
	void	SetWorldPhoto(char *pszPhoto);

// Internal functions
protected:
	static DWORD WINAPI ThreadBootstrap(void *pData);
	int RunThread();
private:
	// The thread handle
	HANDLE m_hThreadHandle;
	// Events...
	HANDLE m_hEventEnd; // Stop rendering, damnit!
	HANDLE m_hEventThreadRunning; // Ok, the thread is done initializing, continue on

	LTBOOL	m_bOldFogEnable;
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

	LTBOOL m_bDrawMultiplayer; // Whether or not to draw the multiplayer info

	// Array of SFX owned by the loading screen
	CMoArray<CSpecialFX *> m_SFXArray;

	// The character and its attachments
	CBaseScaleFX	m_CharSFX;
	int				m_nNumAttachments;
	AttachmentData	m_aAttachment[MAX_INT_ATTACHMENTS];

	// Position variables for internal use
    LTVector m_vPos, m_vU, m_vR, m_vF;
    LTRotation m_rRot;

	// Functions for handling the character/attributes/sfx/etc
	// Note that these are basically copies of what's in BaseFolder
	void CreateScaleFX(char *szFXName);
	void CreateCharFX(INT_CHAR *pChar);
	void CreateAttachFX(INT_ATTACH *pAttach);
	void CreateInterfaceSFX(eFolderID eFolder);
	void ClearAttachFX();
	void RemoveInterfaceSFX();
	void UpdateInterfaceSFX();

	uint32 m_nFrameCounter; // Frame tracking
	float m_fLastFrameTime, m_fCurFrameDelta;


	//data for displaying what level is being loaded
	HSTRING	m_hWorldName;
	LTIntPt m_TextPos;
	
	HSURFACE m_hWorldPhoto;
	LTIntPt m_PhotoPos;
};

#endif //__LOADINGSCREEN_H__