// FolderMission.h: interface for the CFolderMission class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERMISSION_H__989CCE40_4669_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERMISSION_H__989CCE40_4669_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderMission : public CBaseFolder
{
public:
	CFolderMission();
	virtual ~CFolderMission();

	// Build the folder
    LTBOOL   Build();
	void	Term();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    virtual LTBOOL   HandleKeyDown(int key, int rep);


protected:
	CLTGUITextItemCtrl*	m_pMissionCtrl;
	void	BuildObjectivesList();


};

#endif // !defined(AFX_FOLDERMISSION_H__989CCE40_4669_11D3_B2DB_006097097C7B__INCLUDED_)