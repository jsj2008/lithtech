// FolderHost.h: interface for the CFolderHost class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_HOST_H_
#define _FOLDER_HOST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "NetDefs.h"

class CFolderHost : public CBaseFolder
{
public:
	CFolderHost();
	virtual ~CFolderHost();

	// Build the folder
    LTBOOL   Build();
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

    void    OnFocus(LTBOOL bFocus);
	void	Escape();

	virtual void UpdateInterfaceSFX();
	void	ReadyLaunch(LTBOOL bReady);

protected:
	void FillGameStruct(NetGame *pNG);
    LTBOOL LaunchGame();
    LTBOOL SetService( );
	void RunServerOptions();
	void	HandleLaunch();

	LTBOOL	IsValidPort(uint16 nPort);
	LTBOOL	IsValidBandwidth(uint32 nBandwidth);

	void UpdateBandwidth();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	int	m_nGameType;
	char m_szSessionName[MAX_SESSION_NAME];
	char m_szPassword[MAX_PASSWORD];
	char m_szPort[MAX_PASSWORD];
	LTBOOL m_bUsePassword;

	CGroupCtrl			*m_pNameGroup;
	CLTGUIEditCtrl		*m_pEdit;
	CLTGUITextItemCtrl	*m_pLabel;

	CGroupCtrl			*m_pPassGroup;
	CLTGUIEditCtrl		*m_pPassEdit;
	CLTGUITextItemCtrl	*m_pPassLabel;
	CToggleCtrl			*m_pPassToggle;

	CGroupCtrl			*m_pPortGroup;
	CLTGUIEditCtrl		*m_pPortEdit;
	CLTGUITextItemCtrl	*m_pPortLabel;

	CCycleCtrl* m_pTypeCtrl;

	CCycleCtrl*			m_pBandwidthCycle;
	CGroupCtrl*			m_pBandwidthGroup;
	CLTGUIEditCtrl*		m_pBandwidthEdit;
	CLTGUITextItemCtrl*	m_pBandwidthLabel;

	int					m_nBandwidth;
	char				m_szBandwidth[64];

	LTBOOL				m_bReadyToLaunch;
};


inline LTBOOL CFolderHost::IsValidPort(uint16 nPort)
{
	return LTTRUE;
}

inline LTBOOL CFolderHost::IsValidBandwidth(uint32 nBandwidth) 
{
	return (nBandwidth >= 1000 && nBandwidth <= 1000000);
}

#endif // _FOLDER_HOST_H_