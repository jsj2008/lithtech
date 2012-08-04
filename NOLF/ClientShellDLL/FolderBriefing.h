// FolderBriefing.h: interface for the CFolderBriefing class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_BRIEFING_H_
#define _FOLDER_BRIEFING_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderBriefing : public CBaseFolder
{
public:
	CFolderBriefing();
	virtual ~CFolderBriefing();

	// Build the folder
    LTBOOL   Build();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	SetPostMission(LTBOOL bPost) {m_bPostMission = bPost;}


protected:
	CLTGUITextItemCtrl	*m_pMissionCtrl;
	CStaticTextCtrl		*m_pBriefTextCtrl;

	LTBOOL				m_bPostMission;
	
	HLTSOUND			m_hSnd;

};

#endif // _FOLDER_BRIEFING_H_