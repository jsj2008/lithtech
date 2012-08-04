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
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual void        UpdateLayout();

	virtual	void		Show(bool bDraw, bool bScreen = false);


	enum Constants
	{
		kMaxPlayers = 16,
		kNumTeams = 2,
	};

protected:

	void		UpdateTeamPos(uint8 nTeam, LTIntPt pos);

	CLTGUITextCtrl		m_Server;
	CLTGUITextCtrl		m_Team[kNumTeams];
	CLTGUITextCtrl		m_Rounds[kNumTeams];
	CLTGUIColumnCtrl	m_Header[kNumTeams];
	CLTGUIColumnCtrl	m_Columns[kNumTeams][kMaxPlayers];
	CLTGUIFrame			m_Frame[kNumTeams];
	CLTGUIFrame			m_SingleFrame;
	
	int			m_nDraw;
	bool		m_bScreen;
    LTIntPt		m_BasePos;
	uint8		m_nBaseFontSize;
	uint32		m_nTextColor;
	uint32		m_nPlayerTextColor;
	uint32		m_nScreenTextColor;
	uint32		m_nScreenPlayerTextColor;
	bool		m_bControlsInited;
	float		m_fScale;
	uint16		m_nFrameWidth;


};



#endif