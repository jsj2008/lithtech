// FolderEscape.h: interface for the CFolderEscape class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_ESCAPE_H_
#define _FOLDER_ESCAPE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderEscape : public CBaseFolder
{
public:
	CFolderEscape();
	virtual ~CFolderEscape();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	CLTGUITextItemCtrl* m_pResumeCtrl;
	CLTGUITextItemCtrl* m_pSaveCtrl;
	CLTGUITextItemCtrl* m_pLoadCtrl;

};

#endif // _FOLDER_ESCAPE_H_