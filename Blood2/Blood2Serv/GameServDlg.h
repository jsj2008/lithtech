#if !defined(AFX_GAMESERVDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_)
#define AFX_GAMESERVDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GameServDlg.h : header file
//


// Includes...

#include "ServerUtils.h"
#include "MyGameSpyMgr.h"
#include "NetDefs.h"


// Structures...

typedef struct PlayerInfo_t
{
	char	sName[64];
	int		nFrags;
	DWORD	dwID;
	DWORD	dwPing;

}	PLAYERINFO;


/////////////////////////////////////////////////////////////////////////////
// CGameServDlg dialog

class CGameServDlg;

class CGameServSendHandler : public CGameSpySender
{
public:
	CGameServSendHandler() { m_pDlg = NULL; }

	void				SetDlg(CGameServDlg *pDlg) {m_pDlg = pDlg;}
	void				SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port);
	
protected:
	CGameServDlg*		m_pDlg;
};

class CGameServDlg : public CDialog
{
// Construction
public:
	CGameServDlg(CWnd* pParent = NULL);   // standard constructor
	~CGameServDlg() { Term(); }

// Dialog Data
	//{{AFX_DATA(CGameServDlg)
	enum { IDD = IDD_GAMESERVER };
	CListBox	m_lbPlayers;
	CListBox	m_lbLevels;
	CEdit	m_edConsole;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameServDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGameServDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnQuit();
	afx_msg void OnServer();
	afx_msg void OnConsoleSend();
	afx_msg void OnConsoleClear();
	afx_msg void OnCommandsNextLevel();
	afx_msg void OnPlayersBoot();
	afx_msg void OnSelchangePlayersList();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnCommandsOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
	// Member functions...

public:
	BOOL				Init();
	void				Term();
	void				Clear();

	CString				GetCurLevel() { return(m_sCurLevel); }
	int					GetNumPlayers() { return(m_cPlayers); }
	PLAYERINFO*			GetFirstPlayerInfo();
	PLAYERINFO*			GetNextPlayerInfo();
	CString				GetGameSpyGameMode() { return(m_sGameSpyGameMode); }
	CString				GetGameSpyGameType() { return(m_sGameSpyGameType); }

	BOOL				StartServer();
	BOOL				StopServer();
	BOOL				IsRunning() { return(m_bRunning); }
	BOOL				IsLevelChanging() { return(m_bLevelChanging); }

	void				UpdateServerMgr(DWORD timeDelta);
	BOOL				UpdateServerRegistration(DWORD timeDelta);
	BOOL				UpdateGameSpyMgr(DWORD timeDelta);
	void				UpdateConsoleVars();

	BOOL				UnregisterServer();

	void				RemoveMessage(int nMsg, int nMax);

	void				WriteConsoleString(LPCTSTR pMsg, ...);
	void				WriteConsoleString(int nStringID);
	void				WriteServerError();

	void				OnUpdate();
	void				UpdateTimers();
	void				OnShellMessage(char* pMsg);
	void				OnConsoleOutput(char* pMsg);
	void				OnOutOfMemory();
	void				OnProcessNetPacket(char* sData, DWORD nLen, BYTE senderAddr[4], DWORD senderPort);

	ServerInterface*	GetServerMgr() {return m_pServerMgr;}
	void				SetConfigFilename(char *pName) {m_pConfigFilename = pName;}
	CRITICAL_SECTION*	GetCS() {return &m_CS;}

private:
	void				OnStandardUpdate(char* pMsg);
	void				OnLevelChanging(char* pMsg);
	void				OnLevelChangeUpdate(char* pMsg);
	void				OnLevelChangeStop();
	void				OnConsoleMessageUpdate(char* pMsg);


	// Member variables...

private:
	CRITICAL_SECTION	m_CS; // So we don't switch levels and update at the same time.
	BOOL				m_bCSInitted;

	char				*m_pConfigFilename;
	CGameServSendHandler	m_SendHandler;
	ServerInterface*	m_pServerMgr;
	BOOL				m_bRunning;
	BOOL				m_bLevelChanging;
	DWORD				m_timeServerStart;
	DWORD				m_timeServerRunning;
	DWORD				m_timeServerLast;
	DWORD				m_timeLevelStart;
	DWORD				m_timeLevelRunning;
	DWORD				m_timeLevelLast;
	int					m_iPlayer;
	int					m_cPlayers;
	int					m_nCurLevel;
	CString				m_sCurLevel;
	HANDLE				m_hThread;
	DWORD				m_dwThreadID;
	CMyGameSpyMgr		m_GameSpyMgr;
	CString				m_sGameSpyGameMode;
	CString				m_sGameSpyGameType;
	PLAYERINFO			m_aPis[MAX_MULTI_PLAYERS + 2];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMESERVDLG_H__CB06D3C1_2B86_11D2_860A_00609719A842__INCLUDED_)
