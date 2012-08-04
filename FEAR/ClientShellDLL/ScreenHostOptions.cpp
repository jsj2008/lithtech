// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostOptions.cpp
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostOptions.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "CustomCtrls.h"
#include "GameRuleCtrls.h"
#include "GameModeMgr.h"
#include "ClientConnectionMgr.h"


namespace
{
	const uint32 CMD_TEAM1 = CMD_CUSTOM+1;
	const uint32 CMD_TEAM2 = CMD_CUSTOM+2;
	uint8 g_nCurTeam = 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostOptions::CScreenHostOptions() :
	m_pScroll(NULL)
{
	m_pMaxPlayers = NULL;
	m_pList = NULL;
	m_pRunSpeed = NULL;
	m_pFriendlyFire = NULL;
	m_pWeaponsStay = NULL;
	m_pUseLoadout = NULL;
	m_pUseWeaponRestrictions = NULL;
	m_pTeamReflectDamage = NULL;
	m_pTeamDamagePercent = NULL;
	m_pScoreLimit = NULL;
	m_pTimeLimit = NULL;
	m_pSuddenDeathTimeLimit = NULL;
	m_pJoinGracePeriod = NULL;
	m_pMaxWeapons = NULL;
	m_pNumRounds = NULL;
	m_pFragScorePlayer = NULL;
	m_pFragScoreTeam = NULL;
	m_pMaxPlayers = NULL;
	m_pUseTeams = NULL;
	m_pSwitchTeamsBetweenRounds = NULL;
	m_pSpawnSelection = NULL;
	m_pRespawnWaitTime = NULL;
	m_pTeamKillRespawnPenalty = NULL;
	m_pAccumulateRespawnPenalty = NULL;
	m_pUseSlowMo = NULL;
	m_pSlowMoPersistsAcrossDeath = NULL;
	m_pSlowMoRespawnAfterUse = NULL;
	m_pCTFDefendFlagBaseScorePlayer = NULL;
	m_pCTFDefendFlagBaseScoreTeam = NULL;
	m_pCTFDefendFlagBaseRadius = NULL;
	m_pCTFDefendFlagCarrierScorePlayer = NULL;
	m_pCTFDefendFlagCarrierScoreTeam = NULL;
	m_pCTFDefendFlagCarrierRadius = NULL;
	m_pCTFKillFlagCarrierScorePlayer = NULL;
	m_pCTFKillFlagCarrierScoreTeam = NULL;
	m_pCTFTeamKillFlagCarrierPenalty = NULL;
	m_pCTFReturnFlagScorePlayer = NULL;
	m_pCTFReturnFlagScoreTeam = NULL;
	m_pCTFStealFlagScorePlayer = NULL;
	m_pCTFStealFlagScoreTeam = NULL;
	m_pCTFPickupFlagScorePlayer = NULL;
	m_pCTFPickupFlagScoreTeam = NULL;
	m_pCTFCaptureFlagScorePlayer = NULL;
	m_pCTFCaptureFlagScoreTeam = NULL;
	m_pCTFCaptureAssistTimeout = NULL;
	m_pCTFCaptureAssistScorePlayer = NULL;
	m_pCTFCaptureAssistScoreTeam = NULL;
	m_pCTFFlagLooseTimeout = NULL;
	m_pCTFFlagMovementLimit = NULL;
	m_pSlowMoHoldScorePeriod = NULL;
	m_pSlowMoHoldScorePlayer = NULL;
	m_pSlowMoHoldScoreTeam = NULL;
	m_pCPCapturingTime = NULL;
	m_pCPGroupCaptureFactor = NULL;
	m_pCPDefendScorePlayer = NULL;
	m_pCPDefendScoreTeam = NULL;
	m_pCPDefendRadius = NULL;
	m_pCPOwnedScoreAmountTeam = NULL;
	m_pCPOwnedScorePeriod = NULL;
	m_pCPScoreLoseTeam = NULL;
	m_pCPScoreNeutralizeTeam = NULL;
	m_pCPScoreCaptureTeam = NULL;
	m_pCPScoreNeutralizePlayer = NULL;
	m_pCPScoreCapturePlayer = NULL;
	m_pTeamSizeBalancing = NULL;
	m_pTeamScoreBalancing = NULL;
	m_pTeamScoreBalancingPercent = NULL;
	m_pEndRoundMessageTime = NULL;
	m_pEndRoundScoreScreenTime = NULL;
	m_pAllowKillerTrackSpectating = NULL;
}


CScreenHostOptions::~CScreenHostOptions()
{
}

// Build the screen
bool CScreenHostOptions::Build()
{
	CreateTitle("IDS_TITLE_HOST_GAMETYPE_OPTIONS");


 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHostOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_TEAM1:
		{
			g_nCurTeam = 0;
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_TEAM);
		} break;

	case CMD_TEAM2:
		{
			g_nCurTeam = 1;
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_TEAM);
		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


static void ShowGameRuleInOptions( CLTGUICtrl& ctrl, GameRule const& gameRule )
{
	ctrl.Show( gameRule.IsCanModify( ) && gameRule.IsShowInOptions( ));
	bool bCanModify = !g_pPlayerMgr->IsPlayerInWorld( ) || ( !g_pClientConnectionMgr->IsConnectedToRemoteServer( ) && gameRule.IsCanModifyAtRuntime( ));
	ctrl.Enable( bCanModify );
}


// Change in focus
void    CScreenHostOptions::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	if (bFocus)
	{
		int kColumn = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
		int kSlider = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);

		{
			CLTGUIScrollBar_create csb;
			csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
			csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
			csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();

			m_pScroll = CreateScrollBar( csb );
			if( m_pScroll )
			{
				m_pScroll->SetFrameWidth( 1 );
				m_pScroll->Enable( true );
				m_pScroll->Show( true );
			}
		}

		// Create the game options from the game rules.
		CLTGUIListCtrlEx_create listCs;
		listCs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
		listCs.pScrollBar = m_pScroll;
		listCs.nTextIdent = g_pLayoutDB->GetListIndent(m_hLayout,0).x;
		m_pList = AddListEx(listCs);
		if( m_pList )
		{
			m_pList->SetFrameWidth( 1 );
			m_pList->Enable( true );
			m_pList->Show( true );
			m_pList->SetScrollWrap( false );
			m_pList->SetIndent( LTVector2n(g_pLayoutDB->GetListFrameExpand(m_hLayout, 0), g_pLayoutDB->GetListFrameExpand(m_hLayout, 0)) );
		}

		if( m_pScroll )
			AddControl( m_pScroll );

		int32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(kColumn+kSlider,nListFontSize);

		/*
		cs.nCommandID = CMD_TEAM1;
		cs.szHelpID = "IDS_HELP_TEAM_1";
		m_pTeam1 = CreateTextItem("IDS_TEAM_1_OPTIONS", cs );
		m_pList->AddControl( m_pTeam1 );

		cs.nCommandID = CMD_TEAM2;
		cs.szHelpID = "IDS_HELP_TEAM_2";
		m_pTeam2 = CreateTextItem("IDS_TEAM_2_OPTIONS", cs );
		m_pList->AddControl( m_pTeam2 );
		*/

		FloatSliderCtrl::CreateStruct floatSliderCtrlCS;
		floatSliderCtrlCS.m_scs.rnBaseRect.m_vMin.Init();
		floatSliderCtrlCS.m_scs.rnBaseRect.m_vMax = LTVector2n( kColumn+kSlider,nListFontSize);
		floatSliderCtrlCS.m_scs.nBarOffset = kColumn;

		CLTGUIToggle_create tcs;
		tcs.rnBaseRect.m_vMin.Init();
		tcs.rnBaseRect.m_vMax = LTVector2n(kColumn+kSlider,nListFontSize);
		tcs.nHeaderWidth = kColumn;

		CLTGUISlider_create sliderCtrlCS;
		sliderCtrlCS.rnBaseRect.m_vMin.Init();
		sliderCtrlCS.rnBaseRect.m_vMax = LTVector2n( kColumn+kSlider, nListFontSize);
		sliderCtrlCS.nBarOffset = kColumn;

		GameModeMgr& gameModeMgr = GameModeMgr::Instance( );

		m_pRunSpeed = new GameRuleFloatSliderCtrl;
		m_pRunSpeed->Create( *this, gameModeMgr.m_grfRunSpeed, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pRunSpeed );

		m_pFriendlyFire = new GameRuleBoolCtrl;
		m_pFriendlyFire->Create( *this, gameModeMgr.m_grbFriendlyFire, tcs, false );
		m_pList->AddControl( m_pFriendlyFire );

		m_pWeaponsStay = new GameRuleBoolCtrl;
		m_pWeaponsStay->Create( *this, gameModeMgr.m_grbWeaponsStay, tcs, false );
		m_pList->AddControl( m_pWeaponsStay );

		m_pUseLoadout = new GameRuleBoolCtrl;
		m_pUseLoadout->Create( *this, gameModeMgr.m_grbUseLoadout, tcs, false );
		m_pList->AddControl( m_pUseLoadout );

		m_pUseWeaponRestrictions = new GameRuleBoolCtrl;
		m_pUseWeaponRestrictions->Create( *this, gameModeMgr.m_grbUseWeaponRestrictions, tcs, false );
		m_pList->AddControl( m_pUseWeaponRestrictions );

		m_pTeamReflectDamage = new TeamReflectDamageCtrl;
		m_pTeamReflectDamage->Create( *this, gameModeMgr.m_grfTeamReflectDamage, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pTeamReflectDamage );

		m_pTeamDamagePercent = new GameRuleFloatSliderCtrl;
		m_pTeamDamagePercent->Create( *this, gameModeMgr.m_grfTeamDamagePercent, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pTeamDamagePercent );

		m_pScoreLimit = new GameRuleUint32SliderCtrl;
		m_pScoreLimit->Create( *this, gameModeMgr.m_grnScoreLimit, sliderCtrlCS, false );
		m_pList->AddControl( m_pScoreLimit );

		m_pTimeLimit = new GameRuleUint32SliderCtrl;
		m_pTimeLimit->Create( *this, gameModeMgr.m_grnTimeLimit, sliderCtrlCS, false );
		m_pList->AddControl( m_pTimeLimit );

		m_pSuddenDeathTimeLimit = new GameRuleUint32SliderCtrl;
		m_pSuddenDeathTimeLimit->Create( *this, gameModeMgr.m_grnSuddenDeathTimeLimit, sliderCtrlCS, false );
		m_pList->AddControl( m_pSuddenDeathTimeLimit );

		m_pJoinGracePeriod = new GameRuleUint32SliderCtrl;
		m_pJoinGracePeriod->Create( *this, gameModeMgr.m_grnJoinGracePeriod, sliderCtrlCS, false );
		m_pList->AddControl( m_pJoinGracePeriod );

		m_pMaxWeapons = new GameRuleUint32SliderCtrl;
		m_pMaxWeapons->Create( *this, gameModeMgr.m_grnMaxWeapons, sliderCtrlCS, false );
		m_pList->AddControl( m_pMaxWeapons );

		m_pNumRounds = new GameRuleUint32SliderCtrl;
		m_pNumRounds->Create( *this, gameModeMgr.m_grnNumRounds, sliderCtrlCS, false );
		m_pList->AddControl( m_pNumRounds );

		m_pFragScorePlayer = new GameRuleUint32SliderCtrl;
		m_pFragScorePlayer->Create( *this, gameModeMgr.m_grnFragScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pFragScorePlayer );

		m_pFragScoreTeam = new GameRuleUint32SliderCtrl;
		m_pFragScoreTeam->Create( *this, gameModeMgr.m_grnFragScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pFragScoreTeam );

		m_pMaxPlayers = new GameRuleUint32SliderCtrl;
		m_pMaxPlayers->Create( *this, gameModeMgr.m_grnMaxPlayers, sliderCtrlCS, false );
		m_pList->AddControl( m_pMaxPlayers );

		m_pUseTeams = new GameRuleBoolCtrl;
		m_pUseTeams->Create( *this, gameModeMgr.m_grbUseTeams, tcs, false );
		m_pList->AddControl( m_pUseTeams );

		m_pSwitchTeamsBetweenRounds = new GameRuleBoolCtrl;
		m_pSwitchTeamsBetweenRounds->Create( *this, gameModeMgr.m_grbSwitchTeamsBetweenRounds, tcs, false );
		m_pList->AddControl( m_pSwitchTeamsBetweenRounds );

		m_pRespawnWaitTime = new GameRuleUint32SliderCtrl;
		m_pRespawnWaitTime->Create( *this, gameModeMgr.m_grnRespawnWaitTime, sliderCtrlCS, false );
		m_pList->AddControl( m_pRespawnWaitTime );

		m_pTeamKillRespawnPenalty = new GameRuleUint32SliderCtrl;
		m_pTeamKillRespawnPenalty->Create( *this, gameModeMgr.m_grnTeamKillRespawnPenalty, sliderCtrlCS, false );
		m_pList->AddControl( m_pTeamKillRespawnPenalty );

		m_pAccumulateRespawnPenalty = new GameRuleBoolCtrl;
		m_pAccumulateRespawnPenalty->Create( *this, gameModeMgr.m_grbAccumulateRespawnPenalty, tcs, false );
		m_pList->AddControl( m_pAccumulateRespawnPenalty );

		m_pUseSlowMo = new GameRuleBoolCtrl;
		m_pUseSlowMo->Create( *this, gameModeMgr.m_grbUseSlowMo, tcs, false );
		m_pList->AddControl( m_pUseSlowMo );

		m_pSlowMoPersistsAcrossDeath = new GameRuleBoolCtrl;
		m_pSlowMoPersistsAcrossDeath->Create( *this, gameModeMgr.m_grbSlowMoPersistsAcrossDeath, tcs, false );
		m_pList->AddControl( m_pSlowMoPersistsAcrossDeath );
		
		m_pSlowMoRespawnAfterUse = new GameRuleBoolCtrl;
		m_pSlowMoRespawnAfterUse->Create( *this, gameModeMgr.m_grbSlowMoRespawnAfterUse, tcs, false );
		m_pList->AddControl( m_pSlowMoRespawnAfterUse );

		m_pSlowMoNavMarker = new GameRuleBoolCtrl;
		m_pSlowMoNavMarker->Create( *this, gameModeMgr.m_grbSlowMoNavMarker, tcs, false );
		m_pList->AddControl( m_pSlowMoNavMarker );

		m_pSlowMoHoldScorePeriod = new GameRuleUint32SliderCtrl;
		m_pSlowMoHoldScorePeriod->Create( *this, gameModeMgr.m_grnSlowMoHoldScorePeriod, sliderCtrlCS, false );
		m_pList->AddControl( m_pSlowMoHoldScorePeriod );

		m_pSlowMoHoldScorePlayer = new GameRuleUint32SliderCtrl;
		m_pSlowMoHoldScorePlayer->Create( *this, gameModeMgr.m_grnSlowMoHoldScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pSlowMoHoldScorePlayer );

		m_pSlowMoHoldScoreTeam = new GameRuleUint32SliderCtrl;
		m_pSlowMoHoldScoreTeam->Create( *this, gameModeMgr.m_grnSlowMoHoldScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pSlowMoHoldScoreTeam );

		m_pCTFDefendFlagBaseScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagBaseScorePlayer->Create( *this, gameModeMgr.m_grnCTFDefendFlagBaseScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagBaseScorePlayer );

		m_pCTFDefendFlagBaseScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagBaseScoreTeam->Create( *this, gameModeMgr.m_grnCTFDefendFlagBaseScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagBaseScoreTeam );

		m_pCTFDefendFlagBaseRadius = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagBaseRadius->Create( *this, gameModeMgr.m_grnCTFDefendFlagBaseRadius, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagBaseRadius );

		m_pCTFDefendFlagCarrierScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagCarrierScorePlayer->Create( *this, gameModeMgr.m_grnCTFDefendFlagCarrierScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagCarrierScorePlayer );

		m_pCTFDefendFlagCarrierScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagCarrierScoreTeam->Create( *this, gameModeMgr.m_grnCTFDefendFlagCarrierScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagCarrierScoreTeam );

		m_pCTFDefendFlagCarrierRadius = new GameRuleUint32SliderCtrl;
		m_pCTFDefendFlagCarrierRadius->Create( *this, gameModeMgr.m_grnCTFDefendFlagCarrierRadius, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFDefendFlagCarrierRadius );

		m_pCTFKillFlagCarrierScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFKillFlagCarrierScorePlayer->Create( *this, gameModeMgr.m_grnCTFKillFlagCarrierScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFKillFlagCarrierScorePlayer );

		m_pCTFKillFlagCarrierScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFKillFlagCarrierScoreTeam->Create( *this, gameModeMgr.m_grnCTFKillFlagCarrierScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFKillFlagCarrierScoreTeam );

		m_pCTFTeamKillFlagCarrierPenalty = new GameRuleUint32SliderCtrl;
		m_pCTFTeamKillFlagCarrierPenalty->Create( *this, gameModeMgr.m_grnCTFTeamKillFlagCarrierPenalty, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFTeamKillFlagCarrierPenalty );

		m_pCTFReturnFlagScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFReturnFlagScorePlayer->Create( *this, gameModeMgr.m_grnCTFReturnFlagScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFReturnFlagScorePlayer );

		m_pCTFReturnFlagScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFReturnFlagScoreTeam->Create( *this, gameModeMgr.m_grnCTFReturnFlagScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFReturnFlagScoreTeam );

		m_pCTFStealFlagScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFStealFlagScorePlayer->Create( *this, gameModeMgr.m_grnCTFStealFlagScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFStealFlagScorePlayer );

		m_pCTFStealFlagScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFStealFlagScoreTeam->Create( *this, gameModeMgr.m_grnCTFStealFlagScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFStealFlagScoreTeam );

		m_pCTFPickupFlagScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFPickupFlagScorePlayer->Create( *this, gameModeMgr.m_grnCTFPickupFlagScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFPickupFlagScorePlayer );

		m_pCTFPickupFlagScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFPickupFlagScoreTeam->Create( *this, gameModeMgr.m_grnCTFPickupFlagScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFPickupFlagScoreTeam );

		m_pCTFCaptureFlagScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFCaptureFlagScorePlayer->Create( *this, gameModeMgr.m_grnCTFCaptureFlagScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFCaptureFlagScorePlayer );

		m_pCTFCaptureFlagScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFCaptureFlagScoreTeam->Create( *this, gameModeMgr.m_grnCTFCaptureFlagScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFCaptureFlagScoreTeam );

		m_pCTFCaptureAssistTimeout = new GameRuleFloatSliderCtrl;
		m_pCTFCaptureAssistTimeout->Create( *this, gameModeMgr.m_grfCTFCaptureAssistTimeout, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCTFCaptureAssistTimeout );

		m_pCTFCaptureAssistScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCTFCaptureAssistScorePlayer->Create( *this, gameModeMgr.m_grnCTFCaptureAssistScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFCaptureAssistScorePlayer );

		m_pCTFCaptureAssistScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCTFCaptureAssistScoreTeam->Create( *this, gameModeMgr.m_grnCTFCaptureAssistScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCTFCaptureAssistScoreTeam );

		m_pCTFFlagLooseTimeout = new GameRuleFloatSliderCtrl;
		m_pCTFFlagLooseTimeout->Create( *this, gameModeMgr.m_grfCTFFlagLooseTimeout, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCTFFlagLooseTimeout );

		m_pCTFFlagMovementLimit = new GameRuleFloatSliderCtrl;
		m_pCTFFlagMovementLimit->Create( *this, gameModeMgr.m_grfCTFFlagMovementLimit, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCTFFlagMovementLimit );

		m_pCPCapturingTime = new GameRuleFloatSliderCtrl;
		m_pCPCapturingTime->Create( *this, gameModeMgr.m_grfCPCapturingTime, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCPCapturingTime );

		m_pCPGroupCaptureFactor = new GameRuleFloatSliderCtrl;
		m_pCPGroupCaptureFactor->Create( *this, gameModeMgr.m_grfCPGroupCaptureFactor, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCPGroupCaptureFactor );

		m_pCPDefendScorePlayer = new GameRuleUint32SliderCtrl;
		m_pCPDefendScorePlayer->Create( *this, gameModeMgr.m_grnCPDefendScorePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPDefendScorePlayer );

		m_pCPDefendScoreTeam = new GameRuleUint32SliderCtrl;
		m_pCPDefendScoreTeam->Create( *this, gameModeMgr.m_grnCPDefendScoreTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPDefendScoreTeam );

		m_pCPDefendRadius = new GameRuleUint32SliderCtrl;
		m_pCPDefendRadius->Create( *this, gameModeMgr.m_grnCPDefendRadius, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPDefendRadius );

		m_pCPOwnedScoreAmountTeam = new GameRuleUint32SliderCtrl;
		m_pCPOwnedScoreAmountTeam->Create( *this, gameModeMgr.m_grnCPOwnedScoreAmountTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPOwnedScoreAmountTeam );

		m_pCPOwnedScorePeriod = new GameRuleFloatSliderCtrl;
		m_pCPOwnedScorePeriod->Create( *this, gameModeMgr.m_grfCPOwnedScorePeriod, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pCPOwnedScorePeriod );

		m_pCPScoreLoseTeam = new GameRuleUint32SliderCtrl;
		m_pCPScoreLoseTeam->Create( *this, gameModeMgr.m_grnCPScoreLoseTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPScoreLoseTeam );

		m_pCPScoreNeutralizeTeam = new GameRuleUint32SliderCtrl;
		m_pCPScoreNeutralizeTeam->Create( *this, gameModeMgr.m_grnCPScoreNeutralizeTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPScoreNeutralizeTeam );

		m_pCPScoreCaptureTeam = new GameRuleUint32SliderCtrl;
		m_pCPScoreCaptureTeam->Create( *this, gameModeMgr.m_grnCPScoreCaptureTeam, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPScoreCaptureTeam );

		m_pCPScoreNeutralizePlayer = new GameRuleUint32SliderCtrl;
		m_pCPScoreNeutralizePlayer->Create( *this, gameModeMgr.m_grnCPScoreNeutralizePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPScoreNeutralizePlayer );

		m_pCPScoreCapturePlayer = new GameRuleUint32SliderCtrl;
		m_pCPScoreCapturePlayer->Create( *this, gameModeMgr.m_grnCPScoreCapturePlayer, sliderCtrlCS, false );
		m_pList->AddControl( m_pCPScoreCapturePlayer );

		CLTGUICycleCtrl_create ccs;
		ccs.rnBaseRect = cs.rnBaseRect;
		ccs.nHeaderWidth = kColumn;
		m_pSpawnSelection = new GameRuleEnumCtrl;
		m_pSpawnSelection->Create( *this, gameModeMgr.m_greSpawnPointSelect, ccs, false );
		m_pList->AddControl( m_pSpawnSelection );

		m_pTeamSizeBalancing = new GameRuleEnumCtrl;
		m_pTeamSizeBalancing->Create( *this, gameModeMgr.m_greTeamSizeBalancing, ccs, false );
		m_pList->AddControl( m_pTeamSizeBalancing );

		m_pTeamScoreBalancing = new GameRuleEnumCtrl;
		m_pTeamScoreBalancing ->Create( *this, gameModeMgr.m_greTeamScoreBalancing , ccs, false );
		m_pList->AddControl( m_pTeamScoreBalancing  );

		m_pTeamScoreBalancingPercent = new GameRuleFloatSliderCtrl;
		m_pTeamScoreBalancingPercent->Create( *this, gameModeMgr.m_grfTeamScoreBalancingPercent, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pTeamScoreBalancingPercent );

		m_pEndRoundMessageTime = new GameRuleFloatSliderCtrl;
		m_pEndRoundMessageTime->Create( *this, gameModeMgr.m_grfEndRoundMessageTime, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pEndRoundMessageTime );

		m_pEndRoundScoreScreenTime = new GameRuleFloatSliderCtrl;
		m_pEndRoundScoreScreenTime->Create( *this, gameModeMgr.m_grfEndRoundScoreScreenTime, floatSliderCtrlCS, false );
		m_pList->AddControl( m_pEndRoundScoreScreenTime );

		m_pAllowKillerTrackSpectating = new GameRuleBoolCtrl;
		m_pAllowKillerTrackSpectating->Create( *this, gameModeMgr.m_grbAllowKillerTrackSpectating, tcs, false );
		m_pList->AddControl( m_pAllowKillerTrackSpectating );

		ShowGameRuleInOptions( *m_pRunSpeed, gameModeMgr.m_grfRunSpeed );
		ShowGameRuleInOptions( *m_pFriendlyFire, gameModeMgr.m_grbFriendlyFire );
		ShowGameRuleInOptions( *m_pWeaponsStay, gameModeMgr.m_grbWeaponsStay );
		ShowGameRuleInOptions( *m_pUseLoadout, gameModeMgr.m_grbUseLoadout );
		ShowGameRuleInOptions( *m_pUseWeaponRestrictions, gameModeMgr.m_grbUseWeaponRestrictions );
		ShowGameRuleInOptions( *m_pTeamReflectDamage, gameModeMgr.m_grfTeamReflectDamage );
		ShowGameRuleInOptions( *m_pTeamDamagePercent, gameModeMgr.m_grfTeamDamagePercent );
		ShowGameRuleInOptions( *m_pScoreLimit, gameModeMgr.m_grnScoreLimit );
		ShowGameRuleInOptions( *m_pTimeLimit, gameModeMgr.m_grnTimeLimit );
		ShowGameRuleInOptions( *m_pSuddenDeathTimeLimit, gameModeMgr.m_grnSuddenDeathTimeLimit );
		ShowGameRuleInOptions( *m_pJoinGracePeriod, gameModeMgr.m_grnJoinGracePeriod );
		ShowGameRuleInOptions( *m_pMaxWeapons, gameModeMgr.m_grnMaxWeapons );
		ShowGameRuleInOptions( *m_pNumRounds, gameModeMgr.m_grnNumRounds );
		ShowGameRuleInOptions( *m_pFragScorePlayer, gameModeMgr.m_grnFragScorePlayer );
		ShowGameRuleInOptions( *m_pFragScoreTeam, gameModeMgr.m_grnFragScoreTeam );
		ShowGameRuleInOptions( *m_pMaxPlayers, gameModeMgr.m_grnMaxPlayers );
		ShowGameRuleInOptions( *m_pUseTeams, gameModeMgr.m_grbUseTeams );
		ShowGameRuleInOptions( *m_pSwitchTeamsBetweenRounds, gameModeMgr.m_grbSwitchTeamsBetweenRounds );
		ShowGameRuleInOptions( *m_pRespawnWaitTime, gameModeMgr.m_grnRespawnWaitTime );
		ShowGameRuleInOptions( *m_pTeamKillRespawnPenalty, gameModeMgr.m_grnTeamKillRespawnPenalty );
		ShowGameRuleInOptions( *m_pAccumulateRespawnPenalty, gameModeMgr.m_grbAccumulateRespawnPenalty );
		ShowGameRuleInOptions( *m_pSpawnSelection, gameModeMgr.m_greSpawnPointSelect );
		ShowGameRuleInOptions( *m_pUseSlowMo, gameModeMgr.m_grbUseSlowMo );
		ShowGameRuleInOptions( *m_pSlowMoPersistsAcrossDeath, gameModeMgr.m_grbSlowMoPersistsAcrossDeath );
		ShowGameRuleInOptions( *m_pSlowMoRespawnAfterUse, gameModeMgr.m_grbSlowMoRespawnAfterUse );
		ShowGameRuleInOptions( *m_pSlowMoNavMarker, gameModeMgr.m_grbSlowMoNavMarker );
		ShowGameRuleInOptions( *m_pSlowMoHoldScorePeriod, gameModeMgr.m_grnSlowMoHoldScorePeriod );
		ShowGameRuleInOptions( *m_pSlowMoHoldScorePlayer, gameModeMgr.m_grnSlowMoHoldScorePlayer );
		ShowGameRuleInOptions( *m_pSlowMoHoldScoreTeam, gameModeMgr.m_grnSlowMoHoldScoreTeam );

		ShowGameRuleInOptions( *m_pCTFDefendFlagBaseScorePlayer, gameModeMgr.m_grnCTFDefendFlagBaseScorePlayer );
		ShowGameRuleInOptions( *m_pCTFDefendFlagBaseScoreTeam, gameModeMgr.m_grnCTFDefendFlagBaseScoreTeam );
		ShowGameRuleInOptions( *m_pCTFDefendFlagBaseRadius, gameModeMgr.m_grnCTFDefendFlagBaseRadius );
		ShowGameRuleInOptions( *m_pCTFDefendFlagCarrierScorePlayer, gameModeMgr.m_grnCTFDefendFlagCarrierScorePlayer );
		ShowGameRuleInOptions( *m_pCTFDefendFlagCarrierScoreTeam, gameModeMgr.m_grnCTFDefendFlagCarrierScoreTeam );
		ShowGameRuleInOptions( *m_pCTFDefendFlagCarrierRadius, gameModeMgr.m_grnCTFDefendFlagCarrierRadius );
		ShowGameRuleInOptions( *m_pCTFKillFlagCarrierScorePlayer, gameModeMgr.m_grnCTFKillFlagCarrierScorePlayer );
		ShowGameRuleInOptions( *m_pCTFKillFlagCarrierScoreTeam, gameModeMgr.m_grnCTFKillFlagCarrierScoreTeam );
		ShowGameRuleInOptions( *m_pCTFTeamKillFlagCarrierPenalty, gameModeMgr.m_grnCTFTeamKillFlagCarrierPenalty );
		ShowGameRuleInOptions( *m_pCTFReturnFlagScorePlayer, gameModeMgr.m_grnCTFCaptureFlagScorePlayer );
		ShowGameRuleInOptions( *m_pCTFReturnFlagScoreTeam, gameModeMgr.m_grnCTFCaptureFlagScoreTeam );
		ShowGameRuleInOptions( *m_pCTFStealFlagScorePlayer, gameModeMgr.m_grnCTFCaptureFlagScorePlayer );
		ShowGameRuleInOptions( *m_pCTFStealFlagScoreTeam, gameModeMgr.m_grnCTFCaptureFlagScoreTeam );
		ShowGameRuleInOptions( *m_pCTFPickupFlagScorePlayer, gameModeMgr.m_grnCTFCaptureFlagScorePlayer );
		ShowGameRuleInOptions( *m_pCTFPickupFlagScoreTeam, gameModeMgr.m_grnCTFCaptureFlagScoreTeam );
		ShowGameRuleInOptions( *m_pCTFCaptureFlagScorePlayer, gameModeMgr.m_grnCTFCaptureFlagScorePlayer );
		ShowGameRuleInOptions( *m_pCTFCaptureFlagScoreTeam, gameModeMgr.m_grnCTFCaptureFlagScoreTeam );
		ShowGameRuleInOptions( *m_pCTFCaptureAssistTimeout, gameModeMgr.m_grfCTFCaptureAssistTimeout );
		ShowGameRuleInOptions( *m_pCTFCaptureAssistScorePlayer, gameModeMgr.m_grnCTFCaptureAssistScorePlayer );
		ShowGameRuleInOptions( *m_pCTFCaptureAssistScoreTeam, gameModeMgr.m_grnCTFCaptureAssistScoreTeam );
		ShowGameRuleInOptions( *m_pCTFFlagLooseTimeout, gameModeMgr.m_grfCTFFlagLooseTimeout );
		ShowGameRuleInOptions( *m_pCTFFlagMovementLimit, gameModeMgr.m_grfCTFFlagMovementLimit );

		ShowGameRuleInOptions( *m_pCPCapturingTime, gameModeMgr.m_grfCPCapturingTime );
		ShowGameRuleInOptions( *m_pCPGroupCaptureFactor, gameModeMgr.m_grfCPGroupCaptureFactor );
		ShowGameRuleInOptions( *m_pCPDefendScorePlayer, gameModeMgr.m_grnCPDefendScorePlayer );
		ShowGameRuleInOptions( *m_pCPDefendScoreTeam, gameModeMgr.m_grnCPDefendScoreTeam );
		ShowGameRuleInOptions( *m_pCPDefendRadius, gameModeMgr.m_grnCPDefendRadius );
		ShowGameRuleInOptions( *m_pCPOwnedScoreAmountTeam, gameModeMgr.m_grnCPOwnedScoreAmountTeam );
		ShowGameRuleInOptions( *m_pCPOwnedScorePeriod, gameModeMgr.m_grfCPOwnedScorePeriod );
		ShowGameRuleInOptions( *m_pCPScoreLoseTeam, gameModeMgr.m_grnCPScoreLoseTeam );
		ShowGameRuleInOptions( *m_pCPScoreNeutralizeTeam, gameModeMgr.m_grnCPScoreNeutralizeTeam );
		ShowGameRuleInOptions( *m_pCPScoreCaptureTeam, gameModeMgr.m_grnCPScoreCaptureTeam );
		ShowGameRuleInOptions( *m_pCPScoreNeutralizePlayer, gameModeMgr.m_grnCPScoreNeutralizePlayer );
		ShowGameRuleInOptions( *m_pCPScoreCapturePlayer, gameModeMgr.m_grnCPScoreCapturePlayer );

		ShowGameRuleInOptions( *m_pTeamSizeBalancing, gameModeMgr.m_greTeamSizeBalancing );
		ShowGameRuleInOptions( *m_pTeamScoreBalancing, gameModeMgr.m_greTeamScoreBalancing );
		ShowGameRuleInOptions( *m_pTeamScoreBalancingPercent, gameModeMgr.m_grfTeamScoreBalancingPercent );
		ShowGameRuleInOptions( *m_pEndRoundMessageTime, gameModeMgr.m_grfEndRoundMessageTime );
		ShowGameRuleInOptions( *m_pEndRoundScoreScreenTime, gameModeMgr.m_grfEndRoundScoreScreenTime );

		ShowGameRuleInOptions( *m_pAllowKillerTrackSpectating, gameModeMgr.m_grbAllowKillerTrackSpectating );

		// Need to recalculate the positions since they may have been hidden or reshown.
		m_pList->RecalcLayout();

		m_pMaxPlayers->Enable(!g_pPlayerMgr->IsPlayerInWorld());
		if(!gameModeMgr.m_ServerSettings.m_bLANOnly)
		{
			uint32 nBandwidthMax = gameModeMgr.m_ServerSettings.GetMaxPlayersForBandwidth();
			if (nBandwidthMax > 2)
			{
				m_pMaxPlayers->SetSliderRange(2, nBandwidthMax);
			}
			else
			{
				nBandwidthMax = 2;
				m_pMaxPlayers->SetSliderRange(1, 2);
				m_pMaxPlayers->Enable( false );
			}
			if (gameModeMgr.m_grnMaxPlayers > nBandwidthMax)
			{
				gameModeMgr.m_grnMaxPlayers = nBandwidthMax;
			}
		}

        UpdateData(false);

		m_pList->RecalcLayout();
	}
	else
	{
		// Save and send the options to the server if we can modify them in game.
		// We have to avoid calling UpdateData, since the options could have changed while
		// we were viewing this screen, in which case the controls will have old data in them.
		if( !g_pPlayerMgr->IsPlayerInWorld( ) || !g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
		{
			UpdateData();

			pProfile->Save();

			// We do not include server options because this screen does not modify them.  
			// Note that this screen can be used while in a multi-player game, and while in a multi-player
			// game our server options are just the default options.
			GameModeMgr::Instance().WriteToOptionsFile( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ), false);

			if( g_pPlayerMgr->IsPlayerInWorld( ))
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
				GameModeMgr::Instance().WriteToMsg( cMsg );
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}
		}

		SetSelection(kNoSelection);
		RemoveControl( m_pList, true );
		RemoveControl( m_pScroll, true );
		m_pList = NULL;
		m_pScroll = NULL;
		m_pRunSpeed = NULL;
		m_pFriendlyFire = NULL;
		m_pWeaponsStay = NULL;
		m_pUseLoadout = NULL;
		m_pUseWeaponRestrictions = NULL;
		m_pTeamReflectDamage = NULL;
		m_pTeamDamagePercent = NULL;
		m_pScoreLimit = NULL;
		m_pTimeLimit = NULL;
		m_pSuddenDeathTimeLimit = NULL;
		m_pJoinGracePeriod = NULL;
		m_pMaxWeapons = NULL;
		m_pNumRounds = NULL;
		m_pFragScorePlayer = NULL;
		m_pFragScoreTeam = NULL;
		m_pMaxPlayers = NULL;
		m_pUseTeams = NULL;
		m_pSwitchTeamsBetweenRounds = NULL;
		m_pRespawnWaitTime = NULL;
		m_pTeamKillRespawnPenalty = NULL;
		m_pAccumulateRespawnPenalty = NULL;
		m_pSpawnSelection = NULL;
		m_pUseSlowMo = NULL;
		m_pSlowMoPersistsAcrossDeath = NULL;
		m_pSlowMoRespawnAfterUse = NULL;
		m_pCTFDefendFlagBaseScorePlayer = NULL;
		m_pCTFDefendFlagBaseScoreTeam = NULL;
		m_pCTFDefendFlagBaseRadius = NULL;
		m_pCTFDefendFlagCarrierScorePlayer = NULL;
		m_pCTFDefendFlagCarrierScoreTeam = NULL;
		m_pCTFDefendFlagCarrierRadius = NULL;
		m_pCTFKillFlagCarrierScorePlayer = NULL;
		m_pCTFKillFlagCarrierScoreTeam = NULL;
		m_pCTFTeamKillFlagCarrierPenalty = NULL;
		m_pCTFReturnFlagScorePlayer = NULL;
		m_pCTFReturnFlagScoreTeam = NULL;
		m_pCTFStealFlagScorePlayer = NULL;
		m_pCTFStealFlagScoreTeam = NULL;
		m_pCTFPickupFlagScorePlayer = NULL;
		m_pCTFPickupFlagScoreTeam = NULL;
		m_pCTFCaptureFlagScorePlayer = NULL;
		m_pCTFCaptureFlagScoreTeam = NULL;
		m_pCTFCaptureAssistTimeout = NULL;
		m_pCTFCaptureAssistScorePlayer = NULL;
		m_pCTFCaptureAssistScoreTeam = NULL;
		m_pCTFFlagLooseTimeout = NULL;
		m_pCTFFlagMovementLimit = NULL;
		m_pCPCapturingTime = NULL;
		m_pCPGroupCaptureFactor = NULL;
		m_pCPDefendScorePlayer = NULL;
		m_pCPDefendScoreTeam = NULL;
		m_pCPDefendRadius = NULL;
		m_pCPOwnedScoreAmountTeam = NULL;
		m_pCPOwnedScorePeriod = NULL;
		m_pCPScoreLoseTeam = NULL;
		m_pCPScoreNeutralizeTeam = NULL;
		m_pCPScoreCaptureTeam = NULL;
		m_pCPScoreNeutralizePlayer = NULL;
		m_pCPScoreCapturePlayer = NULL;
	}

	CBaseScreen::OnFocus(bFocus);

	if (bFocus && (!g_pPlayerMgr->IsPlayerInWorld( ) || g_pClientConnectionMgr->IsConnectedToRemoteServer( )) )
	{
		SetSelection(kNoSelection);
	}
}

