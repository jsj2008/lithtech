// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHost.h
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_HOST_H_
#define _SCREEN_HOST_H_

#include "BaseScreen.h"
#include "NetDefs.h"

class LabeledEditCtrl;
class GameRuleWStringLabeledEditCtrl;

class CScreenHost : public CBaseScreen
{
public:
	CScreenHost();
	virtual ~CScreenHost();

	// Build the screen
    bool   Build();
    bool   OnLeft();
    bool   OnRight();
    bool   OnLButtonUp(int x, int y);
    bool   OnRButtonUp(int x, int y);

    void    OnFocus(bool bFocus);
    void	Escape();

	virtual bool UpdateInterfaceSFX();
	void	ReadyLaunch(bool bReady);

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

    bool	LaunchGame();
	void	RunServerOptions();
	void	HandleLaunch();

	void	UpdateBandwidth(bool bResetMaxPlayers);
	void	UpdateGameType();
	void	SaveOptions();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	static void SessionNameValueChangingCB( std::wstring& wsValue, void* pUserData );
	static void PortValueChangingCB( std::wstring& wsValue, void* pUserData );
	static void ConfirmResetCB(bool bReturn, void *pData, void* pUserData);

	void	UpdatePunkBuster( );

	bool	m_bUsePassword;
	bool	m_bDedicated;
	bool	m_bAllowScmdCommands;
	bool 	m_bLan;
	bool	m_bUsePunkbuster;


	GameRuleWStringLabeledEditCtrl*	m_pSessionName;

	CLTGUIToggle*		m_pLanToggle;

	LabeledEditCtrl*	m_pPassword;
	std::wstring		m_sPassword;
	CLTGUIToggle*		m_pPassToggle;

	CLTGUIToggle*		m_pUsePunkbuster;

	LabeledEditCtrl*	m_pScmdPassword;
	std::wstring		m_sScmdPassword;
	CLTGUIToggle*		m_pScmdPassToggle;

	LabeledEditCtrl*	m_pPort;
	std::wstring		m_sPort;

	CLTGUICycleCtrl*	m_pBandwidthCycle;
	CLTGUIColumnCtrl*	m_pBandwidth;

	CLTGUITextCtrl*		m_pWeapons;
	CLTGUITextCtrl*		m_pDownload;
	CLTGUITextCtrl*		m_pVoting;

	CLTGUITextCtrl*		m_pOptionsFile;

	uint8				m_nBandwidth;
	std::wstring		m_sBandwidth;

	bool				m_bReadyToLaunch;
	uint8				m_nGameType;
	bool				m_bChangedContentDownload;
};



#endif // _SCREEN_HOST_H_
