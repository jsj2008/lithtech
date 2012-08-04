//----------------------------------------------------------
//
// MODULE  : MENU.H
//
// PURPOSE : Blood 2 Menus
//
// CREATED : 10/15/97
//
//----------------------------------------------------------


#ifndef __MENU_H__
#define __MENU_H__

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "LTGUIMgr.h" // For coolfont


// world menu items (temp)
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

#define		MENU_TITLE_X			35
#define		MENU_TITLE_Y			35

#define		OS_NUM_CATEGORIES		14

#define		SB_MAX_OS_STRING_SIZE	14

#define		OPTION_CFGFILE_VERSION	0x0002

//***********************************************************************

struct MENUITEM
{
	MENUITEM()	{ nID = 0; hSurface = hSurfaceHi = NULL; nWidth = 0; pPrev = NULL; pNext = NULL; pParent = NULL; pChild = NULL; pTop = NULL; hString = NULL; nRing = 0; nRingPos = 0; }

	int			nID;
	HSURFACE	hSurface;
	HSURFACE	hSurfaceHi;
	int			nWidth;
	HSTRING		hString;

	MENUITEM*	pPrev;
	MENUITEM*	pNext;
	MENUITEM*	pParent;
	MENUITEM*	pChild;
	MENUITEM*	pTop;
	int			nRing;
	int			nRingPos;
};

//***********************************************************************

struct OptionsStruct
{
	DBOOL		m_bAlwaysRun;
	DBOOL		m_bViewCenter;
	DBOOL		m_bFreeLook;
	DBOOL		m_bCrosshair;
	DBOOL		m_bHeadBob;
	DBOOL		m_bDodging;
	DBOOL		m_bHideWeapon;

	DBYTE		m_bScreenSize;

	DBYTE		m_bMusicType;
	DBYTE		m_bSoundQuality;
	DBOOL		m_bVoices;

	DBYTE		m_bWeaponCycle0;
	DBYTE		m_bWeaponCycle1;
	DBYTE		m_bWeaponCycle2;
	DBYTE		m_bWeaponCycle3;
	DBYTE		m_bWeaponCycle4;
	DBYTE		m_bWeaponCycle5;
	DBYTE		m_bWeaponCycle6;
	DBYTE		m_bWeaponCycle7;
	DBYTE		m_bWeaponCycle8;
	DBYTE		m_bWeaponCycle9;

	DFLOAT		m_fMouseSenseX;
	DFLOAT		m_fMouseSenseY;
	DBOOL		m_bInvertMouseY;

	DBOOL		m_bEnableJoystick;
};

//***********************************************************************

struct OPTIONITEM
{
	char	type;			// Type of option: title, slider, toggle, value, text, etc.
	char	active;			// Is the option active?

	short	num1;			// Field value 1
	short	num2;			// Field value 1

	char	*string1;		// Field string 1
	char	**string2;		// Field string 1

	char	extra[64];		// Extra string field
};

//***********************************************************************

class CMenu
{
public:
	
	CMenu();
	~CMenu();

	DBOOL		Init (CClientDE* pClientDE);
	void		Term();

	void		Activate(int nBaseMenuID);
	void		StartAnim();
	DBOOL		Animate(HSURFACE hSurface);
	void		StartAnimRings(DBOOL bFast, DFLOAT fRingAngle1, DFLOAT fRingAngle2, DFLOAT fRingAngle3);
	DBOOL		AnimateRings();
	void		Draw();

	DBOOL		HandleKeyDown(int key, int rep);
	void		Reset();
	void		ScrollUp();
	void		ScrollDown();
	DBOOL		BackOneLevel();			// returns DFALSE if already at top
	int			SelectCurrentItem();	// returns id of item, or zero if submenu exists
	MENUITEM*	GetMenuItemByID (int nID);

	// New stuff by Andy
	DBOOL		InitOptionsMenu();
	void		SetupOption(OPTIONITEM &op, char t, char *s1, char **s2, char n1, char n2, char a);
	void		TermOptionsMenu();

	OptionsStruct*	GetOptionsStruct()	{ return &m_osOptions; }

	char		HandleOptionsControl(int key);
	char		LoadConfigList();
	char		SaveConfigFile(DBOOL base);
	char		LoadConfigFile(char	*szFilename, DBOOL base);
	char		DeleteConfigFile(char *szFilename);
	FileEntry*	SortFileList(FileEntry *pfe);
	DBOOL		AcceptConfig();
	char		GetGraphicsMode();

protected:

	void		DrawWorldMenu(HSURFACE hScreen);
	void		DrawNewGameMenu(HSURFACE hScreen);
	void		DrawSaveGameMenu(HSURFACE hScreen);
	void		DrawLoadGameMenu(HSURFACE hScreen);
	void		DrawOptions(HSURFACE hScreen);
	void		DrawHelp(HSURFACE hScreen);
	void		DrawMultiSetup(HSURFACE hScreen);

	void		DeleteMenuItems (MENUITEM* pItems);
	DBOOL		BuildMenu();
	DBOOL		SetupMenuItem (MENUITEM* pItem, int nStringID, MENUITEM* pTop, MENUITEM* pPrev, MENUITEM* pParent, int pos);
	DBOOL		SetupWorldMenuItem (MENUITEM* pItem, int nID, char* strWorldName, MENUITEM* pTop, MENUITEM* pPrev, MENUITEM* pParent);
	
protected:
	
	CClientDE*	m_pClientDE;			// the client interface
	HSTRING		m_hstrVersion;			// The version string from the resources
	HSURFACE	m_hDimBack;				// entire background surface
	HSURFACE	m_hBackL;				// upper-left background surface
	HSURFACE	m_hBackR;				// upper-right background surface
	HSURFACE	m_hRing1;				// Outer ring
	HSURFACE	m_hRing2;				// Middle ring
	HSURFACE	m_hRing3;				// Inner ring
	HSURFACE	m_hTitle;				// Game title
	HSURFACE	m_hMenuTitle1;
	HSURFACE	m_hMenuTitle2;
	HSURFACE	m_hMenuTitle3;
	HSURFACE	m_hMenuScreen;			// Menu screen buffer

	HSURFACE	m_hSaveTitle;			// Title for savegame screen
	HSURFACE	m_hLoadTitle;			// Title for loadgame screen
	HSURFACE	m_hCustomTitle;			// Title for custom level screen

	CoolFontCursor	*m_pMenuCursor;
	CoolFont	*m_pMenuFont1;			// Andy's cool font for the world selection screens

	DBOOL		m_bSlideMenu;			// Menu is sliding open
	DBOOL		m_bRotateRings;			// Rings are rotating

	HSURFACE	m_hPointer;				// current menu item pointer
	DDWORD		m_cxPointer;			// width of pointer image
	DDWORD		m_cyPointer;			// height of pointer image

	DFLOAT		m_nAnimStartTime;		// starting time of the menu animation
	DFLOAT		m_fRingAngle1;			// Rotation angles for the rings
	DFLOAT		m_fRingAngle2;
	DFLOAT		m_fRingAngle3;

	DFLOAT		m_fRingMoveDist1;		// Amount to move the rings to rotate them to the correct pos.
	DFLOAT		m_fRingMoveDist2;
	DFLOAT		m_fRingMoveDist3;

	DFLOAT		m_fDestRingAngle1;		// Position that the rings want to be
	DFLOAT		m_fDestRingAngle2;
	DFLOAT		m_fDestRingAngle3;

//	HDEFONT		m_hFont;				// menu font
//
	MENUITEM*	m_pMenuItems;			// top of menu item tree
	MENUITEM*	m_pCurrentItem;			// current menu item
	MENUITEM*	m_pBaseMenu;			// current base menu

	// Option screen stuff
	char		m_bOptionsOn;			// Is the option screen on?
	OPTIONITEM	m_pOSCategory[OS_NUM_CATEGORIES];	// The main options menu categories
	char		m_bOSCategory;
	OPTIONITEM	*m_pOSSubItems[OS_NUM_CATEGORIES];	// The sub items for each category
	char		m_bOSSubItem;
	char		m_bOSTopSubItem;
	RMode		*m_pRModes;
	char		m_numModes;
	char		m_curMode;
	char		m_bOSInputMode;
	char		m_szOSInputString[SB_MAX_OS_STRING_SIZE];
	char		m_bOSInputChar;
	DeviceInput		m_pInput[MAX_INPUT_BUFFER_SIZE];
	OptionsStruct	m_osOptions;
};

//***********************************************************************

#define STARTMENU(pParent, nRingMenu)\
	{\
		MENUITEM* pMenuParent = (pParent);\
		MENUITEM* pPrev = NULL;\
		MENUITEM* pItem = NULL;\
		MENUITEM* pTop = NULL;\
		if (pParent)\
		{\
			(pParent)->pChild = new MENUITEM;\
			pItem = (pParent)->pChild;\
			pTop = (pParent)->pChild;\
		}\
		else\
		{\
			(pParent) = new MENUITEM;\
			pItem = (pParent);\
			pTop = (pParent);\
		}\
		pTop->nRing = (nRingMenu);

//***********************************************************************

#define ADDMENUITEM(id, pos)\
		SetupMenuItem (pItem, (id), pTop, pPrev, pMenuParent, pos);\
		pItem->pNext = new MENUITEM;\
		pPrev = pItem;\
		pItem = pItem->pNext;

//***********************************************************************

#define ADDWORLDMENUITEM(str, id)\
		SetupWorldMenuItem (pItem, (id), (str), pTop, pPrev, pMenuParent);\
		pItem->pNext = new MENUITEM;\
		pPrev = pItem;\
		pItem = pItem->pNext;

//***********************************************************************

#define ENDMENU()\
		{\
			pPrev->pNext = pTop;\
			pTop->pPrev = pPrev;\
			delete pItem;\
		}\
	}

#endif	// __MENU_H__
