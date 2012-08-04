// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSave.h
//
// PURPOSE : Interface screen for saving games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_SAVE_H_)
#define _SCREEN_SAVE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenSave : public CBaseScreen
{
public:
	CScreenSave();
	virtual ~CScreenSave();

    bool Build();
    void OnFocus(bool bFocus);

	bool HandleKeyDown(int key, int rep);

	bool SaveGame(uint32 slot);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelList();
	void	ClearSavedLevelList();

	void	NameSaveGame(uint32 slot, int index);

	wchar_t	m_wszSaveName[40];
	CLTGUIListCtrl *m_pList;

};

#endif // !defined(_SCREEN_SAVE_H_)