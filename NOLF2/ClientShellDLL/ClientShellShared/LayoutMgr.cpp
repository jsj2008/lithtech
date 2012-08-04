// ----------------------------------------------------------------------- //
//
// MODULE  : LayoutMgr.cpp
//
// PURPOSE : Attribute file manager for interface layout info
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LayoutMgr.h"
#include "InterfaceMgr.h"


#define LO_BASIC_TAG					"BasicLayout"
#define LO_SELECT_TAG					"SelectionLayout"
#define LO_DEFAULT_TAG					"GenericScreen"
#define LO_HUD_TAG						"HUDLayout"
#define LO_MISC_TAG						"Miscellaneous"
#define LO_MASK_TAG						"Overlay"
#define LO_DIALOGUE_TAG					"DialogueWindow"
#define LO_DECISION_TAG					"DecisionWindow"
#define LO_MENU_TAG						"Menu"
#define LO_CHAR_TAG						"Character"
#define LO_LIGHT_TAG					"Light"
#define LO_FX_TAG						"FX"
#define LO_CREDITS_TAG					"Credits"
#define LO_TRANSITION_TAG				"Transition"

#define LO_BASIC_HELP_RECT				"HelpRect"
#define LO_BASIC_HELP_FONT				"HelpFont"
#define LO_BASIC_HELP_SIZE				"HelpSize"
#define LO_BASIC_HUD_FONT				"HUDFont"
#define LO_BASIC_BACK_FONT				"BackFont"
#define LO_BASIC_BACK_SIZE				"BackSize"
#define LO_BASIC_BACK_POS				"BackPos"
#define LO_BASIC_NEXT_POS				"NextPos"
#define LO_BASIC_BACK_COLOR				"BackColor"
#define LO_BASIC_BACK_SPRITE			"BackSprite"
#define LO_BASIC_BACK_SCALE				"BackSpriteScale"
#define LO_BASIC_SLIDER					"SliderTex"
#define LO_BASIC_ARROW_BACK				"ArrowBackTex"
#define LO_BASIC_ARROW_BACK_H			"ArrowBackTexH"
#define LO_BASIC_ARROW_BACK_POS			"ArrowBackPos"
#define LO_BASIC_ARROW_NEXT				"ArrowNextTex"
#define LO_BASIC_ARROW_NEXT_H			"ArrowNextTexH"
#define LO_BASIC_ARROW_NEXT_POS			"ArrowNextPos"
#define LO_BASIC_BOSS_RECT				"BossRect"

#define LO_SCREEN_TITLE_POS				"TitlePos"
#define LO_SCREEN_TITLE_FONT			"TitleFont"
#define LO_SCREEN_TITLE_SIZE			"TitleSize"
#define LO_SCREEN_BACKGROUND			"Background"
#define LO_SCREEN_PAGE_RECT				"PageRect"
#define LO_SCREEN_ITEM_FONT				"FontFace"
#define LO_SCREEN_ITEM_SIZE				"FontSize"
#define LO_SCREEN_ITEM_SPACE			"ItemSpace"
#define LO_SCREEN_ITEM_ALIGN			"ItemAlign"
#define LO_SCREEN_MUSIC_INTENSITY		"MusicIntensity"
#define LO_SCREEN_CHARACTER				"Character"
#define LO_SCREEN_ATTACH				"Attachment"

#define LO_HUD_LAYOUT_NAME				"LayoutName"

#define	LO_HUD_AMMO_BASEPOS				"AmmoBasePos"
#define	LO_HUD_AMMO_USEBAR				"UseAmmoBar"
#define	LO_HUD_AMMO_BAROFF				"AmmoClipOffset"
#define	LO_HUD_AMMO_CLIPSZ				"AmmoClipUnitSize"
#define	LO_HUD_AMMO_OFFSET				"AmmoBarOffset"
#define	LO_HUD_AMMO_USETEXT				"UseAmmoText"
#define	LO_HUD_AMMO_TEXTOFF				"AmmoTextOffset"
#define	LO_HUD_AMMO_ICONOFF				"AmmoIconOffset"
#define	LO_HUD_AMMO_ICONSZ				"AmmoIconSize"
#define	LO_HUD_AMMO_ALPHA				"AmmoTextAlpha"
#define	LO_HUD_AMMO_COLOR				"AmmoTextColor"

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
#define	LO_HUD_HEALTH_ICONSZ			"HealthIconSize"
#define	LO_HUD_HEALTH_ALPHA				"HealthAlpha"
#define	LO_HUD_HEALTH_COLOR				"HealthColor"
#define	LO_HUD_ARMOR_COLOR				"ArmorColor"

#define	LO_HUD_AIR_BASEPOS				"AirBasePos"
#define	LO_HUD_AIR_USEICON				"UseAirIcon"
#define	LO_HUD_AIR_ICONOFF				"AirIconOffset"
#define	LO_HUD_AIR_ICONSZ				"AirIconSize"
#define	LO_HUD_AIR_USETEXT				"UseAirText"
#define	LO_HUD_AIR_TEXTOFF				"AirTextOffset"
#define	LO_HUD_AIR_USEBAR				"UseAirBar"
#define	LO_HUD_AIR_BAROFF				"AirBarOffset"
#define	LO_HUD_AIR_ALPHA				"AirAlpha"
#define	LO_HUD_AIR_COLOR				"AirColor"

#define	LO_HUD_BAR_HEIGHT				"BarHeight"
#define	LO_HUD_BAR_SCALE				"BarScale"
#define	LO_HUD_TEXT_HEIGHT				"TextHeight"

#define LO_HUD_DAMAGE_BASEPOS			"DamageBasePos"
#define LO_HUD_DAMAGE_ICON_SZ			"DamageIconSize"

#define LO_HUD_COMPASS_POS				"CompassPos"
#define LO_HUD_COMPASS_SZ				"CompassSize"

#define LO_HUD_RADAR_OBJSZ				"RadarObjectSize"
#define LO_HUD_RADAR_MAXDIST			"RadarMaxDistance"
#define LO_HUD_RADAR_LIVE				"RadarLivePlayerColor"
#define LO_HUD_RADAR_DEAD				"RadarDeadPlayerColor"
#define LO_HUD_RADAR_TALK				"RadarTalkPlayerColor"
#define LO_HUD_RADAR_FLASH				"RadarFlashTime"

#define LO_HUD_DAMAGE_SZ				"DamageSize"

#define LO_HUD_CARRY_POS				"CarryPos"
#define LO_HUD_CARRY_SZ					"CarrySize"

#define LO_HUD_OBJ_POS					"ObjectivePos"
#define LO_HUD_OBJ_SZ					"ObjectiveSize"
#define LO_HUD_OBJ_BLINKDUR				"ObjectiveBlinkDuration"
#define LO_HUD_OBJ_BLINKSPD				"ObjectiveBlinkSpeed"

#define LO_HUD_HIDE_POS					"HideIconPos"
#define LO_HUD_HIDE_SZ					"HideIconSize"
#define LO_HUD_HIDE_SPEED				"HideIconBlinkSpeed"
#define LO_HUD_HIDE_ALPHA				"HideIconAlpha"
#define	LO_HUD_HIDE_BASEPOSY			"HidingBarBasePosY"
#define LO_HUD_HIDE_OFFSET				"HidingBarOffset"
#define LO_HUD_HIDE_HEIGHT				"HidingBarHeight"
#define LO_HUD_HIDE_SCALE				"HidingBarScale"
#define LO_HUD_HIDE_TEX					"HidingBarTexture"

#define LO_HUD_DIST_POS					"DistanceIconPos"
#define LO_HUD_DIST_SPEED				"DistanceIconBlinkSpeed"
#define LO_HUD_DIST_ALPHA				"DistanceIconAlpha"
#define LO_HUD_DIST_FADESPEED			"DistanceIconFadeOutSpeed"

#define LO_HUD_ACT_POS					"ActivationTextPos"
#define LO_HUD_ACT_SZ					"ActivationTextSize"
#define LO_HUD_ACT_JUST					"ActivationTextJustify"
#define LO_HUD_ACT_ALPHA				"ActivationTextAlpha"
#define LO_HUD_ACT_COLOR				"ActivationTextColor"
#define LO_HUD_ACT_DIS_ALPHA			"ActivationTextDisAlpha"
#define LO_HUD_ACT_DIS_COLOR			"ActivationTextDisColor"

#define LO_HUD_DBG_POS					"DebugTextPos"
#define LO_HUD_DBG_SZ					"DebugTextSize"
#define LO_HUD_DBG_WD					"DebugTextWidth"
#define LO_HUD_DBG_JUST					"DebugTextJustify"
#define LO_HUD_DBG_ALPHA				"DebugTextAlpha"
#define LO_HUD_DBG_COLOR				"DebugTextColor"

#define LO_HUD_CHS_SZ					"ChooserTextSize"
#define LO_HUD_CHS_TEX					"ChooserTextureScale"
#define LO_HUD_CHS_HT					"ChooserIconHeight"
#define LO_HUD_CHS_ALPHA				"ChooserTextAlpha"
#define LO_HUD_CHS_COLOR				"ChooserTextColor"

#define	LO_HUD_PROG_BASEPOSY			"ProgressBarBasePosY"
#define LO_HUD_PROG_OFFSET				"ProgressBarOffset"
#define LO_HUD_PROG_HEIGHT				"ProgressBarHeight"
#define LO_HUD_PROG_SCALE				"ProgressBarScale"
#define LO_HUD_PROG_TEX					"ProgressBarTexture"

#define	LO_HUD_DISPLAYMETER_BASEPOSY	"DisplayMeterBasePosY"
#define LO_HUD_DISPLAYMETER_OFFSET		"DisplayMeterOffset"
#define LO_HUD_DISPLAYMETER_HEIGHT		"DisplayMeterHeight"
#define LO_HUD_DISPLAYMETER_SCALE		"DisplayMeterScale"
#define LO_HUD_DISPLAYMETER_TEX			"DisplayMeterTexture"

#define	LO_HUD_MODE_TEXTPOS				"ModeTextPos"


#define	LO_MISC_SV_MODEL				"SpyVisionModelColor"
#define	LO_MISC_SV_LIGHT				"SpyVisionLightScale"
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
#define LO_MISC_DLG_FRAME				"DialogFrame"
#define LO_MISC_DLG_FONT_FACE			"DialogFontFace"
#define LO_MISC_DLG_FONT_SIZE			"DialogFontSize"
#define LO_MISC_DEATHDELAY				"DeathDelay"
#define LO_MISC_MSG_FADE				"MessageMinimumFade"
#define LO_MISC_MSG_TIME				"MessageMinimumTime"
#define LO_MISC_OBJ_RECT				"ObjectiveRect"
#define LO_MISC_POPUP_RECT				"PopupTextRect"
#define LO_MISC_SUB_TINT				"SubtitleTint"
#define LO_MISC_HEALTH_TINT				"HealthTint"
#define LO_MISC_ARMOR_TINT				"ArmorTint"
#define LO_MISC_AMMO_TINT				"AmmoTint"
#define LO_MISC_POPUP_TINT				"PopupTint"

#define	LO_MASK_SPRITE					"Sprite"
#define	LO_MASK_MODEL					"Model"
#define	LO_MASK_SKIN					"Skin"
#define	LO_MASK_SCALE					"Scale"
#define	LO_MASK_ALPHA					"Alpha"

#define	LO_CHAR_NAME					"Name"
#define	LO_CHAR_MOD						"Model"
#define	LO_CHAR_SKIN					"Skin"
#define	LO_CHAR_STYLE					"RenderStyle"
#define	LO_CHAR_POS						"Pos"
#define	LO_CHAR_SCALE					"Scale"
#define	LO_CHAR_ROT						"Rotation"
#define LO_CHAR_MENULAYER				"MenuLayer"

#define	LO_LIGHT_NAME					"Name"
#define	LO_LIGHT_POS					"Pos"
#define	LO_LIGHT_COLOR					"Color"
#define	LO_LIGHT_RADIUS					"Radius"

#define	LO_CREDITS_FADEIN				"FadeInTime"
#define	LO_CREDITS_HOLD					"HoldTime"
#define	LO_CREDITS_FADEOUT				"FadeOutTime"
#define	LO_CREDITS_DELAY				"DelayTime"
#define	LO_CREDITS_POS_UL				"PositionUL"
#define	LO_CREDITS_POS_UR				"PositionUR"
#define	LO_CREDITS_POS_LR				"PositionLR"
#define	LO_CREDITS_POS_LL				"PositionLL"

#define LO_DEFAULT_MENU_TAG				"GenericMenu"
#define LO_MENU_FONT					"Font"
#define LO_MENU_FONT_SIZE				"FontSize"
#define LO_MENU_TITLE_FONT				"TitleFont"
#define LO_MENU_TITLE_FONT_SIZE			"TitleSize"
#define LO_MENU_SIZE					"Size"
#define LO_MENU_FRAME					"Frame"
#define LO_MENU_FRAME_TIP				"FrameTip"
#define LO_MENU_ARROW_U					"UpArrow"
#define LO_MENU_ARROW_U_H				"UpArrowH"
#define LO_MENU_ARROW_D					"DownArrow"
#define LO_MENU_ARROW_D_H				"DownArrowH"
/*
#define LO_MENU_ARROW_L					"LeftArrow"
#define LO_MENU_ARROW_L_H				"LeftArrowH"
#define LO_MENU_ARROW_R					"RightArrow"
#define LO_MENU_ARROW_R_H				"RightArrowH"
#define LO_MENU_CLOSE					"Close"
#define LO_MENU_CLOSE_H					"CloseH"
*/
#define LO_MENU_INDENT					"Indent"
#define LO_MENU_POS						"Position"
#define LO_MENU_IN_TIME					"SlideInTime"
#define LO_MENU_OUT_TIME				"SlideOutTime"

#define LO_SELECTED_COLOR				"SelectedColor"
#define LO_NONSELECTED_COLOR			"NonSelectedColor"
#define LO_DISABLED_COLOR				"DisabledColor"

//CLayoutMgr* g_pLayoutMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[30];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LTBOOL INT_CHAR::Init(CButeMgr & buteMgr, char* aTagName)
{

	buteMgr.GetString(aTagName, LO_CHAR_NAME, "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_CHAR_MOD, "", szModel, sizeof(szModel));
	if (strlen(szModel) == 0) return LTFALSE;

	blrSkins.Read(&buteMgr, aTagName, LO_CHAR_SKIN, 128);
	blrRenderStyles.Read(&buteMgr, aTagName, LO_CHAR_STYLE, 128);


    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, LO_CHAR_POS, zero);
	fScale = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_SCALE, 1.0);
	fRot = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_CHAR_ROT, 1.0);
	nMenuLayer = (uint8)buteMgr.GetInt( aTagName, LO_CHAR_MENULAYER, 0 );

	return LTTRUE;
}


LTBOOL INT_LIGHT::Init(CButeMgr & buteMgr, char* aTagName)
{

	buteMgr.GetString(aTagName, LO_LIGHT_NAME, "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;


    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, LO_LIGHT_POS, zero);
	vColor = buteMgr.GetVector(aTagName, LO_LIGHT_COLOR, zero);
	vColor *= MATH_ONE_OVER_255;

	fRadius = (LTFLOAT)buteMgr.GetDouble(aTagName, LO_LIGHT_RADIUS, 0.0);

	return LTTRUE;
}

LTBOOL INT_FX::Init(CButeMgr & buteMgr, char* aTagName)
{

	buteMgr.GetString(aTagName, "Name", "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, "FXName", "", szFXName, sizeof(szFXName));
	if (strlen(szFXName) == 0) return LTFALSE;

    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, "Pos", zero);

	bLoop = (LTBOOL)buteMgr.GetInt(aTagName, "Loop", 0);

	return LTTRUE;
}

bool INT_CHAINFX::Init(CButeMgr & buteMgr, char *aTagName, int iNum)
{
	sprintf(s_aAttName, "IntroFX%d", iNum);
	buteMgr.GetString(aTagName, s_aAttName, szIntroName, sizeof(szIntroName));

	sprintf(s_aAttName, "ShortIntroFX%d", iNum);
	buteMgr.GetString(aTagName, s_aAttName, szShortIntroName, sizeof(szShortIntroName));

	sprintf(s_aAttName, "LoopFX%d", iNum);
	buteMgr.GetString(aTagName, s_aAttName, szLoopName, sizeof(szLoopName));

	if (szIntroName[0] || szShortIntroName[0] || szLoopName[0])
	{
		return (true);
	}
	return (false);
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
	m_nNumFonts = 0;

}

CLayoutMgr::~CLayoutMgr()
{
	g_pLayoutMgr = LTNULL;
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CLayoutMgr::Init(const char* szAttributeFile)
{
    if (!szAttributeFile) return LTFALSE;
//    if (g_pLayoutMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;


	// Set up global pointer...

//	g_pLayoutMgr = this;

	m_nNumHUDLayouts = 0;
	m_nNumFonts = 0;

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
			m_CharacterArray.push_back(pChar);
		else
		{
			debug_delete(pChar);
		}
		numChar++;
		sprintf(s_aTagName, "%s%d", LO_CHAR_TAG, numChar);
	}

	sprintf(s_aTagName, "%s0", LO_LIGHT_TAG);
	int numLight = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_LIGHT *pLight = debug_new(INT_LIGHT);
		if (pLight->Init(m_buteMgr,s_aTagName))
			m_LightArray.push_back(pLight);
		else
		{
			debug_delete(pLight);
		}
		numLight++;
		sprintf(s_aTagName, "%s%d", LO_LIGHT_TAG, numLight);
	}

	sprintf(s_aTagName, "%s0", LO_FX_TAG);
	int numFX = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_FX *pFX = debug_new(INT_FX);
		if (pFX->Init(m_buteMgr,s_aTagName))
			m_FXArray.push_back(pFX);
		else
		{
			debug_delete(pFX);
		}
		numFX++;
		sprintf(s_aTagName, "%s%d", LO_FX_TAG, numFX);
	}

	// ABM 2/6/02 parser for new layout item, the transition (chained fx mapped
	// to a specific screen-to-screen transition).
	sprintf(s_aTagName, "%s0", LO_TRANSITION_TAG);
	int numTransitions = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		// Get the TransitionFrom and TransitionTo
		int iTransFrom = 0;
		int iTransTo = 0;
		char szName[128];

		szName[0] = 0;
		m_buteMgr.GetString(s_aTagName, "TransitionFrom", szName, sizeof(szName));
		if (strlen(szName))
		{
			iTransFrom = g_pInterfaceMgr->GetScreenMgr()->GetScreenIDFromName(szName);
			if (iTransFrom == 999) iTransFrom = 0;
		}

		szName[0] = 0;
		m_buteMgr.GetString(s_aTagName, "TransitionTo", szName, sizeof(szName));
		if (strlen(szName))
		{
			iTransTo = g_pInterfaceMgr->GetScreenMgr()->GetScreenIDFromName(szName);
			if (iTransTo == 999) iTransTo = 0;
		}

		// Loop through and create a bunch of INT_CHAINFX
		int numChainFX = 0;
		bool bFound;

		do
		{
			INT_CHAINFX * pChainFX = debug_new(INT_CHAINFX);
			bFound = pChainFX->Init(m_buteMgr, s_aTagName, numChainFX);
			if (bFound)
			{
				// copy over the information for this screen
				pChainFX->iFromScreen = iTransFrom;
				pChainFX->iToScreen = iTransTo;
				m_ChainFXArray.push_back(pChainFX);
			}
			else
			{
				debug_delete(pChainFX);
			}
			numChainFX++;
		} while (bFound);

		numTransitions++;
		sprintf(s_aTagName, "%s%d", LO_TRANSITION_TAG, numTransitions);
	} // while


	sprintf(s_aTagName, "Fonts");
	sprintf(s_aAttName, "FontFace0");
	while (m_buteMgr.Exist(s_aTagName,s_aAttName))
	{
		m_nNumFonts++;
		sprintf(s_aAttName, "FontFace%d", m_nNumFonts);
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
//    g_pLayoutMgr = LTNULL;

	CharacterArray::iterator iter = m_CharacterArray.begin();
	while (iter != m_CharacterArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_CharacterArray.clear();

	LightArray::iterator lIter = m_LightArray.begin();
	while (lIter != m_LightArray.end())
	{
		debug_delete(*lIter);
		lIter++;
	}
	m_LightArray.clear();

	FXArray::iterator fxIter = m_FXArray.begin();
	while (fxIter != m_FXArray.end())
	{
		debug_delete(*fxIter);
		fxIter++;
	}
	m_FXArray.clear();
}


// ------------------------------------------------------------------------//
//
//	Basic Screen Layout
//
// ------------------------------------------------------------------------//

LTRect   CLayoutMgr::GetHelpRect()
{
	return GetRect(LO_BASIC_TAG, LO_BASIC_HELP_RECT);
}

uint8 CLayoutMgr::GetHelpFont()
{
	return (uint8)GetInt(LO_BASIC_TAG, LO_BASIC_HELP_FONT);
}

uint8 CLayoutMgr::GetHelpSize()
{
	return (uint8)GetInt(LO_BASIC_TAG, LO_BASIC_HELP_SIZE);
}

uint8 CLayoutMgr::GetHUDFont()
{
	return (uint8)GetInt(LO_BASIC_TAG, LO_BASIC_HUD_FONT);
}


uint8 CLayoutMgr::GetBackFont()
{
	return (uint8)GetInt(LO_BASIC_TAG, LO_BASIC_BACK_FONT);
}

uint8 CLayoutMgr::GetBackSize()
{
	return (uint8)GetInt(LO_BASIC_TAG, LO_BASIC_BACK_SIZE);
}

LTIntPt  CLayoutMgr::GetBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_BACK_POS);
}

LTIntPt CLayoutMgr::GetNextPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_NEXT_POS);
}

HLTCOLOR CLayoutMgr::GetBackColor()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vColor = m_buteMgr.GetVector(LO_BASIC_TAG, LO_BASIC_BACK_COLOR, vDef);
	return SETRGB(vColor.x,vColor.y,vColor.z);
}

void CLayoutMgr::GetBackSprite(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_BACK_SPRITE, pBuf, nBufLen);
}

LTFLOAT CLayoutMgr::GetBackSpriteScale()
{
	return GetFloat(LO_BASIC_TAG, LO_BASIC_BACK_SCALE);
}

void CLayoutMgr::GetSliderTex(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_SLIDER, pBuf, nBufLen);
}

void CLayoutMgr::GetArrowBackTex(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowBackTexH(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_H, pBuf, nBufLen);
}
LTIntPt  CLayoutMgr::GetArrowBackPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_BACK_POS);
}
void CLayoutMgr::GetArrowNextTex(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT, pBuf, nBufLen);
}
void CLayoutMgr::GetArrowNextTexH(char *pBuf, int nBufLen)
{
	GetString(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_H, pBuf, nBufLen);
}
LTIntPt  CLayoutMgr::GetArrowNextPos()
{
	return GetPoint(LO_BASIC_TAG, LO_BASIC_ARROW_NEXT_POS);
}

LTRect   CLayoutMgr::GetBossRect()
{
	return GetRect(LO_BASIC_TAG, LO_BASIC_BOSS_RECT);
}





// ------------------------------------------------------------------------//
//
//	Specific Screen Layouts
//
// ------------------------------------------------------------------------//

LTIntPt  CLayoutMgr::GetScreenTitlePos(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_TITLE_POS))
	{
		return GetPoint(pTag, LO_SCREEN_TITLE_POS);
	}
	else
		return GetPoint(LO_DEFAULT_TAG, LO_SCREEN_TITLE_POS);
}

uint8 CLayoutMgr::GetScreenTitleFont(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_TITLE_FONT))
	{
		return GetInt(pTag, LO_SCREEN_TITLE_FONT);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_TITLE_FONT);
}

uint8 CLayoutMgr::GetScreenTitleSize(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_TITLE_SIZE))
	{
		return GetInt(pTag, LO_SCREEN_TITLE_SIZE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_TITLE_SIZE);
}

uint8 CLayoutMgr::GetScreenFontFace(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_ITEM_FONT))
	{
		return GetInt(pTag, LO_SCREEN_ITEM_FONT);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_ITEM_FONT);
}

uint8 CLayoutMgr::GetScreenFontSize(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_ITEM_SIZE))
	{
		return GetInt(pTag, LO_SCREEN_ITEM_SIZE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_ITEM_SIZE);
}

LTRect   CLayoutMgr::GetScreenPageRect(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);

	if (m_buteMgr.Exist(pTag,LO_SCREEN_PAGE_RECT))
	{
		return GetRect(pTag, LO_SCREEN_PAGE_RECT);
	}
	else
		return GetRect(LO_DEFAULT_TAG, LO_SCREEN_PAGE_RECT);
}

int		CLayoutMgr::GetScreenItemSpacing(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_ITEM_SPACE))
	{
		return GetInt(pTag, LO_SCREEN_ITEM_SPACE);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_ITEM_SPACE);
}

int		CLayoutMgr::GetScreenItemAlign(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_ITEM_ALIGN))
	{
		return GetInt(pTag, LO_SCREEN_ITEM_ALIGN);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_ITEM_ALIGN);
}

int		CLayoutMgr::GetScreenMusicIntensity(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_MUSIC_INTENSITY))
	{
		return GetInt(pTag, LO_SCREEN_MUSIC_INTENSITY);
	}
	else
		return GetInt(LO_DEFAULT_TAG, LO_SCREEN_MUSIC_INTENSITY);
}

void CLayoutMgr::GetScreenMouseFX(eScreenID screenId, char *pBuf, int nBufLen)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,"MouseFX"))
	{
		GetString(pTag, "MouseFX",pBuf,nBufLen);
	}
	else
		GetString(LO_DEFAULT_TAG, "MouseFX",pBuf,nBufLen);
}
void CLayoutMgr::GetScreenSelectFX(eScreenID screenId, char *pBuf, int nBufLen)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,"SelectFX"))
	{
		GetString(pTag, "SelectFX",pBuf,nBufLen);
	}
	else
		GetString(LO_DEFAULT_TAG, "SelectFX",pBuf,nBufLen);
}

LTBOOL CLayoutMgr::GetScreenSelectFXCenter(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,"SelectFXCenter"))
	{
		return (LTBOOL)GetInt(pTag, "SelectFXCenter");
	}
	else
		return (LTBOOL)GetInt(LO_DEFAULT_TAG, "SelectFXCenter");
}



INT_CHAR* CLayoutMgr::GetScreenCharacter(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,LO_SCREEN_CHARACTER))
	{
		char szTest[128];
		GetString(pTag,LO_SCREEN_CHARACTER,szTest,sizeof(szTest));
		int i = 0;
		while (i < (int)m_CharacterArray.size())
		{
			if (stricmp(szTest,m_CharacterArray[i]->szName) == 0)
				return m_CharacterArray[i];
			i++;
		}

	}
	return LTNULL;
}


INT_CHAR* CLayoutMgr::GetScreenCustomCharacter(eScreenID screenId, char* pName)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	if (m_buteMgr.Exist(pTag,pName))
	{
		char szTest[128];
		GetString(pTag,pName,szTest,sizeof(szTest));
		int i = 0;
		while (i < (int)m_CharacterArray.size())
		{
			if (stricmp(szTest,m_CharacterArray[i]->szName) == 0)
				return m_CharacterArray[i];
			i++;
		}

	}
	return LTNULL;
}


int	CLayoutMgr::GetScreenNumAttachments(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	int nNum = 0;
	sprintf(s_aAttName, "%s%d", LO_SCREEN_ATTACH, nNum);
	while (m_buteMgr.Exist(pTag,s_aAttName))
	{
		nNum++;
		sprintf(s_aAttName, "%s%d", LO_SCREEN_ATTACH, nNum);
	}
	return nNum;

}


void CLayoutMgr::GetScreenAttachment(eScreenID screenId, int num, char *pBuf, int nBufLen)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	sprintf(s_aAttName, "%s%d", LO_SCREEN_ATTACH, num);
	if (m_buteMgr.Exist(pTag,s_aAttName))
	{
		GetString(pTag,s_aAttName,pBuf,nBufLen);
	}
}


uint32 CLayoutMgr::GetScreenSelectedColor(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
    LTVector vColor;
	if (screenId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_SELECTED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_SELECTED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_SELECTED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CLayoutMgr::GetScreenNonSelectedColor(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
    LTVector vColor;
	if (screenId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_NONSELECTED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_NONSELECTED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_NONSELECTED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CLayoutMgr::GetScreenDisabledColor(eScreenID screenId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
    LTVector vColor;
	if (screenId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_DISABLED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_DISABLED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_TAG, LO_DISABLED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}


// Custom Screen values
LTBOOL CLayoutMgr::HasCustomValue(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);

	return m_buteMgr.Exist(pTag, pAttribute);
}

LTIntPt  CLayoutMgr::GetScreenCustomPoint(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	return GetPoint(pTag, pAttribute);
}

LTRect   CLayoutMgr::GetScreenCustomRect(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	return GetRect(pTag, pAttribute);
}

int		CLayoutMgr::GetScreenCustomInt(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	return GetInt(pTag, pAttribute);
}

LTFLOAT CLayoutMgr::GetScreenCustomFloat(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	return GetFloat(pTag, pAttribute);
}

void	CLayoutMgr::GetScreenCustomString(eScreenID screenId, char *pAttribute, char *pBuf, int nBufLen)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	GetString(pTag, pAttribute, pBuf, nBufLen);
}

LTVector CLayoutMgr::GetScreenCustomVector(eScreenID screenId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetScreenMgr()->GetScreenName(screenId);
	return GetVector(pTag, pAttribute);
}

INT_LIGHT* CLayoutMgr::GetLight(const char*szLight)
{
	LightArray::iterator iter = m_LightArray.begin();
	while (iter != m_LightArray.end())
	{
		if (stricmp(szLight,(*iter)->szName) == 0)
			return (*iter);
		iter++;
	}
	return LTNULL;
}

INT_FX* CLayoutMgr::GetFX(const char*szFX)
{
	FXArray::iterator iter = m_FXArray.begin();
	while (iter != m_FXArray.end())
	{
		if (stricmp(szFX,(*iter)->szName) == 0)
			return (*iter);
		iter++;
	}
	return LTNULL;
}

INT_CHAINFX* CLayoutMgr::GetChainFX(unsigned int iChainNum)
{
	if (iChainNum < m_ChainFXArray.size())
	{
		return m_ChainFXArray[iChainNum];
	}
	else
	{
		return LTNULL;
	}
}

// ------------------------------------------------------------------------//
//
//	Fonts
//
// ------------------------------------------------------------------------//

void CLayoutMgr::GetFontName(int nFont, char* pszFontFile, int nFontFileBufLen, 
						char* pszFontFace, int nFontFaceBufLen )
{
	sprintf(s_aAttName, "FontFile%d", nFont);
	GetString("Fonts", s_aAttName, pszFontFile, nFontFileBufLen);
	sprintf(s_aAttName, "FontFace%d", nFont);
	GetString("Fonts", s_aAttName, pszFontFace, nFontFaceBufLen);
}

uint8 CLayoutMgr::GetFontSize(int nFont)
{
	sprintf(s_aAttName, "FontSize%d", nFont);
	return (uint8)GetInt("Fonts", s_aAttName);
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

LTIntPt CLayoutMgr::GetAmmoClipUnitSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_CLIPSZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_AMMO_CLIPSZ);
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

uint8 CLayoutMgr::GetAmmoIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_ICONSZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_AMMO_ICONSZ);
}

uint32 CLayoutMgr::GetAmmoColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_AMMO_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AMMO_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_AMMO_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

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

uint8 CLayoutMgr::GetHealthIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_ICONSZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_HEALTH_ICONSZ);
}

uint32 CLayoutMgr::GetHealthColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_HEALTH_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_HEALTH_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CLayoutMgr::GetArmorColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_HEALTH_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ARMOR_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_ARMOR_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

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

uint32 CLayoutMgr::GetAirColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_AIR_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_AIR_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint8 CLayoutMgr::GetAirIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_AIR_ICONSZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_AIR_ICONSZ);
}



int CLayoutMgr::GetBarHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_BAR_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_BAR_HEIGHT);
}

uint8 CLayoutMgr::GetTextHeight(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_TEXT_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_TEXT_HEIGHT);
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

uint16 CLayoutMgr::GetDamageIconSize( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DAMAGE_ICON_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_DAMAGE_ICON_SZ);
}


LTIntPt CLayoutMgr::GetCompassPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_COMPASS_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_COMPASS_POS);
}

uint16 CLayoutMgr::GetCompassSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_COMPASS_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_COMPASS_SZ);
}

uint16 CLayoutMgr::GetRadarObjectSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_OBJSZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_RADAR_OBJSZ);
}

uint32 CLayoutMgr::GetRadarLivePlayerColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_LIVE))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTVector vColor;
	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_RADAR_LIVE);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CLayoutMgr::GetRadarDeadPlayerColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_DEAD))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTVector vColor;
	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_RADAR_DEAD);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CLayoutMgr::GetRadarTalkPlayerColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_TALK))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTVector vColor;
	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_RADAR_TALK);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

float CLayoutMgr::GetRadarFlashTime(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_FLASH))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_RADAR_FLASH);
}

uint32 CLayoutMgr::GetRadarMaxShowDist(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if( !m_buteMgr.Exist(s_aTagName, LO_HUD_RADAR_MAXDIST))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint32)GetInt(s_aTagName, LO_HUD_RADAR_MAXDIST);
}


uint16 CLayoutMgr::GetDamageSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DAMAGE_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_DAMAGE_SZ);
}

LTIntPt CLayoutMgr::GetCarryIconPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CARRY_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_CARRY_POS);
}

uint16 CLayoutMgr::GetCarryIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CARRY_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_CARRY_SZ);
}


LTIntPt CLayoutMgr::GetObjectiveIconPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_OBJ_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_OBJ_POS);
}

LTIntPt CLayoutMgr::GetObjectiveIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_OBJ_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_OBJ_SZ);
}

float CLayoutMgr::GetObjectiveBlinkDuration(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_OBJ_BLINKDUR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_OBJ_BLINKDUR);
}

float CLayoutMgr::GetObjectiveBlinkSpeed(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_OBJ_BLINKSPD))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_OBJ_BLINKSPD);
}


LTIntPt CLayoutMgr::GetHideIconPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HIDE_POS);
}

LTIntPt CLayoutMgr::GetHideIconSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HIDE_SZ);
}


LTFLOAT CLayoutMgr::GetHideIconBlinkSpeed(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_SPEED))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_HIDE_SPEED);
}

LTFLOAT CLayoutMgr::GetHideIconAlpha(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetFloat(s_aTagName, LO_HUD_HIDE_ALPHA);
}

int CLayoutMgr::GetHidingBarBasePosY( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_BASEPOSY))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_HIDE_BASEPOSY);
}

LTIntPt CLayoutMgr::GetHidingBarOffset( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_OFFSET))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_HIDE_OFFSET);
}

int CLayoutMgr::GetHidingBarHeight( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_HIDE_HEIGHT);
}

float CLayoutMgr::GetHidingBarScale( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_SCALE))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (float)GetFloat(s_aTagName, LO_HUD_HIDE_SCALE);
}

void CLayoutMgr::GetHidingBarTexture( int nLayout, char *pBuf, int nBufLen )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HIDE_TEX))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	GetString( s_aTagName, LO_HUD_HIDE_TEX, pBuf, nBufLen );
}


LTIntPt CLayoutMgr::GetDistanceIconPos(int nLayout)
{
	sprintf( s_aTagName, "%s%d", LO_HUD_TAG, nLayout );
	if( !m_buteMgr.Exist( s_aTagName, LO_HUD_DIST_POS ))
		sprintf( s_aTagName, "%s0", LO_HUD_TAG );

	return GetPoint( s_aTagName, LO_HUD_DIST_POS );
}

LTFLOAT CLayoutMgr::GetDistanceIconBlinkSpeed( int nLayout )
{
	sprintf( s_aTagName, "%s%d", LO_HUD_TAG, nLayout );
	if( !m_buteMgr.Exist( s_aTagName, LO_HUD_DIST_SPEED ))
		sprintf( s_aTagName, "%s0", LO_HUD_TAG );

	return GetFloat( s_aTagName, LO_HUD_DIST_SPEED );
}

LTFLOAT CLayoutMgr::GetDistanceIconAlpha( int nLayout )
{
	sprintf( s_aTagName, "%s%d", LO_HUD_TAG, nLayout );
	if( !m_buteMgr.Exist( s_aTagName, LO_HUD_DIST_ALPHA ))
		sprintf( s_aTagName, "%s0", LO_HUD_TAG );

	return GetFloat( s_aTagName, LO_HUD_DIST_ALPHA );
}

LTFLOAT CLayoutMgr::GetDistanceIconFadeOutSpeed( int nLayout )
{
	sprintf( s_aTagName, "%s%d", LO_HUD_TAG, nLayout );
	if( !m_buteMgr.Exist( s_aTagName, LO_HUD_DIST_FADESPEED ))
		sprintf( s_aTagName, "%s0", LO_HUD_TAG );

	return GetFloat( s_aTagName, LO_HUD_DIST_FADESPEED );
}

LTIntPt CLayoutMgr::GetActivationTextPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_ACT_POS);
}

uint32 CLayoutMgr::GetActivationTextColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_ACT_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_ACT_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint8 CLayoutMgr::GetActivationTextJustify(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_JUST))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_ACT_JUST);
}

uint8 CLayoutMgr::GetActivationTextSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_ACT_SZ);
}


uint32 CLayoutMgr::GetActivationTextDisabledColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_DIS_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_ACT_DIS_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_ACT_DIS_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_ACT_DIS_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}


LTIntPt CLayoutMgr::GetDebugTextPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_DBG_POS);
}

uint32 CLayoutMgr::GetDebugTextColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_DBG_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_DBG_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint8 CLayoutMgr::GetDebugTextJustify(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_JUST))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_DBG_JUST);
}

uint8 CLayoutMgr::GetDebugTextSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_DBG_SZ);
}

uint16 CLayoutMgr::GetDebugTextWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DBG_WD))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_DBG_WD);
}


uint8 CLayoutMgr::GetChooserIconHeight(int nLayout, uint8 nDefault)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_HT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);
	
	return (uint8)GetInt(s_aTagName, LO_HUD_CHS_HT,nDefault);

}

float CLayoutMgr::GetChooserTextureScale(int nLayout, float fDefault)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_TEX))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);
	
	return GetFloat(s_aTagName, LO_HUD_CHS_TEX,fDefault);

}

uint32 CLayoutMgr::GetChooserTextColor(int nLayout, uint32 nDefault)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	uint8 nA,nR,nG,nB;
	GET_ARGB(nDefault, nA, nR, nG, nB);


	float fAlpha = GetFloat(s_aTagName, LO_HUD_CHS_ALPHA, (float)nA / 255.0f );
	nA = (uint8)(255.0f * fAlpha);


    LTVector vColor(  (float)nR,(float)nG,(float)nB);
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	if (m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_COLOR))
		vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_CHS_COLOR);

	nR = (uint8)vColor.x;
	nG = (uint8)vColor.y;
	nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint8 CLayoutMgr::GetChooserTextSize(int nLayout, uint8 nDefault)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_CHS_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);
	
	return (uint8)GetInt(s_aTagName, LO_HUD_CHS_SZ,nDefault);

}

int CLayoutMgr::GetProgressBarBasePosY( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_PROG_BASEPOSY))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_PROG_BASEPOSY);
}

LTIntPt CLayoutMgr::GetProgressBarOffset( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_PROG_OFFSET))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_PROG_OFFSET);
}

int CLayoutMgr::GetProgressBarHeight( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_PROG_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_PROG_HEIGHT);
}

float CLayoutMgr::GetProgressBarScale( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_PROG_SCALE))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (float)GetFloat(s_aTagName, LO_HUD_PROG_SCALE);
}

void CLayoutMgr::GetProgressBarTexture( int nLayout, char *pBuf, int nBufLen )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_PROG_TEX))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	GetString( s_aTagName, LO_HUD_PROG_TEX, pBuf, nBufLen );
}

int CLayoutMgr::GetDisplayMeterBasePosY( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DISPLAYMETER_BASEPOSY))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_DISPLAYMETER_BASEPOSY);
}

LTIntPt CLayoutMgr::GetDisplayMeterOffset( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DISPLAYMETER_OFFSET))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_DISPLAYMETER_OFFSET);
}

int CLayoutMgr::GetDisplayMeterHeight( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DISPLAYMETER_HEIGHT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_DISPLAYMETER_HEIGHT);
}

float CLayoutMgr::GetDisplayMeterScale( int nLayout )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DISPLAYMETER_SCALE))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (float)GetFloat(s_aTagName, LO_HUD_DISPLAYMETER_SCALE);
}

void CLayoutMgr::GetDisplayMeterTexture( int nLayout, char *pBuf, int nBufLen )
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_DISPLAYMETER_TEX))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	GetString( s_aTagName, LO_HUD_DISPLAYMETER_TEX, pBuf, nBufLen );
}

// ------------------------------------------------------------------------//
//
//	Miscellaneous Layout
//
// ------------------------------------------------------------------------//

LTVector CLayoutMgr::GetSpyVisionModelColor()
{
	CAVector vRet(0.0,0.0,0.0);

	return m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_SV_MODEL, vRet);
}

LTVector CLayoutMgr::GetSpyVisionLightScale()
{
	CAVector vDef(0.0,0.0,0.0);

    LTVector vRet = m_buteMgr.GetVector(LO_MISC_TAG, LO_MISC_SV_LIGHT, vDef);
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
	if (eMask == OVM_NONE) return LTFALSE;
	sprintf(s_aAttName,"%s%d",LO_MASK_SPRITE,(uint8)eMask);
	return m_buteMgr.Exist(LO_MASK_TAG, s_aAttName);
}


void CLayoutMgr::GetMaskSprite(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	if (eMask == OVM_NONE)
	{
		pBuf[0] = LTNULL;
		return;
	}
	sprintf(s_aAttName,"%s%d",LO_MASK_SPRITE,(uint8)eMask);

	GetString(LO_MASK_TAG, s_aAttName, pBuf, nBufLen);
}


void CLayoutMgr::GetMaskModel(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	if (eMask == OVM_NONE)
	{
		pBuf[0] = LTNULL;
		return;
	}
	sprintf(s_aAttName,"%s%d",LO_MASK_MODEL,(uint8)eMask);

	GetString(LO_MASK_TAG, s_aAttName, pBuf, nBufLen);
}


void CLayoutMgr::GetMaskSkin(eOverlayMask eMask, char *pBuf, int nBufLen)
{
	if (eMask == OVM_NONE)
	{
		pBuf[0] = LTNULL;
		return;
	}
	sprintf(s_aAttName,"%s%d",LO_MASK_SKIN,(uint8)eMask);

	GetString(LO_MASK_TAG, s_aAttName, pBuf, nBufLen);

}


LTFLOAT CLayoutMgr::GetMaskScale(eOverlayMask eMask)
{
	if (eMask == OVM_NONE)
	{
		return 1.0f;
	}
	sprintf(s_aAttName,"%s%d",LO_MASK_SCALE,(uint8)eMask);

    return (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, s_aAttName, 1.0f);
}


LTFLOAT CLayoutMgr::GetMaskAlpha(eOverlayMask eMask)
{
	if (eMask == OVM_NONE)
	{
		return 1.0f;
	}
	sprintf(s_aAttName,"%s%d",LO_MASK_ALPHA,(uint8)eMask);

    return (LTFLOAT)m_buteMgr.GetDouble(LO_MASK_TAG, s_aAttName, 1.0f);
}


void CLayoutMgr::GetDialogFrame(char *pBuf, int nBufLen)
{
	GetString(LO_MISC_TAG, LO_MISC_DLG_FRAME, pBuf, nBufLen);
}

uint8 CLayoutMgr::GetDialogFontFace()
{
	return (uint8)GetInt(LO_MISC_TAG, LO_MISC_DLG_FONT_FACE);
}

uint8 CLayoutMgr::GetDialogFontSize()
{
	return (uint8)GetInt(LO_MISC_TAG, LO_MISC_DLG_FONT_SIZE);
}

LTFLOAT  CLayoutMgr::GetDeathDelay()
{
    return (LTFLOAT)m_buteMgr.GetDouble(LO_MISC_TAG, LO_MISC_DEATHDELAY, 0.0f);
}

LTFLOAT CLayoutMgr::GetMessageMinFade()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_FADE);
}

LTFLOAT CLayoutMgr::GetMessageMinTime()
{
	return GetFloat(LO_MISC_TAG, LO_MISC_MSG_TIME);
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


//-------------------------------------------------------------------------
//
// Credits
//
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



void CLayoutMgr::GetMenuFrame(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_FRAME, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuFrameTip(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_FRAME_TIP, pBuf, nBufLen);
}
/*
void CLayoutMgr::GetMenuLeftArrow(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_L, pBuf, nBufLen);
}
void CLayoutMgr::GetMenuLeftArrowHighlight(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_L_H, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuRightArrow(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_R, pBuf, nBufLen);
}
void CLayoutMgr::GetMenuRightArrowHighlight(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_R_H, pBuf, nBufLen);
}
*/
void CLayoutMgr::GetMenuUpArrow(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_U, pBuf, nBufLen);
}
void CLayoutMgr::GetMenuUpArrowHighlight(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_U_H, pBuf, nBufLen);
}

void CLayoutMgr::GetMenuDownArrow(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_D, pBuf, nBufLen);
}
void CLayoutMgr::GetMenuDownArrowHighlight(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_ARROW_D_H, pBuf, nBufLen);
}

LTIntPt CLayoutMgr::GetMenuSize()
{
	return GetPoint(LO_MENU_TAG, LO_MENU_SIZE);
}

uint16 CLayoutMgr::GetMenuPosition()
{
	return (uint16)GetInt(LO_MENU_TAG, LO_MENU_POS);
}

float CLayoutMgr::GetMenuSlideInTime()
{
	return GetFloat(LO_MENU_TAG, LO_MENU_IN_TIME);
}

float CLayoutMgr::GetMenuSlideOutTime()
{
	return GetFloat(LO_MENU_TAG, LO_MENU_OUT_TIME);
}

/*
void CLayoutMgr::GetMenuClose(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_CLOSE, pBuf, nBufLen);
}
void CLayoutMgr::GetMenuCloseHighlight(char *pBuf, int nBufLen)
{
	GetString(LO_MENU_TAG, LO_MENU_CLOSE_H, pBuf, nBufLen);
}
*/
uint8 CLayoutMgr::GetMenuFontFace(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_MENU_FONT))
	{
		return (uint8)GetInt(pTag, LO_MENU_FONT);
	}
	else
		return (uint8)GetInt(LO_DEFAULT_MENU_TAG, LO_MENU_FONT);
	
}

uint8 CLayoutMgr::GetMenuFontSize(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_MENU_FONT_SIZE))
	{
		return (uint8)GetInt(pTag, LO_MENU_FONT_SIZE);
	}
	else
		return (uint8)GetInt(LO_DEFAULT_MENU_TAG, LO_MENU_FONT_SIZE);

}

uint8 CLayoutMgr::GetMenuTitleFontFace(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_MENU_TITLE_FONT))
	{
		return (uint8)GetInt(pTag, LO_MENU_TITLE_FONT);
	}
	else
		return (uint8)GetInt(LO_DEFAULT_MENU_TAG, LO_MENU_TITLE_FONT);
}

uint8 CLayoutMgr::GetMenuTitleFontSize(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_MENU_TITLE_FONT_SIZE))
	{
		return (uint8)GetInt(pTag, LO_MENU_TITLE_FONT_SIZE);
	}
	else
		return (uint8)GetInt(LO_DEFAULT_MENU_TAG, LO_MENU_TITLE_FONT_SIZE);
}



LTIntPt  CLayoutMgr::GetMenuIndent(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_MENU_INDENT))
	{
		return GetPoint(pTag, LO_MENU_INDENT);
	}
	else
		return GetPoint(LO_DEFAULT_MENU_TAG, LO_MENU_INDENT);
}

uint32 CLayoutMgr::GetMenuSelectedColor(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
    LTVector vColor;
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_SELECTED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_SELECTED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_MENU_TAG, LO_SELECTED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CLayoutMgr::GetMenuNonSelectedColor(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
    LTVector vColor;
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_NONSELECTED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_NONSELECTED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_MENU_TAG, LO_NONSELECTED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CLayoutMgr::GetMenuDisabledColor(eMenuID menuId)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
    LTVector vColor;
	if (menuId != MENU_ID_NONE && m_buteMgr.Exist(pTag,LO_DISABLED_COLOR))
	{
		vColor = m_buteMgr.GetVector(pTag, LO_DISABLED_COLOR);
	}
	else
		vColor = m_buteMgr.GetVector(LO_DEFAULT_MENU_TAG, LO_DISABLED_COLOR);

	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

// Custom Menu values
LTBOOL CLayoutMgr::MenuHasCustomValue(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);

	return m_buteMgr.Exist(pTag, pAttribute);
}

LTIntPt  CLayoutMgr::GetMenuCustomPoint(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	return GetPoint(pTag, pAttribute);
}

LTRect   CLayoutMgr::GetMenuCustomRect(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	return GetRect(pTag, pAttribute);
}

int		CLayoutMgr::GetMenuCustomInt(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	return GetInt(pTag, pAttribute);
}

LTFLOAT CLayoutMgr::GetMenuCustomFloat(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	return GetFloat(pTag, pAttribute);
}

void	CLayoutMgr::GetMenuCustomString(eMenuID menuId, char *pAttribute, char *pBuf, int nBufLen)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	GetString(pTag, pAttribute, pBuf, nBufLen);
}

LTVector CLayoutMgr::GetMenuCustomVector(eMenuID menuId, char *pAttribute)
{
	char* pTag = (char*)g_pInterfaceMgr->GetMenuMgr()->GetMenuName(menuId);
	return GetVector(pTag, pAttribute);
}


// ------------------------------------------------------------------------//
//
//	Private Helper functions
//
// ------------------------------------------------------------------------//
LTBOOL CLayoutMgr::Exist(char *pTag)
{
	return m_buteMgr.Exist(pTag);
}

LTBOOL CLayoutMgr::HasValue(char *pTag,char *pAttribute)
{
	return m_buteMgr.Exist(pTag,pAttribute);
}

LTBOOL CLayoutMgr::GetBool(char *pTag,char *pAttribute, LTBOOL bDefault)
{
    return (LTBOOL) m_buteMgr.GetInt(pTag,pAttribute, bDefault);
}

LTFLOAT CLayoutMgr::GetFloat(char *pTag,char *pAttribute, float fDefault)
{
    return (LTFLOAT)m_buteMgr.GetDouble(pTag, pAttribute, fDefault);
}

int	CLayoutMgr::GetInt(char *pTag,char *pAttribute, int nDefault)
{
	return m_buteMgr.GetInt(pTag, pAttribute, nDefault);
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

	m_buteMgr.GetString(pTag, pAttribute, "", pBuf, nBufLen);
	
}

LTVector CLayoutMgr::GetVector(char *pTag,char *pAttribute)
{
	CAVector vRet(0.0,0.0,0.0);
	return m_buteMgr.GetVector(pTag, pAttribute, vRet);
}