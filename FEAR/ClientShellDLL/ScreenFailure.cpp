// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.cpp
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenFailure.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "HUDMgr.h"
#include "ClientDB.h"
#include "sys/win/mpstrconv.h"

static float	s_fMinDelay = 1.0f;
uint32 CScreenFailure::sm_nTipIndex = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenFailure::CScreenFailure()
:	CBaseScreen				( ),
	m_pContinue				( NULL ),
	m_pTips					( NULL ),
	m_fDuration				( 0.0f ),
	m_bFlash				( false ),
	m_fFlashTime			( 0.0f ),
	m_bRestart				( false )
{
}

CScreenFailure::~CScreenFailure()
{

}


// Build the screen
bool CScreenFailure::Build()
{
	ClientDB& ClientDatabase = ClientDB::Instance();
	HRECORD hShared = ClientDatabase.GetClientSharedRecord( );
	s_fMinDelay = ClientDatabase.GetFloat(hShared,CDB_fMissionFailureDelay,0);

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMax = LTVector2n(g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	m_pTips = AddTextItem(L"",cs, true, g_pLayoutDB->GetHelpFont());
	m_pTips->SetWordWrap(true);
	

	LTVector2n pos(m_ScreenRect.Left(),m_ScreenRect.Bottom());
	cs.rnBaseRect.m_vMin = pos;
	cs.rnBaseRect.m_vMax = pos + LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	

	m_pContinue = AddTextItem(LoadString("ScreenFailure_PressAnyKey"),cs, true,NULL,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt));


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(false);
	return true;
}


void CScreenFailure::OnFocus(bool bFocus)
{
	
	if (bFocus)
	{
		CreateTitle(g_pInterfaceMgr->GetFailStringID());

		ClientDB& ClientDatabase = ClientDB::Instance();
		HRECORD hShared = ClientDatabase.GetClientSharedRecord( );
		HATTRIBUTE hAtt = ClientDatabase.GetAttribute(hShared,CDB_sMissionFailureTips);
		uint32 nNumTips = g_pLTDatabase->GetNumValues(hAtt);

		if (nNumTips > 0)
		{
			if (sm_nTipIndex >= nNumTips )
			{
				sm_nTipIndex = 0;
			}

			const char* szTip = ClientDatabase.GetString(hAtt,sm_nTipIndex);
			m_pTips->SetString(CreateHelpString(szTip));

			sm_nTipIndex++;

		}
		else
		{
			m_pTips->Show(false);
		}


		m_pContinue->Show(false);

		// Reset our flash data...

		m_bFlash = false;
		m_fFlashTime = RealTimeTimer::Instance().GetTimerAccumulatedS() + 0.333f;
		
		// Reset the amount of time the screen has been active...
		
		m_fDuration = 0.0f;

		m_bRestart = false;

	}
	else
	{
	}

	CBaseScreen::OnFocus(bFocus);
}


void CScreenFailure::Escape()
{

	if (m_bRestart)
	{
		// Load the most recent save game.
		if( !g_pMissionMgr->StartGameFromContinue( ))
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
		}
	}
	else
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
	}
}

bool CScreenFailure::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	if( m_pContinue->IsVisible() )
	{
		if (key != VK_ESCAPE)
			m_bRestart = true;
		Escape();
		return true;
	}
	return false;
#else // !PLATFORM_XENON
	m_bRestart = true;
	Escape();
	return true;
#endif // !PLATFORM_XENON

}
bool CScreenFailure::OnLButtonDown(int x, int y)
{
	if (m_fDuration > s_fMinDelay)
	{
		m_bRestart = true;
		Escape();
		return true;
	}

	return false;
}
bool CScreenFailure::OnRButtonDown(int x, int y)
{
	if (m_fDuration > s_fMinDelay)
	{
		m_bRestart = true;
		Escape();
		return true;
	}
	return false;

}


bool CScreenFailure::Render()
{
	CBaseScreen::Render();

	double fTime = RealTimeTimer::Instance().GetTimerAccumulatedS();
	if (fTime > m_fFlashTime)
	{
		m_bFlash = !m_bFlash;
		m_fFlashTime = fTime + 0.333f;
		if (m_bFlash)
		{
			m_pContinue->SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
		}
		else
		{
			m_pContinue->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		}

	}

	m_fDuration += RealTimeTimer::Instance().GetTimerElapsedS();

	if( (m_fDuration > s_fMinDelay && !m_pContinue->IsVisible())  )
	{
		m_pContinue->Show(true);
		g_pClientSoundMgr->PlayInterfaceDBSound("PressAnyKey");
	}


	return true;

}




