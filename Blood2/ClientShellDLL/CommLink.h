//*************************************************************************
//*************************************************************************
//***** MODULE  : CommLink.h
//***** PURPOSE : Blood 2 Communcation Tab
//***** CREATED : 9/29/98
//*************************************************************************
//*************************************************************************

#ifndef __COMMLINK_H__
#define __COMMLINK_H__

#include "basedefs_de.h"
#include "cpp_clientshell_de.h"
#include "LTGUIMgr.h"

//**********************************************************

#define		COMMTAB_INVISIBLE		0
#define		COMMTAB_SCROLL_IN		1
#define		COMMTAB_STOPPED			2
#define		COMMTAB_SCROLL_OUT		3

#define		COMMTEXT_INVISIBLE		0
#define		COMMTEXT_SCROLL_IN		1
#define		COMMTEXT_STOPPED		2
#define		COMMTEXT_SCROLL_OUT		3

#define		COMM_SCROLL_TIME		0.2f

#define		COMM_MAX_VOICE			200
#define		COMM_MAX_TEXT			500
#define		COMM_MAX_LINES			4

#define		COMM_CALEB				(DBYTE)0
#define		COMM_OPHELIA			(DBYTE)1
#define		COMM_ISHMAEL			(DBYTE)2
#define		COMM_GABREILLA			(DBYTE)3
#define		COMM_VOICE				(DBYTE)4
#define		COMM_UNKNOWN			(DBYTE)5

#define		COMM_BUSY				0
#define		COMM_STARTED			1

//**********************************************************

class CCommLink
{
	public:
		// Constructors, destructors, and intialization functions
		CCommLink();
		~CCommLink();

		DBOOL	Init(CClientDE* pClientDE);
		void	AdjustRes();
		void	Term();

		// Setup the communication
		char	StartCommunication(DBYTE nPic, char *szFile, char *szText);
		char	StartCommunication(char *szPic, char *szFile, char *szText);

		// Animation and drawing functions
		void	Draw(DBOOL bDrawBar);

	protected:
		// Private data members
		CClientDE*	m_pClientDE;

		HSURFACE	m_hFace;			// surface to hold the character face
		HSURFACE	m_hText;			// surface to hold the text

		// Fonts and cursors
		CoolFontCursor	*m_pCommCursor;
		CoolFont		*m_pCommFont1;

		// Communication variables
		DBYTE		m_nCommLevel;
		DBYTE		m_nTextLevel;
		char		m_szCommVoice[COMM_MAX_VOICE];
		char		m_szCommText[COMM_MAX_TEXT];
		DFLOAT		m_fCommUpdateTime;
		DFLOAT		m_fCommScrollRatio;
		DFLOAT		m_fCommLength;

		DFLOAT		m_fTextUpdateTime;
		DFLOAT		m_fTextScrollRatio;
		DFLOAT		m_fTextScrollTime;
		DFLOAT		m_fTextLength;

		HSOUNDDE	m_sCommSound;
		DBOOL		m_bPlayVoice;

		// Communcation locations
		short		m_nCommTextWidth;
		DDWORD		m_nFaceWidth;
		DDWORD		m_nCommTextX;
		DDWORD		m_nCommTextY;

		// General information variables
		DDWORD		m_nScreenWidth;
		DDWORD		m_nScreenHeight;
		HDECOLOR	m_hTransColor;
		HDECOLOR	m_hTextColor;
};

#endif	// _COMMLINK_H_