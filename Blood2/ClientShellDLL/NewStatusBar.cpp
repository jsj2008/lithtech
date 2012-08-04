//*************************************************************************
//*************************************************************************
//***** MODULE  : StatusBar.cpp
//***** PURPOSE : Blood 2 Status Bar
//***** CREATED : 11/4/97
//***** MODIFIED: 3/30/98
//*************************************************************************
//*************************************************************************

#include "ClientRes.h"
#include "NewStatusBar.h"
#include "VKDefs.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"
#include <stdio.h>
#include <mbstring.h>
#include "SoundTypes.h"
#include "BloodClientShell.h"

//*************************************************************************

static float	g_ObjScaleUpX[OBJTAB_SCALE_LEVELS + 1] =
					{2.0f, 1.75f, 1.55f, 1.4f, 1.25f, 1.1f, 1.0f, 0.9f, 0.85f, 0.9f, 1.0f};
static float	g_ObjScaleUpY[OBJTAB_SCALE_LEVELS + 1] =
					{0.0f, 0.01f, 0.04f, 0.09f, 0.16f, 0.25f, 0.36f, 0.49f, 0.64f, 0.81f, 1.0f};
static float	g_ObjScaleDownX[OBJTAB_SCALE_LEVELS + 1] =
					{1.0f, 0.81f, 0.64f, 0.49f, 0.36f, 0.25f, 0.16f, 0.09f, 0.04f, 0.01f, 0.0f};
static float	g_ObjScaleDownY[OBJTAB_SCALE_LEVELS + 1] =
					{1.0f, 0.9f, 0.85f, 0.9f, 1.0f, 1.1f, 1.25f, 1.4f, 1.55f, 1.75f, 2.0f};
//					{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

//*************************************************************************
//*****	Function:	CNewStatusBar()
//*****	Details:	Constructor
//*************************************************************************

CNewStatusBar::CNewStatusBar()
{
	m_pClientDE = 0;

	// Surfaces
	m_hMask = 0;
	m_hItemName = 0;
	m_hObjectives = 0;
	m_hObjectivesIcon = 0;
	m_hPowerBar = 0;
	m_hHealthBar = 0;
	m_hThrowBar = 0;
	m_hFlagIcon = 0;
	m_hTeamIcon = 0;

	// Fonts and cursors
	m_pStatCursor = 0;
	m_pStatFont1 = 0;
	m_pStatNumbers1 = 0;
	m_pStatNumbers2 = 0;
	m_pObjectivesFont1 = 0;

	m_nStatLevel = STATTAB_HIGH;
	m_nOldStatLevel = STATTAB_HIGH;
	m_nInvLevel = INVTAB_INVISIBLE;
	m_nWeapLevel = WEAPTAB_INVISIBLE;
	m_nObjLevel = OBJTAB_INVISIBLE;
	m_nObjIcon = OBJICON_INVISIBLE;
	m_nPowerBar = BARTAB_NONE;
	m_nFlagLevel = MULTIFLAG_NONE;

	// Old values
	m_nOldHealth = -1;
	m_nOldArmor = -1;
	m_nOldAmmo = -1;
	m_nOldAltAmmo = -1;

	// Inventory
	m_nItemNameX = 0;
	m_nItemNameY = 0;

	m_fInvUpdateTime = 0.0f;
	m_fInvScrollRatio = 0.0f;

	// Weapons
	m_fWeapUpdateTime = 0.0f;
	m_fWeapScrollRatio = 0.0f;

	for(int i = 0; i < 10; i++)
	{
		m_pWeapPics[i] = 0;
		m_pWeapPicsH[i] = 0;
	}

	m_nCurrentWeapon = 0;

	// Objectives
	m_fObjUpdateTime = 0.0f;
	m_fObjScaleRatio = 0.0f;

	m_rObjectives.top = 0;
	m_rObjectives.left = 0;
	m_rObjectives.bottom = 0;
	m_rObjectives.right = 0;

	m_nObjectivesX = 100;
	m_nObjectivesY = 75;

	// Power bars
	m_fPowerBarUpdate = 0.0f;

	m_nPowerBarX = 0;
	m_nPowerBarY = 0;

	// General
	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_bHighGraphics = DTRUE;

	m_hTransColor = 0;
	m_hMaskColor = 0;
	m_hThrowColor = 0;
	m_hAirColor = 0;
	m_hBossColor = 0;

	m_fTime = 0.0f;

	m_nTeamIconHeight = 0;
	m_nTeamID         = -1;
}

//*************************************************************************
//*****	Function:	~CNewStatusBar()
//*****	Details:	Destructor
//*************************************************************************

CNewStatusBar::~CNewStatusBar()
{
	for(int i = 0; i < 10; i++)
	{
		if( m_pWeapPics[i] )
			g_pClientDE->FreeString( m_pWeapPics[i] );
		if( m_pWeapPicsH[i] )
			g_pClientDE->FreeString( m_pWeapPicsH[i] );
	}
}

//*************************************************************************
//*****	Function:	Init(CClientDE* pClientDE)
//*****	Details:	Initializes the status screens
//*************************************************************************

DBOOL CNewStatusBar::Init(CClientDE* pClientDE)
{
	if (!pClientDE) return DFALSE;
	Term();

	m_pClientDE = pClientDE;

	// Initialize the graphic surfaces
	m_hMask			= m_pClientDE->CreateSurfaceFromBitmap("interface/pixelmasks/mask25.pcx");
	m_hItemName		= m_pClientDE->CreateSurface(100, 14);
	m_hObjectives	= m_pClientDE->CreateSurfaceFromBitmap("interface/objectives/objectives.pcx");
	m_hObjectivesIcon = m_pClientDE->CreateSurfaceFromBitmap("interface/objectives/skull.pcx");
	m_hPowerBar		= m_pClientDE->CreateSurface(150, 16);
	m_hHealthBar	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/bar_health.pcx");
	m_hThrowBar		= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/bar_throw.pcx");

	// Create a font cursor
	m_pStatCursor = new CoolFontCursor();

	// Setup fonts
	m_pStatFont1 = new CoolFont();
	m_pStatFont1->Init(m_pClientDE, "interface/statusbar/fonts/stat_font_1.pcx");
	m_pStatFont1->LoadXWidths("interface/statusbar/fonts/stat_font_1.fnt");

	m_pStatNumbers1 = new CoolFont();
	m_pStatNumbers1->Init(m_pClientDE, "interface/statusbar/fonts/big_numbers.pcx");
	m_pStatNumbers1->LoadXWidths("interface/statusbar/fonts/big_numbers.fnt");

	m_pStatNumbers2 = new CoolFont();
	m_pStatNumbers2->Init(m_pClientDE, "interface/statusbar/fonts/small_numbers.pcx");
	m_pStatNumbers2->LoadXWidths("interface/statusbar/fonts/small_numbers.fnt");

	m_pObjectivesFont1 = new CoolFont();
	m_pObjectivesFont1->Init(m_pClientDE, "interface/objectives/obj_font_1.pcx");
	m_pObjectivesFont1->LoadXWidths("interface/objectives/obj_font_1.fnt");

	// Setup transparency color
	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	m_hMaskColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, DFALSE);
	m_hThrowColor = m_pClientDE->SetupColor1(1.0f, 1.0f, 0.0f, DFALSE);
	m_hAirColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 1.0f, DFALSE);
	m_hBossColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 0.0f, DFALSE);

	// Init tabs
	InitStatTabs();
	InitInvTabs();
	InitWeaponTabs();
	InitObjectivesTab();

	// Init tab locations
	AdjustRes();

	return DTRUE;
}

//*************************************************************************
//*****	Function:	AdjustRes()
//*****	Details:	Sets new locations for status tabs if the game resolution changes
//*************************************************************************

void CNewStatusBar::AdjustRes()
{
	if (!m_pClientDE) return;

	// Check to see if the screen resolution has changed... if not then return
	DDWORD	width, height;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);
	if((m_nScreenWidth == width) && (m_nScreenHeight == height))	return;

	// Set the screen width and height variables to the new resolution
	m_nScreenWidth = width;
	m_nScreenHeight = height;

	// If we are lower than the prefered res, default to LOW stat mode
	if((m_nScreenWidth < STATTAB_LOW_WIDTH) || (m_nScreenHeight < STATTAB_LOW_HEIGHT))
	{
		m_bHighGraphics = DFALSE;
		m_nStatLevel = STATTAB_LOW;
		m_nInvLevel = INVTAB_INVISIBLE;
		m_nWeapLevel = WEAPTAB_INVISIBLE;
	}
	else
	{
		m_bHighGraphics = DTRUE;
		m_nStatLevel = STATTAB_HIGH;
	}

	// Adjust the stat tab locations to the new resolution
	m_stHealth.locX		= 0;
	m_stHealth.locY		= m_nScreenHeight - m_stHealth.sizeY;

	if(m_nStatLevel == STATTAB_HIGH)	m_stArmor.locX		= m_stHealth.sizeX;
		else							m_stArmor.locX		= m_stHealth.sizeX / 2;
	m_stArmor.locY		= m_nScreenHeight - m_stArmor.sizeY;

	m_stAmmo.locX		= m_nScreenWidth - m_stAmmo.sizeX;
	m_stAmmo.locY		= m_nScreenHeight - m_stAmmo.sizeY;

	m_stAltAmmo.locX	= m_stAmmo.locX + (m_stAmmo.sizeX / 2) - (m_stAltAmmo.sizeX / 2);
	m_stAltAmmo.locY	= m_stAmmo.locY - m_stAltAmmo.sizeY - 3;

	// Adjust the inventory items tabs
	m_stItems[1].locX	= 0;
	m_stItems[1].locY	= (m_nScreenHeight / 2) - (m_stItems[1].sizeY / 2);
	m_stItems[0].locX	= -15;
	m_stItems[0].locY	= m_stItems[1].locY - m_stItems[1].sizeY;
	m_stItems[2].locX	= -15;
	m_stItems[2].locY	= m_stItems[1].locY + m_stItems[1].sizeY;

	// Adjust the current item tab locations to the new resolution
	m_stCurrentItem.locX= 20;
	m_stCurrentItem.locY= m_nScreenHeight - m_stHealth.sizeY - m_stCurrentItem.sizeY;

	if(m_bHighGraphics)
	{
		m_nItemNameX		= m_stItems[1].sizeX + 10;
		m_nItemNameY		= (m_nScreenHeight / 2) - (m_pStatFont1->height / 2);
	}
	else
	{
		m_nItemNameX		= m_stCurrentItem.locX + m_stCurrentItem.sizeX + 10;
		m_nItemNameY		= m_stCurrentItem.locY + (m_stCurrentItem.sizeY / 2) - (m_pStatFont1->height / 2);
	}

	// Adjust the weapon tab locations
	DDWORD	offset		= (m_nScreenHeight / 2) - (m_stWeapons[0].sizeY * 5);

	for(int i = 0; i < 10; i++)
	{
		m_stWeapons[i].locX		= m_nScreenWidth - m_stWeapons[i].sizeX;
		m_stWeapons[i].locY		= offset;
		offset += m_stWeapons[i].sizeY;
	}

	// Adjust the power bar location
	m_pClientDE->GetSurfaceDims(m_hPowerBar, &width, &height);
	m_nPowerBarX = (m_nScreenWidth - width) / 2;
	m_nPowerBarY = m_stAmmo.locY - height - height;
}

//*************************************************************************
//*****	Function:	Term()
//*****	Details:	Terminates the status screens
//*************************************************************************

void CNewStatusBar::Term()
{
	if(m_pClientDE)
	{
		if(m_hMask)			{ m_pClientDE->DeleteSurface(m_hMask); m_hMask = 0; }
		if(m_hItemName)		{ m_pClientDE->DeleteSurface(m_hItemName); m_hItemName = 0; }
		if(m_hObjectives)	{ m_pClientDE->DeleteSurface(m_hObjectives); m_hObjectives = 0; }
		if(m_hPowerBar)		{ m_pClientDE->DeleteSurface(m_hPowerBar); m_hPowerBar = 0; }
		if(m_hHealthBar)	{ m_pClientDE->DeleteSurface(m_hHealthBar); m_hHealthBar = 0; }
		if(m_hThrowBar)		{ m_pClientDE->DeleteSurface(m_hThrowBar); m_hThrowBar = 0; }
		if(m_hTeamIcon)		{ m_pClientDE->DeleteSurface(m_hTeamIcon); m_hTeamIcon = 0; }

		if(m_pStatCursor)	{ delete m_pStatCursor; m_pStatCursor = 0; }
		if(m_pStatFont1)	{ m_pStatFont1->Free(); delete m_pStatFont1; m_pStatFont1 = 0; }
		if(m_pStatNumbers1)	{ m_pStatNumbers1->Free(); delete m_pStatNumbers1; m_pStatNumbers1 = 0; }
		if(m_pStatNumbers2)	{ m_pStatNumbers2->Free(); delete m_pStatNumbers2; m_pStatNumbers2 = 0; }
		if(m_pObjectivesFont1) { m_pObjectivesFont1->Free(); delete m_pObjectivesFont1; m_pObjectivesFont1 = 0; }

		TermStatTabs();
		TermInvTabs();
		TermWeaponTabs();

		m_pClientDE = 0;
	}
}

//*************************************************************************
//*****	Function:	InitStatTabs()
//*****	Details:	Initializes the status tabs
//*************************************************************************

DBOOL CNewStatusBar::InitStatTabs()
{
	// Health status tab
	m_stHealth.sizeX	= 185;
	m_stHealth.sizeY	= 55;

	m_stHealth.hTab		= m_pClientDE->CreateSurface(m_stHealth.sizeX, m_stHealth.sizeY);
	m_stHealth.hBorder	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_health.pcx");
	m_stHealth.hIcon	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/icon_health.pcx");
	m_stHealth.hBar		= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/status_ticker.pcx");

	m_stHealth.borderX	= 10;
	m_stHealth.borderY	= 2;
	m_stHealth.borderJ	= STATTAB_JUST_NW;

	m_stHealth.iconX	= 32;
	m_stHealth.iconY	= 7;
	m_stHealth.iconJ	= STATTAB_JUST_NW;

	m_stHealth.barX		= 115;
	m_stHealth.barY		= 5;
	m_stHealth.barJ		= STATTAB_JUST_NW;

	m_stHealth.valueX	= 80;
	m_stHealth.valueY	= 10;
	m_stHealth.valueJ	= CF_JUSTIFY_CENTER;
	m_stHealth.digits	= 3;

	// Armor status tab
	m_stArmor.sizeX		= 200;
	m_stArmor.sizeY		= 55;

	m_stArmor.hTab		= m_pClientDE->CreateSurface(m_stArmor.sizeX, m_stArmor.sizeY);
	m_stArmor.hBorder	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_armor.pcx");
	m_stArmor.hIcon		= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/icon_armor.pcx");
	m_stArmor.hBar		= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/status_ticker.pcx");

	m_stArmor.borderX	= 0;
	m_stArmor.borderY	= 1;
	m_stArmor.borderJ	= STATTAB_JUST_NW;

	m_stArmor.iconX		= 45;
	m_stArmor.iconY		= 7;
	m_stArmor.iconJ		= STATTAB_JUST_NW;

	m_stArmor.barX		= 125;
	m_stArmor.barY		= 5;
	m_stArmor.barJ		= STATTAB_JUST_NW;

	m_stArmor.valueX	= 90;
	m_stArmor.valueY	= 10;
	m_stArmor.valueJ	= CF_JUSTIFY_CENTER;
	m_stArmor.digits	= 3;

	// Ammo status tab
	m_stAmmo.sizeX		= 135;
	m_stAmmo.sizeY		= 75;

	m_stAmmo.hTab		= m_pClientDE->CreateSurface(m_stAmmo.sizeX, m_stAmmo.sizeY);
	m_stAmmo.hBorder	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_ammo.pcx");
	m_stAmmo.hIcon		= 0;
	m_stAmmo.hBar		= 0;

	m_stAmmo.borderX	= 8;
	m_stAmmo.borderY	= 24;
	m_stAmmo.borderJ	= STATTAB_JUST_NW;

	m_stAmmo.iconX		= 63;
	m_stAmmo.iconY		= 0;
	m_stAmmo.iconJ		= STATTAB_JUST_N;

	m_stAmmo.valueX		= 63;
	m_stAmmo.valueY		= 32;
	m_stAmmo.valueJ		= CF_JUSTIFY_CENTER;
	m_stAmmo.digits		= 3;

	// Alt Ammo status tab
	m_stAltAmmo.sizeX		= 60;
	m_stAltAmmo.sizeY		= 20;

	m_stAltAmmo.hTab	= m_pClientDE->CreateSurface(m_stAmmo.sizeX, m_stAmmo.sizeY);
	m_stAltAmmo.hBorder	= 0;
	m_stAltAmmo.hIcon	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/weapons/raidcanister.pcx");
	m_stAltAmmo.hBar	= 0;

	m_stAltAmmo.borderX	= 0;
	m_stAltAmmo.borderY	= 0;
	m_stAltAmmo.borderJ	= 0;

	m_stAltAmmo.iconX	= 0;
	m_stAltAmmo.iconY	= 0;
	m_stAltAmmo.iconJ	= STATTAB_JUST_NW;

	m_stAltAmmo.valueX	= 35;
	m_stAltAmmo.valueY	= 3;
	m_stAltAmmo.valueJ	= CF_JUSTIFY_LEFT;
	m_stAltAmmo.digits	= 3;

	return DTRUE;
}

//*************************************************************************
//*****	Function:	InitInvTabs()
//*****	Details:	Initializes the inventory tabs
//*************************************************************************

DBOOL CNewStatusBar::InitInvTabs()
{
	// Prev, current, and next items
	for(int i = 0; i < 3; i++)
	{
		m_stItems[i].sizeX		= 75;
		m_stItems[i].sizeY		= 50;

		m_stItems[i].hTab		= m_pClientDE->CreateSurface(m_stItems[i].sizeX, m_stItems[i].sizeY);
		m_stItems[i].hBorder	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_inv.pcx");
		m_stItems[i].hIcon		= 0;
		m_stItems[i].hBar		= 0;

		m_stItems[i].borderX	= 0;
		m_stItems[i].borderY	= 4;
		m_stItems[i].borderJ	= STATTAB_JUST_NW;

		m_stItems[i].iconX		= 50;
		m_stItems[i].iconY		= 18;
		m_stItems[i].iconJ		= STATTAB_JUST_C;

		m_stItems[i].valueX		= 50;
		m_stItems[i].valueY		= 29;
		m_stItems[i].valueJ		= CF_JUSTIFY_CENTER;
		m_stItems[i].digits		= 3;
	}

	// Currently selected item
	m_stCurrentItem.sizeX		= 50;
	m_stCurrentItem.sizeY		= 45;

	m_stCurrentItem.hTab		= m_pClientDE->CreateSurface(m_stCurrentItem.sizeX, m_stCurrentItem.sizeY);
	m_stCurrentItem.hBorder		= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_current.pcx");
	m_stCurrentItem.hIcon		= 0;
	m_stCurrentItem.hBar		= 0;

	m_stCurrentItem.borderX		= 2;
	m_stCurrentItem.borderY		= 0;
	m_stCurrentItem.borderJ		= STATTAB_JUST_NW;

	m_stCurrentItem.iconX		= 25;
	m_stCurrentItem.iconY		= 14;
	m_stCurrentItem.iconJ		= STATTAB_JUST_C;

	m_stCurrentItem.valueX		= 25;
	m_stCurrentItem.valueY		= 25;
	m_stCurrentItem.valueJ		= CF_JUSTIFY_CENTER;
	m_stCurrentItem.digits		= 3;

	return DTRUE;
}

//*************************************************************************
//*****	Function:	InitWeaponTabs()
//*****	Details:	Initializes the weapon tabs
//*************************************************************************

DBOOL CNewStatusBar::InitWeaponTabs()
{
	// Prev, current, and next items
	for(int i = 0; i < 10; i++)
	{
		m_stWeapons[i].sizeX	= 150;
		m_stWeapons[i].sizeY	= 25;

		m_stWeapons[i].hTab		= m_pClientDE->CreateSurface(m_stWeapons[i].sizeX, m_stWeapons[i].sizeY);
		m_stWeapons[i].hBorder	= m_pClientDE->CreateSurfaceFromBitmap("interface/statusbar/border_weapon.pcx");
		m_stWeapons[i].hIcon	= 0;
		m_stWeapons[i].hBar		= 0;

		m_stWeapons[i].borderX	= 122;
		m_stWeapons[i].borderY	= 3;
		m_stWeapons[i].borderJ	= STATTAB_JUST_NW;

		m_stWeapons[i].iconX	= 125;
		m_stWeapons[i].iconY	= 4;
		m_stWeapons[i].iconJ	= STATTAB_JUST_NE;

		m_stWeapons[i].value	= i + 1;
		m_stWeapons[i].valueX	= 150;
		m_stWeapons[i].valueY	= 0;
		m_stWeapons[i].valueJ	= CF_JUSTIFY_RIGHT;
		m_stWeapons[i].digits	= 2;

		SetupTab(m_stWeapons[i], m_pStatNumbers2, TAB_INC_HIGH);
	}

	return DTRUE;
}

//*************************************************************************
//*****	Function:	InitObjectivesTab()
//*****	Details:	Initializes the objectives to some general text
//*************************************************************************

DBOOL CNewStatusBar::InitObjectivesTab()
{
	if(!m_pClientDE || !m_hObjectives)	return DFALSE;

	DDWORD		width, height;

	// Reset the background picture of the objectives
	m_pClientDE->DrawBitmapToSurface(m_hObjectives, "interface/objectives/objectives.pcx", DNULL, 0, 0);
	m_pClientDE->GetSurfaceDims(m_hObjectives, &width, &height);

	// Setup the font and print it out on the surface
	m_pStatCursor->SetFont(m_pObjectivesFont1);
	m_pStatCursor->SetDest(m_hObjectives);

	m_pStatCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pStatCursor->SetLoc(OBJTAB_TITLE_CENTER, OBJTAB_TITLE_TOP);
	m_pStatCursor->Draw("No Current Objective");

	return DTRUE;
}

//*************************************************************************
//*****	Function:	TermStatTabs()
//*****	Details:	Removes the stat tabs
//*************************************************************************

void CNewStatusBar::TermStatTabs()
{
	if(m_stHealth.hTab)		{ m_pClientDE->DeleteSurface(m_stHealth.hTab); m_stHealth.hTab = 0; }
	if(m_stHealth.hBorder)	{ m_pClientDE->DeleteSurface(m_stHealth.hBorder); m_stHealth.hBorder = 0; }
	if(m_stHealth.hIcon)	{ m_pClientDE->DeleteSurface(m_stHealth.hIcon); m_stHealth.hIcon = 0; }
	if(m_stHealth.hBar)		{ m_pClientDE->DeleteSurface(m_stHealth.hBar); m_stHealth.hBar = 0; }

	if(m_stArmor.hTab)		{ m_pClientDE->DeleteSurface(m_stArmor.hTab); m_stArmor.hTab = 0; }
	if(m_stArmor.hBorder)	{ m_pClientDE->DeleteSurface(m_stArmor.hBorder); m_stArmor.hBorder = 0; }
	if(m_stArmor.hIcon)		{ m_pClientDE->DeleteSurface(m_stArmor.hIcon); m_stArmor.hIcon = 0; }
	if(m_stArmor.hBar)		{ m_pClientDE->DeleteSurface(m_stArmor.hBar); m_stArmor.hBar = 0; }

	if(m_stAmmo.hTab)		{ m_pClientDE->DeleteSurface(m_stAmmo.hTab); m_stAmmo.hTab = 0; }
	if(m_stAmmo.hBorder)	{ m_pClientDE->DeleteSurface(m_stAmmo.hBorder); m_stAmmo.hBorder = 0; }
	if(m_stAmmo.hIcon)		{ m_pClientDE->DeleteSurface(m_stAmmo.hIcon); m_stAmmo.hIcon = 0; }
	if(m_stAmmo.hBar)		{ m_pClientDE->DeleteSurface(m_stAmmo.hBar); m_stAmmo.hBar = 0; }

	if(m_stAltAmmo.hTab)	{ m_pClientDE->DeleteSurface(m_stAltAmmo.hTab); m_stAltAmmo.hTab = 0; }
	if(m_stAltAmmo.hBorder)	{ m_pClientDE->DeleteSurface(m_stAltAmmo.hBorder); m_stAltAmmo.hBorder = 0; }
	if(m_stAltAmmo.hIcon)	{ m_pClientDE->DeleteSurface(m_stAltAmmo.hIcon); m_stAltAmmo.hIcon = 0; }
	if(m_stAltAmmo.hBar)	{ m_pClientDE->DeleteSurface(m_stAltAmmo.hBar); m_stAltAmmo.hBar = 0; }
}

//*************************************************************************
//*****	Function:	TermInvTabs()
//*****	Details:	Removes the inventory tabs
//*************************************************************************

void CNewStatusBar::TermInvTabs()
{
	for(int i = 0; i < 3; i++)
	{
		if(m_stItems[i].hTab)	{ m_pClientDE->DeleteSurface(m_stItems[i].hTab); m_stItems[i].hTab = 0; }
		if(m_stItems[i].hBorder){ m_pClientDE->DeleteSurface(m_stItems[i].hBorder); m_stItems[i].hBorder = 0; }
		if(m_stItems[i].hIcon)	{ m_pClientDE->DeleteSurface(m_stItems[i].hIcon); m_stItems[i].hIcon = 0; }
		if(m_stItems[i].hBar)	{ m_pClientDE->DeleteSurface(m_stItems[i].hBar); m_stItems[i].hBar = 0; }
	}

	if(m_stCurrentItem.hTab)	{ m_pClientDE->DeleteSurface(m_stCurrentItem.hTab); m_stCurrentItem.hTab = 0; }
	if(m_stCurrentItem.hBorder)	{ m_pClientDE->DeleteSurface(m_stCurrentItem.hBorder); m_stCurrentItem.hBorder = 0; }
	if(m_stCurrentItem.hIcon)	{ m_pClientDE->DeleteSurface(m_stCurrentItem.hIcon); m_stCurrentItem.hIcon = 0; }
	if(m_stCurrentItem.hBar)	{ m_pClientDE->DeleteSurface(m_stCurrentItem.hBar); m_stCurrentItem.hBar = 0; }
}

//*************************************************************************
//*****	Function:	TermWeaponTabs()
//*****	Details:	Removes the weapon tabs
//*************************************************************************

void CNewStatusBar::TermWeaponTabs()
{
	for(int i = 0; i < 10; i++)
	{
		if(m_stWeapons[i].hTab)		{ m_pClientDE->DeleteSurface(m_stWeapons[i].hTab); m_stWeapons[i].hTab = 0; }
		if(m_stWeapons[i].hBorder)	{ m_pClientDE->DeleteSurface(m_stWeapons[i].hBorder); m_stWeapons[i].hBorder = 0; }
		if(m_stWeapons[i].hIcon)	{ m_pClientDE->DeleteSurface(m_stWeapons[i].hIcon); m_stWeapons[i].hIcon = 0; }
		if(m_stWeapons[i].hBar)		{ m_pClientDE->DeleteSurface(m_stWeapons[i].hBar); m_stWeapons[i].hBar = 0; }
	}
}

//*************************************************************************
//*****	Function:	UpdateInv()
//*****	Details:	Changes the inventory items to the new prev, cur, and next
//*************************************************************************

void CNewStatusBar::UpdateInv(DBOOL bShow, HSTRING name, HSTRING c, HSTRING cH, DDWORD cN, HSTRING p, DDWORD pN, HSTRING n, DDWORD nN)
{
	if(!m_pClientDE) return;

	char	string1[100] = "interface/statusbar/items/";
	char	string2[100];

	// Fill in the name of the current object
	m_pClientDE->FillRect(m_hItemName, DNULL, m_hTransColor);

	if(name)
	{
		m_pStatCursor->SetFont(m_pStatFont1);
		m_pStatCursor->SetDest(m_hItemName);
		m_pStatCursor->SetJustify(CF_JUSTIFY_LEFT);
		m_pStatCursor->SetLoc(0, 0);
		m_pStatCursor->Draw(m_pClientDE->GetStringData(name));
	}

	// Currently selected item
	if(m_stCurrentItem.hIcon)
	{
		m_pClientDE->DeleteSurface(m_stCurrentItem.hIcon);
		m_stCurrentItem.hIcon = 0;
	}

	if(c)
	{
		_mbscpy((unsigned char*)string2, (const unsigned char*)string1);
		_mbscat((unsigned char*)string2, (const unsigned char*)m_pClientDE->GetStringData(c));

		m_stCurrentItem.hIcon = m_pClientDE->CreateSurfaceFromBitmap(string2);
		m_stCurrentItem.value = cN;
	}

	// Currently highlighted item in the inv tabs
	if(m_stItems[1].hIcon)
	{
		m_pClientDE->DeleteSurface(m_stItems[1].hIcon);
		m_stItems[1].hIcon = 0;
	}

	if(cH)
	{
		_mbscpy((unsigned char*)string2, (const unsigned char*)string1);
		_mbscat((unsigned char*)string2, (const unsigned char*)m_pClientDE->GetStringData(cH));

		m_stItems[1].hIcon = m_pClientDE->CreateSurfaceFromBitmap(string2);
		m_stItems[1].value = cN;
	}

	// Previous item in the inv tabs
	if(m_stItems[0].hIcon)
	{
		m_pClientDE->DeleteSurface(m_stItems[0].hIcon);
		m_stItems[0].hIcon = 0;
	}

	if(p)
	{
		_mbscpy((unsigned char*)string2, (const unsigned char*)string1);
		_mbscat((unsigned char*)string2, (const unsigned char*)m_pClientDE->GetStringData(p));

		m_stItems[0].hIcon = m_pClientDE->CreateSurfaceFromBitmap(string2);
		m_stItems[0].value = pN;
	}

	// Next item in the inv tabs
	if(m_stItems[2].hIcon)
	{
		m_pClientDE->DeleteSurface(m_stItems[2].hIcon);
		m_stItems[2].hIcon = 0;
	}

	if(n)
	{
		_mbscpy((unsigned char*)string2, (const unsigned char*)string1);
		_mbscat((unsigned char*)string2, (const unsigned char*)m_pClientDE->GetStringData(n));

		m_stItems[2].hIcon = m_pClientDE->CreateSurfaceFromBitmap(string2);
		m_stItems[2].value = nN;
	}

	
	// Update the time so it stays on the screen for a while
	switch(m_nInvLevel)
	{
		case	INVTAB_INVISIBLE:
			if( bShow )
			{
				m_nInvLevel = INVTAB_SCROLL_IN;
				m_fInvUpdateTime = m_pClientDE->GetTime();
			}
			break;

		case	INVTAB_SCROLL_IN:
			break;

		case	INVTAB_STOPPED:
			m_fInvUpdateTime = m_pClientDE->GetTime();
			break;

		case	INVTAB_SCROLL_OUT:
			m_nInvLevel = INVTAB_SCROLL_IN;
			m_fInvUpdateTime = m_pClientDE->GetTime();
			break;
	}

	// Setup the items tabs with the new information
	for(int i = 0; i < 3; i++)
		if(m_stItems[i].hIcon)
			SetupTab(m_stItems[i], m_pStatFont1, TAB_INC_HIGH);
}

//*************************************************************************
//*****	Function:	ToggleWeapons()
//*****	Details:	Toggles the weapon view
//*************************************************************************

void CNewStatusBar::ToggleWeapons()
{
	if(m_nWeapLevel == WEAPTAB_INVISIBLE)
	{
		m_nWeapLevel = WEAPTAB_SCROLL_IN;
		m_fWeapUpdateTime = m_pClientDE->GetTime();
	}
	else if(m_nWeapLevel == WEAPTAB_STOPPED)
	{
		m_nWeapLevel = WEAPTAB_SCROLL_OUT;
		m_fWeapUpdateTime = m_pClientDE->GetTime();
	}
}

//*************************************************************************
//*****	Function:	UpdateWeap()
//*****	Details:	Changes the weapons to the latest one
//*************************************************************************

void CNewStatusBar::UpdateWeap(HSTRING weap, HSTRING weapH, char slot)
{
	if(!m_pClientDE) return;

	// Delete the old strings
	if(m_pWeapPics[slot])	{ m_pClientDE->FreeString(m_pWeapPics[slot]); m_pWeapPics[slot] = 0; }
	if(m_pWeapPicsH[slot])	{ m_pClientDE->FreeString(m_pWeapPicsH[slot]); m_pWeapPicsH[slot] = 0; }

	// Update the string arrays for the pictures
	if(weap)	m_pWeapPics[slot] = m_pClientDE->CopyString(weap);
	if(weapH)	m_pWeapPicsH[slot] = m_pClientDE->CopyString(weapH);

	// Delete the icon if there is one (there shouldn't be)
	if(m_stWeapons[slot].hIcon)
	{
		m_pClientDE->DeleteSurface(m_stWeapons[slot].hIcon);
		m_stWeapons[slot].hIcon = 0;
	}

	// Set the new icon for this slot
	if(weap)
		m_stWeapons[slot].hIcon = m_pClientDE->CreateSurfaceFromBitmap(m_pClientDE->GetStringData(weap));

	SetupTab(m_stWeapons[slot], m_pStatNumbers2, TAB_INC_HIGH);
}

//*************************************************************************
//*****	Function:	UpdateCurrentWeap()
//*****	Details:	Changes the weapons to the latest one
//*************************************************************************

void CNewStatusBar::UpdateCurrentWeap(char slot)
{
	if((slot < 0) || (slot >= WEAPTAB_SLOTS))
	{
		// Clear out the weapon picture, cause it's probably an inventory weapon
		m_pClientDE->DeleteSurface(m_stAmmo.hIcon);
		m_stAmmo.hIcon = 0;
		m_nOldAmmo = -1;
		m_nOldAltAmmo = -1;
		return;
	}

	// Change the icon above the ammo to the current weapon
	if(m_stAmmo.hIcon)
	{
		m_pClientDE->DeleteSurface(m_stAmmo.hIcon);
		m_stAmmo.hIcon = 0;
	}

	if(m_pWeapPics[slot])
	{
		m_stAmmo.hIcon = m_pClientDE->CreateSurfaceFromBitmap(m_pClientDE->GetStringData(m_pWeapPics[slot]));
		m_nOldAmmo = -1;
		m_nOldAltAmmo = -1;
	}

	// Change the currently highlighted icon back to normal
	if(m_pWeapPics[m_nCurrentWeapon])
	{
		if(m_stWeapons[m_nCurrentWeapon].hIcon)
		{
			m_pClientDE->DeleteSurface(m_stWeapons[m_nCurrentWeapon].hIcon);
			m_stWeapons[m_nCurrentWeapon].hIcon = 0;
		}

		m_stWeapons[m_nCurrentWeapon].hIcon = m_pClientDE->CreateSurfaceFromBitmap(m_pClientDE->GetStringData(m_pWeapPics[m_nCurrentWeapon]));
		SetupTab(m_stWeapons[m_nCurrentWeapon], m_pStatNumbers2, TAB_INC_HIGH);
	}

	// Set the new current weapon to the highlighted icon
	if(m_pWeapPicsH[slot])
	{
		if(m_stWeapons[slot].hIcon)
		{
			m_pClientDE->DeleteSurface(m_stWeapons[slot].hIcon);
			m_stWeapons[slot].hIcon = 0;
		}

		m_stWeapons[slot].hIcon = m_pClientDE->CreateSurfaceFromBitmap(m_pClientDE->GetStringData(m_pWeapPicsH[slot]));
		SetupTab(m_stWeapons[slot], m_pStatNumbers2, TAB_INC_HIGH);
	}

	// Set the current weapon to the new weapon
	m_nCurrentWeapon = slot;
}

//*************************************************************************
//*****	Function:	StatLevelUp()
//*****	Details:	Increases the stat display level
//*************************************************************************

void CNewStatusBar::StatLevelUp()
{
	if(!m_bHighGraphics && (m_nStatLevel == STATTAB_LOW))	return;
	if(m_nStatLevel < STATTAB_HIGH) m_nStatLevel++;
}

//*************************************************************************
//*****	Function:	StatLevelDown()
//*****	Details:	Decreases the stat display level
//*************************************************************************

void CNewStatusBar::StatLevelDown()
{
	if(m_nStatLevel) m_nStatLevel--;
}

//*************************************************************************
//*****	Function:	SetStatLevel()
//*****	Details:	Sets the stat display level
//*************************************************************************

void CNewStatusBar::SetStatLevel(DBYTE nLevel)
{
	if((nLevel > STATTAB_LOW) && !m_bHighGraphics)	return;
	m_nStatLevel = nLevel;
}

//*************************************************************************
//*****	Function:	ToggleObjectives()
//*****	Details:	Turns the objectives screen on and off
//*************************************************************************

void CNewStatusBar::ToggleObjectives()
{
	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT(playSoundInfo);
	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;
	playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"Sounds\\Interface\\MainMenus\\missionobj.wav", sizeof(playSoundInfo.m_szSoundName) - 1);

	if(m_nObjLevel == OBJTAB_INVISIBLE)
	{
		m_nObjLevel = OBJTAB_SCALE_UP;
		m_fObjUpdateTime = m_pClientDE->GetTime();
		m_nObjIcon = OBJICON_INVISIBLE;
		m_pClientDE->PlaySound (&playSoundInfo);
	}
	else if(m_nObjLevel == OBJTAB_STOPPED)
	{
		m_nObjLevel = OBJTAB_SCALE_DOWN;
		m_fObjUpdateTime = m_pClientDE->GetTime();
		m_pClientDE->PlaySound (&playSoundInfo);
	}
}

//*************************************************************************
//*****	Function:	UpdateObjectives()
//*****	Details:	Change the current objective
//*************************************************************************

void CNewStatusBar::UpdateObjectives(char *title, char *string)
{
	if(!m_pClientDE || !m_hObjectives)	return;

	// If we send in a null title or string, reset the objectives
	if(!title || !string)
	{
		InitObjectivesTab();
		m_nObjIcon = OBJICON_INVISIBLE;
		return;
	}

	// If we're not in the right game type for objectives, just return
	if (g_pBloodClientShell->GetGameType() != GAMETYPE_SINGLE && g_pBloodClientShell->GetGameType() != GAMETYPE_CUSTOM)
	{
		return;
	}

	// Reset the background picture of the objectives
	DDWORD width, height;
	m_pClientDE->DrawBitmapToSurface(m_hObjectives, "interface/objectives/objectives.pcx", DNULL, 0, 0);
	m_pClientDE->GetSurfaceDims(m_hObjectives, &width, &height);

	// Setup the font and print it out on the surface
	m_pStatCursor->SetFont(m_pObjectivesFont1);
	m_pStatCursor->SetDest(m_hObjectives);

	m_pStatCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pStatCursor->SetLoc(OBJTAB_TITLE_CENTER, OBJTAB_TITLE_TOP);
	m_pStatCursor->Draw(title);

	m_pStatCursor->SetJustify(CF_JUSTIFY_LEFT);
	m_pStatCursor->SetLoc(OBJTAB_TEXT_LEFT, OBJTAB_TEXT_TOP);
	m_pStatCursor->DrawFormat(string, OBJTAB_TEXT_WIDTH);

	// Turn on the updated icon if we're not currently displaying the objectives
	if(m_nObjLevel == OBJTAB_INVISIBLE || m_nObjLevel == OBJTAB_SCALE_DOWN)
		m_nObjIcon = OBJICON_VISIBLE;
	else
		m_nObjIcon = OBJICON_INVISIBLE;

	// Play a sound to let the player know that the objectives have changed
	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT(playSoundInfo);
	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;
	playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;
	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)"Sounds\\Events\\bellding1.wav", sizeof(playSoundInfo.m_szSoundName) - 1);

	m_pClientDE->PlaySound(&playSoundInfo);
}

//*************************************************************************
//*****	Function:	SetPowerBarLevel()
//*****	Details:	Set the air bar level to a certain percent
//*************************************************************************

void CNewStatusBar::SetPowerBarLevel(DBYTE nBar, DFLOAT fPercent)
{
	if(!m_pClientDE || !m_hPowerBar)	return;

	HSURFACE	bar = 0;
	HDECOLOR	color = 0;
	DDWORD		width, height, centerX, newX;
	DRect		fillArea;

	m_nPowerBar = nBar;
	m_fPowerBarUpdate = m_pClientDE->GetTime();

	m_pClientDE->FillRect(m_hPowerBar, DNULL, m_hTransColor);
	m_pClientDE->GetSurfaceDims(m_hPowerBar, &width, &height);

	centerX = width / 2;

	fillArea.top = 0;
	fillArea.bottom = height;

	switch(m_nPowerBar)
	{
		case	BARTAB_THROW:	bar = m_hThrowBar; color = m_hThrowColor;		break;
		case	BARTAB_AIR:		bar = m_hThrowBar; color = m_hAirColor;			break;
		case	BARTAB_BOSS:	bar = m_hHealthBar; color = m_hBossColor;		break;
	}

	if(bar && color)
	{
		m_pClientDE->GetSurfaceDims(bar, &width, &height);

		newX = centerX - (width / 2);
		fillArea.left = newX;
		fillArea.right = newX + (int)(width * fPercent);

		m_pClientDE->FillRect(m_hPowerBar, &fillArea, color);
		m_pClientDE->DrawSurfaceToSurfaceTransparent(m_hPowerBar, bar, DNULL, newX, 0, m_hMaskColor);
	}
}

//*************************************************************************
//*****	Function:	DisplayFlag()
//*****	Details:	Sets up the flag icon
//*************************************************************************

void CNewStatusBar::DisplayFlag(DBYTE bColor)
{
	if(!m_pClientDE)	return;

	if(m_hFlagIcon)
	{
		m_pClientDE->DeleteSurface(m_hFlagIcon);
		m_hFlagIcon = 0;
	}

	m_nFlagLevel = bColor;

	switch(m_nFlagLevel)
	{
		case	MULTIFLAG_NONE:
			break;

		case	MULTIFLAG_RED:
			m_hFlagIcon = m_pClientDE->CreateSurfaceFromBitmap("interface_multipatch\\flagred.pcx");
			break;

		case	MULTIFLAG_BLUE:
			m_hFlagIcon = m_pClientDE->CreateSurfaceFromBitmap("interface_multipatch\\flagblue.pcx");
			break;
	}
}

//*************************************************************************
//*****	Function:	SetTeamID()
//*****	Details:	Sets up the team icon
//*************************************************************************

void CNewStatusBar::SetTeamID(int nTeamID)
{
	// Check if we are already using this team id...

	if (m_hTeamIcon && (nTeamID == m_nTeamID))
	{
		return;
	}


	// Set the new team id and icon...

	m_nTeamIconHeight = 0;

	if(!m_pClientDE) return;

	if (m_hTeamIcon)
	{
		m_pClientDE->DeleteSurface(m_hTeamIcon);
		m_hTeamIcon = 0;
	}

	m_nTeamID = nTeamID;

	switch(nTeamID)
	{
		case TEAM_1:
		{
			m_hTeamIcon = m_pClientDE->CreateSurfaceFromBitmap("interface_multipatch\\teamblue.pcx");
			break;
		}

		case TEAM_2:
		{
			m_hTeamIcon = m_pClientDE->CreateSurfaceFromBitmap("interface_multipatch\\teamred.pcx");
			break;
		}
	}
}

//*************************************************************************
//*****	Function:	Draw()
//*****	Details:	Draws all active in-game interfaces
//*************************************************************************

void CNewStatusBar::Draw(DBOOL bDrawBar)
{
	if(!m_pClientDE) return;

	m_fTime = m_pClientDE->GetTime();

	AdjustRes();

	if(bDrawBar)
	{
		if(m_nStatLevel)						DrawStatTabs();
		if(m_nInvLevel && m_bHighGraphics)		DrawInvTabs();
		if(m_nWeapLevel && m_bHighGraphics)		DrawWeaponTabs();
		if(m_nObjLevel)							DrawObjectives();
		if(m_nObjIcon)							DrawObjectivesIcon();
		if(m_nPowerBar)							DrawPowerBar();
		if(m_nFlagLevel)						DrawFlagIcon();
		if(m_hTeamIcon)							DrawTeamIcon();
	}
}

//*************************************************************************
//*****	Function:	Reset()
//*****	Details:	Reset all the status bar stuff when switching levels
//*************************************************************************

void CNewStatusBar::Reset()
{
	m_stHealth.value = 100;
	m_stArmor.value = 0;
	m_stAmmo.value = 0;
	m_stAltAmmo.value = 0;

	m_nOldHealth = -1;
	m_nOldArmor = -1;
	m_nOldAmmo = -1;
	m_nOldAltAmmo = -1;

	m_nInvLevel = 0;
	m_nWeapLevel = 0;
	m_nObjLevel = 0;
	m_nObjIcon = 0;
	m_nPowerBar = 0;

	for(int i = 0; i < 10; i++)
		UpdateWeap(0, 0, i);

	UpdateInv(DFALSE, 0, 0, 0, 0, 0, 0, 0, 0);

	DisplayFlag(MULTIFLAG_NONE);
}


//*************************************************************************
//*****	Function:	SetupTab()
//*****	Details:	Setup the tab with the current graphics and values
//*************************************************************************

void CNewStatusBar::SetupTab(StatTab &stTab, CoolFont *font, DBYTE nFlags)
{
	if(!m_pClientDE) return;

	DDWORD		x, y;

	// Clear the surface to the transparent color
	if(stTab.hTab)
		m_pClientDE->FillRect(stTab.hTab, DNULL, m_hTransColor);

	// Draw the border in the offset position
	if(stTab.hBorder && (nFlags & TAB_INC_BORDER))
	{
		x = stTab.borderX;
		y = stTab.borderY;
		JustifySurface(stTab.hBorder, stTab.borderJ, x, y);
		m_pClientDE->DrawSurfaceToSurface(stTab.hTab, stTab.hBorder, DNULL, x, y);
	}

	// Draw the icon in the offset position
	if(stTab.hIcon && (nFlags & TAB_INC_ICON))
	{
		x = stTab.iconX;
		y = stTab.iconY;
		JustifySurface(stTab.hIcon, stTab.iconJ, x, y);
		m_pClientDE->DrawSurfaceToSurfaceTransparent(stTab.hTab, stTab.hIcon, DNULL, x, y, m_hTransColor);
	}

	// If there is a bar, draw the portion of it that should be drawn
	if(stTab.hBar && (nFlags & TAB_INC_BAR))
	{
		DRect	rect;
		DDWORD	width, height;

		m_pClientDE->GetSurfaceDims(stTab.hBar, &width, &height);

		rect.top	= 0;
		rect.left	= 0;
		rect.bottom	= height;
		rect.right	= (int)(width * ((float)stTab.value / (float)stTab.max));
		
		m_pClientDE->DrawSurfaceToSurfaceTransparent(stTab.hTab, stTab.hBar, &rect, stTab.barX, stTab.barY, m_hTransColor);
	}

	// Draw the value if needed
	if(stTab.digits && (nFlags & TAB_INC_VALUE))
	{
		char	*value;
		value = new char[stTab.digits + 1];
		memset(value, 0, stTab.digits + 1);

		m_pStatCursor->SetFont(font);
		m_pStatCursor->SetDest(stTab.hTab);
		m_pStatCursor->SetJustify(stTab.valueJ);
		m_pStatCursor->SetLoc((short)stTab.valueX, (short)stTab.valueY);

		sprintf(value, "%d", stTab.value);
		m_pStatCursor->Draw(value);
		delete value;
		value = 0;
	}
}

//*************************************************************************
//*****	Function:	JustifySurface()
//*****	Details:	Get new coordinates for the surface justification
//*************************************************************************

void CNewStatusBar::JustifySurface(HSURFACE surf, DDWORD just, DDWORD &x, DDWORD &y)
{
	if(!m_pClientDE) return;

	DDWORD	width, height;
	m_pClientDE->GetSurfaceDims(surf, &width, &height);

	switch(just)
	{
		case	STATTAB_JUST_NW:											return;
		case	STATTAB_JUST_N:		x -= (width / 2);						return;
		case	STATTAB_JUST_NE:	x -= width;								return;
		case	STATTAB_JUST_E:		x -= width;			y -= (height / 2);	return;
		case	STATTAB_JUST_SE:	x -= width;			y -= height;		return;
		case	STATTAB_JUST_S:		x -= (width / 2);	y -= height;		return;
		case	STATTAB_JUST_SW:						y -= height;		return;
		case	STATTAB_JUST_W:							y -= (height / 2);	return;
		case	STATTAB_JUST_C:		x -= (width / 2);	y -= (height / 2);	return;
		default:															return;
	}
}

//*************************************************************************
//*****	Function:	DrawStatTabs()
//*****	Details:	Draws the stat tabs at the bottom of the screen
//*************************************************************************

void CNewStatusBar::DrawStatTabs()
{
	if(!m_pClientDE) return;

	HSURFACE hScreen	= m_pClientDE->GetScreenSurface();
	DBYTE nInclude		= m_nStatLevel > STATTAB_LOW ? TAB_INC_HIGH : TAB_INC_LOW;

	// Resetup the tabs if the values have changed
	if(m_stHealth.value != m_nOldHealth || m_nStatLevel != m_nOldStatLevel)
	{
		SetupTab(m_stHealth, m_pStatNumbers1, nInclude);
		m_nOldHealth = m_stHealth.value;
	}

	if(m_stArmor.value != m_nOldArmor || m_nStatLevel != m_nOldStatLevel)
	{
		SetupTab(m_stArmor, m_pStatNumbers1, nInclude);
		m_nOldArmor = m_stArmor.value;
	}

	if(m_stAmmo.value != m_nOldAmmo || m_nStatLevel != m_nOldStatLevel)
	{
		SetupTab(m_stAmmo, m_pStatNumbers1, nInclude);
		m_nOldAmmo = m_stAmmo.value;
	}

	if(m_stAltAmmo.value != m_nOldAltAmmo || m_nStatLevel != m_nOldStatLevel)
	{
		SetupTab(m_stAltAmmo, m_pStatNumbers2, nInclude);
		m_nOldAltAmmo = m_stAltAmmo.value;
	}

	m_nOldStatLevel = m_nStatLevel;

	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stHealth.hTab, DNULL, m_stHealth.locX, m_stHealth.locY, m_hTransColor);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stArmor.hTab, DNULL, m_stArmor.locX, m_stArmor.locY, m_hTransColor);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stAmmo.hTab, DNULL, m_stAmmo.locX, m_stAmmo.locY, m_hTransColor);

	if(m_stAltAmmo.value)
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stAltAmmo.hTab, DNULL, m_stAltAmmo.locX, m_stAltAmmo.locY, m_hTransColor);

	// Draw the currently selected inventory item along with the normal stats
	if(m_stCurrentItem.hIcon)
	{
		SetupTab(m_stCurrentItem, m_pStatFont1, TAB_INC_HIGH);
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stCurrentItem.hTab, DNULL, m_stCurrentItem.locX, m_stCurrentItem.locY, m_hTransColor);

		if(!m_bHighGraphics)
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hItemName, DNULL, m_nItemNameX, m_nItemNameY, m_hTransColor);
	}
}

//*************************************************************************
//*****	Function:	DrawInvTabs()
//*****	Details:	Draws the three item tabs
//*************************************************************************

void CNewStatusBar::DrawInvTabs()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();

	// Handle the scrolling in and out of the inventory item tabs
	switch(m_nInvLevel)
	{
		case	INVTAB_SCROLL_IN:
			if(m_fTime - m_fInvUpdateTime > INVTAB_SCROLL_TIME)
			{
				m_fInvScrollRatio = 1.0f;
				m_nInvLevel = INVTAB_STOPPED;
				m_fInvUpdateTime = m_fTime;
			}
			else
				m_fInvScrollRatio = (m_fTime - m_fInvUpdateTime) / INVTAB_SCROLL_TIME;
			break;

		case	INVTAB_STOPPED:
			if(m_fTime - m_fInvUpdateTime > INVTAB_DISP_TIME)
			{
				m_nInvLevel = INVTAB_SCROLL_OUT;
				m_fInvUpdateTime = m_fTime;
			}
			break;
		case	INVTAB_SCROLL_OUT:
			if(m_fTime - m_fInvUpdateTime > INVTAB_SCROLL_TIME)
			{
				m_fInvScrollRatio = 0.0f;
				m_nInvLevel = INVTAB_INVISIBLE;
				m_fInvUpdateTime = m_fTime;
			}
			else
				m_fInvScrollRatio = 1.0f - (m_fTime - m_fInvUpdateTime) / INVTAB_SCROLL_TIME;

			break;
	}

	// Previous item
	if(m_stItems[0].hIcon)
	{
		DDWORD nNewX = (DDWORD)((m_stItems[0].locX + m_stItems[0].sizeX) * m_fInvScrollRatio) - m_stItems[0].sizeX;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stItems[0].hTab, DNULL, nNewX, m_stItems[0].locY, m_hTransColor);
	}

	// Current item
	if(m_stItems[1].hIcon)
	{
		DDWORD nNewX = (DDWORD)((m_stItems[1].locX + m_stItems[1].sizeX) * m_fInvScrollRatio) - m_stItems[1].sizeX;

		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stItems[1].hTab, DNULL, nNewX, m_stItems[1].locY, m_hTransColor);
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hItemName, DNULL, m_nItemNameX, m_nItemNameY, m_hTransColor);
	}

	// Next item
	if(m_stItems[2].hIcon)
	{
		DDWORD nNewX = (DDWORD)((m_stItems[2].locX + m_stItems[2].sizeX) * m_fInvScrollRatio) - m_stItems[2].sizeX;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stItems[2].hTab, DNULL, nNewX, m_stItems[2].locY, m_hTransColor);
	}
}

//*************************************************************************
//*****	Function:	DrawWeaponTabs()
//*****	Details:	Draws the ten item slots with weapon icons
//*************************************************************************

void CNewStatusBar::DrawWeaponTabs()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();

	// Handle the scrolling in and out of the inventory item tabs
	switch(m_nWeapLevel)
	{
		case	WEAPTAB_SCROLL_IN:
			if(m_fTime - m_fWeapUpdateTime > WEAPTAB_SCROLL_TIME)
			{
				m_fWeapScrollRatio = 1.0f;
				m_nWeapLevel = WEAPTAB_STOPPED;
				m_fWeapUpdateTime = m_fTime;
			}
			else
				m_fWeapScrollRatio = (m_fTime - m_fWeapUpdateTime) / WEAPTAB_SCROLL_TIME;
			break;

		case	WEAPTAB_STOPPED:
			break;

		case	WEAPTAB_SCROLL_OUT:
			if(m_fTime - m_fWeapUpdateTime > WEAPTAB_SCROLL_TIME)
			{
				m_fWeapScrollRatio = 0.0f;
				m_nWeapLevel = WEAPTAB_INVISIBLE;
				m_fWeapUpdateTime = m_fTime;
			}
			else
				m_fWeapScrollRatio = 1.0f - (m_fTime - m_fWeapUpdateTime) / WEAPTAB_SCROLL_TIME;

			break;
	}

	DDWORD nNewX = (DDWORD)(m_stWeapons[0].sizeX - (m_stWeapons[0].sizeX * m_fWeapScrollRatio)) + m_stWeapons[0].locX;

	for(int i = 0; i < 10; i++)
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_stWeapons[i].hTab, DNULL, nNewX, m_stWeapons[i].locY, m_hTransColor);
}

//*************************************************************************
//*****	Function:	DrawObjectives()
//*****	Details:	Draws the objectives tab
//*************************************************************************

void CNewStatusBar::DrawObjectives()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();
	DDWORD		width, height;
	DDWORD		halfWidth, halfHeight;
	DBYTE		scaleIndex = 0;

	m_pClientDE->GetSurfaceDims(m_hObjectives, &width, &height);
	halfWidth = width / 2;
	halfHeight = height / 2;

	// Handle the scrolling in and out of the inventory item tabs
	switch(m_nObjLevel)
	{
		case	OBJTAB_SCALE_UP:
			if(m_fTime - m_fObjUpdateTime > OBJTAB_SCALE_TIME)
			{
				m_fObjScaleRatio = 1.0f;
				m_nObjLevel = OBJTAB_STOPPED;
				m_fObjUpdateTime = m_fTime;
			}
			else
				m_fObjScaleRatio = (m_fTime - m_fObjUpdateTime) / OBJTAB_SCALE_TIME;

			// Scale the rectangle accordingly...
			scaleIndex = (DBYTE)(m_fObjScaleRatio * OBJTAB_SCALE_LEVELS);
			m_rObjectives.top		= (int)(m_nObjectivesY + (halfHeight * (1.0f - g_ObjScaleUpY[scaleIndex])));
			m_rObjectives.left		= (int)(m_nObjectivesX + (halfWidth * (1.0f - g_ObjScaleUpX[scaleIndex])));
			m_rObjectives.bottom	= (int)(m_nObjectivesY + halfHeight + (halfHeight * g_ObjScaleUpY[scaleIndex]));
			m_rObjectives.right		= (int)(m_nObjectivesX + halfWidth + (halfWidth * g_ObjScaleUpX[scaleIndex]));
			break;

		case	OBJTAB_STOPPED:
			// Scale the rectangle accordingly...
			m_rObjectives.top		= m_nObjectivesY;
			m_rObjectives.left		= m_nObjectivesX;
			m_rObjectives.bottom	= m_nObjectivesY + height;
			m_rObjectives.right		= m_nObjectivesX + width;
			break;

		case	OBJTAB_SCALE_DOWN:
			if(m_fTime - m_fObjUpdateTime > OBJTAB_SCALE_TIME)
			{
				m_fObjScaleRatio = 1.0f;
				m_nObjLevel = INVTAB_INVISIBLE;
				m_fObjUpdateTime = m_fTime;
			}
			else
				m_fObjScaleRatio = (m_fTime - m_fObjUpdateTime) / OBJTAB_SCALE_TIME;

			// Scale the rectangle accordingly...
			scaleIndex = (DBYTE)(m_fObjScaleRatio * OBJTAB_SCALE_LEVELS);
			m_rObjectives.top		= (int)(m_nObjectivesY + (halfHeight * (1.0f - g_ObjScaleDownY[scaleIndex])));
			m_rObjectives.left		= (int)(m_nObjectivesX + (halfWidth * (1.0f - g_ObjScaleDownX[scaleIndex])));
			m_rObjectives.bottom	= (int)(m_nObjectivesY + halfHeight + (halfHeight * g_ObjScaleDownY[scaleIndex]));
			m_rObjectives.right		= (int)(m_nObjectivesX + halfWidth + (halfWidth * g_ObjScaleDownX[scaleIndex]));
			break;
	}

	m_pClientDE->ScaleSurfaceToSurfaceTransparent(hScreen, m_hObjectives, &m_rObjectives, DNULL, m_hTransColor);
}

//*************************************************************************
//*****	Function:	DrawObjectivesIcon()
//*****	Details:	Draws the objectives tab
//*************************************************************************

void CNewStatusBar::DrawObjectivesIcon()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();
	DDWORD		width, height;
	DDWORD		width2;

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);
	m_pClientDE->GetSurfaceDims(m_hObjectivesIcon, &width2, &height);
	width = width - width2 - 5;
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hObjectivesIcon, DNULL, width, 5, m_hTransColor);
}

//*************************************************************************
//*****	Function:	DrawPowerBar()
//*****	Details:	Draws the power bar
//*************************************************************************

void CNewStatusBar::DrawPowerBar()
{
	if(!m_pClientDE) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();

	if(m_fTime - m_fPowerBarUpdate > BARTAB_DISP_TIME)
		m_nPowerBar = BARTAB_NONE;

	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hPowerBar, DNULL, m_nPowerBarX, m_nPowerBarY, m_hTransColor);
}

//*************************************************************************
//*****	Function:	DrawFlagIcon()
//*****	Details:	Draws the multiplayer flag icon
//*************************************************************************

void CNewStatusBar::DrawFlagIcon()
{
	if(!m_pClientDE) return;
	if (!g_pBloodClientShell->IsMultiplayerTeamBasedGame()) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	if (!hScreen) return;

	DDWORD		width, height;
	DDWORD		width2;

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);
	m_pClientDE->GetSurfaceDims(m_hFlagIcon, &width2, &height);
	width = width - width2 - 5;
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hFlagIcon, DNULL, width, m_nTeamIconHeight + 10, m_hTransColor);
}

//*************************************************************************
//*****	Function:	DrawTeamIcon()
//*****	Details:	Draws the multiplayer team icon
//*************************************************************************

void CNewStatusBar::DrawTeamIcon()
{
	if (!m_hTeamIcon) return;
	if (!m_pClientDE) return;
	if (!g_pBloodClientShell->IsMultiplayerTeamBasedGame()) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	if (!hScreen) return;

	DDWORD		width, height;
	DDWORD		width2;

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);
	m_pClientDE->GetSurfaceDims(m_hTeamIcon, &width2, &height);
	width = width - width2 - 5;
	m_nTeamIconHeight = height;
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hTeamIcon, DNULL, width, 5, m_hTransColor);
}