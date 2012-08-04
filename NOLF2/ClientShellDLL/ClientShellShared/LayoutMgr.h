// ----------------------------------------------------------------------- //
//
// MODULE  : LayoutMgr.h
//
// PURPOSE : Attribute file manager for interface layout info
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LAYOUTMGR_H_)
#define _LAYOUTMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ScreenMgr.h"
#include "MenuMgr.h"
#include "Overlays.h"


#define LO_DEFAULT_FILE "Attributes\\Layout.txt"

class CLayoutMgr;
extern CLayoutMgr* g_pLayoutMgr;

struct INT_CHAR
{
	INT_CHAR()	{szName[0] = szModel[0] = 0; vPos.Init(); fScale = 1.0f; fRot = 0.0f; nMenuLayer = 0; }

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];

	char	szModel[128];
	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;

    LTVector vPos;
    LTFLOAT	 fScale;
	LTFLOAT	 fRot;
	uint8		nMenuLayer;
};

struct INT_LIGHT
{
	INT_LIGHT()	{szName[0] = 0; vPos.Init(); vColor.Init();	fRadius = 0.0f; }

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];

    LTVector vPos;
    LTVector vColor;
	LTFLOAT	 fRadius;

};

struct INT_FX
{
	INT_FX() {szName[0] = szFXName[0] = 0; bLoop = LTFALSE; vPos.Init();}

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char		szName[128];
	char		szFXName[128];
    LTVector	vPos;
	LTBOOL		bLoop;

};

struct INT_CHAINFX
{
	INT_CHAINFX() {szIntroName[0] = szShortIntroName[0] = szLoopName[0] = 0; iFromScreen = iToScreen = -1;}

	bool	Init(CButeMgr & buteMgr, char* aTagName, int iNum);

	int		iFromScreen;
	int		iToScreen;

	char szIntroName[128];
	char szShortIntroName[128];
	char szLoopName[128];
};


class CLayoutMgr : public CGameButeMgr
{
public:
	CLayoutMgr();
	virtual ~CLayoutMgr();

    virtual LTBOOL      Init(const char* szAttributeFile=LO_DEFAULT_FILE);
	void		Term();

    LTBOOL      WriteFile() { return m_buteMgr.Save(); }
	void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

    LTRect      GetHelpRect();
    uint8		GetHelpFont();
    uint8		GetHelpSize();
    uint8		GetHUDFont();
    uint8		GetBackFont();
    uint8		GetBackSize();
    LTIntPt     GetBackPos();
    LTIntPt     GetNextPos();
    HLTCOLOR    GetBackColor();
	void		GetBackSprite(char *pBuf, int nBufLen);
    LTFLOAT     GetBackSpriteScale();
	void		GetSliderTex(char *pBuf, int nBufLen);
	void		GetArrowBackTex(char *pBuf, int nBufLen);
	void		GetArrowBackTexH(char *pBuf, int nBufLen);
    LTIntPt     GetArrowBackPos();
	void		GetArrowNextTex(char *pBuf, int nBufLen);
	void		GetArrowNextTexH(char *pBuf, int nBufLen);
    LTIntPt     GetArrowNextPos();
    LTRect      GetBossRect();

    LTIntPt     GetScreenTitlePos(eScreenID screenId);
	uint8		GetScreenTitleFont(eScreenID screenId);
	uint8		GetScreenTitleSize(eScreenID screenId);
	uint8		GetScreenFontFace(eScreenID screenId);
	uint8		GetScreenFontSize(eScreenID screenId);
    LTRect      GetScreenPageRect(eScreenID screenId);
	int			GetScreenItemSpacing(eScreenID screenId);
	int			GetScreenItemAlign(eScreenID screenId);
	int			GetScreenMusicIntensity(eScreenID screenId);
	void		GetScreenMouseFX(eScreenID screenId, char *pBuf, int nBufLen);
	void		GetScreenSelectFX(eScreenID screenId, char *pBuf, int nBufLen);
	LTBOOL		GetScreenSelectFXCenter(eScreenID screenId);

	INT_CHAR   *GetScreenCharacter(eScreenID screenId);
	INT_CHAR   *GetScreenCustomCharacter(eScreenID screenId, char* pName);
	int			GetScreenNumAttachments(eScreenID screenId);
	void		GetScreenAttachment(eScreenID screenId, int num, char *pBuf, int nBufLen);

	uint32		GetScreenSelectedColor(eScreenID screenId);
	uint32		GetScreenNonSelectedColor(eScreenID screenId);
	uint32		GetScreenDisabledColor(eScreenID screenId);

    LTBOOL      HasCustomValue(eScreenID screenId, char *pAttribute);
    LTIntPt     GetScreenCustomPoint(eScreenID screenId, char *pAttribute);
    LTRect      GetScreenCustomRect(eScreenID screenId, char *pAttribute);
	int			GetScreenCustomInt(eScreenID screenId, char *pAttribute);
    LTFLOAT     GetScreenCustomFloat(eScreenID screenId, char *pAttribute);
	void		GetScreenCustomString(eScreenID screenId, char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetScreenCustomVector(eScreenID screenId, char *pAttribute);

	INT_LIGHT	*GetLight(const char*szLight);
	INT_FX		*GetFX(const char*szFX);
	INT_CHAINFX	*GetChainFX(unsigned int iChainNum);

	int			GetNumHUDLayouts()			{return m_nNumHUDLayouts;}
	int			GetLayoutName(int nLayout);

	int			GetNumFonts()			{return m_nNumFonts;}
	void		GetFontName(int nFont, char* pszFontFile, int nFontFileBufLen, 
						char* pszFontFace, int nFontFaceBufLen );
	uint8		GetFontSize(int nFont);


    LTBOOL      GetUseAmmoBar(int nLayout);
    LTIntPt     GetAmmoBasePos(int nLayout);
    LTIntPt     GetAmmoClipOffset(int nLayout);
    LTIntPt     GetAmmoClipUnitSize(int nLayout);
    LTIntPt     GetAmmoBarOffset(int nLayout);
    LTBOOL      GetUseAmmoText(int nLayout);
    LTIntPt     GetAmmoTextOffset(int nLayout);
    LTIntPt     GetAmmoIconOffset(int nLayout);
    uint8		GetAmmoIconSize(int nLayout);
	uint32		GetAmmoColor(int nLayout);

    LTBOOL      GetUseHealthBar(int nLayout);
    LTIntPt     GetHealthBasePos(int nLayout);
    LTIntPt     GetHealthBarOffset(int nLayout);
    LTIntPt     GetArmorBarOffset(int nLayout);
    LTBOOL      GetUseHealthText(int nLayout);
    LTIntPt     GetHealthTextOffset(int nLayout);
    LTIntPt     GetArmorTextOffset(int nLayout);
    LTBOOL      GetUseHealthIcon(int nLayout);
    LTIntPt     GetHealthIconOffset(int nLayout);
    LTIntPt     GetArmorIconOffset(int nLayout);
    uint8		GetHealthIconSize(int nLayout);
	uint32		GetHealthColor(int nLayout);
	uint32		GetArmorColor(int nLayout);

    LTBOOL      GetUseAirIcon(int nLayout);
    LTIntPt     GetAirBasePos(int nLayout);
    LTIntPt     GetAirIconOffset(int nLayout);
    uint8		GetAirIconSize(int nLayout);
    LTBOOL      GetUseAirText(int nLayout);
    LTIntPt     GetAirTextOffset(int nLayout);
    LTBOOL      GetUseAirBar(int nLayout);
    LTIntPt     GetAirBarOffset(int nLayout);
	uint32		GetAirColor(int nLayout);

	int			GetBarHeight(int nLayout);
    LTFLOAT     GetBarScale(int nLayout);
	uint8		GetTextHeight(int nLayout);
    LTIntPt     GetModeTextPos(int nLayout);
    LTIntPt     GetDamageBasePos(int nLayout);
	uint16		GetDamageIconSize(int nLayout);

    LTIntPt		GetCompassPos(int nLayout);
    uint16		GetCompassSize(int nLayout);
	
	uint16		GetRadarObjectSize(int nLayout);
	uint32		GetRadarMaxShowDist(int nLayout);
	uint32		GetRadarLivePlayerColor(int nLayout);
	uint32		GetRadarDeadPlayerColor(int nLayout);
	uint32		GetRadarTalkPlayerColor(int nLayout);
	float		GetRadarFlashTime(int nLayout);

    uint16		GetDamageSize(int nLayout);

    LTIntPt		GetCarryIconPos(int nLayout);
    uint16		GetCarryIconSize(int nLayout);

    LTIntPt		GetObjectiveIconPos(int nLayout);
    LTIntPt		GetObjectiveIconSize(int nLayout);
    float		GetObjectiveBlinkDuration(int nLayout);
    float		GetObjectiveBlinkSpeed(int nLayout);

    LTIntPt		GetHideIconPos(int nLayout);
    LTIntPt		GetHideIconSize(int nLayout);
    LTFLOAT     GetHideIconBlinkSpeed(int nLayout);
    LTFLOAT     GetHideIconAlpha(int nLayout);
	
	int			GetHidingBarBasePosY(int nLayout);
	LTIntPt		GetHidingBarOffset(int nLayout);
	int			GetHidingBarHeight(int nLayout);
	float		GetHidingBarScale(int nLayout);
	void		GetHidingBarTexture(int nLayout, char *pBuf, int nBufLen);
	
	LTIntPt		GetDistanceIconPos(int nLayout);
	LTFLOAT		GetDistanceIconBlinkSpeed(int nLayout);
	LTFLOAT		GetDistanceIconAlpha(int nLayout);
	LTFLOAT		GetDistanceIconFadeOutSpeed(int nLayout);

	LTIntPt		GetActivationTextPos(int nLayout);
	uint8		GetActivationTextSize(int nLayout);
	uint8		GetActivationTextJustify(int nLayout);
	uint32		GetActivationTextColor(int nLayout);
	uint32		GetActivationTextDisabledColor(int nLayout);
	LTIntPt		GetDebugTextPos(int nLayout);
	uint8		GetDebugTextSize(int nLayout);
	uint8		GetDebugTextJustify(int nLayout);
	uint32		GetDebugTextColor(int nLayout);
	uint16		GetDebugTextWidth(int nLayout);

	uint8		GetChooserIconHeight(int nLayout, uint8 nDefault);
	float		GetChooserTextureScale(int nLayout, float fDefault);
	uint32		GetChooserTextColor(int nLayout, uint32 nDefault);
	uint8		GetChooserTextSize(int nLayout, uint8 nDefault);

	int			GetProgressBarBasePosY(int nLayout);
	LTIntPt		GetProgressBarOffset(int nLayout);
	int			GetProgressBarHeight(int nLayout);
	float		GetProgressBarScale(int nLayout);
	void		GetProgressBarTexture(int nLayout, char *pBuf, int nBufLen);
	
	int			GetDisplayMeterBasePosY(int nLayout);
	LTIntPt		GetDisplayMeterOffset(int nLayout);
	int			GetDisplayMeterHeight(int nLayout);
	float		GetDisplayMeterScale(int nLayout);
	void		GetDisplayMeterTexture(int nLayout, char *pBuf, int nBufLen);
    
	LTVector    GetSpyVisionModelColor();
    LTVector    GetSpyVisionLightScale();
    LTVector    GetWeaponPickupColor();
    LTVector    GetAmmoPickupColor();
    LTFLOAT     GetTintTime();

    LTFLOAT     GetCrosshairGapMin();
    LTFLOAT     GetCrosshairGapMax();
    LTFLOAT     GetCrosshairBarMin();
    LTFLOAT     GetCrosshairBarMax();
    LTFLOAT     GetPerturbRotationEffect();
    LTFLOAT     GetPerturbIncreaseSpeed();
    LTFLOAT     GetPerturbDecreaseSpeed();
    LTFLOAT     GetPerturbWalkPercent();

	LTBOOL		IsMaskSprite(eOverlayMask eMask);
	void		GetMaskSprite(eOverlayMask eMask, char *pBuf, int nBufLen);
	void		GetMaskModel(eOverlayMask eMask, char *pBuf, int nBufLen);
	void		GetMaskSkin(eOverlayMask eMask, char *pBuf, int nBufLen);
    LTFLOAT     GetMaskScale(eOverlayMask eMask);
    LTFLOAT     GetMaskAlpha(eOverlayMask eMask);

	void		GetDialogFrame(char *pBuf, int nBufLen);
	uint8		GetDialogFontFace();
	uint8		GetDialogFontSize();

    LTFLOAT     GetDeathDelay();

    LTFLOAT     GetMessageMinFade();
    LTFLOAT     GetMessageMinTime();

    LTRect      GetObjectiveRect();
    LTRect      GetPopupTextRect();

	HLTCOLOR	GetSubtitleTint();
	HLTCOLOR	GetHealthTint();
	HLTCOLOR	GetArmorTint();
	HLTCOLOR	GetAmmoTint();

	LTIntPt		GetMenuSize();
	uint16		GetMenuPosition();
	float		GetMenuSlideInTime();
	float		GetMenuSlideOutTime();
	void		GetMenuFrame(char *pBuf, int nBufLen);
	void		GetMenuFrameTip(char *pBuf, int nBufLen);
	void		GetMenuUpArrow(char *pBuf, int nBufLen);
	void		GetMenuUpArrowHighlight(char *pBuf, int nBufLen);
	void		GetMenuDownArrow(char *pBuf, int nBufLen);
	void		GetMenuDownArrowHighlight(char *pBuf, int nBufLen);
//	void		GetMenuLeftArrow(char *pBuf, int nBufLen);
//	void		GetMenuLeftArrowHighlight(char *pBuf, int nBufLen);
//	void		GetMenuRightArrow(char *pBuf, int nBufLen);
//	void		GetMenuRightArrowHighlight(char *pBuf, int nBufLen);
//	void		GetMenuClose(char *pBuf, int nBufLen);
//	void		GetMenuCloseHighlight(char *pBuf, int nBufLen);

    uint8		GetMenuFontFace(eMenuID menuId);
    uint8		GetMenuFontSize(eMenuID menuId);
    uint8		GetMenuTitleFontFace(eMenuID menuId);
    uint8		GetMenuTitleFontSize(eMenuID menuId);
	LTIntPt		GetMenuIndent(eMenuID menuId);
	uint32		GetMenuSelectedColor(eMenuID menuId);
	uint32		GetMenuNonSelectedColor(eMenuID menuId);
	uint32		GetMenuDisabledColor(eMenuID menuId);

    LTBOOL      MenuHasCustomValue(eMenuID menuId, char *pAttribute);
    LTIntPt     GetMenuCustomPoint(eMenuID menuId, char *pAttribute);
    LTRect      GetMenuCustomRect(eMenuID menuId, char *pAttribute);
	int			GetMenuCustomInt(eMenuID menuId, char *pAttribute);
    LTFLOAT     GetMenuCustomFloat(eMenuID menuId, char *pAttribute);
	void		GetMenuCustomString(eMenuID menuId, char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetMenuCustomVector(eMenuID menuId, char *pAttribute);

	LTFLOAT		GetCreditsFadeInTime();
	LTFLOAT		GetCreditsHoldTime();
	LTFLOAT		GetCreditsFadeOutTime();
	LTFLOAT		GetCreditsDelayTime();
	LTIntPt		GetCreditsPositionUL();
	LTIntPt		GetCreditsPositionUR();
	LTIntPt		GetCreditsPositionLL();
	LTIntPt		GetCreditsPositionLR();
	
	LTBOOL      Exist(char *pTag);
	LTBOOL      HasValue(char *pTag,char *pAttribute);
    LTBOOL      GetBool(char *pTag,char *pAttribute, LTBOOL bDefault = LTFALSE);
    LTFLOAT     GetFloat(char *pTag,char *pAttribute, float fDefault = 0.0f);
	int			GetInt(char *pTag,char *pAttribute, int nDefault = 0);
    LTIntPt     GetPoint(char *pTag,char *pAttribute);
    LTRect      GetRect(char *pTag,char *pAttribute);
	void		GetString(char *pTag,char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetVector(char *pTag,char *pAttribute);


protected:
	int			m_nNumHUDLayouts;
	int			m_nNumFonts;

	typedef std::vector<INT_CHAR *> CharacterArray;
	CharacterArray m_CharacterArray;

	typedef std::vector<INT_LIGHT *> LightArray;
	LightArray m_LightArray;

	typedef std::vector<INT_FX *> FXArray;
	FXArray m_FXArray;

	typedef std::vector<INT_CHAINFX *> ChainFXArray;
	ChainFXArray m_ChainFXArray;
};

#endif // !defined(_LAYOUTMGR_H_)