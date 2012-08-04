// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSingle.cpp
//
// PURPOSE : Interface screen for starting, loading, and saving single player
//				games.
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_SINGLE_H_
#define _SCREEN_SINGLE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "ClientUtilities.h"

class CScreenSingle : public CBaseScreen
{
public:
	CScreenSingle();
	virtual ~CScreenSingle();

	// Build the screen
    LTBOOL   Build();
    void	Escape();

    virtual LTBOOL   HandleKeyDown(int key, int rep);
    virtual LTBOOL   OnMouseMove(int x, int y);


    void    OnFocus(LTBOOL bFocus);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	AddFilesToFilenames(FileEntry* pFiles, char* pPath);
	void	BuildCustomLevelsList(int nWidth);
	void	BuildChapterList();

	
	// Sets up the server for local singleplayer game.
	bool	SetupServer( );

protected:

	CLTGUITextCtrl* m_pLoadCtrl;
	CLTGUITextCtrl* m_pChapterCtrl;

	CLTGUIListCtrl* m_pDiff;
	CLTGUIListCtrl* m_pCustom;
	CLTGUIListCtrl* m_pChapter;

	CLTGUIFrame*	m_pDiffFrame;
	CLTGUIFrame*	m_pCustomFrame;
	CLTGUIFrame*	m_pChapterFrame;


	StringSet		m_Filenames;


};

#endif // _SCREEN_SINGLE_H_