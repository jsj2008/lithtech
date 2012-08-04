// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoin.h
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_JOIN_H_
#define _SCREEN_JOIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenJoin : public CBaseScreen
{
public:
	CScreenJoin();
	virtual ~CScreenJoin();

	// Build the screen
    LTBOOL   Build();

	LTBOOL	Render(HSURFACE hDestSurf);

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	HandleKeyDown(int key, int rep);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	uint32	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	void FindServers();

	CLTGUITextCtrl*	m_pFindCtrl;
	CLTGUITextCtrl*	m_pStatusCtrl;
	CLTGUIListCtrl*	m_pServerListCtrl;
	CLTGUIColumnCtrl* m_pCDKeyCtrl;

	struct CServerEntry
	{
		std::string m_sName;
		std::string m_sAddress;
		std::string m_sMap;
		uint32 m_nNumPlayers;
		uint32 m_nMaxPlayers;
	};
	typedef std::vector<CServerEntry> TServerList;
	TServerList m_cServerList;
	uint32 m_nSelectedServer;

	std::string m_sCurCDKey;

	// Read the server list from the server directory
	void ReadCurServerList();
	// Dump the server list into the display
	void DisplayCurServerList();

	bool IsCurrentGame(const char *pAddr);
	void JoinCurGame();
	bool DoJoinGame(const CServerEntry &cServer);
	
	enum EState {
		eState_Inactive,
		eState_Startup,
		eState_UpdateDir,
		eState_QueryDetails,
		eState_ChangeCDKey,
		eState_DisplaySysMOTD,
		eState_DisplayGameMOTD,
		eState_Waiting,
		eState_Transition,
		eState_Error
	};
	EState m_eCurState;

	// Called when entering a state
	bool PreState(EState eNewState);
	// Called when leaving a state
	// Returns false if that is an invalid state transition
	bool PostState(EState eNewState);
	// Change states
	bool ChangeState(EState eNewState);
	void Update();

	// Pre-state handling for each state
	bool PreState_Inactive();
	bool PreState_Startup();
	bool PreState_UpdateDir();
	bool PreState_QueryDetails();
	bool PreState_ChangeCDKey();
	bool PreState_Waiting();

	// Post-state handling for each state
	bool PostState_UpdateDir(EState eNewState);
	bool PostState_QueryDetails(EState eNewState);
	bool PostState_Waiting(EState eNewState);

	// Update handling for each state
	void Update_State_Startup();
	void Update_State_UpdateDir();
};

#endif // _SCREEN_MULTI_H_