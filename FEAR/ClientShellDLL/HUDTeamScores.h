// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTeamScores.h
//
// PURPOSE : HUD Element to display team scores
//
// CREATED : 03/23/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDTEAMSCORES_H__
#define __HUDTEAMSCORES_H__

//******************************************************************************************
//** HUD Team Score display
//******************************************************************************************
class CHUDTeamScores : public CHUDItem
{
public:
	CHUDTeamScores();
	virtual ~CHUDTeamScores() {}	

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();

	virtual void	OnExitWorld() {m_bUpdated = false;}

protected:

	bool			m_bDraw;
	bool			m_bUpdated;
	CLTGUIString	m_Scores[MAX_TEAMS];
	LTVector2n		m_vScoreOffset[MAX_TEAMS];
	uint32			m_TeamColors[MAX_TEAMS];

};


#endif  // __HUDTEAMSCORES_H__
