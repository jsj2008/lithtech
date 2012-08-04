// FolderDifficulty.h: interface for the CFolderDifficulty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_DIFF_H_
#define _FOLDER_DIFF_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderDifficulty : public CBaseFolder
{
public:
	CFolderDifficulty();
	virtual ~CFolderDifficulty();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

};

#endif // _FOLDER_DIFF_H_