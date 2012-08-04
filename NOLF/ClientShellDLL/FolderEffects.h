// FolderEffects.h: interface for the CFolderEffects class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_EFFECTS_H_
#define _FOLDER_EFFECTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderEffects : public CBaseFolder
{
public:
	CFolderEffects();
	virtual ~CFolderEffects();

	// Build the folder
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	LTBOOL	m_bTracers;
	LTBOOL	m_bMuzzleLight;
	LTBOOL	m_bShellCasings;
	LTBOOL	m_bWeather;
	int		m_nImpact;
	int		m_nDebris;


};

#endif // _FOLDER_EFFECTS_H_