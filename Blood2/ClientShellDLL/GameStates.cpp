/****************************************************************************
;
;	 MODULE:		GameStates (.CPP)
;
;	PURPOSE:		BloodClientShell Functions for all the game states
;
;	HISTORY:		10/05/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "BloodClientShell.h"
#include "Splash.h"


// Functions...

DBOOL CBloodClientShell::SetGameState(int nState)
{
	// Sanity checks...

	if (!IsGameStateValid(nState)) return(DFALSE);


	// Transition between the old and new state...

	int nOldState = GetGameState();
	int nNewState = nState;

	if (!TransitionGameState(nOldState, nNewState))
	{
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::IsGameStateValid(int nState)
{
	// Check if the given game state is a valid one...

	if (nState == GS_NONE) return(DTRUE);
	if (nState >= GS_PLAYING && nState <= GS_WAITING) return(DTRUE);

	return(DFALSE);
}

DBOOL CBloodClientShell::TransitionGameState(int nOldState, int nNewState)
{
	// Sanity checks...

	if (!IsGameStateValid(nNewState)) return(DFALSE);


	// Notify the old state that we are exiting it...

	switch (nOldState)
	{
		case GS_PLAYING:		OnExitPlayingState(nNewState); break;
		case GS_MENU:			OnExitMenuState(nNewState); break;
		case GS_MENUANIM:		OnExitMenuAnimState(nNewState); break;
		case GS_LOADINGLEVEL:	OnExitLoadingLevelState(nNewState); break;
		case GS_SAVING:			OnExitSavingState(nNewState); break;
		case GS_MPLOADINGLEVEL: OnExitMultiLoadingLevelState(nNewState); break;
		case GS_CREDITS:		OnExitCreditsState(nNewState); break;
		case GS_MOVIES:			OnExitMoviesState(nNewState); break;
		case GS_SPLASH:			OnExitSplashState(nNewState); break;
		case GS_WAITING:		OnExitWaitingState(nNewState); break;
	}


	// Set the current game state to the new state...

	m_nGameState = nNewState;


	// Notify the new state that we are entering it...

	switch (nNewState)
	{
		case GS_PLAYING:		OnEnterPlayingState(nOldState); break;
		case GS_MENU:			OnEnterMenuState(nOldState); break;
		case GS_MENUANIM:		OnEnterMenuAnimState(nOldState); break;
		case GS_LOADINGLEVEL:	OnEnterLoadingLevelState(nOldState); break;
		case GS_SAVING:			OnEnterSavingState(nOldState); break;
		case GS_MPLOADINGLEVEL: OnEnterMultiLoadingLevelState(nOldState); break;
		case GS_CREDITS:		OnEnterCreditsState(nOldState); break;
		case GS_MOVIES:			OnEnterMoviesState(nOldState); break;
		case GS_SPLASH:			OnEnterSplashState(nOldState); break;
		case GS_WAITING:		OnEnterWaitingState(nOldState); break;
	}


	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterPlayingState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterMenuState(int nOldState)
{
	m_Menu.SetFocus(DTRUE);

	if (nOldState == GS_SPLASH)
	{
		m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
	}


	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterMenuAnimState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterLoadingLevelState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterSavingState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterMultiLoadingLevelState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterCreditsState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterMoviesState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterSplashState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnEnterWaitingState(int nOldState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitPlayingState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitMenuState(int nNewState)
{
	m_Menu.SetFocus(DFALSE);

	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitMenuAnimState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitLoadingLevelState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitSavingState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitMultiLoadingLevelState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitCreditsState(int nNewState)
{
	// Terminate the credits member now that we're done with it...

	m_Credits.Term();


	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitMoviesState(int nNewState)
{
	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitSplashState(int nNewState)
{
	// Term the splash stuff...

	Splash_Term();


	// All done...

	return(DTRUE);
}

DBOOL CBloodClientShell::OnExitWaitingState(int nNewState)
{
	// All done...

	return(DTRUE);
}