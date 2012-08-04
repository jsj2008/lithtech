//*******************************************************************
//
// MODULE  : MainMenus.CPP
//
// PURPOSE : Blood 2 Menus
//
// CREATED : 9/25/1998
//
//*******************************************************************

#include <stdio.h>
#include "MainMenus.h"
#include "ClientRes.h"
#include "stack.h"
#include "dynarray.h"
#include "VKDefs.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"
#include "LoadSave.h"
#include "BloodClientShell.h"
#include "MenuBase.h"
#include "MenuCommands.h"
#include "SoundTypes.h"

//*******************************************************************
// Global variable declarations

#define TRACK_BUFFER_SIZE	8

//*******************************************************************
//*******************************************************************


// Constructor
CMainMenus::CMainMenus()
{
	m_bInit	= DFALSE;
	m_pClientDE	= DNULL;
	m_hSurfBackground = DNULL;
	m_hSurfWork = DNULL;
	m_hSurfSliderBar = DNULL;
	m_hSurfSliderTab = DNULL;
	m_pCurrentMenu = DNULL;
	m_pTopLevelMenu = DNULL;

	m_pSmallFont = DNULL;
	m_pLargeFont = DNULL;
	m_pTitleFont = DNULL;

	m_hstrVersion = DNULL;
	m_lpszBackground = DNULL;
	m_bShiftState = DFALSE;
	m_bInMessageBox = DFALSE;
	m_bLowResolution = DFALSE;
	m_bEnglish = DFALSE;

	m_hAmbientSound = DNULL;

	m_pLargeFadeFonts=DNULL;
	m_nNumLargeFadeFonts=7;

	m_hSurfUpArrow=DNULL;
	m_hSurfDownArrow=DNULL;

	m_nYesVKeyCode=VK_Y;
	m_nNoVKeyCode=VK_N;

	m_nMenuHeight=280;
	m_titlePos.x=82;
	m_titlePos.y=52;
	m_optionsPos.x=180;
	m_optionsPos.y=140;
}

//*******************************************************************

// Destructor
CMainMenus::~CMainMenus()
{
	if ( m_bInit )
	{
		Term();
	}
}

//*******************************************************************

// Initialization
DBOOL CMainMenus::Init(CClientDE* pClientDE)
{
	if (!pClientDE)
	{
		return DFALSE;
	}
	
	m_pClientDE = pClientDE;

	// Set the English flag
	HSTRING hString=m_pClientDE->FormatString(IDS_BLOOD2_LANGUAGE);
	if (hString && _mbsicmp((const unsigned char*)"english", (const unsigned char*)m_pClientDE->GetStringData(hString)) != 0)
	{
		m_bEnglish=DFALSE;
	}
	else
	{
		m_bEnglish=DTRUE;
	}
	m_pClientDE->FreeString(hString);
	hString=DNULL;

	// Load the virtual key codes for yes responses
	hString=m_pClientDE->FormatString(IDS_MENU_VKEY_YES);
	if (hString)
	{
		m_nYesVKeyCode=atoi(m_pClientDE->GetStringData(hString));
		m_pClientDE->FreeString(hString);
		hString=DNULL;
	}

	// Load the virtual key codes for no responses
	hString=m_pClientDE->FormatString(IDS_MENU_VKEY_NO);
	if (hString)
	{
		m_nNoVKeyCode=atoi(m_pClientDE->GetStringData(hString));
		m_pClientDE->FreeString(hString);
		hString=DNULL;
	}

	// Init the SharedResourceMgr class
	m_sharedResourceMgr.Init(m_pClientDE);

	// Determine if we need to set the low resolution flag
	RMode currentMode;
	if (m_pClientDE->GetRenderMode(&currentMode) == LT_OK)
	{
		if (currentMode.m_Width < 512 || currentMode.m_Height < 384)
		{
			m_bLowResolution=DTRUE;
			m_nMenuHeight=180;
		}
	}

	// Initialize the surfaces
	if ( !InitSurfaces() )
	{
		return DFALSE;
	}

	// Initialize the fonts
	InitFonts();

	// Initialize the message box
	m_messageBox.Create(m_pClientDE, "interface/mainmenus/dialog.pcx", GetSmallFont(), DNULL, DNULL);
	m_messageBox.SetTextColor(SETRGB(220,190,170));

	// Initialize the individual menus
	m_mainMenu.Init				(m_pClientDE, this, DNULL,				MENU_ID_MAINMENU,		m_nMenuHeight);
	m_singlePlayerMenu.Init		(m_pClientDE, this, &m_mainMenu,		MENU_ID_SINGLEPLAYER,	m_nMenuHeight);	
	m_bloodBathMenu.Init		(m_pClientDE, this, &m_mainMenu,		MENU_ID_BLOODBATH,		m_nMenuHeight);
	m_optionsMenu.Init			(m_pClientDE, this, &m_mainMenu,		MENU_ID_OPTIONS,		m_nMenuHeight);
	m_difficultyMenu.Init		(m_pClientDE, this, &m_singlePlayerMenu,MENU_ID_DIFFICULTY,		m_nMenuHeight);	
	m_customLevelMenu.Init		(m_pClientDE, this, &m_singlePlayerMenu,MENU_ID_CUSTOM_LEVEL,	m_nMenuHeight);
	m_loadGameMenu.Init			(m_pClientDE, this, &m_singlePlayerMenu,MENU_ID_LOAD_GAME,		m_nMenuHeight);	
	m_saveGameMenu.Init			(m_pClientDE, this, &m_singlePlayerMenu,MENU_ID_SAVE_GAME,		m_nMenuHeight);
	m_controlsMenu.Init			(m_pClientDE, this, &m_optionsMenu,		MENU_ID_CONTROLS,		m_nMenuHeight);
	m_soundMenu.Init			(m_pClientDE, this, &m_optionsMenu,		MENU_ID_SOUND,			m_nMenuHeight);
	m_displayMenu.Init			(m_pClientDE, this, &m_optionsMenu,		MENU_ID_DISPLAY,		m_nMenuHeight);
	m_characterMenu.Init		(m_pClientDE, this, &m_bloodBathMenu,	MENU_ID_CHARACTER,		m_nMenuHeight);	
	m_characterFilesMenu.Init	(m_pClientDE, this, &m_characterMenu,	MENU_ID_CHARACTERFILES,	100);
	m_characterSelectMenu.Init	(m_pClientDE, this, &m_singlePlayerMenu,MENU_ID_CHARACTERSELECT,m_nMenuHeight);
	m_mouseMenu.Init			(m_pClientDE, this, &m_optionsMenu	   ,MENU_ID_MOUSE		   ,m_nMenuHeight);
	m_keyboardMenu.Init			(m_pClientDE, this, &m_optionsMenu	   ,MENU_ID_KEYBOARD	   ,m_nMenuHeight);
	m_joystickMenu.Init			(m_pClientDE, this, &m_optionsMenu	   ,MENU_ID_JOYSTICK	   ,m_nMenuHeight);

	// Add each menu to the array
	m_menuArray.SetSize(0);
	m_menuArray.Add(&m_mainMenu);
	m_menuArray.Add(&m_singlePlayerMenu);
	m_menuArray.Add(&m_bloodBathMenu);
	m_menuArray.Add(&m_optionsMenu);
	m_menuArray.Add(&m_difficultyMenu);
	m_menuArray.Add(&m_customLevelMenu);
	m_menuArray.Add(&m_loadGameMenu);
	m_menuArray.Add(&m_saveGameMenu);
	m_menuArray.Add(&m_controlsMenu);
	m_menuArray.Add(&m_soundMenu);	
	m_menuArray.Add(&m_characterMenu);
	m_menuArray.Add(&m_characterFilesMenu);
	m_menuArray.Add(&m_characterSelectMenu);	
	m_menuArray.Add(&m_mouseMenu);
	m_menuArray.Add(&m_keyboardMenu);
	m_menuArray.Add(&m_joystickMenu);

	// Build each menu
	unsigned int i;
	for (i=0; i < m_menuArray.GetSize(); i++)
	{
		m_menuArray[i]->Build();
	}	

	// This is done after the build above because it shouldn't be built until the user
	// actually wants to go into the menu.	
	m_menuArray.Add(&m_displayMenu);

	SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);	
	
	// Load the version string
	m_hstrVersion = m_pClientDE->FormatString(IDS_VERSION);

	return DTRUE;
}

//*******************************************************************

// Initialize the fonts
void CMainMenus::InitFonts()
{
	m_pSmallFont = new CLTGUIFont;
	m_pLargeFont = new CLTGUIFont;
	
	// Initialize the bitmap fonts if we are in english
	if (IsEnglish())
	{
		char *lpszSmallFontPath="interface/fonts/MenuFont1.pcx";
		char *lpszSmallWidthPath="interface/fonts/MenuFont1.fnt";

		char *lpszLargeFontPath="interface/fonts/MenuFont1.pcx";
		char *lpszLargeWidthPath="interface/fonts/MenuFont1.fnt";
		
		if ( !m_pSmallFont->Init(m_pClientDE, lpszSmallFontPath, lpszSmallWidthPath) )
		{
			char szString[512];
			sprintf(szString, "Cannot load font: %s", lpszSmallFontPath);
			m_pClientDE->CPrint(szString);
		}
		
		if ( !m_pLargeFont->Init(m_pClientDE, lpszLargeFontPath, lpszLargeWidthPath) )
		{
			char szString[512];
			sprintf(szString, "Cannot load font: %s", lpszLargeFontPath);
			m_pClientDE->CPrint(szString);

			delete []m_pLargeFont;
			m_pLargeFont=DNULL;
		}
		
		// Load the large fading fonts
		m_pLargeFadeFonts = new CLTGUIFont[m_nNumLargeFadeFonts];

		int i;
		for (i=0; i < m_nNumLargeFadeFonts; i++)
		{
			char szPath[512];

			// Check to see if we should be loading a disabled font
			if (m_nNumLargeFadeFonts > 1 && i==m_nNumLargeFadeFonts-1)
			{
				sprintf(szPath, "interface/mainmenus/mm_font_dis.pcx", i+1);
			}
			else
			{
				sprintf(szPath, "interface/mainmenus/mm_font_0%d.pcx", i+1);
			}

			if ( !m_pLargeFadeFonts[i].Init(m_pClientDE, szPath, "interface/mainmenus/mm_font.fnt") )
			{
				char szString[512];
				sprintf(szString, "Cannot load font: %s", szPath);
				m_pClientDE->CPrint(szString);			
				
				delete []m_pLargeFadeFonts;

				// Set it to the large font
				m_pLargeFadeFonts=m_pLargeFont;
				m_nNumLargeFadeFonts=1;
			}		
		}
	}
	else
	{
		m_nNumLargeFadeFonts=0;

		// Initialize the engine fonts for non-english resource files
		InitEngineFont(m_pSmallFont, IDS_MENU_FONT_SMALL_NAME, IDS_MENU_FONT_SMALL_WIDTH, IDS_MENU_FONT_SMALL_HEIGHT);
		InitEngineFont(m_pLargeFont, IDS_MENU_FONT_LARGE_NAME, IDS_MENU_FONT_LARGE_WIDTH, IDS_MENU_FONT_LARGE_HEIGHT);

		// Initialize a title font if we aren't in english
		if (!IsEnglish())
		{
			m_pTitleFont = new CLTGUIFont;
			InitEngineFont(m_pTitleFont, IDS_MENU_FONT_TITLE_NAME, IDS_MENU_FONT_TITLE_WIDTH, IDS_MENU_FONT_TITLE_HEIGHT);
		}
	}

	// Set the wrapping method
	HSTRING hString=m_pClientDE->FormatString(IDS_FONT_WRAP_USE_SPACES);
	if (_mbsicmp((const unsigned char*)m_pClientDE->GetStringData(hString), (const unsigned char*)"1") == 0)
	{
		CLTGUIFont::SetWrapMethod(DTRUE);
	}
	else
	{
		CLTGUIFont::SetWrapMethod(DFALSE);
	}
}

//*******************************************************************

// Initialize an engine font from string IDs that represent the name, width, and height
DBOOL CMainMenus::InitEngineFont(CLTGUIFont *pFont, int nNameID, int nWidthID, int nHeightID)
{
	if (!pFont)
	{
		return DFALSE;
	}

	// Get the font name, width, and height
	HSTRING hName=m_pClientDE->FormatString(nNameID);
	HSTRING hWidth=m_pClientDE->FormatString(nWidthID);
	HSTRING hHeight=m_pClientDE->FormatString(nHeightID);

	int nWidth=atoi(m_pClientDE->GetStringData(hWidth));
	int nHeight=atoi(m_pClientDE->GetStringData(hHeight));

	// Initialize the font
	DBOOL bResult;
	bResult=pFont->Init(m_pClientDE, m_pClientDE->GetStringData(hName), nWidth, nHeight, DFALSE, DFALSE, DFALSE);

	if (!bResult)
	{				
		char szString[1024];
		sprintf(szString, "Cannot initialize font: %s", m_pClientDE->GetStringData(hName));
		m_pClientDE->CPrint(szString);		
	}

	// Free the strings
	m_pClientDE->FreeString(hName);
	m_pClientDE->FreeString(hWidth);
	m_pClientDE->FreeString(hHeight);

	return bResult;
}

//*******************************************************************

// Termination
void CMainMenus::Term()
{
	// Terminate the SharedResourceMgr class
	m_sharedResourceMgr.Term();

	// Terminate the surfaces
	TermSurfaces();	
	
	// Terminate the message box
	m_messageBox.Destroy();

	// Term the menus
	// Build each menu
	unsigned int i;
	for (i=0; i < m_menuArray.GetSize(); i++)
	{
		m_menuArray[i]->Term();
	}	
	
	// Terminate the fonts
	if ( m_pSmallFont )
	{
		m_pSmallFont->Term();
		delete m_pSmallFont;
		m_pSmallFont=DNULL;
	}
	if ( m_pLargeFont )
	{
		m_pLargeFont->Term();
		delete m_pLargeFont;
		m_pLargeFont=DNULL;
	}
	if ( m_pTitleFont )
	{
		m_pTitleFont->Term();
		delete m_pTitleFont;
		m_pTitleFont=DNULL;
	}

	if ( m_lpszBackground )
	{
		delete []m_lpszBackground;
		m_lpszBackground=DNULL;
	}

	if ( m_pLargeFadeFonts )
	{
		int i;
		for (i=0; i < m_nNumLargeFadeFonts; i++)
		{
			m_pLargeFadeFonts[i].Term();
		}
		delete []m_pLargeFadeFonts;
		m_pLargeFadeFonts=DNULL;
	}

	// Free the version string
	if (m_pClientDE)
	{
		m_pClientDE->FreeString(m_hstrVersion);
		m_hstrVersion = NULL;
	}
}

//*******************************************************************

// Initialize the surfaces
DBOOL CMainMenus::InitSurfaces()
{
	if ( m_pClientDE )
	{
		// The background image

#ifdef _ADDON
		m_hSurfBackground = m_pClientDE->CreateSurfaceFromBitmap("interface_ao\\background.pcx");
#else
		m_hSurfBackground = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/background.pcx");
#endif

		// The working surface
		m_hSurfWork = m_pClientDE->CreateSurface(640, 480);

		// The slider bar surface
		m_hSurfSliderBar = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/SliderBar.pcx");

		// The slider tab surface
		m_hSurfSliderTab = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/SliderTab.pcx");

		// The up arrow surface		
		m_hSurfUpArrow = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/UpArrow.pcx");

		// The down arrow surface
		m_hSurfDownArrow = m_pClientDE->CreateSurfaceFromBitmap("interface/mainmenus/DownArrow.pcx");
	}
	else
	{
		return DFALSE;
	}

	return DTRUE;
}

//*******************************************************************

// Terminate ths surfaces
void CMainMenus::TermSurfaces()
{
	if (m_pClientDE)
	{
		if (m_hSurfBackground)
		{
			m_pClientDE->DeleteSurface(m_hSurfBackground);
			m_hSurfBackground = NULL;
		}
		if (m_hSurfWork)
		{
			m_pClientDE->DeleteSurface(m_hSurfWork);
			m_hSurfWork = NULL;
		}
		if (m_hSurfSliderBar)
		{
			m_pClientDE->DeleteSurface(m_hSurfSliderBar);
			m_hSurfSliderBar = NULL;
		}
		if (m_hSurfSliderTab)
		{
			m_pClientDE->DeleteSurface(m_hSurfSliderTab);
			m_hSurfSliderTab = NULL;
		}
		if (m_hSurfUpArrow)
		{
			m_pClientDE->DeleteSurface(m_hSurfUpArrow);
			m_hSurfUpArrow = NULL;
		}
		if (m_hSurfDownArrow)
		{
			m_pClientDE->DeleteSurface(m_hSurfDownArrow);
			m_hSurfDownArrow = NULL;
		}
	}
}

//*******************************************************************

// Sets the screen resolution
DBOOL CMainMenus::SwitchResolution(int nWidth, int nHeight)
{
	RMode currentMode;
	m_pClientDE->GetRenderMode(&currentMode);

	currentMode.m_Width=nWidth;
	currentMode.m_Height=nHeight;

	if (m_pClientDE->SetRenderMode(&currentMode) != LT_OK)
	{
		return DFALSE;
	}
	else
	{
		if (nWidth < 512 || nHeight < 384)
		{
			SetLowResolution(DTRUE);
		}
		else
		{
			SetLowResolution(DFALSE);
		}
		return DTRUE;
	}	
}	

//*******************************************************************

// Switches between low and high resolution menus
void CMainMenus::SetLowResolution(DBOOL bLow)
{
	m_bLowResolution=bLow;

	// Set all of the menu heights	
	if (m_bLowResolution)
	{
		m_nMenuHeight=180;
	}
	else
	{
		m_nMenuHeight=280;
	}

	unsigned int i;
	for (i=0; i < m_menuArray.GetSize(); i++)
	{		
		// Set the menu height
		m_menuArray[i]->SetMenuHeight(m_nMenuHeight);
	
		// Tell it whether to use high rez or low rez fonts
		m_menuArray[i]->SetLowResolutionFonts(m_bLowResolution);
	}	
}

//*******************************************************************

// Called when the menus get or lose focus
void CMainMenus::SetFocus(DBOOL bFocus)
{
	if (bFocus)
	{
		// Pause all of the sounds
		m_pClientDE->PauseSounds();

		// Play one of the 4 ambient sounds
		int nSound=GetRandom(1,4);

#ifdef _DEMO
		if (nSound == 1) nSound = 2;
		if (nSound == 3) nSound = 4;
#endif

		char szSoundPath[256];
		sprintf(szSoundPath, "/Sounds/Interface/MainMenus/ambient0%d.wav", nSound);
		m_hAmbientSound=PlaySound(szSoundPath, DTRUE, DTRUE, DTRUE);
	}
	else
	{		
		// Stop the ambient sound
		if (m_hAmbientSound)
		{
			m_pClientDE->KillSound(m_hAmbientSound);
			m_hAmbientSound=DNULL;
		}

		// Resume all of the sounds
		m_pClientDE->ResumeSounds();
	}
}

//*******************************************************************

// Plays a sound
HSOUNDDE CMainMenus::PlaySound(char *lpszSound, DBOOL bStream, DBOOL bLoop, DBOOL bGetHandle)
{
	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT(playSoundInfo);
	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;

	if (bStream)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	}
	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	}
	if (bGetHandle)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	}

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)lpszSound, sizeof( playSoundInfo.m_szSoundName ) - 1 );
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;

	// Play the sound
	if( m_pClientDE->PlaySound (&playSoundInfo) != LT_OK )
		return NULL;

	return playSoundInfo.m_hSound;
}

//*******************************************************************

// Draw the menus
void CMainMenus::Draw()
{
	// The screen surface
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	// Blit the background to the working surface
	if(m_hSurfBackground && m_hSurfWork)
	{
		m_pClientDE->DrawSurfaceToSurface(m_hSurfWork, m_hSurfBackground, NULL, 0, 0);
	}

	// Render the current menu
	if ( m_pCurrentMenu )
	{
		m_pCurrentMenu->Render(m_hSurfWork);
	}	

	if ( m_bInMessageBox )
	{		
		m_messageBox.Render(m_hSurfWork);
	}

	// Blit the working surface to the screen
	BlitWorkingSurface(hScreen);	

	// Draw the version string
	if (m_pCurrentMenu == &m_mainMenu)
		DrawVersionString(hScreen);

	// Handle the tracking of any devices
	CheckTrackingDevices();
}

//*******************************************************************

// Changes the background of the menu
void CMainMenus::ChangeBackground(char *szBackground)
{
	// If the new background doesn't exist, just paint the old one black
	if(!szBackground)
	{
		HDECOLOR	black;
		black = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, DFALSE);
		m_pClientDE->FillRect(m_hSurfBackground, DNULL, black);

		if (m_lpszBackground)
		{
			delete []m_lpszBackground;
			m_lpszBackground=DNULL;
		}
		return;
	}

	// If the backgrounds are the same, then we don't need to reload them!
	if (m_lpszBackground)
	{
		if (_mbsicmp((const unsigned char*)szBackground, (const unsigned char*)m_lpszBackground) == 0)
		{
			return;
		}
		else
		{
			delete []m_lpszBackground;
			m_lpszBackground=DNULL;
		}
	}

	// Copy the background string
	int nBufferSize=_mbstrlen(szBackground)+1;
	m_lpszBackground = new char[nBufferSize];
	memset(m_lpszBackground, 0, nBufferSize);
	_mbscpy((unsigned char*)m_lpszBackground, (const unsigned char*)szBackground);

	// Delete the current background
	if(m_hSurfBackground)
	{
		m_pClientDE->DeleteSurface(m_hSurfBackground);
		m_hSurfBackground = DNULL;
	}

	// Replace it with the new background
	m_hSurfBackground = m_pClientDE->CreateSurfaceFromBitmap(szBackground);
}

//*******************************************************************

// Draws the version string to the lower left hand corner of the surface
void CMainMenus::DrawVersionString(HSURFACE hSurf)
{
	// The small font
	CLTGUIFont *pFont=GetSmallFont();

	if ( !pFont || !m_hstrVersion )
	{		
		return;
	}

	DWORD dwScreenWidth=0;
	DWORD dwScreenHeight=0;
	m_pClientDE->GetSurfaceDims (hSurf, &dwScreenWidth, &dwScreenHeight);
		
	int y=dwScreenHeight-pFont->GetHeight();

	HSTRING hString=m_pClientDE->CreateString(version_string);
	pFont->DrawSolid(hString, hSurf, 0, y, CF_JUSTIFY_LEFT, SETRGB(100,0,0));	

	if (hString)
	{
		m_pClientDE->FreeString(hString);
	}

}

//*******************************************************************

// Blits the working surface to the screen cropping it if necessary.
void CMainMenus::BlitWorkingSurface(HSURFACE hScreen)
{
	DDWORD dwScreenWidth=0;
	DDWORD dwScreenHeight=0;
	DDWORD dwWorkWidth=0;
	DDWORD dwWorkHeight=0;
			
	// Get the dims of the screen and the working surface
	m_pClientDE->GetSurfaceDims (hScreen, &dwScreenWidth, &dwScreenHeight);	
	m_pClientDE->GetSurfaceDims (m_hSurfWork, &dwWorkWidth, &dwWorkHeight);

	// If the screen surface is smaller than the working surface, crop and center the image.	
	if ( dwScreenWidth < dwWorkWidth || dwScreenHeight < dwWorkHeight )
	{	
		int nWidthClip=(dwWorkWidth-dwScreenWidth)/2;		
		int nHeightClip=(dwWorkHeight-dwScreenHeight)/2;

		DRect srcRect;
		srcRect.left=nWidthClip;
		srcRect.right=dwWorkWidth-nWidthClip;
		srcRect.top=nHeightClip;
		srcRect.bottom=dwWorkHeight-nHeightClip;

		DRect destRect;
		destRect.left=0;
		destRect.right=dwScreenWidth;
		destRect.bottom=dwScreenHeight;

		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSurfWork, &srcRect, 0, 0);		
	}
	else
	{
		// Center the image and blit it to the screen
		int nLeft=(dwScreenWidth/2)-(dwWorkWidth/2);
		int nTop=(dwScreenHeight/2)-(dwWorkHeight/2);
		int nBottom=(dwScreenHeight/2)+(dwWorkHeight/2);

		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSurfWork, NULL, nLeft, nTop);
	}
}

//*******************************************************************

// Set the current menu
void CMainMenus::SetCurrentMenu(DWORD dwMenuID, DWORD dwTopLevelMenuID)
{		
	CMenuBase *pNewMenu=GetMenuFromID(dwMenuID);
	if ( !pNewMenu )
	{
		assert(false);
		return;		
	}

	// Tell the old menu that it is losing focus
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->OnFocus(DFALSE);
	}

	m_pCurrentMenu=pNewMenu;
	
	// If the new menu hasn't been built yet... better build it!
	if (!m_pCurrentMenu->IsBuilt())
	{
		m_pCurrentMenu->Build();
	}

	ChangeBackground(m_pCurrentMenu->GetBackground());

	// Do any special case work for each menu
	switch(dwMenuID)
	{
	case MENU_ID_SINGLEPLAYER:
		{
			// Disable the save option under certain circumstances
			if ( !g_pBloodClientShell->IsInWorld() || g_pBloodClientShell->IsMultiplayerGame() || g_pBloodClientShell->IsDead() )
			{
				m_singlePlayerMenu.EnableSave(DFALSE);		
			}
			else
			{
				m_singlePlayerMenu.EnableSave(DTRUE);		
			}
			break;
		}
	case MENU_ID_LOAD_GAME:
		{
			// Add the saved game options to the menu
			m_loadGameMenu.InitSavedGameSlots();			
			break;
		}
	case MENU_ID_SAVE_GAME:
		{
			// Add the saved game options to the menu
			m_saveGameMenu.InitSavedGameSlots();
			break;
		}
	}	

	// Set the top level menu
	if ( dwTopLevelMenuID != MENU_TOPLEVEL_NOT_SPECIFIED )
	{
		m_pTopLevelMenu=GetMenuFromID(dwTopLevelMenuID);
	}

	// Set the selection
	pNewMenu->SetCurrentItem(pNewMenu->GetCurrentItemIndex());

	// Tell the new menu that it is gaining focus
	if (pNewMenu)
	{
		pNewMenu->OnFocus(DTRUE);
	}
}

//*******************************************************************

// Returns a menu based on an ID
CMenuBase *CMainMenus::GetMenuFromID(DWORD dwMenuID)
{
	CMenuBase *pMenu=NULL;

	switch ( dwMenuID )
	{
	case MENU_ID_MAINMENU:
		{			
			pMenu=&m_mainMenu;
			break;
		}
	case MENU_ID_SINGLEPLAYER:
		{
			pMenu=&m_singlePlayerMenu;
			break;
		}
	case MENU_ID_BLOODBATH:
		{
			pMenu=&m_bloodBathMenu;
			break;
		}
	case MENU_ID_OPTIONS:
		{
			pMenu=&m_optionsMenu;
			break;
		}
	case MENU_ID_DIFFICULTY:
		{
			pMenu=&m_difficultyMenu;
			break;
		}
	case MENU_ID_CUSTOM_LEVEL:
		{
			pMenu=&m_customLevelMenu;
			break;
		}
	case MENU_ID_LOAD_GAME:
		{
			pMenu=&m_loadGameMenu;
			break;
		}
	case MENU_ID_SAVE_GAME:
		{		
			pMenu=&m_saveGameMenu;
			break;
		}
	case MENU_ID_CONTROLS:
		{
			pMenu=&m_controlsMenu;
			break;
		}
	case MENU_ID_SOUND:
		{
			pMenu=&m_soundMenu;
			break;
		}
	case MENU_ID_DISPLAY:
		{
			pMenu=&m_displayMenu;
			break;
		}
	case MENU_ID_CHARACTER:
		{
			pMenu=&m_characterMenu;
			break;
		}	
	case MENU_ID_CHARACTERFILES:
		{
			pMenu=&m_characterFilesMenu;
			break;
		}	
	case MENU_ID_CHARACTERSELECT:
		{
			pMenu=&m_characterSelectMenu;
			break;
		}
	case MENU_ID_MOUSE:
		{
			pMenu=&m_mouseMenu;
			break;
		}
	case MENU_ID_KEYBOARD:
		{
			pMenu=&m_keyboardMenu;
			break;
		}
	case MENU_ID_JOYSTICK:
		{
			pMenu=&m_joystickMenu;
			break;
		}
	default:
		{
			// Should have processed a menu above!
			assert(FALSE);
		}
	}

	return pMenu;
}

//*******************************************************************

// Play select sound
void CMainMenus::PlaySelectSound()
{
	PlaySound("sounds/interface/mainmenus/select01.wav");
}

//*******************************************************************

// Play enter sound
void CMainMenus::PlayEnterSound()
{
	PlaySound("sounds/interface/mainmenus/enter01.wav");
}

//*******************************************************************

// Play esc sound
void CMainMenus::PlayEscSound()
{
	PlaySound("sounds/interface/mainmenus/esc01.wav");
}

//*******************************************************************

// Starts tracking devices for input
DBOOL CMainMenus::StartTrackingDevices(int nDevices)
{
	// Start tracking the devices
	if (m_pClientDE->StartDeviceTrack(nDevices, TRACK_BUFFER_SIZE) == LT_OK)
	{
		m_bTrackingDevices=DTRUE;

		return DTRUE;
	}
	else
	{
		return DFALSE;
	}
}

//*******************************************************************

// Sets the string for a message box and shows it on the screen.  The message box receives
// input until KillMessageBox is called.  All current key bindings are removed.
void CMainMenus::DoMessageBox(HSTRING hMessage, CLTGUICommandHandler *pCommandHandler)
{
	m_messageBox.RemoveKeys();
	m_messageBox.SetText(hMessage);	
	m_messageBox.SetCommandHandler(pCommandHandler);

	m_bInMessageBox=DTRUE;
}

//*******************************************************************

// Sets the string for a message box and shows it on the screen.  The message box receives
// input until KillMessageBox is called.  All current key bindings are removed.
void CMainMenus::DoMessageBox(int messageCode, CLTGUICommandHandler *pCommandHandler)
{
	// Get the string
	HSTRING hMessage=m_pClientDE->FormatString(messageCode);

	// Call the message box function
	DoMessageBox(hMessage, pCommandHandler);

	// Free the string
	m_pClientDE->FreeString(hMessage);
}

//*******************************************************************

// Adds a key binding to the message box
void CMainMenus::AddMessageKey(int nVKeyCode, DWORD dwCommandID)
{
	m_messageBox.AddKey(nVKeyCode, dwCommandID);
}

//*******************************************************************

void CMainMenus::KillMessageBox()
{
	m_bInMessageBox=DFALSE;
}

//*******************************************************************

// Displays a message and returns after it is drawn onto the screen.
// This works good for "Please Wait" messages
void CMainMenus::ShowSyncMessage(HSTRING hMessage)
{
	// Start the message box
	DoMessageBox(hMessage, DNULL);

	// Render the screen
	Draw();
	m_pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
	Draw();

	KillMessageBox();
}

//*******************************************************************

// Displays a message and returns after it is drawn onto the screen.
// This works good for "Please Wait" messages
void CMainMenus::ShowSyncMessage(int messageCode)
{
	// Get the string
	HSTRING hMessage=m_pClientDE->FormatString(messageCode);

	// Call the message box function
	ShowSyncMessage(hMessage);

	// Free the string
	m_pClientDE->FreeString(hMessage);
}

//*******************************************************************

// Checks any devices that are being tracked
void CMainMenus::CheckTrackingDevices()
{
	if ( m_bTrackingDevices )
	{
		DeviceInput deviceInput[TRACK_BUFFER_SIZE];
		DDWORD dwInOut=TRACK_BUFFER_SIZE;

		// Track the device
		if ( m_pClientDE->TrackDevice((DeviceInput *)&deviceInput, &dwInOut) == LT_OK )
		{
			if ( dwInOut > 0 )
			{
				// Tell the menu to handle the device tracking information
				if ( m_pCurrentMenu )
				{
					if (m_pCurrentMenu->HandleDeviceTrack((DeviceInput *)&deviceInput, dwInOut))
					{
						m_pClientDE->EndDeviceTrack();
						m_bTrackingDevices=DFALSE;
					}					
				}				
			}
		}
	}
}

//*******************************************************************

// Handle a key up message
DBOOL CMainMenus::HandleKeyUp(int key)
{
	if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
	{
		m_bShiftState = DFALSE;
		CLTGUICtrl::SetShiftState(m_bShiftState);
	}	

	return DTRUE;
}

//*******************************************************************

// Handle a key press
DBOOL CMainMenus::HandleKeyDown(int key, int rep)
{
	if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
	{
		m_bShiftState = DTRUE;
		CLTGUICtrl::SetShiftState(m_bShiftState);
	}

	// Pass the key to the message box if we are in one
	if ( m_bInMessageBox )
	{
		return m_messageBox.HandleKeyDown(key, rep);		
	}

	if (!m_pCurrentMenu)
	{
		return DFALSE;
	}
	if (!m_pCurrentMenu->HandleKeyDown(key, rep))
	{
		switch ( key )
		{
		case VK_ESCAPE:
			{				
				if ( m_pCurrentMenu == m_pTopLevelMenu )
				{
					// Return to the game if we are in a world
					if ( g_pBloodClientShell->IsInWorld() )
					{
						g_pBloodClientShell->MenuReturnToGame();
					}
					else
					{
						// We better be at the main menu!
						assert(m_pCurrentMenu == &m_mainMenu);
					}
				}
				else
				{
					// Play the sound
					PlayEscSound();

					CMenuBase *pParentMenu;
					pParentMenu=m_pCurrentMenu->GetParentMenu();
					if ( pParentMenu )
					{
						assert(m_pTopLevelMenu);
						SetCurrentMenu(pParentMenu->GetMenuID(), m_pTopLevelMenu->GetMenuID());
					}
				}
				break;
			}						
		}
	}
	
	return DTRUE;
}