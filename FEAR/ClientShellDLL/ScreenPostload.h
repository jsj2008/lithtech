// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPostload.h
//
// PURPOSE : Interface screen to be displayed after loading a level but before
//				starting to play it.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_POSTLOAD_H_)
#define _SCREEN_POSTLOAD_H_

#include "BaseScreen.h"

class CScreenPostload : public CBaseScreen
{
public:
	CScreenPostload();
	virtual ~CScreenPostload();

	// Build the screen
    virtual bool	Build();
    virtual void	OnFocus(bool bFocus);
	virtual void	Escape();

	virtual bool	UpdateInterfaceSFX();

    virtual bool   HandleKeyDown(int key, int rep) { Escape(); return true; }
    virtual bool   OnLButtonDown(int x, int y) { Escape(); return true;}

    virtual bool   Render();

protected:

	virtual void	CreateInterfaceSFX();

	CLTGUITextCtrl	m_MissionName;
	CLTGUITextCtrl	m_LevelName;
	CLTGUITextCtrl	m_Briefing;
	CLTGUITextCtrl	m_Help;
	CLTGUITextCtrl	m_Continue;

	std::string		m_photo;

	TextureReference	m_hPhoto;
	LTPoly_GT4			m_photoPoly;
	LTPoly_GT4		m_BackPoly;


	bool	m_bPressAnyKey;
	uint8	m_nLoadDelay; //delay the client loaded message

};

#endif // !defined(_SCREEN_PRELOAD_H_)