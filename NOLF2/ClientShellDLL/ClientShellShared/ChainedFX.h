// ----------------------------------------------------------------------- //
//
// MODULE  : ChainedFX.h
//
// PURPOSE : Base class for chaining ClientFX
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_CHAINED_FX_H_)
#define _CHAINED_FX_H_

#include "ClientFXMgr.h"

struct INT_FX;

class CChainedFX
{
public:
	CChainedFX() : m_pLoopFXStruct(LTNULL) {}

	void	Init(char* szIntroName, char* szShortIntroName, char* szLoopName);

	void	Start(bool bUseShortIntro = false);
	void	End(); // Optional method for preemptively killing a ChainFX

	void	Update();

	bool	IsIntroDone() {return m_bIntroDone;}

	void	SetLooping(bool bLooping) {m_bAllowLooping = bLooping;}
	// ABM new fields added to make chainfx more generic
 	int	m_iFromScreen;
 	int m_iToScreen;

private:
	char m_szIntroName[128];
	char m_szShortIntroName[128];
	char m_szLoopName[128];

	bool m_bIntroDone;	// flag for when this chainfx is past any intro stage, possibly looping
	bool m_bAllowLooping;

	CLIENTFX_LINK	m_IntroFX;
	CLIENTFX_LINK	m_LoopFX;

	INT_FX*			m_pLoopFXStruct;
};

typedef std::vector<CChainedFX*> ChainFXList;

// ABM 2/5/02 Array of ChainFXlist.  Typedef used by the TransitionFXMgr for
// managing all of these little arrays
typedef std::vector<ChainFXList *> TransitionFXList;

#endif