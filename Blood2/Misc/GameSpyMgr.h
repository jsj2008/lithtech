/****************************************************************************
;
;	 MODULE:		GAMESPYMGR (.H)
;
;	PURPOSE:		Game Spy Manager
;
;	HISTORY:		09/21/98  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _GAMESPYMGR_H_
#define _GAMESPYMGR_H_


// Includes...

#include "AfxSock.h"


// Defines...

#define	MAX_RESPONSE		4096
#define TIMER_HEARTBEAT		1 * 60 * 1000
#define VALIDATE_SIZE		6

#define GSMF_USEGAMEPORTFORHEARTBEAT	0x00000001


// Libs...

#pragma comment (lib, "winmm.lib")

#pragma comment (lib, "GameSpyMgr.lib")

// Classes...

class CGameSpySocket : public CAsyncSocket
{
public:
	void					OnReceive(int nErrorCode);
};

class CGameSpyQueryInfo
{
	// Member functions...

public:
	CGameSpyQueryInfo() { Clear(); }

	void					Clear();

	BOOL					IsValid();

	CString					GetData() { return(m_sData); }
	CString					GetAddr() { return(m_sAddr); }
	DWORD					GetPort() { return(m_dwPort); }

	void					SetData(const char* sData) { m_sData = sData; }
	void					SetAddr(const char* sAddr) { m_sAddr = sAddr; }
	void					SetPort(DWORD dwPort) { m_dwPort = dwPort; }


	// Member variables...

private:
	CString					m_sData;
	CString					m_sAddr;
	DWORD					m_dwPort;
};

class CGameSpySender
{
public:
	virtual void	SendTo(const void* pData, unsigned long len, const char* sAddr, unsigned long port) = 0;
};

class CGameSpyMgr
{
	// Member functions...

public:
	CGameSpyMgr() { Clear(); }
	~CGameSpyMgr() { Term(); }

	virtual	BOOL			Init(const char* sGameName, const char* sGameVer, const char* sGameKey, int nGameLoc, int nGamePort, int nSpyPort, DWORD dwFlags = 0);
	virtual	void			Term();
	virtual	void			Clear();

	virtual	int				Update();

	BOOL					OnQuery(CGameSpyQueryInfo* pQueryInfo);
	BOOL					OnQuery(char* sAddr, DWORD dwPort, char* sData, int nLen);
	BOOL					OnEchoQuery(const char* sQuery);
	BOOL					OnSecureQuery(const char* sValue);
	BOOL					OnBasicQuery();
	BOOL					OnStatusQuery();

	virtual	BOOL			OnInfoQuery() = 0;
	virtual	BOOL			OnRulesQuery() = 0;
	virtual	BOOL			OnPlayersQuery() = 0;

	void					SendResponseInfo(const char* sKey, const char* sValue);
	void					SendResponseInfo(const char* sKey, int nValue);

	DWORD					GetGamePort() { return(m_dwGamePort); }
	DWORD					GetSpyPort() { return(m_dwSpyPort); }

	void					SetGameSpyAddr(const char* sAddr) { m_sGameSpyAddr = sAddr; }
	void					SetSendHandler(CGameSpySender* pHandler) { m_pSender = pHandler; }

	BOOL					UseGamePortForHeartbeat() { return(m_dwFlags & GSMF_USEGAMEPORTFORHEARTBEAT); }

private:
	void					StartResponse(const char* sAddr, DWORD dwPort);
	void					SendResponse();
	void					FlushResponse();

	void					AddResponseQueryID();
	BOOL					GetNextQuery(CGameSpyQueryInfo* pQueryInfo);
	BOOL					UpdateHeartbeat();

	CString					GetFirstQueryToken(const char* sQuery);
	CString					GetNextQueryToken();

	BOOL					SendSocketString(const char* sAddr, DWORD dwPort, const char* sText, int nLength = -1);
	void					TraceSocketError();


	// Member variables...

protected:
	CString					m_sGameName;
	CString					m_sGameVer;
	CString					m_sGameKey;
	CString					m_sGameLoc;
	CString					m_sGameSpyAddr;
	DWORD					m_dwQueryID;
	int						m_nPacket;
	char					m_sResponse[MAX_RESPONSE + 64];
	DWORD					m_timerHeartbeat;
	CGameSpySocket			m_socket;
	CGameSpySender*			m_pSender;
	DWORD					m_dwGamePort;
	DWORD					m_dwSpyPort;
	CString					m_sResponseAddr;
	DWORD					m_dwResponsePort;
	DWORD					m_dwFlags;
};


// Inlines...

inline void CGameSpyQueryInfo::Clear()
{
	m_sData.Empty();
	m_sAddr.Empty();
	m_dwPort = 0;
}

inline BOOL CGameSpyQueryInfo::IsValid()
{
	if (m_sData.IsEmpty()) return(FALSE);
	if (m_sAddr.IsEmpty()) return(FALSE);

	return(TRUE);
}

inline void CGameSpyMgr::Clear()
{
	m_sGameName.Empty();
	m_sGameVer.Empty();
	m_sGameKey.Empty();
	m_sGameLoc.Empty();
	m_sGameSpyAddr.Empty();
	m_sResponseAddr.Empty();

	m_sResponse[0] = '\0';

	m_dwQueryID      = 1;
	m_nPacket        = 0;
	m_timerHeartbeat = 0;
	m_dwGamePort     = 0;
	m_dwSpyPort      = 0;
	m_dwResponsePort = 0;
	m_dwFlags        = 0;
	m_pSender        = NULL;
}


// EOF...

#endif

	