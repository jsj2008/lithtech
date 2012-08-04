// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTimer.cpp
//
// PURPOSE : HUDItem to display a meter
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "HUDTimer.h"
#include "MissionMgr.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::CHUDTimer
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDTimer::CHUDTimer()
:	CHUDItem		( ),
	m_fTime(0.0f),
	m_fTimeLeft(0.0f),
	m_bPause(false),
	m_bDraw(false),
	m_nLastMinutes(0),
	m_nLastSeconds(0)
{
	m_eLevel = kHUDRenderText;
	m_pos.Init();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::CHUDTimer
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CHUDTimer::~CHUDTimer()
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::Init
//
//  PURPOSE:	Setup the timer...
//
// ----------------------------------------------------------------------- //

bool CHUDTimer::Init()
{
	UpdateLayout();

	Update();

	SetSourceString( LoadString("HUD_Timer_Chars"));

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::Render
//
//  PURPOSE:	Draw the meter...
//
// ----------------------------------------------------------------------- //

void CHUDTimer::Render()
{
	if( !m_bDraw )
		return;

	SetRenderState();

	// If we're not paused, recalculate the time left.
	if( !m_bPause )
		m_fTimeLeft = m_fTime - g_pLTClient->GetGameTime( );

	// Update/Draw the timer if there is anytime left...
	if( m_fTimeLeft <= 0.0f )
	{
		m_bDraw	= false;
		return;
	}

	// Draw the string to the surface...

	int nMinutes = int(m_fTimeLeft) / 60;
	int nSeconds = m_fTimeLeft > 60.0 ? int(m_fTimeLeft) % 60 : int(m_fTimeLeft);

	wchar_t wBuffer[16] = L"";

	if (nMinutes != m_nLastMinutes || nSeconds != m_nLastSeconds)
	{
		m_nLastMinutes = nMinutes;
		m_nLastSeconds = nSeconds;

		FormatString("HUD_Timer_Format",wBuffer,LTARRAYSIZE(wBuffer),nMinutes, nSeconds);
		m_Text.SetText(wBuffer);
	}


	m_Text.Render();


}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::Update
//
//  PURPOSE:	Set the values for the bar to draw...
//
// ----------------------------------------------------------------------- //

void CHUDTimer::Update()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDTimer::ScaleChanged()
//
//  PURPOSE:	Update to fit new screen resolution
//
// ----------------------------------------------------------------------- //

void CHUDTimer::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}


void CHUDTimerMain::UpdateLayout()
{
	if( g_pMissionMgr->GetServerGameState() == EServerGameState_PlayingSuddenDeath )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTimerSuddenDeath");
	}
	else
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTimerMain");
	}

	CHUDItem::UpdateLayout();
}

void CHUDTimerTeam0::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTimerTeam0");
	}

	CHUDItem::UpdateLayout();

}

void CHUDTimerTeam1::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTimerTeam1");
	}

	CHUDItem::UpdateLayout();
}
