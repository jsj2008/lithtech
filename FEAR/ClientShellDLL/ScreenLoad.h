// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLoad.h
//
// PURPOSE : Interface screen for loading saved games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_LOAD_H_)
#define _SCREEN_LOAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

struct SaveGameData
{
	SaveGameData() {szWorldName[0] = NULL;wszUserName[0] = NULL;szTime[0] = NULL;}
	char szWorldName[128];
	wchar_t wszUserName[128];
	char szTime[128];
};


class CScreenLoad : public CBaseScreen
{
public:
	CScreenLoad();
	virtual ~CScreenLoad();

    bool Build();
    void OnFocus(bool bFocus);
	virtual void	Escape();

	bool HandleKeyDown(int key, int rep);

protected:

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelLists();
	void	ClearSavedLevelLists();
	void	ParseSaveString(char const* pszWorldName, wchar_t const* pszTitle, time_t const& saveTime, SaveGameData *pSG, bool bUserName);
	CLTGUIColumnCtrl* BuildSaveControls( char const* pszIniKey, uint32 nCommandId, const char* szControlHelpStringId, 
									bool bUserName );

};

#endif // !defined(_SCREEN_LOAD_H_)