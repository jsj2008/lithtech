//*************************************************************************
//*************************************************************************
//***** MODULE  : NewStatusBar.h
//***** PURPOSE : Blood 2 Status Bar
//***** CREATED : 9/25/98
//*************************************************************************
//*************************************************************************

#ifndef __NEWSTATUSBAR_H__
#define __NEWSTATUSBAR_H__

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "LTGUIMgr.h"

//**********************************************************

#define		TAB_INC_NONE			0x00
#define		TAB_INC_BORDER			0x01
#define		TAB_INC_ICON			0x02
#define		TAB_INC_BAR				0x04
#define		TAB_INC_VALUE			0x08
#define		TAB_INC_LOW				0x0A	// Icon and number
#define		TAB_INC_HIGH			0x0F	// Everything

//**********************************************************

#define		STATTAB_JUST_NW			0
#define		STATTAB_JUST_N			1
#define		STATTAB_JUST_NE			2
#define		STATTAB_JUST_E			3
#define		STATTAB_JUST_SE			4
#define		STATTAB_JUST_S			5
#define		STATTAB_JUST_SW			6
#define		STATTAB_JUST_W			7
#define		STATTAB_JUST_C			8

#define		STATTAB_NONE			0
#define		STATTAB_LOW				1
#define		STATTAB_HIGH			2

#define		STATTAB_LOW_WIDTH		512
#define		STATTAB_LOW_HEIGHT		384

//**********************************************************

#define		INVTAB_DISP_TIME		5.0f
#define		INVTAB_SCROLL_TIME		0.15f

#define		INVTAB_INVISIBLE		0
#define		INVTAB_SCROLL_IN		1
#define		INVTAB_STOPPED			2
#define		INVTAB_SCROLL_OUT		3

//**********************************************************

#define		WEAPTAB_SLOTS			10
#define		WEAPTAB_SCROLL_TIME		0.25f

#define		WEAPTAB_INVISIBLE		0
#define		WEAPTAB_SCROLL_IN		1
#define		WEAPTAB_STOPPED			2
#define		WEAPTAB_SCROLL_OUT		3

//**********************************************************

#define		OBJTAB_SCALE_TIME		0.10f
#define		OBJTAB_SCALE_LEVELS		10

#define		OBJTAB_INVISIBLE		0
#define		OBJTAB_SCALE_UP			1
#define		OBJTAB_STOPPED			2
#define		OBJTAB_SCALE_DOWN		3

#define		OBJTAB_TITLE_CENTER		135
#define		OBJTAB_TITLE_TOP		35
#define		OBJTAB_TEXT_LEFT		35
#define		OBJTAB_TEXT_TOP			50
#define		OBJTAB_TEXT_WIDTH		200

#define		OBJICON_INVISIBLE		0
#define		OBJICON_VISIBLE			1

//**********************************************************

#define		BARTAB_DISP_TIME		5.0f

#define		BARTAB_NONE				0
#define		BARTAB_THROW			1
#define		BARTAB_AIR				2
#define		BARTAB_BOSS				3

//**********************************************************

#define		MULTIFLAG_NONE			0
#define		MULTIFLAG_RED			1
#define		MULTIFLAG_BLUE			2

//**********************************************************

struct StatTab
{
	StatTab::StatTab();

	HSURFACE	hTab;			// The composite surface for the tab
	HSURFACE	hBorder;		// The border graphic to use for the tab
	HSURFACE	hIcon;			// The icon to place within the border
	HSURFACE	hBar;			// If a bar is used... what's the graphic

	DDWORD		borderX;		// X Offset of the border
	DDWORD		borderY;		// Y Offset of the border
	char		borderJ;		// Justification of the border

	DDWORD		iconX;			// X Offset of the icon
	DDWORD		iconY;			// Y Offset of the icon
	char		iconJ;			// Justification of the icon

	DDWORD		barX;			// X Offset of the bar
	DDWORD		barY;			// Y Offset of the bar
	char		barJ;			// Justification of the bar

	DDWORD		sizeX;			// Width of the tab
	DDWORD		sizeY;			// Height of the tab

	DDWORD		locX;			// X Screen position
	DDWORD		locY;			// Y Screen position

	DDWORD		value;			// Value to display within the tab
	DDWORD		max;			// Maximum value

	DDWORD		valueX;			// X Offset of the value
	DDWORD		valueY;			// Y Offset of the value
	char		valueJ;			// Justification of the value
	char		digits;			// Number of digits of the value to display
};

//----------------------------------------------------------

inline StatTab::StatTab()
{
	memset(this, 0, sizeof(StatTab));
}

//**********************************************************

class CNewStatusBar
{
	public:
		// Constructors, destructors, and intialization functions
		CNewStatusBar();
		~CNewStatusBar();

		DBOOL	Init(CClientDE* pClientDE);
		void	AdjustRes();
		void	Term();

		DBOOL	InitStatTabs();
		DBOOL	InitInvTabs();
		DBOOL	InitWeaponTabs();
		DBOOL	InitObjectivesTab();

		void	TermStatTabs();
		void	TermInvTabs();
		void	TermWeaponTabs();

		// Inventory functions
		void	UpdateInv(DBOOL bShow, HSTRING name, HSTRING c, HSTRING cH, DDWORD cN, HSTRING p, DDWORD pN, HSTRING n, DDWORD nN);

		// Weapon display functions
		void	ToggleWeapons();
		void	UpdateWeap(HSTRING weap, HSTRING weapH, char slot);
		void	UpdateCurrentWeap(char slot);

		// Stat functions
		void	StatLevelUp();
		void	StatLevelDown();
		void	SetStatLevel(DBYTE nLevel);

		// Objective screen functions
		void	ToggleObjectives();
		void	UpdateObjectives(char *title, char *string);

		// Power bar functions
		void	SetPowerBarLevel(DBYTE nBar, DFLOAT fPercent);
		void	TurnOffPowerBar()						{ m_nPowerBar = BARTAB_NONE; }

		// Team/flag functions
		void	DisplayFlag(DBYTE bColor);
		void	SetTeamID(int nTeamID);
		void	ReloadTeamIcon() { SetTeamID(m_nTeamID); }

		// General drawing functions
		void	Draw(DBOOL bDrawBar);

		// Stats interface modification functions
		void	SetHealth(DDWORD nHealth, DDWORD nMax)	{ m_stHealth.value = nHealth; m_stHealth.max = nMax; }
		void	SetArmor(DDWORD nArmor, DDWORD nMax)	{ m_stArmor.value = nArmor; m_stArmor.max = nMax; }
		void	SetAmmo(DDWORD nAmmo)					{ m_stAmmo.value = nAmmo; }
		void	SetAltAmmo(DDWORD nAltAmmo)				{ m_stAltAmmo.value = nAltAmmo; }
		void	Reset();

	private:
		// Private drawing functions
		void	SetupTab(StatTab &stTab, CoolFont *font, DBYTE nFlags);
		void	JustifySurface(HSURFACE surf, DDWORD just, DDWORD &x, DDWORD &y);

		void	DrawStatTabs();
		void	DrawInvTabs();
		void	DrawWeaponTabs();
		void	DrawObjectives();
		void	DrawObjectivesIcon();
		void	DrawPowerBar();
		void	DrawFlagIcon();
		void	DrawTeamIcon();

	private:
		// Private data members
		CClientDE*	m_pClientDE;

		HSURFACE	m_hMask;				// a pixel mask
		HSURFACE	m_hItemName;			// Name of the currently selected item
		HSURFACE	m_hObjectives;			// Surface to hold the objectives screen
		HSURFACE	m_hObjectivesIcon;		// Icon for the updated objectives screen
		HSURFACE	m_hPowerBar;			// Surface to hold the power levels
		HSURFACE	m_hHealthBar;			// Surface to hold the power levels
		HSURFACE	m_hThrowBar;			// Surface to hold the power levels
		HSURFACE	m_hFlagIcon;			// Multiplayer flag icon
		HSURFACE	m_hTeamIcon;			// Multiplayer team icon

		// Fonts and cursors
		CoolFontCursor	*m_pStatCursor;
		CoolFont		*m_pStatFont1;
		CoolFont		*m_pStatNumbers1;
		CoolFont		*m_pStatNumbers2;
		CoolFont		*m_pObjectivesFont1;

		// Stat display variables
		DBYTE		m_nStatLevel;
		DBYTE		m_nOldStatLevel;
		DBYTE		m_nInvLevel;
		DBYTE		m_nWeapLevel;
		DBYTE		m_nObjLevel;
		DBYTE		m_nObjIcon;
		DBYTE		m_nPowerBar;
		DBYTE		m_nFlagLevel;

		// Stat tab data
		StatTab		m_stHealth;
		StatTab		m_stArmor;
		StatTab		m_stAmmo;
		StatTab		m_stAltAmmo;

		StatTab		m_stCurrentItem;
		StatTab		m_stItems[3];

		StatTab		m_stWeapons[WEAPTAB_SLOTS];

		DDWORD		m_nOldHealth;
		DDWORD		m_nOldArmor;
		DDWORD		m_nOldAmmo;
		DDWORD		m_nOldAltAmmo;

		// Inventory variables
		DDWORD		m_nItemNameX;
		DDWORD		m_nItemNameY;

		DFLOAT		m_fInvUpdateTime;
		DFLOAT		m_fInvScrollRatio;

		// Weapon display variables
		DFLOAT		m_fWeapUpdateTime;
		DFLOAT		m_fWeapScrollRatio;

		HSTRING		m_pWeapPics[WEAPTAB_SLOTS];
		HSTRING		m_pWeapPicsH[WEAPTAB_SLOTS];

		char		m_nCurrentWeapon;

		// Objectives display variables
		DFLOAT		m_fObjUpdateTime;
		DFLOAT		m_fObjScaleRatio;

		DRect		m_rObjectives;
		DDWORD		m_nObjectivesX;
		DDWORD		m_nObjectivesY;

		// Power bar variables
		DFLOAT		m_fPowerBarUpdate;

		DDWORD		m_nPowerBarX;
		DDWORD		m_nPowerBarY;

		// General information variables
		DDWORD		m_nScreenWidth;
		DDWORD		m_nScreenHeight;
		DBOOL		m_bHighGraphics;

		HDECOLOR	m_hTransColor;
		HDECOLOR	m_hMaskColor;
		HDECOLOR	m_hThrowColor;
		HDECOLOR	m_hAirColor;
		HDECOLOR	m_hBossColor;

		DFLOAT		m_fTime;

		int			m_nTeamIconHeight;
		int			m_nTeamID;
};

#endif	// __STATUSBAR_H__