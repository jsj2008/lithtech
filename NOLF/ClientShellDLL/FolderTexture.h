// FolderTexture.h: interface for the CFolderTexture class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_TEXTURE_H_
#define _FOLDER_TEXTURE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderTexture : public CBaseFolder
{
public:
	CFolderTexture();
	virtual ~CFolderTexture();

	// Build the folder
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);

	LTBOOL  IsSlider(CLTGUICtrl* pCtrl);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	SetSliders(int nLevel);
	void	CheckOverall();

	int				m_nOverall;
	int				m_nSliderVal[6];
	CSliderCtrl*	m_pSlider[6];
	CCycleCtrl*		m_pOverallCtrl;
};

#endif // _FOLDER_TEXTURE_H_