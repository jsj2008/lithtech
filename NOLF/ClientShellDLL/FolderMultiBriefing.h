// FolderMultiBriefing.h: interface for the CFolderMultiBriefing class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_MP_BRIEFING_H_
#define _FOLDER_MP_BRIEFING_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderMultiBriefing : public CBaseFolder
{
public:
	CFolderMultiBriefing();
	virtual ~CFolderMultiBriefing();

	// Build the folder
    LTBOOL   Build();
	void	Escape();

	LTBOOL	OnEnter() {Escape(); return LTTRUE;}
	LTBOOL	OnLButtonUp(int x, int y) {Escape(); return LTTRUE;}

    void    OnFocus(LTBOOL bFocus);
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


protected:


};

#endif // _FOLDER_MP_BRIEFING_H_