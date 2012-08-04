//----------------------------------------------------------
//
// MODULE  : MENU.CPP
//
// PURPOSE : Blood 2 Menus
//
// CREATED : 10/15/97
//
//----------------------------------------------------------

#include <stdio.h>
#include "Menu.h"
#include "ClientRes.h"
#include "stack.h"
#include "dynarray.h"
#include "VKDefs.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"
#include "LoadSave.h"
#include "BloodClientShell.h"

#define MAX_CS_FILENAME_LEN	100

#define FONT_HEIGHT		40
#define FONT_WIDTH		20

#define MENU_TOP		100
#define MENU_LEFT		30
#define MENU_RIGHT		610
#define MENU_HEIGHT		350

#define RING_1_X			76
#define RING_1_Y			46
#define RING_2_X			130
#define RING_2_Y			101
#define RING_3_X			177
#define RING_3_Y			146

#define RING_MAX_ROTATE_SPEED	1.2
#define RING_MIN_ROTATE_SPEED	0.1


DFLOAT GetMoveSpeed(DFLOAT fDest,DFLOAT fCur,DFLOAT fTotal)
{
	double fMove = fabs(fDest - fCur);
	double fPercent = fMove / fTotal;
	double fSin = sin(fPercent * MATH_PI);
	DFLOAT fRet = ((DFLOAT)(RING_MIN_ROTATE_SPEED + (RING_MAX_ROTATE_SPEED - RING_MIN_ROTATE_SPEED) *
				fSin));
	return fRet;
}


// ----------------------------------------------------------------------- //

DBOOL	m_gGotKeyPress = 0;
DFLOAT	m_gKeyPressTime = 0.0f;

// ----------------------------------------------------------------------- //

// Coordinates for the words on the rotating rings
DFloatPt ptRingPos[6] = 
{
	{ 162, 60 },
	{ 172, 106 },
	{ 195, 158 },
	{ 231, 274 },
	{ 186, 311 },
	{ 167, 357 }
};

// ----------------------------------------------------------------------- //

// Coordinates for the main menu titles
DFloatPt ptMenuTitlePos[3] = 
{
	{ 6, 351 },
	{ 7, 441 },
	{ 8, 396 }
};

// ----------------------------------------------------------------------- //

char	*g_pMusicTypes[3] =
{
	"None",
	"CD Audio",
	"Interactive Music"
};

// ----------------------------------------------------------------------- //

char	*g_pSoundQuality[2] =
{
	"8-Bit",
	"16-Bit"
};

// ----------------------------------------------------------------------- //

char	*g_pActionName[47] =
{
	"Forward",			// 0
	"Backward",			// 1
	"Open",				// 2
	"TurnAround",		// 3
	"Duck",				// 4
	"Jump",				// 5
	"Speed",			// 6
	"Fire",				// 7
	"Strafe",			// 8
	"Left",				// 9
	"Right",			// 10
	"RunLock",			// 11
	"StrafeLeft",		// 12
	"StrafeRight",		// 13
	"AltFire",			// 14
	"MouseAimToggle",	// 15
	"LookUp",			// 16
	"LookDown",			// 17
	"AimUp",			// 18
	"AimDown",			// 19
	"Crosshair",		// 20
	"Message",			// 21
	"UseInventory",		// 22
	"InventoryLeft",	// 23
	"InventoryRight",	// 24
	"ScreenShrink",		// 25
	"ScreenEnlarge",	// 26
	"NextWeapon",		// 27
	"PrevWeapon",		// 28
	"DropWeapon",		// 29
	"InvItem_0",		// 30
	"InvItem_1",		// 31
	"InvItem_2",		// 32
	"InvItem_3",		// 33
	"Proximities",		// 34
	"Remotes",			// 35
	"Grab",				// 36
	"Weapon_0",			// 37
	"Weapon_1",			// 38
	"Weapon_2",			// 39
	"Weapon_3",			// 40
	"Weapon_4",			// 41
	"Weapon_5",			// 42
	"Weapon_6",			// 43
	"Weapon_7",			// 44
	"Weapon_8",			// 45
	"Weapon_9"			// 46
};

// ----------------------------------------------------------------------- //

DFLOAT	fRingAngles1[3] = { DEG2RAD(-4.4f), DEG2RAD(62.7f), DEG2RAD(120.7f) };
DFLOAT	fRingAngles2[3] = { DEG2RAD(134.4f), DEG2RAD(45.6f), DEG2RAD(-46.7f) };
DFLOAT	fRingAngles3[3] = { DEG2RAD(-90.0f), DEG2RAD(150.0f), DEG2RAD(-120.0f) };

// ----------------------------------------------------------------------- //

CMenu::CMenu()
{
	m_pClientDE = NULL;
	m_hstrVersion = NULL;
	m_hDimBack = NULL;
	m_hBackL = NULL;
	m_hBackR = NULL;
	m_hRing1 = NULL;
	m_hRing2 = NULL;
	m_hRing3 = NULL;
	m_hTitle = NULL;
	m_hMenuTitle1 = NULL;
	m_hMenuTitle2 = NULL;
	m_hMenuTitle3 = NULL;

	m_hSaveTitle = NULL;
	m_hLoadTitle = NULL;
	m_hCustomTitle = NULL;
	m_hMenuScreen = NULL;

	m_pMenuCursor = 0;
	m_pMenuFont1 = 0;

	m_bSlideMenu = DFALSE;
	m_bRotateRings = DFALSE;

	m_hPointer = NULL;
	m_cxPointer = 0;
	m_cyPointer = 0;

	m_nAnimStartTime = 0.0f;
	m_fRingAngle1 = 0.0f;
	m_fRingAngle2 = 0.0f;
	m_fRingAngle3 = 0.0f;
	m_fRingMoveDist1 = 0.0f;
	m_fRingMoveDist2 = 0.0f;
	m_fRingMoveDist3 = 0.0f;
	m_fDestRingAngle1 = 0.0f;
	m_fDestRingAngle2 = 0.0f;
	m_fDestRingAngle3 = 0.0f;

//	m_hFont = NULL;
	m_pMenuItems = NULL;
	m_pBaseMenu = NULL;
	m_pCurrentItem = NULL;

	m_bOptionsOn = 0;
}

// ----------------------------------------------------------------------- //

CMenu::~CMenu()
{
	// Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Init (CClientDE* pClientDE)
//
//	PURPOSE:	Initializes the menu
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::Init (CClientDE* pClientDE)
{
	if (!pClientDE) return DFALSE;

	m_pClientDE = pClientDE;

	m_hstrVersion = m_pClientDE->FormatString(IDS_VERSION);

	// initialize the bitmaps
	m_hDimBack = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/Blkout.pcx");
	m_hBackL = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/LHalf.pcx");
	m_hBackR = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/RHalf.pcx");
	m_hRing1 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/Ring1.pcx");
	m_hRing2 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/Ring2.pcx");
	m_hRing3 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/Ring3.pcx");
	m_hTitle = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/chosenHI.pcx");
	m_hMenuTitle1 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/mainHI.pcx");
	m_hMenuTitle2 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/singleHI.pcx");
	m_hMenuTitle3 = m_pClientDE->CreateSurfaceFromBitmap ("interface/menus/bbathHI.pcx");

	m_hSaveTitle = m_pClientDE->CreateSurfaceFromBitmap("interface/menus/savegame.pcx");
	m_hLoadTitle = m_pClientDE->CreateSurfaceFromBitmap("interface/menus/loadgame.pcx");
	m_hCustomTitle = m_pClientDE->CreateSurfaceFromBitmap("interface/menus/customlevel.pcx");
	m_hMenuScreen = m_pClientDE->CreateSurface(640, 480);

	// create the cool font
	m_pMenuFont1 = new CoolFont();
	m_pMenuFont1->Init(m_pClientDE, "interface/fonts/MenuFont1.pcx");
	m_pMenuFont1->LoadXWidths("interface/fonts/MenuFont1.fnt");

	// setup the menu cursor
	m_pMenuCursor = new CoolFontCursor();
	m_pMenuCursor->SetFont(m_pMenuFont1);

	// Set the configuration to the last one used
	InitOptionsMenu();
	if(LoadConfigFile("curb2cfg.cfg", DTRUE))
		AcceptConfig();
	TermOptionsMenu();

	// build the menu
	BuildMenu();
	m_pCurrentItem = m_pMenuItems;
	if (!m_pCurrentItem || !m_pCurrentItem->pTop) return DFALSE;

	if (m_pCurrentItem->pTop->nRing)
	{
		int nRing = m_pCurrentItem->pTop->nRing - 1;
		StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
	}
	else
	{
		StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Term()
//
//	PURPOSE:	Terminates the menu
//
// ----------------------------------------------------------------------- //

void CMenu::Term()
{
	if (m_pClientDE)
	{
		if (m_hstrVersion) { m_pClientDE->FreeString(m_hstrVersion); m_hstrVersion = NULL; }
		if (m_hDimBack)	{ m_pClientDE->DeleteSurface (m_hDimBack); m_hDimBack = NULL; }
		if (m_hBackL)	{ m_pClientDE->DeleteSurface (m_hBackL); m_hBackL = NULL; }
		if (m_hBackR)	{ m_pClientDE->DeleteSurface (m_hBackR); m_hBackR = NULL; }
		if (m_hRing1)	{ m_pClientDE->DeleteSurface (m_hRing1); m_hRing1 = NULL; }
		if (m_hRing2)	{ m_pClientDE->DeleteSurface (m_hRing2); m_hRing2 = NULL; }
		if (m_hRing3)	{ m_pClientDE->DeleteSurface (m_hRing3); m_hRing3 = NULL; }
		if (m_hTitle)	{ m_pClientDE->DeleteSurface (m_hTitle); m_hTitle = NULL; }
		if (m_hMenuTitle1) { m_pClientDE->DeleteSurface (m_hMenuTitle1); m_hMenuTitle1 = NULL; }
		if (m_hMenuTitle2) { m_pClientDE->DeleteSurface (m_hMenuTitle2); m_hMenuTitle2 = NULL; }
		if (m_hMenuTitle3) { m_pClientDE->DeleteSurface (m_hMenuTitle3); m_hMenuTitle3 = NULL; }

		if (m_hSaveTitle)	{ m_pClientDE->DeleteSurface (m_hSaveTitle); m_hSaveTitle = NULL; }
		if (m_hLoadTitle)	{ m_pClientDE->DeleteSurface (m_hLoadTitle); m_hLoadTitle = NULL; }
		if (m_hCustomTitle)	{ m_pClientDE->DeleteSurface (m_hCustomTitle); m_hCustomTitle = NULL; }
		if (m_hMenuScreen) { m_pClientDE->DeleteSurface (m_hMenuScreen); m_hMenuScreen = NULL; }

		if (m_hPointer)		{ m_pClientDE->DeleteSurface (m_hPointer); m_hPointer = NULL; }

		if (m_pMenuCursor)	{ delete m_pMenuCursor; m_pMenuCursor = 0; }
		if (m_pMenuFont1)	{ m_pMenuFont1->Free(); delete m_pMenuFont1; m_pMenuFont1 = 0; }

		if (m_pMenuItems)	{ DeleteMenuItems (m_pMenuItems); m_pMenuItems = NULL; }
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::InitOptionsMenu()
//
//	PURPOSE:	Inits the options menu
//
// --------------------------------------------------------------------------- //

DBOOL	CMenu::InitOptionsMenu()
{
	m_pClientDE->SetInputState(DFALSE);

	memset(m_pOSCategory, 0, sizeof(OPTIONITEM) * OS_NUM_CATEGORIES);
	memset(m_pOSSubItems, 0, sizeof(OPTIONITEM*) * OS_NUM_CATEGORIES);

	m_bOSCategory = 0;
	m_bOSSubItem = 0;

	SetupOption(m_pOSCategory[0], 0, "General Options\0", 0, 7, 0, 1);
	SetupOption(m_pOSCategory[1], 0, "Display Options\0", 0, 3, 0, 1);
	SetupOption(m_pOSCategory[2], 0, "Audio Options\0", 0, 5, 0, 1);
	SetupOption(m_pOSCategory[3], 0, "Advanced Weapon Options\0", 0, 10, 0, 1);
	SetupOption(m_pOSCategory[4], 0, "\0", 0, 0, 0, 0);
	SetupOption(m_pOSCategory[5], 0, "Keyboard Config\0", 0, 45, 0, 1);
	SetupOption(m_pOSCategory[6], 0, "Mouse Config\0", 0, 7, 0, 1);
	SetupOption(m_pOSCategory[7], 0, "Joystick Config\0", 0, 1, 0, 1);
	SetupOption(m_pOSCategory[8], 0, "\0", 0, 0, 0, 0);
	SetupOption(m_pOSCategory[9], 0, "Save Config\0", 0, 1, 0, 1);
	SetupOption(m_pOSCategory[10], 0, "Load Config\0", 0, 0, 0, 1);
	SetupOption(m_pOSCategory[11], 0, "Delete Config\0", 0, 0, 0, 1);
	SetupOption(m_pOSCategory[12], 0, "Accept\0", 0, 0, 0, 1);
	SetupOption(m_pOSCategory[13], 0, "Cancel\0", 0, 0, 0, 1);

	m_pOSSubItems[0] = new OPTIONITEM[m_pOSCategory[0].num1];
	m_pOSSubItems[1] = new OPTIONITEM[m_pOSCategory[1].num1];
	m_pOSSubItems[2] = new OPTIONITEM[m_pOSCategory[2].num1];
	m_pOSSubItems[3] = new OPTIONITEM[m_pOSCategory[3].num1];
	m_pOSSubItems[5] = new OPTIONITEM[m_pOSCategory[5].num1];
	m_pOSSubItems[6] = new OPTIONITEM[m_pOSCategory[6].num1];
	m_pOSSubItems[7] = new OPTIONITEM[m_pOSCategory[7].num1];

	SetupOption(m_pOSSubItems[0][0], 3, "Always Run\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[0][1], 3, "View Centering\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[0][2], 3, "Free Look\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[0][3], 3, "Crosshair\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[0][4], 3, "Head Bobbing\0", 0, 1, 0, 1);
	SetupOption(m_pOSSubItems[0][5], 3, "Dodging\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[0][6], 3, "Hide Weapon\0", 0, 0, 0, 1);

	// Make the display modes list
	m_pRModes = m_pClientDE->GetRenderModes();
	RMode	*modeIndex = m_pRModes;
	RMode	curRMode;
	m_numModes = 0;
	m_curMode = 0;

	m_pClientDE->GetRenderMode(&curRMode);
	while(modeIndex)
	{
		if((modeIndex->m_Width == curRMode.m_Width) && (modeIndex->m_Height == curRMode.m_Height))
			m_curMode = m_numModes;
		m_numModes++;
		modeIndex = modeIndex->m_pNext;
	}

	SetupOption(m_pOSSubItems[1][0], 5, "Graphics Resolution\0", 0, m_curMode, m_numModes - 1, 1);
	SetupOption(m_pOSSubItems[1][1], 2, "Screen Size\0", 0, 4, 4, 1);
	SetupOption(m_pOSSubItems[1][2], 2, "Gamma Correction\0", 0, 5, 10, 1);

	SetupOption(m_pOSSubItems[2][0], 2, "Sound Volume\0", 0, (char)(m_pClientDE->GetSoundVolume() / 10), 10, 1);
	SetupOption(m_pOSSubItems[2][1], 2, "Music Volume\0", 0, (char)(m_pClientDE->GetMusicVolume() / 10), 10, 1);
	SetupOption(m_pOSSubItems[2][2], 5, "Music Source\0", g_pMusicTypes, 0, 2, 1);
	SetupOption(m_pOSSubItems[2][3], 5, "Sound Quality\0", g_pSoundQuality, 0, 1, 1);
	SetupOption(m_pOSSubItems[2][4], 3, "Voices\0", 0, 1, 0, 1);

	SetupOption(m_pOSSubItems[3][0], 4, "Weapon 1 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][1], 4, "Weapon 2 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][2], 4, "Weapon 3 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][3], 4, "Weapon 4 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][4], 4, "Weapon 5 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][5], 4, "Weapon 6 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][6], 4, "Weapon 7 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][7], 4, "Weapon 8 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][8], 4, "Weapon 9 Cycle\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[3][9], 4, "Weapon 0 Cycle\0", 0, 0, 10, 1);

	SetupOption(m_pOSSubItems[5][0], 6, "Forward\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[5][1], 6, "Backward\0", 0, 0, 1, 1);
	SetupOption(m_pOSSubItems[5][2], 6, "Turn Left\0", 0, 0, 9, 1);
	SetupOption(m_pOSSubItems[5][3], 6, "Turn Right\0", 0, 0, 10, 1);
	SetupOption(m_pOSSubItems[5][4], 6, "Strafe\0", 0, 0, 8, 1);
	SetupOption(m_pOSSubItems[5][5], 6, "Strafe Left\0", 0, 0, 12, 1);
	SetupOption(m_pOSSubItems[5][6], 6, "Strafe Right\0", 0, 0, 13, 1);
	SetupOption(m_pOSSubItems[5][7], 6, "Run\0", 0, 0, 6, 1);
	SetupOption(m_pOSSubItems[5][8], 6, "Jump\0", 0, 0, 5, 1);
	SetupOption(m_pOSSubItems[5][9], 6, "Crouch\0", 0, 0, 4, 1);
	SetupOption(m_pOSSubItems[5][10], 6, "Look Up\0", 0, 0, 16, 1);
	SetupOption(m_pOSSubItems[5][11], 6, "Look Down\0", 0, 0, 17, 1);
	SetupOption(m_pOSSubItems[5][12], 6, "Turn Around\0", 0, 0, 3, 1);
	SetupOption(m_pOSSubItems[5][13], 6, "Mouse Look Toggle\0", 0, 0, 15, 1);
	SetupOption(m_pOSSubItems[5][14], 6, "Use\0", 0, 0, 2, 1);
	SetupOption(m_pOSSubItems[5][15], 6, "Grab\0", 0, 0, 36, 1);
	SetupOption(m_pOSSubItems[5][16], 6, "Primary Fire\0", 0, 0, 7, 1);
	SetupOption(m_pOSSubItems[5][17], 6, "Alt Fire \0", 0, 0, 14, 1);
	SetupOption(m_pOSSubItems[5][18], 6, "Use Item\0", 0, 0, 22, 1);
	SetupOption(m_pOSSubItems[5][19], 6, "Next Item\0", 0, 0, 24, 1);
	SetupOption(m_pOSSubItems[5][20], 6, "Previous Item\0", 0, 0, 23, 1);
	SetupOption(m_pOSSubItems[5][21], 6, "Next Weapon\0", 0, 0, 27, 1);
	SetupOption(m_pOSSubItems[5][22], 6, "Previous Weapon\0", 0, 0, 28, 1);
	SetupOption(m_pOSSubItems[5][23], 6, "Drop Weapon\0", 0, 0, 29, 1);
	SetupOption(m_pOSSubItems[5][24], 6, "Weapon Layout\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[5][25], 6, "Send Message\0", 0, 0, 21, 1);
	SetupOption(m_pOSSubItems[5][26], 6, "Send Macro\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[5][27], 6, "Shrink Screen\0", 0, 0, 25, 1);
	SetupOption(m_pOSSubItems[5][28], 6, "Enlarge Screen\0", 0, 0, 26, 1);
	SetupOption(m_pOSSubItems[5][29], 6, "Message Log\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[5][30], 6, "Frags / Level Status\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[5][31], 6, "Use Item 1\0", 0, 0, 30, 1);
	SetupOption(m_pOSSubItems[5][32], 6, "Use Item 2\0", 0, 0, 31, 1);
	SetupOption(m_pOSSubItems[5][33], 6, "Use Item 3\0", 0, 0, 32, 1);
	SetupOption(m_pOSSubItems[5][34], 6, "Use Item 4\0", 0, 0, 33, 1);
	SetupOption(m_pOSSubItems[5][35], 6, "Select Weapon 1\0", 0, 0, 37, 1);
	SetupOption(m_pOSSubItems[5][36], 6, "Select Weapon 2\0", 0, 0, 38, 1);
	SetupOption(m_pOSSubItems[5][37], 6, "Select Weapon 3\0", 0, 0, 39, 1);
	SetupOption(m_pOSSubItems[5][38], 6, "Select Weapon 4\0", 0, 0, 40, 1);
	SetupOption(m_pOSSubItems[5][39], 6, "Select Weapon 5\0", 0, 0, 41, 1);
	SetupOption(m_pOSSubItems[5][40], 6, "Select Weapon 6\0", 0, 0, 42, 1);
	SetupOption(m_pOSSubItems[5][41], 6, "Select Weapon 7\0", 0, 0, 43, 1);
	SetupOption(m_pOSSubItems[5][42], 6, "Select Weapon 8\0", 0, 0, 44, 1);
	SetupOption(m_pOSSubItems[5][43], 6, "Select Weapon 9\0", 0, 0, 45, 1);
	SetupOption(m_pOSSubItems[5][44], 6, "Select Weapon 10\0", 0, 0, 46, 1);

	SetupOption(m_pOSSubItems[6][0], 2, "Mouse X Sensitivity\0", 0, 5, 10, 1);
	SetupOption(m_pOSSubItems[6][1], 2, "Mouse Y Sensitivity\0", 0, 5, 10, 1);
	SetupOption(m_pOSSubItems[6][2], 3, "Invert Y Axis\0", 0, 0, 0, 1);
	SetupOption(m_pOSSubItems[6][3], 0, "\0", 0, 0, 0, 0);
	SetupOption(m_pOSSubItems[6][4], 5, "Mouse Button 1\0", g_pActionName, 7, 46, 1);
	SetupOption(m_pOSSubItems[6][5], 5, "Mouse Button 2\0", g_pActionName, 0, 46, 1);
	SetupOption(m_pOSSubItems[6][6], 5, "Mouse Button 3\0", g_pActionName, 14, 46, 1);

	SetupOption(m_pOSSubItems[7][0], 3, "Enable Joystick\0", 0, 0, 0, 1);

	m_bOptionsOn = 1;
	m_bOSInputMode = 0;
	return	1;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SetupOption()
//
//	PURPOSE:	Initializes a menu option
//
// --------------------------------------------------------------------------- //

void	CMenu::SetupOption(OPTIONITEM &op, char t, char *s1, char **s2, char n1, char n2, char a)
{
	op.type = t;
	op.string1 = s1;
	op.string2 = s2;
	op.num1 = n1;
	op.num2 = n2;
	op.active = a;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::GetGraphicsMode()
//
//	PURPOSE:	Initializes a the graphics mode menu option
//
// --------------------------------------------------------------------------- //

char	CMenu::GetGraphicsMode()
{
	// Make the display modes list
	m_pRModes = m_pClientDE->GetRenderModes();
	RMode	*modeIndex = m_pRModes;
	RMode	curRMode;
	m_numModes = 0;
	m_curMode = 0;

	m_pClientDE->GetRenderMode(&curRMode);
	while(modeIndex)
	{
		if((modeIndex->m_Width == curRMode.m_Width) && (modeIndex->m_Height == curRMode.m_Height))
			m_curMode = m_numModes;
		m_numModes++;
		modeIndex = modeIndex->m_pNext;
	}

	SetupOption(m_pOSSubItems[1][0], 5, "Graphics Resolution\0", 0, m_curMode, m_numModes - 1, 1);
	return	1;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::TermOptionsMenu()
//
//	PURPOSE:	Terminates the data from the options menu
//
// --------------------------------------------------------------------------- //

void	CMenu::TermOptionsMenu()
{
	for(char i = 0; i < OS_NUM_CATEGORIES; i++)
		if(m_pOSSubItems[i])
			delete m_pOSSubItems[i];

	memset(m_pOSCategory, 0, sizeof(OPTIONITEM) * OS_NUM_CATEGORIES);
	memset(m_pOSSubItems, 0, sizeof(OPTIONITEM*) * OS_NUM_CATEGORIES);

	m_bOSCategory = 0;
	m_bOSSubItem = 0;
	m_bOptionsOn = 0;

	m_pClientDE->RelinquishRenderModes(m_pRModes);
	m_pRModes = 0;

	m_pClientDE->SetInputState(DTRUE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::OnKeyDown()
//
//	PURPOSE:	Called upon a key down message from Windows.
//
// --------------------------------------------------------------------------- //
DBOOL CMenu::HandleKeyDown(int key, int rep)
{
	if(m_bOptionsOn)	return HandleOptionsControl(key);

	switch (key)
	{
		case VK_RETURN:
			return	1;
		case VK_DOWN:
		case VK_RIGHT:
		{
			// play the switch sound
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			ScrollDown();
			break;
		}

		case VK_UP:
		case VK_LEFT:
		{
			// play the switch sound
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			ScrollUp();
			break;
		}

		case VK_ESCAPE:
		{
			// play the select sound
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect2.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			if (!BackOneLevel())
			{	
//				Reset(); 
				return 1;	
			}
			break;
		}
	}
	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::HandleOptionsControl(int key)
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char CMenu::HandleOptionsControl(int key)
{
	if((m_bOSInputMode == 2) && (m_bOptionsOn == 2))
	{
		if(m_bOSInputChar < SB_MAX_OS_STRING_SIZE - 1)
		{
			if((key >= VK_0) && (key <= VK_9))
			{
				m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				m_szOSInputString[m_bOSInputChar - 1] = key;
				m_szOSInputString[m_bOSInputChar++] = '_';
				m_szOSInputString[m_bOSInputChar] = 0;
				return	0;
			}

			if((key >= VK_A) && (key <= VK_Z))
			{
				m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				m_szOSInputString[m_bOSInputChar - 1] = key;
				m_szOSInputString[m_bOSInputChar++] = '_';
				m_szOSInputString[m_bOSInputChar] = 0;
				return	0;
			}

			if(key == VK_SPACE)
			{
				m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				m_szOSInputString[m_bOSInputChar - 1] = ' ';
				m_szOSInputString[m_bOSInputChar++] = '_';
				m_szOSInputString[m_bOSInputChar] = 0;
				return	0;
			}
		}

		if((key == VK_BACK) && (m_bOSInputChar > 1))
		{
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			m_bOSInputChar--;
			m_szOSInputString[m_bOSInputChar] = 0;
			m_szOSInputString[m_bOSInputChar - 1] = '_';
			return	0;
		}

		if(key == VK_RETURN)
		{
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			m_bOSInputChar--;
			m_szOSInputString[m_bOSInputChar] = 0;
			SaveConfigFile(DFALSE);

			m_bOSInputMode = 0;
			m_bOptionsOn = 1;
//			m_bOSInputChar = 0;
			return	0;
		}

		if(key == VK_ESCAPE)
		{
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
			m_bOSInputMode = 0;
			m_bOptionsOn = 1;
			memset(m_szOSInputString, 0, SB_MAX_OS_STRING_SIZE);
//			m_bOSInputChar = 0;
			return	0;
		}
		return	0;
	}

	if(m_gGotKeyPress && (m_pClientDE->GetTime() < m_gKeyPressTime + 0.1f))
	{
//		m_bOSInputMode = 0;
		m_gGotKeyPress = 0;
		return	0;
	}

/*	if((m_bOSInputMode == 1) && (m_bOptionsOn == 2) && m_gGotKeyPress)
	{
		m_bOSInputMode = 0;
		m_gGotKeyPress = 0;
		return	0;
	}
*/
	if(!m_bOSInputMode)
		switch(key)
		{
			case	VK_ESCAPE:
				m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect2.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				if(m_bOptionsOn == 1)	
				{	
					TermOptionsMenu(); 
					if (!BackOneLevel())
						return 1;
				}
				else	
					m_bOptionsOn--;

				if((m_bOSCategory == 10) || (m_bOSCategory == 11))
				{
					delete	m_pOSSubItems[m_bOSCategory];
					m_pOSSubItems[m_bOSCategory] = 0;
				}
				return	0;

			case	VK_UP:
				if(m_bOptionsOn == 1)
				{
					m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
					do
					{
						m_bOSCategory--;
						if(m_bOSCategory < 0)	m_bOSCategory = OS_NUM_CATEGORIES - 1;
					}
					while(!m_pOSCategory[m_bOSCategory].active);
				}
				else
				{
					m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
					do
					{
						m_bOSSubItem--;
						if(m_bOSTopSubItem && (m_bOSSubItem == m_bOSTopSubItem))
							m_bOSTopSubItem--;
						if(m_bOSSubItem < 0)
							{ m_bOSSubItem = m_pOSCategory[m_bOSCategory].num1 - 1; m_bOSTopSubItem = m_bOSSubItem - 13; }
						if(m_bOSTopSubItem < 0)
							m_bOSTopSubItem = 0;
					}
					while(!m_pOSSubItems[m_bOSCategory][m_bOSSubItem].active);
				}
				break;

			case	VK_DOWN:
				if(m_bOptionsOn == 1)
				{
					m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
					do
					{
						m_bOSCategory++;
						if(m_bOSCategory > OS_NUM_CATEGORIES - 1)	m_bOSCategory = 0;
					}
					while(!m_pOSCategory[m_bOSCategory].active);
				}
				else
				{
					m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
					do
					{
						m_bOSSubItem++;
						if((m_bOSTopSubItem + 14 < m_pOSCategory[m_bOSCategory].num1) && (m_bOSSubItem == m_bOSTopSubItem + 13))
							m_bOSTopSubItem++;
						if(m_bOSSubItem > m_pOSCategory[m_bOSCategory].num1 - 1)
							{ m_bOSSubItem = 0; m_bOSTopSubItem = 0; }
					}
					while(!m_pOSSubItems[m_bOSCategory][m_bOSSubItem].active);
				}
				break;

			case	VK_LEFT:
				if(m_bOptionsOn == 2)
				{
					OPTIONITEM	*op = &(m_pOSSubItems[m_bOSCategory][m_bOSSubItem]);
					short		tempVal = op->num1;

					switch(op->type)
					{
						case	2:
						case	4:
						case	5:	if(op->num1 > 0) op->num1--;			break;
						case	3:	op->num1 = op->num1 ? 0 : 1;			break;
					}
					if(tempVal != op->num1)
						m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				}
				break;

			case	VK_RIGHT:
				if(m_bOptionsOn == 2)
				{
					OPTIONITEM	*op = &(m_pOSSubItems[m_bOSCategory][m_bOSSubItem]);
					short		tempVal = op->num1;

					switch(op->type)
					{
						case	2:
						case	4:
						case	5:	if(op->num1 < op->num2) op->num1++;		break;
						case	3:	op->num1 = op->num1 ? 0 : 1;			break;
					}
					if(tempVal != op->num1)
						m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
				}
				break;

			case	VK_RETURN:
				if(m_bOptionsOn == 1)
				{
					m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
					m_bOptionsOn = 2;
					m_bOSSubItem = 0;
					m_bOSTopSubItem = 0;

					switch(m_bOSCategory)
					{
						case	9:
							m_bOSInputMode = 2;
							memset(m_szOSInputString, 0, SB_MAX_OS_STRING_SIZE);
							m_szOSInputString[0] = '_';
							m_bOSInputChar = 1;
							break;
						case	10:
						case	11:		LoadConfigList();					break;
						case	12:		if (!AcceptConfig()) 
											return 1;		
										break;
						case	13:		TermOptionsMenu(); 
										if (!BackOneLevel()) 
											return 1;
										break;
					}
				}
				else if(m_bOptionsOn == 2)
				{
					if(m_bOSCategory == 10)
					{
						LoadConfigFile(m_pOSSubItems[m_bOSCategory][m_bOSSubItem].string1, DFALSE);
						delete	m_pOSSubItems[m_bOSCategory];
						m_pOSSubItems[m_bOSCategory] = 0;
						m_bOptionsOn--;
					}
					else if(m_bOSCategory == 11)
					{
						DeleteConfigFile(m_pOSSubItems[m_bOSCategory][m_bOSSubItem].string1);
						delete	m_pOSSubItems[m_bOSCategory];
						m_pOSSubItems[m_bOSCategory] = 0;
						m_bOptionsOn--;
					}
					else if(m_pOSSubItems[m_bOSCategory][m_bOSSubItem].type == 6)
					{
						m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);

						if(m_pClientDE->StartDeviceTrack (DEVICETYPE_KEYBOARD, MAX_INPUT_BUFFER_SIZE) != LT_OK)
							{ m_pClientDE->EndDeviceTrack(); return 0; }

						m_bOSInputMode = 1;
					}
				}
				return	0;
		}

	// Set the volume controls
	m_pClientDE->SetSoundVolume(m_pOSSubItems[2][0].num1 * 10);
	m_pClientDE->SetMusicVolume(m_pOSSubItems[2][1].num1 * 10);

	// Set the Gamma
	char	tempStr[32];
	sprintf(tempStr, "\"gamma\" \"%f\"", 0.5f + (m_pOSSubItems[1][2].num1 / m_pOSSubItems[1][2].num2));
	m_pClientDE->RunConsoleString(tempStr);
	return	0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::LoadConfigList()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char CMenu::LoadConfigList()
{
	FileEntry	*pfe = m_pClientDE->GetFileList("CFGFiles");
	FileEntry	*index = pfe;
	short		i, numFiles = 0;

	// Count the number of files
	while(index)	{ numFiles++; index = index->m_pNext; }

	m_pOSCategory[m_bOSCategory].num1 = numFiles;
	m_pOSSubItems[m_bOSCategory] = new OPTIONITEM[numFiles];

	index = pfe;
	for(i = 0; i < numFiles; i++)
	{
		SetupOption(m_pOSSubItems[m_bOSCategory][i], 1, index->m_pBaseFilename, 0, 0, 0, 1);
		index = index->m_pNext;
	}

	index = 0;
	m_pClientDE->FreeFileList(pfe);
	return	1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SaveConfigFile()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char CMenu::SaveConfigFile(DBOOL base)
{
	FILE	*file = 0;
	char	str1[128];
	char	*str2 = ".cfg";
	short	i, j, num;

	if(base)
		_mbscpy((unsigned char*)str1, (const unsigned char*)"blood2/");
	else
		_mbscpy((unsigned char*)str1, (const unsigned char*)"blood2/cfgfiles/");

	_mbscat((unsigned char*)str1, (const unsigned char*)m_szOSInputString);
	_mbscat((unsigned char*)str1, (const unsigned char*)str2);

	if(file = fopen(str1, "wb"))
	{
		short	version = OPTION_CFGFILE_VERSION;
		fwrite(&version, sizeof(short), 1, file);

		for(i = 0; i < 8; i++)
		{
			num = m_pOSCategory[i].num1;
			for(j = 0; j < num; j++)
			{
				fwrite(&(m_pOSSubItems[i][j].num1), sizeof(short), 1, file);
				fwrite(&(m_pOSSubItems[i][j].num2), sizeof(short), 1, file);
				fwrite(m_pOSSubItems[i][j].extra, sizeof(char), 64, file);
			}
		}
		fclose(file);
	}
	return	1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::LoadConfigFile()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char CMenu::LoadConfigFile(char	*szFilename, DBOOL base)
{
	FILE	*file = 0;
	char	str1[128];
	short	i, j, num;

	if(base)
		_mbscpy((unsigned char*)str1, (const unsigned char*)"blood2/");
	else
		_mbscpy((unsigned char*)str1, (const unsigned char*)"blood2/cfgfiles/");

	_mbscat((unsigned char*)str1, (const unsigned char*)szFilename);

	if(file = fopen(str1, "rb"))
	{
		short	version;
		fread(&version, sizeof(short), 1, file);

		if(version != OPTION_CFGFILE_VERSION)
		{
			fclose(file);
			return	0;
		}

		for(i = 0; i < 8; i++)
		{
			num = m_pOSCategory[i].num1;
			for(j = 0; j < num; j++)
			{
				fread(&(m_pOSSubItems[i][j].num1), sizeof(short), 1, file);
				fread(&(m_pOSSubItems[i][j].num2), sizeof(short), 1, file);
				fread(m_pOSSubItems[i][j].extra, sizeof(char), 64, file);
			}
		}
		fclose(file);
	}
	else
		return	0;

	return	1;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SortFileList()
//
//	PURPOSE:	sorts a file linked list, returns the sorted list.
//
// ----------------------------------------------------------------------- //

FileEntry* CMenu::SortFileList(FileEntry *pfe)
{

	FileEntry	*pfindex;
	FileEntry	*pfList[MAX_FILE_LIST];
	int nCount=0;


	// Build an array of FileEntries.
	pfindex = pfe;
	while (pfindex && nCount < MAX_FILE_LIST)
	{
		pfList[nCount++] = pfindex;
		pfindex = pfindex->m_pNext;
	}
	if (pfindex) // Free any remaining items
	{
		m_pClientDE->FreeFileList(pfindex);
	}

	for (int i = nCount / 2; i > 0; i = (i == 2) ? 1 : (int) (i / 2.2))
	{
		for (int j = i; j < nCount; j++)
		{
			FileEntry *pfTemp = pfList[j];
			
			for (int k = j; k >= i && _mbsicmp((const unsigned char*)pfTemp->m_pBaseFilename, (const unsigned char*)pfList[k - i]->m_pBaseFilename) < 0; k -= i)
			{
				pfList[k] = pfList[k - i];
			}

			pfList[k] = pfTemp;
		}
	}

	pfindex = pfList[0];
	for (i=1; i < nCount-1; i++)
	{
		pfindex->m_pNext = pfList[i];
		pfindex = pfindex->m_pNext;
	}
	pfindex->m_pNext = DNULL;

	return pfList[0];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::DeleteConfigFile()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

char CMenu::DeleteConfigFile(char *szFilename)
{
	char	str1[128] = "blood2/cfgfiles/";
	_mbscat((unsigned char*)str1, (const unsigned char*)szFilename);
	if(_mbscmp((const unsigned char*)szFilename, (const unsigned char*)"default.cfg"))	remove(str1);

	return	1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::AcceptConfig()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::AcceptConfig()
{
	char	tempStr[128];

	// Fill in the options structure for external use
	m_osOptions.m_bAlwaysRun		= (DBOOL)m_pOSSubItems[0][0].num1;
	m_osOptions.m_bViewCenter		= (DBOOL)m_pOSSubItems[0][1].num1;
	m_osOptions.m_bFreeLook			= (DBOOL)m_pOSSubItems[0][2].num1;
	m_osOptions.m_bCrosshair		= (DBOOL)m_pOSSubItems[0][3].num1;
	m_osOptions.m_bHeadBob			= (DBOOL)m_pOSSubItems[0][4].num1;
	m_osOptions.m_bDodging			= (DBOOL)m_pOSSubItems[0][5].num1;
	m_osOptions.m_bHideWeapon		= (DBOOL)m_pOSSubItems[0][6].num1;

	m_osOptions.m_bScreenSize		= (DBYTE)m_pOSSubItems[1][1].num1;

	m_osOptions.m_bMusicType		= (DBYTE)m_pOSSubItems[2][2].num1;
	m_osOptions.m_bSoundQuality		= (DBYTE)m_pOSSubItems[2][3].num1;
	m_osOptions.m_bVoices			= (DBOOL)m_pOSSubItems[2][4].num1;

	m_osOptions.m_bWeaponCycle0		= (DBYTE)m_pOSSubItems[3][0].num1;
	m_osOptions.m_bWeaponCycle1		= (DBYTE)m_pOSSubItems[3][1].num1;
	m_osOptions.m_bWeaponCycle2		= (DBYTE)m_pOSSubItems[3][2].num1;
	m_osOptions.m_bWeaponCycle3		= (DBYTE)m_pOSSubItems[3][3].num1;
	m_osOptions.m_bWeaponCycle4		= (DBYTE)m_pOSSubItems[3][4].num1;
	m_osOptions.m_bWeaponCycle5		= (DBYTE)m_pOSSubItems[3][5].num1;
	m_osOptions.m_bWeaponCycle6		= (DBYTE)m_pOSSubItems[3][6].num1;
	m_osOptions.m_bWeaponCycle7		= (DBYTE)m_pOSSubItems[3][7].num1;
	m_osOptions.m_bWeaponCycle8		= (DBYTE)m_pOSSubItems[3][8].num1;
	m_osOptions.m_bWeaponCycle9		= (DBYTE)m_pOSSubItems[3][9].num1;

	m_osOptions.m_fMouseSenseX		= 0.5f + (2.0f * ((float)m_pOSSubItems[6][0].num1 / (float)m_pOSSubItems[6][0].num2));
	m_osOptions.m_fMouseSenseY		= 0.5f + (2.0f * ((float)m_pOSSubItems[6][1].num1 / (float)m_pOSSubItems[6][1].num2));
	m_osOptions.m_bInvertMouseY		= (DBOOL)m_pOSSubItems[6][2].num1;

	m_osOptions.m_bEnableJoystick	= (DBOOL)m_pOSSubItems[7][0].num1;

	// Set the keyboard configuration
	for(short i = 0; i < 45; i++)
	{
		sprintf(tempStr, "rangebind \"keyboard\" \"%s\" 0 0 \"\"", m_pOSSubItems[5][i].extra);
		m_pClientDE->RunConsoleString(tempStr);

		if(m_pOSSubItems[5][i].num1)
		{
			sprintf(tempStr, "rangebind \"keyboard\" \"%s\" 0 0 \"%s\"",
				 m_pOSSubItems[5][i].extra, g_pActionName[m_pOSSubItems[5][i].num2]);
			m_pClientDE->RunConsoleString(tempStr);
		}
	}

	// Set the mouse configuration
	sprintf(tempStr, "scale \"mouse\" \"x-axis\" %f", m_osOptions.m_fMouseSenseX);
	m_pClientDE->RunConsoleString(tempStr);
	sprintf(tempStr, "scale \"mouse\" \"y-axis\" %f", m_osOptions.m_fMouseSenseY);
	m_pClientDE->RunConsoleString(tempStr);

	sprintf(tempStr, "rangebind \"mouse\" \"button 0\" 0 0 \"%s\"", g_pActionName[m_pOSSubItems[6][4].num1]);
	m_pClientDE->RunConsoleString(tempStr);
	sprintf(tempStr, "rangebind \"mouse\" \"button 1\" 0 0 \"%s\"", g_pActionName[m_pOSSubItems[6][5].num1]);
	m_pClientDE->RunConsoleString(tempStr);
	sprintf(tempStr, "rangebind \"mouse\" \"button 2\" 0 0 \"%s\"", g_pActionName[m_pOSSubItems[6][6].num1]);
	m_pClientDE->RunConsoleString(tempStr);

	// Change the graphics resolution and driver if it is different than the current one
	short	num = m_pOSSubItems[1][0].num1;

	if(num != m_curMode)
	{
		RMode	*modeIndex = m_pRModes;
		char	i, tempStr1[50] = "ResizeScreen ", tempStr2[10], tempStr3[10];

		for(i = 0; i < num; i++)
		{
			if (modeIndex)
			{
				modeIndex = modeIndex->m_pNext;
			}
		}

		if (modeIndex)
		{
			itoa(modeIndex->m_Width, tempStr2, 10);
			itoa(modeIndex->m_Height, tempStr3, 10);
			_mbscat((unsigned char*)tempStr1, (const unsigned char*)tempStr2);
			for(i = 9; i > 0; i--)		tempStr3[i] = tempStr3[i - 1];
			tempStr3[0] = ' ';
			_mbscat((unsigned char*)tempStr1, (const unsigned char*)tempStr3);

			m_pClientDE->RunConsoleString(tempStr1);
		}
	}

	// Save the current configuration for reload purposes
	memset(m_szOSInputString, 0, SB_MAX_OS_STRING_SIZE);
	_mbscpy((unsigned char*)m_szOSInputString, (const unsigned char*)"curb2cfg");
	SaveConfigFile(DTRUE);

	// Terminate the options menu and return control to the main menu
	TermOptionsMenu();
	return BackOneLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::StartAnim()
//
//	PURPOSE:	Records the animation's starting time for use in Animate()
//
// ----------------------------------------------------------------------- //

void CMenu::StartAnim()
{
	if (!m_pClientDE) return;

	// get the start time for the animation
	m_bSlideMenu = DTRUE;
	m_bRotateRings = DTRUE;
	m_nAnimStartTime = m_pClientDE->GetTime();
	m_fRingAngle1 = m_fRingAngle2 = m_fRingAngle3 = 0.0f;
	
	m_fDestRingAngle1 = fRingAngles1[0];
	m_fRingMoveDist1 = (DFLOAT)fabs(m_fDestRingAngle1 - m_fRingAngle1);

	m_fDestRingAngle2 = fRingAngles2[0];
	m_fRingMoveDist2 = (DFLOAT)fabs(m_fDestRingAngle2 - m_fRingAngle2);

	m_fDestRingAngle3 = fRingAngles3[0];
	m_fRingMoveDist3 = (DFLOAT)fabs(m_fDestRingAngle3 - m_fRingAngle3);

	// play the ring rotation sound
	m_pClientDE->PlaySoundLocal("sounds/interface/MenuOpen1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Animate()
//
//	PURPOSE:	Animates the menu background onto the screen
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::Animate(HSURFACE hScreen)
{
	if (!m_pClientDE) return DFALSE;

	if (!m_bSlideMenu) return DFALSE;

	// get the screen's surface and it's dimensions
//	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	DDWORD nScreenWidth = 0;
	DDWORD nScreenHeight = 0;
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	
	float nDelay = 0.30f;
	float nCurrentTime = m_pClientDE->GetTime() - m_nAnimStartTime;
	if (nCurrentTime > nDelay) nCurrentTime = nDelay;
	float nRatio = nCurrentTime / nDelay;
	
	DFloatPt ptFinal;
	ptFinal.x = 266;

	HDECOLOR hTrans = m_pClientDE->SetupColor1 ( 0, 0, 0, DFALSE);
	
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hBackR, NULL, 532 - (int)(ptFinal.x * nRatio), 0, hTrans);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hBackL, NULL, (int)(ptFinal.x * nRatio) - 266, 0, hTrans);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hRing1, NULL, (int)(ptFinal.x * nRatio) - 266 + RING_1_X, RING_1_Y, hTrans);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hRing2, NULL, (int)(ptFinal.x * nRatio) - 266 + RING_2_X, RING_2_Y, hTrans);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hRing3, NULL, (int)(ptFinal.x * nRatio) - 266 + RING_3_X, RING_3_Y, hTrans);

	if (nCurrentTime == nDelay)
	{
		m_bSlideMenu = DFALSE;
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::StartAnimRings()
//
//	PURPOSE:	Starts the rings rotating to their destination positions()
//
// ----------------------------------------------------------------------- //

void CMenu::StartAnimRings(DBOOL bFast, DFLOAT fRingAngle1, DFLOAT fRingAngle2, DFLOAT fRingAngle3)
{
	if (!m_pClientDE) return;

	// get the start time for the animation
	m_bRotateRings = DTRUE;

	// randomize a little
	if (GetRandom(0, 1) && fRingAngle1)
		fRingAngle1 = (fRingAngle1 < 0) ? fRingAngle1 + PIx2 : fRingAngle1 - PIx2;
	
	if (GetRandom(0, 1) && fRingAngle2)
		fRingAngle2 = (fRingAngle2 < 0) ? fRingAngle2 + PIx2 : fRingAngle2 - PIx2;

	if (GetRandom(0, 1) && fRingAngle3)
		fRingAngle3 = (fRingAngle3 < 0) ? fRingAngle3 + PIx2 : fRingAngle3 - PIx2;

	m_fDestRingAngle1 = fRingAngle1;
	m_fRingMoveDist1 = (DFLOAT)fabs(m_fDestRingAngle1 - m_fRingAngle1);

	m_fDestRingAngle2 = fRingAngle2;
	m_fRingMoveDist2 = (DFLOAT)fabs(m_fDestRingAngle2 - m_fRingAngle2);

	m_fDestRingAngle3 = fRingAngle3;
	m_fRingMoveDist3 = (DFLOAT)fabs(m_fDestRingAngle3 - m_fRingAngle3);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::AnimateRings()
//
//	PURPOSE:	Rotates the rings to their correct location
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::AnimateRings()
{
	DBOOL bRetVal = DFALSE;

	if (!m_pClientDE) return DFALSE;

	if (!m_bRotateRings) return DFALSE;

	if (m_fRingAngle1 > m_fDestRingAngle1)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle1, m_fRingAngle1, m_fRingMoveDist1);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange2.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle1 -= fSpeed;
		if (m_fRingAngle1 < m_fDestRingAngle1) m_fRingAngle1 = m_fDestRingAngle1;
		bRetVal = DTRUE;
	}
	else if (m_fRingAngle1 < m_fDestRingAngle1)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle1, m_fRingAngle1, m_fRingMoveDist1);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange2.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle1 += fSpeed;
		if (m_fRingAngle1 > m_fDestRingAngle1) m_fRingAngle1 = m_fDestRingAngle1;
		bRetVal = DTRUE;
	}

	if (m_fRingAngle2 > m_fDestRingAngle2)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle2, m_fRingAngle2, m_fRingMoveDist2);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle2 -= fSpeed;
		if (m_fRingAngle2 < m_fDestRingAngle2) m_fRingAngle2 = m_fDestRingAngle2;
		bRetVal = DTRUE;
	}
	else if (m_fRingAngle2 < m_fDestRingAngle2)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle2, m_fRingAngle2, m_fRingMoveDist2);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle2 += fSpeed;
		if (m_fRingAngle2 > m_fDestRingAngle2) m_fRingAngle2 = m_fDestRingAngle2;
		bRetVal = DTRUE;
	}

	if (m_fRingAngle3 > m_fDestRingAngle3)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle3, m_fRingAngle3, m_fRingMoveDist3);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle3 -= fSpeed;
		if (m_fRingAngle3 < m_fDestRingAngle3) m_fRingAngle3 = m_fDestRingAngle3;
		bRetVal = DTRUE;
	}
	else if (m_fRingAngle3 < m_fDestRingAngle3)
	{
		DFLOAT fSpeed = GetMoveSpeed(m_fDestRingAngle3, m_fRingAngle3, m_fRingMoveDist3);

		m_pClientDE->PlaySoundLocal("sounds/interface/MenuChange1.wav", NULL, SOUNDPRIORITY_MISC_HIGH);
		m_fRingAngle3 += fSpeed;
		if (m_fRingAngle3 > m_fDestRingAngle3) m_fRingAngle3 = m_fDestRingAngle3;
		bRetVal = DTRUE;
	}

	if (!bRetVal)
	{
		m_bRotateRings = DFALSE;
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Draw()
//
//	PURPOSE:	Draws the menu to the screen
//
// ----------------------------------------------------------------------- //

void CMenu::Draw()
{
	if (!m_pClientDE) return;

	char	scale = 1;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	// get the screen's surface and it's dimensions
	DDWORD nScreenWidth = 0;
	DDWORD nScreenHeight = 0;
	m_pClientDE->GetSurfaceDims (m_hMenuScreen, &nScreenWidth, &nScreenHeight);

	if (Animate(m_hMenuScreen))
	{
		return;
	}
	else
	{
		// if the dims are 640x480, no stretching is needed
		HDECOLOR hTrans;

		int nRingMenu = m_pCurrentItem->pTop->nRing;

	//	if(m_bOptionsOn)	{	DrawOptions(m_hMenuScreen); return;	}

		if (!nRingMenu && !AnimateRings())
		{
			switch(m_pCurrentItem->pTop->nID)
			{
				case IDS_NEWGAME_EASY:
					scale = 0;
					DrawNewGameMenu(hScreen);
					break;
				case	ID_SAVEGAME:
					scale = 0;
					DrawSaveGameMenu(hScreen);
					break;
				case	ID_LOADGAME:
					scale = 0;
					DrawLoadGameMenu(hScreen);
					break;
				case	ID_BASEWORLD:
					scale = 0;
					DrawWorldMenu(hScreen);
					break;
				case	ID_BASEOPTION:
					scale = 0;
					DrawOptions(hScreen);
					break;
				case	ID_BASEHELP:
					scale = 0;
					DrawHelp(hScreen);
					break;
				case	ID_BASEMULTISETUP:
					scale = 0;
					DrawMultiSetup(hScreen);
					break;
			}
		}
		else if (nRingMenu >= 0 && nRingMenu <= 3)
		{
			int x;
			int y;

			// Ring menu, draw the menu items
			hTrans = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);

			m_pClientDE->DrawSurfaceToSurface (m_hMenuScreen, m_hBackL, NULL, 0, 0);
			m_pClientDE->DrawSurfaceToSurface (m_hMenuScreen, m_hBackR, NULL, 266, 0);
			m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hMenuScreen, m_hTitle, NULL, 407, 10, hTrans);

			m_pClientDE->TransformSurfaceToSurfaceTransparent (m_hMenuScreen, m_hRing1, NULL, RING_1_X, RING_1_Y, m_fRingAngle1, 1.0f, 1.0f, hTrans);
			m_pClientDE->TransformSurfaceToSurfaceTransparent (m_hMenuScreen, m_hRing2, NULL, RING_2_X, RING_2_Y, m_fRingAngle2, 1.0f, 1.0f, hTrans);
			m_pClientDE->TransformSurfaceToSurfaceTransparent (m_hMenuScreen, m_hRing3, NULL, RING_3_X, RING_3_Y, m_fRingAngle3, 1.0f, 1.0f, hTrans);

			if (!AnimateRings() && nRingMenu != 0)
			{
				// Hilite the menu title
				HSURFACE hMenuTitle = NULL;
				switch (nRingMenu)
				{
					case 1: hMenuTitle = m_hMenuTitle1; break;
					case 2: hMenuTitle = m_hMenuTitle2; break;
					case 3: hMenuTitle = m_hMenuTitle3; break;
				}

				x = (int) ptMenuTitlePos[nRingMenu-1].x;
				y = (int) ptMenuTitlePos[nRingMenu-1].y;

				m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hMenuScreen, hMenuTitle, NULL, x, y, hTrans);


				MENUITEM* pItem = m_pCurrentItem->pTop->pPrev;

				do
				{
					// if this is the current selection, draw it as selected
					x = (int) ptRingPos[pItem->nRingPos].x;
					y = (int) ptRingPos[pItem->nRingPos].y;

					if (m_pCurrentItem == pItem)
					{
						m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hMenuScreen, pItem->hSurfaceHi, NULL, x, y, hTrans);
					}
					else
					{
						m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hMenuScreen, pItem->hSurface, NULL, x, y, hTrans);
					}

					pItem = pItem->pPrev;

				} while (pItem != m_pCurrentItem->pTop->pPrev);
			}
		}
	}

	if(scale)
	{
		DDWORD nWidth = 0;
		DDWORD nHeight = 0;
		
		m_pClientDE->GetSurfaceDims (hScreen, &nWidth, &nHeight);
		DRect rcDst;
		rcDst.left = rcDst.top = 0;
		rcDst.right = nWidth;
		rcDst.bottom = nHeight;

		m_pClientDE->GetSurfaceDims (m_hMenuScreen, &nWidth, &nHeight);
		DRect rcSrc;
		rcSrc.left = rcSrc.top = 0;
		rcSrc.right = nWidth;
		rcSrc.bottom = nHeight;

		m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hMenuScreen, &rcDst, &rcSrc);
	}

	// Display the version string
	if (m_hstrVersion)
	{
		HDECOLOR	white = m_pClientDE->SetupColor1(0.5f, 0.5f, 0.5f, DFALSE);

		m_pMenuCursor->SetDest(hScreen);
		m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
		m_pMenuCursor->SetJustify(CF_JUSTIFY_RIGHT);
		int	nSpacing = m_pMenuCursor->GetHeight();
		m_pMenuCursor->SetLoc((short)nScreenWidth, (short)nScreenHeight - nSpacing);
		m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(m_hstrVersion), white);
	}
}	

//***************************************************************************
//*****		Function:	DrawWorldMenu(HSURFACE hScreen)
//*****		Details:	Draws the custom world selection screen
//***************************************************************************

void	CMenu::DrawWorldMenu(HSURFACE hScreen)
{
//	MENUITEM	*pItem = m_pCurrentItem->pTop->pPrev;
	MENUITEM	*pItem = m_pCurrentItem->pTop;
	HDECOLOR	transColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);

	DDWORD		nWidth = 0, nHeight = 0;
	m_pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	DFLOAT		xRatio = (DFLOAT)nWidth / 640.0f;
	DFLOAT		yRatio = (DFLOAT)nHeight / 480.0f;

	int			nSpacing = m_pMenuCursor->GetHeight();
	int			nCount = 0;
//	int			y = (int)((MENU_TOP + MENU_HEIGHT - FONT_HEIGHT) * yRatio);
	int			y = (int)(MENU_TOP * yRatio);
	int			x = (int)(MENU_LEFT * xRatio);

	// Text menu, draw the text items
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hDimBack, NULL, 0, 0);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hCustomTitle, NULL, (int)(MENU_TITLE_X * xRatio), (int)(MENU_TITLE_Y * yRatio), transColor);
	m_pMenuCursor->SetDest(hScreen);
	m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);

	do
	{
		m_pMenuCursor->SetLoc(x, y);
		m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), 3, 3, black);
		if(m_pCurrentItem == pItem)
			m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), red);
		else
			m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), white);

		y += nSpacing;
		if (y > (int)((MENU_TOP + MENU_HEIGHT - FONT_HEIGHT) * yRatio))
		{	
			y = (int)(MENU_TOP * yRatio); 
			x += 200;		
		}

		pItem = pItem->pNext;
	} while (pItem != m_pCurrentItem->pTop);
}

//***************************************************************************
//*****		Function:	DrawNewGameMenu(HSURFACE hScreen)
//*****		Details:	Draws the new game difficulty level screen
//***************************************************************************

void	CMenu::DrawNewGameMenu(HSURFACE hScreen)
{
	MENUITEM	*pItem = m_pCurrentItem->pTop->pPrev;
	HDECOLOR	transColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);

	DDWORD		nWidth = 0, nHeight = 0;
	m_pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	int			x = nWidth;
	int			y = nHeight;
	DFLOAT		xRatio = (DFLOAT)nWidth / 640.0f;
	DFLOAT		yRatio = (DFLOAT)nHeight / 480.0f;

	int			nSpacing = m_pMenuCursor->GetHeight() * 2;
	int			nCount = 0;
//	int			y = (int)((MENU_TOP + MENU_HEIGHT - FONT_HEIGHT) * yRatio);
//	int			x = (int)(MENU_LEFT * xRatio);

	// Text menu, draw the text items
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hDimBack, NULL, 0, 0);
	// TODO: Replace this title with "NEW GAME" - GK 8/21/98
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hCustomTitle, NULL, (int)(MENU_TITLE_X * xRatio), (int)(MENU_TITLE_Y * yRatio), transColor);
	m_pMenuCursor->SetDest(hScreen);
	m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);

	do
	{
		m_pMenuCursor->SetLoc((short)x >> 1, ((short)y >> 1) - (m_pMenuCursor->GetHeight() >> 1));

//		m_pMenuCursor->SetLoc(x, y);
		m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), 3, 3, black);
		if(m_pCurrentItem == pItem)
			m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), red);
		else
			m_pMenuCursor->DrawSolid(m_pClientDE->GetStringData(pItem->hString), white);

		y -= nSpacing;
		pItem = pItem->pPrev;
	} while (pItem != m_pCurrentItem->pTop->pPrev);
}

//***************************************************************************
//*****		Function:	DrawSaveGameMenu(HSURFACE hScreen)
//*****		Details:	Draws the save game screen
//***************************************************************************

void	CMenu::DrawSaveGameMenu(HSURFACE hScreen)
{
	MENUITEM	*pItem = m_pCurrentItem->pTop->pPrev;
	HDECOLOR	transColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);

	DDWORD		nWidth = 0, nHeight = 0;
	m_pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	DFLOAT		xRatio = (DFLOAT)nWidth / 640.0f;
	DFLOAT		yRatio = (DFLOAT)nHeight / 480.0f;

	int			nSpacing = m_pMenuCursor->GetHeight();
	int			nCount = 0;
	int			y = (int)((MENU_TOP + MENU_HEIGHT - FONT_HEIGHT) * yRatio);
	int			x = (int)(MENU_LEFT * xRatio);

	// Text menu, draw the text items
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hDimBack, NULL, 0, 0);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSaveTitle, NULL, (int)(MENU_TITLE_X * xRatio), (int)(MENU_TITLE_Y * yRatio), transColor);
	m_pMenuCursor->SetDest(hScreen);
	m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);

	char szMenuText[125];
	do
	{
		_mbscpy((unsigned char*)szMenuText, (const unsigned char*)m_pClientDE->GetStringData(pItem->hString));

		// Get additional menu item text, if available
		CSavedGameInfo *pSGInfo = DNULL;
		if (g_pBloodClientShell && (pSGInfo = g_pBloodClientShell->GetSavedGameInfo()))
		{
			if (pItem->nID >= ID_SAVEGAME && pItem->nID <= ID_SAVESLOT9)
			{
				_mbscat((unsigned char*)szMenuText, (const unsigned char*)pSGInfo->gSaveSlotInfo[pItem->nID - ID_SAVEGAME].szName);
			}
		}

		m_pMenuCursor->SetLoc(x, y);
		m_pMenuCursor->DrawSolid(szMenuText, 3, 3, black);
		if(m_pCurrentItem == pItem)
			m_pMenuCursor->DrawSolid(szMenuText, red);
		else
			m_pMenuCursor->DrawSolid(szMenuText, white);

		y -= nSpacing;
		pItem = pItem->pPrev;
	} while (pItem != m_pCurrentItem->pTop->pPrev);
}

//***************************************************************************
//*****		Function:	DrawLoadGameMenu(HSURFACE hScreen)
//*****		Details:	Draws the load game selection screen
//***************************************************************************

void	CMenu::DrawLoadGameMenu(HSURFACE hScreen)
{
	MENUITEM	*pItem = m_pCurrentItem->pTop->pPrev;
	HDECOLOR	transColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);

	DDWORD		nWidth = 0, nHeight = 0;
	m_pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	DFLOAT		xRatio = (DFLOAT)nWidth / 640.0f;
	DFLOAT		yRatio = (DFLOAT)nHeight / 480.0f;

	int			nSpacing = m_pMenuCursor->GetHeight();
	int			nCount = 0;
	int			y = (int)((MENU_TOP + MENU_HEIGHT - FONT_HEIGHT) * yRatio);
	int			x = (int)(MENU_LEFT * xRatio);

	// Text menu, draw the text items
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hDimBack, NULL, 0, 0);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hLoadTitle, NULL, (int)(MENU_TITLE_X * xRatio), (int)(MENU_TITLE_Y * yRatio), transColor);
	m_pMenuCursor->SetDest(hScreen);
	m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);

	char szMenuText[125];
	do
	{
		_mbscpy((unsigned char*)szMenuText, (const unsigned char*)m_pClientDE->GetStringData(pItem->hString));

		// Get additional menu item text, if available
		CSavedGameInfo *pSGInfo = DNULL;
		if (g_pBloodClientShell && (pSGInfo = g_pBloodClientShell->GetSavedGameInfo()))
		{
			if (pItem->nID == ID_LOADAUTOSAVE)	// Get current world name
			{
				_mbscat((unsigned char*)szMenuText, (const unsigned char*)pSGInfo->gCurrentSaveInfo.szName);
			}
			else if (pItem->nID == ID_LOADQUICKSAVE)
			{
				_mbscat((unsigned char*)szMenuText, (const unsigned char*)pSGInfo->gQuickSaveInfo.szName);
			}
			else if (pItem->nID >= ID_LOADSLOT0 && pItem->nID <= ID_LOADSLOT9)
			{
				_mbscat((unsigned char*)szMenuText, (const unsigned char*)pSGInfo->gSaveSlotInfo[pItem->nID - ID_LOADSLOT0].szName);
			}
		}

		m_pMenuCursor->SetLoc(x, y);
		m_pMenuCursor->DrawSolid(szMenuText, 3, 3, black);
		if(m_pCurrentItem == pItem)
			m_pMenuCursor->DrawSolid(szMenuText, red);
		else
			m_pMenuCursor->DrawSolid(szMenuText, white);

		y -= nSpacing;
		pItem = pItem->pPrev;
	} while (pItem != m_pCurrentItem->pTop->pPrev);
}

//***************************************************************************
//*****		Function:	DrawOptions(HSURFACE hScreen)
//*****		Details:	Draws the options screen
//***************************************************************************

void	CMenu::DrawOptions(HSURFACE hScreen)
{
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);
	DDWORD		x, y;
	short		i, max;
	char		*string1, *string2, tempString[50];
	OPTIONITEM	*op;
	DRect		rect;

	m_pClientDE->GetSurfaceDims(hScreen, &x, &y);
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface (hScreen, m_hDimBack, NULL, 0, 0);
	m_pMenuCursor->SetDest(hScreen);

	m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pMenuCursor->SetLoc((short)x >> 1, (short)y - m_pMenuCursor->GetHeight() * 2);
	m_pMenuCursor->DrawSolid("*** NOTE: These menus are currently under construction! ***", white);
	m_pMenuCursor->NewLine();
	m_pMenuCursor->DrawSolid("*** Not all options are functional ***", white);

	if((m_bOSInputMode == 1) && (m_bOptionsOn == 2))
	{
		DDWORD	i, size = MAX_INPUT_BUFFER_SIZE;
		m_pClientDE->TrackDevice(m_pInput, &size);
		for(i = 0; i < size; i++)
		{
			if(m_pInput[i].m_InputValue)
			{
				m_pClientDE->PlaySoundLocal("sounds/interface/MenuSwitch3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);

				m_gGotKeyPress = 1;
				m_gKeyPressTime = m_pClientDE->GetTime();
				if(_mbsicmp((const unsigned char*)m_pInput[i].m_ControlName, (const unsigned char*)"Escape") == 0)
				{
					m_pOSSubItems[m_bOSCategory][m_bOSSubItem].num1 = 0;
					memset(m_pOSSubItems[m_bOSCategory][m_bOSSubItem].extra, 0, 64);
				}
				else
				{
					m_pOSSubItems[m_bOSCategory][m_bOSSubItem].num1 = (short)m_pInput[i].m_InputValue;
					_mbscpy((unsigned char*)m_pOSSubItems[m_bOSCategory][m_bOSSubItem].extra, (const unsigned char*)m_pInput[i].m_ControlName);
				}

				if(_mbsicmp((const unsigned char*)m_pInput[i].m_ControlName, (const unsigned char*)"Left Alt") == 0)
				{
//					m_bOSInputMode = 0;
					m_gGotKeyPress = 0;
				}
				else if(_mbsicmp((const unsigned char*)m_pInput[i].m_ControlName, (const unsigned char*)"Right Alt") == 0)
				{
//					m_bOSInputMode = 0;
					m_gGotKeyPress = 0;
				}

				m_bOSInputMode = 0;
				m_pClientDE->EndDeviceTrack();
				break;
			}
		}
	}

	if(m_bOSInputMode == 2)
	{
		m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
		m_pMenuCursor->SetLoc((short)x >> 1, ((short)y >> 1) - (m_pMenuCursor->GetHeight() >> 1));
		m_pMenuCursor->DrawSolid(m_szOSInputString, white);
		return;
	}

	if(m_bOptionsOn == 1)
	{
		m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
		m_pMenuCursor->SetLoc((short)x >> 1, ((short)y >> 1) - ((m_pMenuCursor->GetHeight() * 14) >> 1));
		for(char i = 0; i < OS_NUM_CATEGORIES; i++)
		{
			m_pMenuCursor->DrawSolid(m_pOSCategory[i].string1, 3, 3, black);

			if(i == m_bOSCategory)
				m_pMenuCursor->DrawSolid(m_pOSCategory[i].string1, red);
			else
				m_pMenuCursor->DrawSolid(m_pOSCategory[i].string1, white);

			m_pMenuCursor->NewLine();
		}
	}
	else
	{
		if(m_pOSCategory[m_bOSCategory].num1 < 14)
		{
			m_pMenuCursor->SetLoc((short)x >> 1, ((short)y >> 1) - ((m_pMenuCursor->GetHeight() * m_pOSCategory[m_bOSCategory].num1) >> 1));
			max = m_pOSCategory[m_bOSCategory].num1;
		}
		else
		{
			m_pMenuCursor->SetLoc((short)x >> 1, ((short)y >> 1) - ((m_pMenuCursor->GetHeight() * 14) >> 1));
			max = 14;
		}

		for(i = m_bOSTopSubItem; i < m_bOSTopSubItem + max; i++)
		{
			op = &(m_pOSSubItems[m_bOSCategory][i]);
			switch(op->type)
			{
				case	1:		// Title / File
					m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
					string1 = op->string1;
					m_pMenuCursor->DrawSolid(string1, 3, 3, black);
					if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, red);
						else				m_pMenuCursor->DrawSolid(string1, white);
					break;
				case	2:		// Slider
					m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);
					string1 = op->string1;
					m_pMenuCursor->DrawSolid(string1, -147, 3, black);
					if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, -150, 0, red);
						else				m_pMenuCursor->DrawSolid(string1, -150, 0, white);

					rect.top = m_pMenuCursor->GetY() + (m_pMenuCursor->GetHeight() >> 1) - 1;
					rect.bottom = rect.top + 2;
					rect.left = m_pMenuCursor->GetX();
					rect.right = rect.left + 150;
					m_pClientDE->FillRect(hScreen, &rect, white);

					rect.top = m_pMenuCursor->GetY() + 1;
					rect.bottom = rect.top + m_pMenuCursor->GetHeight() - 2;
					rect.left = m_pMenuCursor->GetX() + ((140 / op->num2) * op->num1);
					rect.right = rect.left + 10;

					if(i == m_bOSSubItem)
						m_pClientDE->FillRect(hScreen, &rect, red);
					else
						m_pClientDE->FillRect(hScreen, &rect, white);
					break;
				case	3:		// Toggle / T or F
					m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);
					string1 = op->string1;
					m_pMenuCursor->DrawSolid(string1, -147, 3, black);
					if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, -150, 0, red);
						else				m_pMenuCursor->DrawSolid(string1, -150, 0, white);

					m_pMenuCursor->SetJustify(CF_JUSTIFY_RIGHT);
					if(op->num1)
					{
						m_pMenuCursor->DrawSolid("ON", 153, 3, black);
						if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid("ON", 150, 0, red);
							else				m_pMenuCursor->DrawSolid("ON", 150, 0, white);
					}
					else
					{
						m_pMenuCursor->DrawSolid("OFF", 153, 3, black);
						if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid("OFF", 150, 0, red);
							else				m_pMenuCursor->DrawSolid("OFF", 150, 0, white);
					}
					break;
				case	4:		// Value range
				case	6:
					m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);
					string1 = op->string1;
					m_pMenuCursor->DrawSolid(string1, -147, 3, black);
					if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, -150, 0, red);
						else				m_pMenuCursor->DrawSolid(string1, -150, 0, white);

					m_pMenuCursor->SetJustify(CF_JUSTIFY_RIGHT);
					itoa(op->num1, tempString, 10);
					if((m_bOSInputMode == 1) && (i == m_bOSSubItem))
					{
						m_pMenuCursor->DrawSolid("_\0", 153, 3, black);
						m_pMenuCursor->DrawSolid("_\0", 150, 0, red);
					}
					else if(!op->num1)
					{
						m_pMenuCursor->DrawSolid("NA", 153, 3, black);
						if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid("NA", 150, 0, red);
							else				m_pMenuCursor->DrawSolid("NA", 150, 0, white);
					}
					else
					{
						if(op->type == 6)
						{
							string1 = op->extra;
							m_pMenuCursor->DrawSolid(string1, 153, 3, black);
							if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, 150, 0, red);
								else				m_pMenuCursor->DrawSolid(string1, 150, 0, white);
						}
						else
						{
							m_pMenuCursor->DrawSolid(tempString, 153, 3, black);
							if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(tempString, 150, 0, red);
								else				m_pMenuCursor->DrawSolid(tempString, 150, 0, white);
						}
					}
					break;
				case	5:		// Text represented by a value
					m_pMenuCursor->SetJustify(CF_JUSTIFY_LEFT);
					string1 = op->string1;
					m_pMenuCursor->DrawSolid(string1, -147, 3, black);
					if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, -150, 0, red);
						else				m_pMenuCursor->DrawSolid(string1, -150, 0, white);

					m_pMenuCursor->SetJustify(CF_JUSTIFY_RIGHT);

					if(op->string2)
					{
						string1 = op->string2[op->num1];
						m_pMenuCursor->DrawSolid(string1, 153, 3, black);
						if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(string1, 150, 0, red);
							else				m_pMenuCursor->DrawSolid(string1, 150, 0, white);
					}
					else
					{
						RMode	*modeIndex = m_pRModes;
						char	i, tempStr1[250], tempStr2[15];

						for(i = 0; i < op->num1; i++)		modeIndex = modeIndex->m_pNext;
						itoa(modeIndex->m_Width, tempStr1, 10);
						itoa(modeIndex->m_Height, tempStr2, 10);
						for(i = 9; i > 0; i--)		tempStr2[i] = tempStr2[i - 1];
						tempStr2[0] = 'x';
						_mbscat((unsigned char*)tempStr1, (const unsigned char*)tempStr2);
						itoa(modeIndex->m_BitDepth, tempStr2, 10);
						for(i = 9; i > 0; i--)		tempStr2[i] = tempStr2[i - 1];
						tempStr2[0] = 'x';
						_mbscat((unsigned char*)tempStr1, (const unsigned char*)tempStr2);

						m_pMenuCursor->DrawSolid(tempStr1, 153, 3, black);
						if(i == m_bOSSubItem)	m_pMenuCursor->DrawSolid(tempStr1, 150, 0, red);
							else				m_pMenuCursor->DrawSolid(tempStr1, 150, 0, white);

						// This is what the DLLs use to identify a card.
						// This is a 'friendly' string describing the card.

						sprintf(tempStr1, "%s (%s)", modeIndex->m_Description, modeIndex->m_InternalName);
		
						m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
						m_pMenuCursor->DrawSolid(tempStr1, 0, m_pMenuCursor->GetHeight() * 5, white);
						m_pMenuCursor->DrawSolid(modeIndex->m_RenderDLL, 0, m_pMenuCursor->GetHeight() * 6, white);
					}
					break;
			}
			m_pMenuCursor->NewLine();
			string1 = string2 = 0;
		}
	}
}

//***************************************************************************
//*****		Function:	DrawHelp(HSURFACE hScreen)
//*****		Details:	Draws the help screen
//***************************************************************************

void	CMenu::DrawHelp(HSURFACE hScreen)
{
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);
	DDWORD		x, y;

	m_pClientDE->GetSurfaceDims(hScreen, &x, &y);
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface (hScreen, m_hDimBack, NULL, 0, 0);
	m_pMenuCursor->SetDest(hScreen);

	m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pMenuCursor->SetLoc((short)x >> 1, (short)y - m_pMenuCursor->GetHeight() * 2);
	m_pMenuCursor->DrawSolid("*** NOTE: These menus are currently under construction! ***", white);
	m_pMenuCursor->NewLine();
	m_pMenuCursor->DrawSolid("*** Not all options are functional ***", white);

}

//***************************************************************************
//*****		Function:	DrawMultiSetup(HSURFACE hScreen)
//*****		Details:	Draws the help screen
//***************************************************************************

void	CMenu::DrawMultiSetup(HSURFACE hScreen)
{
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);
	HDECOLOR	red = m_pClientDE->SetupColor2 (0.596f, 0.031f, 0.094f, DFALSE);
	HDECOLOR	black = m_pClientDE->SetupColor1 (0.0f, 0.0f, 0.0f, DFALSE);
	DDWORD		x, y;

	m_pClientDE->GetSurfaceDims(hScreen, &x, &y);
	m_pClientDE->ScaleSurfaceToSurface (hScreen, m_hDimBack, NULL, NULL);
//	m_pClientDE->DrawSurfaceToSurface (hScreen, m_hDimBack, NULL, 0, 0);
	m_pMenuCursor->SetDest(hScreen);

	m_pMenuCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pMenuCursor->SetLoc((short)x >> 1, (short)y - m_pMenuCursor->GetHeight() * 2);
	m_pMenuCursor->DrawSolid("*** NOTE: These menus are currently under construction! ***", white);
	m_pMenuCursor->NewLine();
	m_pMenuCursor->DrawSolid("*** Not all options are functional ***", white);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Activate()
//
//	PURPOSE:	Activates the menu system, making the base menu the passed in ID
//
// ----------------------------------------------------------------------- //

void CMenu::Activate(int nBaseMenuID)
{
	if (nBaseMenuID == 0)	// Main menu
	{
		// Reset the menu system
		Reset();
		StartAnim();
	}
	else	// Requested a submenu as the base
	{
		if (nBaseMenuID == IDS_OPTIONS)
		{ 
			InitOptionsMenu(); 
			LoadConfigFile("curb2cfg.cfg", DTRUE); 
			GetGraphicsMode(); 
		}

		MENUITEM *pMenuItem = GetMenuItemByID(nBaseMenuID);
		if (pMenuItem && pMenuItem->pChild)
		{
			m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect2.wav", NULL, SOUNDPRIORITY_MISC_HIGH);

			m_pBaseMenu = m_pCurrentItem = pMenuItem->pChild;

			if (m_pCurrentItem->nRing)
			{
				int nRing = m_pCurrentItem->nRing - 1;
				StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
			}
			else
			{
				StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::Reset()
//
//	PURPOSE:	Reset the current menu item pointer
//
// ----------------------------------------------------------------------- //

void CMenu::Reset()
{
	if (m_pCurrentItem)	
	{
		m_pCurrentItem = m_pBaseMenu = m_pMenuItems;
		if (m_pCurrentItem->pTop->nRing)
		{
			int nRing = m_pCurrentItem->pTop->nRing - 1;
			StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
		}
		else
		{
			StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::ScrollUp()
//
//	PURPOSE:	Move the current selection up one
//
// ----------------------------------------------------------------------- //

void CMenu::ScrollUp()
{
	if (m_pCurrentItem) m_pCurrentItem = m_pCurrentItem->pPrev;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::ScrollDown()
//
//	PURPOSE:	Move the current selection down one
//
// ----------------------------------------------------------------------- //

void CMenu::ScrollDown()
{
	if (m_pCurrentItem) m_pCurrentItem = m_pCurrentItem->pNext;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::BackOneLevel()
//
//	PURPOSE:	Up one menu level - returns DFALSE if at top level
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::BackOneLevel()
{
	if (m_pCurrentItem)
	{
		if (m_pCurrentItem->pTop == m_pBaseMenu)
		{
			m_pCurrentItem = m_pBaseMenu;
//			if (m_pCurrentItem->pTop->nRing)
//			{
//				int nRing = m_pCurrentItem->pTop->nRing - 1;
//				StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
//			}
//			else
//			{
//				StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
//			}
			return DFALSE;
		}
		else
		{
			m_pCurrentItem = m_pCurrentItem->pParent;
			if (m_pCurrentItem->pTop->nRing)
			{
				int nRing = m_pCurrentItem->pTop->nRing - 1;
				StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
			}
			else
			{
				StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
			}
			return DTRUE;
		}
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SelectCurrentItem()
//
//	PURPOSE:	Selects the current item - returns id if no child menu
//
// ----------------------------------------------------------------------- //

int CMenu::SelectCurrentItem()
{
	// play the select sound
	m_pClientDE->PlaySoundLocal("sounds/interface/MenuSelect3.wav", NULL, SOUNDPRIORITY_MISC_HIGH);

	if (m_pCurrentItem->nID == IDS_OPTIONS)
		{ InitOptionsMenu(); LoadConfigFile("curb2cfg.cfg", DTRUE); GetGraphicsMode(); }

	if (m_pCurrentItem)
	{
		if (m_pCurrentItem->pChild)
		{
			m_pCurrentItem = m_pCurrentItem->pChild;
			if (m_pCurrentItem->nRing)
			{
				int nRing = m_pCurrentItem->nRing - 1;
				StartAnimRings(DFALSE, fRingAngles1[nRing], fRingAngles2[nRing], fRingAngles3[nRing]);
			}
			else
			{
				StartAnimRings(DTRUE, 0.0f, 0.0f, 0.0f);
			}
			return 0;
		}
		else
		{
			return m_pCurrentItem->nID;
		}
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::GetMenuItemByID 
//
//	PURPOSE:	returns the menu item with the given id
//
// ----------------------------------------------------------------------- //

MENUITEM* CMenu::GetMenuItemByID (int nID)
{
	if (!m_pMenuItems) return NULL;

	CStack<MENUITEM*> stack (DTRUE);

	// start with first menu
	stack.Push (m_pMenuItems);

	MENUITEM* pItem = NULL;
	while (stack.Pop (pItem))
	{
		MENUITEM* pTemp = pItem;
		do
		{
			if (pTemp->nID == nID) return pTemp;
			if (pTemp->pChild) stack.Push (pTemp->pChild);
			pTemp = pTemp->pNext;

		} while (pTemp != pItem);
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::DeleteMenuItems()
//
//	PURPOSE:	Recurses through the menu items and removes 
//				them from memory
//
// ----------------------------------------------------------------------- //

void CMenu::DeleteMenuItems (MENUITEM* pItems)
{
	if (!pItems) return;

	if (pItems == pItems->pTop && pItems->pPrev)
	{
		pItems->pPrev->pNext = NULL;
	}

	if (pItems->pNext) DeleteMenuItems (pItems->pNext);
	if (pItems->pChild) DeleteMenuItems (pItems->pChild);

	m_pClientDE->DeleteSurface (pItems->hSurface);
	m_pClientDE->FreeString(pItems->hString);
	delete pItems;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::BuildMenu()
//
//	PURPOSE:	Builds the menu items
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::BuildMenu()
{
	MENUITEM* pSubMenu = NULL;
	
	// Create the main menu
	STARTMENU(m_pMenuItems, 1);
	ADDMENUITEM(IDS_SINGLEPLAYER, 0);
	ADDMENUITEM(IDS_MULTIPLAYER, 1);
	ADDMENUITEM(IDS_OPTIONS, 2);
	ADDMENUITEM(IDS_HELP, 3);
//	ADDMENUITEM(0, 4);
	ADDMENUITEM(IDS_EXIT, 5);
	ENDMENU();

	// Create single-player menu
	pSubMenu = GetMenuItemByID (IDS_SINGLEPLAYER);
	STARTMENU(pSubMenu, 2);
	ADDMENUITEM(IDS_NEWGAME, 0);
	ADDMENUITEM(IDS_LOADGAME, 1);
//	ADDMENUITEM(0, 2);
//	ADDMENUITEM(0, 3);
	ADDMENUITEM(IDS_SAVEGAME, 4);
	ADDMENUITEM(IDS_LOADCUSTOM, 5);
	ENDMENU();

	// Create bloodbath menu
	pSubMenu = GetMenuItemByID (IDS_MULTIPLAYER);
	STARTMENU(pSubMenu, 3);
	ADDMENUITEM(IDS_HOSTGAME, 0);
	ADDMENUITEM(IDS_JOINGAME, 1);
//	ADDMENUITEM(0, 2);
//	ADDMENUITEM(0, 3);
//	ADDMENUITEM(0, 4);
	ADDMENUITEM(IDS_SETUP, 5);
	ENDMENU();

	// New options menu by Andy
	pSubMenu = GetMenuItemByID(IDS_OPTIONS);
	STARTMENU(pSubMenu, 0);
	ADDWORLDMENUITEM("Under construction!", ID_BASEOPTION);
	ENDMENU();

	// New help menu by Andy
	pSubMenu = GetMenuItemByID(IDS_HELP);
	STARTMENU(pSubMenu, 0);
	ADDWORLDMENUITEM("Not yet implemented...", ID_BASEHELP);
	ENDMENU();

	// New multiplayer options menu by Andy
	pSubMenu = GetMenuItemByID(IDS_SETUP);
	STARTMENU(pSubMenu, 0);
	ADDWORLDMENUITEM("Not yet implemented...", ID_BASEMULTISETUP);
	ENDMENU();

	// New difficulty level menu GK 8/21/98
	pSubMenu = GetMenuItemByID(IDS_NEWGAME);
	STARTMENU(pSubMenu, 0);
	ADDWORLDMENUITEM("Easy", IDS_NEWGAME_EASY);
	ADDWORLDMENUITEM("Normal", IDS_NEWGAME_NORMAL);
	ADDWORLDMENUITEM("Hard", IDS_NEWGAME_HARD);
//	ADDWORLDMENUITEM("INSANE!", IDS_NEWGAME_INSANE);
	ENDMENU();

	// create the menu

	FileEntry *pfe, *pfindex;

	pfe = m_pClientDE->GetFileList("Worlds");

	pfe = pfindex = SortFileList(pfe);

	pSubMenu = GetMenuItemByID (IDS_LOADCUSTOM);

	int i = 0;
	STARTMENU(pSubMenu, 0);
	while(pfindex)
	{
		_mbslwr((unsigned char*)pfindex->m_pBaseFilename);
		if (_mbsstr((const unsigned char*)pfindex->m_pBaseFilename, (const unsigned char*)".dat"))
		{
			ADDWORLDMENUITEM(pfindex->m_pBaseFilename, ID_BASEWORLD + i);
			i++;
		}
		pfindex = pfindex->m_pNext;
	}

	m_pClientDE->FreeFileList(pfe);

	// Add this directory too..
	pfe = m_pClientDE->GetFileList("Worlds\\Test");
	pfe = pfindex = SortFileList(pfe);

	while(pfindex)
	{
		_strlwr(pfindex->m_pBaseFilename);
		if (_mbsstr((const unsigned char*)pfindex->m_pBaseFilename, (const unsigned char*)".dat"))
		{
			char sName[MAX_CS_FILENAME_LEN];
			sprintf(sName, "Test\\%s", pfindex->m_pBaseFilename);
			ADDWORLDMENUITEM(sName, ID_BASEWORLD + i);
			i++;
		}
		pfindex = pfindex->m_pNext;
	}

	m_pClientDE->FreeFileList(pfe);


	ENDMENU();

	i = 0;
	pSubMenu = GetMenuItemByID (IDS_LOADGAME);
	STARTMENU(pSubMenu, 0);

	ADDWORLDMENUITEM("restart: ", ID_LOADGAME + i); i++;
	ADDWORLDMENUITEM("quicksave: ", ID_LOADGAME + i); i++;

	for (i=0;i<MAX_SAVESLOTS;i++)
	{
		char szMenuItem[25];
		_mbscpy((unsigned char*)szMenuItem, (const unsigned char*)"");
		ADDWORLDMENUITEM(szMenuItem, ID_LOADSLOT0 + i)
	}
	ENDMENU();

	i = 0;
	pSubMenu = GetMenuItemByID (IDS_SAVEGAME);
	STARTMENU(pSubMenu, 0);
	for (i=0;i<MAX_SAVESLOTS;i++)
	{
		char szMenuItem[15];
		_mbscpy((unsigned char*)szMenuItem, (const unsigned char*)"");
		ADDWORLDMENUITEM(szMenuItem, ID_SAVEGAME + i)
	}
	ENDMENU();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SetupMenuItem (...)
//
//	PURPOSE:	Assists BuildMenu() with menu creation
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::SetupMenuItem (MENUITEM* pItem, int nStringID, MENUITEM* pTop, MENUITEM* pPrev, MENUITEM* pParent, int pos)
{
	pItem->hString = m_pClientDE->FormatString(nStringID);

	if (!pItem->hString) return DFALSE;

	pItem->nRingPos = pos;

	char szFile[MAX_CS_FILENAME_LEN];

	sprintf(szFile, "interface/menus/%s.pcx", m_pClientDE->GetStringData(pItem->hString));
	pItem->hSurface = m_pClientDE->CreateSurfaceFromBitmap(szFile);
	sprintf(szFile, "interface/menus/%s2.pcx", m_pClientDE->GetStringData(pItem->hString));
	pItem->hSurfaceHi = m_pClientDE->CreateSurfaceFromBitmap(szFile);

	if (!pItem->hSurface || !pItem->hSurfaceHi) return DFALSE;

	DDWORD nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (pItem->hSurface, &nWidth, &nHeight);

	pItem->nID = nStringID;
	pItem->nWidth = (int)nWidth;
	pItem->pParent = pParent;
	pItem->pPrev = pPrev;
	pItem->pTop = pTop;
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenu::SetupWorldMenuItem (...)
//
//	PURPOSE:	Assists BuildMenu() with menu creation
//
// ----------------------------------------------------------------------- //

DBOOL CMenu::SetupWorldMenuItem (MENUITEM* pItem, int nID, char* strWorldName, MENUITEM* pTop, MENUITEM* pPrev, MENUITEM* pParent)
{
	char tmpstr[255], *pch;
	_mbscpy((unsigned char*)tmpstr, (const unsigned char*)strWorldName);
	if(pch = _mbsstr((const unsigned char*)tmpstr, (const unsigned char*)".dat"))
		*pch = '\0';

	pItem->hString = m_pClientDE->CreateString (tmpstr);
	if (!pItem->hString) return DFALSE;

	DDWORD nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (pItem->hSurface, &nWidth, &nHeight);

	pItem->nID = nID;
	pItem->nWidth = (int)nWidth;
	pItem->pParent = pParent;
	pItem->pPrev = pPrev;
	pItem->pTop = pTop;
	
	return DTRUE;
}