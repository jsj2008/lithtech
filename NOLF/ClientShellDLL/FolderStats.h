// FolderStats.h: interface for the CFolderStats class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERSTATS_H__4E96EB42_4A57_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERSTATS_H__4E96EB42_4A57_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderStats : public CBaseFolder
{
public:
	CFolderStats();
	virtual ~CFolderStats();

	// Build the folder
    LTBOOL   Build();
	void	Term();
	void	Escape();

    void    OnFocus(LTBOOL bFocus);
    uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    virtual LTBOOL	HandleKeyDown(int key, int rep);
    virtual LTBOOL	HandleKeyUp(int key);
	virtual LTBOOL	OnUp() {return OnPageUp();}
	virtual LTBOOL	OnDown() {return OnPageDown();}

    virtual LTBOOL HandleForceUpdate() { UpdateData(); return LTTRUE; }

private:

	void BuildGearList();
	void UpdateData();
	void SetCommandKey();

	int m_nKey;
};

#endif // !defined(AFX_FOLDERSTATS_H__4E96EB42_4A57_11D3_B2DB_006097097C7B__INCLUDED_)