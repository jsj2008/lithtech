// FolderGallery.h: interface for the CFolderGallery class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_GALLERY_H_
#define _FOLDER_GALLERY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderGallery : public CBaseFolder
{
public:
	CFolderGallery();
	virtual ~CFolderGallery();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
	void	BuildMissionList();
	void	ClearMissionList();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

};

#endif // _FOLDER_GALLERY_H_