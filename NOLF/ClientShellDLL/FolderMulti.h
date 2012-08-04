// FolderMulti.h: interface for the CFolderMulti class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_MULTI_H_
#define _FOLDER_MULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderMulti : public CBaseFolder
{
public:
	CFolderMulti();
	virtual ~CFolderMulti();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

};

#endif // _FOLDER_MULTI_H_