// FolderIntel.h: interface for the CFolderIntel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_INTEL_
#define _FOLDER_INTEL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSelectionFolder.h"
#include "IntelItemMgr.h"

class CFolderIntel : public CBaseSelectionFolder
{
public:
	CFolderIntel();
	virtual ~CFolderIntel();

	// Build the folder
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

	virtual	LTBOOL	UpdateSelection();

	void	SetMissionNum(int nNum) {m_nMissionNum = nNum;}

	virtual void	UpdateInterfaceSFX();


protected:
    uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void BuildIntelList();
	void ClearIntelList();

	virtual void	CreateModelSFX();
	virtual void	RemoveInterfaceSFX();

	void UpdateData();

	int	 m_nMissionNum;
	CMoArray<IntelItem *> m_intelArray;

	LTVector m_vScale;
	LTBOOL	 m_bChromakey;
};

#endif 