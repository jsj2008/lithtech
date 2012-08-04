// FolderWeaponControls.h: interface for the CFolderWeaponControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_WEAPON_CONTROLS_H_
#define _FOLDER_WEAPON_CONTROLS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderWeaponControls : public CBaseFolder
{
public:
	CFolderWeaponControls();
	virtual ~CFolderWeaponControls();

	// Build the folder
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

	LTBOOL	OnLeft();
	LTBOOL	OnRight();
	LTBOOL	OnLButtonUp(int x, int y);
	LTBOOL	OnRButtonUp(int x, int y);
	LTBOOL	OnMouseMove(int x, int y);

	virtual LTBOOL HandleKeyDown(int key, int rep);

	virtual void	UpdateInterfaceSFX();

protected:

	void	WriteBindings();
	void	ReadBindings();

	void	UpdateSelection();

	int		GetNextCommand(int nCommand);
	int		GetPreviousCommand(int nCommand);

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


	virtual void	CreateModelSFX();
	virtual void	RemoveInterfaceSFX();


	int		m_nActions[10];
	LTBOOL	m_bLargeIcon;
	int		m_nIconAlpha;
	LTBOOL	m_bUseNumbers;


	CBaseScaleFX	m_ModelSFX;
	char			m_szModel[WMGR_MAX_FILE_PATH];
	char			m_szSkin[WMGR_MAX_FILE_PATH];
	LTFLOAT			m_fSFXRot;
	LTFLOAT			m_fScale;
	LTVector		m_vOffset;


};

#endif // _FOLDER_WEAPON_CONTROLS_H_