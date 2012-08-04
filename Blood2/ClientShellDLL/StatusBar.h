//*************************************************************************
//*************************************************************************
//***** MODULE  : StatusBar.cpp
//***** PURPOSE : Blood 2 Status Bar
//***** CREATED : 11/4/97
//***** MODIFIED: 3/30/98
//*************************************************************************
//*************************************************************************

#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "LTGUIMgr.h"	// For coolfont

//**********************************************************

#define		SB_STATTAB_SIZE_X		108
#define		SB_STATTAB_SIZE_Y		36

#define		SB_STATICON_SIZE_X		28
#define		SB_STATICON_SIZE_Y		28

#define		SB_INVICON_SIZE_X		36
#define		SB_INVICON_SIZE_Y		36

#define		SB_HIGHLIGHT_SIZE_X		44
#define		SB_HIGHLIGHT_SIZE_Y		44

#define		SB_INFOTAB_SIZE_X		300
#define		SB_INFOTAB_SIZE_Y		100

#define		SB_KEYTAB_SIZE_X		32
#define		SB_KEYTAB_SIZE_Y		24

#define		SB_NUMBER_SIZE_X		22
#define		SB_NUMBER_SIZE_Y		28

#define		SB_EQUIP_SIZE_X			240
#define		SB_EQUIP_SIZE_Y			320

#define		SB_SYMBOL_SIZE			86
#define		SB_SPELLNAME_SIZE_X		235

#define		SB_CHARFIELD_SIZE_X		170

#define		SB_CHARPIC_SIZE			50

#define		SB_MAX_CS_STRING_SIZE	14

#define		SB_SPELL_TEXT_WIDTH		245

//**********************************************************

#define		SB_ARMORICON_LOC_X		0
#define		SB_MANAICON_LOC_X		28
#define		SB_HEALTHICON_LOC_X		56
#define		SB_AMMOICON_LOC_X		168

#define		SB_SYMBOL_LOC_X			378
#define		SB_SYMBOL_LOC_Y			116

#define		SB_SPELLNAME_LOC_X		99
#define		SB_SPELLCURSOR_LOC_X	20
#define		SB_SPELLDOT_LOC_X		75

//**********************************************************

#define		SB_LEFTICON_OFFSET_X	76
#define		SB_RIGHTICON_OFFSET_X	3
#define		SB_ICON_OFFSET_Y		3

#define		SB_LEFTNUM_OFFSET_X		4
#define		SB_RIGHTNUM_OFFSET_X	36
#define		SB_NUM_OFFSET_Y			3

#define		SB_HIGHLIGHT_OFFSET_X	-4
#define		SB_HIGHLIGHT_OFFSET_Y	-4

#define		SB_EQUIP_OFFSET_X		15
#define		SB_EQUIP_OFFSET_Y		70
#define		SB_EQUIP_INDENT_X		25

#define		SB_CHARNUM1_OFFSET_X	135
#define		SB_CHARNUM2_OFFSET_X	152

//**********************************************************

#define		SB_SELECTED_ITEM		3
#define		SB_NUM_WEAPONS			10
#define		SB_NUM_ITEMS			8
#define		SB_NUM_SPELLS			6

#define		SB_NUM_HIGHLIGHTS		9

#define		SB_NUM_EQUIP_ITEMS		27
#define		SB_NUM_EQUIP_DISP_ITEMS	13

#define		SB_NUM_SINGLEPLAYER_SPELLS	13		// zero to 13 -- actual amount is 14

#define		SB_NUM_BOXTYPES			7
#define		SB_NUM_CSBOXES			12
#define		SB_NUM_CSFIELDS			34

//**********************************************************

#define		SB_KEYSLIDE_AMOUNT		6

//**********************************************************

#define		SB_KEYTAB_TIME			0.2f

#define		SB_INVENTORY_TIME_1		0.15f
#define		SB_INVENTORY_TIME_2		0.2f
#define		SB_INVENTORY_TIME_3		3.0f
#define		SB_INVENTORY_TIME_4		0.12f

#define		SB_HEARTBEAT_TIME_1		1.6f
#define		SB_HEARTBEAT_TIME_2		0.05f

#define		SB_COMMLINK_TIME_1		0.12f
#define		SB_COMMLINK_TIME_2		0.5f

//**********************************************************

#define		SB_MULTIPLAYER			1
#define		SB_SINGLEPLAYER			0

//**********************************************************

#define		SB_CSFIELD_VALUE		0
#define		SB_CSFIELD_SELECT		1
#define		SB_CSFIELD_NEWNAME		2
#define		SB_CSFIELD_LOADSKIN		3
#define		SB_CSFIELD_SAVECHAR		4
#define		SB_CSFIELD_LOADCHAR		5
#define		SB_CSFIELD_DELETECHAR	6
#define		SB_CSFIELD_ACCEPT		7
#define		SB_CSFIELD_CANCEL		8

//**********************************************************

typedef struct _CInventoryItem
{
	char		slot;
	char		type;		// Type of item
	char		amount;		// Amount of this item
	DDWORD		locXLeft;	// Location in surface - left X
	DDWORD		locXRight;	// Location in surface - right X
}	CInventoryItem;

//**********************************************************

typedef struct _CEquipItem
{
	char		type;		// Type of item
	char		name;		// Index into array of names
	char		offset;		// X offset
}	CEquipItem;

//**********************************************************

typedef struct _CCharBox
{
	char		box;		// Type of field box
	short		x;
	short		y;

	char		**list;		// List of names to use
	char		max;		// Max names in list
}	CCharBox;

//**********************************************************

typedef struct _CCharField
{
	char		type;		// Type of field (numbered, string select, or continue)
	char		active;		// Active field?
	char		use;		// How the field will be used

	char		name;		// String for the name field
	char		num1;
	char		num2;

	char		justify;	// Type of justification for the name

	short		x;			// X location of the field
	short		y;			// Y location of the field
}	CCharField;

//**********************************************************

class	CKeyTab
{
	public:
		// Constructor and destructor functions
		CKeyTab()	{ prev = next = 0; type = 0; currentX = currentY = 0; locYTop = locYBottom = 0; direction = 0; startTime = 0.0f; }
		~CKeyTab()	{	}

		// Public list data members
		CKeyTab		*prev;
		CKeyTab		*next;
		char		type;

		short		currentX;	// Current location on screen X
		short		currentY;	// Current location on screen Y
		DDWORD		locYTop;	// Location in surface Y
		DDWORD		locYBottom;	// Location in surface Y

		DBOOL		direction;	// Movement direction
		DFLOAT		startTime;	// Start time of movement

	protected:
};

//**********************************************************

class	CStatusBar
{
	public:
		// Constructors, destructors, and intialization functions
		CStatusBar();
		~CStatusBar();

		DBOOL		Init (CClientDE* pClientDE);
		void		AdjustRes();
		void		Term();

		DBOOL		InitSpellbook();
		void		TermSpellbook();
		DBOOL		InitCharScreen();
		void		TermCharScreen();

//		void		SetupStatStrings(char **w, char numW, char **i, char numI, char **s, char numS, char **b, char numB);

		// Key interface modification functions
		DBOOL		AddKey(char type);
		DBOOL		RemoveKey(char type);
		void		ClearKeys();

		// Inventory interface modification functions
		DBOOL		AddItem(char type);
		DBOOL		RemoveItem(char type);
		void		ClearItems();
		// DBOOL		AddSpell(char type);
		DBOOL		RemoveSpell(char type);
		void		ClearSpells();
		void		SetupWeapons(char slot, char weapon)	{ m_hWeapons[slot] = weapon; SetupEquipList(); }
		void		ClearWeapons()							{ memset(m_hWeapons, 0, 10); SetupEquipList(); }
		void		SetupEquipList();

		// Control functions
		DBOOL		HandleKeyDown(int key);
		// DBOOL		HandleSpellbookControl(int key);
		DBOOL		HandleEquipControl(int key);
		DBOOL		HandleCharScreenControl(int key);

		// Communications functions
		char		StartCommunication(char chosen, char *szFile, char *szText);

		// Animation and drawing functions
		void		Draw(DBOOL bDrawBar);

		// Stats interface modification functions
		void		SetHealth(DDWORD nHealth) { m_nHealth = nHealth; m_fHeartBeat = SB_HEARTBEAT_TIME_1 * ((float)(m_nHealth % 101) / 100.0f); }
		void		SetArmor(DDWORD nArmor) { m_nArmor = nArmor; }
		void		SetAmmo(DDWORD nAmmo) { m_nAmmo = nAmmo; }
		void		SetMana(DDWORD nMana) { m_nMana = nMana; }
		void		SetSpellLevel(char sLevel)	{	m_bSpellLevel = sLevel;	}
		void		SetInfo(char type)	{ m_bMultiInfo = type; }
		void		SetAmmoType(short type)	{ m_nAmmoType = type; }

		char		GetCurrentItem()	{ return m_bCurrentItem; }
		char		EquipOn()			{ return m_bEquipOn; }
		char		SpellbookOn()		{ return m_bSpellbookOn; }
		char		CharScreenOn()		{ return m_bCharScreenOn; }

		CCharField*	GetFieldList()		{ return m_pCSFieldList; }
		char**		GetNameList()		{ return m_pCSBoxes[3].list; }
		char*		GetCharName()		{ return m_szCSNameString; }

	private:
		// Private drawing functions
		void		DrawCommunication(HSURFACE hScreen);
		void		DrawStatTabs(HSURFACE hScreen);
		void		DrawItemTabs(HSURFACE hScreen);
		void		DrawHighlight(HSURFACE hScreen, char type);
		void		DrawKeyTabs(HSURFACE hScreen);
		void		DrawInfoTab(HSURFACE hScreen);
		void		DrawEquipTab(HSURFACE hScreen);
		void		DrawSpellbook(HSURFACE hScreen);
		void		DrawCharacterScreen(HSURFACE hScreen);

		void		DrawNumber(DDWORD nValue, DDWORD xPos, DDWORD yPos);

		void		HandleItemInventory(HSURFACE hScreen);
		// void		HandleSpellInventory(HSURFACE hScreen);

		char		HandleCharScreenLeft();
		char		HandleCharScreenRight();

		void		SaveB2CFile();
		void		LoadB2CFile(FileEntry *pfe);
		void		DeleteB2CFile(FileEntry *pfe);

		// Private data members
		CClientDE*	m_pClientDE;

		HSURFACE	m_hStatTabs;			// surface to hold the stat tabs
		HSURFACE	m_hStatIcons;			// surface to hold the icons for the stats
		HSURFACE	m_hStatNums;			// surface to hold the font for numbers
		HSURFACE	m_hItemIcons;			// surface to hold the icons for inventory items
		HSURFACE	m_hSpellIcons;			// surface to hold the icons for spells
		HSURFACE	m_hItemLight;			// surface to hold the item icon highlight
		HSURFACE	m_hSpellLight;			// surface to hold the spell icon highlight
		HSURFACE	m_hKeyTabs;				// surface to hold the key tabs
		HSURFACE	m_hEquipTab;			// surface to hold the equipment screen
		HSURFACE	m_hEquipCursor;			// surface to hold the equipment screen cursor
		HSURFACE	m_hCharFaces;			// surface to hold the character faces

		HSURFACE	m_hSpellBG;				// surface to hold the spellbook background
		HSURFACE	m_hSpellsLo;			// surface to hold the greyed out spell names
		HSURFACE	m_hSpellsHi;			// surface to hold the highlighted spell names
		HSURFACE	m_hSpellSymbols;		// surface to hold the spell symbols for the book
		HSURFACE	m_hSpellCursor;			// surface to hold the spellbook cursor
		HSURFACE	m_hSpellDot;			// surface to hold the spellbook dot marker

		HSURFACE	m_hCharFields;			// surface to hold the fields for character creation screen

		// Fonts and font cursors
		CoolFontCursor	*m_pStatCursor;
		CoolFontCursor	*m_pInfoCursor;
		CoolFontCursor	*m_pEquipCursor;
		CoolFontCursor	*m_pSpellCursor;
		CoolFontCursor	*m_pCharCursor;

		CoolFont	*m_pStatFont1;
		CoolFont	*m_pStatFont2;
		CoolFont	*m_pStatFont3;
		CoolFont	*m_pStatFont4;
		CoolFont	*m_pSpellFont1;
		CoolFont	*m_pSpellFont2;
		CoolFont	*m_pCharFont1;

		// Stat tab variables
		DBOOL		m_bStatsOn;
		DDWORD		m_nHealth;
		DDWORD		m_nArmor;
		DDWORD		m_nAmmo;
		DDWORD		m_nMana;
		short		m_nAmmoType;
		short		m_nHeartFrame;
		short		m_nAmmoFrame;
		DFLOAT		m_fHeartBeat;
		DFLOAT		m_fHeartTime;

		// Stat tab locations
		DDWORD		m_nHealthX;
		DDWORD		m_nHealthY;
		DDWORD		m_nArmorX;
		DDWORD		m_nArmorY;
		DDWORD		m_nManaX;
		DDWORD		m_nManaY;
		DDWORD		m_nAmmoX;
		DDWORD		m_nAmmoY;

		// Communication variables
		char		m_bCommState;
		char		m_bCommPic;
		char		m_szCommVoice[200];
		char		m_szCommText[500];
		DFLOAT		m_fCommStartTime;
		HSOUNDDE	m_sCommSound;
		char		m_bPlayVoice;
		DFLOAT		m_fCommLength;
		short		m_nCommTextWidth;

		// Communcation locations
		DDWORD		m_nCommPicX;
		DDWORD		m_nCommPicY;
		DDWORD		m_nCommPicStartY;
		DDWORD		m_nCommTextX;
		DDWORD		m_nCommTextY;

		// General inventory variables
		char		**m_hWeaponNames;
		char		**m_hItemNames;
		char		**m_hSpellNames;
		char		**m_hBindingNames;
		char		m_nTotalWeapons;
		char		m_nTotalItems;
		char		m_nTotalSpells;
		char		m_nTotalBindings;

		// Inventory/Spell tab variables
		DBOOL		m_bInventoryOn;
		DBOOL		m_bInventoryState;
		char		m_hWeapons[SB_NUM_WEAPONS];
		CInventoryItem	m_hItems[SB_NUM_ITEMS];
		CInventoryItem	m_hSpells[SB_NUM_SPELLS];
		CInventoryItem	*m_hItemList[SB_NUM_ITEMS];
		CInventoryItem	*m_hSpellList[SB_NUM_SPELLS];
		CInventoryItem	*m_hSelectedItem;
		CInventoryItem	*m_hSelectedSpell;
		short		m_nNumItems;
		short		m_nNumSpells;
		DFLOAT		m_fInvStartTime;
		char		m_bCurrentItem;

		// Inventory/Spell tab location
		DDWORD		m_nItemX;
		DDWORD		m_nItemY;
		DDWORD		m_nSpellX;
		DDWORD		m_nSpellY;
		DDWORD		m_nInventoryY;
		short		m_nHighlightFrame;

		// Equipment Screen variables
		DBOOL		m_bEquipOn;
		CEquipItem	m_pEquipList[SB_NUM_EQUIP_ITEMS];
		char		m_bTopEquipItem;
		char		m_bSelectedEquipItem;

		// Equipment Screen location
		DDWORD		m_nEquipX;
		DDWORD		m_nEquipY;

		// Spell Book variables
		DBOOL		m_bSpellbookOn;
		char		m_bCurrentSpell;
		char		m_bSpellLevel;
		char		m_pCurrentSpells[SB_NUM_SINGLEPLAYER_SPELLS + 1];
		short		m_nSpellTextWidth;

		// Character Screen variables
		DBOOL		m_bCharScreenOn;
		CCharBox	m_pCSBoxes[SB_NUM_CSBOXES];
		CCharField	m_pCSFieldList[SB_NUM_CSFIELDS];
		CCharField	m_cfExtraPoints;
		char		m_bCSCurrentField;
		DRect		m_rCSFields[SB_NUM_BOXTYPES];
		char		m_bCSInputState;
		char		*m_szCSInputString;
		char		m_szCSNameString[SB_MAX_CS_STRING_SIZE];
		char		m_szCSSaveFile[SB_MAX_CS_STRING_SIZE];
		char		m_bCSInputChar;
		char		m_bCSSelectState;
		FileEntry	*m_pCharFiles;
		FileEntry	*m_pCharFileIndex;
		char		m_bCharFileNum;

		// Key tab variables
		DBOOL		m_bKeysOn;
		CKeyTab		m_hFirstKey;
		CKeyTab		m_hLastKey;

		// Info tab variables
		DBOOL		m_bInfoOn;
		DBOOL		m_bMultiInfo;
		DFLOAT		m_fInfoStartTime;

		// Info tab location
		short		m_nInfoX;
		short		m_nInfoY;

		// General information variables
		DDWORD		m_nScreenWidth;
		DDWORD		m_nScreenHeight;
		HDECOLOR	m_hTransColor;
};

#endif	// __STATUSBAR_H__