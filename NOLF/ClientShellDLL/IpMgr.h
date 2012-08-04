/****************************************************************************
;
;	 MODULE:		IPMGR (.H)
;
;	PURPOSE:		IP Manager Classes
;
;	HISTORY:		11/09/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _IPMGR_H_
#define _IPMGR_H_


// Defines...

#define IPM_MAX_ADDRESS		32
#define IPM_MAX_IPS			256


// Externs...

class ILTClient;


// Classes...

class CIp
{
	// Member functions...

public:
	CIp() { Clear(); }
	~CIp() { Term(); }

	BOOL				Init(char* sIp);
	void				Term();
	void				Clear();

	char*				GetAddress() { return(m_sIp); }


	// Member variables...

private:
	char				m_sIp[IPM_MAX_ADDRESS + 2];
};

class CIpMgr
{
	// Member functions...

public:
	CIpMgr() { Clear(); }
	~CIpMgr() { Term(); }

    BOOL                Init(ILTClient* pClientDE);
	void				Term();
	void				Clear(BOOL bClearClientDE=TRUE);

	int					GetNumIps() { return(m_cIps); }
	BOOL				GetAllIpString(char* sBuf, int nBufSize);
	CIp*				GetIp(int i);

	BOOL				ExistIp(char* sIp);

	BOOL				AddIp(char* sIp);
	BOOL				AddIpFromEditControl(HWND hEdit, HWND hList = NULL);

	BOOL				RemoveIp(char* sIp);
	void				RemoveAll();
	BOOL				RemoveSelectedIpFromListBox(HWND hList);

	int					FillListBox(HWND hList);

	int					ReadIps();
	int					WriteIps();


	// Member variables...

private:
	int					m_cIps;
	CIp*				m_aIps[IPM_MAX_IPS];
    ILTClient*          m_pClientDE;
};


// Inlines...

inline void CIp::Clear()
{
	m_sIp[0] = '\0';
}

inline void CIp::Term()
{
	Clear();
}

inline void CIpMgr::Clear(BOOL bClearClientDE)
{
	m_cIps      = 0;

	if(bClearClientDE)
	{
		m_pClientDE = NULL;
	}

	for (int i = 0; i < IPM_MAX_IPS; i++) m_aIps[i] = NULL;
}

inline CIp* CIpMgr::GetIp(int i)
{
	 if (i < 0) return(NULL);
	 if (i >= m_cIps) return(NULL);
	 return(m_aIps[i]);
}


// EOF...

#endif