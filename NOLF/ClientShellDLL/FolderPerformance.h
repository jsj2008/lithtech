// FolderPerformance.h: interface for the CFolderPerformance class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_PERFORMANCE_H_
#define _FOLDER_PERFORMANCE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderPerformance : public CBaseFolder
{
public:
	CFolderPerformance();
	virtual ~CFolderPerformance();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);

	void	ConfirmSetting(LTBOOL bConfirm);

	int		GetOverall();
	void	SetOverall(int n);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	
	CCycleCtrl*		m_pPerformance;
	CToggleCtrl*	m_pSoundFilters; 

	LTBOOL	m_bSoundFilters;
	int		m_nOverall;

	uint32	m_nSettingToConfirm;

};

#endif // _FOLDER_PERFORMANCE_H_