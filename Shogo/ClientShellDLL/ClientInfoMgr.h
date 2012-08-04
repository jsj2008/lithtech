#ifndef __CLIENTINFOMGR_H
#define __CLIENTINFOMGR_H

#include "ltbasedefs.h"
#include "ClientUtilities.h"

class ILTClient;

struct CLIENT_INFO
{
	CLIENT_INFO()	{ r = g = b = 0; m_Ping = 0.0f; nID = 0; hstrName = LTNULL; nFrags = 0; hName = LTNULL; szName.cx = szName.cy = 0; hFragCount = LTNULL; szFragCount.cx = szFragCount.cy = 0; pPrev = LTNULL; pNext = LTNULL;}

	LTFLOAT			m_Ping;
	uint32			nID;
	HSTRING			hstrName;
	int				nFrags;

	HSURFACE		hName;
	CSize			szName;
	
	HSURFACE		hFragCount;
	CSize			szFragCount;

	// Player color.
	uint8			r, g, b, padding;

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

class CClientInfoMgr
{
public:

	CClientInfoMgr();
	~CClientInfoMgr();

	void	Init (ILTClient* pClientDE);

	void	AddClient (HSTRING hstrName, uint32 nID, int nFragCount, uint8 r, uint8 g, uint8 b);
	void	RemoveClient (uint32 nID);
	void	RemoveAllClients();
	
	void	AddFrag (uint32 nLocalID, uint32 nID);
	void	RemoveFrag (uint32 nLocalID, uint32 nID);

	CLIENT_INFO* GetClientByID(uint32 nID);
	
	uint32	GetNumClients();
	char*	GetPlayerName (uint32 nID);

	void	UpdateFragDisplay (int nFrags);

	void	UpdateAllFragSurfaces();
	void	ClearAllFragSurfaces();
	void	UpdateSingleFragSurface (CLIENT_INFO* pClient);
	void	ClearUpToDate() {m_bFragSurfacesUpToDate = LTFALSE;}

	void	Draw (LTBOOL bDrawSingleFragCount, LTBOOL bDrawAllFragCounts);

protected:

	ILTClient*			m_pClientDE;
	CLIENT_INFO*		m_pClients;

	uint32				m_nLastFontSize;
	LTBOOL				m_bFragSurfacesUpToDate;

	HSURFACE			m_hFragDisplay;
	uint32				m_cxFragDisplay;
	uint32				m_cyFragDisplay;
};

#endif