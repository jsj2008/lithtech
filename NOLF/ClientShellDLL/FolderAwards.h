// FolderAwards.h: interface for the CFolderAwards class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FOLDERAWARDS_H_)
#define _FOLDERAWARDS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderAwards : public CBaseFolder
{
public:
	CFolderAwards();
	virtual ~CFolderAwards();

	// Build the folder
    LTBOOL  Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	UpdateData();
    LTBOOL	HasAwards() {return m_controlArray.GetSize() > 1;}


};

#endif // !defined(_FOLDERAWARDS_H_)