// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostOptions.h
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_HOST_OPTIONS_H_
#define _SCREEN_HOST_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "NetDefs.h"

class FloatSliderCtrl;
class GameRuleFloatSliderCtrl;
class GameRuleBoolCtrl;
class TeamReflectDamageCtrl;
class GameRuleUint32SliderCtrl;
class GameRuleEnumCtrl;

class CScreenHostOptions : public CBaseScreen
{
public:
	CScreenHostOptions();
	virtual ~CScreenHostOptions();

	// Build the screen
    bool   Build();

    void    OnFocus(bool bFocus);

protected:

	uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:

	CLTGUIScrollBar* m_pScroll;
	CLTGUIListCtrlEx* m_pList;
	GameRuleFloatSliderCtrl* m_pRunSpeed;
	GameRuleBoolCtrl* m_pFriendlyFire;
	GameRuleBoolCtrl* m_pWeaponsStay;
	GameRuleBoolCtrl* m_pUseLoadout;
	GameRuleBoolCtrl* m_pUseWeaponRestrictions;
	TeamReflectDamageCtrl* m_pTeamReflectDamage;
	GameRuleFloatSliderCtrl* m_pTeamDamagePercent;
	GameRuleUint32SliderCtrl* m_pScoreLimit;
	GameRuleUint32SliderCtrl* m_pTimeLimit;
	GameRuleUint32SliderCtrl* m_pSuddenDeathTimeLimit;
	GameRuleUint32SliderCtrl* m_pJoinGracePeriod;
	GameRuleUint32SliderCtrl* m_pMaxWeapons;
	GameRuleUint32SliderCtrl* m_pNumRounds;
	GameRuleUint32SliderCtrl* m_pFragScorePlayer;
	GameRuleUint32SliderCtrl* m_pFragScoreTeam;
	GameRuleUint32SliderCtrl* m_pMaxPlayers;
	GameRuleBoolCtrl* m_pUseTeams;
	GameRuleBoolCtrl* m_pSwitchTeamsBetweenRounds;
	GameRuleUint32SliderCtrl* m_pRespawnWaitTime;
	GameRuleUint32SliderCtrl* m_pTeamKillRespawnPenalty;
	GameRuleBoolCtrl *m_pAccumulateRespawnPenalty;
	GameRuleEnumCtrl *m_pSpawnSelection;
	GameRuleBoolCtrl *m_pUseSlowMo;
	GameRuleBoolCtrl *m_pSlowMoPersistsAcrossDeath;
	GameRuleBoolCtrl *m_pSlowMoRespawnAfterUse;
	GameRuleBoolCtrl *m_pSlowMoNavMarker;
	GameRuleUint32SliderCtrl* m_pSlowMoHoldScorePeriod;
	GameRuleUint32SliderCtrl* m_pSlowMoHoldScorePlayer;
	GameRuleUint32SliderCtrl* m_pSlowMoHoldScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagBaseScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagBaseScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagBaseRadius;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagCarrierScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagCarrierScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFDefendFlagCarrierRadius;
	GameRuleUint32SliderCtrl* m_pCTFKillFlagCarrierScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFKillFlagCarrierScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFTeamKillFlagCarrierPenalty;
	GameRuleUint32SliderCtrl* m_pCTFReturnFlagScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFReturnFlagScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFStealFlagScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFStealFlagScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFPickupFlagScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFPickupFlagScoreTeam;
	GameRuleUint32SliderCtrl* m_pCTFCaptureFlagScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFCaptureFlagScoreTeam;
	GameRuleFloatSliderCtrl* m_pCTFCaptureAssistTimeout;
	GameRuleUint32SliderCtrl* m_pCTFCaptureAssistScorePlayer;
	GameRuleUint32SliderCtrl* m_pCTFCaptureAssistScoreTeam;
	GameRuleFloatSliderCtrl* m_pCTFFlagLooseTimeout;
	GameRuleFloatSliderCtrl* m_pCTFFlagMovementLimit;

	GameRuleFloatSliderCtrl* m_pCPCapturingTime;
	GameRuleFloatSliderCtrl* m_pCPGroupCaptureFactor;
	GameRuleUint32SliderCtrl* m_pCPDefendScorePlayer;
	GameRuleUint32SliderCtrl* m_pCPDefendScoreTeam;
	GameRuleUint32SliderCtrl* m_pCPDefendRadius;
	GameRuleUint32SliderCtrl* m_pCPOwnedScoreAmountTeam;
	GameRuleFloatSliderCtrl* m_pCPOwnedScorePeriod;
	GameRuleUint32SliderCtrl* m_pCPScoreLoseTeam;
	GameRuleUint32SliderCtrl* m_pCPScoreNeutralizeTeam;
	GameRuleUint32SliderCtrl* m_pCPScoreCaptureTeam;
	GameRuleUint32SliderCtrl* m_pCPScoreNeutralizePlayer;
	GameRuleUint32SliderCtrl* m_pCPScoreCapturePlayer;

	GameRuleEnumCtrl *m_pTeamSizeBalancing;
	GameRuleEnumCtrl *m_pTeamScoreBalancing;
	GameRuleFloatSliderCtrl* m_pTeamScoreBalancingPercent;

	GameRuleFloatSliderCtrl* m_pEndRoundMessageTime;
	GameRuleFloatSliderCtrl* m_pEndRoundScoreScreenTime;

	GameRuleBoolCtrl *m_pAllowKillerTrackSpectating;
};

#endif // _SCREEN_HOST_OPTIONS_H_