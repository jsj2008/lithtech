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
    bool   Build();
    void	Escape();

    virtual bool   HandleKeyDown(int key, int rep);
    virtual bool   OnMouseMove(int x, int y);


    void    OnFocus(bool bFocus);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	AddFilesToFilenames(FileEntry* pFiles, char* pPath);
	void	BuildCustomLevelsList(int32 nWidth);
//	void	BuildChapterList();

	
	// Sets up the server for local singleplayer game.
	bool	SetupServer( );

protected:

	CLTGUITextCtrl* m_pLoadCtrl;
//	CLTGUITextCtrl* m_pChapterCtrl;

	CLTGUIListCtrl* m_pDiff;
	CLTGUIListCtrlEx* m_pCustom;
	CLTGUIScrollBar* m_pCustomScroll;
//	CLTGUIListCtrl* m_pChapter;

	StringSet		m_Filenames;


};

#endif // _SCREEN_SINGLE_H_