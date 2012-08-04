// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_PLAYER_H_
#define _SCREEN_PLAYER_H_

#include "BaseScreen.h"

class CScreenPlayer : public CBaseScreen
{
public:
	CScreenPlayer();
	virtual ~CScreenPlayer();


	// Build the screen
    bool   Build();
	void	Term();

    void    OnFocus(bool bFocus);

	virtual bool OnLeft();
	virtual bool OnRight();
	virtual bool OnUp();
	virtual bool OnDown();
    virtual bool OnMouseMove(int x, int y);
    bool   OnLButtonUp(int x, int y);
    bool   OnRButtonUp(int x, int y);


	void	NextModel();
	void	PrevModel();

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	void	UpdateBandwidth();

	void	UpdateChar();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	SetupInsigniaPatches();

	void	UpdatePunkBuster();

	CLTGUIColumnCtrl*		m_pName;
	CLTGUIColumnCtrl*		m_pCDKeyCtrl;
	CLTGUICycleCtrl*		m_pPatchCycle;
	CLTGUITextCtrl*			m_pPatchPreSaleWarning;
	CLTGUITextCtrl*			m_pModel;
	CLTGUITextCtrl*			m_pDMModel;
	CLTGUITextCtrl*			m_pTeamModel;
	CLTGUITextureButton*	m_pPatch;
	CLTGUITextureButton*	m_pLeft;
	CLTGUITextureButton*	m_pRight;
	CLTGUITextCtrl*			m_pModelPreSaleWarning;

	uint32			m_nCurrentDMModel;
	uint32			m_nCurrentTeamModel;
	std::wstring	m_sPlayerName;
	std::string		m_sCurCDKey;
	std::string		m_sLastValidCDKey;

	CLTGUICycleCtrl*	m_pBandwidthCycle;
	CLTGUIColumnCtrl*	m_pBandwidth;
	uint8				m_nBandwidth;
	std::wstring		m_sBandwidth;
	

	CLTGUIToggle*		m_pAllowDownload;
	CLTGUIToggle*		m_pAllowRedirect;
	CLTGUICycleCtrl*	m_pMaxDownload;
	CLTGUIToggle*		m_pUsePunkbuster;

	bool				m_bAllowBroadcast;

	bool				m_bAllowContentDownload;
	bool				m_bAllowRedirect;
	uint8				m_nMaxDownload;
	bool				m_bUsePunkbuster;

	uint8				m_nPatch;
	StringArray			m_sPatches;
	StringArray			m_sPreSalePatches;
};


#endif // _SCREEN_PLAYER_H_