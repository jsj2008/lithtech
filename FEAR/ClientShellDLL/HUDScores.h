// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScores.h
//
// PURPOSE : Definition of CHUDScores to display player scores
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_SCORES_H
#define __HUD_SCORES_H

#include "HUDItem.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDScores : public CHUDItem
{
public:
	CHUDScores();
	

    virtual bool	Init();
	virtual void	Term();

    virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	SetBasePos( LTVector2n vBasePos );

	virtual void	UpdateLayout( );

	virtual	void	Show(bool bDraw, bool bScreen = false);

	LTVector2n		GetBaseSize( ) const { return LTVector2n( m_SingleFrame.GetBaseWidth(), m_SingleFrame.GetBaseHeight()); }

	virtual void	OnExitWorld() { m_bDraw = false; };

	enum Constants
	{
		kMaxPlayers = 16,
		kNumTeams = 2,
	};

protected:

	void		UpdateTeamPos(uint8 nTeam, LTVector2n pos);

	CLTGUITextCtrl		m_Server;
	CLTGUITextCtrl		m_RoundInfo;
	CLTGUITextCtrl		m_Team[kNumTeams];
	CLTGUITextCtrl		m_Rounds[kNumTeams];
	CLTGUIColumnCtrl	m_Header[kNumTeams];
	CLTGUIColumnCtrl	m_Columns[kNumTeams][kMaxPlayers];
	CLTGUIFrame			m_Frame[kNumTeams];
	CLTGUIFrame			m_SingleFrame;
	TextureReference	m_hFrameTexture[kNumTeams];
	
	bool		m_bDraw;
	bool		m_bScreen;
	bool		m_bFirstScreenUpdate;

	uint32		m_cPlayerTextColor;
	uint32		m_cPlayerDeadColor;
	uint32		m_cDeadColor;

	bool		m_bControlsInited;
	uint32		m_nFrameWidth;
	bool		m_bInitialized;
};



#endif