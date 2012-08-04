//*************************************************************************
//*************************************************************************
//***** MODULE  : MenuCharacter.h
//***** PURPOSE : Blood 2 Character Creation Screen
//***** CREATED : 10/11/98
//*************************************************************************
//*************************************************************************

#if !defined(AFX_MENUCHARACTER_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUCHARACTER_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//*************************************************************************

#include "MenuBase.h"
#include "SharedDefs.h"

//*************************************************************************

#ifdef _ADDON
#define		MAX_CHARACTER_INFO		28
#else
#define		MAX_CHARACTER_INFO		20
#endif

#define		MAX_ATTRIBUTE			5

#define		ARROW_CTRLS				0

#define		MIN_STAT_INDEX			1
#define		MAX_STAT_INDEX			4

#define		MIN_WEAPON_INDEX		5
#define		MAX_WEAPON_INDEX		13

//*************************************************************************

typedef struct CharacterMenuInfo
{
	char	*szFile;
	DDWORD	nCharacter;
	DDWORD	nColor;
}	CharacterMenuInfo;

//*************************************************************************

class CMenuCharacter : public CMenuBase  
{
	public:
		CMenuCharacter();
		virtual ~CMenuCharacter();	

		// Build the menu
		void	Build();		

		// Override the left and right controls
		void	OnLeft();
		void	OnRight();

		// Renders the menu to a surface
		void	Render(HSURFACE hDestSurf);

		// This is called to determine whether we should ask the user if they wish to switch
		// resolutions back to the previous resolution if it had been changed to 640x480.
		void	SetResolutionSwitch(DBOOL bAsk, int nOldWidth, int nOldHeight);

		// This is called when the menu gets or loses focus
		void	OnFocus(DBOOL bFocus);

		// Extra member functions for file control
		DBOOL	SaveB2CFile(char *szFile);
		DBOOL	LoadB2CFile(char *szFile);
		DBOOL	DeleteB2CFile(char *szFile);

		void	UpdateScreenFromStruct();

	protected:
		DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

		void	BuildExtraCtrls();

		void	CheckSelectedLinks();

		void	ChangeCharacterPic(int index);
		void	UpdateCharacterStruct();
		void	SetPlayerIndexFromStruct();

		DBOOL	IsWeaponADupe(int index);

		// Custom control setup functions
		CLTGUIFadeItemCtrl* InitFadeItemCtrl(char *lpszOptionSurfPrefix, int nSurfaces, char *lpszOptionText, DWORD dwCommandID, int x, int y);
		CLTGUITextItemCtrl* InitNumberCtrl(int low, int high, int index, DWORD dwCommandID, int x, int y);
		CLTGUITextItemCtrl* InitWeaponCtrl(int index, DWORD dwCommandID, int x, int y, DBOOL enable);
		CLTGUITextItemCtrl* InitWeaponNumCtrl(int index, DWORD dwCommandID, int x, int y, DBOOL enable);

		// Extra member variables
		HSURFACE	m_hCharacterPic;
		HDECOLOR	m_hTransColor;

		int			m_PlayerIndex;
		DDWORD		m_ExtraPoints;

		B2C			m_Player;

		CLTGUIFadeItemCtrl		*m_hRightArrow;

		CLTGUITextItemCtrl		*m_hStats[4];

		CLTGUIFadeItemCtrl		*m_hExtraCtrl;
		CLTGUITextItemCtrl		*m_hExtraNumCtrl;

		CLTGUIFadeItemCtrl		*m_hWeaponsCtrl;
		CLTGUITextItemCtrl		*m_hWeaponNums[10];
		CLTGUITextItemCtrl		*m_hWeapons[10];

		DBOOL		m_bSwitchResolutions;
		DBOOL		m_bGoingToFilesMenu;

		int			m_nOldScreenWidth;
		int			m_nOldScreenHeight;
};

//*************************************************************************

#endif // !defined(AFX_MENUCHARACTER_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)