// FolderFavoriteLevels.h: interface for the CFolderFavoriteLevels class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_FAVORITE_LEVELS_H_
#define _FOLDER_FAVORITE_LEVELS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderFavoriteLevels : public CBaseFolder
{
public:
	CFolderFavoriteLevels();
	virtual ~CFolderFavoriteLevels();

	// Build the folder
    LTBOOL   Build();

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

};

#endif // _FOLDER_FAVORITE_LEVELS_H_