//----------------------------------------------------------
//
// MODULE  : MainMenus.H
//
// PURPOSE : Blood 2 Menus
//
// CREATED : 9/28/1998 (around)
//
//----------------------------------------------------------

#ifndef __NEWMENUS_H__
#define __NEWMENUS_H__

#include "LTGUIMgr.h"
#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "MenuBase.h"

#include "MenuMain.h"
#include "MenuBloodBath.h"
#include "MenuDifficulty.h"
#include "MenuOptions.h"
#include "MenuSinglePlayer.h"
#include "MenuCustomLevel.h"
#include "MenuLoadGame.h"
#include "MenuSaveGame.h"
#include "MenuControls.h"
#include "MenuSound.h"
#include "MenuDisplay.h"
#include "MenuCharacter.h"
#include "MenuCharacterFiles.h"
#include "MenuCharacterSelect.h"
#include "MenuMouse.h"
#include "MenuJoystick.h"
#include "MenuKeyboard.h"

#include "SharedResourceMgr.h"

//***********************************************************************

#define ID_QUICKSAVE			128
#define	ID_SAVEGAME				129
#define ID_SAVESLOT9			(ID_SAVEGAME + 9)
#define	ID_LOADGAME				196
#define ID_LOADAUTOSAVE			196
#define ID_LOADQUICKSAVE		197
#define ID_LOADSLOT0			198
#define ID_LOADSLOT9			(ID_LOADSLOT0 + 9)
#define ID_BASEWORLD			256

#define	ID_BASEOPTION			32
#define	ID_BASEHELP				96
#define	ID_BASEMULTISETUP		112

#define MAX_FILE_LIST			128

//***********************************************************************

#define MENU_MAX_STRING_SIZE	128
#define OPTION_CFGFILE_VER		0x0003

//***********************************************************************

// The different menu IDs
#define MENU_ID_MAINMENU		1
#define MENU_ID_SINGLEPLAYER	2
#define MENU_ID_BLOODBATH		3
#define MENU_ID_OPTIONS			4
#define MENU_ID_DIFFICULTY		5
#define MENU_ID_LOAD_GAME		6
#define MENU_ID_SAVE_GAME		7
#define MENU_ID_CUSTOM_LEVEL	8
#define MENU_ID_CONTROLS		9
#define MENU_ID_SOUND			10
#define MENU_ID_DISPLAY			11
#define MENU_ID_CHARACTER		12
#define MENU_ID_CHARACTERSELECT	13
#define MENU_ID_MOUSE			14
#define MENU_ID_JOYSTICK		15
#define MENU_ID_KEYBOARD		16
#define MENU_ID_CHARACTERFILES	17

// Used when changing the menus but not changing the top level menu
#define MENU_TOPLEVEL_NOT_SPECIFIED	32768

class CMainMenus
{
	public:
		CMainMenus();
		~CMainMenus();

		// Init/Term
		DBOOL		Init (CClientDE* pClientDE);
		void		Term();

		// dwMenuID			- Specifies what the current menu should be
		// dwTopLevelMenuID - Specifies the menu which is the "top level" menu meaning
		//					  that if you are at this menu and you press ESC the state
		//					  switches back to the game. Passing in MENU_TOPLEVEL_NOT_SPECIFIED
		//					  means that the menu ID should not be changed.		
		void		SetCurrentMenu(DWORD dwMenuID, DWORD dwTopLevelMenuID=MENU_TOPLEVEL_NOT_SPECIFIED);		
		void		Reset();

		// Called when the menus get or lose focus
		void		SetFocus(DBOOL bFocus);

		// Menu drawing functions
		void		Draw();
		void		ChangeBackground(char *szBackground);

		// Key up/down messages
		DBOOL		HandleKeyDown(int key, int rep);
		DBOOL		HandleKeyUp(int key);

		// Starts tracking devices for input
		DBOOL		StartTrackingDevices(int nDevices);

		// Plays a sound
		HSOUNDDE	PlaySound(char *lpszSound, DBOOL bStream=DFALSE, DBOOL bLoop=DFALSE, DBOOL bGetHandle=DFALSE);		

		// Plays select, enter, and ESC sound
		void		PlaySelectSound();
		void		PlayEnterSound();
		void		PlayEscSound();		

		// Message box routines
		void		DoMessageBox(HSTRING hMessage, CLTGUICommandHandler *pCommandHandler);
		void		DoMessageBox(int messageCode, CLTGUICommandHandler *pCommandHandler);
		void		AddMessageKey(int nVKeyCode, DWORD dwCommandID);
		void		KillMessageBox();

		// Displays a message and returns after it is drawn onto the screen.
		// Good for "Please Wait" messages
		void		ShowSyncMessage(HSTRING hMessage);
		void		ShowSyncMessage(int nMessageCode);

		// SharedResourceMgr access routines
		HSURFACE	GetSharedSurface(char *lpszPath)	{ return m_sharedResourceMgr.GetSurface(lpszPath); }
		void		FreeSharedSurface(HSURFACE hSurf)	{ m_sharedResourceMgr.FreeSurface(hSurf); }
		void		FreeSharedSurface(char *lpszPath)	{ m_sharedResourceMgr.FreeSurface(lpszPath); }

		// Sets the screen resolution
		DBOOL		SwitchResolution(int nWidth, int nHeight);

		// Switches between low and high resolution menus
		void		SetLowResolution(DBOOL bLow);

		// Access to member variables
		DIntPt				GetTitlePos()			{ return m_titlePos; }
		DIntPt				GetOptionsPos()			{ return m_optionsPos; }

		CLTGUIFont			*GetSmallFont()			{ return m_pSmallFont; }
		CLTGUIFont			*GetLargeFont()			{ return m_pLargeFont; }		
		CLTGUIFont			*GetTitleFont()			{ return m_pTitleFont; }

		CLTGUIFont			*GetLargeFadeFonts()	{ return m_pLargeFadeFonts; }
		int					GetNumLargeFadeFonts()	{ return m_nNumLargeFadeFonts; }

		CMenuDifficulty		*GetDifficultyMenu()	{ return &m_difficultyMenu; }

		HSURFACE			GetSurfaceSliderBar()	{ return m_hSurfSliderBar; }
		HSURFACE			GetSurfaceSliderTab()	{ return m_hSurfSliderTab; }
		HSURFACE			GetSurfaceUpArrow()		{ return m_hSurfUpArrow; }
		HSURFACE			GetSurfaceDownArrow()	{ return m_hSurfDownArrow; }

		DBOOL				IsLowResolution()		{ return m_bLowResolution; }
		DBOOL				IsEnglish()				{ return m_bEnglish; }

		CMenuCharacter		*GetCharacterSetup()	{ return &m_characterMenu; }
		CMenuCharacterFiles	*GetCharacterFiles()	{ return &m_characterFilesMenu; }

		int					GetYesVKeyCode()		{ return m_nYesVKeyCode; }
		int					GetNoVKeyCode()			{ return m_nNoVKeyCode; }

protected:
		// More initialization
		DBOOL		InitSurfaces();
		void		TermSurfaces();

		void		InitFonts();

		// Returns a menu based on an ID
		CMenuBase	*GetMenuFromID(DWORD dwID);

		// Blits the working surface to the screen cropping it if necessary
		void		BlitWorkingSurface(HSURFACE hScreen);

		// Draws the version string to the lower left hand corner of the surface
		void		DrawVersionString(HSURFACE hSurf);

		// Handle the tracking of any devices
		void		CheckTrackingDevices();

		// Initialize an engine font from string IDs that represent the name, width, and height
		DBOOL		InitEngineFont(CLTGUIFont *pFont, int nNameID, int nWidthID, int nHeightID);

protected:
		DBOOL			m_bInit;

		CClientDE*		m_pClientDE;			// The client interface
		DBOOL			m_bEnglish;				// True if the resource file has English as the specified language

		HSURFACE		m_hSurfBackground;		// Background surface
		HSURFACE		m_hSurfWork;			// The working surface

		HSURFACE		m_hSurfSliderBar;		// The slider bar surface
		HSURFACE		m_hSurfSliderTab;		// The slider tab surface
		
		HSURFACE		m_hSurfUpArrow;			// The up arrow surface
		HSURFACE		m_hSurfDownArrow;		// The down arrow surface

		CLTGUIFont		*m_pSmallFont;			// Small font
		CLTGUIFont		*m_pLargeFont;			// Large font
		CLTGUIFont		*m_pTitleFont;			// The title font used in non-english versions

		HSOUNDDE		m_hAmbientSound;		// The looping ambient sound

		CLTGUIFont		*m_pLargeFadeFonts;		// The large fading fonts
		int				m_nNumLargeFadeFonts;	// The number of large fading fonts in the array

		DBOOL			m_bTrackingDevices;		// Indicates that devices are being tracked
		DBOOL			m_bShiftState;			// Track whether the shift key is down or not

		DBOOL			m_bInMessageBox;		// True if we are in a message box

		int				m_nYesVKeyCode;			// The virtual key code for "yes" responses
		int				m_nNoVKeyCode;			// The virtual key code for "no" responses

		// Screen coordinates for the menus
		int				m_nMenuHeight;
		DIntPt			m_titlePos;
		DIntPt			m_optionsPos;

		DBOOL			m_bLowResolution;		// Set to true for any resolutions under 512x384

		char			*m_lpszBackground;		// The current background
		HSTRING			m_hstrVersion;			// The version number
		
		CMenuBase		*m_pCurrentMenu;		// The current menu
		CMenuBase		*m_pTopLevelMenu;		// The current top level menu

		CLTGUIMessageBox		m_messageBox;			// Message box for the menus

		CMenuMain				m_mainMenu;				// Main menu
		CMenuSinglePlayer		m_singlePlayerMenu;		// Single player menu
		CMenuBloodBath			m_bloodBathMenu;		// Bloodbath menu
		CMenuOptions			m_optionsMenu;			// Options menu
		CMenuDifficulty			m_difficultyMenu;		// Difficulty menu
		CMenuCustomLevel		m_customLevelMenu;		// Custom level menu
		CMenuLoadGame			m_loadGameMenu;			// Load game menu
		CMenuSaveGame			m_saveGameMenu;			// Save game menu
		CMenuControls			m_controlsMenu;			// Controls menu
		CMenuSound				m_soundMenu;			// The sound menu
		CMenuDisplay			m_displayMenu;			// The display menu
		CMenuCharacter			m_characterMenu;		// The character creation / mod menu		
		CMenuCharacterFiles		m_characterFilesMenu;	// Select a character file
		CMenuCharacterSelect	m_characterSelectMenu;	// The character selection menu used in action mode
		CMenuMouse				m_mouseMenu;			// The mouse settings menu
		CMenuKeyboard			m_keyboardMenu;			// The keyboard menu
		CMenuJoystick			m_joystickMenu;			// The joystick menu

		CMoArray<CMenuBase *>	m_menuArray;			// Pointer to each menu

		CSharedResourceMgr		m_sharedResourceMgr;	// Used to share title graphics
};

#endif	// __NEWMENUS_H__