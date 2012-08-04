// LayoutMgr.cpp: implementation of the CLayoutMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LayoutMgr.h"


#define LO_BASIC_TAG					"BasicLayout"
#define LO_SELECT_TAG					"SelectionLayout"
#define LO_DEFAULT_TAG					"GenericFolder"
#define LO_HUD_TAG						"HUDLayout"
#define LO_MISC_TAG						"Miscellaneous"
#define LO_MT_TAG						"MissionText"
#define LO_SUBTITLE_TAG					"Subtitle"
#define LO_MASK_TAG						"Overlay"
#define LO_DIALOGUE_TAG					"DialogueWindow"
#define LO_DECISION_TAG					"DecisionWindow"
#define LO_MENU_TAG						"MenuWindow"
#define LO_CHAR_TAG						"Character"
#define LO_CREDITS_TAG					"Credits"

#define LO_BASIC_HELP_RECT				"HelpRect"
#define LO_BASIC_BACK_POS				"BackPos"
#define LO_BASIC_CONT_POS				"ContinuePos"
#define LO_BASIC_MAIN_POS				"MainPos"
#define LO_BASIC_SHADE_COLOR			"ShadeColor"
#define LO_BASIC_BAR_COLOR				"BarColor"
#define LO_BASIC_BAR_HT					"BarHeight"
#define LO_BASIC_TOP_SHADE_HT			"TopShadeHeight"
#define LO_BASIC_BOT_SHADE_HT			"BottomShadeHeight"
#define LO_BASIC_BACK_SPRITE			"BackSprite"
#define LO_BASIC_BACK_SCALE				"BackSpriteScale"
#define LO_BASIC_ARROW_BACK				"ArrowBackSFX"
#define LO_BASIC_ARROW_NEXT				"ArrowNextSFX"
#define LO_BASIC_ARROW_BACK_BMP			"ArrowBackBitmap"
#define LO_BASIC_ARROW_NEXT_BMP			"ArrowNextBitmap"
#define LO_BASIC_ARROW_BACK_POS			"ArrowBackPos"
#define LO_BASIC_ARROW_NEXT_POS			"ArrowNextPos"
#define LO_BASIC_LOAD_POS				"LoadStringPos"
#define LO_BASIC_PHOTO_POS				"LoadPhotoPos"
#define LO_BASIC_BOSS_RECT				"BossRect"

#define LO_SELECT_SLOT_POS				"SlotOffset"
#define LO_SELECT_LIST_RECT				"ListRect"
#define LO_SELECT_LIST_FONT				"ListFontSize"
#define LO_SELECT_NAME_RECT				"NameRect"
#define LO_SELECT_NAME_FONT				"NameFontSize"
#define LO_SELECT_PHOTO_POS				"PhotoPos"
#define LO_SELECT_DESC_RECT				"DescriptionRect"
#define LO_SELECT_DESC_FONT				"DescFontSize"
#define LO_SELECT_UP_POS				"UpArrowOffset"
#define LO_SELECT_DOWN_POS				"DownArrowOffset"

#define LO_FOLDER_TITLE_POS				"TitlePos"
#define LO_FOLDER_TITLE_ALIGN			"TitleAlign"
#define LO_FOLDER_BACKGROUND			"Background"
#define LO_FOLDER_PAGE_RECT				"PageRect"
#define LO_FOLDER_ITEM_SPACE			"ItemSpace"
#define LO_FOLDER_ITEM_ALIGN			"ItemAlign"
#define LO_FOLDER_MUSIC_INTENSITY		"MusicIntensity"
#define LO_FOLDER_UP_POS				"UpArrowPos"
#define LO_FOLDER_DOWN_POS				"DownArrowPos"
#define LO_FOLDER_CHARACTER				"Character"
#define LO_FOLDER_ATTACH				"Attachment"

#define LO_HUD_LAYOUT_NAME				"LayoutName"

#define	LO_HUD_AMMO_BASEPOS				"AmmoBasePos"
#define	LO_HUD_AMMO_USEBAR				"UseAmmoBar"
#define	LO_HUD_AMMO_BAROFF				"AmmoClipOffset"
#define	LO_HUD_AMMO_OFFSET				"AmmoBarOffset"
#define	LO_HUD_AMMO_USETEXT				"UseAmmoText"
#define	LO_HUD_AMMO_TEXTOFF				"AmmoTextOffset"
#define	LO_HUD_AMMO_ICONOFF				"AmmoIconOffset"

#define	LO_HUD_HEALTH_BASEPOS			"HealthBasePos"
#define	LO_HUD_HEALTH_USEBAR			"UseHealthBar"
#define	LO_HUD_HEALTH_BAROFF			"HealthBarOffset"
#define	LO_HUD_ARMOR_OFFSET				"ArmorBarOffset"
#define	LO_HUD_HEALTH_USETEXT			"UseHealthText"
#define	LO_HUD_HEALTH_TEXTOFF			"HealthTextOffset"
#define	LO_HUD_ARMOR_TEXTOFF			"ArmorTextOffset"
#define	LO_HUD_HEALTH_USEICON			"UseHealthIcon"
#define	LO_HUD_HEALTH_ICONOFF			"HealthIconOffset"
#define	LO_HUD_ARMOR_ICONOFF			"ArmorIconOffset"

#define	LO_HUD_AIR_BASEPOS				"AirBasePos"
#define	LO_HUD_AIR_USEICON				"UseAirIcon"
#define	LO_HUD_AIR_ICONOFF				"AirIconOffset"
#define	LO_HUD_AIR_USETEXT				"UseAirText"
#define	LO_HUD_AIR_TEXTOFF				"AirTextOffset"
#define	LO_HUD_AIR_USEBAR				"UseAirBar"
#define	LO_HUD_AIR_BAROFF				"AirBarOffset"
#define	LO_HUD_BAR_HEIGHT				"BarHeight"
#define	LO_HUD_BAR_SCALE				"BarScale"

#define LO_HUD_DAMAGE_BASEPOS			"DamageBasePos"

#define	LO_HUD_MODE_TEXTPOS				"ModeTextPos"

#define	LO_MISC_FLASHSPEED				"FlashSpeed"
#define	LO_MISC_FLASHDUR				"FlashDuration"
#define	LO_MISC_NV_MODEL				"NightVisionModelColor"
#define	LO_MISC_NV_SCREEN				"NightVisionScreenTint"
#define	LO_MISC_IR_MODEL				"InfraredModelColor"
#define	LO_MISC_IR_LIGHT				"InfraredLightScale"
#define	LO_MISC_MD_SCREEN				"MineDetectScreenTint"
#define	LO_MISC_WPN_COLOR				"WeaponPickupColor"
#define	LO_MISC_AMMO_COLOR				"AmmoPickupColor"
#define	LO_MISC_TINTTIME				"TintTime"
#define	LO_MISC_GAPMIN					"CrosshairGapMin"
#define	LO_MISC_GAPMAX					"CrosshairGapMax"
#define	LO_MISC_BARMIN					"CrosshairBarMin"
#define	LO_MISC_BARMAX					"CrosshairBarMax"
#define	LO_MISC_ROTEFFECT				"PerturbRotationEffect"
#define	LO_MISC_PERTURBINC				"PerturbIncreaseSpeed"
#define	LO_MISC_PERTURBDEC				"PerturbDecreaseSpeed"
#define	LO_MISC_WALKPER					"PerturbWalkPercent"
#define LO_MISC_MB_BACK					"MessageBoxBackground"
#define LO_MISC_MB_ALPHA				"MessageBoxAlpha"
#define LO_MISC_FAILBACK				"FailScreenBackground"
#define LO_MISC_FAILPOS					"FailStringPos"
#define LO_MISC_FAILDELAY				"FailScreenDelay"
#define LO_MISC_DEATHDELAY				"DeathDelay"
#define LO_MISC_HELPFONT				"HelpFont"
#define LO_MISC_SMALLFONT				"SmallFontBase"
#define LO_MISC_MEDFONT					"MediumFontBase"
#define LO_MISC_LARGEFONT				"LargeFontBase"
#define LO_MISC_TITLEFONT				"TitleFont"
#define LO_MISC_MSGFOREFONT				"MsgForeFont"
#define LO_MISC_HUDFOREFONT				"HUDForeFont"
#define LO_MISC_AIRFONT					"HUDAirFont"
#define LO_MISC_CHOOSERFONT				"HUDChooserFont"
#define LO_MISC_CHOOSER_HCOLOR			"HUDChoserHColor"
#define LO_MISC_MSG_FADE				"MessageFade"
#define LO_MISC_MSG_TIME				"MessageTime"
#define LO_MISC_MSG_NUM					"MaxMessages"
#define LO_MISC_OBJ_RECT				"ObjectiveRect"
#define LO_MISC_POPUP_RECT				"PopupTextRect"
#define LO_MISC_SUB_TINT				"SubtitleTint"
#define LO_MISC_HEALTH_TINT				"HealthTint"
#define LO_MISC_ARMOR_TINT				"ArmorTint"
#define LO_MISC_AMMO_TINT				"AmmoTint"
#define LO_MISC_POPUP_TINT				"PopupTint"
#define LO_MISC_TEAM1_COLOR				"Team1Color"
#define LO_MISC_TEAM2_COLOR				"Team2Color"


#define LO_MT_WIDTH						"Width"
#define LO_MT_NUM_LINES					"NumLines"
#define LO_MT_LETTER_DELAY				"LetterDelay"
#define LO_MT_LINE_DELAY				"LineDelay"
#define LO_MT_LINE_SCROLL				"LineScrollTime"
#define LO_MT_FADE_DELAY				"FadeDelay"
#define LO_MT_FADE_TIME					"FadeTime"
#define LO_MT_POS						"Pos"
#define LO_MT_TYPE_SOUND				"TypeSound"
#define LO_MT_SCROLL_SOUND				"ScrollSound"

#define LO_SUBTITLE_NUM_LINES			"NumLines"
#define LO_SUBTITLE_LINE_SCROLL			"LineScrollTime"
#define LO_SUBTITLE_CINE_POS			"Pos"
#define LO_SUBTITLE_CINE_WIDTH			"Width"
#define LO_SUBTITLE_FULL_POS			"FullScreenPos"
#define LO_SUBTITLE_FULL_WIDTH			"FullScreenWidth"

#define	LO_MASK_SCOPESPRITE				"ScopeSprite"
#define	LO_MASK_SUNSPRITE				"SunglassSprite"
#define	LO_MASK_SCUBASPRITE				"ScubaSprite"
#define	LO_MASK_SPACESPRITE				"SpaceSprite"
#define	LO_MASK_STATICSPRITE			"StaticSprite"
#define	LO_MASK_CAMERASPRITE			"CameraSprite"
#define	LO_MASK_CAMERAACTIVATESPRITE	"CameraASprite"
#define	LO_MASK_ZOOMIN_SPRITE			"ZoomInSprite"
#define	LO_MASK_ZOOMOUT_SPRITE			"ZoomOutSprite"

#define	LO_MASK_SCOPEMODEL				"ScopeModel"
#define	LO_MASK_SUNMODEL				"SunglassModel"
#define	LO_MASK_SCUBAMODEL				"ScubaModel"
#define	LO_MASK_SPACEMODEL				"SpaceModel"
#define	LO_MASK_STATICMODEL				"StaticModel"
#define	LO_MASK_CAMERAMODEL				"CameraModel"
#define	LO_MASK_ZOOMIN_MODEL			"ZoomInModel"
#define	LO_MASK_ZOOMOUT_MODEL			"ZoomOutModel"

#define	LO_MASK_SCOPESKIN				"ScopeSkin"
#define	LO_MASK_SUNSKIN					"SunglassSkin"
#define	LO_MASK_SCUBASKIN				"ScubaSkin"
#define	LO_MASK_SPACESKIN				"SpaceSkin"
#define	LO_MASK_STATICSKIN				"StaticSkin"
#define	LO_MASK_CAMERASKIN				"CameraSkin"
#define	LO_MASK_ZOOMIN_SKIN				"ZoomInSkin"
#define	LO_MASK_ZOOMOUT_SKIN			"ZoomOutSkin"

#define	LO_MASK_SCOPESCALE				"ScopeScale"
#define	LO_MASK_SUNSCALE				"SunglassScale"
#define	LO_MASK_SCUBASCALE				"ScubaScale"
#define	LO_MASK_SPACESCALE				"SpaceScale"
#define	LO_MASK_STATICSCALE				"StaticScale"
#define	LO_MASK_CAMERASCALE				"CameraScale"
#define	LO_MASK_ZOOMIN_SCALE			"ZoomInScale"
#define	LO_MASK_ZOOMOUT_SCALE			"ZoomOutScale"

#define	LO_WINDOW_POS					"Position"
#define	LO_WINDOW_SIZE					"Size"
#define	LO_WINDOW_TEXT_OFFSET			"TextOffset"
#define	LO_WINDOW_SPACING				"Spacing"
#define	LO_WINDOW_BACK					"Background"
#define	LO_WINDOW_ALPHA					"Alpha"
#define	LO_WINDOW_FRAME					"Frame"
#define	LO_WINDOW_FONT					"Font"
#define	LO_WINDOW_OPEN					"OpenTime"
#define	LO_WINDOW_CLOSE					"CloseTime"
#define	LO_WINDOW_SND_OPEN				"OpenSound"
#define	LO_WINDOW_SND_CLOSE				"CloseSound"

#define	LO_CHAR_NAME					"Name"
#define	LO_CHAR_MOD						"Model"
#define	LO_CHAR_STYLE					"Style"
#define	LO_CHAR_POS						"Pos"
#define	LO_CHAR_SCALE					"Scale"
#define	LO_CHAR_ROT						"Rotation"

#define	LO_CREDITS_FADEIN				"FadeInTime"
#define	LO_CREDITS_HOLD					"HoldTime"
#define	LO_CREDITS_FADEOUT				"FadeOutTime"
#define	LO_CREDITS_DELAY				"DelayTime"
#define	LO_CREDITS_POS_UL				"PositionUL"
#define	LO_CREDITS_POS_UR				"PositionUR"
#define	LO_CREDITS_POS_LR				"PositionLR"
#define	LO_CREDITS_POS_LL				"PositionLL"


CLayoutMgr* g_pLayoutMgr = LTNULL;

char s_aFolderTag[FOLDER_ID_UNASSIGNED+1][32] =
{
	LO_DEFAULT_TAG,			//FOLDER_ID_NONE,
	"FolderMain",			//FOLDER_ID_MAIN,
	"FolderSingle",			//FOLDER_ID_SINGLE,
	"FolderGallery",		//FOLDER_ID_GALLERY,
	"FolderEscape",			//FOLDER_ID_ESCAPE,
	"FolderOptions",		//FOLDER_ID_OPTIONS,
	"FolderDisplay",		//FOLDER_ID_DISPLAY,
	"FolderAudio",			//FOLDER_ID_AUDIO,
	"FolderGame",			//FOLDER_ID_GAME,
	"FolderPerformance",	//FOLDER_ID_PERFORMANCE,
	"FolderAdvDisplay",		//FOLDER_ID_ADVDISPLAY,
	"FolderTexture",		//FOLDER_ID_TEXTURE,
	"FolderEffects",		//FOLDER_ID_EFFECTS,
	"FolderControls",		//FOLDER_ID_CONTROLS,
	"FolderCustomControls",	//FOLDER_ID_CUST_CONTROLS,
	"FolderWeaponControls",	//FOLDER_ID_WPN_CONTROLS,
	"FolderCrosshair",		//FOLDER_ID_CROSSHAIR,
	"FolderJoystick",		//FOLDER_ID_JOYSTICK,
	"FolderMouse",			//FOLDER_ID_MOUSE,
	"FolderKeyboard",			//FOLDER_ID_KEYBOARD,
	"FolderCustomLevel",    //FOLDER_ID_CUSTOM_LEVEL,
	"FolderFavoriteLevels", //FOLDER_ID_FAVORITE_LEVEL,
	"FolderNew",			//FOLDER_ID_NEW,
	"FolderDifficulty",			//FOLDER_ID_DIFFICULTY,
	"FolderLoad",			//FOLDER_ID_LOAD,
	"FolderSave",			//FOLDER_ID_SAVE,
	"FolderBriefing",		//FOLDER_ID_BRIEFING,
	"FolderMultiBriefing",	//FOLDER_ID_MP_BRIEFING,
	"FolderMultiSummary",	//FOLDER_ID_MP_SUMMARY,
	"FolderObjectives",		//FOLDER_ID_OBJECTIVES,
	"FolderWeapons",		//FOLDER_ID_WEAPONS,
	"FolderGadgets",		//FOLDER_ID_GADGETS,
	"FolderMods",			//FOLDER_ID_MODS,
	"FolderGear",			//FOLDER_ID_GEAR,
	"FolderInventory",		//FOLDER_ID_INVENTORY,
	"FolderViewInventory",	//FOLDER_ID_VIEW_INV,
	"FolderMission",		//FOLDER_ID_MISSION,
	"FolderMultiMission",	//FOLDER_ID_MULTI_MISSION,
	"FolderStats",			//FOLDER_ID_STATS,
	"FolderIntel",			//FOLDER_ID_INTEL,
	"FolderSummary",		//FOLDER_ID_SUMMARY,
	"FolderAwards",			//FOLDER_ID_AWARDS,
	"FolderFailure",		//FOLDER_ID_FAILURE,
	"FolderMulti",			//FOLDER_ID_MULTIPLAYER,
	"FolderPlayer",			//FOLDER_ID_PLAYER,
	"FolderJoin",			//FOLDER_ID_JOIN,
	"FolderJoinLAN",		//FOLDER_ID_JOIN_LAN,
	"FolderHost",			//FOLDER_ID_HOST,
	"FolderHostOptions",	//FOLDER_ID_HOST_OPTIONS,
	"FolderHostLevels",		//FOLDER_ID_HOST_LEVELS,
	"LoadScreenSingle",		//FOLDER_ID_LOADSCREEN_SINGLE,
	"LoadScreenMulti",		//FOLDER_ID_LOADSCREEN_MULTI,

	LO_DEFAULT_TAG			//FOLDER_ID_UNASSIGNED,

};

static char s_aTagName[30];
static char s_aAttName[30];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

INT_CHAR::INT_CHAR()
{
	szName[0] = LTNULL;

	szModel[0] = LTNULL;
	szStyle[0] = LTNULL;

    vPos.Init();
    fScale = 1.0f;
	fRot = 0.0f;
}

LTBOOL INT_CHAR::Init(CButeMgr & buteMgr, char* aTagName)
{

	CString str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_NAME);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szName, (char*)(LPCSTR)str, sizeof(szName));


	str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_MOD);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szModel, (char*)(LPCSTR)str, sizeof(szModel));

	str = "";
	str = buteMgr.GetString(aTagName, LO_CHAR_STYLE);
	if (str.GetLength() == 0) return LTFALSE;
	strncpy(szStyle, (char*)(LPCSTR)str, sizeof(szStyle));


    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, LO_CHAR_POS, zero);
	fScale = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_SCALE, 1.0);
	fRot = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_ROT, 1.0);

	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayoutMgr::CLayoutMgr()
{
	m_nNumHUDLayouts = 0;

}

CLayoutMgr::~CLayoutMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CLayoutMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pLayoutMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pLayoutMgr = this;

	m_nNumHUDLayouts = 0;

	sprintf(s_aTagName, "%s0", LO_HUD_TAG);
	while (m_buteMgr.Exist(s_aTagName))
	{
		m_nNumHUDLayouts++;
		sprintf(s_aTagName, "%s%d", LO_HUD_TAG, m_nNumHUDLayouts);
	}

	sprintf(s_aTagName, "%s0", LO_CHAR_TAG);
	int numChar = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_CHAR *pChar = debug_new(INT_CHAR);
		if (pChar->Init(m_buteMgr,s_aTagName))
			m_CharacterArray.Add(pChar);
		else
		{
			debug_delete(pChar);
		}
		numChar++;
		sprintf(s_aTagName, "%s%d", LO_CHAR_TAG, numChar);
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CLayoutMgr::Term()
{
    g_pLayoutMgr = LTNULL;

	while (m_CharacterArray.GetSize())
	{
		debug_delete(m_CharacterArray[0]);
		m_CharacterArray.Remove(0);

	}
}


// ------------------------------------------------------------------------//
//
//	Basic Folder Layout
//
// ------------------------------------------------------------------------//

LTRect   CLayoutMgr::GetHelpRect()
{
	return GetRect(LO_BASIC_TAG, LO_BASIC_HELP_RECT);
}

LTIntPt  CLayoutMgr::GetBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_BACK_POS);
}

LTIntPt CLayoutMgr::GetContinuePos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_CONT_POS);
}

LTIntPt CLayoutMgr::GetMainPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_MAIN_POS);
}


HLTCOLOR CLayoutMgr::GetShadeColor()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_BASIC_TAG, LO_BASIC_SHADE_COLOR, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetBarColor()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_BASIC_TAG, LO_BASIC_BAR_COLOR, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

int CLayoutMgr::GetBarHeight()
{
	return m_buteMgr.GetInt(LO_BASIC_TAG, LO_BASIC_BAR_HT, 0);
}

int CLayoutMgr::GetTopShadeHeight()
{
	return m_buteMgr.GetInt(LO_BASIC_TAG, LO_BASIC_TOP_SHADE_HT, 0);
}

int CLayoutMgr::GetBottomShadeHeight()
{
	return m_buteMgr.GetInt(LO_BASIC_TAG, LO_BASIC_BOT_SHADE_HT, 0);
}

void CLayoutMgr::GetBackSprite(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_BACK_SPRITE, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetBackSpriteScale()
{
	return GetFloat(LO_BASIC_TAG, LO_BASIC_BACK_SCALE);
}

void CLayoutMgr::GetArrowBackSFX(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowNextSFX(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT, pBuf, nBufLen);
}

void CLayoutMgr::GetArrowBackBmp(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_BMP, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowNextBmp(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_BMP, pBuf, nBufLen);
}

LTIntPt  CLayoutMgr::GetArrowBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_POS);
}

LTIntPt CLayoutMgr::GetArrowNextPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_POS);
}

LTIntPt CLayoutMgr::GetLoadStringPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_LOAD_POS);
}

LTIntPt CLayoutMgr::GetLoadPhotoPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_PHOTO_POS);
}

LTRect   CLayoutMgr::GetBossRect()
{
	return GetRect(LO_BASIC_TAG, LO_BASIC_BOSS_RECT);
}


// ------------------------------------------------------------------------//
//
//	Selection Folder Layout
//
// ------------------------------------------------------------------------//

LTIntPt CLayoutMgr::GetSlotOffset(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_SLOT_POS))
	{
		return GetPoint(pTag, LO_SELECT_SLOT_POS);
	}
	return GetPoint(LO_SELECT_TAG, LO_SELECT_SLOT_POS);
}


LTRect CLayoutMgr::GetListRect(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_LIST_RECT))
	{
		return GetRect(pTag, LO_SELECT_LIST_RECT);
	}
	return GetRect(LO_SELECT_TAG, LO_SELECT_LIST_RECT);
}

int CLayoutMgr::GetListFontSize(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_LIST_FONT))
	{
		return GetInt(pTag, LO_SELECT_LIST_FONT);
	}
	return GetInt(LO_SELECT_TAG, LO_SELECT_LIST_FONT);
}

LTRect CLayoutMgr::GetNameRect(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_NAME_RECT))
	{
		return GetRect(pTag, LO_SELECT_NAME_RECT);
	}
	return GetRect(LO_SELECT_TAG, LO_SELECT_NAME_RECT);
}

int CLayoutMgr::GetNameFontSize(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_NAME_FONT))
	{
		return GetInt(pTag, LO_SELECT_NAME_FONT);
	}
	return GetInt(LO_SELECT_TAG, LO_SELECT_NAME_FONT);
}

LTIntPt CLayoutMgr::GetPhotoPos(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_PHOTO_POS))
	{
		return GetPoint(pTag, LO_SELECT_PHOTO_POS);
	}
	return GetPoint(LO_SELECT_TAG, LO_SELECT_PHOTO_POS);
}

LTRect CLayoutMgr::GetDescriptionRect(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_DESC_RECT))
	{
		return GetRect(pTag, LO_SELECT_DESC_RECT);
	}
	return GetRect(LO_SELECT_TAG, LO_SELECT_DESC_RECT);
}

int CLayoutMgr::GetDescriptionFontSize(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_DESC_FONT))
	{
		return GetInt(pTag, LO_SELECT_DESC_FONT);
	}
	return GetInt(LO_SELECT_TAG, LO_SELECT_DESC_FONT);
}

LTIntPt CLayoutMgr::GetUpArrowOffset(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_UP_POS))
	{
		return GetPoint(pTag, LO_SELECT_UP_POS);
	}
	return GetPoint(LO_SELECT_TAG, LO_SELECT_UP_POS);
}

LTIntPt CLayoutMgr::GetDownArrowOffset(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_SELECT_DOWN_POS))
	{
		return GetPoint(pTag, LO_SELECT_DOWN_POS);
	}
	return GetPoint(LO_SELECT_TAG, LO_SELECT_DOWN_POS);
}




// ------------------------------------------------------------------------//
//
//	Specific Folder Layouts
//
// ------------------------------------------------------------------------//
LTIntPt CLayoutMgr::GetUpArrowPos(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_UP_POS))
	{
		return GetPoint(pTag, LO_FOLDER_UP_POS);
	}
	return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_UP_POS);
}

LTIntPt CLayoutMgr::GetDownArrowPos(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_DOWN_POS))
	{
		return GetPoint(pTag, LO_FOLDER_DOWN_POS);
	}
	return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_DOWN_POS);
}


LTIntPt  CLayoutMgr::GetFolderTitlePos(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_TITLE_POS))
	{
		return GetPoint(pTag, LO_FOLDER_TITLE_POS);
	}
	else
		return GetPoint(LO_DEFAULT_TAG, LO_FOLDER_TITLE_POS);
}

int		CLayoutMgr::GetFolderTitleAlign(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_TITLE_ALIGN))
	{
		return GetInt(pTag, LO_FOLDER_TITLE_ALIGN);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_TITLE_ALIGN);
}

void	CLayoutMgr::GetFolderBackground(eFolderID folderId, char *pBuf, int nBufLen)
{
	char* pTag = s_aFolderTag[folderId];

	if (m_buteMgr.Exist(pTag,LO_FOLDER_BACKGROUND))
	{
		GetString(pTag, LO_FOLDER_BACKGROUND, pBuf, nBufLen);
	}
	else
		GetString(LO_DEFAULT_TAG, LO_FOLDER_BACKGROUND, pBuf, nBufLen);
}

LTRect   CLayoutMgr::GetFolderPageRect(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];

	if (m_buteMgr.Exist(pTag,LO_FOLDER_PAGE_RECT))
	{
		return GetRect(pTag, LO_FOLDER_PAGE_RECT);
	}
	else
		return GetRect(LO_DEFAULT_TAG, LO_FOLDER_PAGE_RECT);
}

int		CLayoutMgr::GetFolderItemSpacing(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_ITEM_SPACE))
	{
		return GetInt(pTag, LO_FOLDER_ITEM_SPACE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_ITEM_SPACE);
}

int		CLayoutMgr::GetFolderItemAlign(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_ITEM_ALIGN))
	{
		return GetInt(pTag, LO_FOLDER_ITEM_ALIGN);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_ITEM_ALIGN);
}

int		CLayoutMgr::GetFolderMusicIntensity(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_MUSIC_INTENSITY))
	{
		return GetInt(pTag, LO_FOLDER_MUSIC_INTENSITY);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_FOLDER_MUSIC_INTENSITY);
}


INT_CHAR* CLayoutMgr::GetFolderCharacter(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	if (m_buteMgr.Exist(pTag,LO_FOLDER_CHARACTER))
	{
		char szTest[128];
		GetString(pTag,LO_FOLDER_CHARACTER,szTest,sizeof(szTest));
		int i = 0;
		while (i < (int)m_CharacterArray.GetSize())
		{
			if (stricmp(szTest,m_CharacterArray[i]->szName) == 0)
				return m_CharacterArray[i];
			i++;
		}

	}
	return LTNULL;
}

int	CLayoutMgr::GetFolderNumAttachments(eFolderID folderId)
{
	char* pTag = s_aFolderTag[folderId];
	int nNum = 0;
	sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, nNum);
	while (m_buteMgr.Exist(pTag,s_aAttName))
	{
		nNum++;
		sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, nNum);
	}
	return nNum;

}


void CLayoutMgr::GetFolderAttachment(eFolderID folderId, int num, char *pBuf, int nBufLen)
{
	char* pTag = s_aFolderTag[folderId];
	sprintf(s_aAttName, "%s%d", LO_FOLDER_ATTACH, num);
	if (m_buteMgr.Exist(pTag,s_aAttName))
	{
		GetString(pTag,s_aAttName,pBuf,nBufLen);
	}
}



// Custom Folder values
LTBOOL CLayoutMgr::HasCustomValue(eFolderID folderId, char *pAttribute)
{
	return m_buteMgr.Exist(s_aFolderTag[folderId], pAttribute);
}

LTIntPt  CLayoutMgr::GetFolderCustomPoint(eFolderID folderId, char *pAttribute)
{
	return GetPoint(s_aFolderTag[folderId], pAttribute);
}

LTRect   CLayoutMgr::GetFolderCustomRect(eFolderID folderId, char *pAttribute)
{
	return GetRect(s_aFolderTag[folderId], pAttribute);
}

int		CLayoutMgr::GetFolderCustomInt(eFolderID folderId, char *pAttribute)
{
	return GetInt(s_aFolderTag[folderId], pAttribute);
}

LTFLOAT CLayoutMgr::GetFolderCustomFloat(eFolderID folderId, char *pAttribute)
{
	return GetFloat(s_aFolderTag[folderId], pAttribute);
}

void	CLayoutMgr::GetFolderCustomString(eFolderID folderId, char *pAttribute, char *pBuf, int nBufLen)
{
	GetString(s_aFolderTag[folderId], pAttribute, pBuf, nBufLen);
}

LTVector CLayoutMgr::GetFolderCustomVector(eFolderID folderId, char *pAttribute)
{
	return GetVector(s_aFolderTag[folderId], pAttribute);
}


// ------------------------------------------------------------------------//
//
//	HUD Layouts
//
// ------------------------------------------------------------------------//
int CLayoutMgr::GetLayoutName(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_LAYOUT_NAME))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName,LO_HUD_LAYOUT_NAME);
}


LTBOOL CLayoutMgr::GetUseAmmoBar(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_USEBAR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_AMMO_USEBAR);
}

LTIntPt CLayoutMgr::GetAmmoBasePos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_BASEPOS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_BASEPOS);
}

LTIntPt CLayoutMgr::GetAmmoClipOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_BAROFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_BAROFF);
}


LTIntPt CLayoutMgr::GetAmmoBarOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_OFFSET))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_OFFSET);
}


LTBOOL CLayoutMgr::GetUseAmmoText(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_USETEXT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_AMMO_USETEXT);
}

LTIntPt CLayoutMgr::GetAmmoTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_TEXTOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_TEXTOFF);
}


LTIntPt CLayoutMgr::GetAmmoIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_ICONOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_ICONOFF);
}



LTBOOL CLayoutMgr::GetUseHealthBar(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_USEBAR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_HEALTH_USEBAR);
}

LTIntPt CLayoutMgr::GetHealthBasePos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_BASEPOS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HEALTH_BASEPOS);
}

LTIntPt CLayoutMgr::GetHealthBarOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_BAROFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HEALTH_BAROFF);
}


LTIntPt CLayoutMgr::GetArmorBarOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ARMOR_OFFSET))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_ARMOR_OFFSET);
}


LTBOOL CLayoutMgr::GetUseHealthText(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_USETEXT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_HEALTH_USETEXT);
}

LTIntPt CLayoutMgr::GetHealthTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_TEXTOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HEALTH_TEXTOFF);
}

LTIntPt CLayoutMgr::GetArmorTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ARMOR_TEXTOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_ARMOR_TEXTOFF);
}


LTBOOL CLayoutMgr::GetUseHealthIcon(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_USEICON))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_HEALTH_USEICON);
}

LTIntPt CLayoutMgr::GetHealthIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_ICONOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HEALTH_ICONOFF);
}

LTIntPt CLayoutMgr::GetArmorIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ARMOR_ICONOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_ARMOR_ICONOFF);
}


LTBOOL CLayoutMgr::GetUseAirIcon(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_USEICON))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_AIR_USEICON);
}

LTIntPt CLayoutMgr::GetAirBasePos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_BASEPOS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AIR_BASEPOS);
}

LTIntPt CLayoutMgr::GetAirIconOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_ICONOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AIR_ICONOFF);
}


LTBOOL CLayoutMgr::GetUseAirText(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_USETEXT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_AIR_USETEXT);
}


LTIntPt CLayoutMgr::GetAirTextOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_TEXTOFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AIR_TEXTOFF);
}



LTBOOL CLayoutMgr::GetUseAirBar(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_USEBAR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetBool(s_aTagName,LO_HUD_AIR_USEBAR);
}

LTIntPt CLayoutMgr::GetAirBarOffset(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_BAROFF))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AIR_BAROFF);
}


int CLayoutMgr::GetBarHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_BAR_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_BAR_HEIGHT);
}

LTFLOAT CLayoutMgr::GetBarScale(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_BAR_SCALE))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_BAR_SCALE);
}

LTIntPt CLayoutMgr::GetModeTextPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_MODE_TEXTPOS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_MODE_TEXTPOS);
}

LTIntPt CLayoutMgr::GetDamageBasePos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DAMAGE_BASEPOS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_DAMAGE_BASEPOS);
}



// ------------------------------------------------------------------------//
//
//	Miscellaneous Layout
//
// ------------------------------------------------------------------------//

LTFLOAT  CLayoutMgr::GetFlashSpeed()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHSPEED, 0.0f);
}

LTFLOAT  CLayoutMgr::GetFlashDuration()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FLASHDUR, 0.0f);
}

LTVector CLayoutMgr::GetNightVisionModelColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_NV_MODEL, vRet);
}

LTVector CLayoutMgr::GetNightVisionScreenTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_NV_SCREEN, vDef);
	return (vRet * MATH_ONE_OVER_255);
}

LTVector CLayoutMgr::GetInfraredModelColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_IR_MODEL, vRet);
}

LTVector CLayoutMgr::GetInfraredLightScale()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_IR_LIGHT, vDef);
	return (vRet * MATH_ONE_OVER_255);
}


LTVector CLayoutMgr::GetMineDetectScreenTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_MD_SCREEN, vDef);
	return (vRet * MATH_ONE_OVER_255);
}


LTVector CLayoutMgr::GetWeaponPickupColor()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_WPN_COLOR, vDef);
	return (vRet * MATH_ONE_OVER_255);
}


LTVector CLayoutMgr::GetAmmoPickupColor()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_AMMO_COLOR, vDef);
	return (vRet * MATH_ONE_OVER_255);
}

LTVector CLayoutMgr::GetChooserHighlightColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_CHOOSER_HCOLOR, vRet);
}

LTFLOAT CLayoutMgr::GetTintTime()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_TINTTIME, 0.0f);
}

LTFLOAT CLayoutMgr::GetCrosshairGapMin()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_GAPMIN, 0.0f);
}

LTFLOAT CLayoutMgr::GetCrosshairGapMax()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_GAPMAX, 0.0f);
}

LTFLOAT CLayoutMgr::GetCrosshairBarMin()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_BARMIN, 0.0f);
}

LTFLOAT CLayoutMgr::GetCrosshairBarMax()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_BARMAX, 0.0f);
}

LTFLOAT CLayoutMgr::GetPerturbRotationEffect()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_ROTEFFECT, 0.0f);
}

LTFLOAT CLayoutMgr::GetPerturbIncreaseSpeed()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_PERTURBINC, 0.0f);
}

LTFLOAT CLayoutMgr::GetPerturbDecreaseSpeed()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_PERTURBDEC, 0.0f);
}

LTFLOAT CLayoutMgr::GetPerturbWalkPercent()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_WALKPER, 0.0f);
}


LTBOOL CLayoutMgr::IsMaskSprite(eOverlayMask eMask)
{
	LTBOOL bFound = LTFALSE;

	switch (eMask)
	{
	case OVM_SUNGLASS:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SUNSPRITE);
		break;
	case OVM_SCOPE:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SCOPESPRITE);
		break;
	case OVM_SCUBA:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SCUBASPRITE);
		break;
	case OVM_SPACE:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_SPACESPRITE);
		break;
	case OVM_STATIC:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_STATICSPRITE);
		break;
	case OVM_CAMERA:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_CAMERASPRITE);
		break;
	case OVM_ZOOM_IN:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_ZOOMIN_SPRITE);
		break;
	case OVM_ZOOM_OUT:
		bFound = m_buteMgr.Exist(LO_MASK_TAG, LO_MASK_ZOOMOUT_SPRITE);
		break;
	}
	return bFound;
}


void CLayoutMgr::GetMaskSprite(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNSPRITE, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPESPRITE, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBASPRITE, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACESPRITE, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICSPRITE, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERASPRITE, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_SPRITE, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_SPRITE, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}
}

void CLayoutMgr::GetCameraActivateSprite(char *pBuf, int nBufLen)
{
	GetString(LO_MASK_TAG, LO_MASK_CAMERAACTIVATESPRITE, pBuf, nBufLen);
}

void CLayoutMgr::GetMaskModel(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNMODEL, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPEMODEL, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBAMODEL, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACEMODEL, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICMODEL, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERAMODEL, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_MODEL, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_MODEL, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}

}


void CLayoutMgr::GetMaskSkin(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	switch (eMask)
	{
	case OVM_SUNGLASS:
		GetString(LO_MASK_TAG, LO_MASK_SUNSKIN, pBuf, nBufLen);
		break;
	case OVM_SCOPE:
		GetString(LO_MASK_TAG, LO_MASK_SCOPESKIN, pBuf, nBufLen);
		break;
	case OVM_SCUBA:
		GetString(LO_MASK_TAG, LO_MASK_SCUBASKIN, pBuf, nBufLen);
		break;
	case OVM_SPACE:
		GetString(LO_MASK_TAG, LO_MASK_SPACESKIN, pBuf, nBufLen);
		break;
	case OVM_STATIC:
		GetString(LO_MASK_TAG, LO_MASK_STATICSKIN, pBuf, nBufLen);
		break;
	case OVM_CAMERA:
		GetString(LO_MASK_TAG, LO_MASK_CAMERASKIN, pBuf, nBufLen);
		break;
	case OVM_ZOOM_IN:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMIN_SKIN, pBuf, nBufLen);
		break;
	case OVM_ZOOM_OUT:
		GetString(LO_MASK_TAG, LO_MASK_ZOOMOUT_SKIN, pBuf, nBufLen);
		break;
	default:
        pBuf[0] = LTNULL;
		break;
	}

}


LTFLOAT CLayoutMgr::GetMaskScale(eOverlayMask eMask)
{
    LTFLOAT fRet = 1.0f;
	switch (eMask)
	{
	case OVM_SUNGLASS:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SUNSCALE, 0.0f);
		break;
	case OVM_SCOPE:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SCOPESCALE, 0.0f);
		break;
	case OVM_SCUBA:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SCUBASCALE, 0.0f);
		break;
	case OVM_SPACE:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_SPACESCALE, 0.0f);
		break;
	case OVM_STATIC:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_STATICSCALE, 0.0f);
		break;
	case OVM_CAMERA:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_CAMERASCALE, 0.0f);
		break;
	case OVM_ZOOM_IN:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_ZOOMIN_SCALE, 0.0f);
		break;
	case OVM_ZOOM_OUT:
        fRet = (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, LO_MASK_ZOOMOUT_SCALE, 0.0f);
		break;
	}
	return fRet;
}





void CLayoutMgr::GetMessageBoxBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MB_BACK, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetMessageBoxAlpha()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MB_ALPHA);
}

void CLayoutMgr::GetFailScreenBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_FAILBACK, pBuf, nBufLen);
}


LTIntPt  CLayoutMgr::GetFailStringPos()
{
	return GetPoint(LO_MISC_TAG, LO_MISC_FAILPOS);
}

LTFLOAT  CLayoutMgr::GetFailScreenDelay()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_FAILDELAY, 0.0f);
}

LTFLOAT  CLayoutMgr::GetDeathDelay()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_DEATHDELAY, 0.0f);
}

void CLayoutMgr::GetHelpFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_HELPFONT, pBuf, nBufLen);
}


void CLayoutMgr::GetSmallFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_SMALLFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetMediumFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MEDFONT, pBuf, nBufLen);
}


void CLayoutMgr::GetLargeFontBase(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_LARGEFONT, pBuf, nBufLen);
}


void CLayoutMgr::GetTitleFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_TITLEFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetMsgForeFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_MSGFOREFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetHUDForeFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_HUDFOREFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetAirFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_AIRFONT, pBuf, nBufLen);
}

void CLayoutMgr::GetChooserFont(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_CHOOSERFONT, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetMessageFade()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_FADE);
}

LTFLOAT CLayoutMgr::GetMessageTime()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_TIME);
}

int CLayoutMgr::GetMaxNumMessages()
{
	return GetInt(LO_MISC_TAG, LO_MISC_MSG_NUM);
}


LTRect   CLayoutMgr::GetObjectiveRect()
{
	return GetRect(LO_MISC_TAG, LO_MISC_OBJ_RECT);
}

LTRect   CLayoutMgr::GetPopupTextRect()
{
	return GetRect(LO_MISC_TAG, LO_MISC_POPUP_RECT);
}

HLTCOLOR CLayoutMgr::GetSubtitleTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_SUB_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetHealthTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_HEALTH_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetArmorTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_ARMOR_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetAmmoTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_AMMO_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetPopupTextTint()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_POPUP_TINT, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetTeam1Color()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_TEAM1_COLOR, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

HLTCOLOR CLayoutMgr::GetTeam2Color()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_TEAM2_COLOR, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}




int CLayoutMgr::GetMissionTextWidth()
{
	return GetInt(LO_MT_TAG, LO_MT_WIDTH);
}

int CLayoutMgr::GetMissionTextNumLines()
{
	return GetInt(LO_MT_TAG, LO_MT_NUM_LINES);
}

LTFLOAT CLayoutMgr::GetMissionTextLetterDelay()
{
	return GetFloat(LO_MT_TAG, LO_MT_LETTER_DELAY);
}

LTFLOAT CLayoutMgr::GetMissionTextLineDelay()
{
	return GetFloat(LO_MT_TAG, LO_MT_LINE_DELAY);
}

LTFLOAT CLayoutMgr::GetMissionTextLineScrollTime()
{
	return GetFloat(LO_MT_TAG, LO_MT_LINE_SCROLL);
}

LTFLOAT CLayoutMgr::GetMissionTextFadeDelay()
{
	return GetFloat(LO_MT_TAG, LO_MT_FADE_DELAY);
}

LTFLOAT CLayoutMgr::GetMissionTextFadeTime()
{
	return GetFloat(LO_MT_TAG, LO_MT_FADE_TIME);
}

LTIntPt CLayoutMgr::GetMissionTextPos()
{
	return GetPoint(LO_MT_TAG, LO_MT_POS);
}

void CLayoutMgr::GetMissionTextTypeSound(char *pBuf, int nBufLen)
{
	GetString(LO_MT_TAG, LO_MT_TYPE_SOUND, pBuf, nBufLen);
}
void CLayoutMgr::GetMissionTextScrollSound(char *pBuf, int nBufLen)
{
	GetString(LO_MT_TAG, LO_MT_SCROLL_SOUND, pBuf, nBufLen);
}


int CLayoutMgr::GetSubtitleNumLines()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_NUM_LINES);
}

LTFLOAT CLayoutMgr::GetSubtitleLineScrollTime()
{
	return GetFloat(LO_SUBTITLE_TAG, LO_SUBTITLE_LINE_SCROLL);
}

int CLayoutMgr::GetSubtitleCinematicWidth()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_CINE_WIDTH);
}

LTIntPt CLayoutMgr::GetSubtitleCinematicPos()
{
	return GetPoint(LO_SUBTITLE_TAG, LO_SUBTITLE_CINE_POS);
}

int CLayoutMgr::GetSubtitleFullScreenWidth()
{
	return GetInt(LO_SUBTITLE_TAG, LO_SUBTITLE_FULL_WIDTH);
}

LTIntPt CLayoutMgr::GetSubtitleFullScreenPos()
{
	return GetPoint(LO_SUBTITLE_TAG, LO_SUBTITLE_FULL_POS);
}


int CLayoutMgr::GetDialoguePosition()
{
	return GetInt(LO_DIALOGUE_TAG, LO_WINDOW_POS);
}

LTIntPt CLayoutMgr::GetDialogueSize()
{
	return GetPoint(LO_DIALOGUE_TAG, LO_WINDOW_SIZE);
}
LTIntPt CLayoutMgr::GetDialogueTextOffset()
{
	return GetPoint(LO_DIALOGUE_TAG, LO_WINDOW_TEXT_OFFSET);
}

void CLayoutMgr::GetDialogueBackground(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetDialogueAlpha()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_ALPHA);
}

void CLayoutMgr::GetDialogueFrame(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_FRAME, pBuf, nBufLen);
}

int CLayoutMgr::GetDialogueFont()
{
	return GetInt(LO_DIALOGUE_TAG, LO_WINDOW_FONT);
}

LTFLOAT CLayoutMgr::GetDialogueOpenTime()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetDialogueCloseTime()
{
	return GetFloat(LO_DIALOGUE_TAG, LO_WINDOW_CLOSE);
}

void CLayoutMgr::GetDialogueOpenSound(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_SND_OPEN, pBuf, nBufLen);
}

void CLayoutMgr::GetDialogueCloseSound(char *pBuf, int nBufLen)
{
	GetString(LO_DIALOGUE_TAG, LO_WINDOW_SND_CLOSE, pBuf, nBufLen);
}



int CLayoutMgr::GetDecisionPosition()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_POS);
}


LTIntPt CLayoutMgr::GetDecisionTextOffset()
{
	return GetPoint(LO_DECISION_TAG, LO_WINDOW_TEXT_OFFSET);
}

int CLayoutMgr::GetDecisionSpacing()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_SPACING);
}

void CLayoutMgr::GetDecisionBackground(char *pBuf, int nBufLen)
{
	GetString(LO_DECISION_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetDecisionAlpha()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_ALPHA);
}


int CLayoutMgr::GetDecisionFont()
{
	return GetInt(LO_DECISION_TAG, LO_WINDOW_FONT);
}


LTFLOAT CLayoutMgr::GetDecisionOpenTime()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetDecisionCloseTime()
{
	return GetFloat(LO_DECISION_TAG, LO_WINDOW_CLOSE);
}



int CLayoutMgr::GetMenuPosition()
{
	return GetInt(LO_MENU_TAG, LO_WINDOW_POS);
}


LTIntPt CLayoutMgr::GetMenuTextOffset()
{
	return GetPoint(LO_MENU_TAG, LO_WINDOW_TEXT_OFFSET);
}

int CLayoutMgr::GetMenuSpacing()
{
	return GetInt(LO_MENU_TAG, LO_WINDOW_SPACING);
}

void CLayoutMgr::GetMenuBackground(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_BACK, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuFrame(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_FRAME, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetMenuAlpha()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_ALPHA);
}


LTFLOAT CLayoutMgr::GetMenuOpenTime()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_OPEN);
}

LTFLOAT CLayoutMgr::GetMenuCloseTime()
{
	return GetFloat(LO_MENU_TAG, LO_WINDOW_CLOSE);
}

void CLayoutMgr::GetMenuOpenSound(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_SND_OPEN, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuCloseSound(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_WINDOW_SND_CLOSE, pBuf, nBufLen);
}

//-------------------------------------------------------------------------
LTFLOAT CLayoutMgr::GetCreditsFadeInTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_FADEIN);
}

LTFLOAT CLayoutMgr::GetCreditsHoldTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_HOLD);
}

LTFLOAT CLayoutMgr::GetCreditsFadeOutTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_FADEOUT);
}

LTFLOAT CLayoutMgr::GetCreditsDelayTime()
{
	return GetFloat(LO_CREDITS_TAG, LO_CREDITS_DELAY);
}

LTIntPt CLayoutMgr::GetCreditsPositionUL()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_UL);
}

LTIntPt CLayoutMgr::GetCreditsPositionUR()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_UR);
}

LTIntPt CLayoutMgr::GetCreditsPositionLL()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_LL);
}

LTIntPt CLayoutMgr::GetCreditsPositionLR()
{
	return GetPoint(LO_CREDITS_TAG, LO_CREDITS_POS_LR);
}



// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//

LTBOOL CLayoutMgr::GetBool(char *pTag,char *pAttribute)
{
    return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, 0);
}

LTFLOAT CLayoutMgr::GetFloat(char *pTag,char *pAttribute)
{
    return (LTFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, 0.0f);
}

int	CLayoutMgr::GetInt(char *pTag,char *pAttribute)
{
	return m_buteMgr.GetInt(pTag, pAttribute, 0);
}

LTIntPt CLayoutMgr::GetPoint(char *pTag,char *pAttribute)
{
    CPoint zero(0,0);
    CPoint tmp = m_buteMgr.GetPoint(pTag, pAttribute, zero);
    LTIntPt pt(tmp.x,tmp.y);
	return pt;
}

LTRect CLayoutMgr::GetRect(char *pTag,char *pAttribute)
{
    CRect zero(0,0,0,0);
    CRect tmp = m_buteMgr.GetRect(pTag, pAttribute, zero );
    LTRect rect(tmp.left,tmp.top,tmp.right,tmp.bottom);
	return rect;

}

void CLayoutMgr::GetString(char *pTag,char *pAttribute,char *pBuf, int nBufLen)
{
	CString str = "";
	str = m_buteMgr.GetString(pTag, pAttribute);
	strncpy(pBuf, (char*)(LPCSTR)str, nBufLen);
}

LTVector CLayoutMgr::GetVector(char *pTag,char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}