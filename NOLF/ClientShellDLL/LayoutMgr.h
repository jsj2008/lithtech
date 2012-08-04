// LayoutMgr.h: interface for the CLayoutMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "FolderMgr.h"
#include "Overlays.h"


#define LO_DEFAULT_FILE "Attributes\\Layout.txt"

class CLayoutMgr;
extern CLayoutMgr* g_pLayoutMgr;

struct INT_CHAR
{
	INT_CHAR();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];

	char	szModel[128];
	char	szStyle[128];

    LTVector vPos;
    LTFLOAT	 fScale;
	LTFLOAT	 fRot;

};



class CLayoutMgr : public CGameButeMgr
{
public:
	CLayoutMgr();
	virtual ~CLayoutMgr();

    LTBOOL      Init(ILTCSBase *pInterface, const char* szAttributeFile=LO_DEFAULT_FILE);
	void		Term();

    LTBOOL      WriteFile() { return m_buteMgr.Save(); }
	void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

    LTRect      GetHelpRect();
    LTIntPt     GetBackPos();
    LTIntPt     GetContinuePos();
    LTIntPt     GetMainPos();
    HLTCOLOR    GetShadeColor();
    HLTCOLOR    GetBarColor();
	int			GetBarHeight();
	int			GetTopShadeHeight();
	int			GetBottomShadeHeight();
	void		GetBackSprite(char *pBuf, int nBufLen);
    LTFLOAT     GetBackSpriteScale();
	void		GetArrowBackSFX(char *pBuf, int nBufLen);
	void		GetArrowNextSFX(char *pBuf, int nBufLen);
	void		GetArrowBackBmp(char *pBuf, int nBufLen);
	void		GetArrowNextBmp(char *pBuf, int nBufLen);
    LTIntPt     GetArrowBackPos();
    LTIntPt     GetArrowNextPos();
	LTIntPt		GetLoadStringPos();
	LTIntPt		GetLoadPhotoPos();
    LTRect      GetBossRect();

    LTIntPt     GetSlotOffset(eFolderID folderId);
    LTRect      GetListRect(eFolderID folderId);
	int			GetListFontSize(eFolderID folderId);
    LTRect      GetNameRect(eFolderID folderId);
	int			GetNameFontSize(eFolderID folderId);
    LTIntPt     GetPhotoPos(eFolderID folderId);
    LTRect      GetDescriptionRect(eFolderID folderId);
	int			GetDescriptionFontSize(eFolderID folderId);
    LTIntPt     GetUpArrowOffset(eFolderID folderId);
    LTIntPt     GetDownArrowOffset(eFolderID folderId);


    LTIntPt     GetFolderTitlePos(eFolderID folderId);
	int			GetFolderTitleAlign(eFolderID folderId);
	void		GetFolderBackground(eFolderID folderId, char *pBuf, int nBufLen);
    LTRect      GetFolderPageRect(eFolderID folderId);
	int			GetFolderItemSpacing(eFolderID folderId);
	int			GetFolderItemAlign(eFolderID folderId);
	int			GetFolderMusicIntensity(eFolderID folderId);
    LTIntPt     GetUpArrowPos(eFolderID folderId);
    LTIntPt     GetDownArrowPos(eFolderID folderId);

	INT_CHAR   *GetFolderCharacter(eFolderID folderId);
	int			GetFolderNumAttachments(eFolderID folderId);
	void		GetFolderAttachment(eFolderID folderId, int num, char *pBuf, int nBufLen);


    LTBOOL      HasCustomValue(eFolderID folderId, char *pAttribute);
    LTIntPt     GetFolderCustomPoint(eFolderID folderId, char *pAttribute);
    LTRect      GetFolderCustomRect(eFolderID folderId, char *pAttribute);
	int			GetFolderCustomInt(eFolderID folderId, char *pAttribute);
    LTFLOAT     GetFolderCustomFloat(eFolderID folderId, char *pAttribute);
	void		GetFolderCustomString(eFolderID folderId, char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetFolderCustomVector(eFolderID folderId, char *pAttribute);


	int			GetNumHUDLayouts()			{return m_nNumHUDLayouts;}
	int			GetLayoutName(int nLayout);

    LTBOOL      GetUseAmmoBar(int nLayout);
    LTIntPt     GetAmmoBasePos(int nLayout);
    LTIntPt     GetAmmoClipOffset(int nLayout);
    LTIntPt     GetAmmoBarOffset(int nLayout);
    LTBOOL      GetUseAmmoText(int nLayout);
    LTIntPt     GetAmmoTextOffset(int nLayout);
    LTIntPt     GetAmmoIconOffset(int nLayout);

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

    LTBOOL      GetUseAirIcon(int nLayout);
    LTIntPt     GetAirBasePos(int nLayout);
    LTIntPt     GetAirIconOffset(int nLayout);
    LTBOOL      GetUseAirText(int nLayout);
    LTIntPt     GetAirTextOffset(int nLayout);
    LTBOOL      GetUseAirBar(int nLayout);
    LTIntPt     GetAirBarOffset(int nLayout);
	int			GetBarHeight(int nLayout);
    LTFLOAT     GetBarScale(int nLayout);
	int			GetBarBorder(int nLayout);
    LTIntPt     GetModeTextPos(int nLayout);
    LTIntPt     GetDamageBasePos(int nLayout);

    LTFLOAT     GetFlashSpeed();
    LTFLOAT     GetFlashDuration();
    LTVector    GetNightVisionModelColor();
    LTVector    GetNightVisionScreenTint();
    LTVector    GetInfraredModelColor();
    LTVector    GetInfraredLightScale();
    LTVector    GetMineDetectScreenTint();
    LTVector    GetWeaponPickupColor();
    LTVector    GetAmmoPickupColor();
    LTVector    GetChooserHighlightColor();
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

	void		GetCameraActivateSprite(char *pBuf, int nBufLen);

	void		GetMessageBoxBackground(char *pBuf, int nBufLen);
    LTFLOAT     GetMessageBoxAlpha();

	void		GetFailScreenBackground(char *pBuf, int nBufLen);
    LTIntPt     GetFailStringPos();
    LTFLOAT     GetFailScreenDelay();
    LTFLOAT     GetDeathDelay();

	void		GetHelpFont(char *pBuf, int nBufLen);
	void		GetSmallFontBase(char *pBuf, int nBufLen);
	void		GetMediumFontBase(char *pBuf, int nBufLen);
	void		GetLargeFontBase(char *pBuf, int nBufLen);
	void		GetTitleFont(char *pBuf, int nBufLen);
	void		GetMsgForeFont(char *pBuf, int nBufLen);
	void		GetHUDForeFont(char *pBuf, int nBufLen);
	void		GetAirFont(char *pBuf, int nBufLen);
	void		GetChooserFont(char *pBuf, int nBufLen);

    LTFLOAT     GetMessageFade();
    LTFLOAT     GetMessageTime();
	int			GetMaxNumMessages();

    LTRect      GetObjectiveRect();
    LTRect      GetPopupTextRect();

	HLTCOLOR	GetSubtitleTint();
	HLTCOLOR	GetHealthTint();
	HLTCOLOR	GetArmorTint();
	HLTCOLOR	GetAmmoTint();
	HLTCOLOR	GetPopupTextTint();
	HLTCOLOR	GetTeam1Color();
	HLTCOLOR	GetTeam2Color();

	int			GetMissionTextWidth();
	int			GetMissionTextNumLines();
    LTFLOAT     GetMissionTextLetterDelay();
    LTFLOAT     GetMissionTextLineDelay();
    LTFLOAT     GetMissionTextLineScrollTime();
    LTFLOAT     GetMissionTextFadeDelay();
    LTFLOAT     GetMissionTextFadeTime();
    LTIntPt     GetMissionTextPos();
	void		GetMissionTextTypeSound(char *pBuf, int nBufLen);
	void		GetMissionTextScrollSound(char *pBuf, int nBufLen);

	int			GetSubtitleNumLines();
    LTFLOAT     GetSubtitleLineScrollTime();
	int			GetSubtitleCinematicWidth();
    LTIntPt     GetSubtitleCinematicPos();
	int			GetSubtitleFullScreenWidth();
    LTIntPt     GetSubtitleFullScreenPos();

	int			GetDialoguePosition();
    LTIntPt     GetDialogueSize();
    LTIntPt     GetDialogueTextOffset();
	void		GetDialogueBackground(char *pBuf, int nBufLen);
    LTFLOAT     GetDialogueAlpha();
	void		GetDialogueFrame(char *pBuf, int nBufLen);
	int			GetDialogueFont();
    LTFLOAT     GetDialogueOpenTime();
    LTFLOAT     GetDialogueCloseTime();
	void		GetDialogueOpenSound(char *pBuf, int nBufLen);
	void		GetDialogueCloseSound(char *pBuf, int nBufLen);

	int			GetDecisionPosition();
    LTIntPt     GetDecisionTextOffset();
	int			GetDecisionSpacing();
	void		GetDecisionBackground(char *pBuf, int nBufLen);
    LTFLOAT     GetDecisionAlpha();
	int			GetDecisionFont();
    LTFLOAT     GetDecisionOpenTime();
    LTFLOAT     GetDecisionCloseTime();

	int			GetMenuPosition();
    LTIntPt     GetMenuTextOffset();
	int			GetMenuSpacing();
	void		GetMenuBackground(char *pBuf, int nBufLen);
	void		GetMenuFrame(char *pBuf, int nBufLen);
    LTFLOAT     GetMenuAlpha();
    LTFLOAT     GetMenuOpenTime();
    LTFLOAT     GetMenuCloseTime();
	void		GetMenuOpenSound(char *pBuf, int nBufLen);
	void		GetMenuCloseSound(char *pBuf, int nBufLen);

	LTFLOAT		GetCreditsFadeInTime();
	LTFLOAT		GetCreditsHoldTime();
	LTFLOAT		GetCreditsFadeOutTime();
	LTFLOAT		GetCreditsDelayTime();
	LTIntPt		GetCreditsPositionUL();
	LTIntPt		GetCreditsPositionUR();
	LTIntPt		GetCreditsPositionLL();
	LTIntPt		GetCreditsPositionLR();

protected:
    LTBOOL      GetBool(char *pTag,char *pAttribute);
    LTFLOAT     GetFloat(char *pTag,char *pAttribute);
	int			GetInt(char *pTag,char *pAttribute);
    LTIntPt     GetPoint(char *pTag,char *pAttribute);
    LTRect      GetRect(char *pTag,char *pAttribute);
	void		GetString(char *pTag,char *pAttribute, char *pBuf, int nBufLen);
    LTVector    GetVector(char *pTag,char *pAttribute);


protected:
	int			m_nNumHUDLayouts;

	CMoArray<INT_CHAR *> m_CharacterArray;


};

#endif // !defined(AFX_LAYOUTMGR_H__D238C1C0_8635_11D3_B2DB_006097097C7B__INCLUDED_)