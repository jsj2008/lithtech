// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerTeam.h
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREENPLAYERTEAM_H_
#define _SCREENPLAYERTEAM_H_

#include "BaseScreen.h"
#include "LayoutMgr.h"

class CScreenPlayerTeam : public CBaseScreen
{
public:
	CScreenPlayerTeam();
	virtual ~CScreenPlayerTeam();


	// Build the screen
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

	virtual void    Escape();

    virtual LTBOOL   OnMouseMove(int x, int y);
    virtual LTBOOL   OnUp();
    virtual LTBOOL   OnDown();
    virtual LTBOOL   OnLButtonDown(int x, int y);
    virtual LTBOOL   OnLButtonUp(int x, int y);


	// Returns false if the screen should exit as a result of this update
	virtual bool	UpdateInterfaceSFX();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual void	CreateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();

	void	CreateTeamFX(uint8 nTeam);
	void	CreateTeamAttachFX(INT_ATTACH *pAttach, uint8 nTeam);
	void	ClearTeamAttachFX(uint8 nTeam);

	void	SelectTeam(uint8 nTeam);
	void	UpdateTeam();

	CLTGUITextCtrl*	m_pTeams[2];
	CLTGUIListCtrl* m_pPlayers[2];
	CLTGUIFrame*	m_pFrame[2];
	CLTGUITextCtrl*	m_pAuto;

	uint8	m_nTeam;

	CBaseScaleFX	m_TeamSFX[2];
	int				m_nNumTeamAttachments[2];
	AttachmentData	m_aTeamAttachment[2][MAX_INT_ATTACHMENTS];


};


#endif // _SCREENPLAYERTEAM_H_