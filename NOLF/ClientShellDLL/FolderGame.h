// FolderGame.h: interface for the CFolderGame class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_GAME_H_
#define _FOLDER_GAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderGame : public CBaseFolder
{
public:
	CFolderGame();
	virtual ~CFolderGame();

	// Build the folder
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);


protected:


    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
    int		            m_nSubtitles;
    LTBOOL				m_bGore;
    LTBOOL				m_bFadeBodies;
	int					m_nDifficulty;
	LTBOOL				m_bAlwaysRun;
	int					m_nLayout;
	int					m_nHeadBob;
	int					m_nWeaponSway;
	int					m_nPickupMsgDur;
	LTBOOL				m_bObjMessages;

	CLTGUICycleCtrl		*m_pDifficultyCtrl;			// The difficulty control

};

#endif // _FOLDER_GAME_H_