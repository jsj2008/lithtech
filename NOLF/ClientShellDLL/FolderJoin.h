// FolderJoin.h: interface for the CFolderJoin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_JOIN_H_
#define _FOLDER_JOIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "gamespyclientmgr.h"


// Defines...

#define FSS_IDLE				0			// Find Servers State
#define	FSS_GETSERVICES			1
#define	FSS_GETPINGS			2
#define FSS_GETALLDATA			3
#define FSS_GETSELPING			4
#define FSS_DUMMYSTATUS			5

#define POP_FILTER_ALL			0			// population filter
#define POP_FILTER_NOTEMPTY		1
#define POP_FILTER_NOTFULL		2
#define POP_FILTER_NOTBOTH		3
#define POP_FILTER_LAST			POP_FILTER_NOTBOTH



class CFolderJoin : public CBaseFolder
{
public:
	CFolderJoin();
	virtual ~CFolderJoin();

	// Build the folder
    LTBOOL   Build();
	virtual void	Term();
	
    LTBOOL   Render(HSURFACE hDestSurf);

    void    DrawBar(HSURFACE hDestSurf,LTRect *rect);
    void    DrawFrame(HSURFACE hDestSurf,LTRect *rect);

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	HandleKeyDown(int key, int rep);

	void	Escape();

	virtual LTBOOL OnUp();
	virtual LTBOOL OnDown();
	virtual LTBOOL OnEnter();
	virtual LTBOOL OnLButtonUp(int x, int y);
	virtual LTBOOL OnLButtonDblClick(int x, int y);
	virtual LTBOOL OnRButtonUp(int x, int y);


	CGameSpyClientMgr*	GetGameSpyClientMgr() { return(&m_GameSpyClientMgr); }
	void*				GetCurGameServerHandle() { return(m_pCurServerHandle); }
	void				SetCurGameServerHandle(void* pHandle);

	int					GetGameFilter() { return(m_nGameFilter); }
	int					GetPopFilter() { return(m_nPopFilter); }
	int					GetServerSort() { return(m_nServerSort); }
	int					GetPlayerSort() { return(m_nPlayerSort); }

	void				SetState(int nNewState);
	void				SetStatusText(HSTRING hStr);
	void				SetServerCounts(int c1, int c2) { m_nNumServers = c1; m_nNumServersListed = c2; }
	void				SetPopFilter(int f) { m_nPopFilter = f; }
	void				SetGameFilter(int f) { m_nGameFilter = f; }
	void				SetServerSort(int s) { m_nServerSort = s; }
	void				SetPlayerSort(int s) { m_nPlayerSort = s; }

	void				ForceNextUpdate() { m_bForceNextUpdate = TRUE; }


	void	Update(HSURFACE hDestSurf);
	void	UpdateGetServices(HSURFACE hDestSurf);
	void	UpdateGetPings(HSURFACE hDestSurf);
	void	UpdateGetAllData(HSURFACE hDestSurf);
	void	UpdateIdle(HSURFACE hDestSurf);
	void	UpdateGetSelPing(HSURFACE hDestSurf);
	void	UpdateDummyStatus(HSURFACE hDestSurf);

    void	JoinCurGame(LTBOOL bPasswordSet = LTFALSE);
    LTBOOL  DoJoinGame(CGameSpyServer* pGame);

	void	AskForPassword();
	LTBOOL	AskingForPassword() {return m_bAskingForPassword;}

	void	SortServers();
	void	SortPlayers();

	LTBOOL	IsCurrentGame(CGameSpyServer* pGame);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	SetDummyStatusState(HSTRING hStr, DWORD dwWaitTime, int nNextState = FSS_IDLE);
	int		AddServerCtrl(CGameSpyServer* pGame);
	int		AddPlayerCtrl(CGameSpyPlayer* pPlr);
	int		AddOptionCtrl(int nID, int nStringId);
	int		AddOptionCtrl(int nID, char *pszValue);
//	int		AddOptionCtrl(char *pszOption, char *pszValue);
    void    UpdateLists(LTBOOL bForce = LTFALSE);
    void    UpdateServers(LTBOOL bForce = LTFALSE);
    void    UpdatePlayers(LTBOOL bForce = LTFALSE);
    void    UpdateOptions(LTBOOL bForce = LTFALSE);
    void    UpdateCommands(LTBOOL bForce = LTFALSE);

private:
	int						m_nState;

	CGameSpyClientMgr		m_GameSpyClientMgr;
	void*					m_pCurServerHandle;

    LTBOOL                   m_bForceNextUpdate;
    LTBOOL                   m_bNeedServerSorting;
    uint32                  m_timeNextSelPing;
    uint32                  m_timeNextAllData;
    uint32                  m_timeStateStart;
    uint32                  m_timeDummyEnd;
	HSTRING					m_hServersShown;
	HSTRING					m_hDummyStatus;
	HSTRING					m_hStatus;
	int						m_nNextDummyState;
	int						m_nNumServers;
	int						m_nNumServersListed;
	int						m_nPopFilter;
	int						m_nVersionFilter;
	int						m_nGameFilter;
	int						m_nServerSort;
	int						m_nPlayerSort;

	CListCtrl*				m_pServerList;
	CListCtrl*				m_pPlayerList;
	CListCtrl*				m_pOptionList;

	CLTGUITextItemCtrl*		m_pRefresh;
	CLTGUITextItemCtrl*		m_pRePing;
	CLTGUITextItemCtrl*		m_pVersionFilter;
	CLTGUITextItemCtrl*		m_pGameFilter;
	CLTGUITextItemCtrl*		m_pPopFilter;
	CLTGUITextItemCtrl*		m_pResort;
	CLTGUITextItemCtrl*		m_pJoin;

	LTBOOL					m_bAskingForPassword;
	char					m_szPassword[MAX_PASSWORD];
	CLTGUIEditCtrl			*m_pPassEdit;
	CLTGUITextItemCtrl		*m_pPassLabel;
	CBitmapCtrl				*m_pPassBack;


};

#endif // _FOLDER_JOIN_H_