// FolderSave.h: interface for the CFolderSave class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERSAVE_H__A20A45C0_5AE1_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERSAVE_H__A20A45C0_5AE1_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderSave : public CBaseFolder
{
public:
	CFolderSave();
	virtual ~CFolderSave();

    LTBOOL Build();
    void OnFocus(LTBOOL bFocus);
	void Escape();

	LTBOOL HandleKeyDown(int key, int rep);
	virtual LTBOOL OnLButtonUp(int x, int y);
	virtual LTBOOL OnRButtonUp(int x, int y);

	LTBOOL SaveGame(uint32 slot);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelList();
	void	ClearSavedLevelList();

	void	NameSaveGame(uint32 slot, int index);

	CLTGUIEditCtrl		*m_pEdit;
	char				m_szSaveName[40];


};

#endif // !defined(AFX_FOLDERSAVE_H__A20A45C0_5AE1_11D3_B2DB_006097097C7B__INCLUDED_)