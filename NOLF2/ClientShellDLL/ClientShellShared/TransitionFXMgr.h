// ----------------------------------------------------------------------- //
//
// MODULE  : TransitionFXMgr.h
//
// PURPOSE : Base management class for handling multiple transitions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_TRANSITIONFXMGR_H)
#define _TRANSITIONFXMGR_H

// *sigh* I didn't want to do this.
#include "ChainedFX.h"

class CTransitionFXMgr
{
public:

	CTransitionFXMgr();
	~CTransitionFXMgr();

	// Call from ScreenMgr
	void Init();	// load all the transitions from Layout.txt
	void Term();	// iterate through m_TransitionFX and iterate through each chain, deleting.

	void AddTransitionFX(ChainFXList * pChainedFX);
	void AddTransitionFX(int screen);

	// Called by the BaseScreen::OnFocus(true/false) for bookkeeping
	void EnterScreen(int screen); // set current
	void ExitScreen(int screen);	// arg is optional, but good for checking
	void ClearScreenHistory();

	// Called by the individual screen for maintenance
	void StartTransitionFX(bool bVisited);
	void UpdateTransitionFX();

	LTBOOL	HasTransitionFX();

	bool IsTransitionComplete();
	void EndTransition();

	void SetLooping(bool bAllowLooping) {m_bAllowLooping = bAllowLooping;}

protected:

	void CheckForBestTransition(ChainFXList * pChainFX);

	void TermChain(ChainFXList * pChain);

	// Array of all the little arrays of ChainedFX
	TransitionFXList m_TransitionFX;
	ChainFXList * m_pCurrentChain;
	int m_iPreviousScreen;
	int m_iCurrentScreen;
	bool m_bAllowLooping;
};

#endif // _TRANSITIONFXMGR_H