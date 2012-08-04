// FolderSummary.h: interface for the CFolderSummary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FOLDERSUMMARY_H_)
#define _FOLDERSUMMARY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderSummary : public CBaseFolder
{
public:
	CFolderSummary();
	virtual ~CFolderSummary();

	// Build the folder
    LTBOOL  Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);
    uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	LTBOOL	HandleKeyDown(int key, int rep);

    virtual LTBOOL HandleForceUpdate() { UpdateData(); return LTTRUE; }
private:

	void UpdateData();
};

#endif // !defined(_FOLDERSUMMARY_H_)