// FolderLoad.h: interface for the CFolderLoad class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERLOAD_H__A20A45C1_5AE1_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERLOAD_H__A20A45C1_5AE1_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

struct SaveGameData
{
	SaveGameData() {szWorldName[0] = LTNULL;szUserName[0] = LTNULL;szTime[0] = LTNULL;}
	char szWorldName[128];
	char szUserName[128];
	char szTime[128];
};


class CFolderLoad : public CBaseFolder
{
public:
	CFolderLoad();
	virtual ~CFolderLoad();

    LTBOOL Build();
    void OnFocus(LTBOOL bFocus);

	LTBOOL HandleKeyDown(int key, int rep);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	BuildSavedLevelList();
	void	ClearSavedLevelList();
	void	ParseSaveString(char* pszSaveStr, SaveGameData *pSG, LTBOOL bUserName = LTTRUE);


};

#endif // !defined(AFX_FOLDERLOAD_H__A20A45C1_5AE1_11D3_B2DB_006097097C7B__INCLUDED_)