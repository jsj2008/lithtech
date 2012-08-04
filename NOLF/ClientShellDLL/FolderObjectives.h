// FolderObjectives.h: interface for the CFolderObjectives class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_OBJECTIVES_H_
#define _FOLDER_OBJECTIVES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderObjectives : public CBaseFolder
{
public:
	CFolderObjectives();
	virtual ~CFolderObjectives();

	// Build the folder
    LTBOOL   Build();
	void	Term();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


protected:
	CLTGUITextItemCtrl	*m_pMissionCtrl;
	CLTGUITextItemCtrl	*m_pSpacerCtrl;
	CLTGUITextItemCtrl	*m_pSkipCtrl;
	CLTGUITextItemCtrl	*m_pSelectCtrl;

	void	BuildObjectivesList();

	eFolderID m_eNextFolder;

};

#endif // _FOLDER_OBJECTIVES_H_