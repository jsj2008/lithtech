// FolderPlayer.h: interface for the CFolderPlayer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_PLAYER_H_
#define _FOLDER_PLAYER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderPlayer : public CBaseFolder
{
public:
	CFolderPlayer();
	virtual ~CFolderPlayer();

	// Build the folder
    LTBOOL   Build();
	void	Term();

	virtual void	Escape();
    virtual void    OnFocus(LTBOOL bFocus);
	virtual void	CreateInterfaceSFX();
	virtual void	UpdateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();


	virtual LTBOOL	OnLButtonUp(int x, int y);
	virtual LTBOOL	OnRButtonUp(int x, int y);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL   CreatePlayerModel(LTBOOL bNewSkin = LTTRUE);
	void	BuildModelList();
	void	ClearModelList();


	CGroupCtrl			*m_pNameGroup;
	CLTGUIEditCtrl		*m_pEdit;
	CLTGUITextItemCtrl	*m_pLabel;
	CCycleCtrl			*m_pModelCtrl;
	CCycleCtrl			*m_pSkinCtrl;
	CCycleCtrl			*m_pHeadCtrl;
	char	m_aszHeads[32][128];
	char	m_aszSkins[32][128];
	char	m_szPlayerName[MAX_PLAYER_NAME];
	char	m_szPlayerModel[128];
	char	m_szPlayerSkin[128];
	char	m_szPlayerHead[128];
	char	m_szModName[128];
	char	m_szStyleName[128];
	int		m_nTargetNameTransparency;
	int		m_nTargetNameSize;
	int		m_nModNum;
	int		m_nSkinNum;
	int		m_nHeadNum;
	LTBOOL	m_bRestoreSkinHead;
	int		m_nTeam;
	int		m_nConnect;
	LTBOOL	m_bAutoSwitchWeapons;
	LTBOOL	m_bAutoSwitchAmmo;
	LTBOOL	m_bIgnoreTaunts;

};

#endif // _FOLDER_PLAYER_H_