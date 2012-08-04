// FolderMods.h: interface for the CFolderMods class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_MODS_H_
#define _FOLDER_MODS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSelectionFolder.h"

class CFolderMods : public CBaseSelectionFolder
{
public:
	CFolderMods();
	virtual ~CFolderMods();

	// Build the folder
    LTBOOL	Build();

	// This is called when the folder gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);
	virtual void	UpdateInterfaceSFX();

    virtual uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    HSTRING         GetHelpString(uint32 dwHelpId, int nControlIndex);

	virtual	LTBOOL	UpdateSelection();
    virtual LTBOOL	IsAvailable();
	void	SkipOutfitting();

protected:

	void			FillArray();
	void			ClearArray();
    void			BuildModsList();
	void			SaveModData();
	void			ClearModsList();
	void			SetContinue();

	virtual void	CreateModelSFX();
	virtual void	RemoveInterfaceSFX();

	int*			m_nMods;
	int				m_nAvailMods;

};

#endif // _FOLDER_MODS_H_