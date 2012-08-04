// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndMission.h
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_END_MISSION_H_)
#define _SCREEN_END_MISSION_H_

#include "BaseScreen.h"

class CScreenEndMission : public CBaseScreen
{
public:
	CScreenEndMission();
	virtual ~CScreenEndMission();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	OnLButtonDown(int x, int y);
    virtual LTBOOL	OnRButtonDown(int x, int y);

    virtual LTBOOL	Render(HSURFACE hDestSurf);


protected:

	CLTGUITextCtrl		*m_pMission;
	CLTGUITextCtrl		*m_pAccuracy;
	CLTGUITextCtrl		*m_pMarksmanship;
	CLTGUITextCtrl		*m_pNonInjury;
	CLTGUITextCtrl		*m_pTimesHit;
	CLTGUIListCtrl		*m_pList;

	CLTGUITextCtrl		*m_pContinue;




};

#endif // !defined(_SCREEN_END_MISSION_H_)