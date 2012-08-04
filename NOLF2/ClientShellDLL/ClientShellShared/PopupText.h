// PopupText.h: interface for the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _POPUPTEXT_INCLUDED_
#define _POPUPTEXT_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LTGuiMgr.h"

class CPopupText
{
public:
	CPopupText();
	virtual ~CPopupText();

	void	Init(bool bUsePopupState=true);
	void	Term();


	void	Show(uint32 nTextId, uint8 nPopupId) { DisplayPopup( nPopupId, LTNULL, nTextId ); }
	void	Show(char* pText, uint8 nPopupId) { DisplayPopup( nPopupId, pText, (uint32)-1 ); }
	void	Update();
	void	Close();

#ifndef __PSX2
	LTBOOL OnKeyDown(int key, int rep);
#else
	LTBOOL OnInterfaceCommand(int command);
#endif	

	void	Draw();

	LTBOOL  IsVisible() { return m_bVisible;}

private:

	void	DisplayPopup( uint8 nPopupId, char *pText, uint32 nTextId );
	LTBOOL	m_bVisible;
	bool	m_bUsePopupState;

	bool	m_bWeaponsEnabled;

	CLTGUIFrame			m_Frame;
	CLTGUITextCtrl		m_Text;
/*
	void	ClearSurfaces();
	void	ShowText(int nStringId);
	void	CreateScaleFX(char *szFXName);

	LTRect			m_rcRect;
	LTIntPt			m_pos;
	uint32			m_dwWidth;
	uint32			m_dwHeight;

	int				m_nLineHeight;
#ifndef __PSX2 //*@
	int				m_nKey;

	HSURFACE		m_hForeSurf;
	CLTGUIFont		*m_pForeFont;
#else
	*/
};

#endif // _POPUPTEXT_INCLUDED_