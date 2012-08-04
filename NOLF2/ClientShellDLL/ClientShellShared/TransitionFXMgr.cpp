// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionFXMgr.cpp
//
// PURPOSE : Base management class for handling multiple transitions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ChainedFX.h"
#include "TransitionFXMgr.h"
#include "GameClientShell.h"
#include "LayoutMgr.h"

/*
  NOTES on the TransitionFXMgr

  The TransitionFXMgr is a class for managing the visual effects displayed
  when changing from one menu screen to another.  It acts as a wrapper
  class for JimG's "ChainedFX" class, and as a supplement/replacement for
  the interface to ChainedFX contained in the BaseScreen class.

  It is contained as a data member within the ScreenMgr class, which
  handles initialization and termination for it.  All other calls to the
  TransitionFXMgr are done directly from the screen that uses it, through
  calls embedded throughout the CBaseScreen class.  In the interests of
  minimizing (circular) dependencies, no header files #include the
  declaration of the TransitionFXMgr, favoring forward declarations instead
  and #including the header only within the CPP files.  At the moment, the
  only three files that make use of the header are BaseScreen.cpp, this
  file (TransitionFXMgr.cpp) and ScreenMgr.cpp.

  The data handled by the TransitionFXMgr is contained within the BUTE file
  "Layout.txt" in the form of entries of the "[Transition%d]" format.

  Access to the TransitionFXMgr is accomplished solely through a ScreenMgr
  accessor function, GetTransitionFXMgr().

*/

// ----------------------------------------------------------------------- //
// Constructor and Destructor
// ----------------------------------------------------------------------- //
CTransitionFXMgr::CTransitionFXMgr()
{
	m_iCurrentScreen = 0;
	m_iPreviousScreen = -1;
	m_pCurrentChain = NULL;
}

CTransitionFXMgr::~CTransitionFXMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::Init()
//
// Called from ScreenMgr. Load all the transitions from Layout.txt
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::Init()
{
	int iCurrentFrom = 0;
	int iCurrentTo = 0;
	m_bAllowLooping = true;

	ChainFXList * pChainFXList = LTNULL;

	// Iterate through all available transitionFX in layout.txt and add
	// them all to m_TransitionFX
	unsigned int nChainNum = 0;
	INT_CHAINFX * pFX = g_pLayoutMgr->GetChainFX(nChainNum);
	while (pFX)
	{
		// build the new ChainFX.
		CChainedFX * pChainFX = debug_new(CChainedFX);
		pChainFX->Init(pFX->szIntroName, pFX->szShortIntroName, pFX->szLoopName);
		pChainFX->m_iFromScreen = pFX->iFromScreen;
		pChainFX->m_iToScreen = pFX->iToScreen;

		// Where to add this new ChainFX?
		// If it's the first, then create a new chainFXList
		// It it's not, but doesn't match the last "from/to" pair, then create a new ChainFXList
		// If it matches, then add it to the end of the current ChainFXList

		if (!pChainFXList) // first time through
		{
			// Create a new ChainFXList pChainFXList
			pChainFXList = debug_new(ChainFXList);

			// Add the new ChainFXList to m_TransitionFX
			m_TransitionFX.push_back(pChainFXList);

			// Add pChainFX to the list pChainFXList
			pChainFXList->push_back(pChainFX);

			iCurrentFrom = pFX->iFromScreen;
			iCurrentTo = pFX->iToScreen;
		}
		else if (pFX->iFromScreen != iCurrentFrom || pFX->iToScreen != iCurrentTo)
		{
			// Create a new ChainFXList pChainFXList
			pChainFXList = debug_new(ChainFXList);

			// Add the new ChainFXList to m_TransitionFX
			m_TransitionFX.push_back(pChainFXList);

			// Add pChainFX to the list pChainFXList
			pChainFXList->push_back(pChainFX);

			iCurrentFrom = pFX->iFromScreen;
			iCurrentTo = pFX->iToScreen;
		}
		else
		{
			// Add pChainFX to the list pChainFXList
			pChainFXList->push_back(pChainFX);
		}

		nChainNum++;
		pFX = g_pLayoutMgr->GetChainFX(nChainNum);
	}
	while (pFX);
}


// ----------------------------------------------------------------------- //
// CTransitionFXMgr::Term()
//
// Called from ScreenMgr.
// iterate through m_TransitionFX and iterate through each chain, deleting.
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::Term()
{
	m_iCurrentScreen = 0;

	TransitionFXList::iterator iter = m_TransitionFX.begin();
	while (iter != m_TransitionFX.end())
	{
		TermChain(*iter);
		debug_delete(*iter);
		iter++;
	}
	m_TransitionFX.clear();
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::TermChain()
//
// Private helper function
// iterate through a single element of m_TransitionFX (a chain) and delete it
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::TermChain(ChainFXList * pChain)
{
	ChainFXList::iterator iter = pChain->begin();
	while (iter != pChain->end())
	{
		debug_delete(*iter);
		iter++;
	}
	pChain->clear();
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::AddTransitionFX(pChainFX)
//
// Possibly unused call to have the individual folder request that its effects be added
// to the master array.  If there is an effect already in the list that takes precedence
// then this ChainFX will be discarded.
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::AddTransitionFX(ChainFXList * pChainedFX)
{
	// Probably unused.  I prefer AddTransitionFX(screen), the next function down
	m_TransitionFX.push_back(pChainedFX);
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::AddTransitionFX(screen)
//
// Similar to the above call, potentially unused.  Call by the individual
// folder to have the TransitionFXMgr actually sift through that screen's
// own layout data to find chained effects to add to the master array of
// chained effects.  If there is an effect already in the list that takes
// precedence, then this ChainFX will be discarded
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::AddTransitionFX(int screen)
{
	// Andy, try to use this function, and remove it from BaseFolder
	// It involves including the screen names so that this function
	// can look for the right key.
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::CheckForBestTransition()
//
// Internal function for checking to see if this particular effect is a
// more suitable chainfx than the one currently referenced by m_pCurrentChain
// ----------------------------------------------------------------------- //
void CTransitionFXMgr::CheckForBestTransition(ChainFXList * pChainFX)
{
	// Quick discard case
	// If we already have a perfect match, exit.
	if (m_pCurrentChain)
	{
		CChainedFX * pCurrentFX = *(m_pCurrentChain->begin());
		if (pCurrentFX->m_iFromScreen == m_iPreviousScreen
			&& pCurrentFX->m_iToScreen == m_iCurrentScreen)
			return;
	}

	// Grab the first chainfx in this list
	CChainedFX * pFX = *(pChainFX->begin());
	int iFromScreen = pFX->m_iFromScreen;
	int iToScreen = pFX->m_iToScreen;

	// Second easy discard case
	// if this From and To match, it's the new currentchain
	if (iToScreen == m_iCurrentScreen && iFromScreen == m_iPreviousScreen)
	{
		m_pCurrentChain = pChainFX;
		return;
	}

	// If the "to" matches
	if (iToScreen == m_iCurrentScreen)
	{
		// "From" does not match, as we know from the previous "if" statement.
		// It can be 0, meaning that the "from" doesn't matter, or any other
		// value, meaning that it's just not a match.
		if (iFromScreen != 0)
			return;
		// We now know that this ChainedFX has the right "to" value, and is a
		// general purpose intro to this screen.  This takes precedence over
		// anything except a perfect match, which we already tested for, so
		// this ChainedFX is now our new "currentChain".
		m_pCurrentChain = pChainFX;
		return;
	}

	// if the "from" matches (and not the "to") it's an "Outro" FX for the previous
	// screen.  "Outro FX" are only considered when there is no intro of any kind
	// available.
	if (iFromScreen == m_iPreviousScreen)
	{
		if (!m_pCurrentChain)
			m_pCurrentChain = pChainFX;
		return;
	}
	// if neither matches, exit
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::EnterScreen()
//
// Called by the ScreenMgr for bookkeeping.  Sets "current" screen
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::EnterScreen(int screen)
{
	// If this assert fires, then a new screen is being entered before
	// cleaning up the previous screen
	ASSERT(m_pCurrentChain == NULL);
	if (m_pCurrentChain)
		return;

	m_iCurrentScreen = screen;
	// look through all the ChainFX in m_TransitionFX to find the chainedFX
	// with the highest priority.
	TransitionFXList::iterator iter = m_TransitionFX.begin();
	while (iter != m_TransitionFX.end())
	{
		CheckForBestTransition(*iter);
		iter++;
	}
}

void CTransitionFXMgr::ClearScreenHistory()
{
	m_iPreviousScreen = -1;
}
// ----------------------------------------------------------------------- //
// CTransitionFXMgr::HasTransitionFX()
//
// Simple test to see if the Start and Update functions need to be called.
// ----------------------------------------------------------------------- //

LTBOOL CTransitionFXMgr::HasTransitionFX()
{
	return (m_pCurrentChain != NULL);
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::ExitScreen(int screen)
//
// Called by the ScreenMgr for bookkeeping.  The arg is optional but worth
// checking.  Moves screen/current to "previous".  Note that a screen
// will already automatically remove all FX as part of its OnFocus(FALSE)
// functionality.
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::ExitScreen(int screen)
{
	if (screen != m_iCurrentScreen)
	{
		// If you get this assert, then somehow the ScreenMgr has lost track
		// of which screen you are exiting.  Non lethal
		ASSERT(0);
	}
	m_iPreviousScreen = m_iCurrentScreen;
	m_pCurrentChain = NULL;
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::StartTransitionFX()
//
// Called by BaseFolder::CreateInterfaceSFX.  Initializes the appropriate
// ChainFX for each individual screen
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::StartTransitionFX(bool bVisited)
{
	if (!m_pCurrentChain)
		return;

	ChainFXList::iterator iter = m_pCurrentChain->begin();

	while (iter != m_pCurrentChain->end())
	{
		(*iter)->SetLooping(m_bAllowLooping);
		(*iter)->Start(bVisited);
		iter++;
	}
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::UpdateTransitionFX()
//
// Called by BaseFolder::UpdateInterfaceSFX.  Updates the appropriate
// ChainFX for each individual screen
// ----------------------------------------------------------------------- //

void CTransitionFXMgr::UpdateTransitionFX()
{
	if (!m_pCurrentChain)
		return;

	ChainFXList::iterator iter = m_pCurrentChain->begin();
	while (iter != m_pCurrentChain->end())
	{
		(*iter)->Update();
		iter++;
	}
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::IsTransitionComplete()
//
// Optional check by a screen to see if all of the FX are looping.  If so
// then a transition is considered "complete"
// ----------------------------------------------------------------------- //

bool CTransitionFXMgr::IsTransitionComplete()
{
	if (!m_pCurrentChain)
		return true;

	ChainFXList::iterator iter = m_pCurrentChain->begin();
	while (iter != m_pCurrentChain->end())
	{
		if (!(*iter)->IsIntroDone())
		{
			return false;
		}
		iter++;
	}

	return true;
}

// ----------------------------------------------------------------------- //
// CTransitionFXMgr::EndTransition()
//
// Optional method for killing all ChainFX on the screen at an arbitrary
// time such as when the screen wishes to handle its own drawing when the
// screen is up.
// ----------------------------------------------------------------------- //
void CTransitionFXMgr::EndTransition()
{
	if (!m_pCurrentChain)
		return;

	ChainFXList::iterator iter = m_pCurrentChain->begin();
	while (iter != m_pCurrentChain->end())
	{
		(*iter)->End();
		iter++;
	}
	m_pCurrentChain = LTNULL;
}
