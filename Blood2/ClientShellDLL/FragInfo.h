//*********************************************************************************

#ifndef __FRAGINFO_H__
#define __FRAGINFO_H__

//*********************************************************************************

#include "basedefs_de.h"
#include "ClientUtilities.h"
#include "LTGUIMgr.h"

//*********************************************************************************

#ifdef _ADDON
#define NUM_FRAG_PICS		10
#else
#define NUM_FRAG_PICS		6
#endif

#define FRAG_PIC_VOICE		(NUM_FRAG_PICS-2)
#define FRAG_PIC_UNKNOWN	(NUM_FRAG_PICS-1)

//*********************************************************************************

struct CLIENT_INFO
{
	CLIENT_INFO()	{ nID = 0; hstrName = DNULL; nFrags = 0; nPing = 0; pPrev = DNULL; pNext = DNULL;}

	DDWORD			nID;
	HSTRING			hstrName;
	int				nFrags;
	int				nPing;
	DBYTE			byCharacter;

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

//*********************************************************************************

#define		FRAG_COUNT_OFFSET		20
#define		FRAG_COLUMN_SPACE		32

//*********************************************************************************

class CTeamMgr;
class CTeam;

class CFragInfo
{
	public:

		CFragInfo();
		~CFragInfo();

		void	Init (CClientDE* pClientDE, CTeamMgr* pTeamMgr);

		void	AddClient (HSTRING hstrName, DDWORD nID, int nFragCount, DBYTE nCharacter);
		void	RemoveClient (DDWORD nID);
		void	RemoveAllClients();
		
		void	AddFrag (DDWORD nLocalID, DDWORD nID);
		void	AddFrags (DDWORD nLocalID, DDWORD nID, int nFrags);
		void	RemoveFrag (DDWORD nLocalID, DDWORD nID);

		void	UpdatePlayerPing(DDWORD dwPlayerID, int nPing);
		
		DDWORD	GetNumClients();
		char*	GetPlayerName (DDWORD nID);
		DBYTE	GetPlayerCharacter(DDWORD nID);
		CLIENT_INFO*	GetClientInfo(DDWORD dwID);
		char*	GetDisplayPingAndName(CLIENT_INFO* pClient, char* sBuf);

		void	Draw (DBOOL bDrawSingleFragCount, DBOOL bDrawAllFragCounts);
		void	Draw (DBOOL bDrawSingleFragCount, DBOOL bDrawAllFragCounts, HSURFACE hSurf);

		void	AdjustRes();
		void	UpdateFragTable();

		void	DrawClip(char* sText, int nMax);
		void	DrawTeams();
		void	DrawSingleColumnTeams();
		void	DrawTeamColumn(CTeam* pTeam, int xStart, int yStart, int nMax);

		void	TurnOn();
		void	TurnOff();


	protected:

		CClientDE*			m_pClientDE;
		CTeamMgr*			m_pTeamMgr;
		CLIENT_INFO*		m_pClients;

		// Fonts and font cursors
		CoolFontCursor		*m_pCursor;
		CoolFont			*m_pFont;

		// Surfaces to draw on
		HSURFACE			m_hFragPics[NUM_FRAG_PICS];
		HSURFACE			m_hFragBar;
		HSURFACE			m_hFragTable;

		// Location variables
		DDWORD				m_nFragTableX;
		DDWORD				m_nFragTableY;
		DDWORD				m_nFragTableHeight;
		DDWORD				m_nFragTableMaxDisp;

		// General varaibles
		DDWORD				m_nScreenWidth;
		DDWORD				m_nScreenHeight;
		int					m_nTeamID;

		HDECOLOR	m_hTransColor;
};

//*********************************************************************************

#endif __FRAGINFO_H__