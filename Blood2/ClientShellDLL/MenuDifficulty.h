// MenuDifficulty.h: interface for the CMenuDifficulty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUDIFFICULTY_H__08A492F1_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUDIFFICULTY_H__08A492F1_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuDifficulty : public CMenuBase  
{
public:
	CMenuDifficulty();
	virtual ~CMenuDifficulty();

	// Build the menu
	void	Build();	
	
	// Access to member variables
	void	SetStoryMode(DBOOL bStory)		{ m_bStoryMode=bStory; m_bNightmares=DFALSE; }
	void	SetNightmaresMode(DBOOL bNight)	{ m_bNightmares=bNight; m_bStoryMode=DFALSE; }
	void	SetCharacter(int nCharacter)	{ m_nCharacter=nCharacter; }

	int		GetCharacter()					{ return m_nCharacter; }

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

	// Starts the game
	void	StartGame(DWORD dwDifficulty);

protected:
	DBOOL	m_bStoryMode;	// TRUE if we are to start a game in story mode
	DBOOL	m_bNightmares;	// TRUE if we are to start addon game in nightmares mode
	int		m_nCharacter;	// The character which is used in action mode
};

#endif // !defined(AFX_MENUDIFFICULTY_H__08A492F1_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_)
