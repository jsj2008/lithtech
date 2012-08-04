//*************************************************************************
//*************************************************************************
//***** MODULE  : StatusBar.cpp
//***** PURPOSE : Blood 2 Status Bar
//***** CREATED : 11/4/97
//***** MODIFIED: 3/30/98
//*************************************************************************
//*************************************************************************

#include "ClientRes.h"
#include "StatusBar.h"
#include "VKDefs.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"
#include <stdio.h>

//*************************************************************************

static short g_NumberXOffsets[10] = {0, 22, 44, 66, 88, 110, 132, 154, 176, 198};
static short g_NumberYOffsets[5] = {0, 28, 56, 84, 112};
static short g_ItemXOffsets[8] = {-108, -72, -36, 0, 36, 72, 108, 144};
static short g_SpellXOffsets[6] = {-108, -72, -36, 0, 36, 72};

static char  g_SpellNameFix[14] = {0,1,2,3,4,5,6,13,8,10,11,12,9,7};
static short g_SpellNameYOffsets[15] = {0,26,53,80,106,134,161,187,213,239,265,292,317,344,371};
static short g_SpellNameYLocs[14] = {84,110,135,162,188,212,239,265,291,315,341,368,393,417};
static short g_SpellCursorYLocs[14] = {68,93,118,145,170,197,222,246,273,298,325,351,374,401};
static short g_SpellDotYLocs[14] = {88,115,140,164,191,216,241,267,293,318,345,370,396,422};

static char  g_FieldBoxTypes[12] = {2,3,4,0,0,5,6,0,0,0,0,0};
static short g_FieldBoxesYPos[8] = {0,20,40,145,216,406,494,616};
static short g_FieldBoxesXLoc[12] = {15,15,15,235,235,455,455,455,455,455,455,455};
static short g_FieldBoxesYLoc[12] = {15,162,275,415,435,15,143,305,340,375,410,445};

static char  g_FieldNames[34]   = {0,1,2,3,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,6,7,8};
static char  g_FieldTypes[34]   = {0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,3,4,5,5,5,5,6,6,6,6,6,6,7,8,9,10,11};
static char  g_FieldActiveS[34] = {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1};
static char  g_FieldActiveM[34] = {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static char  g_FieldUses[34]    = {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,/**/2,3,1,1,1,1,1,1,1,1,1,1,4,5,6,7,8};
static char  g_FieldJustify[34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1};
static short g_FieldXOffset[34] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,85,85,5,5,5,5,5,5,5,5,5,5,85,85,85,85,85};
static short g_FieldYOffset[34] = {19,37,54,71,19,37,54,19,37,54,71,88,105,122,139,156,173,3,3,19,37,54,71,19,37,54,71,88,105,3,3,3,3,3};

//*************************************************************************

static char *g_WeaponNames[33] =
{
	"<EMPTY>\0",
	"BERETTA\0",
	"SUB MACHINE GUN\0",
	"PULSE GUN\0",
	"FLARE GUN\0",
	"SHOTGUN\0",
	"SNIPER RIFLE\0",
	"COMBAT SHOTGUN\0",
	"RG LAUNCHER\0",
	"ASSAULT CANNON\0",
	"MISSILE LAUNCHER\0",
	"ENERGY CANNON\0",
	"CARBON FREEZER\0",
	"PARTICLE GUN\0",
	"SINGULARITY GUN\0",
	"ASSAULT RIFLE\0",
	"FLAMER\0",
	"MINIGUN\0",
	"LASER RIFLE\0",
	"MICROWAVE GUN\0",
	"GYROJET GUN\0",
	"TESLA CANNON\0",
	"VOODOO DOLL\0",
	"DEADLY DISC\0",
	"LIFE LEECH\0",
	"FLAYER\0",
	"SLICER\0",
	"WITHERING HAND\0",
	"DISINTEGRATOR\0",
	"EXPLOSIVE FIST\0",
	"BURN\0",
	"LIGHTNING STORM\0",
	"KNIFE\0"
};

static char g_WeaponStr[33] = {0,1,2,3,2,3,4,3,3,6,5,6,4,4,5,3,2,5,4,3,4,3,0,0,0,0,0,0,0,0,0,0,1};
static char g_WeaponFoc[33] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,4,2,2,2,3,4,3,4,0};
static char	*g_ItemNames[14] =
{
	"<EMPTY>\0",
	"MEDICAL KIT\0",
	"NIGHT VISION\0",
	"DEFENSE GLOBE\0",
	"THE EYE\0",
	"BINOCULARS\0",
	"PORTAL\0",
	"CLOAK\0",
	"FLASHLIGHT\0",
	"SPELL BOOK\0",
	"PROX BOMB\0",
	"REMOTE BOMB\0",
	"SMOKE BOMB\0",
	"TIME BOMB\0"
};

static char	*g_SpellNames[15] =
{
	"<EMPTY>\0",
	"SHIELD\0",
	"REFLECTION\0",
	"STONE\0",
	"TELEPORT\0",
	"ANIMATION\0",
	"SPEED\0",
	"HEAL\0",
	"CLAIRVOYANCE\0",
	"AURA\0",
	"FORCE WALL\0",
	"DOUBLE\0",
	"DARKNESS\0",
	"ILLUSION\0",
	"GRIM COMMUNION\0"
};

static char *g_BindingNames[16] =
{
	"<EMPTY>\0",
	"STRENGTH\0",
	"SPEED\0",
	"RESISTANCE\0",
	"FOCUS\0",
	"JUMPING\0",
	"CLIMB\0",
	"STEALTH\0",
	"PROTECTION\0",
	"CONSTITUTION\0",
	"REGENERATION\0",
	"IMPACT RESISTANCE\0",
	"QUICKNESS\0",
	"DAMAGE\0",
	"BLENDING\0",
	"SOUL STEALING\0"
};

static char *g_EquipTitles[3] =
{
	"WEAPONS:\0",
	"ITEMS:\0",
	"SPELLS:\0"
};

static char *g_SpellRequires[14] =
{
	" MAGIC: 1| FOCUS: SPECIAL\0",
	" MAGIC: 1| FOCUS: SPECIAL\0",
	" MAGIC: 2| FOCUS: 200\0",
	" MAGIC: 2| FOCUS: 200\0",
	" MAGIC: 2| FOCUS: 200\0",
	" MAGIC: 1| FOCUS: 100\0",
	" MAGIC: 1| FOCUS: SPECIAL\0",
	" MAGIC: 1| FOCUS: 100\0",
	" MAGIC: 1| FOCUS: 100\0",
	" MAGIC: 2| FOCUS: 150\0",
	" MAGIC: 1| FOCUS: 50\0",
	" MAGIC: 1| FOCUS: 100\0",
	" MAGIC: 1| FOCUS: 100\0",
	" MAGIC: 2| FOCUS: 150\0"
};

static char *g_SpellInfo[14] =
{
	"~A MAGICAL BARRIER THAT, WHEN ACTIVATED, WILL DETRACT PHYSICAL DAMAGE FROM YOUR FOCUS INSTEAD OF YOUR HEALTH.\0",
	"~REFLECTS ATTACKS BACK AT THEIR ORIGIN. THE MORE POWERFUL THE ATTACK, THE MORE FOCUS USED.|~INEFFECTIVE AGAINST AREA OF EFFECT DAMAGE.\0",
	"~TURNS THE CASTER TO STONE FOR A FEW SECONDS, MAKING HIM COMPLETELY IMMUNE TO ATTACKS, BUT PREVENTS MOVEMENT.\0",
	"~THE CASTER INSTANTLY TRAVERSES PHYSICAL SPACE FOR A SET DISTANCE IN A STRAIGHT LINE FORWARD, STOPPED ONLY BY PHYSICAL OBSTRUCTIONS.\0",
	"~REANIMATE THE DEAD AND HAVE THEM DO YOUR BIDDING.\0",
	"~GIVES THE CASTER A TEMPORARY BURST OF SPEED THAT ALLOWS FASTER MOVEMENT AND INCREASES THE SPEED OF MELEE ATTACKS.\0",
	"~RESTORES ONE POINT OF HEALTH FOR EVERY TWO FOCUS, UP TO THE PLAYERS MAXIMUM HEALTH OR FOCUS, WHICHEVER COMES FIRST.\0",
	"~ABSORBS BODIES AND ORGANIC REFUSE IN THE IMMEDIATE AREA AS HEALTH AND ARMOR.\0",
	"~A MAGICAL AURA APPEARS AROUND ALL ORGANIC MATTER, MAKING THEM EASIER TO SEE, EVEN IF HIDING IN DARKNESS.\0",
	"~CREATE UP TO THREE AUTONOMOUS DOUBLES TO FOOL YOUR ENEMIES.|~THEY WILL NOT ATTACK, BUT CANNOT BE HARMED EITHER.\0",
	"~CREATES AN AREA OF DARKNESS IN EVEN THE BRIGHTEST OF AREAS FOR A LIMITED PERIOD OF TIME.\0",
	"~ALLOWS THE CASTER TO USE ILLUSION TO ASSUME FORM OF A RANDOM POWERUP.|~INEFFECTIVE IF THE PLAYER MOVES OR ATTACKS.\0",
	"~CREATES A BARRIER THAT PREVENTS LIVING MATTER FROM PASSING FOR A LIMITED TIME.\0",
	"~ALLOWS THE CASTER TO SEE THROUGH THE EYES OF CREATURES THAT MAY BE NEARBY, EVEN IF THE CASTER CANNOT SEE THEM.\0"
};

static char g_SpellLevel[14] = {1,1,2,2,2,1,1,1,1,1,2,1,1,2};

static char *g_CharScreenValues[10] =	{"0\0","1\0","2\0","3\0","4\0","5\0","6\0","7\0","8\0","9\0"};

static char *g_FieldStatNames[9] =
{
	"STRENGTH\0",
	"SPEED\0",
	"RESISTANCE\0",
	"FOCUS\0",
	"SAVE CHARACTER",
	"LOAD CHARACTER",
	"DELETE CHARACTER",
	"CONTINUE",
	"CANCEL"
};

static char *g_SkinNames[4] =
{
	"DEFAULT SKIN\0",
	"BARE ASS NAKED\0",
	"SUPER HERO\0",
	"WHIPS AND CHAINS\0"
};

static char *g_CharacterNames[5] =
{
	"   CHARACTER NAME   \0",
	"CALEB\0",
	"OPHELIA\0",
	"ISHMAEL\0",
	"GABREILLA\0"
};

//*************************************************************************
//*****	Function:	CStatusBar()
//*****	Details:	Constructor
//*************************************************************************

CStatusBar::CStatusBar()
{
	m_pClientDE = 0;

	m_hStatTabs = 0;
	m_hStatIcons = 0;
	m_hStatNums = 0;
	m_hItemIcons = 0;
	m_hSpellIcons = 0;
	m_hItemLight = 0;
	m_hSpellLight = 0;
	m_hKeyTabs = 0;
	m_hEquipTab = 0;
	m_hEquipCursor = 0;
	m_hCharFaces = 0;

	m_hSpellBG = 0;
	m_hSpellsLo = 0;
	m_hSpellsHi = 0;
	m_hSpellSymbols = 0;
	m_hSpellCursor = 0;
	m_hSpellDot = 0;

	m_pStatCursor = 0;
	m_pInfoCursor = 0;
	m_pEquipCursor = 0;
	m_pSpellCursor = 0;
	m_pCharCursor = 0;

	m_pStatFont1 = 0;
	m_pStatFont2 = 0;
	m_pStatFont3 = 0;
	m_pStatFont4 = 0;
	m_pSpellFont1 = 0;
	m_pSpellFont2 = 0;
	m_pCharFont1 = 0;

	m_bStatsOn = 4;			// All stats are drawn initially
	m_nHealth = 0;
	m_nArmor = 0;
	m_nAmmo = 0;
	m_nMana = 0;
	m_nAmmoType = 0;
	m_nHeartFrame = 0;
	m_nAmmoFrame = 0;
	m_fHeartBeat = SB_HEARTBEAT_TIME_1;
	m_fHeartTime = 0.0f;

	m_nHealthX = 0;
	m_nHealthY = 0;
	m_nArmorX = 0;
	m_nArmorY = 0;
	m_nManaX = 0;
	m_nManaY = 0;
	m_nAmmoX = 0;
	m_nAmmoY = 0;

	m_bCommState = 0;
	m_bCommPic = 0;
	memset(m_szCommVoice, 0, 100);
	memset(m_szCommText, 0, 200);
	m_fCommStartTime = 0.0f;
	m_sCommSound = 0;
	m_nCommTextWidth = 0;

	m_nCommPicX = 0;
	m_nCommPicY = 0;
	m_nCommPicStartY = 0;
	m_nCommTextX = 0;
	m_nCommTextY = 0;

	m_bInventoryOn = 0;		// Inventory is not active initially
	m_bInventoryState = 0;
	memset(m_hWeapons, 0, SB_NUM_WEAPONS);
	memset(m_hItems, 0, sizeof(CInventoryItem) * SB_NUM_ITEMS);
	memset(m_hSpells, 0, sizeof(CInventoryItem) * SB_NUM_SPELLS);
	memset(m_hItemList, 0, sizeof(CInventoryItem*) * SB_NUM_ITEMS);
	memset(m_hSpellList, 0, sizeof(CInventoryItem*) * SB_NUM_SPELLS);
	m_hSelectedItem = 0;
	m_hSelectedSpell = 0;
	m_nNumItems = 0;
	m_nNumSpells = 0;
	m_fInvStartTime = 0.0f;
	m_bCurrentItem = 0;

	m_nItemX = 0;
	m_nItemY = 0;
	m_nSpellX = 0;
	m_nSpellY = 0;
	m_nInventoryY = 0;
	m_nHighlightFrame = 0;

	m_bEquipOn = 0;			// Equipment screen is not active initially
	memset(m_pEquipList, 0, sizeof(CEquipItem) * SB_NUM_EQUIP_ITEMS);
	m_bTopEquipItem = 0;
	m_bSelectedEquipItem = 1;

	m_nEquipX = 0;
	m_nEquipY = 0;

	m_bSpellbookOn = 0;		// Spellbook is not active initially
	m_bCurrentSpell = 0;
	m_bCurrentSpell = 0;
	m_bSpellLevel = 0;
	memset(m_pCurrentSpells, 0, SB_NUM_SINGLEPLAYER_SPELLS + 1);
	m_nSpellTextWidth = SB_SPELL_TEXT_WIDTH;

	m_bCharScreenOn = 0;
	memset(m_pCSBoxes, 0, sizeof(CCharBox) * SB_NUM_CSBOXES);
	memset(m_pCSFieldList, 0, sizeof(CCharField) * SB_NUM_CSFIELDS);
	memset(&m_cfExtraPoints, 0, sizeof(CCharField));
	m_bCSCurrentField = 0;
	memset(m_rCSFields, 0, sizeof(DRect) * SB_NUM_BOXTYPES);
	m_bCSInputChar = 0;
	m_bCSSelectState = 0;
	m_pCharFiles = 0;
	m_pCharFileIndex = 0;
	m_bCharFileNum = 0;

	m_bKeysOn = 1;			// Keys are turned on by default

	m_bInfoOn = 0;			// Information screen in not displayed initially
	m_bMultiInfo = 0;
	m_fInfoStartTime = 0.0f;

	m_nInfoX = 0;
	m_nInfoY = 0;

	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_hTransColor = 0;
}

//*************************************************************************
//*****	Function:	~CStatusBar()
//*****	Details:	Destructor
//*************************************************************************

CStatusBar::~CStatusBar()
{
	if( m_sCommSound )
	{
		g_pClientDE->KillSound( m_sCommSound );
	}
}

//*************************************************************************
//*****	Function:	Init(CClientDE* pClientDE)
//*****	Details:	Initializes the status screens
//*************************************************************************

DBOOL CStatusBar::Init(CClientDE* pClientDE)
{
	if (!pClientDE) return DFALSE;
	Term();

	m_pClientDE = pClientDE;

	// Initialize the graphic surfaces
	m_hStatTabs = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/stattabs.pcx");
	m_hStatIcons = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/staticons.pcx");
	m_hStatNums = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/statnums.pcx");
	m_hItemIcons = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/itemicons.pcx");
	m_hSpellIcons = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/spellicons.pcx");
	m_hItemLight = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/itemhighlight.pcx");
	m_hSpellLight = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/spellhighlight.pcx");
	m_hKeyTabs = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/keytabs.pcx");
	m_hEquipTab = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/equiptab.pcx");
	m_hEquipCursor = m_pClientDE->CreateSurfaceFromBitmap ("interface/status/equipcursor.pcx");
	m_hCharFaces = m_pClientDE->CreateSurfaceFromBitmap("interface/status/characterfaces.pcx");

	// Setup fonts
	m_pStatFont1 = new CoolFont();
	m_pStatFont1->Init(m_pClientDE, "interface/fonts/StatFont1.pcx");
	m_pStatFont1->LoadXWidths("blood2/interface/fonts/StatFont1.fnt");

	m_pStatFont2 = new CoolFont();
	m_pStatFont2->Init(m_pClientDE, "interface/fonts/StatFont2.pcx");
	m_pStatFont2->LoadXWidths("blood2/interface/fonts/StatFont2.fnt");

	m_pStatFont3 = new CoolFont();
	m_pStatFont3->Init(m_pClientDE, "interface/fonts/StatFont2Hi.pcx");
	m_pStatFont3->LoadXWidths("blood2/interface/fonts/StatFont2.fnt");

	m_pStatFont4 = new CoolFont();
	m_pStatFont4->Init(m_pClientDE, "interface/fonts/MenuFont1.pcx");
	m_pStatFont4->LoadXWidths("blood2/interface/fonts/MenuFont1.fnt");

//	m_pStatFont4 = new CoolFont();
//	m_pStatFont4->Init(m_pClientDE, "interface/fonts/fixedsys8x14.pcx");
//	m_pStatFont4->CalcAll(8, 14);

	// Setup cursor with specific font
	m_pStatCursor = new CoolFontCursor();
	m_pStatCursor->SetFont(m_pStatFont2);
	m_pStatCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_pInfoCursor = new CoolFontCursor();
	m_pInfoCursor->SetFont(m_pStatFont1);
	m_pEquipCursor = new CoolFontCursor();
	m_pEquipCursor->SetFont(m_pStatFont2);

	// Setup transparency color
	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);

	// Init Key list
	m_hFirstKey.next = &m_hLastKey;
	m_hLastKey.prev = &m_hFirstKey;

	// Init tab locations
	AdjustRes();

	// Init inventory lists
	ClearItems();
	ClearSpells();
	SetupEquipList();

	return DTRUE;
}

//*************************************************************************
//*****	Function:	AdjustRes()
//*****	Details:	Sets new locations for status tabs if the game resolution changes
//*************************************************************************

void	CStatusBar::AdjustRes()
{
	if (!m_pClientDE) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &m_nScreenWidth, &m_nScreenHeight);

	m_nHealthX = 0;
	m_nArmorX = SB_STATTAB_SIZE_X;
	m_nAmmoX = m_nScreenWidth - SB_STATTAB_SIZE_X;
	m_nManaX = m_nAmmoX - SB_STATTAB_SIZE_X;
	m_nHealthY = m_nArmorY = m_nManaY = m_nAmmoY = m_nScreenHeight - SB_STATTAB_SIZE_Y;

	m_nItemX = SB_STATTAB_SIZE_X << 1;
	m_nSpellX = m_nScreenWidth - m_nItemX - SB_INVICON_SIZE_X;
	m_nInventoryY = m_nItemY = m_nSpellY = m_nScreenHeight - SB_INVICON_SIZE_Y;
	if(!m_bStatsOn) m_nInventoryY = m_nScreenHeight;

	m_nCommPicX = m_nCommPicY = 25;
	m_nCommPicStartY = -50;
	m_nCommTextX = 85;
	m_nCommTextY = 25;
	m_nCommTextWidth = (short)m_nScreenWidth - 25 - 85;

	m_pStatCursor->SetLoc((short)(m_nScreenWidth >> 1), (short)(m_nScreenHeight - 25));

	m_hFirstKey.currentY = (short)m_nScreenHeight - SB_STATTAB_SIZE_Y;

	m_nInfoX = (short)m_nScreenWidth - SB_INFOTAB_SIZE_X;

	m_nEquipX = (m_nScreenWidth >> 1) - (SB_EQUIP_SIZE_X >> 1);
	m_nEquipY = (m_nScreenHeight >> 1) - (SB_EQUIP_SIZE_Y >> 1);
}

//*************************************************************************
//*****	Function:	Term()
//*****	Details:	Terminates the status screens
//*************************************************************************

void	CStatusBar::Term()
{
	if (m_pClientDE)
	{
		if(m_hStatTabs)		{ m_pClientDE->DeleteSurface(m_hStatTabs); m_hStatTabs = 0; }
		if(m_hStatIcons)	{ m_pClientDE->DeleteSurface(m_hStatIcons); m_hStatIcons = 0; }
		if(m_hStatNums)		{ m_pClientDE->DeleteSurface(m_hStatNums); m_hStatNums = 0; }
		if(m_hItemIcons)	{ m_pClientDE->DeleteSurface(m_hItemIcons); m_hItemIcons = 0; }
		if(m_hSpellIcons)	{ m_pClientDE->DeleteSurface(m_hSpellIcons); m_hSpellIcons = 0; }
		if(m_hItemLight)	{ m_pClientDE->DeleteSurface(m_hItemLight); m_hItemLight = 0; }
		if(m_hSpellLight)	{ m_pClientDE->DeleteSurface(m_hSpellLight); m_hSpellLight = 0; }
		if(m_hKeyTabs)		{ m_pClientDE->DeleteSurface(m_hKeyTabs); m_hKeyTabs = 0; }
		if(m_hEquipTab)		{ m_pClientDE->DeleteSurface(m_hEquipTab); m_hEquipTab = 0; }
		if(m_hEquipCursor)	{ m_pClientDE->DeleteSurface(m_hEquipCursor); m_hEquipCursor = 0; }
		if(m_hCharFaces)	{ m_pClientDE->DeleteSurface(m_hCharFaces); m_hCharFaces = 0; }

		if(m_pStatCursor)	{ delete m_pStatCursor; m_pStatCursor = 0; }
		if(m_pInfoCursor)	{ delete m_pInfoCursor; m_pInfoCursor = 0; }
		if(m_pEquipCursor)	{ delete m_pEquipCursor; m_pEquipCursor = 0; }
		if(m_pStatFont1)	{ m_pStatFont1->Free(); delete m_pStatFont1; m_pStatFont1 = 0; }
		if(m_pStatFont2)	{ m_pStatFont2->Free(); delete m_pStatFont2; m_pStatFont2 = 0; }
		if(m_pStatFont3)	{ m_pStatFont3->Free(); delete m_pStatFont3; m_pStatFont3 = 0; }
		if(m_pStatFont4)	{ m_pStatFont4->Free(); delete m_pStatFont4; m_pStatFont4 = 0; }

		ClearKeys();
		ClearItems();
		ClearSpells();

		TermSpellbook();

		m_pClientDE = 0;
	}
}

//*************************************************************************
//*****	Function:	InitSpellbook()
//*****	Details:	Allocate and setup the spellbook interface
//*************************************************************************

DBOOL	CStatusBar::InitSpellbook()
{
	m_pClientDE->SetInputState (DFALSE);

	m_hSpellBG = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spellBG.pcx");
	m_hSpellsLo = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spellsLo.pcx");
	m_hSpellsHi = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spellsHi.pcx");
	m_hSpellSymbols = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spellSymbols.pcx");
	m_hSpellCursor = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spellCursor.pcx");
	m_hSpellDot = m_pClientDE->CreateSurfaceFromBitmap("interface/spellbook/spelldot.pcx");

	m_pSpellFont1 = new CoolFont();
	m_pSpellFont1->Init(m_pClientDE, "interface/fonts/SpellFont1.pcx");
	m_pSpellFont1->LoadXWidths("blood2/interface/fonts/SpellFont1.fnt");

	m_pSpellFont2 = new CoolFont();
	m_pSpellFont2->Init(m_pClientDE, "interface/fonts/SpellFont2.pcx");
	m_pSpellFont2->LoadXWidths("blood2/interface/fonts/SpellFont2.fnt");

	m_pSpellCursor = new CoolFontCursor();
	m_pSpellCursor->SetFont(m_pSpellFont1);

	m_bSpellbookOn = 1;

	return	1;
}

//*************************************************************************
//*****	Function:	TermSpellbook()
//*****	Details:	Release any memory used by the spellbook interface
//*************************************************************************

void	CStatusBar::TermSpellbook()
{
	m_bSpellbookOn = 0;

	if(m_hSpellBG)		{ m_pClientDE->DeleteSurface(m_hSpellBG); m_hSpellBG = 0; }
	if(m_hSpellsLo)		{ m_pClientDE->DeleteSurface(m_hSpellsLo); m_hSpellsLo = 0; }
	if(m_hSpellsHi)		{ m_pClientDE->DeleteSurface(m_hSpellsHi); m_hSpellsHi = 0; }
	if(m_hSpellSymbols)	{ m_pClientDE->DeleteSurface(m_hSpellSymbols); m_hSpellSymbols = 0; }
	if(m_hSpellCursor)	{ m_pClientDE->DeleteSurface(m_hSpellCursor); m_hSpellCursor = 0; }
	if(m_hSpellDot)		{ m_pClientDE->DeleteSurface(m_hSpellDot); m_hSpellDot = 0; }

	if(m_pSpellCursor)	{ delete m_pSpellCursor; m_pSpellCursor = 0; }
	if(m_pSpellFont1)	{ m_pSpellFont1->Free(); delete m_pSpellFont1; m_pSpellFont1 = 0; }
	if(m_pSpellFont2)	{ m_pSpellFont2->Free(); delete m_pSpellFont2; m_pSpellFont2 = 0; }
}

//*************************************************************************
//*****	Function:	InitCharScreen()
//*****	Details:	Allocate and setup the character screen interface
//*************************************************************************

DBOOL	CStatusBar::InitCharScreen()
{
	short	i;

	m_hCharFields = m_pClientDE->CreateSurfaceFromBitmap("interface/status/characterfields.pcx");

	m_pCharFont1 = new CoolFont();
	m_pCharFont1->Init(m_pClientDE, "interface/fonts/MenuFont1.pcx");
	m_pCharFont1->LoadXWidths("blood2/interface/fonts/MenuFont1.fnt");

	m_pCharCursor = new CoolFontCursor();
	m_pCharCursor->SetFont(m_pCharFont1);

	memset(m_pCSBoxes, 0, sizeof(CCharBox) * SB_NUM_CSBOXES);
	memset(m_pCSFieldList, 0, sizeof(CCharField) * SB_NUM_CSFIELDS);
	memset(&m_cfExtraPoints, 0, sizeof(CCharField));
	memset(m_rCSFields, 0, sizeof(DRect) * SB_NUM_BOXTYPES);

	for(i = 0; i < SB_NUM_CSBOXES; i++)
	{
		m_pCSBoxes[i].box = g_FieldBoxTypes[i];
		m_pCSBoxes[i].x = g_FieldBoxesXLoc[i];
		m_pCSBoxes[i].y = g_FieldBoxesYLoc[i];
	}

	m_pCSBoxes[0].list = g_FieldStatNames;
	m_pCSBoxes[1].list = g_BindingNames;
	m_pCSBoxes[2].list = g_WeaponNames;
	m_pCSBoxes[3].list = g_CharacterNames;
	m_pCSBoxes[4].list = g_SkinNames;
	m_pCSBoxes[5].list = g_ItemNames;
	m_pCSBoxes[6].list = g_SpellNames;
	m_pCSBoxes[7].list = g_FieldStatNames;
	m_pCSBoxes[8].list = g_FieldStatNames;
	m_pCSBoxes[9].list = g_FieldStatNames;
	m_pCSBoxes[10].list = g_FieldStatNames;
	m_pCSBoxes[11].list = g_FieldStatNames;

	m_pCSBoxes[1].max = 16;
	m_pCSBoxes[2].max = 33;
	m_pCSBoxes[3].max = 5;
	m_pCSBoxes[4].max = 4;
	m_pCSBoxes[5].max = 9;
	m_pCSBoxes[6].max = 15;
	m_pCSBoxes[7].max = 1;
	m_pCSBoxes[8].max = 1;
	m_pCSBoxes[9].max = 1;
	m_pCSBoxes[10].max = 1;
	m_pCSBoxes[11].max = 1;

	for(i = 0; i < SB_NUM_BOXTYPES; i++)
	{
		m_rCSFields[i].left = 0;
		m_rCSFields[i].right = SB_CHARFIELD_SIZE_X;
		m_rCSFields[i].top = g_FieldBoxesYPos[i];
		m_rCSFields[i].bottom = g_FieldBoxesYPos[i + 1];
	}

	for(i = 0; i < SB_NUM_CSFIELDS; i++)
	{
		m_pCSFieldList[i].type = g_FieldTypes[i];

//		if(m_bMultiInfo)
			m_pCSFieldList[i].active = g_FieldActiveM[i];
//		else
//			m_pCSFieldList[i].active = g_FieldActiveS[i];

		m_pCSFieldList[i].use = g_FieldUses[i];
		m_pCSFieldList[i].name = g_FieldNames[i];
		m_pCSFieldList[i].justify = g_FieldJustify[i];
		m_pCSFieldList[i].x = g_FieldXOffset[i];
		m_pCSFieldList[i].y = g_FieldYOffset[i];
	}

	m_pCSFieldList[0].num1 = m_pCSFieldList[0].num2 = 2;
	m_pCSFieldList[1].num1 = m_pCSFieldList[1].num2 = 2;
	m_pCSFieldList[2].num1 = m_pCSFieldList[2].num2 = 2;
	m_pCSFieldList[3].num1 = m_pCSFieldList[3].num2 = 1;

	m_cfExtraPoints.num1 = m_cfExtraPoints.num2 = 4;
	m_cfExtraPoints.x = 5;
	m_cfExtraPoints.y = 88;

	m_bCharScreenOn = 1;
	m_bCSCurrentField = 0;

	m_bCSInputState = 0;
	m_szCSInputString = 0;
	memset(m_szCSNameString, 0, SB_MAX_CS_STRING_SIZE);
	memset(m_szCSSaveFile, 0, SB_MAX_CS_STRING_SIZE);
	m_bCSInputChar = 0;
	m_bCSSelectState = 0;
	m_pCharFiles = 0;
	m_pCharFileIndex = 0;
	m_bCharFileNum = 0;
	return	1;
}

//*************************************************************************
//*****	Function:	TermCharScreen()
//*****	Details:	Release any memory used by the character screen interface
//*************************************************************************

void	CStatusBar::TermCharScreen()
{
	m_bCharScreenOn = 0;

	if(m_hCharFields)	{ m_pClientDE->DeleteSurface(m_hCharFields); m_hCharFields = 0; }

	if(m_pCharCursor)	{ delete m_pCharCursor; m_pCharCursor = 0; }
	if(m_pCharFont1)	{ m_pCharFont1->Free(); delete m_pCharFont1; m_pCharFont1 = 0; }
}

//*************************************************************************
//*****	Function:	SetupStatStrings(char **w, char numW, char **i, char numI, char **s, char numS)
//*****	Details:	
//*************************************************************************

/*void	CStatusBar::SetupStatStrings(char **w, char numW, char **i, char numI, char **s, char numS, char **b, char numB)
{
	m_hWeaponNames = w;
	m_nTotalWeapons = numW;
	m_hItemNames = i;
	m_nTotalItems = numI;
	m_hSpellNames = s;
	m_nTotalSpells = numS;
	m_hBindingNames = b;
	m_nTotalBindings = numB;
}*/

//*************************************************************************
//*****	Function:	AddKey(char type)
//*****	Details:	Adds a new key to the end of the current key list
//*************************************************************************

DBOOL	CStatusBar::AddKey(char type)
{
	CKeyTab		*key = m_hFirstKey.next;

	while(key->next)
	{
		if(key->type == type)	return	0;
		key = key->next;
	}

	key = new CKeyTab;
	key->prev = m_hLastKey.prev;
	key->next = &m_hLastKey;
	m_hLastKey.prev->next = key;
	m_hLastKey.prev = key;
	key->type = type;

	key->currentX = -SB_KEYTAB_SIZE_X;
	key->currentY = key->prev->currentY - SB_KEYTAB_SIZE_Y;
	key->locYTop = type * SB_KEYTAB_SIZE_Y;
	key->locYBottom = key->locYTop + SB_KEYTAB_SIZE_Y;
	key->startTime = m_pClientDE->GetTime();
	key->direction = 1;

	return	1;
}

//*************************************************************************
//*****	Function:	RemoveKey(char type)
//*****	Details:	Removes a specific key from anywhere in the current key list
//*************************************************************************

DBOOL	CStatusBar::RemoveKey(char type)
{
	CKeyTab		*set, *key = m_hFirstKey.next;
	DBOOL		removeKey = 0;
	DFLOAT		time;

	while(key->next)
	{
		if(key->type == type)	{	removeKey = 1; break;	}
		key = key->next;
	}

	if(removeKey)
	{
		set = key->next;
		key->prev->next = key->next;
		key->next->prev = key->prev;
		key->prev = key->next = 0;
		delete key;

		time = m_pClientDE->GetTime();
		while(set->next)
		{
			set->direction = 2;
			set->startTime = time;
			set = set->next;
		}
		return	1;
	}

	return	0;
}

//*************************************************************************
//*****	Function:	ClearKeys()
//*****	Details:	Removes all keys
//*************************************************************************

void	CStatusBar::ClearKeys()
{
	CKeyTab		*key = m_hFirstKey.next;

	while(key->next)
		RemoveKey(key->type);
}

//*************************************************************************
//*****	Function:	AddItem(char type)
//*****	Details:	Add a specific item to the player's inventory
//*************************************************************************

DBOOL	CStatusBar::AddItem(char type)
{
	CInventoryItem	*tempItem;
	char	i;

	for(i = 0; i < SB_NUM_ITEMS; i++)
		if(m_hItems[i].type == type)	{ m_hItems[i].amount++; return 1; }

	if(m_nNumItems == SB_NUM_ITEMS)	return 0;

	for(i = 0; i < SB_NUM_ITEMS; i++)
		if(!m_hItems[i].type)	{ tempItem = &(m_hItems[i]); break; }

	tempItem->type = type;
	tempItem->slot = i;
	tempItem->amount = 1;
	tempItem->locXLeft = type * SB_INVICON_SIZE_X;
	tempItem->locXRight = tempItem->locXLeft + SB_INVICON_SIZE_X;
	m_nNumItems++;
	m_hSelectedItem = tempItem;
	m_bCurrentItem = m_hSelectedItem->type;
	tempItem = 0;

	SetupEquipList();
	return	1;
}

//*************************************************************************
//*****	Function:	RemoveItem(char type)
//*****	Details:	Remove a specific item from the player's inventory
//*************************************************************************

DBOOL	CStatusBar::RemoveItem(char type)
{
	if(!m_nNumItems)	return	0;

	short	i;
	char	found = 0;

	for(i = 0; i < SB_NUM_ITEMS; i++)
		if(m_hItems[i].type == type)
		{
			if(m_hItems[i].amount > 1)	{ m_hItems[i].amount--; return 1; }

			memset(&(m_hItems[i]), 0, sizeof(CInventoryItem));
			m_hItems[i].locXRight = SB_INVICON_SIZE_X;
			m_nNumItems--;

			SetupEquipList();
			return	1;
		}

	return	0;
}

//*************************************************************************
//*****	Function:	ClearItems()
//*****	Details:	Removes all items in the player's inventory
//*************************************************************************

void	CStatusBar::ClearItems()
{
	char	i;

	memset(m_hItems, 0, sizeof(CInventoryItem) * SB_NUM_ITEMS);

	for(i = 0; i < SB_NUM_ITEMS; i++)
	{
		m_hItems[i].slot = i;
		m_hItems[i].locXRight = SB_INVICON_SIZE_X;
		m_hItemList[i] = &(m_hItems[i]);
	}

	for(i = 4; i < SB_NUM_ITEMS; i++)
	{
		m_hItems[i].type = (INV_PROXIMITY + i - 4);
		m_hItems[i].amount = 0;
		m_hItems[i].locXLeft = (INV_PROXIMITY + i - 4) * SB_INVICON_SIZE_X;
		m_hItems[i].locXRight = m_hItems[i].locXLeft + SB_INVICON_SIZE_X;
		m_nNumItems++;
	}

	m_hSelectedItem = m_hItemList[0];
	m_bCurrentItem = m_hSelectedItem->type;
	m_nNumItems = 0;
}

//*************************************************************************
//*****	Function:	AddSpell(char type)
//*****	Details:	Add a spell to the player's spell book
//*************************************************************************
/*
DBOOL	CStatusBar::AddSpell(char type)
{
	CInventoryItem	*tempSpell;
	char	i;

	if(m_nNumSpells == SB_NUM_SPELLS)	return 0;

	for(i = 0; i < SB_NUM_SPELLS; i++)
		if(m_hSpells[i].type == type)	return 0;

	for(i = 0; i < SB_NUM_SPELLS; i++)
		if(!m_hSpells[i].type)	{ tempSpell = &(m_hSpells[i]); break; }

	tempSpell->type = type;
	tempSpell->slot = i;
	tempSpell->amount = 1;
	tempSpell->locXLeft = type * SB_INVICON_SIZE_X;
	tempSpell->locXRight = tempSpell->locXLeft + SB_INVICON_SIZE_X;
	m_nNumSpells++;
	m_hSelectedSpell = tempSpell;
	m_bCurrentItem = INV_BASESPELL + m_hSelectedSpell->type - 1;
	tempSpell = 0;

	SetupEquipList();
	if(type)	m_pCurrentSpells[type-1] = 1;
	return	1;
}
*/

//*************************************************************************
//*****	Function:	RemoveSpell(char type)
//*****	Details:	Removes a spell from the player's spell book
//*************************************************************************

DBOOL	CStatusBar::RemoveSpell(char type)
{
	if(!m_nNumSpells)	return	0;

	short	i;
	char	found = 0;

	for(i = 0; i < SB_NUM_SPELLS; i++)
		if(m_hSpellList[i]->type == type)
		{
			memset(m_hSpellList[i], 0, sizeof(CInventoryItem));
			m_hSpellList[i]->locXRight = SB_INVICON_SIZE_X;
			m_nNumSpells--;
			if(type)	m_pCurrentSpells[type-1] = 0;

			SetupEquipList();
			return	1;
		}

	return	0;
}

//*************************************************************************
//*****	Function:	ClearSpells()
//*****	Details:	Removes all spells in the player's spell book
//*************************************************************************

void	CStatusBar::ClearSpells()
{
	char	i;

	memset(m_hSpells, 0, sizeof(CInventoryItem) * SB_NUM_SPELLS);

	for(i = 0; i < SB_NUM_SPELLS; i++)
	{
		m_hSpells[i].slot = i;
		m_hSpells[i].locXRight = SB_INVICON_SIZE_X;
		m_hSpellList[i] = &(m_hSpells[i]);
	}

	m_hSelectedSpell = m_hSpellList[0];
	m_nNumSpells = 0;
	memset(m_pCurrentSpells, 0, SB_NUM_SINGLEPLAYER_SPELLS + 1);
}

//*************************************************************************
//*****	Function:	SetupEquipList(char *weapons, char *items, char *spells, char spellOffset)
//*****	Details:	Fill in the equipment list with all correct values
//*************************************************************************

void	CStatusBar::SetupEquipList()
{
	char	i;

	m_pEquipList[0].type = 0; m_pEquipList[0].name = 0; m_pEquipList[0].offset = 0;
	m_pEquipList[1].type = 1; m_pEquipList[1].name = WEAP_MELEE; m_pEquipList[1].offset = 25;
	for(i = 2; i < 11; i++)
		{ m_pEquipList[i].type = 1; m_pEquipList[i].name = m_hWeapons[i-1]; m_pEquipList[i].offset = 25; }

	m_pEquipList[11].type = 0; m_pEquipList[11].name = 1; m_pEquipList[11].offset = 0;
	for(i = 12; i < 16; i++)
		{ m_pEquipList[i].type = 2; m_pEquipList[i].name = m_hItems[i-12].type; m_pEquipList[i].offset = 25; }
	for(i = 16; i < 20; i++)
		{ m_pEquipList[i].type = 3; m_pEquipList[i].name = m_hItems[i-12].type; m_pEquipList[i].offset = 25; }

	m_pEquipList[20].type = 0; m_pEquipList[20].name = 2; m_pEquipList[20].offset = 0;
	for(i = 21; i < 27; i++)
		{ m_pEquipList[i].type = 4;	m_pEquipList[i].name = m_hSpells[i-21].type; m_pEquipList[i].offset = 25; }
}

//*************************************************************************
//*****	Function:	HandleKeyDown()
//*****	Details:	Handle the in-game interface control
//*************************************************************************

DBOOL	CStatusBar::HandleKeyDown(int key)
{
	switch(key)
	{
		case	VK_F1:		// turn on information screen
			m_bInfoOn = !m_bInfoOn;
			return	1;

/*		case	VK_TAB:		// turn on the big inventory screen
			m_bEquipOn = 1;
			m_bInventoryOn = 0;
			return	1;
*/
		case	VK_ADD:		// turn on more stats
		case	0x00BB:
			if(m_bStatsOn < 4) m_bStatsOn++;
			m_nInventoryY = m_nScreenHeight - SB_INVICON_SIZE_Y;
			return	1;

		case	VK_SUBTRACT:// turn off more stats
		case	0x00BD:
			if(m_bStatsOn > 0) m_bStatsOn--;
			if(!m_bStatsOn) m_nInventoryY = m_nScreenHeight + SB_HIGHLIGHT_OFFSET_Y;
			return	1;

/*		case	VK_W:		// Spell cycle right
			if(m_bInventoryOn != 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryOn = 2; m_bInventoryState = 0;	}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 4;	}
			return	1;

		case	VK_Q:		// Spell cycle left
			if(m_bInventoryOn != 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryOn = 2; m_bInventoryState = 0;	}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 3;	}
			return	1;

		case	0xDD:		// ']' = Item cycle right
			if(m_bInventoryOn != 1)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryOn = 1; m_bInventoryState = 0;	}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 4;	}
			return	1;

		case	0xDB:		// '[' = Item cycle left
			if(m_bInventoryOn != 1)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryOn = 1; m_bInventoryState = 0;	}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 3;	}
			return	1;

		case	VK_E:
			if(!m_bInventoryOn)
			{
				if(m_hSelectedSpell->type)
					{	m_bCurrentItem = m_hSelectedSpell->slot; return	4;	}
				else
					m_bCurrentItem = 0;
			}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 5; return 2;	}
			return	1;

		case	VK_RETURN:
			if(!m_bInventoryOn)
			{
				m_bCurrentItem = m_hSelectedItem->slot;
				if(m_hSelectedItem->type == INV_SPELLBOOK)	{	InitSpellbook(); return 1;	}
				return 3;
			}
			else if(m_bInventoryState == 2)
				{	m_fInvStartTime = m_pClientDE->GetTime(); m_bInventoryState = 5; return 2;	}
			return	1;
*/
//		case	VK_F5:
//			InitCharScreen();
//			m_pClientDE->SetInputState (DFALSE);
//			return	1;

		case	VK_DELETE:
			return	5;
	}
	return	0;
}

//*************************************************************************
//*****	Function:	HandleSpellbookControl(int key)
//*****	Details:	
//*************************************************************************
/*
DBOOL	CStatusBar::HandleSpellbookControl(int key)
{
	switch(key)
	{
		case	VK_ESCAPE:
			TermSpellbook();
			m_pClientDE->SetInputState (DTRUE);
			return	1;

		case	VK_DOWN:
		case	VK_RIGHT:
		case	VK_W:
		case	0xDD:		// Move down the list of spells
			m_bCurrentSpell++;
			if(m_bCurrentSpell > SB_NUM_SINGLEPLAYER_SPELLS)	m_bCurrentSpell = 0;
			return	1;

		case	VK_UP:
		case	VK_LEFT:
		case	VK_Q:
		case	0xDB:		// Move up the list of spells
			m_bCurrentSpell--;
			if(m_bCurrentSpell < 0)	m_bCurrentSpell = SB_NUM_SINGLEPLAYER_SPELLS;
			return	1;

		case	VK_DELETE:	// Remove spell
			if(m_pCurrentSpells[g_SpellNameFix[m_bCurrentSpell]])
			{
				m_bCurrentItem = INV_BASESPELL + g_SpellNameFix[m_bCurrentSpell];
				return	3;
			}
			return	1;

		case	VK_RETURN:	// Memorize a spell
			if(!m_pCurrentSpells[g_SpellNameFix[m_bCurrentSpell]] && (m_bSpellLevel >= g_SpellLevel[g_SpellNameFix[m_bCurrentSpell]]) && (m_nNumSpells < SB_NUM_SPELLS))
			{
				m_bCurrentItem = INV_BASESPELL + g_SpellNameFix[m_bCurrentSpell];
				TermSpellbook();
				m_pClientDE->SetInputState (DTRUE);
				return	2;
			}
			return	1;
	}
	return	0;
}
*/

//*************************************************************************
//*****	Function:	HandleEquipControl(int key)
//*****	Details:	
//*************************************************************************

DBOOL	CStatusBar::HandleEquipControl(int key)
{
	switch(key)
	{
		case	VK_TAB:		// turn off the big inventory screen
			m_bEquipOn = 0;
			return	1;

		case	VK_W:
		case	0xDD:		// Move down the list of equipment
		{
			char	i = 0;
			if(m_bSelectedEquipItem < SB_NUM_EQUIP_ITEMS - 1) { m_bSelectedEquipItem++; i++; }
			if(!m_pEquipList[m_bSelectedEquipItem].type) { m_bSelectedEquipItem++; i++; }
			if(m_bSelectedEquipItem > m_bTopEquipItem + SB_NUM_EQUIP_DISP_ITEMS - 1)
				m_bTopEquipItem += i;
			return	1;
		}

		case	VK_Q:
		case	0xDB:		// Move up the list of equipment
			if(m_bSelectedEquipItem == 1) { m_bTopEquipItem = 0; return 1; }
			if(m_bSelectedEquipItem > 1) m_bSelectedEquipItem--;
			if(!m_pEquipList[m_bSelectedEquipItem].type) m_bSelectedEquipItem--;
			if(m_bSelectedEquipItem < m_bTopEquipItem)
				m_bTopEquipItem = m_bSelectedEquipItem;
			return	1;

		case	VK_DELETE:	// Remove item / spell / weapon
			switch(m_pEquipList[m_bSelectedEquipItem].type)
			{
				case	1:
					if(m_pEquipList[m_bSelectedEquipItem].name)
						{	m_bCurrentItem = m_bSelectedEquipItem - 1; return	2;	}
					break;
				case	2:
					if(m_pEquipList[m_bSelectedEquipItem].name)
						{	m_bCurrentItem = m_bSelectedEquipItem - 12; return	3;	}
					break;
				case	3:
					break;
				case	4:
					/*
					if(m_pEquipList[m_bSelectedEquipItem].name)
						{	m_bCurrentItem = INV_BASESPELL + m_pEquipList[m_bSelectedEquipItem].name - 1; return 4;	}
					*/
					break;
			}
			return	1;

		case	VK_RETURN:	// Ready item / spell / weapon
			switch(m_pEquipList[m_bSelectedEquipItem].type)
			{
				case	1:
					if(m_pEquipList[m_bSelectedEquipItem].name)
						{	m_bCurrentItem = m_bSelectedEquipItem - 1; return	5;	}
					break;
				case	2:
					m_hSelectedItem = &(m_hItems[m_bSelectedEquipItem - 12]);
					break;
				case	3:
					m_hSelectedItem = &(m_hItems[m_bSelectedEquipItem - 12]);
					m_bCurrentItem = m_hSelectedItem->slot;
					return	6;
				case	4:
					m_hSelectedSpell = &(m_hSpells[m_bSelectedEquipItem - 21]);
					break;
			}
			return	1;
	}
	return	0;
}

//*************************************************************************
//*****	Function:	HandleCharScreenControl(int key)
//*****	Details:	
//*************************************************************************

DBOOL	CStatusBar::HandleCharScreenControl(int key)
{
	if(m_bCSSelectState)
	{
		switch(key)
		{
			case	VK_ESCAPE:
				m_bCSSelectState = 0;
				m_pCharFileIndex = 0;
				m_pClientDE->FreeFileList(m_pCharFiles);
				m_pCharFiles = 0;
				return	1;

			case	VK_RETURN:
				if(m_bCSSelectState == 2)	LoadB2CFile(m_pCharFileIndex);
				if(m_bCSSelectState == 3)	DeleteB2CFile(m_pCharFileIndex);

				m_bCSSelectState = 0;
				m_pCharFileIndex = 0;
				m_pClientDE->FreeFileList(m_pCharFiles);
				m_pCharFiles = 0;
				return	1;

			case	VK_UP:
			case	VK_LEFT:
				return	HandleCharScreenLeft();

			case	VK_DOWN:
			case	VK_RIGHT:
				return	HandleCharScreenRight();
		}
		return	0;
	}

	if(m_bCSInputState)
	{
		if(m_bCSInputState == 1)	m_szCSInputString = m_szCSNameString;
			else					m_szCSInputString = m_szCSSaveFile;

		if(m_bCSInputChar < SB_MAX_CS_STRING_SIZE - 1)
		{
			if((key >= VK_0) && (key <= VK_9))
			{
				m_szCSInputString[m_bCSInputChar - 1] = key;
				m_szCSInputString[m_bCSInputChar++] = '_';
				m_szCSInputString[m_bCSInputChar] = 0;
				return	1;
			}

			if((key >= VK_A) && (key <= VK_Z))
			{
				m_szCSInputString[m_bCSInputChar - 1] = key;
				m_szCSInputString[m_bCSInputChar++] = '_';
				m_szCSInputString[m_bCSInputChar] = 0;
				return	1;
			}

			if(key == VK_SPACE)
			{
				m_szCSInputString[m_bCSInputChar - 1] = ' ';
				m_szCSInputString[m_bCSInputChar++] = '_';
				m_szCSInputString[m_bCSInputChar] = 0;
				return	1;
			}
		}

		if((key == VK_BACK) && (m_bCSInputChar > 1))
		{
			m_bCSInputChar--;
			m_szCSInputString[m_bCSInputChar] = 0;
			m_szCSInputString[m_bCSInputChar - 1] = '_';
			return	1;
		}

		if(key == VK_RETURN)
		{
			m_bCSInputChar--;
			m_szCSInputString[m_bCSInputChar] = 0;
			if(m_bCSInputState == 2)	SaveB2CFile();

			m_bCSInputState = 0;
//			m_bCSInputChar = 0;
			m_szCSInputString = 0;
			return	1;
		}

		if(key == VK_ESCAPE)
		{
			m_bCSInputState = 0;
			memset(m_szCSInputString, 0, SB_MAX_CS_STRING_SIZE);
			m_szCSInputString = 0;
//			m_bCSInputChar = 0;
			return	1;
		}
		return	0;
	}

	switch(key)
	{
		case	VK_DOWN:
		case	VK_W:
			do
			{
				m_bCSCurrentField++;
				if(m_bCSCurrentField >= SB_NUM_CSFIELDS)	m_bCSCurrentField = 0;
			}
			while(!(m_pCSFieldList[m_bCSCurrentField].active));
			return	1;

		case	VK_UP:
		case	VK_Q:
			do
			{
				m_bCSCurrentField--;
				if(m_bCSCurrentField < 0)	m_bCSCurrentField = SB_NUM_CSFIELDS - 1;
			}
			while(!(m_pCSFieldList[m_bCSCurrentField].active));
			return	1;

		case	VK_RIGHT:
		case	0xDD:
			return	HandleCharScreenRight();

		case	VK_LEFT:
		case	0xDB:
			return	HandleCharScreenLeft();

		case	VK_ESCAPE:
			if(m_bCSSelectState)
			{
				m_bCSSelectState = 0;
				LoadB2CFile(m_pCharFileIndex);
				m_pCharFileIndex = 0;
				m_pClientDE->FreeFileList(m_pCharFiles);
				m_pCharFiles = 0;
			}
			else
			{
				TermCharScreen();
				m_pClientDE->SetInputState(DTRUE);
			}
			return	1;

		case	VK_RETURN:
			switch(m_pCSFieldList[m_bCSCurrentField].use)
			{
				case	SB_CSFIELD_NEWNAME:
					m_bCSInputState = 1;
					memset(m_szCSNameString, 0, SB_MAX_CS_STRING_SIZE);
					m_szCSNameString[0] = '_';
					m_bCSInputChar = 1;
					break;
				case	SB_CSFIELD_LOADSKIN:
					break;
				case	SB_CSFIELD_SAVECHAR:
					m_bCSInputState = 2;
					memset(m_szCSSaveFile, 0, SB_MAX_CS_STRING_SIZE);
					m_szCSSaveFile[0] = '_';
					m_bCSInputChar = 1;
					break;
				case	SB_CSFIELD_LOADCHAR:
 					m_bCSSelectState = 2;
					m_bCharFileNum = 0;
					if(m_pCharFiles)	m_pClientDE->FreeFileList(m_pCharFiles);
					m_pCharFiles = m_pClientDE->GetFileList("B2CFiles");
					m_pCharFileIndex = m_pCharFiles;
					break;
				case	SB_CSFIELD_DELETECHAR:
					m_bCSSelectState = 3;
					m_bCharFileNum = 0;
					if(m_pCharFiles)	m_pClientDE->FreeFileList(m_pCharFiles);
					m_pCharFiles = m_pClientDE->GetFileList("B2CFiles");
					m_pCharFileIndex = m_pCharFiles;
					break;
				case	SB_CSFIELD_ACCEPT:
					TermCharScreen();
					m_pClientDE->SetInputState(DTRUE);
					return	2;
				case	SB_CSFIELD_CANCEL:
					TermCharScreen();
					m_pClientDE->SetInputState(DTRUE);
					break;
			}
			return	1;
	}
	return	0;
}

//*************************************************************************
//*****	Function:	HandleCharScreenLeft()
//*****	Details:	
//*************************************************************************

char	CStatusBar::HandleCharScreenLeft()
{
	char	use = m_pCSFieldList[m_bCSCurrentField].use;

	switch(use)
	{
		case	SB_CSFIELD_VALUE:
			if((m_bCSCurrentField != 3) && (m_pCSFieldList[m_bCSCurrentField].num2 == 1))
				return	1;
			if(m_pCSFieldList[m_bCSCurrentField].num2 > 0)
			{
				m_pCSFieldList[m_bCSCurrentField].num2--;
				m_cfExtraPoints.num2++;
			}
			return	1;

		case	SB_CSFIELD_SELECT:
		{
			char	type = m_pCSFieldList[m_bCSCurrentField].type;
			char	i, start, num, selected;

			switch(type)
			{
				case	1:	start = 4; num = 3;		break;		// Binding list
				case	2:	start = 7; num = 10;	break;		// Weapon list
				case	5:	start = 19; num = 4;	break;		// Item list
				case	6:	start = 23; num = 6;	break;		// Spell list
			}

			selected = m_pCSFieldList[m_bCSCurrentField].name;
			while(selected > 0)
			{
				selected--;
				for(i = start; i < start + num; i++)
					if(selected && (selected == m_pCSFieldList[i].name))	break;

				if(i == start + num)
					{	m_pCSFieldList[m_bCSCurrentField].name = selected; break;	}
			}
			return	1;
		}

		case	SB_CSFIELD_NEWNAME:
			return	1;

		case	SB_CSFIELD_LOADCHAR:
		case	SB_CSFIELD_DELETECHAR:
			if(m_bCSSelectState && (m_bCharFileNum > 0))
			{
				m_bCharFileNum--;
				m_pCharFileIndex = m_pCharFiles;
				for(char i = 0; i < m_bCharFileNum; i++)
					m_pCharFileIndex = m_pCharFileIndex->m_pNext;
			}
			return	1;
	}
	return	0;
}

//*************************************************************************
//*****	Function:	HandleCharScreenRight()
//*****	Details:	
//*************************************************************************

char	CStatusBar::HandleCharScreenRight()
{
	char	use = m_pCSFieldList[m_bCSCurrentField].use;

	switch(use)
	{
		case	SB_CSFIELD_VALUE:
			if(m_cfExtraPoints.num2 > 0) 
			{
				if((m_bCSCurrentField == 3) && (m_pCSFieldList[m_bCSCurrentField].num2 == 4))
					return	1;
				if(m_pCSFieldList[m_bCSCurrentField].num2 < 5)
				{
					m_pCSFieldList[m_bCSCurrentField].num2++;
					m_cfExtraPoints.num2--;
				}
			}
			return	1;

		case	SB_CSFIELD_SELECT:
		{
			char	type = m_pCSFieldList[m_bCSCurrentField].type;
			char	i, start, num, selected;

			switch(type)
			{
				case	1:	start = 4; num = 3;		break;		// Binding list
				case	2:	start = 7; num = 10;	break;		// Weapon list
				case	5:	start = 19; num = 4;	break;		// Item list
				case	6:	start = 23; num = 6;	break;		// Spell list
			}

			selected = m_pCSFieldList[m_bCSCurrentField].name;
			while(selected < m_pCSBoxes[type].max - 1)
			{
				selected++;
				for(i = start; i < start + num; i++)
					if(selected == m_pCSFieldList[i].name)		break;

				if(i == start + num)
					{	m_pCSFieldList[m_bCSCurrentField].name = selected; break;	}
			}
			return	1;
		}

		case	SB_CSFIELD_NEWNAME:
			return	1;

		case	SB_CSFIELD_LOADCHAR:
		case	SB_CSFIELD_DELETECHAR:
			if(m_bCSSelectState && m_pCharFileIndex->m_pNext)
			{
				m_bCharFileNum++;
				m_pCharFileIndex = m_pCharFiles;
				for(char i = 0; i < m_bCharFileNum; i++)
					m_pCharFileIndex = m_pCharFileIndex->m_pNext;
			}
			return	1;
	}
	return	0;
}

//*************************************************************************
//*****	Function:	StartCommunication(char chosen, char *szFile, char *szText)
//*****	Details:	Draws all active in-game interfaces
//*************************************************************************

char	CStatusBar::StartCommunication(char chosen, char *szFile, char *szText)
{
	if(m_bCommState)		return	0;

	m_bCommState = 1;
	m_bCommPic = chosen;
	memset(m_szCommVoice, 0, 100);
	strcpy(m_szCommVoice, szFile);
	memset(m_szCommText, 0, 200);
	strcpy(m_szCommText, szText);
	m_fCommLength = strlen(szText) / 15.0f;
	if(m_fCommLength < 1.5f)	m_fCommLength = 1.5f;
	if( m_sCommSound )
	{
		g_pClientDE->KillSound( m_sCommSound );
		m_sCommSound = 0;
	}

	if(szFile && strcmp(szFile, "default.wav"))
		m_bPlayVoice = 1;
	else
		m_bPlayVoice = 0;

	m_fCommStartTime = m_pClientDE->GetTime();
	return	1;
}

//*************************************************************************
//*****	Function:	Draw()
//*****	Details:	Draws all active in-game interfaces
//*************************************************************************

void	CStatusBar::Draw(DBOOL bDrawBar)
{
	if (!m_pClientDE) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	// Always draw this.. GK 9/1
	if(m_bCommState)	DrawCommunication(hScreen);

	// this stuff depends on whether the status bar is visible..
	if (bDrawBar)
	{
		if(m_bCharScreenOn)	{ DrawCharacterScreen(hScreen); return; }
		if(m_bSpellbookOn)	{ DrawSpellbook(hScreen); return; }
		if(m_bStatsOn)	DrawStatTabs(hScreen);
		if(m_bKeysOn)	DrawKeyTabs(hScreen);
		if(m_bInfoOn)	DrawInfoTab(hScreen);

		switch(m_bInventoryOn)
		{
			case	0:	if(m_bStatsOn)	DrawItemTabs(hScreen);	break;
			case	1:	HandleItemInventory(hScreen);	break;
			// case	2:	HandleSpellInventory(hScreen);	break;
		}

		if(m_bEquipOn)	DrawEquipTab(hScreen);
	}
}

//*************************************************************************
//*****	Function:	DrawCommunication(HSURFACE hScreen)
//*****	Details:	Draws the communication link at the top left
//*************************************************************************

void	CStatusBar::DrawCommunication(HSURFACE hScreen)
{
	DRect	locRect;
	DFLOAT	currentTime = m_pClientDE->GetTime();
	DFLOAT	ratio, dif;
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);

	locRect.top = 0;
	locRect.bottom = SB_CHARPIC_SIZE;
	locRect.left = SB_CHARPIC_SIZE * m_bCommPic;
	locRect.right = locRect.left + SB_CHARPIC_SIZE;

	switch(m_bCommState)
	{
		case	1:
			dif = currentTime - m_fCommStartTime;
			if(dif > SB_COMMLINK_TIME_1)
			{
				m_bCommState = 2;
				m_fCommStartTime = currentTime;
				ratio = 0.0;
			}
			else
				ratio = 1.0f - (dif / SB_COMMLINK_TIME_1);

			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFaces, &locRect, m_nCommPicX, (DDWORD)(m_nCommPicY - ((m_nCommPicY - m_nCommPicStartY) * ratio)));
			break;

		case	2:
			if(currentTime - m_fCommStartTime > SB_COMMLINK_TIME_2)
			{
				m_bCommState = 3;

				if(m_bPlayVoice)
				{
					if( m_sCommSound )
					{
						g_pClientDE->KillSound( m_sCommSound );
						m_sCommSound = 0;
					}
					m_sCommSound = PlaySoundLocal(m_szCommVoice, SOUNDPRIORITY_MISC_HIGH,
												  DFALSE, DTRUE, DTRUE, DFALSE, 100);
				}
				m_fCommStartTime = currentTime;
			}

			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFaces, &locRect, m_nCommPicX, m_nCommPicY);
			break;

		case	3:
			if(!m_sCommSound || !m_bPlayVoice)
			{
				if(currentTime - m_fCommStartTime > m_fCommLength)
				{
					m_bCommState = 4;
					m_fCommStartTime = currentTime;
				}
			}
			else if(m_bPlayVoice && m_sCommSound && m_pClientDE->IsDone(m_sCommSound))
			{
				m_bCommState = 4;
				m_pClientDE->KillSound(m_sCommSound);
				m_sCommSound = DNULL;
				m_fCommStartTime = currentTime;
			}

			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFaces, &locRect, m_nCommPicX, m_nCommPicY);
			m_pInfoCursor->SetFont(m_pStatFont4);
			m_pInfoCursor->SetDest(hScreen);
			m_pInfoCursor->SetJustify(CF_JUSTIFY_LEFT);
			m_pInfoCursor->SetLoc((short)m_nCommTextX, (short)m_nCommTextY);
			m_pInfoCursor->DrawSolidFormat(m_szCommText, m_nCommTextWidth, white);
			break;
		case	4:
			if(currentTime - m_fCommStartTime > SB_COMMLINK_TIME_2)
			{
				m_bCommState = 5;
				m_fCommStartTime = currentTime;
			}

			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFaces, &locRect, m_nCommPicX, m_nCommPicY);
			break;
		case	5:
			dif = currentTime - m_fCommStartTime;
			if(dif > SB_COMMLINK_TIME_1)
			{
				m_bCommState = 0;
				m_fCommStartTime = currentTime;
				ratio = 1.0;
			}
			else
				ratio = (dif / SB_COMMLINK_TIME_1);

			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFaces, &locRect, m_nCommPicX, (DDWORD)(m_nCommPicY - ((m_nCommPicY - m_nCommPicStartY) * ratio)));
			break;
	}
}

//*************************************************************************
//*****	Function:	DrawStatTabs(HSURFACE hScreen)
//*****	Details:	Draws the stat tabs at the bottom of the screen
//*************************************************************************

void	CStatusBar::DrawStatTabs(HSURFACE hScreen)
{
	DRect	locRect;
	DFLOAT	currentTime = m_pClientDE->GetTime();

	locRect.top = 0;
	locRect.left = 0;
	locRect.right = SB_STATTAB_SIZE_X;
	locRect.bottom = SB_STATTAB_SIZE_Y;

	if(m_bStatsOn > 3)
	{
		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hStatTabs, &locRect, m_nHealthX, m_nHealthY);
		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hStatTabs, &locRect, m_nArmorX, m_nArmorY);

		locRect.left = SB_STATTAB_SIZE_X;
		locRect.right += SB_STATTAB_SIZE_X;

//		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hStatTabs, &locRect, m_nManaX, m_nManaY);
		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hStatTabs, &locRect, m_nAmmoX, m_nAmmoY);
	}

	locRect.bottom = SB_STATICON_SIZE_Y;

	if(m_bStatsOn > 2)
	{
		switch(m_nHeartFrame)
		{
			case	0:	if(currentTime - m_fHeartTime > m_fHeartBeat)
							{ m_nHeartFrame++; m_fHeartTime = currentTime; }
						break;
			case	1:
			case	2:	if(currentTime - m_fHeartTime > SB_HEARTBEAT_TIME_2)
							{ m_nHeartFrame++; m_fHeartTime = currentTime; }
						break;
			case	3:	if(currentTime - m_fHeartTime > SB_HEARTBEAT_TIME_2)
							{ m_nHeartFrame = 0; m_fHeartTime = currentTime; }
						break;
		}

		locRect.left = SB_HEALTHICON_LOC_X + m_nHeartFrame * SB_STATICON_SIZE_X; locRect.right = locRect.left + SB_STATICON_SIZE_X;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hStatIcons, &locRect, m_nHealthX + SB_LEFTICON_OFFSET_X, m_nHealthY + SB_ICON_OFFSET_Y, m_hTransColor);

		locRect.left = SB_ARMORICON_LOC_X; locRect.right = locRect.left + SB_STATICON_SIZE_X;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hStatIcons, &locRect, m_nArmorX + SB_LEFTICON_OFFSET_X, m_nArmorY + SB_ICON_OFFSET_Y, m_hTransColor);
//		locRect.left = SB_MANAICON_LOC_X; locRect.right = locRect.left + SB_STATICON_SIZE_X;
//		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hStatIcons, &locRect, m_nManaX + SB_RIGHTICON_OFFSET_X, m_nManaY + SB_ICON_OFFSET_Y, m_hTransColor);
		locRect.left = SB_AMMOICON_LOC_X; locRect.right = locRect.left + SB_STATICON_SIZE_X;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hStatIcons, &locRect, m_nAmmoX + SB_RIGHTICON_OFFSET_X, m_nAmmoY + SB_ICON_OFFSET_Y, m_hTransColor);
	}

	if(m_bStatsOn > 1)
	{
		DrawNumber(m_nHealth, m_nHealthX + SB_LEFTNUM_OFFSET_X, m_nHealthY + SB_NUM_OFFSET_Y);
		DrawNumber(m_nArmor, m_nArmorX + SB_LEFTNUM_OFFSET_X, m_nArmorY + SB_NUM_OFFSET_Y);
//		DrawNumber(m_nMana, m_nManaX + SB_RIGHTNUM_OFFSET_X, m_nManaY + SB_NUM_OFFSET_Y);
		DrawNumber(m_nAmmo, m_nAmmoX + SB_RIGHTNUM_OFFSET_X, m_nAmmoY + SB_NUM_OFFSET_Y);
	}
}

//*************************************************************************
//*****	Function:	DrawItemAndNameTabs(HSURFACE hScreen, DDWORD yPos)
//*****	Details:	Draws the selected item and spell tabs + names at the bottom of the screen
//*************************************************************************

void	CStatusBar::DrawItemTabs(HSURFACE hScreen)
{
/*	DRect	locRect;
	m_pStatCursor->SetDest(hScreen);

	locRect.top = 0;
	locRect.bottom = SB_INVICON_SIZE_Y;

	locRect.left = m_hSelectedItem->locXLeft;
	locRect.right = m_hSelectedItem->locXRight;

	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, m_nItemX, m_nItemY);

	locRect.left = m_hSelectedSpell->locXLeft;
	locRect.right = m_hSelectedSpell->locXRight;

	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, m_nSpellX, m_nSpellY);

	if(m_bInventoryOn == 1)		m_pStatCursor->Draw(g_ItemNames[m_hItemList[SB_SELECTED_ITEM]->type]);
	if(m_bInventoryOn == 2)		m_pStatCursor->Draw(g_SpellNames[m_hSpellList[SB_SELECTED_ITEM]->type]);
*/}

//*************************************************************************
//*****	Function:	DrawHighlight(HSURFACE hScreen, char type)
//*****	Details:	Draws the item or spell highlight
//*************************************************************************

void	CStatusBar::DrawHighlight(HSURFACE hScreen, char type)
{
/*	DRect	locRect;
	locRect.top = 0;
	locRect.bottom = SB_HIGHLIGHT_SIZE_Y;
	locRect.left = SB_HIGHLIGHT_SIZE_X * m_nHighlightFrame;
	locRect.right = locRect.left + SB_HIGHLIGHT_SIZE_X;

	if(type)
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSpellLight, &locRect, m_nSpellX + SB_HIGHLIGHT_OFFSET_X, m_nInventoryY + SB_HIGHLIGHT_OFFSET_Y - SB_INVICON_SIZE_Y, m_hTransColor);
	else
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hItemLight, &locRect, m_nItemX + SB_HIGHLIGHT_OFFSET_X, m_nInventoryY + SB_HIGHLIGHT_OFFSET_Y - SB_INVICON_SIZE_Y, m_hTransColor);

	if(m_nHighlightFrame < SB_NUM_HIGHLIGHTS)
		m_nHighlightFrame++;
	else
		m_nHighlightFrame = 0;
*/}

//*************************************************************************
//*****	Function:	DrawKeyTabs()
//*****	Details:	Draws the key tabs at the left of the screen
//*************************************************************************

void	CStatusBar::DrawKeyTabs(HSURFACE hScreen)
{
	CKeyTab		*key = m_hFirstKey.next;
	DFLOAT		currentTime = m_pClientDE->GetTime();
	DFLOAT		dif, ratio;
	DRect	locRect;

	locRect.left = 0;
	locRect.right = SB_KEYTAB_SIZE_X;

	// Start drawing the key tabs
	while(key->next)
	{
		locRect.top = key->locYTop;
		locRect.bottom = key->locYBottom;

		switch(key->direction)
		{
			case	1:
				dif = currentTime - key->startTime;
				if(dif > SB_KEYTAB_TIME)	{	ratio = 0.0; key->direction = 0;	}
					else	ratio = (dif / SB_KEYTAB_TIME) - 1.0f;
				key->currentX = (short)(SB_KEYTAB_SIZE_X * ratio);
				break;

			case	2:
				if(key->currentY < key->prev->currentY - SB_KEYTAB_SIZE_Y)	key->currentY += SB_KEYSLIDE_AMOUNT;
					else	key->direction = 0;
				break;
		}

		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hKeyTabs, &locRect, (DDWORD)key->currentX, (DDWORD)key->currentY);
		key = key->next;
	}
}

//*************************************************************************
//*****	Function:	DrawInfoTab()
//*****	Details:	Draws the information tab at the top of the screen
//*************************************************************************

void	CStatusBar::DrawInfoTab(HSURFACE hScreen)
{
	HDECOLOR	white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);

	m_pInfoCursor->SetFont(m_pStatFont1);
	m_pInfoCursor->SetDest(hScreen);

	if(m_bMultiInfo)
	{
		m_pInfoCursor->SetJustify(CF_JUSTIFY_LEFT);
		m_pInfoCursor->SetLoc(m_nInfoX, m_nInfoY);
		m_pInfoCursor->Draw("< NAME OF LEVEL >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->SetFont(m_pStatFont4);
		m_pInfoCursor->Draw("< PLAYER 1 > = < KILLS > < DEATHS >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->Draw("< PLAYER 2 > = < KILLS > < DEATHS >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->Draw("< PLAYER 3 > = < KILLS > < DEATHS >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->Draw("< PLAYER 4 > = < KILLS > < DEATHS >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->Draw("< HUMILIATIONS > = < YOU > < THEM >");
	}
	else
	{
		m_pInfoCursor->SetJustify(CF_JUSTIFY_CENTER);
		m_pInfoCursor->SetLoc((short)(m_nScreenWidth >> 1), (short)(m_nScreenHeight >> 1) - 140);
		m_pInfoCursor->Draw("< INSERT LEVEL NAME HERE >"); m_pInfoCursor->NewLine();
		m_pInfoCursor->SetFont(m_pStatFont4);
		m_pInfoCursor->DrawSolid("< OBJECTIVE 1 > = KILL EVERYTHING YOU CAN!", white); m_pInfoCursor->NewLine();
		m_pInfoCursor->DrawSolid("< OBJECTIVE 2 > = KILL MORE STUFF!", white); m_pInfoCursor->NewLine();
		m_pInfoCursor->DrawSolid("< OBJECTIVE 3 > = INSERT YOUR OWN OBJECTIVE HERE...", white); m_pInfoCursor->NewLine();
		m_pInfoCursor->DrawSolid("< OBJECTIVE 4 > = DUNNO... JUST KILL SOMTHIN'!", white); m_pInfoCursor->NewLine();
		m_pInfoCursor->DrawSolid("< KILLS > = < TOTAL > < THIS LEVEL >", white);
	}
}

//*************************************************************************
//*****	Function:	DrawEquipTab()
//*****	Details:	Draws the information tab at the top of the screen
//*************************************************************************

void	CStatusBar::DrawEquipTab(HSURFACE hScreen)
{
	char	*string, i, j = m_bTopEquipItem + SB_NUM_EQUIP_DISP_ITEMS;
	short	x, y;

	if(j > SB_NUM_EQUIP_ITEMS)	j = SB_NUM_EQUIP_ITEMS;

	m_pEquipCursor->SetDest(hScreen);
	m_pEquipCursor->SetLoc((short)(m_nEquipX + SB_EQUIP_OFFSET_X), (short)(m_nEquipY + SB_EQUIP_OFFSET_Y));
	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hEquipTab, 0, m_nEquipX, m_nEquipY);

	for(i = m_bTopEquipItem; i < j; i++)
	{
		switch(m_pEquipList[i].type)
		{
			case	0:	string = g_EquipTitles[m_pEquipList[i].name];	break;
			case	1:	string = g_WeaponNames[m_pEquipList[i].name];	break;
			case	2:
			case	3:	string = g_ItemNames[m_pEquipList[i].name];		break;
			case	4:	string = g_SpellNames[m_pEquipList[i].name];	break;
		}

		if(m_bSelectedEquipItem == i)
		{
			m_pEquipCursor->GetLoc(x, y);
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hEquipCursor, 0, x, y, m_hTransColor);

			m_pEquipCursor->SetFont(m_pStatFont3);
			m_pEquipCursor->Draw(string, m_pEquipList[i].offset, 0);
			m_pEquipCursor->SetFont(m_pStatFont2);
		}
		else
			m_pEquipCursor->Draw(string, m_pEquipList[i].offset, 0);

		m_pEquipCursor->NewLine();
	}
}

//*************************************************************************
//*****	Function:	DrawSpellbook(HSURFACE hScreen)
//*****	Details:	Draw the spellbook interface
//*************************************************************************

void	CStatusBar::DrawSpellbook(HSURFACE hScreen)
{
	short	i;
	DRect	rect;

	m_pSpellCursor->SetDest(hScreen);
	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellBG, 0, 0, 0);

	rect.left = 0;
	rect.right = SB_SPELLNAME_SIZE_X;

	for(i = 0; i <= SB_NUM_SINGLEPLAYER_SPELLS; i++)
	{
		if(m_bSpellLevel < g_SpellLevel[g_SpellNameFix[i]])
		{
			rect.top = g_SpellNameYOffsets[i];
			rect.bottom = g_SpellNameYOffsets[i+1];
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSpellsLo, &rect, SB_SPELLNAME_LOC_X, g_SpellNameYLocs[i], m_hTransColor);
		}
		else if(i == m_bCurrentSpell)
		{
			rect.top = g_SpellNameYOffsets[i];
			rect.bottom = g_SpellNameYOffsets[i+1];
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSpellsHi, &rect, SB_SPELLNAME_LOC_X, g_SpellNameYLocs[i], m_hTransColor);
		}

		if(m_pCurrentSpells[g_SpellNameFix[i]])
			m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSpellDot, 0, SB_SPELLDOT_LOC_X, g_SpellDotYLocs[i], m_hTransColor);
	}

	rect.right = SB_SYMBOL_SIZE;
	rect.top = SB_SYMBOL_SIZE * m_bCurrentSpell;
	rect.bottom = rect.top + SB_SYMBOL_SIZE;

	m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellSymbols, &rect, SB_SYMBOL_LOC_X, SB_SYMBOL_LOC_Y);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hSpellCursor, 0, 0, g_SpellCursorYLocs[m_bCurrentSpell] - 7, m_hTransColor);

	m_pSpellCursor->SetLoc(SB_SYMBOL_LOC_X + SB_SYMBOL_SIZE, SB_SYMBOL_LOC_Y);
	m_pSpellCursor->SetFont(m_pSpellFont2);
	m_pSpellCursor->NewLine();
	m_pSpellCursor->NewLine();
	m_pSpellCursor->DrawFormat(g_SpellRequires[m_bCurrentSpell], 0);

	m_pSpellCursor->SetLoc(SB_SYMBOL_LOC_X, SB_SYMBOL_LOC_Y + SB_SYMBOL_SIZE);
	m_pSpellCursor->SetFont(m_pSpellFont2);
	m_pSpellCursor->NewLine();
	m_pSpellCursor->DrawFormat(g_SpellInfo[m_bCurrentSpell], m_nSpellTextWidth);
}

//*************************************************************************
//*****	Function:	DrawCharacterScreen(HSURFACE hScreen)
//*****	Details:	Draw the character creation interface
//*************************************************************************

void	CStatusBar::DrawCharacterScreen(HSURFACE hScreen)
{
	short	i;
	char	type, **list;
	char	str = m_pCSFieldList[0].num2, foc = m_pCSFieldList[3].num2;

	for(i = 4; i < 7; i++)
	{
		if(m_pCSFieldList[i].name == POWERUP_STRENGTHBINDING)	str++;
		if(m_pCSFieldList[i].name == POWERUP_MAGICBINDING)		foc++;
	}

	//**********  SETUP ALL OF THE TEXT COLORS  **********
	HDECOLOR red = m_pClientDE->SetupColor1(1.0f, 0.0f, 0.0f, DFALSE);
	HDECOLOR green = m_pClientDE->SetupColor1(0.0f, 1.0f, 0.0f, DFALSE);
	HDECOLOR blue = m_pClientDE->SetupColor1(0.0f, 0.0f, 1.0f, DFALSE);
	HDECOLOR yellow = m_pClientDE->SetupColor1(1.0f, 1.0f, 0.0f, DFALSE);
	HDECOLOR grey = m_pClientDE->SetupColor1(0.3f, 0.3f, 0.3f, DFALSE);
	HDECOLOR white = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);

	m_pCharCursor->SetDest(hScreen);

	//**********  DRAW THE BOXES  ***********
	for(i = 0; i < SB_NUM_CSBOXES; i++)
		m_pClientDE->DrawSurfaceToSurface(hScreen, m_hCharFields, &(m_rCSFields[m_pCSBoxes[i].box]), m_pCSBoxes[i].x, m_pCSBoxes[i].y);

	//**********  DRAW THE EXTRA POINTS  ************
	m_pCharCursor->SetJustify(m_cfExtraPoints.justify);
	m_pCharCursor->SetLoc(m_pCSBoxes[0].x + m_cfExtraPoints.x, m_pCSBoxes[0].y + m_cfExtraPoints.y);
	m_pCharCursor->DrawSolid("Extra Points\0", red);
	m_pCharCursor->DrawSolid(g_CharScreenValues[m_cfExtraPoints.num1], SB_CHARNUM1_OFFSET_X, 0, blue);

	if(m_cfExtraPoints.num2 == m_cfExtraPoints.num1)
		m_pCharCursor->DrawSolid(g_CharScreenValues[m_cfExtraPoints.num2], SB_CHARNUM2_OFFSET_X, 0, blue);
	else if(m_cfExtraPoints.num2 > m_cfExtraPoints.num1)
		m_pCharCursor->DrawSolid(g_CharScreenValues[m_cfExtraPoints.num2], SB_CHARNUM2_OFFSET_X, 0, yellow);
	else
		m_pCharCursor->DrawSolid(g_CharScreenValues[m_cfExtraPoints.num2], SB_CHARNUM2_OFFSET_X, 0, red);

	//**********  START DRAWING THE FIELD NAMES  ***********
	for(i = 0; i < SB_NUM_CSFIELDS; i++)
	{
		type = m_pCSFieldList[i].type;
		list = m_pCSBoxes[type].list;

		m_pCharCursor->SetJustify(m_pCSFieldList[i].justify);
		m_pCharCursor->SetLoc(m_pCSBoxes[type].x + m_pCSFieldList[i].x, m_pCSBoxes[type].y + m_pCSFieldList[i].y);

		if(m_pCSFieldList[i].use == SB_CSFIELD_VALUE)
		{
			if(i == m_bCSCurrentField)	m_pCharCursor->DrawSolid(list[m_pCSFieldList[i].name], green);
				else					m_pCharCursor->DrawSolid(list[m_pCSFieldList[i].name], white);

			m_pCharCursor->DrawSolid(g_CharScreenValues[m_pCSFieldList[i].num1], SB_CHARNUM1_OFFSET_X, 0, blue);
			if(m_pCSFieldList[i].num2 == m_pCSFieldList[i].num1)
				m_pCharCursor->DrawSolid(g_CharScreenValues[m_pCSFieldList[i].num2], SB_CHARNUM2_OFFSET_X, 0, blue);
			else if(m_pCSFieldList[i].num2 > m_pCSFieldList[i].num1)
				m_pCharCursor->DrawSolid(g_CharScreenValues[m_pCSFieldList[i].num2], SB_CHARNUM2_OFFSET_X, 0, yellow);
			else
				m_pCharCursor->DrawSolid(g_CharScreenValues[m_pCSFieldList[i].num2], SB_CHARNUM2_OFFSET_X, 0, red);
		}
		else
		{
			char	*name = list[m_pCSFieldList[i].name];

			if(((m_bCSInputState == 1) || m_szCSNameString[0]) && (m_pCSFieldList[i].use == SB_CSFIELD_NEWNAME))
				name = m_szCSNameString;
			if((m_bCSInputState == 2) && (m_pCSFieldList[i].use == SB_CSFIELD_SAVECHAR))
				name = m_szCSSaveFile;

			if((m_bCSSelectState == 2) && (m_pCSFieldList[i].use == SB_CSFIELD_LOADCHAR))
				name = m_pCharFileIndex->m_pBaseFilename;
			if((m_bCSSelectState == 3) && (m_pCSFieldList[i].use == SB_CSFIELD_DELETECHAR))
				name = m_pCharFileIndex->m_pBaseFilename;

			if(!(m_pCSFieldList[i].active))	{ m_pCharCursor->DrawSolid(name, grey); continue; }
			if((type == 2) && m_pCSFieldList[i].name && ((str < g_WeaponStr[m_pCSFieldList[i].name]) || (foc < g_WeaponFoc[m_pCSFieldList[i].name])))
			{
				if(i == m_bCSCurrentField)
					m_pCharCursor->DrawSolid(name, yellow);
				else
					m_pCharCursor->DrawSolid(name, red);
				continue;
			}
			if((type == 6) && m_pCSFieldList[i].name && (foc < g_SpellLevel[m_pCSFieldList[i].name]))
			{
				if(i == m_bCSCurrentField)
					m_pCharCursor->DrawSolid(name, yellow);
				else
					m_pCharCursor->DrawSolid(name, red);
				continue;
			}
			if(i == m_bCSCurrentField)		{ m_pCharCursor->DrawSolid(name, green); continue; }
			m_pCharCursor->DrawSolid(name, white);
		}
	}
}

//*************************************************************************
//*****	Function:	DrawNumber(DDWORD nValue, DDWORD xPos, DDWORD yPos)
//*****	Details:	Draws a 3 digit number display for a status bar item
//*************************************************************************

void	CStatusBar::DrawNumber(DDWORD nValue, DDWORD xPos, DDWORD yPos)
{
	HSURFACE	hScreen = m_pClientDE->GetScreenSurface();
	DRect		SrcRect;
	DDWORD		nDigit[3];
	DBYTE		nYIndex;
	int			i;

	if(nValue > 999)	nValue = 999;

	if(!nValue)	nYIndex = 0;
		else	nYIndex = (unsigned char)((nValue - 1) / 25);
	if(nYIndex > 3)	nYIndex = 3;

	nDigit[0] = nValue / 100;
	nValue = nValue % 100;
	nDigit[1] = nValue / 10;
	nValue = nValue % 10;
	nDigit[2] = nValue;

	SrcRect.top = g_NumberYOffsets[nYIndex];
	SrcRect.bottom = SrcRect.top + SB_NUMBER_SIZE_Y;

	for(i = 0; i < 3; i++)
	{
		SrcRect.left = g_NumberXOffsets[nDigit[i]];
		SrcRect.right = SrcRect.left + SB_NUMBER_SIZE_X;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hStatNums, &SrcRect, xPos, yPos, m_hTransColor);
		xPos += SB_NUMBER_SIZE_X;
	}
}

//*************************************************************************
//*****	Function:	HandleItemInventory(HSURFACE hScreen)
//*****	Details:	Draws a 3 digit number display for a status bar item
//*************************************************************************

void	CStatusBar::HandleItemInventory(HSURFACE hScreen)
{
	DFLOAT	currentTime = m_pClientDE->GetTime();
	DFLOAT	dif, ratio;
	short	tempX, tempY;
	DRect	locRect;
	short	count;
	DDWORD	tempCalc;

	locRect.top = 0;
	locRect.bottom = SB_INVICON_SIZE_Y;
	dif = currentTime - m_fInvStartTime;

	switch(m_bInventoryState)
	{
		case	0:
			if(dif > SB_INVENTORY_TIME_1)
				{ ratio = 1.0; m_bInventoryState = 1; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_1;

			tempCalc = m_nInventoryY - (int)(SB_INVICON_SIZE_Y * ratio);
			locRect.left = m_hItemList[0]->locXLeft; locRect.right = m_hItemList[0]->locXRight;
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, m_nItemX, tempCalc);

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			break;

		case	1:
			if(dif > SB_INVENTORY_TIME_2)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_2;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			for(count = 0; count < SB_NUM_ITEMS; count++)
			{
				locRect.left = m_hItemList[count]->locXLeft;
				locRect.right = m_hItemList[count]->locXRight;
				tempX = (short)(m_nItemX + (int)(g_ItemXOffsets[count] * ratio));
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			}

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			break;

		case	2:
			if(dif > SB_INVENTORY_TIME_3) { m_bInventoryState = 0; m_bInventoryOn = 0; }

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			for(count = 0; count < SB_NUM_ITEMS; count++)
			{
				locRect.left = m_hItemList[count]->locXLeft;
				locRect.right = m_hItemList[count]->locXRight;
				tempX = (short)(m_nItemX + g_ItemXOffsets[count]);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			}

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			DrawHighlight(hScreen, 0);
			break;

		case	3:
			if(dif > SB_INVENTORY_TIME_4)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_4;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			tempCalc = (short)(SB_INVICON_SIZE_X * ratio);
			for(count = 0; count < SB_NUM_ITEMS - 1; count++)
			{
				locRect.left = m_hItemList[count]->locXLeft;
				locRect.right = m_hItemList[count]->locXRight;
				tempX = (short)(m_nItemX + g_ItemXOffsets[count] + tempCalc);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			}

			//***** Draw the cut off icon with half on one side, half on the other *****
			locRect.left = m_hItemList[count]->locXLeft;
			locRect.right = m_hItemList[count]->locXRight - tempCalc;
			tempX = (short)(m_nItemX + g_ItemXOffsets[count] + tempCalc);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);

			locRect.left = m_hItemList[count]->locXLeft + SB_INVICON_SIZE_X - tempCalc;
			locRect.right = m_hItemList[count]->locXRight;
			tempX = (short)(m_nItemX + g_ItemXOffsets[0]);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			//**************************************************************************

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			if(m_bInventoryState == 2)		// Rotate point through the item list
			{
				CInventoryItem *tempItem = m_hItemList[SB_NUM_ITEMS - 1];
				for(count = SB_NUM_ITEMS - 1; count > 0; count--)
					m_hItemList[count] = m_hItemList[count - 1];
				m_hItemList[0] = tempItem;
				tempItem = 0;
			}
			DrawHighlight(hScreen, 0);
			break;

		case	4:
			if(dif > SB_INVENTORY_TIME_4)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_4;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			tempCalc = (short)(SB_INVICON_SIZE_X * ratio);
			for(count = 1; count < SB_NUM_ITEMS; count++)
			{
				locRect.left = m_hItemList[count]->locXLeft;
				locRect.right = m_hItemList[count]->locXRight;
				tempX = (short)(m_nItemX + g_ItemXOffsets[count] - tempCalc);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			}

			//***** Draw the cut off icon with half on one side, half on the other *****
			locRect.left = m_hItemList[0]->locXLeft;
			locRect.right = m_hItemList[0]->locXRight - SB_INVICON_SIZE_X + tempCalc;
			tempX = (short)(m_nItemX + g_ItemXOffsets[count - 1] + SB_INVICON_SIZE_X - tempCalc);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);

			locRect.left = m_hItemList[0]->locXLeft + tempCalc;
			locRect.right = m_hItemList[0]->locXRight;
			tempX = (short)(m_nItemX + g_ItemXOffsets[0]);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, tempX, tempY);
			//**************************************************************************

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			if(m_bInventoryState == 2)		// Rotate point through the item list
			{
				CInventoryItem *tempItem;
				tempItem = m_hItemList[0];
				for(count = 0; count < SB_NUM_ITEMS - 1; count++)
					m_hItemList[count] = m_hItemList[count + 1];
				m_hItemList[count] = tempItem;
				tempItem = 0;
			}
			DrawHighlight(hScreen, 0);
			break;

		case	5:
			if(dif > SB_INVENTORY_TIME_1)
			{
				ratio = 1.0; m_bInventoryState = 0; m_bInventoryOn = 0;
				m_hSelectedItem = m_hItemList[3];
				m_bCurrentItem = m_hSelectedItem->type;
			}
			else	ratio = dif / SB_INVENTORY_TIME_1;

			if(!m_hItemList[3]->type)
				{ m_bInventoryState = 0; m_bInventoryOn = 0; }

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			locRect.left = m_hItemList[3]->locXLeft;
			locRect.right = m_hItemList[3]->locXRight;
			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y + (SB_INVICON_SIZE_Y * ratio));
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hItemIcons, &locRect, m_nItemX, tempY);
			break;
	}
}

//*************************************************************************
//*****	Function:	HandleSpellInventory(HSURFACE hScreen)
//*****	Details:	Draws a 3 digit number display for a status bar item
//*************************************************************************
/*
void	CStatusBar::HandleSpellInventory(HSURFACE hScreen)
{
	DFLOAT	currentTime = m_pClientDE->GetTime();
	DFLOAT	dif, ratio;
	short	tempX, tempY;
	DRect	locRect;
	short	count;
	DDWORD	tempCalc;

	locRect.top = 0;
	locRect.bottom = SB_INVICON_SIZE_Y;
	dif = currentTime - m_fInvStartTime;

	switch(m_bInventoryState)
	{
		case	0:
			if(dif > SB_INVENTORY_TIME_1)
				{ ratio = 1.0; m_bInventoryState = 1; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_1;

			tempCalc = m_nInventoryY - (int)(SB_INVICON_SIZE_Y * ratio);
			locRect.left = m_hSpellList[0]->locXLeft; locRect.right = m_hSpellList[0]->locXRight;
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, m_nSpellX, tempCalc);

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			break;

		case	1:
			if(dif > SB_INVENTORY_TIME_2)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_2;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			for(count = 0; count < SB_NUM_SPELLS; count++)
			{
				locRect.left = m_hSpellList[count]->locXLeft;
				locRect.right = m_hSpellList[count]->locXRight;
				tempX = (short)(m_nSpellX + (int)(g_SpellXOffsets[count] * ratio));
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			}

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			break;

		case	2:
			if(dif > SB_INVENTORY_TIME_3) { m_bInventoryState = 0; m_bInventoryOn = 0; }

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			for(count = 0; count < SB_NUM_SPELLS; count++)
			{
				locRect.left = m_hSpellList[count]->locXLeft;
				locRect.right = m_hSpellList[count]->locXRight;
				tempX = (short)(m_nSpellX + g_SpellXOffsets[count]);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			}

			if(m_bStatsOn)	DrawItemTabs(hScreen);
			DrawHighlight(hScreen, 1);
			break;

		case	3:
			if(dif > SB_INVENTORY_TIME_4)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_4;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			tempCalc = (short)(SB_INVICON_SIZE_X * ratio);
			for(count = 0; count < SB_NUM_SPELLS - 1; count++)
			{
				locRect.left = m_hSpellList[count]->locXLeft;
				locRect.right = m_hSpellList[count]->locXRight;
				tempX = (short)(m_nSpellX + g_SpellXOffsets[count] + tempCalc);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			}

			//***** Draw the cut off icon with half on one side, half on the other *****
			locRect.left = m_hSpellList[count]->locXLeft;
			locRect.right = m_hSpellList[count]->locXRight - tempCalc;
			tempX = (short)(m_nSpellX + g_SpellXOffsets[count] + tempCalc);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);

			locRect.left = m_hSpellList[count]->locXLeft + SB_INVICON_SIZE_X - tempCalc;
			locRect.right = m_hSpellList[count]->locXRight;
			tempX = (short)(m_nSpellX + g_SpellXOffsets[0]);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			//**************************************************************************

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			if(m_bInventoryState == 2)		// Rotate point through the spell list
			{
				CInventoryItem *tempItem;
				tempItem = m_hSpellList[SB_NUM_SPELLS - 1];
				for(count = SB_NUM_SPELLS - 1; count > 0; count--)
					m_hSpellList[count] = m_hSpellList[count - 1];
				m_hSpellList[0] = tempItem;
				tempItem = 0;
			}
			DrawHighlight(hScreen, 1);
			break;

		case	4:
			if(dif > SB_INVENTORY_TIME_4)
				{ ratio = 1.0; m_bInventoryState = 2; m_fInvStartTime = currentTime; }
			else	ratio = dif / SB_INVENTORY_TIME_4;

			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y);
			tempCalc = (short)(SB_INVICON_SIZE_X * ratio);
			for(count = 1; count < SB_NUM_SPELLS; count++)
			{
				locRect.left = m_hSpellList[count]->locXLeft;
				locRect.right = m_hSpellList[count]->locXRight;
				tempX = (short)(m_nSpellX + g_SpellXOffsets[count] - tempCalc);
				m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			}

			//***** Draw the cut off icon with half on one side, half on the other *****
			locRect.left = m_hSpellList[0]->locXLeft;
			locRect.right = m_hSpellList[0]->locXRight - SB_INVICON_SIZE_X + tempCalc;
			tempX = (short)(m_nSpellX + g_SpellXOffsets[count - 1] + SB_INVICON_SIZE_X - tempCalc);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);

			locRect.left = m_hSpellList[0]->locXLeft + tempCalc;
			locRect.right = m_hSpellList[0]->locXRight;
			tempX = (short)(m_nSpellX + g_SpellXOffsets[0]);
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, tempX, tempY);
			//**************************************************************************

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			if(m_bInventoryState == 2)		// Rotate point through the spell list
			{
				CInventoryItem *tempItem;
				tempItem = m_hSpellList[0];
				for(count = 0; count < SB_NUM_SPELLS - 1; count++)
					m_hSpellList[count] = m_hSpellList[count + 1];
				m_hSpellList[count] = tempItem;
				tempItem = 0;
			}
			DrawHighlight(hScreen, 1);
			break;

		case	5:
			if(dif > SB_INVENTORY_TIME_1)
			{
				ratio = 1.0; m_bInventoryState = 0; m_bInventoryOn = 0;
				m_hSelectedSpell = m_hSpellList[SB_NUM_SPELLS >> 1];
				if(m_hSelectedSpell->type)
					m_bCurrentItem = INV_BASESPELL + m_hSelectedSpell->type - 1;
				else
					m_bCurrentItem = 0;
			}
			else	ratio = dif / SB_INVENTORY_TIME_1;

			if(!m_hSpellList[SB_NUM_SPELLS >> 1]->type)
				{ m_bInventoryState = 0; m_bInventoryOn = 0; }

			if(m_bStatsOn)	DrawItemTabs(hScreen);

			locRect.left = m_hSpellList[SB_NUM_SPELLS >> 1]->locXLeft;
			locRect.right = m_hSpellList[SB_NUM_SPELLS >> 1]->locXRight;
			tempY = (short)(m_nInventoryY - SB_INVICON_SIZE_Y + (SB_INVICON_SIZE_Y * ratio));
			m_pClientDE->DrawSurfaceToSurface(hScreen, m_hSpellIcons, &locRect, m_nSpellX, tempY);
			break;
	}
}
*/

//*************************************************************************
//*****	Function:	SaveB2CFile()
//*****	Details:	
//*************************************************************************

void	CStatusBar::SaveB2CFile()
{
	FILE	*file;
	char	str1[128] = "blood2/b2cfiles/";
	char	*str2 = ".b2c";

	strcat(str1, m_szCSSaveFile);
	strcat(str1, str2);

	if(file = fopen(str1, "wb"))
	{
		fwrite(m_szCSNameString, 1, SB_MAX_CS_STRING_SIZE, file);

		for(short i = 0; i < 4; i++)
		{
			fwrite(&(m_pCSFieldList[i].num1), 1, 1, file);
			fwrite(&(m_pCSFieldList[i].num2), 1, 1, file);
		}

		fwrite(&(m_cfExtraPoints.num1), 1, 1, file);
		fwrite(&(m_cfExtraPoints.num2), 1, 1, file);

		for(i = 4; i < SB_NUM_CSFIELDS - 5; i++)
			fwrite(&(m_pCSFieldList[i].name), 1, 1, file);

		fclose(file);
	}
}

//*************************************************************************
//*****	Function:	LoadB2CFile()
//*****	Details:	
//*************************************************************************

void	CStatusBar::LoadB2CFile(FileEntry *pfe)
{
	FILE	*file;
	char	tempList[SB_NUM_CSFIELDS - 9];

	char	str1[128] = "blood2/b2cfiles/";
	strcat(str1, pfe->m_pBaseFilename);

	if(file = fopen(str1, "rb"))
	{
		fread(m_szCSNameString, 1, SB_MAX_CS_STRING_SIZE, file);

		for(short i = 0; i < 4; i++)
		{
			fread(&(m_pCSFieldList[i].num1), 1, 1, file);
			fread(&(m_pCSFieldList[i].num2), 1, 1, file);
		}

		fread(&(m_cfExtraPoints.num1), 1, 1, file);
		fread(&(m_cfExtraPoints.num2), 1, 1, file);

		fread(tempList, 1, SB_NUM_CSFIELDS - 9, file);
		for(i = 4; i < SB_NUM_CSFIELDS - 5; i++)
			if(m_pCSFieldList[i].active)
				m_pCSFieldList[i].name = tempList[i - 4];

		fclose(file);
	}
}

//*************************************************************************
//*****	Function:	DeleteB2CFile()
//*****	Details:	
//*************************************************************************

void	CStatusBar::DeleteB2CFile(FileEntry *pfe)
{
	char	str1[128] = "blood2/b2cfiles/";
	strcat(str1, pfe->m_pBaseFilename);
	if(strcmp(pfe->m_pBaseFilename, "default.b2c"))	remove(str1);
}