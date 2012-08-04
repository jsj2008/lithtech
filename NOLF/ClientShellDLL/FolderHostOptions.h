// FolderJoin.h: interface for the CFolderJoin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_HOST_OPTIONS_H_
#define _FOLDER_HOST_OPTIONS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"


class CFolderHostOptions : public CBaseFolder
{
public:
	CFolderHostOptions();
	virtual ~CFolderHostOptions();

	// Build the folder
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    LTBOOL   IsAllowedGameType(int nGameType) {return (nGameType == 0 || nGameType == m_nGameType);}
protected:
	int		m_nGameType;
	int		*m_anValues;
};

#endif // _FOLDER_HOST_OPTIONS_H_