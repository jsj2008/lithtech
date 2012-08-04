// FolderCrosshair.h: interface for the CFolderCrosshair class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERCROSSHAIR_H__64448061_B8A2_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_FOLDERCROSSHAIR_H__64448061_B8A2_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderCrosshair : public CBaseFolder
{
public:
	CFolderCrosshair();
	virtual ~CFolderCrosshair();

	// Build the folder
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);
    LTBOOL   Render(HSURFACE hDestSurf);

    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

protected:
	void	GetConsoleVariables();
	void	SetConsoleVariables();
	void	WriteConsoleVariables();

protected:

	CCycleCtrl		*m_pStyle;

    LTBOOL           m_bCrosshair;
	int				m_nAlpha;
	int				m_nColorR;
	int				m_nColorG;
	int				m_nColorB;
	int				m_nStyle;
    LTBOOL           m_bDynamic;

};

#endif // !defined(AFX_FOLDERCROSSHAIR_H__64448061_B8A2_11D3_B2DB_006097097C7B__INCLUDED_)